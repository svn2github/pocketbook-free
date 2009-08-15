#include <iostream>
#include <string>
#include <vector>

#include "config.hpp"
#include "ical-parser.hpp"

using namespace std;
using namespace ical;

#ifndef TRACE_ICAL_PARSING
#undef dbg
// Ugly, but...
ostringstream null;
#define dbg null
#endif

char* read_stream(char *s, size_t size, void *d)

{
	char *c = fgets(s, size, (FILE*) d);
	return c;
}

string safe_string(const char* s) {
	if (!s)
		return "";

	return string(s);
}

const string& ical_parser::file() const {
	return ical_file;
}

string ical_parser::component_name(icalcomponent* c) {
	switch (icalcomponent_isa(c)) {
	case ICAL_VCALENDAR_COMPONENT:
		return "CALENDAR";
	default:
		return "UNKNOWN";
	}
}

bool ical_parser::required_participant(icalproperty* pp) {
	icalparameter* rp = icalproperty_get_first_parameter(pp,
			ICAL_ROLE_PARAMETER);
	parameter_g pg(rp);
	if (!rp) {
		dbg << "Failed to get role" << endl;
		return true;
	}

	switch (icalparameter_get_role(rp)) {
	case ICAL_ROLE_REQPARTICIPANT:
		return true;
	case ICAL_ROLE_OPTPARTICIPANT:
	default:
		return false;
	}
}

bool ical_parser::add_participants(list<participant>& participants,
		icalcomponent* cc) {
	icalproperty* pp = icalcomponent_get_first_property(cc,
			ICAL_ATTENDEE_PROPERTY);
	property_g pg(pp);
	do {
		const char* attc = icalproperty_get_attendee(pp);
		if (!attc) {
			dbg << "Could not get attendee property" << endl;
			continue;
		}

		std::string att(attc);

		// TODO: FIX against mailto:
		short pos = att.find(':', 0);
		if (pos != -1)
			att = att.substr(pos + 1, att.length() - pos);
		participant p(att, true);
		p.required = required_participant(pp);
		participants.push_front(p);
		dbg << "Added " << (p.required ? "required" : "optional")
				<< " participant: " << p.name << endl;
	} while ((pp = icalcomponent_get_next_property(cc, ICAL_ATTENDEE_PROPERTY)));
	return !participants.empty();
}

std::ostream& ical::operator <<(std::ostream& o, const event& ev) {
	o << "START:" << ev.start << endl;
	o << "DURATION: " << ev.duration << endl;
	o << "SUMMARY: " << ev.summary << endl;
	o << "LOCATION: " << ev.location << endl;
	o << "DESCRIPTION: " << ev.description << endl;
	o << "RECURRENT: " << (ev.recurrent ? "YES" : "NO") << endl;
	return o;
}

void ical_parser::parse_calendar(icalcomponent* c, schedule& s,
		bg::date bmonth, bg::date emonth) {
	// VCALENDAR
	icalcomponent *cc = icalcomponent_get_first_component(c,
			ICAL_VEVENT_COMPONENT);

	using namespace boost::gregorian;

	while (cc) {
		while (true) {
			// Will automatically free
			component_g g(cc);

			dbg << endl << "===============" << endl;
			icaltimetype dtstart = icalcomponent_get_dtstart(cc), dtend =
					icalcomponent_get_dtend(cc);
			bg::date start(dtstart.year, dtstart.month, dtstart.day), end(
					dtend.year, dtend.month, dtend.day);

			event ev;

			ev.summary = safe_string(icalcomponent_get_summary(cc));
			ev.location = safe_string(icalcomponent_get_location(cc));
			ev.description = safe_string(icalcomponent_get_description(cc));
			ev.start = bp::ptime(start, bp::time_duration(dtstart.hour,
					dtstart.minute, dtstart.second));
			icaldurationtype dur = icalcomponent_get_duration(cc);
			bp::ptime evend = ev.start + days(dur.days) + bp::time_duration(
					dur.hours, dur.minutes, dur.seconds);
			ev.duration = evend - ev.start;

			icalproperty* rrule = icalcomponent_get_first_property(cc,
					ICAL_RRULE_PROPERTY);
			ev.recurrent = true;
			if (!rrule)
				ev.recurrent = false;

			if (start > emonth || (end < bmonth && !ev.recurrent)) {
				dbg << ev << endl << ">>> SKIPPED AS NOT RELEVANT" << endl;
				break;
			}

			if (!ev.recurrent) {
				// For the relevant event, get participants list
				add_participants(ev.participants, cc);

				s.events.insert(ev);
				dbg << ev << endl;
				dbg << ">>> EVENT ADDED!" << endl;
				break;
			}

			icalrecurrencetype recur = icalproperty_get_rrule(rrule);
			icaltimetype rstart = icaltime_from_day_of_year(
					bmonth.day_of_year(), bmonth.year());

			icalrecur_iterator* ritr = icalrecur_iterator_new(recur, rstart);

			if (!ritr) {
				dbg << ">>> SKIPPED (All events in past)" << endl;
				break;
			}

			icaltimetype next = icalrecur_iterator_next(ritr);
			if (icaltime_is_null_time(next)) {
				dbg << ">>> SKIPPED (All events in past)" << endl;
				break;
			}

			dbg << "Adding recurrencies for event " << endl;
			dbg << ev << endl;

			// For the relevant event, get participants list
			add_participants(ev.participants, cc);
			for (; !icaltime_is_null_time(next) && date(next.year, next.month,
					next.day) < emonth; next = icalrecur_iterator_next(ritr)) {
				date nd = date(next.year, next.month, next.day);
				//DONE: FIXME: workaround against wrong time returned in icalrecur_iterator_next
				bp::ptime ndt(nd, ev.start.time_of_day());
				dbg << "Event recurs at" << ndt << endl;
				event rev = ev;
				rev.start = ndt;
				s.events.insert(rev);

			}
			break;
		}
		cc = icalcomponent_get_next_component(c, ICAL_VEVENT_COMPONENT);

	}
	dbg << endl << "---------------" << endl;
	dbg << "EXIT: parse_calendar" << endl;
}

ical_parser::ical_parser(const std::string& file) :
	ical_file(file), parser(icalparser_new()) {
}

ical_parser::~ical_parser() {
	// Be tidy
	icalparser_free(parser);
}

void ical_parser::fill_events(schedule& s, bg::date start, bg::date end) {
	using namespace boost::gregorian;

	// TODO: Move to C++ streams
	FILE* stream = fopen(ical_file.c_str(), "r");
	if (!stream) {
		dbg << "File '" << ical_file << "' does not exist!" << endl;
		// TODO: Show message
		return;
	}

	icalparser_set_gen_data(parser, stream);

	char* line = 0;
	do {
		line = icalparser_get_line(parser, read_stream);
		icalcomponent *c = icalparser_add_line(parser, line);

		if (c != 0) {
			//component_g cg(c);

			// TODO: what is icalcomponent_foreach_recurrence
			dbg << "Component type: " << icalcomponent_isa(c) << " ("
					<< component_name(c) << ")" << endl;

			switch (icalcomponent_isa(c)) {
			case ICAL_VCALENDAR_COMPONENT:
				break;
			default:
				dbg << "Skipping component!" << endl;
				continue;
			}

			parse_calendar(c, s, start, end);
		}

	} while (line != 0);
	dbg << "EXIT: Fill_events" << endl;
}


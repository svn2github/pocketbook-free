/*
 * ical-parser.hpp
 *
 *  Created on: Jun 25, 2009
 *      Author: fax
 */

#ifndef ICALPARSER_HPP_
#define ICALPARSER_HPP_

#include <string>
#include <list>

#include <iostream>
#include <queue>
#include <set>

#include <libical/ical.h>

#include "config.hpp"

namespace ical {
struct participant {
	std::string name;
	bool required;

	participant(const std::string& name_, bool required_) :
		name(name_), required(required_) {
	}

	participant() {
	}
};

struct event {
	std::string summary;
	std::string location;
	std::string description;
	bool recurrent;
	bool has_reminder;

	boost::posix_time::ptime start;
	boost::posix_time::time_duration duration;

	std::list<participant> participants;
	// TODO: Add reminder list
	//std::set<reminder> reminders;

	/*	event(const string& s, const string& l, const list<string>& pl,
	 bg::date& start, bg::date end) :
	 summary(s), location(l), participants(pl), period(start, end) {
	 }*/

	bool is_full_day() const {
		return duration.hours() >= 24;
	}
};
std::ostream& operator <<(std::ostream& o, const event& ev);

template<typename T, void(*free_func)(T*)>
class guard {
public:
	guard(T* obj) :
		obj_(obj) {
	}

	~guard() {
		//		std::dbg << "Destructing " << obj_ << " with function="
		//				<< (long) (free_func) << std::endl;
		free_func(obj_);
	}
private:
	T* obj_;
};

typedef guard<icalcomponent, icalcomponent_free> component_g;
typedef guard<icalparameter, icalparameter_free> parameter_g;
typedef guard<icalproperty, icalproperty_free> property_g;
}
namespace std {
template<>
struct less<ical::event> :
		public binary_function<ical::event, ical::event, bool> {
	bool operator()(const ical::event& __x, const ical::event& __y) const {
		return __x.start < __y.start || (__x.start == __y.start
				&& __x.is_full_day());
	}
};
}
namespace ical {
//TODO: Add virtual event END-OF-MONTH, s.t. reminder refill will take place
struct schedule {
	typedef std::set<event> container;
	//std::priority_queue<event> events;
	container events;
};

struct monthly_schedule:
		public schedule {
	const std::vector<event> events;
};

class ical_parser {
	std::string ical_file;
	icalparser *parser;

private:
	std::string component_name(icalcomponent* c);
	bool required_participant(icalproperty* pp);
	bool add_participants(std::list<participant> & participants,
			icalcomponent * cc);
	void parse_calendar(icalcomponent* c, schedule& s, bg::date bmonth,
			bg::date emonth);
public:
	ical_parser(const std::string& file);

	const std::string& file() const;

	~ical_parser();
	void fill_events(schedule& s, bg::date start, bg::date end);
};
}

#endif /* ICALPARSER_HPP_ */

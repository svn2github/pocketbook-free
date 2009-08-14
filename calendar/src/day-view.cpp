/*
 * day-view.cpp
 *
 *  Created on: Jun 14, 2009
 *      Author: fax
 */

#include <boost/shared_ptr.hpp>
#include <sstream>
#include <algorithm>

using namespace std;

#include "day-view.hpp"

const int ENTRY_WIDTH = 50;

const int SMALL_CALENDAR_WIDTH = 154;
const int SMALL_CALENDAR_HEIGHT = 120;
const int SMALL_CALENDAR_MARGIN = 20;
const int MARGIN_EVENT = 0;
const int EVENT_SEPARATOR = 10;
const int TIME_FONT_SIZE = 20;
const int REC_SPACING = 5;

extern ibitmap icon_recurrent;

//TODO: UP/DOWN simply draws events starting from the event #k, just check how many did not enter
//TODO: !Last event is hidden by mini-calendar

#define dbg cout

namespace bg = boost::gregorian;
namespace bd = boost::date_time;

DayView::DayView(iconfig* cfg, ViewManager* vm) :
	View(cfg, vm), day_f(s.open_font("LiberationSerif", 80, true)), dow_f(
			s.open_font("LiberationSerif", 40, true)), time_f(s.open_font(
			"LiberationSerif-Bold", TIME_FONT_SIZE, true)), summary_f(
			s.open_font("LiberationSerif", 14, true)), summary_bf(s.open_font(
			"LiberationSerif-Bold", 14, true)), starting_index(0), selected(0) {
}

void DayView::draw_day_header() {
	s.set_font(day_f, WHITE);
	ostringstream ss;
	ss << setw(2) << setfill('0') << cur_day.day();
	int w = s.string_width(ss.str());
	int h = s.rect_height(w, ss.str(), ALIGN_CENTER | VALIGN_MIDDLE);

	Rect r(2, 2, w + 5, h + 5);
	s.draw_rect(r, DGRAY);
	s.draw_rect(r.inflated_by(-1, -1), DGRAY);
	s.draw_text_rect(r, ss.str(), ALIGN_CENTER | VALIGN_MIDDLE);

	s.set_font(dow_f, WHITE);
	ss.str("");
	ss << cur_day.day_of_week();
	s.draw_text_rect(r.shifted_by(r.width(), 0), TR(ss.str()), ALIGN_CENTER
			| VALIGN_TOP | ROTATE);
}

void DayView::draw_header() {

	ostringstream ss;
	ss << "'" << setw(2) << setfill('0') << (cur_day.year() % 100);
	View::draw_header(mgr->cur_month().name(), ss.str());
	draw_day_header();
}

void DayView::draw_small_month() {

	unsigned x = s.width() - SMALL_CALENDAR_WIDTH - SMALL_CALENDAR_MARGIN, y =
			s.height() - SMALL_CALENDAR_HEIGHT - SMALL_CALENDAR_MARGIN;

	// DONE: Also do a frame
	Rect cal(x, y, SMALL_CALENDAR_WIDTH, SMALL_CALENDAR_HEIGHT);
	std::set<bg::date> sel;
	sel.insert(cur_day);
	View::draw_small_month(cal, sel);
}

short DayView::add_info_line(short y, const std::string& label,
		const std::string& val) {
	// No need to show empty lines
	if (val == "")
		return 0;

	string l = string(TR(label)) + " ";

	s.set_font(summary_bf, BLACK);
	short w = s.string_width(l);
	short h = s.rect_height(w, l, ALIGN_LEFT | VALIGN_TOP);

	Rect r(MARGIN_SIDE, y, w, h);
	s.draw_text_rect(r, l, ALIGN_LEFT | VALIGN_TOP);

	// TODO: Use cut_ functions
	r.x += w;
	r.w = s.width() - r.left() - MARGIN_SIDE;

	s.set_font(summary_f, BLACK);
	s.draw_text_rect(r, val, ALIGN_LEFT | VALIGN_TOP | DOTS);
	return h;
}

short DayView::draw_summary(short y, const ical::event& evt) {
	s.set_font(time_f, BLACK);
	ostringstream ss;

	if (evt.is_full_day()) {
		ss << TR(FULL_DAY);
	} else {
		bp::time_duration st = evt.start.time_of_day(), et = st + evt.duration;
		ss << st.hours() << ":" << setw(2) << setfill('0') << st.minutes()
				<< "-" << et.hours() << ":" << setw(2) << setfill('0')
				<< et.minutes();
	}
	short w = s.width() - 2 * MARGIN_SIDE;
	short h = s.rect_height(w, ss.str(), ALIGN_RIGHT);

	Rect r(MARGIN_SIDE, y, w, h);
	s.draw_text_rect(r, ss.str(), ALIGN_RIGHT | VALIGN_TOP);
	short dw = s.string_width(ss.str());
	r.cut_right(dw);

	if (evt.recurrent) {
		s.draw_bitmap(r.right() - icon_recurrent.width - REC_SPACING, r.top()
				+ (TIME_FONT_SIZE - icon_recurrent.height) / 2 + 3,
				&icon_recurrent);
		r.cut_right(icon_recurrent.width + 2 * REC_SPACING);
	}

	s.draw_text_rect(r, evt.summary, ALIGN_LEFT | VALIGN_TOP | DOTS);
	return h;
}

short DayView::draw_details(short y, const ical::event& evt) {
	short h = 0;

	if (config().view.day.show_location)
		h += add_info_line(y + h, LOCATION, evt.location);

	if (config().view.day.show_description)
		h += add_info_line(y + h, DESCRIPTION, evt.description);

	if (config().view.day.show_participants) {
		string participants;

		// TODO: Add required/optional hilighting
		for (std::list<ical::participant>::const_iterator it =
				evt.participants.begin(); it != evt.participants.end(); ++it, participants
				+= " ")
			participants += it -> name;

		h += add_info_line(y + h, PARTICIPANTS, participants);
	}

	return h;
}

void DayView::draw() {
	s.clear();
	draw_header();

	event_rects.clear();

	unsigned y = HEADER_WIDTH + MARGIN_TOP;

	ev_cnt::iterator it = events.begin();

	// Skip another starting_index events
	for (int i = 0; i < starting_index && it != events.end(); ++i, ++it)
		;

	//DONE: Output full-day events first
	for (; it != events.end() && y < s.height() - MARGIN_BOTTOM; ++it) {

#ifdef TRACE_DAYVIEW_EVENT_DRAW
		dbg << "Processing event " << endl << *it << endl;
#endif

		short yst = y;
		// DONE: Split into functions
		y += draw_summary(y, *it);
		y += draw_details(y, *it);

		s.draw_hline(y + EVENT_SEPARATOR / 2, MARGIN_SIDE + MARGIN_EVENT,
				s.width() - MARGIN_SIDE, LGRAY);
		event_rects.push_back(Rect(MARGIN_SIDE, yst, s.width() - 2
				* MARGIN_SIDE, y - yst));

		y += EVENT_SEPARATOR;

		// We needn't select separation lines
	}

	all_fit = event_rects.size() + starting_index == events.size();
	// TODO: If start_index > 0 draw ^ on the top
	// TODO: If not all fit the screen, draw v on the bottom, the same size as in View::vselection

	s.set_font(selection_f, DGRAY);
	if (!all_fit) {
		short w = s.char_width(ARROW_DOWN);
		s.draw_symbol((s.width() - w) / 2, s.height() - MARGIN_BOTTOM,
				ARROW_DOWN);
	}

	if (starting_index) {
		short w = s.char_width(ARROW_UP);
		s.draw_symbol((s.width() - w) / 2, HEADER_WIDTH, ARROW_UP);
	}

	if (!event_rects.empty())
		draw_vselection(event_rects[selected]);

	draw_small_month();

	s.full_update();
}

bool DayView::advance(int days) {
	bg::date d = cur_day + bg::days(days);
	mgr -> set_date(d);
	starting_index = 0;
	selected = 0;

	return true;
}

void DayView::set_date(bg::date d) {
	View::set_date(d);

	const ical::schedule & sch = mgr->schedule();

	events.clear();

	ical::schedule::container::iterator it = sch.events.begin();

	// Skip not relevant events
	while (it != sch.events.end() && it->start.date() < cur_day)
		++it;

	for (; it != sch.events.end() && it->start.date() == cur_day; ++it) {
#ifdef TRACE_DAYVIEW_EVENT_FILL
		dbg << "DayView: Filling events" << endl << *it << endl;
#endif
		events.push_back(*it);
	}
}

bool DayView::advance_minor(short delta) {
	short oldst = starting_index;
	short olds = selected;

	int item = selected + starting_index + delta;

	//TODO: FIXME
	while (true) {
		if (item < 0) {
			item = selected = starting_index = 0;
			break;
		}

		if (item >= int(events.size())) {
			item = events.size() - 1;
			starting_index = events.size() - event_rects.size();
			selected = event_rects.size() - 1;
			break;
		}

		if (item >= int(starting_index + event_rects.size())) {
			starting_index = item - event_rects.size() + 1;
			selected = event_rects.size() - 1;
			break;
		}

		if (item < starting_index) {
			starting_index = item;
			selected = 0;
			break;
		}

		selected = item - starting_index;
		if (oldst == starting_index) {
			dbg << "selected=" << selected << ", starting_index="
					<< starting_index << endl;
			draw_vselection(event_rects[selected], false, true);
			draw_vselection(event_rects[olds], true, true);
		}
		return false;
	}

	dbg << "selected=" << selected << ", starting_index=" << starting_index
			<< endl;

	if (oldst != starting_index)
		draw();

	/*	selected = min(selected + delta, static_cast<int> (events.size()
	 - starting_index - 1));

	 if (selected < 0) {
	 starting_index = starting_index + selected;
	 selected = 0;

	 if (starting_index < 0)
	 starting_index = 0;
	 } else if (selected > event_rects.size()) {
	 // Scroll down

	 starting_index +=  event_rects.size()
	 } else {
	 draw_vselection(event_rects[selected], false, true);
	 draw_vselection(event_rects[olds], true, true);
	 full_redraw = false;
	 }

	 if (selected == olds)
	 return false;

	 if (full_redraw)
	 draw();
	 */

	return false;
}


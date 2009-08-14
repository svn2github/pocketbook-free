/*
 * week-view.cpp
 *
 *  Created on: Jun 18, 2009
 *      Author: fax
 */

#include <sstream>
using namespace std;

#include "week-view.hpp"

// DONE: !Add +x events... to week-view
// DONE: !Parse 3 months at a time, because week can cross month boundary
// DONE: !Redesign week view
// TODO: When crossing month boundary, same week should be stopped twice (HOW?)




const int MONTH_HEADER = 30;
const int MARGIN_DAY = 3;

WeekView::WeekView(iconfig* cfg, ViewManager* mgr) :
	View(cfg, mgr), wheader_f(s.open_font("DroidSans", 18, 1)), date_f(
			s.open_font("LiberationSerif", 70, 1)), year_digit_f(s.open_font(
			"LiberationSerif", 120, 1)) {
	recalc_layout();
}

void WeekView::recalc_layout() {
	unsigned yt = HEADER_WIDTH + MARGIN_TOP - MONTH_HEADER;
	HSTEP = (s.width() - 2 * MARGIN_SIDE) / 3;
	VSTEP = (s.height() - yt - MARGIN_BOTTOM) / 3;
}

WeekView::~WeekView() {
}

Rect WeekView::day_rect(short dow) {
	unsigned yt = HEADER_WIDTH + MARGIN_TOP - MONTH_HEADER;

	short pos = (dow + 7 - week_start()) % 7;
	short line = pos / 3;
	unsigned x = MARGIN_SIDE + (pos % 3) * HSTEP, y = yt + VSTEP * line;
	return Rect(x, y + MONTH_HEADER, HSTEP + 1, VSTEP + 1);
}

bg::date WeekView::cur_week_start() {
	return cur_day - bg::days(cur_day.day_of_week() - week_start());
}

void WeekView::draw_header() {
	ostringstream ss;
	ss << TR(WW) << week_number(cur_day);
	string sl = ss.str();
	ss.str("");
	ss << "'" << setfill('0') << setw(2) << (cur_day.year() % 100);
	View::draw_header(sl, ss.str());

	s.set_font(wheader_f, WHITE);

	using namespace boost::gregorian;

	date wstart = cur_week_start();
	date wend = wstart + weeks(1);
	day_iterator ditr = wstart;

	for (unsigned cnt = 0; ditr != wend; ++ditr, ++cnt) {
		Rect r = day_rect(ditr->day_of_week());
		r.h = MONTH_HEADER;
		//		r.shift_by(0, -WDAY_HEADER);

		s.fill_rect(r, BLACK);
		s.draw_rect(r, LGRAY);

		ostringstream tr;

		ostringstream os;
		tr << ditr->day_of_week();
		string dow = tr.str();
		tr.str("");
		tr << ditr -> month();
		string mon = tr.str();
		os << TR(dow) << ", " << TR(mon) << " " << ditr -> day();
		s.draw_text_rect(r, os.str(), ALIGN_CENTER | VALIGN_MIDDLE);
	}
}

void WeekView::draw_day_lines() {
	for (int i = 0; i < 8; ++i)
		s.draw_rect(day_rect(i), LGRAY);
}

void WeekView::draw_times() {

}

void WeekView::draw_day_events(Rect r, bg::date d, ev_cnt::iterator& it) {
	// Skip non-relevant events;
	while (it != events.end() && it -> start.date() < d) {
		dbg << "BUG: Skipping irrelevant event: " << it -> summary << endl;
		++it;
	}

	r.cut_left(MARGIN_DAY);
	r.cut_right(MARGIN_DAY);

	short y = r.top();

	s.set_font(get_font("week.event"), BLACK);
	short h = s.rect_height(s.width(), "TEST", ALIGN_LEFT | VALIGN_TOP);
	dbg << "h=" << h << ", rect=" << r << endl;
	if (it != events.end())
		dbg << "Starting from " << it -> start.date() << endl;
	else
		dbg << "No events left";

	for (; it != events.end() && it->start.date() == d && y < r.bottom() - 2
			* h; ++it) {
		ostringstream ss;

		bp::time_duration st = it-> start.time_of_day(), et = st + it->duration;
		ss << st.hours() << ":" << setw(2) << setfill('0') << st.minutes()
				<< "-" << et.hours() << ":" << setw(2) << setfill('0')
				<< et.minutes();

		Rect re(r.x, y, r.w, h);

		if (config().view.week.show_time) {
			short dw = s.string_width(ss.str());
			// TODO Make uniform (all times, or no times at all)
			if (dw < r.w / 2) {
				s.draw_text_rect(re, ss.str(), ALIGN_RIGHT | VALIGN_TOP);
				re.cut_right(dw);
			} else
#ifdef TRACE_WEEKVIEW_EVENT_DETAILS
				dbg << "no space to draw time!" << endl
#endif

				;
		}

		ss.str("");
		ss << it->summary;
		s.draw_text_rect(re, ss.str(), ALIGN_LEFT | VALIGN_TOP | DOTS);
		y += h;

#ifdef TRACE_WEEKVIEW_EVENT_DETAILS
		dbg << "Drawn event " << it->summary << endl;
#endif
	}

	int skipped = 0;

	// Count skipped events
	for (; it != events.end() && it->start.date() == d; ++skipped, ++it)
		;

	if (skipped) {
		ostringstream ss;
		ss << "+ ";
		if (skipped >= 5)
			ss << skipped << TR(evt[4]);
		else
			ss << TR(evt[skipped - 1]);

		Rect re(r.x, y, r.w, h);
		s.draw_text_rect(re, ss.str(), ALIGN_LEFT | VALIGN_TOP | DOTS);
	}

	if (it != events.end())
		dbg << "Finished at" << it -> start.date() << endl;
	else
		dbg << "Finished all events" << endl;
	;

}

void WeekView::draw_events() {
	using namespace bg;

	date wstart = cur_week_start();
	date wend = wstart + weeks(1);
	day_iterator ditr = wstart;

	ev_cnt::iterator it = events.begin();

	for (; ditr != wend; ++ditr) {
		dbg << *ditr << endl;
		Rect dr = day_rect(ditr->day_of_week());
		dr.cut_top(MONTH_HEADER);
		draw_day_events(dr, *ditr, it);
	}

}

void WeekView::draw() {
	// TODO: Check if clear forces screen redraw; if it is, use fill_area instead();
	s.clear();
	draw_header();
	draw_day_lines();
	draw_times();
	draw_events();
	draw_small_month();

	draw_day_selection(false);

	s.full_update();
}

void WeekView::draw_small_month() {
	unsigned yt = HEADER_WIDTH + MARGIN_TOP;
	unsigned x = MARGIN_SIDE + 2 * HSTEP, y = yt + VSTEP * 2;
	Rect r(x, y, HSTEP + 1, VSTEP + 1);

	bg::date wstart = cur_day - bg::days(cur_day.day_of_week() - week_start());
	bg::date wend = wstart + bg::weeks(1);
	bg::day_iterator ditr = wstart;
	std::set<bg::date> selected;
	// TODO: std::for_each, back_insert_iterator, or so..
	for (; ditr != wend; ++ditr)
		selected.insert(*ditr);
	View::draw_small_month(r, selected);

}

void WeekView::draw_day_selection(bool update) {
	/*
	 s.set_font(date_f, erase ? WHITE : DGRAY);
	 short w = s.char_width(ARROW_DOWN);

	 unsigned x = MARGIN_SIDE - w / 2 + (selected_day + 0.5) * HSTEP, y =
	 HEADER_WIDTH + MARGIN_TOP - WDAY_HEADER;
	 dbg << "Returned" << s.draw_symbol(x, y, ARROW_DOWN) << endl;*/

	Rect r = day_rect(cur_day.day_of_week());
	//	r.shift_by(0, -WDAY_HEADER);

	s.invert_area(r);
	if (update) {
		s.partial_update(r);

		dbg << "Updating area " << r << endl;
	}
	/*	if (update)
	 s.partial_update(Rect(x, y, x + w, y + s.char_width(ARROW_RIGHT)));
	 */
	dbg << "Selected day #" << selected_day << endl;
}

bool WeekView::advance(int weeks) {
	bg::date d = cur_day + bg::weeks(weeks);
	mgr -> set_date(d);
	return true;
}

bool WeekView::advance_minor(short days) {
	bg::date d = cur_day + bg::days(days);

	bool full_redraw = false;
	if (week_number(d) != week_number(cur_day))
		full_redraw = true;

	if (!full_redraw)
		// Erase
		draw_day_selection(true);

	mgr->set_date(d);

	if (!full_redraw)
		// Draw
		draw_day_selection(true);

	return full_redraw;
}

void WeekView::set_date(bg::date d) {
	View::set_date(d);

	// TODO: Optimize. BTW, do we need to re-fill on each advancement?
	dbg << "Re-reading events" << endl;

	const ical::schedule & sch = mgr->schedule();

	events.clear();

	ical::schedule::container::iterator it = sch.events.begin();

	// Skip not relevant events
		while (it != sch.events.end() && it->start.date() < cur_week_start())
		++it;

#ifdef TRACE_WEEKVIEW_EVENT_FILL
	dbg << "Skipped irrelevant events" << endl;
#endif

	for (; it != sch.events.end() && it->start.date() < cur_week_start()
			+ bg::weeks(1); ++it) {
		events.push_back(*it);
#ifdef TRACE_WEEKVIEW_EVENT_FILL
		dbg << "Filling: " << endl << it -> summary << endl;
#endif
	}
#ifdef TRACE_WEEKVIEW_EVENT_FILL
	dbg << "Filled events" << endl;
#endif
}

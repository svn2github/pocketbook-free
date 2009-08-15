/*
 * month-view.cpp
 *
 *  Created on: Jun 14, 2009
 *      Author: fax
 */

#include <sstream>

using namespace std;

#include "month-view.hpp"

#define dbg cout

namespace bg = boost::gregorian;
namespace bd = boost::date_time;

const int WDAY_HEADER = 30;
const int WW_MARGIN = 40;

// TODO: show work-weeek, add this to config files
// REJECTED: Use always 6 weeks for uniform display
// DONE: Fix wrong WW representation
// TODO: Add status line showing controls < prev month > next month ^ week up v week down
// DONE: Draw month beginning with week start (WTF?)
// DONE: !In December, etc: month cannot have less than 4 weeks
// DONE: !Add recalculation on layout change


MonthView::MonthView(iconfig* cfg, ViewManager* vm) :
	View(cfg, vm), month_f(s.open_font("LiberationSerif", 45, 1)), month_bf(
			s.open_font("LiberationSerif-Bold", 45, 1)), date_f(s.open_font(
			"LiberationSerif", 70, 1)), year_digit_f(s.open_font(
			"LiberationSerif", 120, 1)), time_f(s.open_font("LiberationSerif",
			40, 1)), event_f(s.open_font("DroidSans", 14, 1)), mheader_f(
			s.open_font("DroidSans", 18, 1)), selected_week(0) {

}

Config::ViewType MonthView::view_id() {
	return Config::MonthView;
}


Rect MonthView::week_row(int week) const {
	int x = MARGIN_SIDE, y = HEADER_WIDTH + MARGIN_TOP + week * VSTEP;
	return Rect(x, y, s.width() - 2 * MARGIN_SIDE, VSTEP);
}

void MonthView::draw_header() {
	//	Bitmap* icon;

	// TODO: Move to View::, because it has become uniform
	ostringstream ss;
	ss << "'" << setfill('0') << setw(2) << (mgr->cur_month().year() % 100);
	string s1 = ss.str();

	View::draw_header(mgr->cur_month().name(), s1);

	s.set_font(mheader_f, WHITE);

	using namespace boost::gregorian;

	for (short dow = week_start(), c = 0; c < 7; ++c, dow = (dow + 1) % 7) {
		date::day_of_week_type dowt = dow;
		unsigned x1 = MARGIN_SIDE + c * HSTEP, y1 = HEADER_WIDTH + MARGIN_TOP
				- WDAY_HEADER;
		Rect r(Rect(x1, y1, HSTEP + 1, WDAY_HEADER));
		s.fill_rect(r, BLACK);
		s.draw_rect(r, LGRAY);

		ostringstream os;
		os << dowt;
		s.draw_text_rect(r, TR(os.str().c_str()), ALIGN_CENTER | VALIGN_MIDDLE);
	}
}

void MonthView::draw_week_selection(bool erase, bool update) {
	Rect r = week_row(selected_week);
	// TODO: If using old selection, add if here
#if 0
	r.w += r.x;
	r.x = 0;
#endif
	draw_vselection(r, erase, update);
	dbg << "Selected week #" << selected_week << endl;
}

void MonthView::draw_wws() {
	s.set_font(mheader_f, LGRAY);
	for (int w = 0; w < mgr->cur_month().weeks(); ++w) {
		unsigned x = MARGIN_SIDE - WW_MARGIN, y = HEADER_WIDTH + MARGIN_TOP + w
				* VSTEP;
		ostringstream ss;
		ss << TR(WW) << setfill('0') << setw(2)
				<< (mgr->cur_month().begin().week_number() + w);

		s.draw_text_rect(Rect(x, y, WW_MARGIN, VSTEP), ss.str(), ALIGN_CENTER
				| VALIGN_BOTTOM | ROTATE);
	}
}

void MonthView::draw() {
	s.clear();

	draw_header();

	using namespace boost::gregorian;

	day_iterator ditr = mgr->cur_month().begin();

	unsigned line = 0;

	dbg << "Drawing month '" << mgr->cur_month().name() << "' with "
			<< mgr->cur_month().weeks() << " weeks, begins on WW"
			<< mgr->cur_month().starting_week() << " and ends on WW"
			<< mgr->cur_month().ending_week() << endl;

	if (config().view.month.show_ww)
		draw_wws();

	date today = day_clock::local_day();
	const ical::schedule& sch = mgr->schedule();
	ical::schedule::container::iterator it = sch.events.begin();
	while (it != sch.events.end() && it->start.date() < mgr->cur_month().begin())
		++it;

	while (ditr -> day_of_week() != week_start())
		--ditr;

	day_iterator dend = mgr->cur_month().end();
	while (dend -> day_of_week() != week_end())
		++dend;

	++dend;

	for (; ditr != *dend; ++ditr) {
		int dw = day_column(ditr -> day_of_week());
		int day = ditr -> day();

		unsigned x1 = MARGIN_SIDE + dw * HSTEP, y1 = HEADER_WIDTH + MARGIN_TOP
				+ line * VSTEP;
		Rect rt(x1, y1, HSTEP + 1, VSTEP + 1);
		s.draw_rect(rt, LGRAY);
		if (ditr == today) {
			s.draw_rect(rt, BLACK);
			s.draw_rect(rt.inflated_by(-1, -1), BLACK);
		}

		char buf[3];
		sprintf(buf, "%2d", day);
		Rect r(x1, y1, HSTEP, VSTEP);

		if (ditr ->month() != mgr->cur_month().begin().month()) {
			s.set_font(month_f, LGRAY);
			s.draw_text_rect(r, buf, ALIGN_RIGHT | VALIGN_TOP);
			continue;
		}

		// Avoid thick lines

		int events = 0;

		for (; it != sch.events.end() && it->start.date() == *ditr; ++it, ++events)
			//dbg << "Found event " << *it << endl
			;

		s.set_font(events > 0 ? month_bf : month_f, BLACK);
		s.draw_text_rect(r, buf, ALIGN_RIGHT | VALIGN_TOP);
		if (events) {
			ostringstream ss;
			if (events >= 5)
				ss << events << TR(evt[4]);
			else
				ss << TR(evt[events - 1]);

			s.set_font(event_f, BLACK);
			s.draw_text_rect(Rect(x1 + 3, y1, HSTEP, VSTEP), ss.str(),
					ALIGN_LEFT | VALIGN_BOTTOM);
		}

		if (dw == 6)
			++line;

	}

	draw_week_selection(false, false);

	//msg("Ready");
	s.full_update();
}

bool MonthView::advance(int months) {
	bg::date d = mgr->cur_month().begin() + bg::months(months);
	mgr -> set_date(d);
	return true;
}

bool MonthView::advance_minor(short delta) {
	bg::date d = cur_day + bg::weeks(delta);
	bool redraw = cur_day.month() != d.month() || cur_day.year() != d.year();
	draw_week_selection(true, !redraw);
	mgr->set_date(d);
	draw_week_selection(false, !redraw);

	return redraw;
}

void MonthView::set_date(boost::gregorian::date d) {
	cur_day = d;
	selected_week = week_number(d) - mgr->cur_month().starting_week();
	if (selected_week < 0 || selected_week > mgr->cur_month().weeks()) {
		// TODO: Think of year boundary
		dbg << ">>> Remember to fix the bug!" << endl;
		selected_week = 0;
	}

	dbg << "selected_week=" << selected_week << endl;

	recalc_layout();
}

void MonthView::recalc_layout() {
	HSTEP = (s.width() - 2 * MARGIN_SIDE) / 7;
	VSTEP = (s.height() - HEADER_WIDTH - MARGIN_TOP - MARGIN_BOTTOM)
			/ mgr->cur_month().weeks();

}


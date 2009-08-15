/*
 * week-view.cpp
 *
 *  Created on: Jun 18, 2009
 *      Author: fax
 */

#include <sstream>
using namespace std;

#include "year-view.hpp"

// DONE: !Add +x events... to week-view
// DONE: !Parse 3 months at a time, because week can cross month boundary
// DONE: !Redesign week view

//TODO:Unify, place to one place
const int MONTH_HEADER = 30;
const int MARGIN_DAY = 3;

YearView::YearView(iconfig* cfg, ViewManager* mgr) :
	View(cfg, mgr), wheader_f(s.open_font("DroidSans", 18, 1)), date_f(
			s.open_font("LiberationSerif", 70, 1)), year_digit_f(s.open_font(
			"LiberationSerif", 120, 1)) {
	recalc_layout();
}

Config::ViewType YearView::view_id() {
	return Config::YearView;
}


void YearView::recalc_layout() {
	cols = s.width() > s.height() ? 4 : 3;
	rows = 12 / cols;

	unsigned yt = HEADER_WIDTH + MARGIN_TOP - MONTH_HEADER;
	HSTEP = (s.width() - 2 * MARGIN_SIDE) / cols;
	VSTEP = (s.height() - yt - MARGIN_BOTTOM) / rows;
}

YearView::~YearView() {
}

Rect YearView::month_rect(short m) {
	unsigned yt = HEADER_WIDTH + MARGIN_TOP - MONTH_HEADER;

	short line = m / cols;
	unsigned x = MARGIN_SIDE + (m % cols) * HSTEP, y = yt + VSTEP * line;
	return Rect(x, y + MONTH_HEADER, HSTEP + 1, VSTEP + 1);
}

void YearView::draw_header() {
	string sl = TR(YEAR);

	ostringstream ss;
	ss << "'" << setfill('0') << setw(2) << (cur_day.year() % 100);
	View::draw_header(sl, ss.str());

	s.set_font(wheader_f, WHITE);
	for (int i = 1; i <= 12; ++i) {
		Rect r = month_rect(i - 1);
		r.h = MONTH_HEADER;
		s.fill_rect(r, BLACK);
		s.draw_rect(r, LGRAY);

		s.draw_text_rect(r, TR(month_names[i]), ALIGN_CENTER | VALIGN_MIDDLE);
	}
}

void YearView::draw_month_lines() {
	for (int i = 0; i < 12; ++i) {
		s.draw_rect(month_rect(i), LGRAY);
	}
}

void YearView::draw() {
	s.clear();
	draw_header();
	draw_month_lines();

	for (int i = 0; i < 12; ++i) {
		Rect r = month_rect(i);
		r.cut_top(MONTH_HEADER);
		draw_small_month(r, month(cur_day.year(), i + 1));
	}

	draw_month_selection(false);

	s.full_update();
}

void YearView::draw_small_month(const Rect& r, month m) {
	std::set<bg::date> selected;
	View::draw_small_month(r, m, selected, false);
}

void YearView::draw_month_selection(bool update) {
	Rect r = month_rect(cur_day.month()-1);

	s.invert_area(r);
	if (update)
		s.partial_update(r);
}

bool YearView::advance(int years) {
	bg::date d = cur_day + bg::years(years);
	mgr -> set_date(d);
	return true;
}

bool YearView::advance_minor(short months) {
	bg::date d = cur_day + bg::months(months);

	bool full_redraw = false;
	if (d.year() != cur_day.year())
		full_redraw = true;

	if (!full_redraw)
		// Erase
		draw_month_selection(true);

	mgr->set_date(d);

	if (!full_redraw)
		// Draw
		draw_month_selection(true);

	return full_redraw;
}

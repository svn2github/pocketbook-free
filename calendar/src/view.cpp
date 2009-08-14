/*
 * view.cpp
 *
 *  Created on: Jun 14, 2009
 *      Author: fax
 */

#include <iostream>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "config.hpp"
using namespace std;

namespace bg = boost::gregorian;
namespace bd = boost::date_time;

#include "view.hpp"


View::View(iconfig* cfg, ViewManager* vm) :
	s(Screen::get_default()), mgr(vm), cur_day(bg::day_clock::local_day()),
			small_cal_f(s.open_font("LiberationSerif", 14, 1)),
			small_cal_head_f(s.open_font("LiberationSerif-Bold", 14, 1)),
			selection_f(s.open_font("LiberationSerif", 50, 1)), month_f(
					s.open_font("LiberationSerif", 75, true)), year_digit_f(
					s.open_font("LiberationSerif", 100, true)) {
	//	parse_config();

}

View::~View() {
}

void View::check_update(bool full_update) {
	if (full_update) {
		dbg << "Forcing full redraw" << endl;
		draw();
	} else
		dbg << "Redraw request ignored" << endl;
}

void View::draw_header(const std::string& sl, const std::string& sr) {
	s.fill_rect(Rect(0, 0, s.width(), HEADER_WIDTH), BLACK);

	s.set_font(year_digit_f, WHITE);
	short w = s.string_width(sr) + 5;
	s.draw_shaded_rect(Rect(s.width() - w, 0, w, HEADER_WIDTH), sr, ALIGN_LEFT
			| VALIGN_BOTTOM, BLACK, WHITE, year_digit_f);

	s.set_font(month_f, WHITE);
	s.draw_text_rect(Rect(0, 0, s.width() - w, HEADER_WIDTH), sl, ALIGN_RIGHT
			| VALIGN_BOTTOM);

}

ViewAction View::handle_event(int type, int par1, int par2) {
	switch (par1) {

	case KEY_OK:
		return ZOOM_IN;

	case KEY_LEFT:
		check_update(advance(par2 == 0 ? -1 : -4));
		return PROCESSED;
	case KEY_RIGHT:
		check_update(advance(par2 == 0 ? 1 : 4));
		return PROCESSED;

	case KEY_UP: {
		check_update(advance_minor(-1));
		return PROCESSED;
	}
	case KEY_DOWN: {
		check_update(advance_minor(1));
		return PROCESSED;
	}

	case KEY_BACK:
		return ZOOM_OUT;

	case KEY_MUSIC:
	case KEY_MENU:
	default:
		return NOT_PROCESSED;
	}

	return NOT_PROCESSED;
}

void View::set_date(boost::gregorian::date d) {
	cur_day = d;
}

void View::change_orientation(int new_ort) {
	//DONE: Recalculate layout
	recalc_layout();
}

void View::recalc_layout() {

}

const Config& View::config() const {
	return mgr->config();
}

void View::draw() {

}

bd::weekdays View::week_start() {
	return mgr ->week_start();
}

short View::week_number(bg::date d) {
	return time_utils::week_number(d, week_start());
}

boost::date_time::weekdays View::week_end() {
	return static_cast<bd::weekdays> ((week_start() + 6) % 7);
}

boost::gregorian::date View::selected() {
	return cur_day;
}

short View::day_column(short d) {
	return (d - week_start() + 7) % 7;
}

void View::draw_small_month(const Rect& cal_area,
		const std::set<bg::date>& selection, bool frame) {
	draw_small_month(cal_area, mgr->cur_month(), selection, frame);
}

void View::draw_small_month(const Rect& cal_area, month m, const std::set<
		bg::date>& selection, bool frame) {
	using namespace boost::gregorian;

	Rect cal = cal_area;

	if (frame) {
		s.fill_rect(cal, BLACK);
		s.clear_rect(cal.inflated_by(-2, -2));
	}

	cal = cal.inflated_by(-10, -10);
	int HSTEP = cal.width() / 7;
	int VSTEP = cal.height() / (mgr->cur_month().weeks() + 1);

	s.set_font(small_cal_head_f, BLACK);
	for (short i = 0; i < 7; ++i) {
		unsigned x = cal.left() + i * HSTEP, y = cal.top();
		s .draw_text_rect(Rect(x, y, HSTEP, VSTEP), TR(days_ss[(i
				+ week_start()) % 7]), ALIGN_RIGHT | VALIGN_TOP);
	}

	day_iterator ditr = m.begin();
	short line = 1;
	for (; ditr <= m.end(); ++ditr) {
		int dw = day_column(ditr -> day_of_week());
		int day = ditr -> day();
		unsigned x1 = cal.left() + dw * HSTEP, y1 = cal.top() + line * VSTEP;

		//dbg << "Drawing day #" << day << ", x1=" << x1 << ", y1=" << y1 << endl;
		ostringstream os;
		os << setw(2) << day;

		s.set_font(selection.find(*ditr) != selection.end() ? small_cal_head_f
				: small_cal_f, BLACK);
		s.draw_text_rect(Rect(x1, y1, HSTEP, VSTEP), os.str(), ALIGN_RIGHT
				| VALIGN_TOP);

#if 0
		dbg << "day=" << day << ", dw=" << dw << ", week_start="
		<< week_start() << ", week_end=" << week_end() << endl;
#endif

		if (dw == 6)
			++line;
	}
}

void View::draw_vselection_old(const Rect& r, bool erase, bool update) {
	s.set_font(selection_f, erase ? WHITE : DGRAY);

	short w = s.char_width(ARROW_RIGHT);

	static char str[] = { ARROW_RIGHT, 0 };
	short h = s.rect_height(w, str, ALIGN_CENTER | VALIGN_MIDDLE);

	short y = r.y + r.height() / 2 - h;

	// Dirty hack
	// TODO: Make really dependent on rect
	s.draw_symbol(0, y, ARROW_RIGHT);
	s.draw_symbol(s.width() - w, y, ARROW_LEFT);

	if (update)
		s.partial_update(Rect(0, y, s.width(), h));
}

void View::draw_vselection(const Rect& r, bool erase, bool update) {
	s.invert_area(r);

	if (update)
		s.partial_update(r);
}

const Font& View::get_font(const std::string& name) const {
	return mgr ->get_font(name);
}

/*
 * day-view.cpp
 *
 *  Created on: Jun 14, 2009
 *      Author: fax
 */

#include <boost/shared_ptr.hpp>
#include <sstream>

using namespace std;

#include "day-graphic-view.hpp"

const int HEADER_WIDTH = 100;
const int ENTRY_WIDTH = 50;

const int SMALL_CALENDAR_WIDTH = 140;
const int SMALL_CALENDAR_HEIGHT = 120;
const int SMALL_CALENDAR_MARGIN = 20;

//TODO: UP/DOWN simply draws events starting from the event #k, just check how many did not enter

#define dbg cout

namespace bg = boost::gregorian;
namespace bd = boost::date_time;

DayGraphicView::DayGraphicView(iconfig* cfg, ViewManager* vm) :
	View(cfg, vm), year_digit_f(s.open_font("LiberationSerif", 100, 1)), day_f(
			s.open_font("LiberationSerif", 80, 1)), dow_f(s.open_font("LiberationSerif", 40, 1)),
			time_f(s.open_font("LiberationSerif", 40, 1)), month_f(s.open_font("LiberationSerif",
					75, 1)) {
}

void DayGraphicView::draw_header() {
	s.fill_rect(Rect(0, 0, s.width(), HEADER_WIDTH), BLACK);

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
	s.draw_text_rect(r.shifted_by(r.width(), 0), ss.str(), ALIGN_CENTER
			| VALIGN_TOP | ROTATE);

	s.set_font(month_f, WHITE);
	s.draw_text_rect(Rect(0, 0, s.width() - 150, HEADER_WIDTH),
			mgr->cur_month().name(), ALIGN_RIGHT | VALIGN_BOTTOM);

	s.set_font(time_f, WHITE);
	ss.str("");
	ss << "'" << setw(2) << setfill('0') << (cur_day.year() % 100);
	s.draw_shaded_rect(Rect(s.width() - 150, 0, 150, HEADER_WIDTH), ss.str(),
			ALIGN_RIGHT | VALIGN_BOTTOM, BLACK, WHITE, year_digit_f);

}

void DayGraphicView::draw_small_month() {

	unsigned x = s.width() - SMALL_CALENDAR_WIDTH - SMALL_CALENDAR_MARGIN, y =
			s.height() - SMALL_CALENDAR_HEIGHT - SMALL_CALENDAR_MARGIN;

	// DONE: Also do a frame
	Rect cal(x, y, SMALL_CALENDAR_WIDTH, SMALL_CALENDAR_HEIGHT);
	std::set<bg::date> sel;
	sel.insert(cur_day);
	View::draw_small_month(cal, sel);
}

void DayGraphicView::draw() {
	s.clear();
	draw_header();

	s.set_font(time_f, BLACK);
	for (int h = 8; h < 20; ++h) {
		char str[] = "12:00";

		sprintf(str, "%02d:00", h);
		int y = HEADER_WIDTH + (h - 7) * ENTRY_WIDTH;
		s.draw_string(10, y, str);
		s.draw_hline(y, 10, s.width() - 10, LGRAY);
	}

	draw_small_month();

	s.full_update();
}

bool DayGraphicView::advance(int days) {
	bg::date d = cur_day + bg::days(days);
	mgr -> set_date(d);
	return true;
}

bool DayGraphicView::advance_minor(short hours) {
	dbg << "Not implemented!" << endl;
	return true;
}

void DayGraphicView::set_date(bg::date d) {
	dbg << "Setting date " << d << endl;
	cur_day = d;
	dbg << "Done" << endl;

}

ViewAction DayGraphicView::handle_event(int type, int par1, int par2) {
	switch (par1) {

	case KEY_OK:
		break;
	case KEY_BACK:
		return ZOOM_OUT;
	case KEY_LEFT:
		advance(par2 == 0 ? -1 : -4);
		return PROCESSED;
	case KEY_RIGHT:
		advance(par2 == 0 ? 1 : 4);
		return PROCESSED;

	case KEY_UP:
	case KEY_DOWN:
	case KEY_MUSIC:
	case KEY_MENU:
	default:
		return NOT_PROCESSED;
	}

	return NOT_PROCESSED;
}

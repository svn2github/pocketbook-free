/*
 * week-view.hpp
 *
 *  Created on: Jun 18, 2009
 *      Author: fax
 */

#ifndef WEEKVIEW_HPP_
#define WEEKVIEW_HPP_

#include "view.hpp"

class WeekView:
		public View {
private:
	void draw_header();
	void draw_day_lines();
	void draw_times();
	void draw_events();
	void draw_day_selection(bool update);
	void draw_day_events(Rect r, bg::date d, ev_cnt::iterator& it);
	void draw_small_month();

	bool advance(int weeks);
	bool advance_minor(short delta);

	void recalc_layout();

	Rect day_rect(short dow);

	bg::date cur_week_start();

public:
	WeekView(iconfig* cfg, ViewManager* mgr);
	virtual ~WeekView();

	void set_date(boost::gregorian::date d);

	virtual void draw();

protected:

private:
	Font wheader_f, date_f, year_digit_f;
	unsigned HSTEP, VSTEP;
	int selected_day;

	ev_cnt events;
};

#endif /* WEEKVIEW_HPP_ */

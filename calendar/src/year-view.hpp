/*
 * week-view.hpp
 *
 *  Created on: Jun 18, 2009
 *      Author: fax
 */

#ifndef YearView_HPP_
#define YearView_HPP_

#include "view.hpp"

class YearView:
		public View {
private:
	void draw_header();
	void draw_month_lines();
	void draw_month_selection(bool update);
	void draw_small_month(const Rect& r, month m);

	bool advance(int weeks);
	bool advance_minor(short delta);

	void recalc_layout();

	Rect month_rect(short dow);

	bg::date cur_week_start();

public:
	YearView(iconfig* cfg, ViewManager* mgr);
	virtual ~YearView();

	virtual void draw();
	virtual Config::ViewType view_id();


protected:

private:
	Font wheader_f, date_f, year_digit_f;
	unsigned HSTEP, VSTEP;

	short rows, cols;
};

#endif /* YearView_HPP_ */

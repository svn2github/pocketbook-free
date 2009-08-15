/*
 * month-view.hpp
 *
 *  Created on: Jun 14, 2009
 *      Author: fax
 */

#ifndef MONTHVIEW_HPP_
#define MONTHVIEW_HPP_

#include "date-time.hpp"
#include "view.hpp"

class MonthView: public View {
private:

	void draw_header();
	void draw_wws();
	void draw_week_selection(bool erase, bool update = true);

	bool advance(int months);
	bool advance_minor(short delta);

	/**
	 * Calculates the coordinates of rectangular area
	 * where week #week should be drawn
	 */
	Rect week_row(int week) const;
public:
	void draw();
	void set_date(boost::gregorian::date d);

	void set_selection(int day);



	MonthView(iconfig* cfg, ViewManager* vm);
	virtual Config::ViewType view_id();


protected:
	void recalc_layout();

private:
	int HSTEP, VSTEP;

	Font month_f, month_bf, date_f, year_digit_f, time_f, event_f, mheader_f;

	short selected_week;

};

#endif /* MONTHVIEW_HPP_ */

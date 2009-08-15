/*
 * day-view.hpp
 *
 *  Created on: Jun 14, 2009
 *      Author: fax
 */

#ifndef DayGraphicView_HPP_
#define DayGraphicView_HPP_

#include "view.hpp"

class DayGraphicView: public View {
private:
	void draw_header();
	void draw_small_month();
public:

	DayGraphicView(iconfig* cfg, ViewManager* vm);
	void draw();

	ViewAction handle_event(int type, int par1, int par2);
	bool advance(int days);
	bool advance_minor( short hours);
	void set_date(boost::gregorian::date d);

private:
	bool initialized;
	Font year_digit_f, day_f, dow_f, time_f, month_f;
};

#endif /* DayGraphicView_HPP_ */

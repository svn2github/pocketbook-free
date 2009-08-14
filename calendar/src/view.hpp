/*
 * view.hpp
 *
 *  Created on: Jun 14, 2009
 *      Author: fax
 */

#ifndef VIEW_HPP_
#define VIEW_HPP_

#include <boost/date_time/gregorian/gregorian.hpp>

#include <set>

#include "config.hpp"
#include "screen.hpp"
#include "view-manager.hpp"
#include <inkview.h>

//DONE: Unify, place to one place
const int MARGIN_TOP = 30;
const int MARGIN_BOTTOM = 80;
const int MARGIN_SIDE = 50;


enum ViewAction {
	PROCESSED, NOT_PROCESSED, ZOOM_OUT, ZOOM_IN, CONFIGURE
};

#include "font.hpp"
#include "ical-parser.hpp"

class ViewManager;

class View {
private:
	void check_update(bool full_update);
public:

	typedef std::vector<ical::event> ev_cnt;

	View(iconfig * cfg, ViewManager* vm);
	virtual ~View();

	virtual void set_date(boost::gregorian::date d);

	/**
	 * Advances a view forward/backward, like month in a month view
	 * @return true if full redraw is required
	 */
	virtual bool advance(int units) = 0;

	/**
	 * Advances inside a view, like week forward/backwar in the month view, etc.
	 * @return true if full redraw is required
	 */
	virtual bool advance_minor(short units)=0;

	// TODO: Maybe, a good idea is to make screen a part of parameter list?
	virtual void draw();

	virtual void change_orientation(int new_ort = -1);

	const Config& config() const;

	boost::date_time::weekdays week_start();
	boost::date_time::weekdays week_end();
	short week_number(bg::date d);

	short day_column(short d);

	boost::gregorian::date selected();

	/**
	 * @return PROCESSED when current event was handled
	 * @see ViewAction
	 */
	virtual ViewAction handle_event(int type, int par1, int par2);
protected:
	virtual void draw_vselection(const Rect& r, bool erase = false,
			bool update = false);
	virtual void draw_vselection_old(const Rect& r, bool erase = false,
			bool update = false);

	virtual void recalc_layout();
	void draw_small_month(const Rect& cal_area, month m, const std::set<
			bg::date>& selection, bool frame = false);
	void draw_small_month(const Rect& r, const std::set<bg::date>& selected,
			bool frame = true);
	void draw_header(const std::string& s1, const std::string& s2);

	const Font& get_font(const std::string& name) const;

protected:
	static const int HEADER_WIDTH = 100;
protected:
	Screen& s;
	ViewManager* mgr;

	boost::gregorian::date cur_day;
	Font small_cal_f, small_cal_head_f, selection_f, month_f, year_digit_f;

};

#endif /* VIEW_HPP_ */

/*
 * view-manager.hpp
 *
 *  Created on: Jun 16, 2009
 *      Author: fax
 */

#ifndef VIEWMANAGER_HPP_
#define VIEWMANAGER_HPP_

#include <list>
#include <map>

#include <boost/date_time/gregorian/gregorian.hpp>

#include "config.hpp"
#include "date-time.hpp"
#include "view.hpp"
#include "ical-parser.hpp"

#include <inkview.h>

class View;

// TODO: Let current date be stored in the manager
// TODO: On date change, only current view will be notified

class ViewManager {
public:
	ViewManager(iconfig* cfg);
	virtual ~ViewManager();

	void zoom_out(boost::gregorian::date d);
	void zoom_in(boost::gregorian::date d);

	void zoom_out();
	void zoom_in();

	void addView(View* v);

	View* cur_view() const;
	void set_view(View* v, bool redraw = true);

	void set_date(boost::gregorian::date d);

	void update_configuration();

	void change_orientation(int ort);
	void rotate();

	const ical::schedule& schedule() const;

	// CONFIGURATION
	boost::date_time::weekdays week_start();

	// DATE FUNCTIONS
	month cur_month() const;

	const Font& get_font(const std::string& name);

	const Config& config() const;

private:
	Font create_font(const std::string & fname);
	void fill_events();

private:
	std::list<View*> views;
	std::list<View*>::iterator cview;

	typedef std::map<std::string, Font> FontCnt;
	FontCnt fonts;

	Config cconfig;

	month curm;
	ical::schedule month_schedule;

	Font def_font;

	// TODO: this is workaround
	boost::gregorian::date cur_day;
};

#endif /* VIEWMANAGER_HPP_ */

/*
 * day-view.hpp
 *
 *  Created on: Jun 14, 2009
 *      Author: fax
 */

#ifndef DAYVIEW_HPP_
#define DAYVIEW_HPP_

#include <vector>
#include "view.hpp"

class DayView:
		public View {
private:
	void draw_header();
	void draw_day_header();
	void draw_small_month();
	short draw_details(short y, const ical::event& evt);
	short draw_summary(short y, const ical::event& evt);
	short add_info_line(short y, const std::string& label,
			const std::string& val);
public:

	DayView(iconfig* cfg, ViewManager* vm);
	void draw();

	bool advance(int days);
	bool advance_minor(short events);

	void set_date(bg::date d);

	virtual Config::ViewType view_id();

private:
	Font day_f, dow_f, time_f, summary_f, summary_bf;

	//! Events are drawed from this index, i.e. first (starting-index) events are skipped when drawn;
	/// This is used when too many events are on the screen
	int starting_index;
	int selected;
	std::vector<Rect> event_rects;
	// true when all events fit the view
	bool all_fit;

	//! Regular && full-day events
	/// After sorting full day events before all was added
	/// no need in fd_events
	ev_cnt events;
};

#endif /* DAYVIEW_HPP_ */

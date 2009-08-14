/*
 * view-manager.cpp
 *
 *  Created on: Jun 16, 2009
 *      Author: fax
 */

#include <string>
using namespace std;

#define dbg cout

#include "view-manager.hpp"
namespace bg = boost::gregorian;
namespace bd = boost::date_time;

ViewManager::ViewManager(iconfig* cfg) :
	cview(views.begin()), cconfig(cfg), def_font(
			Screen::get_default().open_font("LiberationSerif", 10, 1)),
			cur_day(bg::day_clock::local_day()) {

	update_configuration();
	fill_events();
}

ViewManager::~ViewManager() {
}

const ical::schedule& ViewManager::schedule() const {
	return month_schedule;
}

void ViewManager::rotate() {
	dbg << "Setting orientation: " << cconfig.view.orientation << endl;
	change_orientation(cconfig.view.orientation);
}

void ViewManager::zoom_out(boost::gregorian::date d) {
	if (++cview == views.end()) {
		--cview;
		return;
	}

	(*cview) -> set_date(d);
	(*cview) -> draw();
}

void ViewManager::zoom_out() {
	bg::date d = (*cview) -> selected();
	zoom_out(d);
}

void ViewManager::zoom_in(boost::gregorian::date d) {
	if (cview == views.begin()) {
		return;
	}

	--cview;

	(*cview) -> set_date(d);
	(*cview) -> draw();
}
void ViewManager::zoom_in() {
	bg::date d = (*cview) -> selected();
	zoom_in(d);
}

void ViewManager::addView(View* v) {
	views.push_back(v);
	cview = views.begin();
}

View* ViewManager::cur_view() const {
	return *cview;
}

void ViewManager::set_date(boost::gregorian::date d) {
	cur_day = d;

	short oldm = curm.begin().month();
	short oldy = curm.begin().year();
	curm = month(d.year(), d.month());

	for (std::list<View*>::iterator it = views.begin(); it != views.end(); ++it)
		(*it)->set_date(d);

	if (oldm != d.month() || oldy != d.year())
		fill_events();
}

void ViewManager::set_view(View* v, bool redraw) {
	for (std::list<View*>::iterator it = views.begin(); it != views.end(); ++it)
		if (*it == v) {
			cview = it;
			if (redraw)
				cur_view() -> draw();
			return;
		}

	dbg << "Requested view not found!" << endl;
}

void ViewManager::fill_events() {
	// DONE: Re-parse calendar when month changes
	dbg << "Parsing calendar for month " << curm.name() << endl;
	month_schedule.events.clear();

	// A little bit overkill, but... ))
	cconfig.parser->fill_events(month_schedule, curm.begin() - bg::months(1),
			curm.end() + bg::months(1));

	dbg << "Found " << month_schedule.events.size() << " events" << endl;
}

bd::weekdays ViewManager::week_start() {
	return cconfig.calendar.week_start;

}

void ViewManager::update_configuration() {
	dbg << "Manager runs config" << endl;
	cconfig.update();

	fonts.clear();
	fonts.insert(make_pair("week.event", create_font(cconfig.fonts.week_event)));
	// Defensive, but no other way...
	fill_events();
	set_date(cur_day);
}

Font ViewManager::create_font(const std::string & fname) {

	dbg << "Parsing font " << fname << endl;
	int pos = fname.find(',');
	string name_part = fname.substr(0, pos);

	dbg << "Font name part is " << name_part << endl;
	string size_part = fname.substr(pos + 1);

	int size;
	istringstream ss(size_part);
	ss >> size;

	return Font(Screen::get_default().open_font(name_part, size, true));

}

month ViewManager::cur_month() const {
	return curm;
}

const Font& ViewManager::get_font(const std::string& name) {
	FontCnt::const_iterator it = fonts.find(name);

	bool no_font = fonts.end() == it;
	return no_font ? def_font : it->second;
}

void ViewManager::change_orientation(int ort) {
	if (ort == GetOrientation())
		return;

	SetOrientation(ort);
	dbg << "Hardware orientation set" << endl;
	for (std::list<View*>::iterator it = views.begin(); it != views.end(); ++it)
		(*it)->change_orientation(ort);

	cur_view()->draw();

}

const Config& ViewManager::config() const {
	return cconfig;
}

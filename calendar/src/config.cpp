/*
 * config.cpp
 *
 *  Created on: Jul 4, 2009
 *      Author: fax
 */

#include "config.hpp"
#include "ical-parser.hpp"

using namespace std;

Config::Config(iconfig* cfg) :
	config(cfg), parser(0) {
	//	update();
}

Config::~Config() {
	delete parser;
}

template<int k>
unsigned index_of(const std::string* lst, const std::string& str) {
	for (int i = 0; i < k; ++i) {
		if (str == lst[i])
			return i;
		dbg << str << "!=" << lst[i] << endl;
	}

	return 0;
}

void Config::update() {

	// TODO: Declare all *_vnt as extern and reference them from here
	dbg << "Starting updating configuration" << endl;

	string ws = read_string("view.weekstart", "Sunday");
	calendar.week_start = (ws == "Sunday") ? bd::Sunday : bd::Monday;

	view.language = read_string("view.language", "en");
	translations.switch_language(view.language);
	dbg << "Loading language " << US(view.language) << endl;

	profile.dir = read_string("profile.dir", "./");
	profile.p1_name = read_string("profile.file1", "basic.ics");
	profile.p2_name = read_string("profile.file2", "basic2.ics");
	profile.index = read_string("profile.cur", CFG_PROFILE_I) == CFG_PROFILE_I;

	string newname = profile.index ? profile.p1_name : profile.p2_name;
	if (!parser || parser->file() != newname) {
		delete parser;
		dbg << "Creating parser for file " << newname << endl;
		parser = new ical::ical_parser(newname);
	}

	view.week.show_time = read_boolean("weekview.showtime", true);
	view.month.show_ww = read_boolean("monthview.showww", true);

	view.day.show_location = read_boolean("dayview.show_location", true);
	view.day.show_description = read_boolean("dayview.show_description", true);
	view.day.show_participants
			= read_boolean("dayview.show_participants", true);

	fonts.week_event = read_string("weekview.event_font", "LiberationSerif,10");

	view.starting_view = ViewType(index_of<5> (CFG_VIEW_STARTVIEW_VNT,
			read_string("view.stview", CFG_VIEW_STARTVIEW_VNT[0])));

	view.last_view = ViewType(read_int("view.last_view", 1));

	int ort = index_of<5> (ORIENTATIONS, read_string("view.orientation",
			ORIENTATIONS[4]));
	view.orientation = Orientation(ort == 4 ? Auto : ort);

}

std::string Config::read_string(const std::string& param_name,
		const std::string& defval) {
	return ReadString(config, const_cast<char*> (param_name.c_str()),
			const_cast<char*> (defval.c_str()));

}

int Config::read_int(const std::string& param_name, int defval) {
	return ReadInt(config, const_cast<char*> (param_name.c_str()), defval);
}

bool Config::read_boolean(const std::string& param_name, bool defval) {
	string ans = read_string(param_name, defval ? "Yes" : "No");
	if (ans == "Yes")
		return true;

	return false;
}

/*
 * config.hpp
 *
 *  Created on: Jun 17, 2009
 *      Author: fax
 */

#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/gregorian_calendar.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

#include <string>

#include "translations.hpp"

namespace bd = boost::date_time;
namespace bg = boost::gregorian;
namespace bp = boost::posix_time;

namespace ical {
class ical_parser;
}

#define dbg cout

inline char* unsafe_string(const std::string& str) {
	return const_cast<char*> (str.c_str());
}

#define US(x) unsafe_string(x)

#include <inkview.h>

class Config {
public:
	Config(iconfig* cfg);
	virtual ~Config();

	void update();

	bool read_boolean(const std::string& param_name, bool defval);
	std::string read_string(const std::string& param_name,
			const std::string& defval);

	enum ViewType {
		LastView, MonthView, WeekView, TodayView, YearView
	};

	enum Orientation {
		Portrait, Landscape, ReverseLandscape, ReversePortrait, Auto = -1
	};

	struct {
		bd::weekdays week_start;
	} calendar;

	struct {
		std::string language;
		ViewType starting_view;
		Orientation orientation;
		struct {
			bool show_time;
		} week;

		struct {
			bool show_ww;
		} month;

		struct {
			bool show_description;
			bool show_location;
			bool show_participants;
		} day;
	} view;

	struct {
		std::string dir;
		std::string p1_name, p2_name;

		short index;
	} profile;

	struct {
		std::string week_event;
	} fonts;

	iconfig* config;
	Translations translations;
	ical::ical_parser *parser;
};

#endif /* CONFIG_HPP_ */

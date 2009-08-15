/*
 * date-time.hpp
 *
 *  Created on: Jun 14, 2009
 *      Author: fax
 */

#ifndef DATETIME_HPP_
#define DATETIME_HPP_

#include <string>

#include "config.hpp"

class time_utils {
public:

	static short week_number(bg::date d, boost::date_time::weekdays week_start) {
		using namespace boost::gregorian;
		return (d - days(week_start)).week_number();
	}

};

class month {

private:
	boost::gregorian::date bmonth;
	boost::gregorian::date emonth;

	short y_;
	short m_;

	struct fmt:
			public boost::date_time::simple_format<char> {
		static boost::date_time::month_format_spec month_format() {
			return boost::date_time::month_as_long_string;
		}
	};

public:

	short starting_week(boost::date_time::weekdays week_start =
			boost::date_time::Sunday) const {
		return time_utils::week_number(bmonth, week_start);
	}

	short ending_week(boost::date_time::weekdays week_start =
			boost::date_time::Sunday) const {
		return time_utils::week_number(emonth, week_start);
	}

	short weeks(boost::date_time::weekdays week_start =
			boost::date_time::Sunday) const {
		using namespace boost::gregorian;

		short w = ending_week() - starting_week() + 1;
		if (w < 0) {
			// Can happen in January
			date last_d = emonth;

			while( last_d.week_number() == 1)
				last_d = last_d - bg::days(1);

			return last_d.week_number() - starting_week() + 2;
		}

		return w;
	}

	month(int y, int m) :
		bmonth(y, m, 1), emonth(bmonth.end_of_month()), y_(y), m_(m) {
	}

	month() :
		bmonth(boost::gregorian::day_clock::local_day()
				- boost::gregorian::days(
						boost::gregorian::day_clock::local_day().day() - 1)),
				emonth(bmonth.end_of_month()), y_(bmonth.year()), m_(
						bmonth.month()) {
	}

	short year() const {
		return y_;
	}

	boost::gregorian::date begin() const {
		return bmonth;
	}

	boost::gregorian::date end() const {
		return emonth;
	}

	std::string name() const {
		using namespace boost::gregorian;
#if USE_BOOST_IMPLEMENTATION
		namespace bd = boost::date_time;
		date::month_type m(m_);
		bd::month_formatter<date::month_type, fmt> f;

		ostringstream ss;
		f.format_month(m, ss);
		return ss.str();
#else
		assert( m_ >=1 && m_ <= 12);
		return TR(month_names[m_]);
#endif
	}

};

#endif /* DATETIME_HPP_ */

/*
 * event.hpp
 *
 *  Created on: Jun 17, 2009
 *      Author: fax
 */

#ifndef EVENT_HPP_
#define EVENT_HPP_

#include "reminder.hpp"

#include <string>
#include <list>

struct Event {

	enum Type {
		Recurrent, Single, FullDay, Todo
	};

	std::string summary;
	std::string location;
	std::list<std::string> participants;

	std::list<Reminder> reminders;
public:
	Event();
	virtual ~Event();
};

#endif /* EVENT_HPP_ */

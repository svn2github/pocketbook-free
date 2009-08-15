/*
 * scheduler.hpp
 *
 *  Created on: Jun 17, 2009
 *      Author: fax
 */

#ifndef SCHEDULER_HPP_
#define SCHEDULER_HPP_

#include <queue>

#include "event.hpp"

class Scheduler {
public:
	Scheduler();
	virtual ~Scheduler();

private:
	std::queue< Event > events;
};

#endif /* SCHEDULER_HPP_ */

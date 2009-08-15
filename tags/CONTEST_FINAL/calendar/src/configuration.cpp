/*
 * configuration.cpp
 *
 *  Created on: Jun 27, 2009
 *      Author: fax
 */

#include "configuration.hpp"

Configuration* Configuration::instance = 0;

Configuration::Configuration(iconfig* cfg) {

}

Configuration* Configuration::get() {
	if (0 == instance)
		instance = new Configuration(0);

	return instance;
}


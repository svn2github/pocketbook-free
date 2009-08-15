/*
 * configuration.hpp
 *
 *  Created on: Jun 27, 2009
 *      Author: fax
 */

#ifndef CONFIGURATION_HPP_
#define CONFIGURATION_HPP_

#include <inkview.h>

class Configuration {
public:
	static Configuration* get();
private:
	Configuration(iconfig* config);
	static Configuration* instance;
};

#endif /* CONFIGURATION_HPP_ */

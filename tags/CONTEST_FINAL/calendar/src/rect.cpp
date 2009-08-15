/*
 * rect.cpp
 *
 *  Created on: Jul 3, 2009
 *      Author: fax
 */

#include "rect.hpp"

std::ostream& operator<<(std::ostream& o, const Rect& r) {
	o << "(" << r.left() << ", " << r.top() << ") - (" << r.right() << ", "
			<< r.bottom() << ")" << std::endl;
	return o;
}



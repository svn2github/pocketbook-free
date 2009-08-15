/*
 * Font.cpp
 *
 *  Created on: Jun 9, 2009
 *      Author: fax
 */

#include <boost/shared_ptr.hpp>
#include <iostream>

using namespace std;
#define dbg cout

#include <inkview.h>

#include "font.hpp"

//#define TRACE_FONT_LIFE

Font::Font(ifont* font, bool owner) :
	f(font), destroy(owner) {
#ifdef TRACE_FONT_LIFE
	dbg << "Constructing font from pointer " << font << ", autodestruct = "
			<< destroy << endl;
#endif
}

Font::~Font() {
#ifdef TRACE_FONT_LIFE
	dbg << "Destructing font " << f.get() << ", autodestruct = "
			<< destroy << endl;
#endif
	if (!destroy) {
		// Disable auto-destruction
		f.reset();
	}
}

ifont* Font::font() const {
	return f.get();
}


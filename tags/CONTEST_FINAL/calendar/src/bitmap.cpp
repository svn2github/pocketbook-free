/*
 * bitmap.cpp
 *
 *  Created on: Jun 14, 2009
 *      Author: fax
 */

#include <inkview.h>
#include "bitmap.hpp"

Bitmap::Bitmap(ibitmap* bitmap) :
	bmp(bitmap) {

}

Bitmap::~Bitmap() {
}

int Bitmap::width() const {
	return bmp ->width;
}
int Bitmap::height() const {
	return bmp -> height;
}

ibitmap* Bitmap::bitmap() const {
	return bmp;
}

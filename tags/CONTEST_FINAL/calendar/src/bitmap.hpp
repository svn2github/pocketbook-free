/*
 * bitmap.hpp
 *
 *  Created on: Jun 14, 2009
 *      Author: fax
 */

#ifndef BITMAP_HPP_
#define BITMAP_HPP_

struct Bitmap {
public:
	Bitmap( ibitmap * bitmap);
	virtual ~Bitmap();

	int width() const;
	int height() const;

	ibitmap* bitmap() const;

private:
	ibitmap* bmp;
};

#endif /* BITMAP_HPP_ */

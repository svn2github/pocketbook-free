/*
 * rect.hpp
 *
 *  Created on: Jun 10, 2009
 *      Author: fax
 */

#ifndef RECT_HPP_
#define RECT_HPP_

#include <assert.h>
#include <iostream>

struct Rect {
	int x, y, w, h;
public:
	Rect() :
		x(0), y(0), w(-1), h(-1) {
	}
	;

	Rect(int x_, int y_, int w_, int h_) :
		x(x_), y(y_), w(w_), h(h_) {
		//assert(x >= 0);
		//assert(y >= 0);
		assert(w >= 0);
		assert(h >= 0);
	}

	int width() const {
		return w;
	}

	int height() const {
		return h;
	}

	int left() const {
		return x;
	}
	int right() const {
		return x + w;
	}
	int top() const {
		return y;
	}
	int bottom() const {
		return y + h;
	}

	bool is_empty() const {
		return w == 0 || h == 0;
	}

	void reset() {
		x = y = w = h = 0;
	}

	void shift_by(int dx, int dy) {
		*this = this->shifted_by(dx, dy);
	}

	Rect shifted_by(int dx, int dy) const {
		return Rect(x + dx, y + dy, w, h);
	}

	void union_with(const Rect& r) {
		int x1 = x, x2 = x + w, y1 = y, y2 = y + w;
		if (r.x < x)
			x1 = r.x;
		if (r.y < y)
			y1 = r.y;
		if (r.x + r.w > x + w)
			x2 = r.x + r.w;
		if (r.y + r.h > y + h)
			y2 = r.y + r.h;

		*this = Rect(x1, y1, x2 - x1, y2 - y1);
	}

	Rect unioned_with(const Rect& r) const {
		Rect result = *this;
		result.union_with(r);
		return result;
	}

	void inflate(int dw, int dh) {
		x -= dw;
		y -= dh;
		w += dw * 2;
		h += dh * 2;
	}

	void cut_left(int dw) {
		x += dw;
#ifdef RECT_TRACE_CUT
		std::cout << "Cutting from " << w << ", " << dw << std::endl;
#endif
		w -= dw;
		assert( w >= 0);
	}

	void cut_right(int dw) {
#ifdef RECT_TRACE_CUT
		std::cout << "Cutting from " << w << ", " << dw << std::endl;
#endif
		w -= dw;
		assert( w >= 0);
	}

	void cut_top(int dh) {
#ifdef RECT_TRACE_CUT
		std::cout << "Cutting from " << w << ", " << dw << std::endl;
#endif
		y += dh;
		h -= dh;
		assert( h >= 0);
	}

	Rect cutted_left(int dw) const {
		Rect result = *this;
		result.cut_left(dw);
		return result;
	}

	Rect cutted_right(int dw) const {
		// TODO: Do it more efficiently?
		Rect result = *this;
		result.cut_right(dw);
		return result;
	}
	Rect inflated_by(int dw, int dh) const {
		Rect result = *this;
		result.inflate(dw, dh);
		return result;
	}

};

std::ostream& operator<<(std::ostream& o, const Rect& r);

#endif /* RECT_HPP_ */

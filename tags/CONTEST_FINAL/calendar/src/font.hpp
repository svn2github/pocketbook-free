/*
 * Font.h
 *
 *  Created on: Jun 9, 2009
 *      Author: fax
 */

#ifndef FONT_H_
#define FONT_H_

//TODO: After code compiles, add operator ifont*()

#include <boost/shared_ptr.hpp>

#include <inkview.h>

class Font {
public:
	Font(ifont* font, bool owner = false);
	virtual ~Font();

	ifont* font() const;
private:
	boost::shared_ptr<ifont> f;
	bool destroy;
};

namespace std {
template<>
struct less<Font> :
		public binary_function<Font, Font, bool> {
	bool operator()(const Font& __x, const Font& __y) const {
		return __x.font() < __y.font();
	}
};
}

#endif /* FONT_H_ */

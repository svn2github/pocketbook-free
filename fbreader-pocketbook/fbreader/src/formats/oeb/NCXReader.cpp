/*
 * Copyright (C) 2004-2009 Geometer Plus <contact@geometerplus.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <stdlib.h>

#include "NCXReader.h"

NCXReader::NCXReader(BookReader &modelReader) : myModelReader(modelReader), myReadState(READ_NONE), myPlayIndex(-65535) {
}

static const std::string TAG_NAVMAP = "navMap";
static const std::string TAG_NAVPOINT = "navPoint";
static const std::string TAG_NAVLABEL = "navLabel";
static const std::string TAG_CONTENT = "content";
static const std::string TAG_TEXT = "text";

void NCXReader::startElementHandler(const char *tag, const char **attributes) {
	switch (myReadState) {
		case READ_NONE:
			if (TAG_NAVMAP == tag) {
				myReadState = READ_MAP;
			}
			break;
		case READ_MAP:
			if (TAG_NAVPOINT == tag) {
				const char *order = attributeValue(attributes, "playOrder");
				myPointStack.push_back(NavPoint((order != 0) ? atoi(order) : myPlayIndex++, myPointStack.size()));
				myReadState = READ_POINT;
			}
			break;
		case READ_POINT:
			if (TAG_NAVPOINT == tag) {
				const char *order = attributeValue(attributes, "playOrder");
				myPointStack.push_back(NavPoint((order != 0) ? atoi(order) : myPlayIndex++, myPointStack.size()));
			} else if (TAG_NAVLABEL == tag) {
				myReadState = READ_LABEL;
			} else if (TAG_CONTENT == tag) {
				const char *src = attributeValue(attributes, "src");
				if (src != 0) {
					myPointStack.back().ContentHRef = src;
				}
			}
			break;
		case READ_LABEL:
			if (TAG_TEXT == tag) {
				myReadState = READ_TEXT;
			}
			break;
		case READ_TEXT:
			break;
	}
}

void NCXReader::endElementHandler(const char *tag) {
	switch (myReadState) {
		case READ_NONE:
			break;
		case READ_MAP:
			if (TAG_NAVMAP == tag) {
				myReadState = READ_NONE;
			}
			break;
		case READ_POINT:
			if (TAG_NAVPOINT == tag) {
				if (myPointStack.back().Text.empty()) {
					myPointStack.back().Text = "...";
				}
				myNavigationMap[myPointStack.back().Order] = myPointStack.back();
				myPointStack.pop_back();
				myReadState = myPointStack.empty() ? READ_MAP : READ_POINT;
			}
		case READ_LABEL:
			if (TAG_NAVLABEL == tag) {
				myReadState = READ_POINT;
			}
			break;
		case READ_TEXT:
			if (TAG_TEXT == tag) {
				myReadState = READ_LABEL;
			}
			break;
	}
}

void NCXReader::characterDataHandler(const char *text, int len) {

	if (myReadState == READ_TEXT) {
		myPointStack.back().Text.append(text, len);
	}
}

const std::map<int,NCXReader::NavPoint> &NCXReader::navigationMap() const {
	return myNavigationMap;
}

NCXReader::NavPoint::NavPoint() {
}

NCXReader::NavPoint::NavPoint(int order, size_t level) : Order(order), Level(level) {
}

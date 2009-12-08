/*
 * Copyright (C) 2004-2008 Geometer Plus <contact@geometerplus.com>
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

#include <ZLUnicodeUtil.h>

#include "HtmlMetainfoReader.h"

HtmlMetainfoReader::HtmlMetainfoReader(BookDescription &description, ReadType readType) : HtmlReader(description.encoding()), myDescription(description), myReadType(readType) {
}

bool HtmlMetainfoReader::tagHandler(const HtmlReader::HtmlTag &tag) {
	if (tag.Name == "BODY") {
		return false;
	} else if (((myReadType & TITLE) == TITLE) && (tag.Name == "DC:TITLE")) {
		myReadTitle = tag.Start;
		if (!tag.Start && !myTitle.empty()) {
			myDescription.title() = myTitle;
			myTitle.erase();
		}
	} else if (((myReadType & AUTHOR) == AUTHOR) && (tag.Name == "DC:CREATOR")) {
		if (tag.Start) {
			bool flag = false;
			for (size_t i = 0; i < tag.Attributes.size(); ++i) {
				if (tag.Attributes[i].Name == "ROLE") {
					flag = ZLUnicodeUtil::toUpper(tag.Attributes[i].Value) == "AUT";
					break;
				}
			}
			if (flag) {
				if (!myAuthor.empty()) {
					myAuthor += ", ";
				}
				myReadAuthor = true;
			}
		} else {
			myReadAuthor = false;
			if (!myAuthor.empty()) {
				myDescription.addAuthor(myAuthor);
			}
		}
	}
	return true;
}

void HtmlMetainfoReader::startDocumentHandler() {
	myReadAuthor = false;
	myReadTitle = false;
}

void HtmlMetainfoReader::endDocumentHandler() {
}

bool HtmlMetainfoReader::characterDataHandler(const char *text, int len, bool convert) {
	if (myReadTitle) {
		if (convert) {
			myConverter->convert(myTitle, text, text + len);
		} else {
			myTitle.append(text, len);
		}
	} else if (myReadAuthor) {
		if (convert) {
			myConverter->convert(myAuthor, text, text + len);
		} else {
			myAuthor.append(text, len);
		}
	}
	return true;
}

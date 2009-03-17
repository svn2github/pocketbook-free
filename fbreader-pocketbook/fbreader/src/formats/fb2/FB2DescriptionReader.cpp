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

#include <stdlib.h>

#include <ZLInputStream.h>
#include <ZLStringUtil.h>

#include "FB2DescriptionReader.h"

FB2DescriptionReader::FB2DescriptionReader(BookDescription &description) : myDescription(description) {
	myDescription.clearAuthor();
	myDescription.title().erase();
	myDescription.language().erase();
}

void FB2DescriptionReader::characterDataHandler(const char *text, int len) {
	if (myReadSomething) {
		if (myReadTitle) {
			myDescription.title().append(text, len);
		} else if (myReadLanguage) {
			myDescription.language().append(text, len);
		} else {
			for (int i = 0; i < 3; ++i) {
				if (myReadAuthorName[i]) {
					myAuthorNames[i].append(text, len);
					break;
				}
			}
		}
	}
}

void FB2DescriptionReader::startElementHandler(int tag, const char **attributes) {
	switch (tag) {
		case _BODY:
			myReturnCode = true;
			interrupt();
			break;
		case _TITLE_INFO:
			myReadSomething = true;
			break;
		case _BOOK_TITLE:
			myReadTitle = true;
			break;
		case _AUTHOR:
			myReadAuthor = true;
			break;
		case _LANG:
			myReadLanguage = true;
			break;
		case _FIRST_NAME:
			if (myReadAuthor) {
				myReadAuthorName[0] = true;
			}
			break;
		case _MIDDLE_NAME:
			if (myReadAuthor) {
				myReadAuthorName[1] = true;
			}
			break;
		case _LAST_NAME:
			if (myReadAuthor) {
				myReadAuthorName[2] = true;
			}
			break;
		case _SEQUENCE:
			if (myReadSomething) {
				const char *name = attributeValue(attributes, "name");
				if (name != 0) {
					std::string sequenceName = name;
					ZLStringUtil::stripWhiteSpaces(sequenceName);
					myDescription.sequenceName() = sequenceName;
					const char *number = attributeValue(attributes, "number");
					myDescription.numberInSequence() = (number != 0) ? atoi(number) : 0;
				}
			}
			break;
		default:
			break;
	}
}

void FB2DescriptionReader::endElementHandler(int tag) {
	switch (tag) {
		case _TITLE_INFO:
			myReadSomething = false;
			break;
		case _BOOK_TITLE:
			myReadTitle = false;
			break;
		case _AUTHOR:
			if (myReadSomething) {
				ZLStringUtil::stripWhiteSpaces(myAuthorNames[0]);
				ZLStringUtil::stripWhiteSpaces(myAuthorNames[1]);
				ZLStringUtil::stripWhiteSpaces(myAuthorNames[2]);
				std::string fullName = myAuthorNames[0];
				if (!fullName.empty() && !myAuthorNames[1].empty()) {
					fullName += ' ';
				}
				fullName += myAuthorNames[1];
				if (!fullName.empty() && !myAuthorNames[2].empty()) {
					fullName += ' ';
				}
				fullName += myAuthorNames[2];
				myDescription.addAuthor(fullName, myAuthorNames[2]);
				myAuthorNames[0].erase();
				myAuthorNames[1].erase();
				myAuthorNames[2].erase();
				myReadAuthor = false;
			}
			break;
		case _LANG:
			myReadLanguage = false;
			break;
		case _FIRST_NAME:
			myReadAuthorName[0] = false;
			break;
		case _MIDDLE_NAME:
			myReadAuthorName[1] = false;
			break;
		case _LAST_NAME:
			myReadAuthorName[2] = false;
			break;
		default:
			break;
	}
}

bool FB2DescriptionReader::readDescription(const std::string &fileName) {
	myReadSomething = false;
	myReadTitle = false;
	myReadAuthor = false;
	myReadLanguage = false;
	for (int i = 0; i < 3; ++i) {
		myReadAuthorName[i] = false;
	}
	return readDocument(fileName);
}

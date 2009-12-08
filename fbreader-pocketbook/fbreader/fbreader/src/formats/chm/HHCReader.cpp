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

#include <ZLUnicodeUtil.h>

#include "HHCReader.h"
#include "CHMReferenceCollection.h"

extern char *encoding_override;

struct xxx_toc_entry {
	int paragraph;
	int level;
	std::string text;
};

extern std::vector<struct xxx_toc_entry> xxx_myTOC;

HHCReader::HHCReader(CHMReferenceCollection &collection, BookModel &model, const std::string &encoding) : HtmlReader(encoding), myReferenceCollection(collection), myBookReader(model) {

fprintf(stderr, "-----\n");
	is1251 = false;
	const char *s = encoding.c_str();
	if (s && strcasecmp(s, "windows-1251") == 0) {
		is1251 = true;
		fprintf(stderr, "----- WIN-1251\n");
	}
fprintf(stderr, "-----\n");


}

HHCReader::~HHCReader() {
}

void HHCReader::startDocumentHandler() {
	myBookReader.setMainTextModel();
}

void HHCReader::endDocumentHandler() {
	std::string tmp0;
	myText.swap(tmp0);
	std::string tmp1;
	myReference.swap(tmp1);
}

static const std::string UL = "UL";
static const std::string LI = "LI";
static const std::string OBJECT = "OBJECT";
static const std::string PARAM = "PARAM";
static const std::string NAME = "NAME";
static const std::string VALUE = "VALUE";
static const std::string NAME_VALUE = "Name";
static const std::string LOCAL_VALUE = "Local";

static bool isFirstChild = false;

bool HHCReader::tagHandler(const HtmlTag &tag) {
	if (tag.Start) {
		if (tag.Name == UL) {
			isFirstChild = true;
		} else if (tag.Name == LI) {
		} else if (tag.Name == OBJECT) {
			myText.erase();
			myReference.erase();
		} else if (tag.Name == PARAM) {
			std::string name;
			std::string value;
			for (std::vector<HtmlAttribute>::const_iterator it = tag.Attributes.begin(); it != tag.Attributes.end(); ++it) {
				if (it->Name == NAME) {
					name = it->Value;
				} else if (it->Name == VALUE) {
					value = it->Value;
				}
			}
			if (name == NAME_VALUE) {
				myText = value;
			} else if (name == LOCAL_VALUE) {
				myReference = myReferenceCollection.addReference(value, true);
			}
		}
	} else {
		if (tag.Name == UL) {
			myBookReader.endContentsParagraph();
		} else if (tag.Name == OBJECT) {
			if (!myText.empty() || !myReference.empty()) {
				if (!isFirstChild) {
					myBookReader.endContentsParagraph();
				} else {
					isFirstChild = false;
				}
				myBookReader.beginContentsParagraph();
				if (myText.empty()) {
					myText = "...";
				}

				if (is1251 || (encoding_override && strcasecmp(encoding_override, "windows-1251")) == 0) {
					char buf[1024];
					unsigned char *p = (unsigned char *) myText.c_str();
					unsigned char *ps = (unsigned char *) buf;
					unsigned char *pp = ps;
					while (*p != 0 && pp-ps < sizeof(buf)-2) {
						unsigned char cc = *(p+1);
						if (*p == 0xc3 && (cc >= 0x80 && cc <= 0xbf)) {
							if (cc <= 0xaf) {
								*(pp++) = 0xd0;
								*(pp++) = cc + 0x10;
							} else {
								*(pp++) = 0xd1;
								*(pp++) = cc - 0x30;
							}
							p += 2;
						} else if (*p == '&' && strncasecmp((char *)p, "&ntilde;", 8) == 0) {
							*(pp++) = 0xd1;
							*(pp++) = 0x81;
							p += 8;
						} else if (*p == '&' && strncasecmp((char *)p, "&dstrok;", 8) == 0) {
							*(pp++) = 0xd0;
							*(pp++) = 0xa0;
							p += 8;
						} else {
							*(pp++) = *(p++);
						}
					}
					*pp = 0;
					myText = buf;
				}
				
				myBookReader.addContentsData(myText.empty() ? "..." : myText);
				myReferenceVector.push_back(ZLUnicodeUtil::toLower(myReference));
			}
		}
	}
	return true;
}

bool HHCReader::characterDataHandler(const char*, int, bool) {
	return true;
}

void HHCReader::setReferences() {
	for (size_t i = 0; i < myReferenceVector.size(); ++i) {
		myBookReader.setReference(i, myBookReader.model().label(myReferenceVector[i]).ParagraphNumber);
		if(xxx_myTOC.size() > i)
			xxx_myTOC[i].paragraph = myBookReader.model().label(myReferenceVector[i]).ParagraphNumber;

	}
}

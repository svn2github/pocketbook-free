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

#include <algorithm>

#include <ZLUnicodeUtil.h>
#include <ZLFileImage.h>

#include "OEBBookReader.h"
#include "../xhtml/XHTMLReader.h"
#include "../util/MiscUtil.h"
#include "../../bookmodel/BookModel.h"
#include "NCXReader.h"

OEBBookReader::OEBBookReader(BookModel &model) : myModelReader(model) {
}

void OEBBookReader::characterDataHandler(const char*, int) {
}

static const std::string MANIFEST = "manifest";
static const std::string SPINE = "spine";
static const std::string GUIDE = "guide";
static const std::string TOUR = "tour";
static const std::string SITE = "site";

static const std::string ITEM = "item";
static const std::string ITEMREF = "itemref";
static const std::string REFERENCE = "reference";

void OEBBookReader::startElementHandler(const char *tag, const char **xmlattributes) {
	const std::string tagString = ZLUnicodeUtil::toLower(tag);
	if (MANIFEST == tagString) {
		myState = READ_MANIFEST;
	} else if (SPINE == tagString) {
		const char *toc = attributeValue(xmlattributes, "toc");
		if (toc != 0) {
			myNCXTOCFileName = myIdToHref[toc];
		}
		myState = READ_SPINE;
	} else if (GUIDE == tagString) {
		myState = READ_GUIDE;
	} else if (TOUR == tagString) {
		myState = READ_TOUR;
	} else if ((myState == READ_MANIFEST) && (ITEM == tagString)) {
		const char *id = attributeValue(xmlattributes, "id");
		const char *href = attributeValue(xmlattributes, "href");
		if ((id != 0) && (href != 0)) {
			myIdToHref[id] = href;
		}
	} else if ((myState == READ_SPINE) && (ITEMREF == tagString)) {
		const char *id = attributeValue(xmlattributes, "idref");
		if (id != 0) {
			const std::string &fileName = myIdToHref[id];
			if (!fileName.empty()) {
				myHtmlFileNames.push_back(fileName);
			}
		}
	} else if ((myState == READ_GUIDE) && (REFERENCE == tagString)) {
		const char *type = attributeValue(xmlattributes, "type");
		const char *title = attributeValue(xmlattributes, "title");
		const char *href = attributeValue(xmlattributes, "href");
		if (href != 0) {
			if (title != 0) {
				myGuideTOC.push_back(std::pair<std::string,std::string>(title, href));
			}
			static const std::string COVER_IMAGE = "other.ms-coverimage-standard";
			if ((type != 0) && (COVER_IMAGE == type)) {
				myModelReader.setMainTextModel();
				myModelReader.addImageReference(href);
				myModelReader.addImage(href, new ZLFileImage("image/auto", myFilePrefix + href, 0));
			}
		}
	} else if ((myState == READ_TOUR) && (SITE == tagString)) {
		const char *title = attributeValue(xmlattributes, "title");
		const char *href = attributeValue(xmlattributes, "href");
		if ((title != 0) && (href != 0)) {
			myTourTOC.push_back(std::pair<std::string,std::string>(title, href));
		}
	}
}

void OEBBookReader::endElementHandler(const char *tag) {
	const std::string tagString = ZLUnicodeUtil::toLower(tag);
	if ((MANIFEST == tagString) || (SPINE == tagString) || (GUIDE == tagString) || (TOUR == tagString)) {
		myState = READ_NONE;
	}
}

bool OEBBookReader::readBook(const std::string &fileName) {
	myFilePrefix = MiscUtil::htmlDirectoryPrefix(fileName);

	myIdToHref.clear();
	myHtmlFileNames.clear();
	myNCXTOCFileName.erase();
	myTourTOC.clear();
	myGuideTOC.clear();
	myState = READ_NONE;

	if (!readDocument(fileName)) {
		return false;
	}

	myModelReader.setMainTextModel();
	myModelReader.pushKind(REGULAR);

	for (std::vector<std::string>::const_iterator it = myHtmlFileNames.begin(); it != myHtmlFileNames.end(); ++it) {
		XHTMLReader(myModelReader).readFile(myFilePrefix, *it, *it);
	}

	generateTOC();

/*
	std::vector<std::pair<std::string,std::string> > &toc = myTourTOC.empty() ? myGuideTOC : myTourTOC;
	for (std::vector<std::pair<std::string,std::string> >::const_iterator it = toc.begin(); it != toc.end(); ++it) {
		int index = myModelReader.model().label(it->second).ParagraphNumber;
		if (index != -1) {
			myModelReader.beginContentsParagraph(index);
			myModelReader.addContentsData(it->first);
			myModelReader.endContentsParagraph();
		}
	}
*/

	return true;
}

void OEBBookReader::generateTOC() {
	if (!myNCXTOCFileName.empty()) {
		NCXReader ncxReader(myModelReader);
		if (ncxReader.readDocument(myFilePrefix + myNCXTOCFileName)) {
			const std::map<int,NCXReader::NavPoint> navigationMap = ncxReader.navigationMap();
			if (!navigationMap.empty()) {
				size_t level = 0;
				for (std::map<int,NCXReader::NavPoint>::const_iterator it = navigationMap.begin(); it != navigationMap.end(); ++it) {
					const NCXReader::NavPoint &point = it->second;
					int index = myModelReader.model().label(point.ContentHRef).ParagraphNumber;
					while (level > point.Level) {
						myModelReader.endContentsParagraph();
						--level;
					}
					while (++level <= point.Level) {
						myModelReader.beginContentsParagraph(-2);
						myModelReader.addContentsData("...");
					}
					myModelReader.beginContentsParagraph(index);
					myModelReader.addContentsData(point.Text);
				}
				while (level > 0) {
					myModelReader.endContentsParagraph();
					--level;
				}
				return;
			}
		}
	}
}



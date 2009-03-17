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

#include <string.h>

#include <ZLFileImage.h>
#include <ZLUnicodeUtil.h>

#include "XHTMLReader.h"
#include "../util/EntityFilesCollector.h"
#include "../util/MiscUtil.h"

#include "../../bookmodel/BookReader.h"
#include "../../bookmodel/BookModel.h"

std::map<std::string,XHTMLTagAction*> XHTMLReader::ourTagActions;

XHTMLTagAction::~XHTMLTagAction() {
}

class XHTMLTagParagraphAction : public XHTMLTagAction {

public:
	void doAtStart(XHTMLReader &reader, const char **xmlattributes);
	void doAtEnd(XHTMLReader &reader);
};

class XHTMLTagRestartParagraphAction : public XHTMLTagAction {

public:
	void doAtStart(XHTMLReader &reader, const char **xmlattributes);
	void doAtEnd(XHTMLReader &reader);
};

class XHTMLTagImageAction : public XHTMLTagAction {

public:
	XHTMLTagImageAction(const std::string &nameAttribute);

	void doAtStart(XHTMLReader &reader, const char **xmlattributes);
	void doAtEnd(XHTMLReader &reader);

private:
	const std::string myNameAttribute;
};

class XHTMLTagItemAction : public XHTMLTagAction {

public:
	void doAtStart(XHTMLReader &reader, const char **xmlattributes);
	void doAtEnd(XHTMLReader &reader);
};

class XHTMLTagHyperlinkAction : public XHTMLTagAction {

public:
	void doAtStart(XHTMLReader &reader, const char **xmlattributes);
	void doAtEnd(XHTMLReader &reader);

private:
	std::stack<FBTextKind> myHyperlinkStack;
};

class XHTMLTagControlAction : public XHTMLTagAction {

public:
	XHTMLTagControlAction(FBTextKind control);

	void doAtStart(XHTMLReader &reader, const char **xmlattributes);
	void doAtEnd(XHTMLReader &reader);

private:
	FBTextKind myControl;
};

class XHTMLTagParagraphWithControlAction : public XHTMLTagAction {

public:
	XHTMLTagParagraphWithControlAction(FBTextKind control);

	void doAtStart(XHTMLReader &reader, const char **xmlattributes);
	void doAtEnd(XHTMLReader &reader);

private:
	FBTextKind myControl;
};

class XHTMLTagPreAction : public XHTMLTagAction {

public:
	void doAtStart(XHTMLReader &reader, const char **xmlattributes);
	void doAtEnd(XHTMLReader &reader);
};

void XHTMLTagParagraphAction::doAtStart(XHTMLReader &reader, const char**) {
	reader.myModelReader.beginParagraph();
}

void XHTMLTagParagraphAction::doAtEnd(XHTMLReader &reader) {
	reader.myModelReader.endParagraph();
}

void XHTMLTagRestartParagraphAction::doAtStart(XHTMLReader &reader, const char**) {
	reader.myModelReader.endParagraph();
	reader.myModelReader.beginParagraph();
}

void XHTMLTagRestartParagraphAction::doAtEnd(XHTMLReader&) {
}

void XHTMLTagItemAction::doAtStart(XHTMLReader &reader, const char**) {
	reader.myModelReader.endParagraph();
	// TODO: increase left indent
	reader.myModelReader.beginParagraph();
	// TODO: replace bullet sign by number inside OL tag
	const std::string bullet = "\xE2\x80\xA2\xC0\xA0";
	reader.myModelReader.addData(bullet);
}

void XHTMLTagItemAction::doAtEnd(XHTMLReader &reader) {
	reader.myModelReader.endParagraph();
}

XHTMLTagImageAction::XHTMLTagImageAction(const std::string &nameAttribute) : myNameAttribute(nameAttribute) {
}

void XHTMLTagImageAction::doAtStart(XHTMLReader &reader, const char **xmlattributes) {
	const char *fileName = reader.attributeValue(xmlattributes, myNameAttribute.c_str());
	if (fileName != 0) {
		bool flag = reader.myModelReader.paragraphIsOpen();
		if (flag) {
			reader.myModelReader.endParagraph();
		}
		if ((strlen(fileName) > 2) && strncmp(fileName, "./", 2) == 0) {
			fileName +=2;
		}
		const std::string fullfileName = reader.myPathPrefix + fileName;
		reader.myModelReader.addImageReference(fullfileName);
		reader.myModelReader.addImage(fullfileName, new ZLFileImage("image/auto", fullfileName, 0));
		if (flag) {
			reader.myModelReader.beginParagraph();
		}
	}
}

void XHTMLTagImageAction::doAtEnd(XHTMLReader&) {
}

XHTMLTagControlAction::XHTMLTagControlAction(FBTextKind control) : myControl(control) {
}

void XHTMLTagControlAction::doAtStart(XHTMLReader &reader, const char**) {
	reader.myModelReader.pushKind(myControl);
	reader.myModelReader.addControl(myControl, true);
}

void XHTMLTagControlAction::doAtEnd(XHTMLReader &reader) {
	reader.myModelReader.addControl(myControl, false);
	reader.myModelReader.popKind();
}

void XHTMLTagHyperlinkAction::doAtStart(XHTMLReader &reader, const char **xmlattributes) {
	const char *href = reader.attributeValue(xmlattributes, "href");
	if (href != 0) {
		const std::string link = (*href == '#') ? (reader.myReferenceName + href) : href;
		FBTextKind hyperlinkType = MiscUtil::isReference(link) ? EXTERNAL_HYPERLINK : INTERNAL_HYPERLINK;
		myHyperlinkStack.push(hyperlinkType);
		reader.myModelReader.addHyperlinkControl(hyperlinkType, link);
	} else {
		myHyperlinkStack.push(REGULAR);
	}
	const char *name = reader.attributeValue(xmlattributes, "name");
	if (name != 0) {
		reader.myModelReader.addHyperlinkLabel(reader.myReferenceName + "#" + name);
	}
}

void XHTMLTagHyperlinkAction::doAtEnd(XHTMLReader &reader) {
	FBTextKind kind = myHyperlinkStack.top();
	if (kind != REGULAR) {
		reader.myModelReader.addControl(kind, false);
	}
	myHyperlinkStack.pop();
}

XHTMLTagParagraphWithControlAction::XHTMLTagParagraphWithControlAction(FBTextKind control) : myControl(control) {
}

void XHTMLTagParagraphWithControlAction::doAtStart(XHTMLReader &reader, const char**) {
	if ((myControl == TITLE) && (reader.myModelReader.model().bookTextModel()->paragraphsNumber() > 1)) {
		reader.myModelReader.insertEndOfSectionParagraph();
	}
	reader.myModelReader.pushKind(myControl);
	reader.myModelReader.beginParagraph();
}

void XHTMLTagParagraphWithControlAction::doAtEnd(XHTMLReader &reader) {
	reader.myModelReader.endParagraph();
	reader.myModelReader.popKind();
}

void XHTMLTagPreAction::doAtStart(XHTMLReader &reader, const char**) {
	reader.myPreformatted = true;
	reader.myModelReader.beginParagraph();
	reader.myModelReader.addControl(CODE, true);
}

void XHTMLTagPreAction::doAtEnd(XHTMLReader &reader) {
	reader.myModelReader.addControl(CODE, false);
	reader.myModelReader.endParagraph();
	reader.myPreformatted = false;
}

void XHTMLReader::addAction(const std::string &tag, XHTMLTagAction *action) {
	ourTagActions.insert(std::pair<std::string,XHTMLTagAction*>(tag,action));
}

void XHTMLReader::fillTagTable() {
	if (ourTagActions.empty()) {
		//addAction("html",	new XHTMLTagAction());
		addAction("body",	new XHTMLTagParagraphAction());
		//addAction("title",	new XHTMLTagAction());
		//addAction("meta",	new XHTMLTagAction());
		//addAction("script",	new XHTMLTagAction());

		//addAction("font",	new XHTMLTagAction());
		//addAction("style",	new XHTMLTagAction());

		addAction("p",	new XHTMLTagParagraphAction());
		addAction("h1",	new XHTMLTagParagraphWithControlAction(H1));
		addAction("h2",	new XHTMLTagParagraphWithControlAction(H2));
		addAction("h3",	new XHTMLTagParagraphWithControlAction(H3));
		addAction("h4",	new XHTMLTagParagraphWithControlAction(H4));
		addAction("h5",	new XHTMLTagParagraphWithControlAction(H5));
		addAction("h6",	new XHTMLTagParagraphWithControlAction(H6));

		//addAction("ol",	new XHTMLTagAction());
		//addAction("ul",	new XHTMLTagAction());
		//addAction("dl",	new XHTMLTagAction());
		addAction("li",	new XHTMLTagItemAction());

		addAction("strong",	new XHTMLTagControlAction(STRONG));
		addAction("b",	new XHTMLTagControlAction(BOLD));
		addAction("em",	new XHTMLTagControlAction(EMPHASIS));
		addAction("i",	new XHTMLTagControlAction(ITALIC));
		addAction("code",	new XHTMLTagControlAction(CODE));
		addAction("tt",	new XHTMLTagControlAction(CODE));
		addAction("kbd",	new XHTMLTagControlAction(CODE));
		addAction("var",	new XHTMLTagControlAction(CODE));
		addAction("samp",	new XHTMLTagControlAction(CODE));
		addAction("cite",	new XHTMLTagControlAction(CITE));
		addAction("sub",	new XHTMLTagControlAction(SUB));
		addAction("sup",	new XHTMLTagControlAction(SUP));
		addAction("dd",	new XHTMLTagControlAction(DEFINITION_DESCRIPTION));
		addAction("dfn",	new XHTMLTagControlAction(DEFINITION));
		addAction("strike",	new XHTMLTagControlAction(STRIKETHROUGH));

		addAction("a",	new XHTMLTagHyperlinkAction());

		addAction("img",	new XHTMLTagImageAction("src"));
		addAction("object",	new XHTMLTagImageAction("data"));

		//addAction("area",	new XHTMLTagAction());
		//addAction("map",	new XHTMLTagAction());

		//addAction("base",	new XHTMLTagAction());
		//addAction("blockquote",	new XHTMLTagAction());
		addAction("br",	new XHTMLTagRestartParagraphAction());
		//addAction("center",	new XHTMLTagAction());
		addAction("div",	new XHTMLTagParagraphAction());
		//addAction("dt",	new XHTMLTagAction());
		//addAction("head",	new XHTMLTagAction());
		//addAction("hr",	new XHTMLTagAction());
		//addAction("link",	new XHTMLTagAction());
		//addAction("param",	new XHTMLTagAction());
		//addAction("q",	new XHTMLTagAction());
		//addAction("s",	new XHTMLTagAction());

		addAction("pre",	new XHTMLTagPreAction());
		//addAction("big",	new XHTMLTagAction());
		//addAction("small",	new XHTMLTagAction());
		//addAction("u",	new XHTMLTagAction());

		//addAction("table",	new XHTMLTagAction());
		addAction("td",	new XHTMLTagParagraphAction());
		addAction("th",	new XHTMLTagParagraphAction());
		//addAction("tr",	new XHTMLTagAction());
		//addAction("caption",	new XHTMLTagAction());
		//addAction("span",	new XHTMLTagAction());
	}
}

XHTMLReader::XHTMLReader(BookReader &modelReader) : myModelReader(modelReader) {
}

bool XHTMLReader::readFile(const std::string &pathPrefix, const std::string &fileName, const std::string &referenceName) {
	myModelReader.addHyperlinkLabel(referenceName);

	fillTagTable();

	myPathPrefix = pathPrefix;
	myReferenceName = referenceName;

	myPreformatted = false;

	return readDocument(pathPrefix + fileName);
}


void XHTMLReader::startElementHandler(const char *tag, const char **attributes) {
	static const std::string HASH = "#";
	const char *id = attributeValue(attributes, "id");
	if (id != 0) {
		myModelReader.addHyperlinkLabel(myReferenceName + HASH + id);
	}

	XHTMLTagAction *action = ourTagActions[ZLUnicodeUtil::toLower(tag)];
	if (action != 0) {
		action->doAtStart(*this, attributes);
	}
}

void XHTMLReader::endElementHandler(const char *tag) {
	XHTMLTagAction *action = ourTagActions[ZLUnicodeUtil::toLower(tag)];
	if (action != 0) {
		action->doAtEnd(*this);
	}
}

void XHTMLReader::characterDataHandler(const char *text, int len) {
	if (myPreformatted) {
		if ((*text == '\r') || (*text == '\n')) {
			myModelReader.addControl(CODE, false);
			myModelReader.endParagraph();
			myModelReader.beginParagraph();
			myModelReader.addControl(CODE, true);
		}
		int spaceCounter = 0;
		while ((spaceCounter < len) && isspace((unsigned char)*text)) {
			++spaceCounter;
		}
		myModelReader.addFixedHSpace(spaceCounter);
		text += spaceCounter;
		len -= spaceCounter;
	}
	if (len > 0) {
		myModelReader.addData(std::string(text, len));
	}
}

const std::vector<std::string> &XHTMLReader::externalDTDs() const {
	return EntityFilesCollector::instance().externalDTDs("xhtml");
}

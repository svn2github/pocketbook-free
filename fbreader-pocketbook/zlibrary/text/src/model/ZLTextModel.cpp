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

#include <algorithm>

#include <ZLSearchUtil.h>

#include "ZLTextModel.h"
#include "ZLTextParagraph.h"

ZLTextModel::ZLTextModel(const size_t rowSize) : myAllocator(rowSize), myLastEntryStart(0) {
}

ZLTextModel::~ZLTextModel() {
	for (std::vector<ZLTextParagraph*>::const_iterator it = myParagraphs.begin(); it != myParagraphs.end(); ++it) {
		delete *it;
	}
}

void ZLTextModel::search(const std::string &text, size_t startIndex, size_t endIndex, bool ignoreCase) const {
	ZLSearchPattern pattern(text, ignoreCase);
	myMarks.clear();

	std::vector<ZLTextParagraph*>::const_iterator start =
		(startIndex < myParagraphs.size()) ? myParagraphs.begin() + startIndex : myParagraphs.end();
	std::vector<ZLTextParagraph*>::const_iterator end =
		(endIndex < myParagraphs.size()) ? myParagraphs.begin() + endIndex : myParagraphs.end();
	for (std::vector<ZLTextParagraph*>::const_iterator it = start; it < end; ++it) {
		int offset = 0;
		for (ZLTextParagraph::Iterator jt = **it; !jt.isEnd(); jt.next()) {
			if (jt.entryKind() == ZLTextParagraphEntry::TEXT_ENTRY) {
				const ZLTextEntry& textEntry = (ZLTextEntry&)*jt.entry();
				const char *str = textEntry.data();
				const size_t len = textEntry.dataLength();
				for (int pos = ZLSearchUtil::find(str, len, pattern); pos != -1; pos = ZLSearchUtil::find(str, len, pattern, pos + 1)) {
					myMarks.push_back(ZLTextMark(it - myParagraphs.begin(), offset + pos, pattern.length()));
				}
				offset += len;
			}
		}
	}
}

void ZLTextModel::selectParagraph(size_t index) const {
	if (index < paragraphsNumber()) {
		myMarks.clear();
		myMarks.push_back(ZLTextMark(index, 0, (*this)[index]->textLength()));
	}
}

ZLTextMark ZLTextModel::firstMark() const {
	return marks().empty() ? ZLTextMark() : marks().front();
}

ZLTextMark ZLTextModel::lastMark() const {
	return marks().empty() ? ZLTextMark() : marks().back();
}

ZLTextMark ZLTextModel::nextMark(ZLTextMark position) const {
	std::vector<ZLTextMark>::const_iterator it = std::upper_bound(marks().begin(), marks().end(), position);
	return (it != marks().end()) ? *it : ZLTextMark();
}

ZLTextMark ZLTextModel::previousMark(ZLTextMark position) const {
	if (marks().empty()) {
		return ZLTextMark();
	}
	std::vector<ZLTextMark>::const_iterator it = std::lower_bound(marks().begin(), marks().end(), position);
	if (it == marks().end()) {
		--it;
	}
	if (*it >= position) {
		if (it == marks().begin()) {
			return ZLTextMark();
		}
		--it;
	}
	return *it;
}

ZLTextTreeModel::ZLTextTreeModel() : ZLTextModel(8192) {
	myRoot = new ZLTextTreeParagraph();
	myRoot->open(true);
}

ZLTextTreeModel::~ZLTextTreeModel() {
	delete myRoot;
}

void ZLTextModel::addParagraphInternal(ZLTextParagraph *paragraph) {
	myParagraphs.push_back(paragraph);
	myLastEntryStart = 0;
}

void ZLTextModel::removeParagraphInternal(int index) {
	if ((index >= 0) && (index < (int)myParagraphs.size())) {
		myParagraphs.erase(myParagraphs.begin() + index);
	}
}

void ZLTextTreeModel::removeParagraph(int index) {
	ZLTextTreeParagraph *p = (ZLTextTreeParagraph*)(*this)[index];
	p->removeFromParent();
	removeParagraphInternal(index);
	delete p;
}

ZLTextTreeParagraph *ZLTextTreeModel::createParagraph(ZLTextTreeParagraph *parent) {
	if (parent == 0) {
		parent = myRoot;
	}
	ZLTextTreeParagraph *tp = new ZLTextTreeParagraph(parent);
	addParagraphInternal(tp);
	return tp;
}

void ZLTextTreeModel::search(const std::string &text, size_t startIndex, size_t endIndex, bool ignoreCase) const {
	ZLTextModel::search(text, startIndex, endIndex, ignoreCase);
	for (std::vector<ZLTextMark>::const_iterator it = marks().begin(); it != marks().end(); ++it) {
		((ZLTextTreeParagraph*)(*this)[it->ParagraphNumber])->openTree();
	}
}

void ZLTextTreeModel::selectParagraph(size_t index) const {
	if (index < paragraphsNumber()) {
		ZLTextModel::selectParagraph(index);
		((ZLTextTreeParagraph*)(*this)[index])->openTree();
	}
}

ZLTextPlainModel::ZLTextPlainModel(const size_t rowSize) : ZLTextModel(rowSize) {
}

void ZLTextPlainModel::createParagraph(ZLTextParagraph::Kind kind) {
	ZLTextParagraph *paragraph = (kind == ZLTextParagraph::TEXT_PARAGRAPH) ? new ZLTextParagraph() : new ZLTextSpecialParagraph(kind);
	addParagraphInternal(paragraph);
}

void ZLTextModel::addText(const std::string &text) {
	size_t len = text.length();
	if ((myLastEntryStart != 0) && (*myLastEntryStart == ZLTextParagraphEntry::TEXT_ENTRY)) {
		size_t oldLen = 0;
		memcpy(&oldLen, myLastEntryStart + 1, sizeof(size_t));
		size_t newLen = oldLen + len;
		myLastEntryStart = myAllocator.reallocateLast(myLastEntryStart, newLen + sizeof(size_t) + 1);
		memcpy(myLastEntryStart + 1, &newLen, sizeof(size_t));
		memcpy(myLastEntryStart + sizeof(size_t) + 1 + oldLen, text.data(), len);
	} else {
		myLastEntryStart = myAllocator.allocate(len + sizeof(size_t) + 1);
		*myLastEntryStart = ZLTextParagraphEntry::TEXT_ENTRY;
		memcpy(myLastEntryStart + 1, &len, sizeof(size_t));
		memcpy(myLastEntryStart + sizeof(size_t) + 1, text.data(), len);
		myParagraphs.back()->addEntry(myLastEntryStart);
	}
}

void ZLTextModel::addText(const std::vector<std::string> &text) {
	if (text.size() == 0) {
		return;
	}
	size_t len = 0;
	for (std::vector<std::string>::const_iterator it = text.begin(); it != text.end(); ++it) {
		len += it->length();
	}
	if ((myLastEntryStart != 0) && (*myLastEntryStart == ZLTextParagraphEntry::TEXT_ENTRY)) {
		size_t oldLen = 0;
		memcpy(&oldLen, myLastEntryStart + 1, sizeof(size_t));
		size_t newLen = oldLen + len;
		myLastEntryStart = myAllocator.reallocateLast(myLastEntryStart, newLen + sizeof(size_t) + 1);
		memcpy(myLastEntryStart + 1, &newLen, sizeof(size_t));
		size_t offset = sizeof(size_t) + 1 + oldLen;
		for (std::vector<std::string>::const_iterator it = text.begin(); it != text.end(); ++it) {
			memcpy(myLastEntryStart + offset, it->data(), it->length());
			offset += it->length();
		}
	} else {
		myLastEntryStart = myAllocator.allocate(len + sizeof(size_t) + 1);
		*myLastEntryStart = ZLTextParagraphEntry::TEXT_ENTRY;
		memcpy(myLastEntryStart + 1, &len, sizeof(size_t));
		size_t offset = sizeof(size_t) + 1;
		for (std::vector<std::string>::const_iterator it = text.begin(); it != text.end(); ++it) {
			memcpy(myLastEntryStart + offset, it->data(), it->length());
			offset += it->length();
		}
		myParagraphs.back()->addEntry(myLastEntryStart);
	}
}

void ZLTextModel::addFixedHSpace(unsigned char length) {
	myLastEntryStart = myAllocator.allocate(2);
	*myLastEntryStart = ZLTextParagraphEntry::FIXED_HSPACE_ENTRY;
	*(myLastEntryStart + 1) = length;
	myParagraphs.back()->addEntry(myLastEntryStart);
}

void ZLTextModel::addControl(ZLTextKind textKind, bool isStart) {
	myLastEntryStart = myAllocator.allocate(2);
	*myLastEntryStart = ZLTextParagraphEntry::CONTROL_ENTRY;
	*(myLastEntryStart + 1) = (textKind << 1) + (isStart ? 1 : 0);
	myParagraphs.back()->addEntry(myLastEntryStart);
}

void ZLTextModel::addControl(const ZLTextForcedControlEntry &entry) {
	myLastEntryStart = myAllocator.allocate(3 + 2 * sizeof(short));
	*myLastEntryStart = ZLTextParagraphEntry::FORCED_CONTROL_ENTRY;
	*(myLastEntryStart + 1) = entry.myMask;
	memcpy(myLastEntryStart + 2, &entry.myLeftIndent, sizeof(short));
	memcpy(myLastEntryStart + 2 + sizeof(short), &entry.myRightIndent, sizeof(short));
	*(myLastEntryStart + 2 + 2 * sizeof(short)) = entry.myAlignmentType;
	myParagraphs.back()->addEntry(myLastEntryStart);
}

void ZLTextModel::addHyperlinkControl(ZLTextKind textKind, const std::string &label) {
	myLastEntryStart = myAllocator.allocate(label.length() + 3);
	*myLastEntryStart = ZLTextParagraphEntry::HYPERLINK_CONTROL_ENTRY;
	*(myLastEntryStart + 1) = textKind;
	memcpy(myLastEntryStart + 2, label.data(), label.length());
	*(myLastEntryStart + label.length() + 2) = '\0';
	myParagraphs.back()->addEntry(myLastEntryStart);
}

void ZLTextModel::addImage(const std::string &id, const ZLImageMap &imageMap, short vOffset) {
	myLastEntryStart = myAllocator.allocate(sizeof(const ZLImageMap*) + sizeof(short) + id.length() + 2);
	*myLastEntryStart = ZLTextParagraphEntry::IMAGE_ENTRY;
	const ZLImageMap *imageMapAddress = &imageMap;
	memcpy(myLastEntryStart + 1, &imageMapAddress, sizeof(const ZLImageMap*));
	memcpy(myLastEntryStart + 1 + sizeof(const ZLImageMap*), &vOffset, sizeof(short));
	memcpy(myLastEntryStart + 1 + sizeof(const ZLImageMap*) + sizeof(short), id.data(), id.length());
	*(myLastEntryStart + 1 + sizeof(const ZLImageMap*) + sizeof(short) + id.length()) = '\0';
	myParagraphs.back()->addEntry(myLastEntryStart);
}

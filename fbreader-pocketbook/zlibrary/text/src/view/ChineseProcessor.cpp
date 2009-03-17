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
#include <ZLImage.h>

#include <ZLTextParagraph.h>

#include "ZLTextParagraphCursor.h"
#include "ZLTextWord.h"

ZLTextParagraphCursor::ChineseProcessor::ChineseProcessor(const ZLTextParagraph &paragraph, const std::vector<ZLTextMark> &marks, int paragraphNumber, ZLTextElementVector &elements) : Processor(paragraph, marks, paragraphNumber, elements) {
}

void ZLTextParagraphCursor::ChineseProcessor::processTextEntry(const ZLTextEntry &textEntry) {
	if (textEntry.dataLength() != 0) {
		const char *start = textEntry.data();
		const char *end = start + textEntry.dataLength();
		ZLUnicodeUtil::Ucs2Char ch;
		ZLUnicodeUtil::firstChar(ch, start);
		bool spaceInserted = false;
		if (ZLUnicodeUtil::isSpace(ch)) {
			myElements.push_back(ZLTextElementPool::Pool.HSpaceElement);
			spaceInserted = true;
		}
		const char *firstNonSpace = 0;
		int charLength = 0;
		bool breakableBefore = false;
		for (const char *ptr = start; ptr < end; ptr += charLength) {
			if (breakableBefore) {
				if (firstNonSpace != 0) {
					addWord(firstNonSpace, myOffset + (firstNonSpace - textEntry.data()), ptr - firstNonSpace);
					firstNonSpace = 0;
					spaceInserted = false;
				}
				charLength = 0;
				breakableBefore = false;
				continue;
			}
			charLength = ZLUnicodeUtil::firstChar(ch, ptr);
			if (ZLUnicodeUtil::isSpace(ch)) {
				if (firstNonSpace != 0) {
					addWord(firstNonSpace, myOffset + (firstNonSpace - textEntry.data()), ptr - firstNonSpace);
					myElements.push_back(ZLTextElementPool::Pool.HSpaceElement);
					spaceInserted = true;
					firstNonSpace = 0;
				} else if (!spaceInserted) {
					myElements.push_back(ZLTextElementPool::Pool.HSpaceElement);
					spaceInserted = true;
				}
			} else if (firstNonSpace == 0) {
				firstNonSpace = ptr;
			} else {
				switch (ZLUnicodeUtil::isBreakable(ch)) {
					case ZLUnicodeUtil::NO_BREAKABLE:
						break;
					case ZLUnicodeUtil::BREAKABLE_BEFORE:
						addWord(firstNonSpace, myOffset + (firstNonSpace - textEntry.data()), ptr - firstNonSpace);
						firstNonSpace = ptr;
						break;
					case ZLUnicodeUtil::BREAKABLE_AFTER:
						breakableBefore = true;
						break;
				}
			}
		}
		if (firstNonSpace != 0) {
			addWord(firstNonSpace, myOffset + (firstNonSpace - textEntry.data()), end - firstNonSpace);
		}
		myOffset += textEntry.dataLength();
	}
}

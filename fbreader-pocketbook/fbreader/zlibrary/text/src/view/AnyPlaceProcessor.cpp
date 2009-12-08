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

ZLTextParagraphCursor::AnyPlaceProcessor::AnyPlaceProcessor(const ZLTextParagraph &paragraph, const std::vector<ZLTextMark> &marks, int paragraphNumber, ZLTextElementVector &elements) : Processor(paragraph, marks, paragraphNumber, elements) {
}

void ZLTextParagraphCursor::AnyPlaceProcessor::processTextEntry(const ZLTextEntry &textEntry) {
	if (textEntry.dataLength() != 0) {
		const char *start = textEntry.data();
		const char *end = start + textEntry.dataLength();
		bool addSpace = true;
		for (const char *ptr = start; ptr < end;) {
			ZLUnicodeUtil::Ucs2Char ch;
			int len = ZLUnicodeUtil::firstChar(ch, ptr);
			if (ZLUnicodeUtil::isSpace(ch)) {
				if (addSpace && (ch != '\n') && (ch != '\r')) {
					myElements.push_back(ZLTextElementPool::Pool.HSpaceElement);
					addSpace = false;
				}
			} else {
				if (ptr + len <= end) {
					addWord(ptr, myOffset + (ptr - start), len);
					addSpace = true;
				}
			}
			ptr += len;
		}
	}
}

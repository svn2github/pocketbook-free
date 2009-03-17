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

#include "FB2Reader.h"

void FB2Reader::startElementHandler(const char *t, const char **attributes) {
	startElementHandler(tag(t), attributes);
}

void FB2Reader::endElementHandler(const char *t) {
	endElementHandler(tag(t));
}

static const FB2Reader::Tag TAGS[] = {
	{"p", FB2Reader::_P0},
	{"subtitle", FB2Reader::_SUBTITLE},
	{"cite", FB2Reader::_CITE},
	{"text-author", FB2Reader::_TEXT_AUTHOR},
	{"date", FB2Reader::_DATE},
	{"section", FB2Reader::_SECTION},
	{"v", FB2Reader::_V},
	{"title", FB2Reader::_TITLE},
	{"poem", FB2Reader::_POEM},
	{"stanza", FB2Reader::_STANZA},
	{"epigraph", FB2Reader::_EPIGRAPH},
	{"annotation", FB2Reader::_ANNOTATION},
	{"sub", FB2Reader::_SUB},
	{"sup", FB2Reader::_SUP},
	{"code", FB2Reader::_CODE},
	{"strikethrough", FB2Reader::_STRIKETHROUGH},
	{"strong", FB2Reader::_STRONG},
	{"emphasis", FB2Reader::_EMPHASIS},
	{"a", FB2Reader::_A},
	{"image", FB2Reader::_IMAGE},
	{"binary", FB2Reader::_BINARY},
	{"body", FB2Reader::_BODY},
	{"empty-line", FB2Reader::_EMPTY_LINE},
	{"title-info", FB2Reader::_TITLE_INFO},
	{"book-title", FB2Reader::_BOOK_TITLE},
	{"author", FB2Reader::_AUTHOR},
	{"lang", FB2Reader::_LANG},
	{"first-name", FB2Reader::_FIRST_NAME},
	{"middle-name", FB2Reader::_MIDDLE_NAME},
	{"last-name", FB2Reader::_LAST_NAME},
	{"coverpage", FB2Reader::_COVERPAGE},
	{"sequence", FB2Reader::_SEQUENCE},
	{0, FB2Reader::_UNKNOWN}
};

int FB2Reader::tag(const char *name) {
	for (int i = 0; ; ++i) {
		if ((TAGS[i].tagName == 0) || (strcmp(name, TAGS[i].tagName) == 0)) {
			return TAGS[i].tagCode;
		}
	}
}

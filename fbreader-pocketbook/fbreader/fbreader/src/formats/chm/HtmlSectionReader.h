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

#ifndef __HTMLSECTIONREADER_H__
#define __HTMLSECTIONREADER_H__

#include "../html/HtmlBookReader.h"
#include "CHMFile.h"

class CHMReferenceCollection;

class HtmlSectionReader : public HtmlBookReader {

public:
	HtmlSectionReader(BookModel &model, const PlainTextFormat &format, const std::string &encoding, shared_ptr<CHMFileInfo> info, CHMReferenceCollection &collection);
	void setSectionName(const std::string &sectionName);

private:
	void startDocumentHandler();
	void endDocumentHandler();

private:
	shared_ptr<HtmlTagAction> createAction(const std::string &tag);

private:
	shared_ptr<CHMFileInfo> myInfo;
	CHMReferenceCollection &myReferenceCollection;
	std::string myCurrentSectionName;

friend class HtmlSectionHrefTagAction;
friend class HtmlSectionImageTagAction;
};

#endif /* __HTMLSECTIONREADER_H__ */

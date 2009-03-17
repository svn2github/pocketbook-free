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

#ifndef __COLLECTIONVIEW_H__
#define __COLLECTIONVIEW_H__

#include <map>

#include "FBView.h"
#include "../description/BookDescription.h"
#include "../collection/BookCollection.h"

class CollectionModel;

class CollectionView : public FBView {

public:
	CollectionView(FBReader &reader, shared_ptr<ZLPaintContext> context);
	~CollectionView();
	const std::string &caption() const;

	bool _onStylusPress(int x, int y);

	void paint();
	void updateModel();
	void synchronizeModel();

	void selectBook(BookDescriptionPtr book);

	BookCollection &collection();

private:
	CollectionModel &collectionModel();

private:
	BookCollection myCollection;
	bool myTreeStateIsFrozen;
	bool myUpdateModel;
};

inline BookCollection &CollectionView::collection() {
	return myCollection;
}

#endif /* __COLLECTIONVIEW_H__ */

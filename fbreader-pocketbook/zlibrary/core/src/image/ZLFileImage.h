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

#ifndef __ZLFILEIMAGE_H__
#define __ZLFILEIMAGE_H__

#include "ZLStreamImage.h"

class ZLFileImage : public ZLStreamImage {

public:
	ZLFileImage(const std::string &mimeType, const std::string &path, size_t offset, size_t size = 0);

private:
	shared_ptr<ZLInputStream> inputStream() const;

private:
	std::string myPath;
};

inline ZLFileImage::ZLFileImage(const std::string &mimeType, const std::string &path, size_t offset, size_t size) : ZLStreamImage(mimeType, offset, size), myPath(path) {}

#endif /* __ZLFILEIMAGE_H__ */

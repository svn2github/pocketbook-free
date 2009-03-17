/*
 * Copyright (C) 2008 Alexander Egorov <lunohod@gmx.de>
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

#ifndef __ZLNXFSMANAGER_H__
#define __ZLNXFSMANAGER_H__

#include "../../../../core/src/unix/filesystem/ZLUnixFSManager.h"

class ZLNXFSManager : public ZLUnixFSManager {

	public:
		static void createInstance() { ourInstance = new ZLNXFSManager(); }

	private:
		ZLNXFSManager() {}

	protected:
		std::string convertFilenameToUtf8(const std::string &name) const;
};

#endif /* __ZLNXFSMANAGER_H__ */

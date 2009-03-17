/*
 * Copyright (C) 2007-2008 Geometer Plus <contact@geometerplus.com>
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

#ifndef __ZLLANGUAGEOPTIONENTRY_H__
#define __ZLLANGUAGEOPTIONENTRY_H__

#include <map>

#include <ZLOptionEntry.h>

class ZLLanguageOptionEntry : public ZLComboOptionEntry {

public:
	ZLLanguageOptionEntry(ZLStringOption &languageOption, const std::vector<std::string> &languageCodes);

	const std::string &initialValue() const;
	const std::vector<std::string> &values() const;
	void onAccept(const std::string &value);

private:
	std::vector<std::string> myValues;
	std::map<std::string,std::string> myValuesToCodes;
	std::string myInitialValue;
	ZLStringOption &myLanguageOption;
};

#endif /* __ZLLANGUAGEOPTIONENTRY_H__ */

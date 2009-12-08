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

#ifndef __ZLWIN32MESSAGE_H__
#define __ZLWIN32MESSAGE_H__

#include <string>

#include <ZLUnicodeUtil.h>
#include <ZLMessage.h>

class ZLWin32CommunicationManager : public ZLCommunicationManager {

public:
	static void createInstance();

	shared_ptr<ZLMessageOutputChannel> createMessageOutputChannel(const std::string &protocol, const std::string &testFile);
	void addInputMessageDescription(const std::string&, const std::string&, const Data&) {}

private:
	ZLWin32CommunicationManager();
};

class ZLWin32ExecMessageOutputChannel : public ZLMessageOutputChannel {

public:
	shared_ptr<ZLMessageSender> createSender(const ZLCommunicationManager::Data &data);
};

class ZLWin32ExecMessageSender : public ZLMessageSender {

private:
	ZLWin32ExecMessageSender(const std::string &program);

public:
	void sendStringMessage(const std::string &message);

private:
	ZLUnicodeUtil::Ucs2String myProgram;

friend class ZLWin32ExecMessageOutputChannel;
};

#endif /* __ZLWIN32MESSAGE_H__ */

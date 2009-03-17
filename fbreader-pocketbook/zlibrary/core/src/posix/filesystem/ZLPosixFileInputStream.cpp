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

#include "ZLPosixFileInputStream.h"
#include <inkview.h>

ZLPosixFileInputStream::ZLPosixFileInputStream(const std::string &name) : myName(name) {
	myFile = 0;
}

ZLPosixFileInputStream::~ZLPosixFileInputStream() {
	close();
}

bool ZLPosixFileInputStream::open() {
	close();
	myFile = iv_fopen((char *)myName.c_str(), "rb");
	return myFile != 0;
}

size_t ZLPosixFileInputStream::read(char *buffer, size_t maxSize) {
	if (buffer != 0) {
		return iv_fread(buffer, 1, maxSize, myFile);
	} else {
		int pos = iv_ftell(myFile);
		iv_fseek(myFile, maxSize, SEEK_CUR);
		return iv_ftell(myFile) - pos;
	}
}

void ZLPosixFileInputStream::close() {
	if (myFile != 0) {
		iv_fclose(myFile);
		myFile = 0;
	}
}

size_t ZLPosixFileInputStream::sizeOfOpened() {
	if (myFile == 0) {
		return 0;
	}
	long pos = iv_ftell(myFile);
	iv_fseek(myFile, 0, SEEK_END);
	long size = iv_ftell(myFile);
	iv_fseek(myFile, pos, SEEK_SET);
	return size;
}

void ZLPosixFileInputStream::seek(int offset, bool absoluteOffset) {
	iv_fseek(myFile, offset, absoluteOffset ? SEEK_SET : SEEK_CUR);
}

size_t ZLPosixFileInputStream::offset() const {
	return iv_ftell(myFile);
}

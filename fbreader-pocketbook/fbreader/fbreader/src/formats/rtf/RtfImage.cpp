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

#include <ZLStringUtil.h>
#include <ZLInputStream.h>
#include <ZLFile.h>
#include <stdio.h>

#include "RtfImage.h"

static void append_hex(shared_ptr<std::string> s, char *buf, int len) {

	int i, j;
	char c;

	len &= ~1;
	if (len == 0) return;
	for (i=j=0; i<len; i+=2) {
		c = buf[i];
		if (c >= 'a') c -= 0x20;
		c = (c >= 'A') ? c-0x37 : c-0x30;
		buf[j] = ((c & 0xf) << 4);
		c = buf[i+1];
		if (c >= 'a') c -= 0x20;
		c = (c >= 'A') ? c-0x37 : c-0x30;
		buf[j] |= (c & 0xf);
		j++;
	}
	s->append(buf, j);

}

void RtfImage::read() const {
	shared_ptr<ZLInputStream> stream = ZLFile(myFileName).inputStream();
	char tbuf[256];
	int tpos=0;
	if (!stream.isNull() && stream->open()) {
		myData = new std::string();
		myData->reserve(myLength / 2);
		stream->seek(myStartOffset, false);
		const size_t bufferSize = 1024;
		char *buffer = new char[bufferSize];
		for (unsigned int i = 0; i < myLength; i += bufferSize) {
			size_t toRead = std::min(bufferSize, myLength - i);
			if (stream->read(buffer, toRead) != toRead) {
				break;
			}
			for (size_t j = 0; j < toRead; j ++) {
				char c = buffer[j];
				if (! isxdigit(c)) continue;
				tbuf[tpos++] = c;
				if (tpos == 256) {
					append_hex(myData, tbuf, tpos);
					tpos = 0;
				}
			}
		}
		append_hex(myData, tbuf, tpos);
		delete[] buffer;
		stream->close();
	}
}

const shared_ptr<std::string> RtfImage::stringData() const {
	if (myData.isNull()) {
		read();
	}
	return myData;
}

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

#include <ZLFile.h>
#include <ZLResource.h>

#include "MobipocketStream.h"

// Denis//
int PrcRecordNumber;
int PrcmyMaxRecordIndex;
// Denis//

MobipocketStream::MobipocketStream(ZLFile &file) : PalmDocStream(file), myErrorCode(ERROR_NONE) {
}

bool MobipocketStream::open() {

	myErrorCode = ERROR_NONE;

	if (!PdbStream::open()) {
		myErrorCode = ERROR_UNKNOWN;
		return false;
	}

	myBaseSize = myBase->sizeOfOpened();

	unsigned short version;
	PdbUtil::readUnsignedShort(*myBase, version);
	if ((version != 1) && (version != 2) && (version != 17480)) {
		myErrorCode = ERROR_COMPRESSION;
		return false;
	}
	myCompressionMethod = version;
	myBase->seek(6, false);
	unsigned short records;
	PdbUtil::readUnsignedShort(*myBase, records);
	myMaxRecordIndex = std::min(records, (unsigned short)(myHeader.Offsets.size() - 1));
	PdbUtil::readUnsignedShort(*myBase, myMaxRecordSize);
	if (myMaxRecordSize == 0) {
		myErrorCode = ERROR_UNKNOWN;
		return false;
	}
	unsigned short reserved2;
	PdbUtil::readUnsignedShort(*myBase, reserved2);
	myBuffer = new char[myMaxRecordSize];

	if (version == 17480) {
		myBase->seek(0x62, false);
		PdbUtil::readUnsignedLong(*myBase, myHuffOffset);
		PdbUtil::readUnsignedLong(*myBase, myHuffNumber);
	}
// Denis //
	myRecordIndex = 0;
	if (PrcRecordNumber == 0){
		PrcRecordNumber = myHeader.Offsets.size() - 2;
		PrcmyMaxRecordIndex = myMaxRecordIndex;
		int count = 0;
		while (PrcRecordNumber > 0 && count < 2){
			if (myHeader.Offsets[PrcRecordNumber] - myHeader.Offsets[PrcRecordNumber - 1] < 10){
				PrcRecordNumber--;
				continue;
			}
			PrcRecordNumber--;
			myBase->seek(myHeader.Offsets[PrcRecordNumber], true);
			char tmp[10];
			myBase->read(tmp, 10);
			if ((strncmp(tmp, "BM", 2) == 0) ||
				(strncmp(tmp, "GIF8", 4) == 0) ||
				(strncmp(tmp + 6, "JFIF", 4) == 0)) {
				count++;
			}
		}
	}
// Denis //
	
	return true;
}

bool MobipocketStream::hasExtraSections() const {
	return myMaxRecordIndex < myHeader.Offsets.size() - 1;
}

std::pair<int,int> MobipocketStream::imageLocation(int index) {

	index += myMaxRecordIndex + 1;
	int recordNumber = myHeader.Offsets.size();
	if (index > recordNumber - 1) {
		return std::pair<int,int>(-1, -1);
	} else {
		int start = myHeader.Offsets[index];
		int end = (index < recordNumber - 1) ?
			myHeader.Offsets[index + 1] : myBaseSize;
		return std::pair<int,int>(start, end - start);
	}
}

const std::string &MobipocketStream::error() const {
	static const ZLResource &resource = ZLResource::resource("mobipocketPlugin");
	switch (myErrorCode) {
		default:
		{
			static const std::string EMPTY;
			return EMPTY;
		}
		case ERROR_UNKNOWN:
			return resource["unknown"].value();
		case ERROR_COMPRESSION:
			return resource["unsupportedCompressionMethod"].value();
		case ERROR_ENCRIPTION:
			return resource["encriptedFile"].value();
	}
}

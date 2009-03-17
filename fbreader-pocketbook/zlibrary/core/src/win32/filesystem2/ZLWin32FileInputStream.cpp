
#include "ZLWin32FileInputStream.h"
#include "ZLWin32FSManager.h"
#include "../w32widgets/W32WCHARUtil.h"

ZLWin32FileInputStream::ZLWin32FileInputStream(const std::string &name) {
	myName = "\\\\?\\" + name;
	myFile = INVALID_HANDLE_VALUE;
}

ZLWin32FileInputStream::~ZLWin32FileInputStream() {
	close();
}

#include <iostream>

bool ZLWin32FileInputStream::open() {
	close();
	ZLUnicodeUtil::Ucs2String buffer;
	std::cerr << myName << "\n";
	myFile = CreateFile(::wchar(::createNTWCHARString(buffer, myName)), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	return myFile != INVALID_HANDLE_VALUE;
}

size_t ZLWin32FileInputStream::read(char *buffer, size_t maxSize) {
	if (buffer != 0) {
		DWORD len;
		ReadFile(myFile, buffer, maxSize, &len, 0);
		return len;
	} else {
		DWORD oldPointer = SetFilePointer(myFile, 0, 0, FILE_CURRENT);
		DWORD newPointer = SetFilePointer(myFile, maxSize, 0, FILE_CURRENT);
		if (newPointer == INVALID_SET_FILE_POINTER) {
			return 0;
		}
		return newPointer - oldPointer;
	}
}

void ZLWin32FileInputStream::close() {
	if (myFile != INVALID_HANDLE_VALUE) {
		CloseHandle(myFile);
		myFile = INVALID_HANDLE_VALUE;
	}
}

size_t ZLWin32FileInputStream::sizeOfOpened() {
	if (myFile == INVALID_HANDLE_VALUE) {
		return 0;
	}
	DWORD size;
	GetFileSize(myFile, &size);
	return size;
}

void ZLWin32FileInputStream::seek(int offset, bool absoluteOffset) {
	SetFilePointer(myFile, offset, 0, absoluteOffset ? FILE_BEGIN : FILE_CURRENT);
}

size_t ZLWin32FileInputStream::offset() const {
	return SetFilePointer(myFile, 0, 0, FILE_CURRENT);
}

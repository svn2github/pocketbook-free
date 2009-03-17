
#include <windows.h>

#include "ZLWin32RootDir.h"

void ZLWin32RootDir::collectSubDirs(std::vector<std::string> &names, bool) {
	char *buffer = new char[26 * 4];
	DWORD length = GetLogicalDriveStringsA(26 * 4, buffer);
	for (char *ptr = buffer; ptr < buffer + length; ) {
		std::string drive = ptr;
		names.push_back(drive);
		ptr += drive.length() + 1;
	}
	delete[] buffer;
}

void ZLWin32RootDir::collectFiles(std::vector<std::string>&, bool) {
}

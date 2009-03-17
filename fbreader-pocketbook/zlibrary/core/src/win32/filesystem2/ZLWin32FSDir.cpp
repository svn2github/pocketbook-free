
#include <algorithm>

#include <windows.h>

#include "ZLWin32FSDir.h"

void ZLWin32FSDir::collectNames(std::vector<std::string> &names, bool filesNotDirectories) {
	const std::string pattern = path() + "\\*";
	WIN32_FIND_DATAA fileData;
	std::string fileName;

	HANDLE hFind = FindFirstFileA(pattern.c_str(), &fileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}
	if (((fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) == filesNotDirectories) {
		fileName = fileData.cFileName;
		if (filesNotDirectories || ((fileName != ".") && (fileName != ".."))) {
			names.push_back(fileName);
		}
	}

	while (FindNextFileA(hFind, &fileData) != 0) {
		if (((fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) == filesNotDirectories) {
			fileName = fileData.cFileName;
			if (filesNotDirectories || ((fileName != ".") && (fileName != ".."))) {
				names.push_back(fileName);
			}
		}
	}
	FindClose(hFind);
	std::sort(names.begin(), names.end());
}

void ZLWin32FSDir::collectSubDirs(std::vector<std::string> &names, bool) {
	collectNames(names, false);
}

void ZLWin32FSDir::collectFiles(std::vector<std::string> &names, bool) {
	collectNames(names, true);
}

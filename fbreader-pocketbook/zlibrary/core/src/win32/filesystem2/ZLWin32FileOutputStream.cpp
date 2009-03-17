
#include "ZLWin32FileOutputStream.h"
#include "ZLWin32FSManager.h"

ZLWin32FileOutputStream::ZLWin32FileOutputStream(const std::string &name) : myHasErrors(false) {
	myName = ZLFile::fileNameToUtf8(name);
	myFile = INVALID_HANDLE_VALUE;
}

ZLWin32FileOutputStream::~ZLWin32FileOutputStream() {
	close();
}

bool ZLWin32FileOutputStream::open() {
	close();

	std::string dirName = ((ZLWin32FSManager&)ZLWin32FSManager::instance()).parentPath(myName);
	myTemporaryName.erase();
	myTemporaryName.append(MAX_PATH, '\0');
	GetTempFileNameA(dirName.c_str(), "XXX", 0, (CHAR*)myTemporaryName.data());
	myTemporaryName.erase(myTemporaryName.find('\0'));

	myFile = CreateFileA(myTemporaryName.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	return myFile != INVALID_HANDLE_VALUE;
}

void ZLWin32FileOutputStream::write(const std::string &str) {
	DWORD size = str.length();
	DWORD realSize;
	if ((WriteFile(myFile, str.data(), size, &realSize, 0) == 0) || (realSize != size)) {
		myHasErrors = true;
	}
}

void ZLWin32FileOutputStream::close() {
	if (myFile != INVALID_HANDLE_VALUE) {
		CloseHandle(myFile);
		myFile = INVALID_HANDLE_VALUE;
		if (!myHasErrors) {
			MoveFileExA(myTemporaryName.c_str(), myName.c_str(), MOVEFILE_REPLACE_EXISTING);
		}
	}
}

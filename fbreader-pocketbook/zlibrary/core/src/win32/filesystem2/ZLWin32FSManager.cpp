
#include <iostream>

#include <windows.h>

#include <ZLibrary.h>
#include <ZLStringUtil.h>

#include "ZLWin32FSManager.h"
#include "ZLWin32FSDir.h"
#include "ZLWin32RootDir.h"
#include "ZLWin32FileInputStream.h"
#include "ZLWin32FileOutputStream.h"
#include "../w32widgets/W32WCHARUtil.h"

ZLFSDir *ZLWin32FSManager::createPlainDirectory(const std::string &path) const {
	if (path.empty()) {
		return new ZLWin32RootDir();
	} else {
		return new ZLWin32FSDir(path);
	}
}

ZLInputStream *ZLWin32FSManager::createPlainInputStream(const std::string &path) const {
	return new ZLWin32FileInputStream(path);
}

ZLOutputStream *ZLWin32FSManager::createOutputStream(const std::string &path) const {
	return new ZLWin32FileOutputStream(path);
}

ZLFileInfo ZLWin32FSManager::fileInfo(const std::string &path) const {
	ZLFileInfo info;
	if (path.empty()) {
		info.Exists = true;
		info.Size = 0;
		info.MTime = 0;
		info.IsDirectory = true;
	} else {
		WIN32_FILE_ATTRIBUTE_DATA data;
		ZLUnicodeUtil::Ucs2String buffer;
		info.Exists = GetFileAttributesEx(::wchar(::createNTWCHARString(buffer, path)), GetFileExInfoStandard, &data) != 0;
		if (info.Exists) {
			info.Size = data.nFileSizeLow;
			info.MTime = data.ftLastWriteTime.dwLowDateTime;
			info.IsDirectory = data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
		}
	}
	return info;
}

bool ZLWin32FSManager::removeFile(const std::string &path) const {
	ZLUnicodeUtil::Ucs2String buffer;
	return DeleteFile(::wchar(::createNTWCHARString(buffer, path)));
}

int ZLWin32FSManager::findLastFileNameDelimiter(const std::string &path) const {
	int index = findArchiveFileNameDelimiter(path);
	if (index == -1) {
		index = path.rfind(ZLibrary::FileNameDelimiter);
	}
	return index;
}

void ZLWin32FSManager::normalize(std::string &path) const {
	std::cerr << "path before = " << path << "\n";
	if (path.empty()) {
		return;
	}

	static std::string HomeDir(2048, '\0');
	if (HomeDir.length() == 2048) {
		HomeDir.erase(GetEnvironmentVariableA("USERPROFILE", (CHAR*)HomeDir.data(), 2047));
	}

	static std::string PwdDir;
	if (PwdDir.empty()) {
		DWORD len = GetCurrentDirectoryW(0, 0);
		ZLUnicodeUtil::Ucs2String buffer(len, 0);
		GetCurrentDirectoryW(len, (WCHAR*)::wchar(buffer));
		// TODO: GetFullPathName
		DWORD strLen = WideCharToMultiByte(GetACP(), 0, ::wchar(buffer), len - 1, 0, 0, 0, 0);
		PwdDir.append(strLen + 1, '\0');
		WideCharToMultiByte(GetACP(), 0, ::wchar(buffer), len - 1, (CHAR*)PwdDir.data(), strLen, 0, 0);
		PwdDir.erase(strLen);
	}

	static std::string APPattern = "%APPLICATION_PATH%";
	static std::string AP;
	if (AP.empty()) {
		AP = _pgmptr;
		int index = AP.rfind('\\');
		if (index != -1) {
			AP = AP.substr(0, index);
		}
	}

	if (path[0] == '~') {
		path = HomeDir + path.substr(1);
	} else if (path.substr(0, APPattern.length()) == APPattern) {
		path = AP + path.substr(APPattern.length());
	} else if ((path.length() > 1) && (path[1] != ':')) {
		path = PwdDir + "\\" + path;
	}
	
	int index;
	while ((index = path.find("\\..")) != -1) {
		int prevIndex = path.rfind('\\', index - 1);
		if (prevIndex == -1) {
			break;
		}
		path.erase(prevIndex, index + 3 - prevIndex);
	}
	while ((index = path.find("\\\\")) != -1) {
		path.erase(index, 1);
	}

	if (!path.empty()) {
		path[0] = toupper(path[0]);
	}
	std::cerr << "path after = " << path << "\n";
}

ZLFSDir *ZLWin32FSManager::createNewDirectory(const std::string &path) const {
	ZLUnicodeUtil::Ucs2String buffer;
	return (CreateDirectory(::wchar(::createNTWCHARString(buffer, path)), 0) != 0) ? createPlainDirectory(path) : 0;
}

std::string ZLWin32FSManager::convertFilenameToUtf8(const std::string &name) const {
	int len = name.length();
	ZLUnicodeUtil::Ucs2String ucs2String;
	ucs2String.insert(ucs2String.end(), len, 0);
	int ucs2Len = MultiByteToWideChar(GetACP(), MB_PRECOMPOSED, name.c_str(), len, (WCHAR*)&ucs2String.front(), len);
	ucs2String.erase(ucs2String.begin() + ucs2Len, ucs2String.end());

	std::string utf8String;
	ZLUnicodeUtil::ucs2ToUtf8(utf8String, ucs2String);
	return utf8String;
}

int ZLWin32FSManager::findArchiveFileNameDelimiter(const std::string &path) const {
	int index = path.rfind(':');
	return (index == 1) ? -1 : index;
}

static const std::string RootPath = "";

shared_ptr<ZLDir> ZLWin32FSManager::rootDirectory() const {
	return new ZLWin32RootDir();
}

const std::string &ZLWin32FSManager::rootDirectoryPath() const {
	return RootPath;
}

std::string ZLWin32FSManager::parentPath(const std::string &path) const {
	if (path.length() <= 3) {
		return RootPath;
	}
	int index = findLastFileNameDelimiter(path);
	std::string result = path.substr(0, index);
	return (result.length() == 2) ? result + '\\' : result;
}

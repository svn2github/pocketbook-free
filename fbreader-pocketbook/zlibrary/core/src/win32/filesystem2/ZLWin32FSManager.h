
#ifndef __ZLWIN32FSMANAGER_H__
#define __ZLWIN32FSMANAGER_H__

#include "../../abstract/filesystem/ZLFSManager.h"

class ZLWin32FSManager : public ZLFSManager {

public:
	static void createInstance() { ourInstance = new ZLWin32FSManager(); }
	
private:
	ZLWin32FSManager() {}
	
protected:
	ZLFSDir *createPlainDirectory(const std::string &path) const;
	ZLFSDir *createNewDirectory(const std::string &path) const;
	ZLInputStream *createPlainInputStream(const std::string &path) const;
	ZLOutputStream *createOutputStream(const std::string &path) const;
	ZLFileInfo fileInfo(const std::string &path) const;
	bool removeFile(const std::string &path) const;

	int findLastFileNameDelimiter(const std::string &path) const;

	void normalize(std::string &path) const;

	std::string convertFilenameToUtf8(const std::string &name) const;
	int findArchiveFileNameDelimiter(const std::string &path) const;
	shared_ptr<ZLDir> rootDirectory() const;
	const std::string &rootDirectoryPath() const;

public:
	std::string parentPath(const std::string &path) const;
};

#endif /* __ZLWIN32FSMANAGER_H__ */

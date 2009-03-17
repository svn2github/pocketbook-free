
#ifndef __ZLWIN32ROOTDIR_H__
#define __ZLWIN32ROOTDIR_H__

#include "../../abstract/filesystem/ZLFSDir.h"

class ZLWin32RootDir : public ZLFSDir {

public:
	ZLWin32RootDir() : ZLFSDir(std::string()) {}

	void collectSubDirs(std::vector<std::string> &names, bool includeSymlinks);
	void collectFiles(std::vector<std::string> &names, bool includeSymlinks);
};

#endif /* __ZLWIN32ROOTDIR_H__ */

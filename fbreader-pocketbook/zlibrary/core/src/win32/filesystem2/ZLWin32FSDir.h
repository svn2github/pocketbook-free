
#ifndef __ZLWIN32FSDIR_H__
#define __ZLWIN32FSDIR_H__

#include "../../abstract/filesystem/ZLFSDir.h"

class ZLWin32FSDir : public ZLFSDir {

public:
	ZLWin32FSDir(const std::string &name) : ZLFSDir(name) {}

	void collectSubDirs(std::vector<std::string> &names, bool includeSymlinks);
	void collectFiles(std::vector<std::string> &names, bool includeSymlinks);

private:
	void collectNames(std::vector<std::string> &names, bool filesNotDirectories);
};

#endif /* __ZLWIN32FSDIR_H__ */

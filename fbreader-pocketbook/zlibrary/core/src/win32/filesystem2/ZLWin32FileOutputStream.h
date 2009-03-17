
#ifndef __ZLWIN32FILEOUTPUTSTREAM_H__
#define __ZLWIN32FILEOUTPUTSTREAM_H__

#include <windows.h>

#include "../../abstract/filesystem/ZLOutputStream.h"

class ZLWin32FileOutputStream : public ZLOutputStream {

public:
	ZLWin32FileOutputStream(const std::string &name);
	~ZLWin32FileOutputStream();
	bool open();
	void write(const std::string &str);
	void close();

private:
	std::string myName;
	std::string myTemporaryName;
	bool myHasErrors;
	HANDLE myFile;
};

#endif /* __ZLWIN32FILEOUTPUTSTREAM_H__ */

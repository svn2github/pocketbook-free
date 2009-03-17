
#ifndef __ZLWIN32FILEINPUTSTREAM_H__
#define __ZLWIN32FILEINPUTSTREAM_H__

#include <windows.h>

#include <ZLInputStream.h>

class ZLWin32FileInputStream : public ZLInputStream {

public:
	ZLWin32FileInputStream(const std::string &name);
	~ZLWin32FileInputStream();
	bool open();
	size_t read(char *buffer, size_t maxSize);
	void close();

	void seek(int offset, bool absoluteOffset);
	size_t offset() const;
	size_t sizeOfOpened();

private:
	std::string myName;
	HANDLE myFile;
};

#endif /* __ZLWIN32FILEINPUTSTREAM_H__ */

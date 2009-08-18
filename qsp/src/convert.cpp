#include "convert.h"


void to_utf8(const unsigned char *from, char *to, const unsigned short *encoding)
{
	unsigned int i, j;
	for (i = 0, j = 0; i < strlen((const char *)from); i++)
	{
		unsigned short ch = from[i];
					
		if (ch >= 0200)
		{
			ch = encoding [ch - 0200];
		}

		if (ch < 0x80)
		{
			to[j++] = ch;
		}
		else if (ch < 0x800)
		{
			to[j++] = (ch >> 6 | 0xc0);
			to[j++] = ((ch & 0x3f) | 0x80);
		}
		else
		{
			to[j++] = (ch >> 12 | 0xe0);
			to[j++] = (((ch >> 6) & 0x3f) | 0x80);
			to[j++] = ((ch & 0x3f) | 0x80);
		}
		
	}
	
	to[j] = '\0';
}

void to_utf8(const unsigned char *from, std::string *to, const unsigned short *encoding)
{
	char* buf = new char[strlen((const char*)from)*3+1];
	to_utf8(from, buf, encoding);
	
	to->assign(buf);	
	delete[] buf;
}


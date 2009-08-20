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

//static wchar_t qspDirectConvertUC(char ch, const unsigned short *encoding)
//{
//	unsigned char ch2 = (unsigned char)ch;
//	return (ch2 >= 0x80 ? table[ch2 - 0x80] : ch);
//}

char utf8_char_to(unsigned short ch, const unsigned short *encoding)
{
	long i;
	if (ch < 0x80) return (char)ch;
	for (i = 127; i >= 0; --i)
		if (encoding[i] == ch) return (char)(i + 0x80);
	return 0x20;
}

std::string utf8_to(const unsigned short *from, const unsigned short *encoding)
{
	std::string to;
	
	for (int i = 0; i < strlen((const char *)from); i++)
	{
		wchar_t ch = from[i];
		to.append(1, utf8_char_to(ch, encoding));
	}
	
	return to;
}


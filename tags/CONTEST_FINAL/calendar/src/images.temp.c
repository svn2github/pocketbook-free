typedef struct ibitmap_s {
	short width;
	short height;
	short depth;
	short scanline;
	unsigned char data[];
} ibitmap;

const ibitmap icon_recurrent = {
	16, 16, 2, 4,
	{
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x90,0x06,0xff,0xfe,0x01,0x40,0x7f,
		0xef,0xbf,0xf9,0x1f,0xdb,0xff,0xff,0x4f,0xc3,0xff,0xff,0x8a,0x81,0xff,0xfe,0x01,
		0x40,0xbf,0xff,0x42,0xa2,0xff,0xff,0xc3,0xf1,0xff,0xff,0xe7,0xf4,0x6f,0xfe,0xfb,
		0xfd,0x01,0x40,0xbf,0xff,0x90,0x06,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	}
};


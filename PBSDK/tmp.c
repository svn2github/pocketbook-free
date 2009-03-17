typedef struct ibitmap_s {
	short width;
	short height;
	short depth;
	short scanline;
	unsigned char data[];
} ibitmap;

const ibitmap def_battery_0 = {
	32, 12, 2, 8,
	{
		0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,
		0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,
		0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x03,0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x03,
		0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x03,0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x03,
		0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,
		0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,
	}
};

const ibitmap def_battery_1 = {
	32, 12, 2, 8,
	{
		0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,
		0xcc,0x03,0xff,0xff,0xff,0xff,0xff,0x3f,0xcc,0x03,0xff,0xff,0xff,0xff,0xff,0x3f,
		0xcc,0x03,0xff,0xff,0xff,0xff,0xff,0x03,0xcc,0x03,0xff,0xff,0xff,0xff,0xff,0x03,
		0xcc,0x03,0xff,0xff,0xff,0xff,0xff,0x03,0xcc,0x03,0xff,0xff,0xff,0xff,0xff,0x03,
		0xcc,0x03,0xff,0xff,0xff,0xff,0xff,0x3f,0xcc,0x03,0xff,0xff,0xff,0xff,0xff,0x3f,
		0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,
	}
};

const ibitmap def_battery_2 = {
	32, 12, 2, 8,
	{
		0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,
		0xcc,0x03,0x00,0xff,0xff,0xff,0xff,0x3f,0xcc,0x03,0x00,0xff,0xff,0xff,0xff,0x3f,
		0xcc,0x03,0x00,0xff,0xff,0xff,0xff,0x03,0xcc,0x03,0x00,0xff,0xff,0xff,0xff,0x03,
		0xcc,0x03,0x00,0xff,0xff,0xff,0xff,0x03,0xcc,0x03,0x00,0xff,0xff,0xff,0xff,0x03,
		0xcc,0x03,0x00,0xff,0xff,0xff,0xff,0x3f,0xcc,0x03,0x00,0xff,0xff,0xff,0xff,0x3f,
		0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,
	}
};

const ibitmap def_battery_3 = {
	32, 12, 2, 8,
	{
		0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,
		0xcc,0x03,0x00,0xc0,0x3f,0xff,0xff,0x3f,0xcc,0x03,0x00,0xc0,0x3f,0xff,0xff,0x3f,
		0xcc,0x03,0x00,0xc0,0x3f,0xff,0xff,0x03,0xcc,0x03,0x00,0xc0,0x3f,0xff,0xff,0x03,
		0xcc,0x03,0x00,0xc0,0x3f,0xff,0xff,0x03,0xcc,0x03,0x00,0xc0,0x3f,0xff,0xff,0x03,
		0xcc,0x03,0x00,0xc0,0x3f,0xff,0xff,0x3f,0xcc,0x03,0x00,0xc0,0x3f,0xff,0xff,0x3f,
		0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,
	}
};

const ibitmap def_battery_4 = {
	32, 12, 2, 8,
	{
		0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,
		0xcc,0x03,0x00,0xc0,0x30,0x0f,0xff,0x3f,0xcc,0x03,0x00,0xc0,0x30,0x0f,0xff,0x3f,
		0xcc,0x03,0x00,0xc0,0x30,0x0f,0xff,0x03,0xcc,0x03,0x00,0xc0,0x30,0x0f,0xff,0x03,
		0xcc,0x03,0x00,0xc0,0x30,0x0f,0xff,0x03,0xcc,0x03,0x00,0xc0,0x30,0x0f,0xff,0x03,
		0xcc,0x03,0x00,0xc0,0x30,0x0f,0xff,0x3f,0xcc,0x03,0x00,0xc0,0x30,0x0f,0xff,0x3f,
		0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,
	}
};

const ibitmap def_battery_5 = {
	32, 12, 2, 8,
	{
		0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,
		0xcc,0x03,0x00,0xc0,0x30,0x0c,0x03,0x3f,0xcc,0x03,0x00,0xc0,0x30,0x0c,0x03,0x3f,
		0xcc,0x03,0x00,0xc0,0x30,0x0c,0x03,0x03,0xcc,0x03,0x00,0xc0,0x30,0x0c,0x03,0x03,
		0xcc,0x03,0x00,0xc0,0x30,0x0c,0x03,0x03,0xcc,0x03,0x00,0xc0,0x30,0x0c,0x03,0x03,
		0xcc,0x03,0x00,0xc0,0x30,0x0c,0x03,0x3f,0xcc,0x03,0x00,0xc0,0x30,0x0c,0x03,0x3f,
		0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,
	}
};

const ibitmap def_battery_c = {
	32, 12, 2, 8,
	{
		0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,
		0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,0xcf,0xff,0xff,0xf0,0x0f,0xff,0xff,0x3f,
		0xcf,0xff,0xfc,0x00,0x3f,0xff,0xff,0x03,0xcf,0xff,0xc0,0x00,0x00,0x00,0xff,0x03,
		0xcf,0xff,0xc0,0x00,0x00,0x00,0xff,0x03,0xcf,0xff,0xfc,0x00,0x3f,0xff,0xff,0x03,
		0xcf,0xff,0xff,0xf0,0x0f,0xff,0xff,0x3f,0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,
		0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,
	}
};

const ibitmap def_dicmenu = {
	170, 32, 2, 43,
	{
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf0,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xf0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xf0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf0,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xfc,0x00,0xcc,0xcc,0xcc,0xcc,0xcf,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xf0,0xff,0xff,0xc0,0x00,0x00,0x00,0x3f,0xff,0xff,
		0xfc,0x00,0xf3,0x33,0x33,0x33,0x33,0xff,0xff,0xff,0x03,0x00,0xc0,0x30,0x3f,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfc,0x3f,
		0xff,0xf0,0xff,0xff,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xfc,0x00,0xcc,0xcc,0xcc,
		0xcc,0xcf,0xff,0xff,0xff,0x03,0x00,0xc0,0x30,0x3f,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf0,0x3f,0xff,0xf0,0xff,0xfc,0x00,
		0x00,0x00,0x00,0x03,0xff,0xff,0xfc,0x00,0xf3,0x33,0x33,0x33,0x33,0xff,0xff,0xff,
		0x0f,0xff,0xff,0xfc,0x3f,0xff,0xff,0x00,0x00,0x30,0x00,0x03,0x00,0x00,0x3f,0xff,
		0xff,0xff,0xff,0xff,0xc0,0xff,0xff,0xf0,0xff,0xfc,0x03,0xff,0xff,0xfc,0x03,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0x3f,0xff,0x33,0xff,0xf3,0x3f,0xff,0x3f,0xff,0xff,0xff,0xff,0xff,0x03,
		0xf0,0xff,0xf0,0xff,0xfc,0x0f,0xff,0xff,0xff,0x03,0xff,0xff,0xfc,0x00,0xcc,0xcc,
		0xcc,0xcc,0xcf,0xff,0xff,0xff,0x0f,0xff,0xff,0xfc,0x3f,0xff,0xff,0x30,0xff,0x33,
		0x0f,0xf3,0x30,0xff,0x3f,0xff,0xff,0xff,0xff,0xfc,0x0f,0xc0,0xff,0xf0,0xff,0xfc,
		0x3f,0xff,0xff,0xff,0x03,0xff,0xff,0xfc,0x00,0xf3,0x33,0x33,0x33,0x33,0xff,0xff,
		0xff,0x03,0x00,0xc0,0x30,0x3f,0xff,0xff,0x30,0xff,0x33,0x0f,0xf3,0x30,0xff,0x3f,
		0xff,0xff,0xff,0xff,0xf0,0x3f,0x00,0xff,0xf0,0xff,0xff,0xff,0xff,0xff,0xff,0x03,
		0xff,0xff,0xfc,0x00,0xcc,0xcc,0xcc,0xcc,0xcf,0xff,0xff,0xff,0x03,0x00,0xc0,0x30,
		0x3f,0xff,0xff,0x3f,0xff,0x33,0xff,0xf3,0x3f,0xff,0x3f,0xff,0xff,0xf3,0xff,0xc0,
		0xfc,0x03,0xff,0xf0,0xff,0xff,0xf0,0xff,0xff,0xff,0x03,0xff,0xff,0xfc,0x00,0xf3,
		0x33,0x33,0x33,0x33,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,0xff,
		0x33,0xff,0xf3,0x3f,0xff,0x3f,0xff,0xff,0xf3,0xff,0x03,0xf0,0x03,0xff,0xf0,0xff,
		0xff,0xc0,0xff,0xff,0xff,0x03,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0x3f,0xff,0xff,0xff,0xff,0x3f,0xff,0x33,0xff,0xf3,0x3f,0xff,
		0x3f,0xff,0xff,0xc0,0xfc,0x0f,0xc0,0x03,0xff,0xf0,0xff,0xff,0x00,0x00,0x00,0x3f,
		0x03,0xff,0xff,0xfc,0x00,0xcc,0xcc,0xcc,0xcc,0xcf,0xff,0xff,0xff,0xff,0xf0,0x03,
		0xff,0xff,0xff,0xff,0x3f,0xff,0x33,0xff,0xf3,0x3f,0xff,0x3f,0xff,0xff,0xc0,0x00,
		0x3f,0x00,0x0f,0xff,0xf0,0xff,0xfc,0x00,0x00,0x00,0x3f,0x03,0xff,0xff,0xfc,0x00,
		0xf3,0x33,0x33,0x33,0x33,0xff,0xff,0xff,0xff,0x00,0x00,0x3f,0xff,0xff,0xff,0x00,
		0x00,0x30,0x00,0x03,0x00,0x00,0x3f,0xff,0xff,0x00,0x00,0xfc,0x00,0x0f,0xff,0xf0,
		0xff,0xfc,0x00,0x00,0x00,0x3f,0x03,0xff,0xff,0xfc,0x00,0xcc,0xcc,0xcc,0xcc,0xcf,
		0xff,0xff,0xff,0xfc,0x0c,0x0c,0x0f,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0x00,0x03,0xf0,0x00,0x0f,0xff,0xf0,0xff,0xff,0x00,0xff,0xff,
		0xff,0x03,0xff,0xff,0xfc,0x00,0xf3,0x33,0x33,0x33,0x33,0xff,0xff,0xff,0xff,0xfc,
		0x0f,0xff,0xff,0xff,0xff,0xff,0xc0,0x00,0x0c,0x00,0x00,0xff,0xff,0xff,0xff,0x00,
		0x0f,0xc0,0x00,0x3f,0xff,0xf0,0xff,0xff,0xc0,0xff,0xff,0xff,0x03,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfc,0x0f,0xff,0xff,0xff,0xff,
		0xff,0xcf,0xff,0xcc,0xff,0xfc,0xff,0xff,0xff,0xfc,0x00,0x3f,0x03,0xf0,0x3f,0xff,
		0xf0,0xff,0xff,0xf0,0xff,0xff,0xff,0x03,0xff,0xff,0xfc,0x00,0xcc,0xcc,0xcc,0xcc,
		0xcf,0xff,0xff,0xff,0xff,0xfc,0x0f,0xff,0xff,0xff,0xff,0xff,0xcc,0x3f,0xcc,0xc3,
		0xfc,0xff,0xff,0xff,0xfc,0x00,0xfc,0x0f,0xfc,0xff,0xff,0xf0,0xff,0xff,0xff,0xff,
		0xff,0xff,0x03,0xff,0xff,0xfc,0x00,0xf3,0x33,0x33,0x33,0x33,0xff,0xff,0xff,0xff,
		0xf0,0x03,0xff,0xff,0xff,0xff,0xff,0xcc,0x3f,0xcc,0xc3,0xfc,0xff,0xff,0xff,0xfc,
		0x03,0xf0,0x3f,0xfc,0xff,0xff,0xf0,0xff,0xfc,0x3f,0xff,0xff,0xff,0x03,0xff,0xff,
		0xfc,0x00,0xcc,0xcc,0xcc,0xcc,0xcf,0xff,0xff,0xff,0xff,0xf0,0x03,0xff,0xff,0xff,
		0xff,0xff,0xcf,0xff,0xcc,0xff,0xfc,0xff,0xff,0xff,0xf0,0x0f,0xc0,0xff,0xff,0xff,
		0xff,0xf0,0xff,0xfc,0x0f,0xff,0xff,0xff,0x03,0xff,0xff,0xfc,0x00,0xf3,0x33,0x33,
		0x33,0x33,0xff,0xff,0xff,0xff,0xf0,0x03,0xff,0xff,0xff,0xff,0xff,0xcf,0xff,0xcc,
		0xff,0xfc,0xff,0xff,0xff,0xf0,0x3f,0x03,0xff,0xff,0xff,0xff,0xf0,0xff,0xfc,0x03,
		0xff,0xff,0xfc,0x03,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xc0,0x00,0xff,0xff,0xff,0xff,0xff,0xcf,0xff,0xcc,0xff,0xfc,0xff,0xff,0xff,
		0xf0,0xfc,0x0f,0xff,0xff,0xff,0xff,0xf0,0xff,0xfc,0x00,0x00,0x00,0x00,0x03,0xff,
		0xff,0xfc,0x00,0xcc,0xcc,0xcc,0xcc,0xcf,0xff,0xff,0xff,0xff,0xc0,0x00,0xff,0xff,
		0xff,0xff,0xff,0xcf,0xff,0xcc,0xff,0xfc,0xff,0xff,0xff,0xff,0xf0,0x3f,0xff,0xff,
		0xff,0xff,0xf0,0xff,0xff,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xfc,0x00,0xf3,0x33,
		0x33,0x33,0x33,0xff,0xff,0xff,0xff,0x00,0x00,0x3f,0xff,0xff,0xff,0xff,0xc0,0x00,
		0x0c,0x00,0x00,0xff,0xff,0xff,0xff,0xc0,0xff,0xff,0xff,0xff,0xff,0xf0,0xff,0xff,
		0xc0,0x00,0x00,0x00,0x3f,0xff,0xff,0xfc,0x00,0xcc,0xcc,0xcc,0xcc,0xcf,0xff,0xff,
		0xff,0xff,0x00,0x00,0x3f,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xc3,0xff,0xff,0xff,0xff,0xff,0xf0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xfc,0x00,0xf3,0x33,0x33,0x33,0x33,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xf0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf0,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf0,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xf0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf0,
	}
};

const ibitmap def_fileicon = {
	16, 20, 2, 4,
	{
		0x00,0x00,0x03,0xff,0x3f,0xff,0xfc,0xff,0x3f,0xff,0xfc,0x3f,0x3f,0xff,0xfc,0x0f,
		0x3f,0xff,0xff,0xf3,0x3f,0xff,0xff,0xf3,0x3f,0xff,0xff,0xf3,0x3f,0xff,0xff,0xf3,
		0x3f,0xff,0xff,0xf3,0x3f,0xff,0xff,0xf3,0x3f,0xff,0xff,0xf3,0x3f,0xff,0xff,0xf3,
		0x3f,0xff,0xff,0xf3,0x3f,0xff,0xff,0xf3,0x3f,0xff,0xff,0xf3,0x3f,0xff,0xff,0xf3,
		0x3f,0xff,0xff,0xf3,0x3f,0xff,0xff,0xf3,0x00,0x00,0x00,0x03,0xff,0xff,0xff,0xff,
	}
};

const ibitmap def_hourglass = {
	32, 32, 32770, 8,
	{
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0xff,0xff,0xff,0xff,0xff,0xff,0xfc,
		0x3f,0xff,0xff,0xff,0xff,0xff,0xff,0xfc,0x3f,0xff,0xc0,0x00,0x00,0x00,0xff,0xfc,
		0x3f,0xff,0xc0,0x00,0x00,0x00,0xff,0xfc,0x3f,0xff,0xc3,0xff,0xff,0xf0,0xff,0xfc,
		0x3f,0xff,0xc0,0x00,0x00,0x00,0xff,0xfc,0x3f,0xff,0xf3,0xff,0xff,0xf3,0xff,0xfc,
		0x3f,0xff,0xf3,0xff,0xff,0xf3,0xff,0xfc,0x3f,0xff,0xf3,0xff,0xff,0xf3,0xff,0xfc,
		0x3f,0xff,0xf0,0xf3,0x33,0xc3,0xff,0xfc,0x3f,0xff,0xfc,0x3c,0xcf,0x0f,0xff,0xfc,
		0x3f,0xff,0xff,0x0f,0x3c,0x3f,0xff,0xfc,0x3f,0xff,0xff,0xc3,0xf0,0xff,0xff,0xfc,
		0x3f,0xff,0xff,0xf0,0xc3,0xff,0xff,0xfc,0x3f,0xff,0xff,0xf0,0xc3,0xff,0xff,0xfc,
		0x3f,0xff,0xff,0xf0,0xc3,0xff,0xff,0xfc,0x3f,0xff,0xff,0xf0,0xc3,0xff,0xff,0xfc,
		0x3f,0xff,0xff,0xc3,0xf0,0xff,0xff,0xfc,0x3f,0xff,0xff,0x0f,0x3c,0x3f,0xff,0xfc,
		0x3f,0xff,0xfc,0x3f,0xff,0x0f,0xff,0xfc,0x3f,0xff,0xf0,0xff,0x3f,0xc3,0xff,0xfc,
		0x3f,0xff,0xf3,0xfc,0xcf,0xf3,0xff,0xfc,0x3f,0xff,0xf3,0xf3,0x33,0xf3,0xff,0xfc,
		0x3f,0xff,0xf3,0xcc,0xcc,0xf3,0xff,0xfc,0x3f,0xff,0xc0,0x00,0x00,0x00,0xff,0xfc,
		0x3f,0xff,0xc3,0xff,0xff,0xf0,0xff,0xfc,0x3f,0xff,0xc0,0x00,0x00,0x00,0xff,0xfc,
		0x3f,0xff,0xc0,0x00,0x00,0x00,0xff,0xfc,0x3f,0xff,0xff,0xff,0xff,0xff,0xff,0xfc,
		0x3f,0xff,0xff,0xff,0xff,0xff,0xff,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	}
};

const ibitmap def_leaf_closed = {
	16, 12, 2, 4,
	{
		0x33,0x3c,0xcc,0xff,0xff,0xff,0xff,0xff,0x3f,0xff,0xfc,0xff,0xff,0xc3,0xff,0xff,
		0x3f,0xc3,0xfc,0xff,0xfc,0x00,0x3f,0xff,0xfc,0x00,0x3f,0xff,0x3f,0xc3,0xfc,0xff,
		0xff,0xc3,0xff,0xff,0x3f,0xff,0xfc,0xff,0xff,0xff,0xff,0xff,0x33,0x3c,0xcc,0xff,
	}
};

const ibitmap def_leaf_open = {
	16, 12, 2, 4,
	{
		0x33,0x3c,0xcc,0xff,0xff,0xff,0xff,0xff,0x3f,0xff,0xfc,0xff,0xff,0xff,0xff,0xff,
		0x3f,0xff,0xfc,0xff,0xfc,0x00,0x3f,0xff,0xfc,0x00,0x3f,0xff,0x3f,0xff,0xfc,0xff,
		0xff,0xff,0xff,0xff,0x3f,0xff,0xfc,0xff,0xff,0xff,0xff,0xff,0x33,0x3c,0xcc,0xff,
	}
};

const ibitmap def_playing = {
	29, 28, 32770, 8,
	{
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xfc,0x00,0x00,0x00,0x00,
		0x00,0x00,0x03,0xff,0xc0,0x00,0x00,0x00,0x00,0x00,0x03,0xff,0xfc,0x00,0x00,0x00,
		0x00,0x00,0x03,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x03,0xf0,0x3f,0xc0,0x00,0x00,
		0x00,0x00,0x03,0xf0,0x0f,0xc0,0x00,0x00,0x00,0x00,0x03,0xf0,0x0f,0xc0,0x00,0x00,
		0x00,0x00,0x03,0xf0,0x0f,0xc0,0x00,0x00,0x00,0x00,0x03,0xf0,0x0f,0x00,0x00,0x00,
		0x00,0x00,0x03,0xf0,0x0f,0x00,0x00,0x00,0x00,0x00,0x03,0xf0,0x3c,0x00,0x00,0x00,
		0x00,0x00,0x03,0xf0,0x30,0x00,0x00,0x00,0x00,0x00,0x03,0xf0,0xc0,0x00,0x00,0x00,
		0x00,0x03,0xff,0xf0,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xf0,0x00,0x00,0x00,0x00,
		0x03,0xff,0xff,0xf0,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xf0,0x00,0x00,0x00,0x00,
		0x0f,0xff,0xff,0xf0,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xc0,0x00,0x00,0x00,0x00,
		0x03,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0xf0,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x07,
		0x00,0x00,0x00,0x07,0x00,0x1e,0x00,0x07,0x00,0x1f,0x80,0x07,0x00,0x1f,0xe0,0x07,
		0x00,0x1f,0xf0,0x07,0x00,0x1c,0x78,0x07,0x00,0x1c,0x38,0x07,0x00,0x1c,0x38,0x07,
		0x00,0x1c,0x38,0x07,0x00,0x1c,0x30,0x07,0x00,0x1c,0x30,0x07,0x00,0x1c,0x60,0x07,
		0x00,0x1c,0x40,0x07,0x00,0x1c,0x80,0x07,0x01,0xfc,0x00,0x07,0x0f,0xfc,0x00,0x07,
		0x1f,0xfc,0x00,0x07,0x3f,0xfc,0x00,0x07,0x3f,0xfc,0x00,0x07,0x3f,0xf8,0x00,0x07,
		0x1f,0xf0,0x00,0x07,0x07,0xc0,0x00,0x07,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x07,
	}
};

const ibitmap def_usbex = {
	32, 31, 2, 8,
	{
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xaf,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0x9b,0xff,0xff,0xff,0xff,0xff,0xaa,0xaa,0x96,0xbf,0xff,
		0xff,0xff,0xfa,0x55,0x55,0x55,0x6f,0xff,0xff,0xff,0xa5,0x55,0x55,0x55,0x5f,0xff,
		0xff,0xfe,0x55,0x55,0xaa,0x55,0xbf,0xff,0xff,0xf9,0x55,0x6b,0xff,0x9a,0xff,0xff,
		0xff,0xe5,0x55,0xbf,0xff,0xaf,0xff,0xff,0xff,0x95,0x56,0xff,0xff,0xff,0xff,0xff,
		0xff,0x95,0x5b,0xff,0xff,0xff,0xfa,0xaf,0xfe,0x55,0x6f,0xff,0xff,0xff,0x95,0x6f,
		0xfd,0x55,0xbf,0xff,0xff,0xff,0x55,0x6f,0xf9,0x55,0xbf,0xff,0xff,0xff,0x55,0x6f,
		0xf9,0x55,0xff,0xff,0xff,0xff,0x55,0x6f,0xf5,0x56,0xff,0xff,0xff,0xfe,0x55,0x6f,
		0xe5,0x56,0xff,0xff,0xff,0xfe,0x55,0x6f,0xe5,0x56,0xff,0xff,0xff,0xfe,0x55,0x7f,
		0xe5,0x56,0xff,0xff,0xff,0xf9,0x55,0xbf,0xe5,0x56,0xff,0xff,0xff,0xf9,0x55,0xbf,
		0xfa,0xab,0xff,0xff,0xff,0xe5,0x56,0xff,0xff,0xff,0xff,0xff,0xff,0x95,0x5b,0xff,
		0xff,0xff,0xfa,0xff,0xfe,0x55,0x5b,0xff,0xff,0xff,0xa6,0xff,0xea,0x65,0x6f,0xff,
		0xff,0xfe,0xaa,0xaa,0xa5,0x9a,0xbf,0xff,0xff,0xea,0x66,0xaa,0x9a,0x6a,0xff,0xff,
		0xff,0xea,0xaa,0x69,0xaa,0xaf,0xff,0xff,0xff,0xfe,0xaa,0xaa,0xaa,0xff,0xff,0xff,
		0xff,0xff,0xaa,0xfa,0xff,0xff,0xff,0xff,0xff,0xff,0xea,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	}
};

const ibitmap keyboard_arrow = {
	16, 13, 2, 4,
	{
		0xff,0xff,0xff,0xff,0xff,0xfc,0x3f,0xff,0xff,0xf0,0x0f,0xff,0xff,0xc3,0xc3,0xff,
		0xff,0x0f,0xf0,0xff,0xfc,0x3f,0xfc,0x3f,0xf0,0xff,0xff,0x0f,0xc3,0xff,0xff,0xc3,
		0xc0,0x3f,0xfc,0x03,0xfc,0x3f,0xfc,0x3f,0xfc,0x3f,0xfc,0x3f,0xfc,0x00,0x00,0x3f,
		0xff,0xff,0xff,0xff,
	}
};

const ibitmap keyboard_bs = {
	12, 12, 2, 3,
	{
		0xff,0xff,0xff,0xff,0xcf,0xff,0xff,0x0f,0xff,0xfc,0x0f,0xff,0xf0,0x00,0x03,0xc0,
		0x00,0x03,0x00,0x00,0x03,0xc0,0x00,0x03,0xf0,0x00,0x03,0xfc,0x0f,0xff,0xff,0x0f,
		0xff,0xff,0xcf,0xff,
	}
};

const ibitmap ok_11 = {
	11, 11, 32770, 3,
	{
		0x00,0x00,0x00,0x3f,0x0f,0x3c,0xff,0xcf,0x3c,0xf3,0xcf,0x3c,0xf3,0xcf,0xf0,0xf3,
		0xcf,0xc0,0xf3,0xcf,0xf0,0xf3,0xcf,0x3c,0xff,0xcf,0x3c,0x3f,0x0f,0x3c,0x00,0x00,
		0x00,0x00,0x1f,0x73,0x7f,0xfb,0x7f,0xdb,0x7f,0xdb,0xdf,0xdb,0x9f,0xdb,0xdf,0xdb,
		0x7f,0xfb,0x7f,0x73,0x7f,0x00,0x1f,
	}
};

const ibitmap ok_13 = {
	13, 13, 32770, 4,
	{
		0x00,0x00,0x00,0x00,0x3f,0xc3,0xc3,0xc0,0xff,0xf3,0xc3,0xc0,0xf0,0xf3,0xc3,0xc0,
		0xf0,0xf3,0xcf,0x00,0xf0,0xf3,0xfc,0x00,0xf0,0xf3,0xfc,0x00,0xf0,0xf3,0xfc,0x00,
		0xf0,0xf3,0xcf,0x00,0xf0,0xf3,0xc3,0xc0,0xff,0xf3,0xc3,0xc0,0x3f,0xc3,0xc3,0xc0,
		0x00,0x00,0x00,0x00,0x00,0x07,0x79,0x9f,0xfd,0x9f,0xcd,0x9f,0xcd,0xb7,0xcd,0xe7,
		0xcd,0xe7,0xcd,0xe7,0xcd,0xb7,0xcd,0x9f,0xfd,0x9f,0x79,0x9f,0x00,0x07,
	}
};

const ibitmap ok_15 = {
	15, 15, 32770, 4,
	{
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xf0,0xf0,0xf0,0x3f,0xfc,0xf0,0xf0,
		0x3c,0x3c,0xf0,0xf0,0x3c,0x3c,0xf3,0xc0,0x3c,0x3c,0xff,0x00,0x3c,0x3c,0xff,0x00,
		0x3c,0x3c,0xff,0x00,0x3c,0x3c,0xf3,0xc0,0x3c,0x3c,0xf0,0xf0,0x3f,0xfc,0xf0,0xf0,
		0x0f,0xf0,0xf0,0xf0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x01,
		0x3c,0xcd,0x7e,0xcd,0x66,0xcd,0x66,0xd9,0x66,0xf1,0x66,0xf1,0x66,0xf1,0x66,0xd9,
		0x66,0xcd,0x7e,0xcd,0x3c,0xcd,0x00,0x01,0x00,0x01,
	}
};

const ibitmap ok_7 = {
	7, 7, 32770, 2,
	{
		0x30,0xcc,0xfc,0xcc,0xcc,0xf0,0xcc,0xf0,0xcc,0xf0,0xfc,0xcc,0x30,0xcc,0x4b,0xeb,
		0xad,0xad,0xad,0xeb,0x4b,
	}
};

const ibitmap ok_9 = {
	9, 9, 32770, 3,
	{
		0x00,0x00,0x00,0x3c,0x30,0xc0,0xff,0x33,0xc0,0xc3,0x3f,0x00,0xc3,0x3c,0x00,0xc3,
		0x3f,0x00,0xff,0x33,0xc0,0x3c,0x30,0xc0,0x00,0x00,0x00,0x00,0x7f,0x64,0xff,0xf5,
		0xff,0x97,0x7f,0x96,0x7f,0x97,0x7f,0xf5,0xff,0x64,0xff,0x00,0x7f,
	}
};

const ibitmap selarrows = {
	9, 16, 32770, 3,
	{
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x08,0x7f,0x1c,0x7f,0x3e,0x7f,0x7f,0x7f,0xff,0xff,0xff,0xff,0x00,0x7f,0x00,0x7f,
		0x00,0x7f,0x00,0x7f,0xff,0xff,0xff,0xff,0x7f,0x7f,0x3e,0x7f,0x1c,0x7f,0x08,0x7f,
	}
};


#ifndef _citychoose_h_
#define	_citychoose_h_

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <clocale> 
#include <list>
#include <string>
#include <math.h>

#include <algorithm>
#include <vector>
#include <locale>

#include "inkview.h"
#include "SimpleIni.h"
#include "mytext.h"
#include "define.h"
#include "draw.h"

//#include "iniparser.h"


using namespace std;

typedef struct city_s {
	char path[MAPPATH_SIZE];
	char name[LINENAME_SIZE];
} structcity;

typedef struct country_s {
	char name[LINENAME_SIZE];
	int cities[CITIES_IN_COUNTRY];
} structcountry;

typedef struct additional_s {
	point add[ADDITIONAL_POINTS];
	int count;
	int prevst;
	int nextst;
	int line;
} additional;

typedef struct line_s {
	char name[LINENAME_SIZE];
	int color;
	int beginst;
	int endst;
} line;

typedef struct station_s {
	int x;
	int y;
	int rectx;
	int recty;
	int rectw;
	int recth;
	bool solid;
	char name[STATIONNAME_SIZE];
	int colortype;
} station;

typedef struct edge_s {
	int st[2];
	int addcount;
	point addit[ADDITIONAL_POINTS];
	int time[2];
	int colortype;
	int color;
} edge;

typedef struct transfer_s {
	int st[2];
	int time[2];
	bool invisible;
} transfer;

/*
void city_selected(int page);
int map_paint_handler(int type, int par1, int par2);
*/
void city_init();
int city_choose_handler(int type, int par1, int par2);
int map_paint_handler(int type, int par1, int par2);
void map_init(char * city);

bool go(int x, int y);
void abs_go(int x, int y);

void draw_map();
int parse_trp(const char * path);
void parse_map(const char * path);
bool utfcompare (structcountry i, structcountry j);
void calc_fscale_step(int sc = 100);

#endif	/* citychoose_h */

using namespace std;
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <dlfcn.h>
#include "inkview.h"
#include "calendar.h"
#include "cards.h"
#include "config.h"

//#define FILELOG
#define VERSION "v1.3.1"

class	GLOBAL
{
/* */
public:
	static std::ostream * m_lout;
	static CONFIG	m_Config;
	static CARDLIST m_CardList;
	static CALENDAR m_Calendar;
};
void	SetSState(int state);

#define Config				(GLOBAL::m_Config)
#define CardList			(GLOBAL::m_CardList)
#define Calendar			(GLOBAL::m_Calendar)
#define lout				(*GLOBAL::m_lout)
#define CurrentCard			(*(CardList.m_CurCard))
#define sz(str)				(const_cast<char *>((str).c_str ()))
#define RCTVAL(ID)			(Config.m_Rects[ID].x), (Config.m_Rects[ID].y), (Config.m_Rects[ID].w), (Config.m_Rects[ID].h)
#define RCT(ID)				(Config.m_Rects[ID])
#define Beetwen(x, x1, x2)	(x >= (x1) && x <= (x2))
#define InsideR(r)			(Beetwen(par1, r.x, r.x + r.w) && Beetwen(par2, r.y, r.y + r.h))
#define Inside(x, w, y, h)	(Beetwen(par1, x, x + w) && Beetwen(par2, y, y + h))

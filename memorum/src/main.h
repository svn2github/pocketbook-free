#pragma once

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

class GLOBAL
{ public: 
	static ostream *m_lout;
	static CONFIG   m_Config;
	static CARDLIST m_CardList;
	static CALENDAR m_Calendar;
};

#define Config   (GLOBAL::m_Config)
#define CardList (GLOBAL::m_CardList)
#define Calendar (GLOBAL::m_Calendar)
#define lout (*GLOBAL::m_lout)

#define CurrentCard (*(CardList.m_CurCard))
#define sz(str) (const_cast<char*>((str).c_str()))
#define RCTVAL(ID) (Config.m_Rects[ID].x), (Config.m_Rects[ID].y), (Config.m_Rects[ID].w), (Config.m_Rects[ID].h)
#define RCT(ID) (Config.m_Rects[ID])
//#define RCTVAL2(rect) ((rect).x), ((rect).y), ((rect).w), ((rect).h)
//#define RCTVALT(ID, i) (Config.m_Rects[ID].x+(i)), (Config.m_Rects[ID].y+(i)), (Config.m_Rects[ID].w-(i)*2), (Config.m_Rects[ID].h-(i)*2)
//#define RCTVALT2(rect, i) ((rect).x+(i)), ((rect).y+(i)), ((rect).w-(i)*2), ((rect).h-(i)*2)

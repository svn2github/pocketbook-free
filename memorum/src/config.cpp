#include "main.h"

#define CFGEDITVALUE(configedit)	(Config.m_Config), (configedit.name), (configedit.deflt)
#define CFGEDITIVALUE(configedit)	(Config.m_Config), (configedit.name), atoi(configedit.deflt)

char		*YesNoVariants[] = { "@Yes", "@No", NULL };
char		*KeyConfigActionVariants[] =
{
	"@KA_none",		//
	"@KA_mem_0",	//Menu
	"@KA_mem_14",	//MoveNext
	"@KA_mem_15",	//MovePrev
	"@KA_mem_16",	//Sound
	"@KA_mem_17",	//Expand
	"@KA_mem_1",	//Exit
	"@KA_mem_2",	//Configuration
	"@KA_mem_3",	//Smpl_mode
	"@KA_mem_4",	//Memo_mode
	"@KA_mem_5",	//Test_mode

	//"@KA_mem_6",	//Auto_mode
	"@KA_mem_7",	//Change_sides
	"@KA_mem_8",	//Change_sides2
	"@KA_mem_9",	//Rand
	"@KA_mem_10",	//Reset
	"@KA_mem_11",	//Statistic
	"@KA_mem_12",	//Calendar
	"@KA_mem_13",	//Calendar_add
	NULL
};
iconfigedit AdvancedConfigEditor[] =
{
	{ CFG_ENTEXT, NULL, "@Percentstart", "@Percentstart_hint", "param.advanced.percentstart", "5", NULL, NULL },
	{ CFG_ENTEXT, NULL, "@Percentsteap", "@Percentsteap_hint", "param.advanced.percentsteap", "3", NULL, NULL },
	{ CFG_ENTEXT, NULL, "@Graphic", "@Graphic_hint", "param.advanced.graphic", "1;1;3;7;14;30;30;", NULL, NULL },
	{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};
iconfigedit FontConfigEditor[] =
{
	{ CFG_FONT, NULL, "@Cardfont", "@Cardfont_hint", "param.cardfont", DEFAULTFONT ",50", NULL, NULL },
	{ CFG_FONT, NULL, "@Headerfont", "@Headerfont_hint", "param.headerfont", DEFAULTFONT ",24", NULL, NULL },
	{ CFG_FONT, NULL, "@Footerfont", "@Footerfont_hint", "param.footerfont", DEFAULTFONT ",24", NULL, NULL },
	{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};
iconfigedit KeyConfigEditor[] =
{
	{ CFG_CHOICE, NULL, "@Key_left", "@Key_test_hint", "param.key.19.0", "@KA_mem_15", KeyConfigActionVariants,
			NULL },
	{ CFG_CHOICE, NULL, "@Key_left_h", NULL, "param.key.19.1", "@KA_none", KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Key_right", "@Key_test_hint", "param.key.20.0", "@KA_mem_14", KeyConfigActionVariants,
			NULL },
	{ CFG_CHOICE, NULL, "@Key_right_h", NULL, "param.key.20.1", "@KA_none", KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Key_up", "@Key_test_hint", "param.key.17.0", "@KA_mem_0", KeyConfigActionVariants,
			NULL },
	{ CFG_CHOICE, NULL, "@Key_up_h", NULL, "param.key.17.1", "@KA_none", KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Key_down", "@Key_test_hint", "param.key.18.0", "@KA_mem_17", KeyConfigActionVariants,
			NULL },
	{ CFG_CHOICE, NULL, "@Key_down_h", NULL, "param.key.18.1", "@KA_mem_1", KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Key_ok", "@Key_test_hint", "param.key.10.0", "@KA_mem_16", KeyConfigActionVariants,
			NULL },
	{ CFG_CHOICE, NULL, "@Key_ok_h", NULL, "param.key.10.1", "@KA_mem_0", KeyConfigActionVariants, NULL },
	{ CFG_HIDDEN, NULL, "@Key_back", NULL, "param.key.27.0", "@KA_mem_1", KeyConfigActionVariants, NULL },
	{ CFG_HIDDEN, NULL, "@Key_back_h", NULL, "param.key.27.1", "@KA_none", KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Key_next", NULL, "param.key.25.0", "@KA_mem_14", KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Key_next_h", NULL, "param.key.25.1", "@KA_none", KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Key_prev", NULL, "param.key.24.0", "@KA_mem_15", KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Key_prev_h", NULL, "param.key.24.1", "@KA_none", KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Key_next2", NULL, "param.key.29.0", "@KA_mem_14", KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Key_next2_h", NULL, "param.key.29.1", "@KA_none", KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Key_prev2", NULL, "param.key.28.0", "@KA_mem_15", KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Key_prev2_h", NULL, "param.key.28.1", "@KA_none", KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Key_minus", NULL, "param.key.21.0", "@KA_none", KeyConfigActionVariants, NULL },
	{ CFG_HIDDEN, NULL, "@Key_minus_h", NULL, "param.key.21.1", "@KA_none", KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Key_plus", NULL, "param.key.22.0", "@KA_none", KeyConfigActionVariants, NULL },
	{ CFG_HIDDEN, NULL, "@Key_plus_h", NULL, "param.key.22.1", "@KA_none", KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Key_music", NULL, "param.key.30.0", "@KA_none", KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Key_music_h", NULL, "param.key.30.1", "@KA_none", KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Key_menu", NULL, "param.key.23.0", "@KA_none", KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Key_menu_h", NULL, "param.key.23.1", "@KA_none", KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Key_delete", NULL, "param.key.08.0", "@KA_none", KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Key_delete_h", NULL, "param.key.08.1", "@KA_none", KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Pointgesture_side1", "@Pointgesture_side1_hint", "param.pg.1", "@KA_mem_16",
			KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Pointgesture_side2", "@Pointgesture_side2_hint", "param.pg.2", "@KA_mem_17",
			KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Pointgesture_side3", "@Pointgesture_side3_hint", "param.pg.3", "@KA_mem_17",
			KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Pointgesture_left", "@Pointgesture_left_hint", "param.pg.4", "@KA_mem_15",
			KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Pointgesture_right", "@Pointgesture_right_hint", "param.pg.5", "@KA_mem_14",
			KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Pointgesture_up", "@Pointgesture_up_hint", "param.pg.6", "@KA_none",
			KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Pointgesture_down", "@Pointgesture_down_hint", "param.pg.7", "@KA_none",
			KeyConfigActionVariants, NULL },
	{ CFG_CHOICE, NULL, "@Pointgesture_longtap", "@Pointgesture_longtap_hint", "param.pg.8", "@KA_mem_0",
			KeyConfigActionVariants, NULL },
	{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};
iconfigedit ConfigEditor[] =
{
	{ CFG_INFO, NULL, "@About_program", NULL, "about", NULL, NULL, NULL },
	{ CFG_CHOICE, NULL, "@Autorand", "@Autorand_hint", "param.autorand", "@Yes", YesNoVariants, NULL },
	{ CFG_SUBMENU, NULL, "@Main_font", "@Main_font_hint", NULL, NULL, NULL, FontConfigEditor },
	{ CFG_ENTEXT, NULL, "@Pagemargins", "@Pagemargins_hint", "param.pagemargins", "5", NULL, NULL },
	{ CFG_CHOICE, NULL, "@Proportional", "@Proportional_hint", "param.proportional", "@Yes", YesNoVariants, NULL },
	{ CFG_HIDDEN, NULL, "@Grayscale16", "@Grayscale16_hint", "param.grayscale16", "@No", YesNoVariants, NULL },
	{ CFG_SUBMENU, NULL, "@Keys_config", NULL, NULL, NULL, NULL, KeyConfigEditor },
	{ CFG_SUBMENU, NULL, "@Advanced", "@Advanced_hint", NULL, NULL, NULL, AdvancedConfigEditor },
	{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};

/* */
ifont *ReadFont(iconfig *cfg, char *name, char *deflt)
{
	char	*fname, buf[256];
	int		size;
	strcpy(buf, ReadString(cfg, name, deflt));
	fname = strtok(buf, ",");
	sscanf(fname + strlen(fname) + 1, "%d", &size);

	ifont	*fnt = OpenFont(fname, size, 0);
	return fnt;
}

/* */
void UpdateData()
{
	Config.Autorand = ReadString(CFGEDITVALUE(ConfigEditor[1]))[1] == 'Y';
	Config.Cardfont = ReadFont(CFGEDITVALUE(FontConfigEditor[0]));
	Config.Headerfont = ReadFont(CFGEDITVALUE(FontConfigEditor[1]));
	Config.Footerfont = ReadFont(CFGEDITVALUE(FontConfigEditor[2]));
	Config.Pagemargins = ReadInt(CFGEDITIVALUE(ConfigEditor[3]));
	Config.Proportional = ReadString(CFGEDITVALUE(ConfigEditor[4]))[1] == 'Y';
	Config.Grayscale16 = ReadString(CFGEDITVALUE(ConfigEditor[5]))[1] == 'Y';
	Config.Percentstart = ReadInt(CFGEDITIVALUE(AdvancedConfigEditor[0]));
	Config.Percentsteap = ReadInt(CFGEDITIVALUE(AdvancedConfigEditor[1]));

	char	*szGraphic = new char[256];
	char	*tc = szGraphic;
	strcpy(szGraphic, ReadString(CFGEDITVALUE(AdvancedConfigEditor[2])));
	while (szGraphic[0])
	{
		szGraphic = strtok(szGraphic, ";");
		Config.Graphic.push_back(atoi(szGraphic));
		szGraphic += strlen(szGraphic) + 1;
	}

	delete[] tc;

	Config.RecalcRects();

	Config.MainHandlerDescription.clear();
	for (int i = 0; i < 30; i = i + 2) Config.MainHandlerDescription.push_back(new KeyConfig(i));
	Config.MainHandlerDescription.push_back(new PointerGestConfig(33, -200, 150, true));
	Config.MainHandlerDescription.push_back(new PointerGestConfig(34, 200, 150, true));
	Config.MainHandlerDescription.push_back(new PointerGestConfig(35, 150, -200, false));
	Config.MainHandlerDescription.push_back(new PointerGestConfig(36, 150, 200, false));
	Config.MainHandlerDescription.push_back(new PointerTapConfig(37, EVT_POINTERLONG, Config.m_Rects[FullScreen]));
	Config.MainHandlerDescription.push_back(new PointerTapConfig(30, EVT_POINTERUP, Config.m_Rects[Card3Sd1]));
	Config.MainHandlerDescription.push_back(new PointerTapConfig(31, EVT_POINTERUP, Config.m_Rects[Card3Sd2]));
	Config.MainHandlerDescription.push_back(new PointerTapConfig(32, EVT_POINTERUP, Config.m_Rects[Card3Sd3]));

	SaveConfig(Config.m_Config);
	if (CardList.m_MainImpliment) CardList.Repaint();
}

/* */
void ItemChange(char *name)
{
	if (!strcmp(name, "about"))
		Message(ICON_INFORMATION, "Memorum", "Memorum "VERSION "\n(c) Naydenov Ivan (aka Samogot)", 5000);
}

/* */
KeyConfig::KeyConfig(int i)
{
	evt_type = EVT_KEYRELEASE;
	key = (int) atof(KeyConfigEditor[i].name + 10);

	char			*tempS;
	KeyHandlerFunc	tempF;
	tempS = ReadString(CFGEDITVALUE(KeyConfigEditor[i]));
	if (!strcmp(tempS, "@KA_none"))
		tempF = NULL;
	else
		tempF = Config.MainHandlerDefenition[atoi(tempS + 8)];
	func = tempF;
	tempS = ReadString(CFGEDITVALUE(KeyConfigEditor[i + 1]));
	if (!strcmp(tempS, "@KA_none"))
		tempF = NULL;
	else
		tempF = Config.MainHandlerDefenition[atoi(tempS + 8)];
	func2 = tempF;
}

/* */
bool KeyConfig::RunIf(int evt, int par1, int par2)
{
	if (evt == evt_type && par1 == key)
	{
		if (par2 == 0 && func != NULL)
		{
			func(par1, par2);
			return true;
		}

		if (par2 > 0 && func2 != NULL)
		{
			func2(par1, par2);
			return true;
		}
	}

	return false;
}

/* */
PointerGestConfig::PointerGestConfig(int i, int w, int h, bool flip)
{
	evt_type = EVT_POINTERUP;
	this->w = w;
	this->h = h;
	this->fl = flip;
	x = -1;

	char	*tempS = ReadString(CFGEDITVALUE(KeyConfigEditor[i]));
	if (!strcmp(tempS, "@KA_none"))
		func = NULL;
	else
		func = Config.MainHandlerDefenition[atoi(tempS + 8)];
}

/* */
bool PointerGestConfig::RunIf(int evt, int par1, int par2)
{
	if (func != NULL && evt == EVT_POINTERMOVE && x < 0)
	{
		x = par1;
		y = par2;
		return false;
	}

	if (evt == EVT_POINTERUP && x > 0 && func != NULL)
	{
		int w = par1 - x, h = par2 - y;
		x = -1;
		if (fl && -this->h < h && h < this->h && ((w > this->w && this->w > 0) || (w < this->w && this->w < 0)))
		{
			func(par1, par2);
			lout << "handled" << endl;
			return true;
		}

		if (!fl && (-this->w < w && w < this->w) && ((h > this->h && this->h > 0) || (h < this->h && this->h < 0)))
		{
			func(par1, par2);
			lout << "handled" << endl;
			return true;
		}
	}

	return false;
}

/* */
PointerTapConfig::PointerTapConfig(int i, int evt, irect rect)
{
	evt_type = evt;
	this->rect = rect;
	move = false;

	char	*tempS = ReadString(CFGEDITVALUE(KeyConfigEditor[i]));
	if (!strcmp(tempS, "@KA_none"))
		func = NULL;
	else
		func = Config.MainHandlerDefenition[atoi(tempS + 8)];
}

/* */
bool PointerTapConfig::RunIf(int evt, int par1, int par2)
{
	if (evt == evt_type && func != NULL && !move && InsideR(rect))
	{
		func(par1, par2);
		lout << "handled" << endl;
		return true;
	}

	move = evt == EVT_POINTERMOVE;
	return false;
}

/* */
void CONFIG::Init()
{
	m_ConfigHandler = &UpdateData;
	m_ItemChangeHandler = &ItemChange;
	m_ConfigEditor = ConfigEditor;
	m_Config = OpenConfig(CONFIGPATH "/memorum.cfg", m_ConfigEditor);
	m_ConfigHandler();
	Delftfont = OpenFont(DEFAULTFONT, 45, 0);
	Delftfontb = OpenFont(DEFAULTFONTB, 45, 0);
}

/* */
void CONFIG::Show()
{
	OpenConfigEditor("@Configuration", m_Config, m_ConfigEditor, m_ConfigHandler, m_ItemChangeHandler);
}

/* */
void CONFIG::RecalcRects()
{
#ifdef EMULATOR
	int de = 87;
#else
	int de = 0;
#endif
	int ScrW = ScreenWidth();
	int ScrH = ScreenHeight() - de;
	int HeaderH = Headerfont->height + 4;
	int FooterH = Footerfont->height + 14;
	int CrdH2S = (ScrH - HeaderH - 3 * Pagemargins) / 2;
	int CrdH3S = (ScrH - HeaderH - 4 * Pagemargins) / 3;
	int CrdW1C = (ScrW - 2 * Pagemargins) / 1;
	int CrdW2C = (ScrW - 3 * Pagemargins) / 2;
	int SelW4C = (ScrW - 5 * Pagemargins) / 4;

	m_Rects[FullScreen].x = 0;
	m_Rects[FullScreen].y = 0;
	m_Rects[FullScreen].w = ScrW;
	m_Rects[FullScreen].h = ScrH;
	m_Rects[FullScreen].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[Header].x = 0;
	m_Rects[Header].y = 0;
	m_Rects[Header].w = ScrW;
	m_Rects[Header].h = HeaderH;
	m_Rects[Header].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[Footer].x = 0;
	m_Rects[Footer].y = ScrH - FooterH - 2 * Pagemargins;
	m_Rects[Footer].w = ScrW;
	m_Rects[Footer].h = FooterH + 2 * Pagemargins;
	m_Rects[Footer].flags = ALIGN_RIGHT | VALIGN_MIDDLE;

	m_Rects[SelButt1].x = Pagemargins + 0 * (SelW4C + Pagemargins);
	m_Rects[SelButt1].y = ScrH - FooterH - Pagemargins;
	m_Rects[SelButt1].w = SelW4C;
	m_Rects[SelButt1].h = FooterH;
	m_Rects[SelButt1].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[SelButt2].x = Pagemargins + 1 * (SelW4C + Pagemargins);
	m_Rects[SelButt2].y = ScrH - FooterH - Pagemargins;
	m_Rects[SelButt2].w = SelW4C;
	m_Rects[SelButt2].h = FooterH;
	m_Rects[SelButt2].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[SelButt3].x = Pagemargins + 2 * (SelW4C + Pagemargins);
	m_Rects[SelButt3].y = ScrH - FooterH - Pagemargins;
	m_Rects[SelButt3].w = SelW4C;
	m_Rects[SelButt3].h = FooterH;
	m_Rects[SelButt3].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[SelButt4].x = Pagemargins + 3 * (SelW4C + Pagemargins);
	m_Rects[SelButt4].y = ScrH - FooterH - Pagemargins;
	m_Rects[SelButt4].w = SelW4C;
	m_Rects[SelButt4].h = FooterH;
	m_Rects[SelButt4].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[Card2Sd1].x = Pagemargins;
	m_Rects[Card2Sd1].y = HeaderH + Pagemargins;
	m_Rects[Card2Sd1].w = CrdW1C;
	m_Rects[Card2Sd1].h = CrdH2S;
	m_Rects[Card2Sd1].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[Card2Sd2].x = Pagemargins;
	m_Rects[Card2Sd2].y = HeaderH + 2 * Pagemargins + CrdH2S;
	m_Rects[Card2Sd2].w = CrdW1C;
	m_Rects[Card2Sd2].h = CrdH2S;
	m_Rects[Card2Sd2].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[Card3Sd1].x = Pagemargins;
	m_Rects[Card3Sd1].y = HeaderH + Pagemargins;
	m_Rects[Card3Sd1].w = CrdW1C;
	m_Rects[Card3Sd1].h = CrdH3S;
	m_Rects[Card3Sd1].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[Card3Sd2].x = Pagemargins;
	m_Rects[Card3Sd2].y = HeaderH + 2 * Pagemargins + CrdH3S;
	m_Rects[Card3Sd2].w = CrdW1C;
	m_Rects[Card3Sd2].h = CrdH3S;
	m_Rects[Card3Sd2].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[Card3Sd3].x = Pagemargins;
	m_Rects[Card3Sd3].y = HeaderH + 3 * Pagemargins + 2 * CrdH3S;
	m_Rects[Card3Sd3].w = CrdW1C;
	m_Rects[Card3Sd3].h = CrdH3S;
	m_Rects[Card3Sd3].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[CardTst0].x = Pagemargins;
	m_Rects[CardTst0].y = HeaderH + Pagemargins;
	m_Rects[CardTst0].w = CrdW1C;
	m_Rects[CardTst0].h = CrdH3S;
	m_Rects[CardTst0].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[CardTst11].x = Pagemargins;
	m_Rects[CardTst11].y = HeaderH + 2 * Pagemargins + CrdH3S;
	m_Rects[CardTst11].w = CrdW2C;
	m_Rects[CardTst11].h = CrdH3S;
	m_Rects[CardTst11].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[CardTst12].x = 2 * Pagemargins + CrdW2C;
	m_Rects[CardTst12].y = HeaderH + 2 * Pagemargins + CrdH3S;
	m_Rects[CardTst12].w = CrdW2C;
	m_Rects[CardTst12].h = CrdH3S;
	m_Rects[CardTst12].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[CardTst21].x = Pagemargins;
	m_Rects[CardTst21].y = HeaderH + 3 * Pagemargins + 2 * CrdH3S;
	m_Rects[CardTst21].w = CrdW2C;
	m_Rects[CardTst21].h = CrdH3S;
	m_Rects[CardTst21].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[CardTst22].x = 2 * Pagemargins + CrdW2C;
	m_Rects[CardTst22].y = HeaderH + 3 * Pagemargins + 2 * CrdH3S;
	m_Rects[CardTst22].w = CrdW2C;
	m_Rects[CardTst22].h = CrdH3S;
	m_Rects[CardTst22].flags = ALIGN_CENTER | VALIGN_MIDDLE;
}

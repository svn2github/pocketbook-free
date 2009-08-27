#include "main.h"

#define CFGEDITVALUE(configedit)  (Config.m_Config), (configedit.name),      (configedit.deflt)
#define CFGEDITIVALUE(configedit) (Config.m_Config), (configedit.name), (atoi(configedit.deflt))

char *YesNoVariants[] = {"@Yes", "@No"};
iconfigedit AdvancedConfigEditor[] = {
	{ CFG_TEXT, NULL, "@Percentstart", "@Percentstart_hint", "param.advanced.percentstart", "5", NULL, NULL },
	{ CFG_TEXT, NULL, "@Percentsteap", "@Percentsteap_hint", "param.advanced.percentsteap", "3", NULL, NULL },
	{ CFG_TEXT, NULL, "@Graphic", "@Graphic_hint", "param.advanced.graphic", "1;1;3;7;14;30;30;", NULL, NULL },
	{ 0, NULL, NULL, NULL, NULL, NULL, NULL}
};
iconfigedit FontConfigEditor[] = {
	{ CFG_FONT, NULL, "@Cardfont", "@Cardfont_hint", "param.cardfont", DEFAULTFONT",50", NULL, NULL },
	{ CFG_FONT, NULL, "@Numbersfont", "@Numbersfont_hint", "param.numbersfont", DEFAULTFONTI",20", NULL, NULL },
	{ CFG_FONT, NULL, "@Headerfont", "@Headerfont_hint", "param.headerfont", DEFAULTFONT",24", NULL, NULL },
	{ CFG_FONT, NULL, "@Footerfont", "@Footerfont_hint", "param.footerfont", DEFAULTFONT",24", NULL, NULL },
	{ 0, NULL, NULL, NULL, NULL, NULL, NULL}
};
iconfigedit ConfigEditor[] = {
	{ CFG_CHOICE, NULL, "@Autorand", "@Autorand_hint", "param.autorand", "@Yes", YesNoVariants, NULL },
	{ CFG_SUBMENU, NULL, "@Main_font", "@Main_font_hint", NULL, NULL, NULL, FontConfigEditor },
	{ CFG_TEXT, NULL, "@Pagemargins", "@Pagemargins_hint", "param.pagemargins", "5", NULL, NULL },
	{ CFG_CHOICE, NULL, "@Proportional", "@Proportional_hint", "param.proportional", "@Yes", YesNoVariants, NULL },
	{ CFG_SUBMENU, NULL, "@Advanced", "@Advanced_hint", NULL, NULL, NULL, AdvancedConfigEditor },
	{ 0, NULL, NULL, NULL, NULL, NULL, NULL}
};
ifont *ReadFont(iconfig *cfg, char *name, char *deflt)
{
	char *fname, buf[256];
	int size;
	strcpy(buf, ReadString(cfg, name, deflt));
	fname = strtok(buf, ",");
	sscanf(fname+strlen(fname)+1, "%d", &size);
	ifont *fnt = OpenFont(fname, size, 0);
	return fnt;
}
void UpdateData()
{
	Config.Autorand     = ReadString(CFGEDITVALUE(ConfigEditor[0]));
	Config.Cardfont     = ReadFont(CFGEDITVALUE(FontConfigEditor[0]));
	Config.Numbersfont  = ReadFont(CFGEDITVALUE(FontConfigEditor[1]));
	Config.Headerfont   = ReadFont(CFGEDITVALUE(FontConfigEditor[2]));
	Config.Footerfont   = ReadFont(CFGEDITVALUE(FontConfigEditor[3]));
	Config.Pagemargins  = ReadInt(CFGEDITIVALUE(ConfigEditor[2]));
	Config.Proportional = ReadString(CFGEDITVALUE(ConfigEditor[3]));
	Config.Percentstart = ReadInt(CFGEDITIVALUE(AdvancedConfigEditor[0]));
	Config.Percentsteap = ReadInt(CFGEDITIVALUE(AdvancedConfigEditor[1]));

	char *szGraphic = new char[256];
	char *tc=szGraphic;
	strcpy(szGraphic, ReadString(CFGEDITVALUE(AdvancedConfigEditor[2])));
	int t;
	while(szGraphic[0])
	{
		lout<<szGraphic<<"."<<endl;
		szGraphic = strtok(szGraphic, ";");
		t=atoi(szGraphic);
		lout<<t<<endl;
		Config.Graphic.push_back(t);
		szGraphic+=strlen(szGraphic)+1;
	}
	delete[] tc;

	SaveConfig(Config.m_Config);
	Config.RecalcRects();
	if(CardList.m_MainImpliment)
		CardList.Repaint();
}
void CONFIG::Init()
{
	m_ConfigHandler = &UpdateData;
	m_ConfigEditor = ConfigEditor;
	m_Config = OpenConfig(CONFIGPATH"/memorum.cfg", m_ConfigEditor);
	m_ConfigHandler();
	Delftfont = OpenFont(DEFAULTFONT, 45, 0);
	Delftfontb = OpenFont(DEFAULTFONTB, 45, 0);
}
void CONFIG::Show()
{
	OpenConfigEditor("@Configuration", m_Config, m_ConfigEditor, m_ConfigHandler, NULL);
}
void CONFIG::RecalcRects()
{
	int de = 0;
//	de=85;
	int ScrW = ScreenWidth();
	int ScrH = ScreenHeight()-de;
	int HeaderH = Headerfont->height+4;
	int FooterH = Footerfont->height+4;
	int CrdH2S = (ScrH-HeaderH-3*Pagemargins)/2;
	int CrdH3S = (ScrH-HeaderH-4*Pagemargins)/3;
	int CrdHTs = (ScrH-HeaderH-FooterH-6*Pagemargins)/3;
	int CrdW1C = (ScrW-2*Pagemargins)/1;
	int CrdW2C = (ScrW-3*Pagemargins)/2;
	int SelW4C = (ScrW-5*Pagemargins)/4;

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
	m_Rects[Footer].y = ScrH-FooterH-2*Pagemargins;
	m_Rects[Footer].w = ScrW;
	m_Rects[Footer].h = FooterH+2*Pagemargins;
	m_Rects[Footer].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[SelButt1].x = Pagemargins+0*(SelW4C+Pagemargins);
	m_Rects[SelButt1].y = ScrH-FooterH-Pagemargins;
	m_Rects[SelButt1].w = SelW4C;
	m_Rects[SelButt1].h = FooterH;
	m_Rects[SelButt1].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[SelButt2].x = Pagemargins+1*(SelW4C+Pagemargins);
	m_Rects[SelButt2].y = ScrH-FooterH-Pagemargins;
	m_Rects[SelButt2].w = SelW4C;
	m_Rects[SelButt2].h = FooterH;
	m_Rects[SelButt2].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[SelButt3].x = Pagemargins+2*(SelW4C+Pagemargins);
	m_Rects[SelButt3].y = ScrH-FooterH-Pagemargins;
	m_Rects[SelButt3].w = SelW4C;
	m_Rects[SelButt3].h = FooterH;
	m_Rects[SelButt3].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[SelButt4].x = Pagemargins+3*(SelW4C+Pagemargins);
	m_Rects[SelButt4].y = ScrH-FooterH-Pagemargins;
	m_Rects[SelButt4].w = SelW4C;
	m_Rects[SelButt4].h = FooterH;
	m_Rects[SelButt4].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[Card2Sd1].x = Pagemargins;
	m_Rects[Card2Sd1].y = HeaderH+Pagemargins;
	m_Rects[Card2Sd1].w = CrdW1C;
	m_Rects[Card2Sd1].h = CrdH2S;
	m_Rects[Card2Sd1].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[Card2Sd2].x = Pagemargins;
	m_Rects[Card2Sd2].y = HeaderH+2*Pagemargins+CrdH2S;
	m_Rects[Card2Sd2].w = CrdW1C;
	m_Rects[Card2Sd2].h = CrdH2S;
	m_Rects[Card2Sd2].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[Card3Sd1].x = Pagemargins;
	m_Rects[Card3Sd1].y = HeaderH+Pagemargins;
	m_Rects[Card3Sd1].w = CrdW1C;
	m_Rects[Card3Sd1].h = CrdH3S;
	m_Rects[Card3Sd1].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[Card3Sd2].x = Pagemargins;
	m_Rects[Card3Sd2].y = HeaderH+2*Pagemargins+CrdH3S;
	m_Rects[Card3Sd2].w = CrdW1C;
	m_Rects[Card3Sd2].h = CrdH3S;
	m_Rects[Card3Sd2].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[Card3Sd3].x = Pagemargins;
	m_Rects[Card3Sd3].y = HeaderH+3*Pagemargins+2*CrdH3S;
	m_Rects[Card3Sd3].w = CrdW1C;
	m_Rects[Card3Sd3].h = CrdH3S;
	m_Rects[Card3Sd3].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[CardTst0].x = Pagemargins;
	m_Rects[CardTst0].y = HeaderH+Pagemargins;
	m_Rects[CardTst0].w = CrdW1C;
	m_Rects[CardTst0].h = CrdHTs;
	m_Rects[CardTst0].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[CardTst11].x = Pagemargins;
	m_Rects[CardTst11].y = HeaderH+2*Pagemargins+CrdHTs;
	m_Rects[CardTst11].w = CrdW2C;
	m_Rects[CardTst11].h = CrdHTs;
	m_Rects[CardTst11].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[CardTst12].x = 2*Pagemargins+CrdW2C;
	m_Rects[CardTst12].y = HeaderH+2*Pagemargins+CrdHTs;
	m_Rects[CardTst12].w = CrdW2C;
	m_Rects[CardTst12].h = CrdHTs;
	m_Rects[CardTst12].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[CardTst21].x = Pagemargins;
	m_Rects[CardTst21].y = HeaderH+3*Pagemargins+2*CrdHTs;
	m_Rects[CardTst21].w = CrdW2C;
	m_Rects[CardTst21].h = CrdHTs;
	m_Rects[CardTst21].flags = ALIGN_CENTER | VALIGN_MIDDLE;

	m_Rects[CardTst22].x = 2*Pagemargins+CrdW2C;
	m_Rects[CardTst22].y = HeaderH+3*Pagemargins+2*CrdHTs;
	m_Rects[CardTst22].w = CrdW2C;
	m_Rects[CardTst22].h = CrdHTs;
	m_Rects[CardTst22].flags = ALIGN_CENTER | VALIGN_MIDDLE;
}

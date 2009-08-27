#pragma once

enum RectID {
	FullScreen,
	SelButt1,
	SelButt2,
	SelButt3,
	SelButt4,
	Footer,
	Card2Sd1,
	Card2Sd2,
	Header,
	Card3Sd1,
	Card3Sd3,
	Card3Sd2,
	CardTst0,
	CardTst11,
	CardTst12,
	CardTst21,
	CardTst22,
	MaxId
};

class CONFIG
{
	iconfig *m_Config;
	iconfigedit *m_ConfigEditor;
	iv_confighandler m_ConfigHandler;
	friend void UpdateData();

public:
	irect m_Rects[MaxId];

	bool Autorand;
	ifont *Cardfont;
	ifont *Boldfont;
	ifont *Numbersfont;
	ifont *Headerfont;
	ifont *Footerfont;
	ifont *Delftfontb;
	ifont *Delftfont;
	int Pagemargins;
	bool Proportional;
	int Percentstart;
	int Percentsteap;
	vector<int> Graphic;

	void Init();
	void Show();
	void RecalcRects();
};

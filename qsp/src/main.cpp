#include "main.h"

extern ifont *defaultFont;
extern MainScreen mainScreen;

iconfig *config = 0;
#define CONFIG_FONT "Font"
iconfigedit QSPConfig[] =
{
  { CFG_TEXT, NULL, "״נטפע", NULL, CONFIG_FONT, "LiberationSans, 18", NULL, NULL },
  { 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};

void LoadConfig()
{
	try
	{
		config = OpenConfig(CONFIGPATH"/qsp.cfg", QSPConfig);
		char *fontName = ReadString(config, CONFIG_FONT, "LiberationSans, 18");
		std::string font(fontName);
	
		size_t div_pos = font.find_first_of(',');
		if (div_pos != std::string::npos)
		{
			int size = atoi(font.substr(div_pos+1).c_str());
			if (defaultFont != 0)
				CloseFont(defaultFont);
			OpenFont((char *)font.substr(0, div_pos).c_str(), size, 1);
		}
	}
	catch(...)
	{
		fprintf(stderr, "\nerror loading config");
	}
}

void SaveConfig()
{
	try
	{
		char fontName[80];
		sprintf(fontName, "%s,%d", defaultFont->name, defaultFont->size); 
		WriteString(config, CONFIG_FONT, fontName);
		SaveConfig(config);
		CloseConfig(config);
	}
	catch(...)
	{
		fprintf(stderr, "\nerror saving config");
	}
}

int main_handler(int type, int par1, int par2)
{
	
	if (type == EVT_INIT)
	{
		defaultFont = OpenFont("LiberationSans", 18, 1);
		mainScreen.SetControlFont(defaultFont);
		SetOrientation(GetGlobalOrientation(-1));
		QSPInit();
		QSPCallbacks::SetQSPCallbacks();
		
		//LoadConfig();
	}
	
	mainScreen.HandleMsg(type, par1, par2);

	if (type == EVT_EXIT)
	{
		//SaveConfig();
		QSPCallbacks::DeInit();
		QSPDeInit();
		CloseFont(defaultFont);
	}

	return 0;

}

int main(int argc, char **argv)
{
	//mainScreen = new MainScreen();
	InkViewMain(main_handler);
	return 0;
}


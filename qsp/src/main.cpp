#include "main.h"

extern ifont *defaultFont;
extern MainScreen mainScreen;

std::string prevGame;

iconfig *config = 0;
#define CONFIG_FONT "Font"
#define CONFIG_GAME "Game"
iconfigedit QSPConfig[] =
{
  { CFG_TEXT, NULL, "Øðèôò", NULL, CONFIG_FONT, "LiberationSans, 22", NULL, NULL },
  { CFG_TEXT, NULL, "Êíèãà", NULL, CONFIG_GAME, "", NULL, NULL },
  { 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};

void LoadConfig()
{
	try
	{
		config = OpenConfig(CONFIGPATH"/qsp.cfg", QSPConfig);
		char *fontName = ReadString(config, CONFIG_FONT, "LiberationSans, 22");
		std::string font(fontName);
	
		size_t div_pos = font.find_first_of(',');
		if (div_pos != std::string::npos)
		{
			int size = atoi(font.substr(div_pos+1).c_str());
			if (defaultFont != 0)
				CloseFont(defaultFont);
			defaultFont = OpenFont((char *)font.substr(0, div_pos).c_str(), size, 1);
		}
		
		if (prevGame.size() == 0)
		{
			char *gameName = ReadString(config, CONFIG_GAME, 0);
			if (gameName != 0)
				prevGame = gameName;
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
		
		WriteString(config, CONFIG_GAME, (char*)QSPGetQstFullPath());
		
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
		LoadConfig();
		if (defaultFont == 0)
			defaultFont = OpenFont(DEFAULTFONT, 22, 1);
		mainScreen.SetControlFont(defaultFont);
		SetOrientation(GetGlobalOrientation(-1));
		
		QSPInit();
		QSPCallbacks::SetQSPCallbacks();
		
		InterfaceEventsTimer();
		
		if (prevGame.size() > 0)
			SendQSPEvent(QSP_EVT_OPENGAME, prevGame);
			
	}
	
	mainScreen.HandleMsg(type, par1, par2);

	if (type == EVT_EXIT)
	{
		SaveConfig();
		QSPCallbacks::DeInit();
		QSPDeInit();
		CloseFont(defaultFont);
	}

	return 0;

}

int main(int argc, char **argv)
{
	//mainScreen = new MainScreen();
	pthread_t qspThread; 
	int res = pthread_create(&qspThread, NULL, QSPThreadProc, NULL);
	
	if(argc>1) 
	{
		prevGame = argv[1];
	}
	
	InkViewMain(main_handler);
	return 0;
}


#include "main.h"

extern ifont *defaultFont;
extern MainScreen mainScreen;

int main_handler(int type, int par1, int par2)
{
	
	if (type == EVT_INIT)
	{
		defaultFont = OpenFont("times", 16, 1);
		mainScreen.SetControlFont(defaultFont);
		
		QSPInit();
		QSPCallbacks::SetQSPCallbacks();
	}
	
	mainScreen.HandleMsg(type, par1, par2);

	if (type == EVT_EXIT)
	{
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


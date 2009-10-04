#include "sys/wait.h"
#include "string.h"
#include "qspcallbacks.h"
#include "screens.h"
#include "qspthread.h"

extern MainScreen mainScreen;

void QSPCallbacks::SetQSPCallbacks()
{
	QSPSetCallBack(QSP_CALL_SETTIMER, (QSP_CALLBACK)&SetTimer);
	QSPSetCallBack(QSP_CALL_REFRESHINT, (QSP_CALLBACK)&RefreshInt);
	QSPSetCallBack(QSP_CALL_SETINPUTSTRTEXT, (QSP_CALLBACK)&SetInputStrText);
	QSPSetCallBack(QSP_CALL_ISPLAYINGFILE, (QSP_CALLBACK)&IsPlay);
	QSPSetCallBack(QSP_CALL_PLAYFILE, (QSP_CALLBACK)&PlayFile);
	QSPSetCallBack(QSP_CALL_CLOSEFILE, (QSP_CALLBACK)&CloseFile);
	QSPSetCallBack(QSP_CALL_SHOWMSGSTR, (QSP_CALLBACK)&Msg);
	QSPSetCallBack(QSP_CALL_SLEEP, (QSP_CALLBACK)&Sleep);
	QSPSetCallBack(QSP_CALL_GETMSCOUNT, (QSP_CALLBACK)&GetMSCount);
	QSPSetCallBack(QSP_CALL_DELETEMENU, (QSP_CALLBACK)&DeleteMenu);
	QSPSetCallBack(QSP_CALL_ADDMENUITEM, (QSP_CALLBACK)&AddMenuItem);
	QSPSetCallBack(QSP_CALL_SHOWMENU, (QSP_CALLBACK)&ShowMenu);
	QSPSetCallBack(QSP_CALL_INPUTBOX, (QSP_CALLBACK)&Input);
	QSPSetCallBack(QSP_CALL_SHOWIMAGE, (QSP_CALLBACK)&ShowImage);
	QSPSetCallBack(QSP_CALL_SHOWWINDOW, (QSP_CALLBACK)&ShowPane);
	QSPSetCallBack(QSP_CALL_OPENGAMESTATUS, (QSP_CALLBACK)&OpenGameStatus);
	QSPSetCallBack(QSP_CALL_SAVEGAMESTATUS, (QSP_CALLBACK)&SaveGameStatus);
}

long timer_interval = 0;
void timer_proc()
{
	if (!QSPExecCounter(QSP_TRUE))
		ShowError();
	SetHardTimer("QSPTIMER", timer_proc, timer_interval);
}

void QSPCallbacks::SetTimer(long msecs)
{
	timer_interval = msecs;
	if (msecs <= 0)
	{
		ClearTimer(timer_proc);
	}
	else
	{
		SetHardTimer("QSPTIMER", timer_proc, msecs);
	}
}

void QSPCallbacks::RefreshInt(QSP_BOOL isRedraw)
{
	SendIntEvent(INT_EVT_UPDATE);
	//mainScreen.UpdateUI(false);
}

void QSPCallbacks::SetInputStrText(const QSP_CHAR *text)
{
	std::string text_str;
	to_utf8((unsigned char *)text, &text_str, koi8_to_unicode);
	mainScreen.GetGameScreen()->SetLastCommand(text_str);
}

QSP_BOOL QSPCallbacks::IsPlay(const QSP_CHAR *file)
{
	return false;
}

void QSPCallbacks::CloseFile(const QSP_CHAR *file)
{
}

void QSPCallbacks::PlayFile(const QSP_CHAR *file, long volume)
{
}

void QSPCallbacks::ShowPane(long type, QSP_BOOL isShow)
{
	mainScreen.GetGameScreen()->ShowWindow(type, isShow);
}

void QSPCallbacks::Sleep(long msecs)
{
	usleep(msecs);
}

long QSPCallbacks::GetMSCount()
{
	static time_t lastTime = 0;
	time_t now = time(0);
	long interval;
	
	if (lastTime == 0)
		interval = 0;
	else
		interval = (long)(difftime(now, lastTime) * 1000);
		
	lastTime = now;
	
	return interval;
}

void QSPCallbacks::Msg(const QSP_CHAR *str)
{
	std::string text;
	to_utf8((unsigned char *)str, &text, koi8_to_unicode);
	Message(ICON_INFORMATION, "", (char *)text.c_str(), 5000);
	//SendIntEvent(INT_EVT_MESSAGE, text);
}


static imenu dynamicMenu[DYN_MENU_SIZE];
static int dynamicMenuSize = 0;
	
void QSPCallbacks::DeleteMenu()
{
	dynamicMenuSize = 0;
	dynamicMenu[0].type = 0;
}

void QSPCallbacks::AddMenuItem(const QSP_CHAR *name, const QSP_CHAR *imgPath)
{
	if (dynamicMenuSize >= DYN_MENU_SIZE)
		return;
		
	std::string itemName;
	to_utf8((unsigned char *)name, &itemName, koi8_to_unicode);
	if (itemName == "-")
	{
		dynamicMenu[dynamicMenuSize].type = ITEM_SEPARATOR;
		dynamicMenu[dynamicMenuSize].index = 0;
	}
	else
	{
		char *text = new char [strlen(itemName.c_str())+1];
		strcpy(text, itemName.c_str());
		dynamicMenu[dynamicMenuSize].type = ITEM_ACTIVE;
		dynamicMenu[dynamicMenuSize].index = dynamicMenuSize;
		dynamicMenu[dynamicMenuSize].text = text;
		dynamicMenu[dynamicMenuSize].submenu = 0;
	}
	
	dynamicMenuSize++;
	
	dynamicMenu[dynamicMenuSize].type = 0;
}

void HandleDynamicMenuItem(int index)
{
	QSPSelectMenuItem(index);
}

void QSPCallbacks::ShowMenu()
{
	OpenMenu(dynamicMenu, 0, ScreenWidth()/3, ScreenHeight()/4, HandleDynamicMenuItem);
}

static char inputBuf[1024] = "";
volatile bool keyb_closed;
void keyboard_entry(char *s)
{
	keyb_closed = true;
}

std::string inputTitle;
long inputMaxLen;

void QSPCallbacks::Input(const QSP_CHAR *text, QSP_CHAR *buffer, long maxLen)
{
	inputMaxLen = maxLen / 2;
	to_utf8((unsigned char *)text, &inputTitle, koi8_to_unicode);
	
   	keyb_closed = false;
	OpenKeyboard((char*)inputTitle.c_str(), inputBuf, inputMaxLen, 0, keyboard_entry);
	while (!keyb_closed) usleep(500);

	std::string encoded_text = utf8_to((const unsigned char *)inputBuf, koi8_to_unicode);
	SetStringToCharString(buffer, encoded_text, maxLen);
	
	
	// HACK instead of input box return last command
	/*
	static std::string lastText = "";
	static int textCounter = 0;
	std::string command = mainScreen.GetGameScreen()->GetLastCommand();
	if (lastText == command)
	{
		textCounter++;
		if (textCounter > 2)
		{
			textCounter = 0;
			if (!QSPOpenSavedGame((QSP_CHAR*)"autosave.sav", QSP_TRUE))
			{
				Message(ICON_INFORMATION, "", "Чтобы прервать бесконечный цикл ввода текста, квест будет перезапущен", 4000);
				QSPRestartGame(QSP_TRUE);
			}
			return;
		}
	}
	else
	{
		lastText = command;
		textCounter = 0;
	}
	
		
	std::string title;
	to_utf8((unsigned char *)text, &title, koi8_to_unicode);

	std::string encoded_command = utf8_to((const unsigned char *)command.c_str(), koi8_to_unicode);
	SetStringToCharString(buffer, encoded_command, maxLen);
	
	Message(ICON_INFORMATION, (char *)title.c_str(), (char *)command.c_str(), 2000);
	*/
}
 
void QSPCallbacks::ShowImage(const QSP_CHAR *file)
{
	if (file == 0 || strlen(file) == 0)
		return;
		
	std::string path; // = GetQuestPath();
	path += file;
	to_utf8((unsigned char *)path.c_str(), &path, koi8_to_unicode);
	ibitmap *image = OpenImage(path);
	if (image != 0)
	{
		mainScreen.GetGameScreen()->ShowImage(image);
	}
}

void QSPCallbacks::OpenGameStatus()
{
}

void QSPCallbacks::SaveGameStatus()
{
}

void QSPCallbacks::DeInit()
{
	for (int i = 0; i < DYN_MENU_SIZE; i++)
		delete [] dynamicMenu[i].text;
}

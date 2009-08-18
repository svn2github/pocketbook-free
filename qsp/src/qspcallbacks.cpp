#include "time.h"
#include "qspcallbacks.h"
#include "screens.h"

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

void QSPCallbacks::SetTimer(long msecs)
{
}

void QSPCallbacks::RefreshInt(QSP_BOOL isRedraw)
{
	mainScreen.UpdateUI();
}

void QSPCallbacks::SetInputStrText(const QSP_CHAR *text)
{
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
	switch (type)
	{
	case QSP_WIN_ACTS:
		break;
	case QSP_WIN_OBJS:
		break;
	case QSP_WIN_VARS:
		break;
	case QSP_WIN_INPUT:
		break;
	}
}

void QSPCallbacks::Sleep(long msecs)
{
	sleep(msecs/1000);
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
	Message(ICON_INFORMATION, "", (char *)str, 5000);
}

void QSPCallbacks::DeleteMenu()
{
}

void QSPCallbacks::AddMenuItem(const QSP_CHAR *name, const QSP_CHAR *imgPath)
{
}

void QSPCallbacks::ShowMenu()
{
}

QSP_CHAR *input_buffer;
void keyboard_entry(char *s)
{
	input_buffer = s;
}

void QSPCallbacks::Input(const QSP_CHAR *text, QSP_CHAR *buffer, long maxLen)
{
	//std::string title;
	//to_utf8((unsigned char *)text, &title, koi8_to_unicode);
	//input_buffer = buffer;
	buffer[0] = 0;
	//OpenKeyboard((char*)title.c_str(), (char*)buffer, maxLen, 0, keyboard_entry);
}
 
void QSPCallbacks::ShowImage(const QSP_CHAR *file)
{
}

void QSPCallbacks::OpenGameStatus()
{
}

void QSPCallbacks::SaveGameStatus()
{
}


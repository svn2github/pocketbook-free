#include "pthread.h"

#include "qspthread.h"

#include "screens.h"

extern MainScreen mainScreen;

pthread_t MainThread;
 
bool QSPEventProcessed;
int QSPEvent(0);
std::string QSPEventStr1;
int QSPEventInt1;
void SendQSPEvent(int type, std::string str1, int int1)
{
	QSPEventProcessed = false;
	QSPEventStr1 = str1;
	QSPEventInt1 = int1;
	QSPEvent = type;
	
	//QSPThreadProc(0);
	//while(!QSPEventProcessed) sleep(1);
}

void* QSPThreadProc(void *ptr) 
{
	while (QSPEvent != QSP_EVT_EXIT)
	{
		int QSPEvent1 = QSPEvent;
		switch (QSPEvent)
		{
			case QSP_EVT_OPENGAME:
				if (!QSPLoadGameWorld(QSPEventStr1.c_str()))
					ShowError();
				else
				{
					chdir(GetQuestPath().c_str());
					QSPRestartGame(QSP_TRUE);
				}
				break;
				
			case QSP_EVT_RESTART:
				if (!QSPRestartGame(QSP_TRUE))
					ShowError();
				break;
				
			case QSP_EVT_SAVEGAME:
				if (!QSPSaveGame((QSP_CHAR*)QSPEventStr1.c_str(), QSP_FALSE))
					ShowError();
				break;
				
			case QSP_EVT_OPENSAVEDGAME:
				if (!QSPOpenSavedGame((QSP_CHAR*)QSPEventStr1.c_str(), QSP_TRUE))
					ShowError();
				break;
				
			case QSP_EVT_EXECSTRING:
				if (!QSPExecString((const QSP_CHAR *)QSPEventStr1.c_str(), QSP_TRUE))
					ShowError();
				break;
				
			case QSP_EVT_EXECSELACTION:
				if (!QSPExecuteSelActionCode(QSP_TRUE))
					ShowError();
				break;
				
			case QSP_EVT_SETOBJINDEX:
				if (!QSPSetSelObjectIndex(QSPEventInt1, QSP_TRUE))
					ShowError();
				break;
			
			case QSP_EVT_SETUSERINPUT:
				QSPSetInputStrText(QSPEventStr1.c_str());
				break;
				
			case QSP_EVT_EXECUSERINPUT:
				if (!QSPExecUserInput(QSP_TRUE))
					ShowError();
				break;
				
			default:
				usleep(500);
		}
			
		if (QSPEvent != 0 && QSPEvent1 != 0)
		{
			QSPEvent = 0;
			QSPEventProcessed = true;
		}
	}
	
	return 0;
}

void QSPEventsTimer()
{
	int QSPEvent1 = QSPEvent;
	
	switch (QSPEvent)
	{
		case QSP_EVT_OPENGAME:
			if (!QSPLoadGameWorld(QSPEventStr1.c_str()))
				ShowError();
			else
			{
				chdir(GetQuestPath().c_str());
				QSPRestartGame(QSP_TRUE);
			}
			break;
			
		case QSP_EVT_RESTART:
			if (!QSPRestartGame(QSP_TRUE))
				ShowError();
			break;
			
		case QSP_EVT_SAVEGAME:
			if (!QSPSaveGame((QSP_CHAR*)QSPEventStr1.c_str(), QSP_FALSE))
				ShowError();
			break;
			
		case QSP_EVT_OPENSAVEDGAME:
			if (!QSPOpenSavedGame((QSP_CHAR*)QSPEventStr1.c_str(), QSP_TRUE))
				ShowError();
			break;
			
		case QSP_EVT_EXECSTRING:
			if (!QSPExecString((const QSP_CHAR *)QSPEventStr1.c_str(), QSP_TRUE))
				ShowError();
			break;
			
		case QSP_EVT_EXECSELACTION:
			if (!QSPExecuteSelActionCode(QSP_TRUE))
				ShowError();
			break;
			
		case QSP_EVT_EXECUSERINPUT:
			if (!QSPExecUserInput(QSP_TRUE))
				ShowError();
			break;
	}
		
	
	if (QSPEvent != QSP_EVT_EXIT)
		SetHardTimer("QSP_EVENTS_TIMER", QSPEventsTimer, 500);
	
	if (QSPEvent != 0 && QSPEvent1 != 0)
	{
		QSPEventProcessed = true;
		QSPEvent = 0;
	}
}

int IntEvent(0);
std::string IntEventStr1;
bool IntEventProcessed;

void SendIntEvent(int type, std::string str1)
{
	IntEvent = type;
	IntEventStr1 = str1;
	
	IntEventProcessed = false;
	//while(!IntEventProcessed) usleep(100);
}

void InterfaceEventsTimer()
{
	int IntEvent1 = IntEvent;
	switch (IntEvent)
	{
		case INT_EVT_UPDATE:
			IntEventProcessed = false;
			mainScreen.UpdateUI(false);
			IntEventProcessed = true;
			break;
	}
       
	if (IntEvent != 0 && IntEvent1 != 0)
		IntEvent = 0;
		
	SetHardTimer("INTERFACE_EVENTS_TIMER", InterfaceEventsTimer, 500);
}


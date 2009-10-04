#ifndef QSPTHREAD_H
#define QSPTHREAD_H

#include <stdlib.h>
#include "helper.h"
#include "qspcallbacks.h"
#include "qsp/qsp.h"

#define QSP_EVT_EXIT	101
#define QSP_EVT_OPENGAME	102
#define QSP_EVT_EXECSTRING	103
#define QSP_EVT_EXECSELACTION	104
#define QSP_EVT_RESTART	105
#define QSP_EVT_SAVEGAME	106
#define QSP_EVT_OPENSAVEDGAME	107
#define QSP_EVT_SETUSERINPUT	108
#define QSP_EVT_EXECUSERINPUT	109
#define QSP_EVT_SETOBJINDEX	110

#define INT_EVT_UPDATE 201
#define INT_EVT_MESSAGE 202

void SendQSPEvent(int type, std::string str1 = "", int int1 = 0);
void* QSPThreadProc(void *ptr);
void QSPEventsTimer();

void SendIntEvent(int type, std::string str1 = "");
void InterfaceEventsTimer();

#endif


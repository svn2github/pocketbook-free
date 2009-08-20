#ifndef QSPCALLBACKS_H
#define QSPCALLBACKS_H

#include <vector>
#include "convert.h"
#include "inkview.h"
#include "qsp/qsp.h"

#define DYN_MENU_SIZE 20

class QSPCallbacks
{
public:
	static void SetQSPCallbacks();
	static void RefreshInt(QSP_BOOL isRedraw);
	static void SetTimer(long msecs);
	static void SetInputStrText(const QSP_CHAR *text);
	static QSP_BOOL IsPlay(const QSP_CHAR *file);
	static void CloseFile(const QSP_CHAR *file);
	static void PlayFile(const QSP_CHAR *file, long volume);
	static void ShowPane(long type, QSP_BOOL isShow);
	static void Sleep(long msecs);
	static long GetMSCount();
	static void Msg(const QSP_CHAR *str);
	static void DeleteMenu();
	static void AddMenuItem(const QSP_CHAR *name, const QSP_CHAR *imgPath);
	static void ShowMenu();
	static void Input(const QSP_CHAR *text, QSP_CHAR *buffer, long maxLen);
	static void ShowImage(const QSP_CHAR *file);
	static void OpenGameStatus();
	static void SaveGameStatus();
	static void DeInit();
};

#endif


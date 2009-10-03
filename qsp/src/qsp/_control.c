/* Copyright (C) 2005-2009 Valeriy Argunov (nporep AT mail DOT ru) */
/*
* This library is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2.1 of the License, or
* (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "declarations.h"
#include "actions.h"
#include "callbacks.h"
#include "common.h"
#include "errors.h"
#include "game.h"
#include "locations.h"
#include "mathops.h"
#include "menu.h"
#include "objects.h"
#include "statements.h"
#include "text.h"
#include "time.h"
#include "variables.h"
#include "variant.h"

volatile QSP_BOOL qspIsMustWait = QSP_FALSE;

static void qspWait(QSP_BOOL);

static void qspWait(QSP_BOOL isBlock)
{
	while (qspIsMustWait) qspCallSleep(2);
	if (isBlock) qspIsMustWait = QSP_TRUE;
}
/* ------------------------------------------------------------ */
/* ���������� � ������ */

/* ������ */
const QSP_CHAR *QSPGetVersion()
{
	return QSP_VER;
}
/* ���� � ����� ���������� */
const QSP_CHAR *QSPGetCompiledDateTime()
{
	return QSP_FMT(__DATE__) QSP_FMT(", ") QSP_FMT(__TIME__);
}
/* ------------------------------------------------------------ */
/* ���������� ������ ���������� ������� */
long QSPGetFullRefreshCount()
{
	qspWait(QSP_FALSE);
	return qspFullRefreshCount;
}
/* ------------------------------------------------------------ */
/* ������ ���� � ������������ ����� ���� */
const QSP_CHAR *QSPGetQstFullPath()
{
	qspWait(QSP_FALSE);
	return qspQstFullPath;
}
/* ------------------------------------------------------------ */
/* �������� ������� ������� */
const QSP_CHAR *QSPGetCurLoc()
{
	QSP_CHAR *ret;
	qspWait(QSP_TRUE);
	ret = (qspCurLoc >= 0 ? qspLocs[qspCurLoc].Name : 0);
	qspIsMustWait = QSP_FALSE;
	return ret;
}
/* ------------------------------------------------------------ */
/* �������� �������� ������� */

/* ����� ��������� ���� �������� ������� */
const QSP_CHAR *QSPGetMainDesc()
{
	qspWait(QSP_FALSE);
	return qspCurDesc;
}
/* ����������� ��������� ������ ��������� �������� */
QSP_BOOL QSPIsMainDescChanged()
{
	qspWait(QSP_FALSE);
	return qspIsMainDescChanged;
}
/* ------------------------------------------------------------ */
/* �������������� �������� ������� */

/* ����� ��������������� ���� �������� ������� */
const QSP_CHAR *QSPGetVarsDesc()
{
	qspWait(QSP_FALSE);
	return qspCurVars;
}
/* ����������� ��������� ������ ��������������� �������� */
QSP_BOOL QSPIsVarsDescChanged()
{
	qspWait(QSP_FALSE);
	return qspIsVarsDescChanged;
}
/* ------------------------------------------------------------ */
/* �������� �������� ���������� ��������� */
QSP_BOOL QSPGetExprValue(const QSP_CHAR *expr, QSP_BOOL *isString, long *numVal, QSP_CHAR *strVal, long strValBufSize)
{
	QSPVariant v;
	qspWait(QSP_TRUE);
	qspResetError(QSP_TRUE);
	v = qspExprValue((QSP_CHAR *)expr);
	if (qspErrorNum)
	{
		qspIsMustWait = QSP_FALSE;
		return QSP_FALSE;
	}
	*isString = v.IsStr;
	if (v.IsStr)
	{
		QSP_STRNCPY(strVal, QSP_STR(v), strValBufSize - 1);
		free(QSP_STR(v));
		strVal[strValBufSize - 1] = 0;
	}
	else
		*numVal = QSP_NUM(v);
	qspIsMustWait = QSP_FALSE;
	return QSP_TRUE;
}
/* ------------------------------------------------------------ */
/* ����� ������ ����� */
void QSPSetInputStrText(const QSP_CHAR *val)
{
	qspWait(QSP_TRUE);
	qspCurInputLen = qspAddText(&qspCurInput, (QSP_CHAR *)val, 0, -1, QSP_FALSE);
	qspIsMustWait = QSP_FALSE;
}
/* ------------------------------------------------------------ */
/* ������ �������� */

/* ���������� �������� */
long QSPGetActionsCount()
{
	qspWait(QSP_FALSE);
	return qspCurActionsCount;
}
/* ������ �������� � ��������� �������� */
void QSPGetActionData(long ind, QSP_CHAR **image, QSP_CHAR **desc)
{
	qspWait(QSP_TRUE);
	if (ind >= 0 && ind < qspCurActionsCount)
	{
		*image = qspCurActions[ind].Image;
		*desc = qspCurActions[ind].Desc;
	}
	else
		*image = *desc = 0;
	qspIsMustWait = QSP_FALSE;
}
/* ���������� ���� ���������� �������� */
QSP_BOOL QSPExecuteSelActionCode(QSP_BOOL isRefresh)
{
	qspWait(QSP_TRUE);
	if (qspCurSelAction >= 0)
	{
		qspPrepareExecution();
		qspExecAction(qspCurSelAction);
		if (qspErrorNum)
		{
			qspIsMustWait = QSP_FALSE;
			return QSP_FALSE;
		}
		qspIsMustWait = QSP_FALSE;
		if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	}
	else
		qspIsMustWait = QSP_FALSE;
	return QSP_TRUE;
}
/* ���������� ������ ���������� �������� */
QSP_BOOL QSPSetSelActionIndex(long ind, QSP_BOOL isRefresh)
{
	qspWait(QSP_TRUE);
	if (ind >= 0 && ind < qspCurActionsCount && ind != qspCurSelAction)
	{
		qspCurSelAction = ind;
		qspPrepareExecution();
		qspExecLocByVarName(QSP_FMT("ONACTSEL"));
		if (qspErrorNum)
		{
			qspIsMustWait = QSP_FALSE;
			return QSP_FALSE;
		}
		qspIsMustWait = QSP_FALSE;
		if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	}
	else
		qspIsMustWait = QSP_FALSE;
	return QSP_TRUE;
}
/* �������� ������ ���������� �������� */
long QSPGetSelActionIndex()
{
	qspWait(QSP_FALSE);
	return qspCurSelAction;
}
/* ����������� ��������� ������ �������� */
QSP_BOOL QSPIsActionsChanged()
{
	qspWait(QSP_FALSE);
	return qspIsActionsChanged;
}
/* ------------------------------------------------------------ */
/* ������ �������� */

/* ���������� �������� */
long QSPGetObjectsCount()
{
	qspWait(QSP_FALSE);
	return qspCurObjectsCount;
}
/* ������ ������� � ��������� �������� */
void QSPGetObjectData(long ind, QSP_CHAR **image, QSP_CHAR **desc)
{
	qspWait(QSP_TRUE);
	if (ind >= 0 && ind < qspCurObjectsCount)
	{
		*image = qspCurObjects[ind].Image;
		*desc = qspCurObjects[ind].Desc;
	}
	else
		*image = *desc = 0;
	qspIsMustWait = QSP_FALSE;
}
/* ���������� ������ ���������� ������� */
QSP_BOOL QSPSetSelObjectIndex(long ind, QSP_BOOL isRefresh)
{
	qspWait(QSP_TRUE);
	if (ind >= 0 && ind < qspCurObjectsCount && ind != qspCurSelObject)
	{
		qspCurSelObject = ind;
		qspPrepareExecution();
		qspExecLocByVarName(QSP_FMT("ONOBJSEL"));
		if (qspErrorNum)
		{
			qspIsMustWait = QSP_FALSE;
			return QSP_FALSE;
		}
		qspIsMustWait = QSP_FALSE;
		if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	}
	else
		qspIsMustWait = QSP_FALSE;
	return QSP_TRUE;
}
/* �������� ������ ���������� ������� */
long QSPGetSelObjectIndex()
{
	qspWait(QSP_FALSE);
	return qspCurSelObject;
}
/* ����������� ��������� ������ �������� */
QSP_BOOL QSPIsObjectsChanged()
{
	qspWait(QSP_FALSE);
	return qspIsObjectsChanged;
}
/* ------------------------------------------------------------ */
/* ����� / ������� ���� */
void QSPShowWindow(long type, QSP_BOOL isShow)
{
	qspWait(QSP_TRUE);
	switch (type)
	{
	case QSP_WIN_ACTS:
		qspCurIsShowActs = isShow;
		break;
	case QSP_WIN_OBJS:
		qspCurIsShowObjs = isShow;
		break;
	case QSP_WIN_VARS:
		qspCurIsShowVars = isShow;
		break;
	case QSP_WIN_INPUT:
		qspCurIsShowInput = isShow;
		break;
	}
	qspIsMustWait = QSP_FALSE;
}
/* ------------------------------------------------------------ */
/* ���������� */

/* �������� ���������� ��������� ������� */
QSP_BOOL QSPGetVarValuesCount(const QSP_CHAR *name, long *count)
{
	QSPVar *var;
	qspWait(QSP_TRUE);
	qspResetError(QSP_TRUE);
	var = qspVarReference((QSP_CHAR *)name, QSP_FALSE);
	if (qspErrorNum)
	{
		qspIsMustWait = QSP_FALSE;
		return QSP_FALSE;
	}
	*count = var->ValsCount;
	qspIsMustWait = QSP_FALSE;
	return QSP_TRUE;
}
/* �������� �������� ���������� �������� ������� */
QSP_BOOL QSPGetVarValues(const QSP_CHAR *name, long ind, long *numVal, QSP_CHAR **strVal)
{
	QSPVar *var;
	qspWait(QSP_TRUE);
	qspResetError(QSP_TRUE);
	var = qspVarReference((QSP_CHAR *)name, QSP_FALSE);
	if (qspErrorNum)
	{
		qspIsMustWait = QSP_FALSE;
		return QSP_FALSE;
	}
	*numVal = var->Value[ind];
	*strVal = var->TextValue[ind];
	qspIsMustWait = QSP_FALSE;
	return QSP_TRUE;
}
/* �������� ������������ ���������� ���������� */
long QSPGetMaxVarsCount()
{
	return QSP_VARSCOUNT;
}
/* �������� ��� ���������� � ��������� �������� */
QSP_BOOL QSPGetVarNameByIndex(long index, QSP_CHAR **name)
{
	qspWait(QSP_TRUE);
	if (index < 0 || index >= QSP_VARSCOUNT || !qspVars[index].Name)
	{
		qspIsMustWait = QSP_FALSE;
		return QSP_FALSE;
	}
	*name = qspVars[index].Name;
	qspIsMustWait = QSP_FALSE;
	return QSP_TRUE;
}
/* ------------------------------------------------------------ */
/* ���������� ���� */

/* ���������� ������ ���� */
QSP_BOOL QSPExecString(const QSP_CHAR *s, QSP_BOOL isRefresh)
{
	qspWait(QSP_TRUE);
	qspPrepareExecution();
	qspExecStringAsCode((QSP_CHAR *)s);
	if (qspErrorNum)
	{
		qspIsMustWait = QSP_FALSE;
		return QSP_FALSE;
	}
	qspIsMustWait = QSP_FALSE;
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return QSP_TRUE;
}
/* ���������� ���� ��������� ������� */
QSP_BOOL QSPExecLocationCode(const QSP_CHAR *name, QSP_BOOL isRefresh)
{
	qspWait(QSP_TRUE);
	qspPrepareExecution();
	qspExecLocByName((QSP_CHAR *)name, QSP_FALSE);
	if (qspErrorNum)
	{
		qspIsMustWait = QSP_FALSE;
		return QSP_FALSE;
	}
	qspIsMustWait = QSP_FALSE;
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return QSP_TRUE;
}
/* ���������� ���� �������-�������� */
QSP_BOOL QSPExecCounter(QSP_BOOL isRefresh)
{
	if (!(qspIsMustWait || qspIsInCallBack))
	{
		qspIsMustWait = QSP_TRUE;
		qspPrepareExecution();
		qspExecLocByVarName(QSP_FMT("COUNTER"));
		if (qspErrorNum)
		{
			qspIsMustWait = QSP_FALSE;
			return QSP_FALSE;
		}
		qspIsMustWait = QSP_FALSE;
		if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	}
	return QSP_TRUE;
}
/* ���������� ���� �������-����������� ������ ����� */
QSP_BOOL QSPExecUserInput(QSP_BOOL isRefresh)
{
	qspWait(QSP_TRUE);
	qspPrepareExecution();
	qspExecLocByVarName(QSP_FMT("USERCOM"));
	if (qspErrorNum)
	{
		qspIsMustWait = QSP_FALSE;
		return QSP_FALSE;
	}
	qspIsMustWait = QSP_FALSE;
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return QSP_TRUE;
}
/* ------------------------------------------------------------ */
/* ������ */

/* �������� ���������� � ��������� ������ */
void QSPGetLastErrorData(long *errorNum, QSP_CHAR **errorLoc, long *errorWhere, long *errorLine)
{
	qspWait(QSP_TRUE);
	*errorNum = qspErrorNum;
	*errorLoc = (qspErrorLoc >= 0 && qspErrorLoc < qspLocsCount ? qspLocs[qspErrorLoc].Name : 0);
	*errorWhere = qspErrorWhere;
	*errorLine = qspErrorLine;
	qspIsMustWait = QSP_FALSE;
}
/* �������� �������� ������ �� �� ������ */
const QSP_CHAR *QSPGetErrorDesc(long errorNum)
{
	QSP_CHAR *str;
	switch (errorNum)
	{
	case QSP_ERR_DIVBYZERO: str = QSP_FMT("Division by zero!"); break;
	case QSP_ERR_TYPEMISMATCH: str = QSP_FMT("Type mismatch!"); break;
	case QSP_ERR_STACKOVERFLOW: str = QSP_FMT("Stack overflow!"); break;
	case QSP_ERR_TOOMANYITEMS: str = QSP_FMT("Too many items in expression!"); break;
	case QSP_ERR_FILENOTFOUND: str = QSP_FMT("File not found!"); break;
	case QSP_ERR_CANTLOADFILE: str = QSP_FMT("Can't load file!"); break;
	case QSP_ERR_GAMENOTLOADED: str = QSP_FMT("Game not loaded!"); break;
	case QSP_ERR_COLONNOTFOUND: str = QSP_FMT("Sign [:] not found!"); break;
	case QSP_ERR_CANTINCFILE: str = QSP_FMT("Can't add file!"); break;
	case QSP_ERR_CANTADDACTION: str = QSP_FMT("Can't add action!"); break;
	case QSP_ERR_EQNOTFOUND: str = QSP_FMT("Sign [=] not found!"); break;
	case QSP_ERR_LOCNOTFOUND: str = QSP_FMT("Location not found!"); break;
	case QSP_ERR_ENDNOTFOUND: str = QSP_FMT("[end] not found!"); break;
	case QSP_ERR_LABELNOTFOUND: str = QSP_FMT("Label not found!"); break;
	case QSP_ERR_NOTCORRECTNAME: str = QSP_FMT("Incorrect variable's name!"); break;
	case QSP_ERR_QUOTNOTFOUND: str = QSP_FMT("Quote not found!"); break;
	case QSP_ERR_BRACKNOTFOUND: str = QSP_FMT("Bracket not found!"); break;
	case QSP_ERR_BRACKSNOTFOUND: str = QSP_FMT("Brackets not found!"); break;
	case QSP_ERR_SYNTAX: str = QSP_FMT("Syntax error!"); break;
	case QSP_ERR_UNKNOWNACTION: str = QSP_FMT("Unknown action!"); break;
	case QSP_ERR_ARGSCOUNT: str = QSP_FMT("Incorrect arguments' count!"); break;
	case QSP_ERR_CANTADDOBJECT: str = QSP_FMT("Can't add object!"); break;
	case QSP_ERR_CANTADDMENUITEM: str = QSP_FMT("Can't add menu's item!"); break;
	case QSP_ERR_TOOMANYVARS: str = QSP_FMT("Too many variables!"); break;
	case QSP_ERR_INCORRECTREGEXP: str = QSP_FMT("Regular expression's error!"); break;
	default: str = QSP_FMT("Unknown error!"); break;
	}
	return str;
}
/* ------------------------------------------------------------ */
/* ���������� ����� */

/* �������� ����� ���� */
QSP_BOOL QSPLoadGameWorld(const QSP_CHAR *fileName)
{
	qspWait(QSP_TRUE);
	qspResetError(QSP_TRUE);
	qspOpenQuest((QSP_CHAR *)fileName, QSP_FALSE);
	if (qspErrorNum)
	{
		qspIsMustWait = QSP_FALSE;
		return QSP_FALSE;
	}
	qspIsMustWait = QSP_FALSE;
	return QSP_TRUE;
}
/* ���������� ��������� */
QSP_BOOL QSPSaveGame(const QSP_CHAR *fileName, QSP_BOOL isRefresh)
{
	qspWait(QSP_TRUE);
	qspPrepareExecution();
	qspSaveGameStatus((QSP_CHAR *)fileName);
	if (qspErrorNum)
	{
		qspIsMustWait = QSP_FALSE;
		return QSP_FALSE;
	}
	qspIsMustWait = QSP_FALSE;
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return QSP_TRUE;
}
/* �������� ��������� */
QSP_BOOL QSPOpenSavedGame(const QSP_CHAR *fileName, QSP_BOOL isRefresh)
{
	qspWait(QSP_TRUE);
	qspPrepareExecution();
	qspOpenGameStatus((QSP_CHAR *)fileName);
	if (qspErrorNum)
	{
		qspIsMustWait = QSP_FALSE;
		return QSP_FALSE;
	}
	qspIsMustWait = QSP_FALSE;
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return QSP_TRUE;
}
/* ���������� ���� */
QSP_BOOL QSPRestartGame(QSP_BOOL isRefresh)
{
	qspWait(QSP_TRUE);
	qspPrepareExecution();
	qspNewGame(QSP_TRUE);
	if (qspErrorNum)
	{
		qspIsMustWait = QSP_FALSE;
		return QSP_FALSE;
	}
	qspIsMustWait = QSP_FALSE;
	if (isRefresh) qspCallRefreshInt(QSP_FALSE);
	return QSP_TRUE;
}
/* ------------------------------------------------------------ */
/* ���� */
/* �-� ������������� ������ ��� ������ �� CallBack'� QSP_CALL_SHOWMENU */
void QSPSelectMenuItem(long index)
{
	QSPVariant arg;
	if (index >= 0 && index < qspCurMenuItems)
	{
		arg.IsStr = QSP_FALSE;
		QSP_NUM(arg) = index + 1;
		qspExecLocByNameWithArgs(qspCurMenuLocs[index], &arg, 1);
	}
}
/* ------------------------------------------------------------ */
/* ��������� CALLBACK'�� */
void QSPSetCallBack(long type, QSP_CALLBACK func)
{
	qspWait(QSP_TRUE);
	qspSetCallBack(type, func);
	qspIsMustWait = QSP_FALSE;
}
/* ------------------------------------------------------------ */
/* ������������� */
void QSPInit()
{
	#ifdef _DEBUG
		mwInit();
	#endif
	qspIsMustWait = QSP_FALSE;
	qspRefreshCount = qspFullRefreshCount = 0;
	qspQstPath = qspQstFullPath = 0;
	qspQstPathLen = 0;
	qspQstCRC = 0;
	qspMSCount = 0;
	qspLocs = 0;
	qspLocsNames = 0;
	qspLocsCount = 0;
	qspCurLoc = -1;
	qspTimerInterval = 0;
	qspCurIsShowObjs = qspCurIsShowActs = qspCurIsShowVars = qspCurIsShowInput = QSP_TRUE;
	setlocale(LC_ALL, QSP_LOCALE);
	qspSetSeed(0);
	qspPrepareExecution();
	qspMemClear(QSP_TRUE);
	qspInitCallBacks();
	qspInitStats();
	qspInitMath();
}
/* ��������������� */
void QSPDeInit()
{
	qspWait(QSP_TRUE);
	qspMemClear(QSP_FALSE);
	qspCreateWorld(0, 0);
	if (qspQstPath) free(qspQstPath);
	if (qspQstFullPath) free(qspQstFullPath);
	qspIsMustWait = QSP_FALSE;
	#ifdef _DEBUG
		mwTerm();
	#endif
}

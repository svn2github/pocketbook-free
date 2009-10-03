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

#include "locations.h"
#include "common.h"
#include "errors.h"
#include "game.h"
#include "statements.h"
#include "text.h"
#include "variables.h"

QSPLocation *qspLocs = 0;
QSPLocName *qspLocsNames = 0;
long qspLocsCount = 0;
long qspCurLoc = -1;
long qspRefreshCount = 0;
long qspFullRefreshCount = 0;

static int qspLocsCompare(const void *, const void *);
static int qspLocStringCompare(const void *, const void *);

static int qspLocsCompare(const void *locName1, const void *locName2)
{
	return QSP_STRCMP(((QSPLocName *)locName1)->Name, ((QSPLocName *)locName2)->Name);
}

static int qspLocStringCompare(const void *name, const void *compareTo)
{
	return QSP_STRCMP((QSP_CHAR *)name, ((QSPLocName *)compareTo)->Name);
}

void qspCreateWorld(long start, long locsCount)
{
	long i, j;
	for (i = start; i < qspLocsCount; ++i)
	{
		free(qspLocsNames[i].Name);
		free(qspLocs[i].Name);
		free(qspLocs[i].Desc);
		qspFreeStrs(qspLocs[i].OnVisitLines, qspLocs[i].OnVisitLinesCount, QSP_FALSE);
		for (j = 0; j < QSP_MAXACTIONS; ++j)
			if (qspLocs[i].Actions[j].Desc)
			{
				if (qspLocs[i].Actions[j].Image) free(qspLocs[i].Actions[j].Image);
				free(qspLocs[i].Actions[j].Desc);
				qspFreeStrs(qspLocs[i].Actions[j].OnPressLines, qspLocs[i].Actions[j].OnPressLinesCount, QSP_FALSE);
			}
	}
	if (qspLocsCount != locsCount)
	{
		qspLocsCount = locsCount;
		qspLocs = (QSPLocation *)realloc(qspLocs, qspLocsCount * sizeof(QSPLocation));
		qspLocsNames = (QSPLocName *)realloc(qspLocsNames, qspLocsCount * sizeof(QSPLocName));
	}
	for (i = start; i < qspLocsCount; ++i)
	{
		qspLocsNames[i].Name = 0;
		for (j = 0; j < QSP_MAXACTIONS; ++j)
			qspLocs[i].Actions[j].Desc = 0;
	}
}

void qspPrepareLocs()
{
	long i;
	for (i = 0; i < qspLocsCount; ++i)
	{
		if (qspLocsNames[i].Name) free(qspLocsNames[i].Name);
		qspLocsNames[i].Index = i;
		qspUpperStr(qspLocsNames[i].Name = qspGetNewText(qspLocs[i].Name, -1));
	}
	qsort(qspLocsNames, qspLocsCount, sizeof(QSPLocName), qspLocsCompare);
}

long qspLocIndex(QSP_CHAR *name)
{
	QSPLocName *loc;
	QSP_CHAR *uName;
	if (!qspLocsCount) return -1;
	uName = qspDelSpc(name);
	if (!(*uName))
	{
		free(uName);
		return -1;
	}
	qspUpperStr(uName);
	loc = (QSPLocName *)bsearch(uName, qspLocsNames, qspLocsCount, sizeof(QSPLocName), qspLocStringCompare);
	free(uName);
	if (loc) return loc->Index;
	return -1;
}

void qspExecLocByIndex(long locInd, QSP_BOOL isChangeDesc)
{
	QSPVariant args[2];
	QSP_CHAR *str, **code;
	long oldRefreshCount, i, count, oldLoc, oldWhere, oldLine;
	QSPLocation *loc = qspLocs + locInd;
	oldLoc = qspRealCurLoc;
	oldWhere = qspRealWhere;
	oldLine = qspRealLine;
	qspRealCurLoc = locInd;
	qspRealWhere = QSP_AREA_ONLOCVISIT;
	qspRealLine = 0;
	oldRefreshCount = qspRefreshCount;
	str = qspFormatText(loc->Desc);
	if (qspRefreshCount != oldRefreshCount || qspErrorNum)
	{
		qspRealLine = oldLine;
		qspRealWhere = oldWhere;
		qspRealCurLoc = oldLoc;
		return;
	}
	if (isChangeDesc)
	{
		if (qspCurDesc) free(qspCurDesc);
		qspCurDescLen = (long)QSP_STRLEN(qspCurDesc = str);
		qspIsMainDescChanged = QSP_TRUE;
	}
	else
	{
		if (*str)
		{
			qspCurDescLen = qspAddText(&qspCurDesc, str, qspCurDescLen, -1, QSP_FALSE);
			qspIsMainDescChanged = QSP_TRUE;
		}
		free(str);
	}
	qspRealWhere = QSP_AREA_ONLOCACTION;
	for (i = 0; i < QSP_MAXACTIONS; ++i)
	{
		str = loc->Actions[i].Desc;
		if (!(str && *str)) break;
		str = qspFormatText(str);
		if (qspRefreshCount != oldRefreshCount || qspErrorNum)
		{
			qspRealLine = oldLine;
			qspRealWhere = oldWhere;
			qspRealCurLoc = oldLoc;
			return;
		}
		args[0].IsStr = QSP_TRUE;
		QSP_STR(args[0]) = str;
		str = loc->Actions[i].Image;
		if (str && *str)
		{
			args[1].IsStr = QSP_TRUE;
			QSP_STR(args[1]) = str;
			count = 2;
		}
		else
			count = 1;
		qspAddAction(args, count, loc->Actions[i].OnPressLines, 0, loc->Actions[i].OnPressLinesCount, QSP_TRUE);
		free(QSP_STR(args[0]));
		if (qspErrorNum)
		{
			qspRealLine = oldLine;
			qspRealWhere = oldWhere;
			qspRealCurLoc = oldLoc;
			return;
		}
	}
	qspRealWhere = QSP_AREA_ONLOCVISIT;
	if (locInd < qspLocsCount - qspCurIncLocsCount)
		qspExecCode(loc->OnVisitLines, 0, loc->OnVisitLinesCount, 1, 0);
	else
	{
		count = loc->OnVisitLinesCount;
		qspCopyStrs(&code, loc->OnVisitLines, 0, count);
		qspExecCode(code, 0, count, 1, 0);
		qspFreeStrs(code, count, QSP_FALSE);
	}
	qspRealLine = oldLine;
	qspRealWhere = oldWhere;
	qspRealCurLoc = oldLoc;
}

void qspExecLocByName(QSP_CHAR *name, QSP_BOOL isChangeDesc)
{
	long locInd = qspLocIndex(name);
	if (locInd < 0)
	{
		qspSetError(QSP_ERR_LOCNOTFOUND);
		return;
	}
	qspExecLocByIndex(locInd, isChangeDesc);
}

void qspExecLocByVarName(QSP_CHAR *name)
{
	QSP_CHAR *locName = qspGetVarStrValue(name);
	if (qspIsAnyString(locName)) qspExecLocByName(locName, QSP_FALSE);
}

void qspExecLocByNameWithArgs(QSP_CHAR *name, QSPVariant *args, long count)
{
	QSPVar local, *var;
	long oldRefreshCount;
	if (!(var = qspVarReference(QSP_VARARGS, QSP_TRUE))) return;
	qspMoveVar(&local, var);
	qspSetArgs(var, args, count);
	oldRefreshCount = qspRefreshCount;
	qspExecLocByName(name, QSP_FALSE);
	if (qspRefreshCount != oldRefreshCount || qspErrorNum)
	{
		qspEmptyVar(&local);
		return;
	}
	if (!(var = qspVarReference(QSP_VARARGS, QSP_TRUE)))
	{
		qspEmptyVar(&local);
		return;
	}
	qspEmptyVar(var);
	qspMoveVar(var, &local);
}

void qspExecLocByVarNameWithArgs(QSP_CHAR *name, QSPVariant *args, long count)
{
	QSP_CHAR *locName = qspGetVarStrValue(name);
	if (qspIsAnyString(locName)) qspExecLocByNameWithArgs(locName, args, count);
}

void qspRefreshCurLoc(QSP_BOOL isChangeDesc)
{
	long oldRefreshCount;
	qspClearActions(QSP_FALSE);
	++qspRefreshCount;
	if (isChangeDesc) ++qspFullRefreshCount;
	oldRefreshCount = qspRefreshCount;
	qspExecLocByIndex(qspCurLoc, isChangeDesc);
	if (qspErrorNum) return;
	if (qspRefreshCount == oldRefreshCount)
		qspExecLocByVarName(QSP_FMT("ONNEWLOC"));
}

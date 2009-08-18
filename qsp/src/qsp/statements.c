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

#include "statements.h"
#include "actions.h"
#include "callbacks.h"
#include "codetools.h"
#include "common.h"
#include "errors.h"
#include "game.h"
#include "locations.h"
#include "mathops.h"
#include "menu.h"
#include "objects.h"
#include "playlist.h"
#include "text.h"
#include "variables.h"

QSPStatement qspStats[qspStatLast_Statement];
long qspStatMaxLen = 0;

static void qspAddStatement(long, QSP_CHAR *, QSP_CHAR *, char, QSP_STATEMENT, long, long, ...);
static long qspGetStatCode(QSP_CHAR *, QSP_BOOL, QSP_CHAR **);
static long qspSearchElse(QSP_CHAR **, long, long);
static long qspSearchEnd(QSP_CHAR **, long, long);
static long qspSearchLabel(QSP_CHAR **, long, long, QSP_CHAR *);
static QSP_BOOL qspExecString(QSP_CHAR *, QSP_CHAR **);
static QSP_BOOL qspStatementIf(QSP_CHAR *, QSP_CHAR **);
static QSP_BOOL qspStatementAddText(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementClear(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementExit(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementGoSub(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementGoTo(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementJump(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementWait(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementSetTimer(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementShowWin(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementRefInt(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementView(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementMsg(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementExec(QSPVariant *, long, QSP_CHAR **, char);
static QSP_BOOL qspStatementDynamic(QSPVariant *, long, QSP_CHAR **, char);

static void qspAddStatement(long statCode,
							QSP_CHAR *statName,
							QSP_CHAR *statAltName,
							char extArg,
							QSP_STATEMENT func,
							long minArgs,
							long maxArgs,
							...)
{
	long i;
	va_list marker;
	qspStats[statCode].Names[0] = statName;
	qspStats[statCode].Names[1] = statAltName;
	qspStats[statCode].NamesLens[0] = (long)QSP_STRLEN(statName);
	qspStats[statCode].NamesLens[1] = (statAltName ? (long)QSP_STRLEN(statAltName) : 0);
	qspStats[statCode].ExtArg = extArg;
	qspStats[statCode].Func = func;
	qspStats[statCode].MinArgsCount = minArgs;
	qspStats[statCode].MaxArgsCount = maxArgs;
	if (maxArgs > 0)
	{
		va_start(marker, maxArgs);
		for (i = 0; i < maxArgs; ++i)
			qspStats[statCode].ArgsTypes[i] = va_arg(marker, int);
		va_end(marker);
	}
	/* Max length */
	for (i = 0; i < 2; ++i)
		if (qspStats[statCode].NamesLens[i] > qspStatMaxLen)
			qspStatMaxLen = qspStats[statCode].NamesLens[i];
}

void qspInitStats()
{
	/*
	Format:
		qspAddStatement(
			Statement,
			Name,
			Alternative Name,
			Extended Argument,
			Statement's Function,
			Minimum Arguments' Count,
			Maximum Arguments' Count,
			Arguments' Types [optional]
		);

		"Arguments' Types":
		0 - Unknown / Any
		1 - String
		2 - Number
	*/
	qspStatMaxLen = 0;
	qspAddStatement(qspStatElse, QSP_FMT("ELSE"), 0, 0, 0, 0, 0);
	qspAddStatement(qspStatEnd, QSP_FMT("END"), 0, 0, 0, 0, 0);
	qspAddStatement(qspStatSet, QSP_FMT("SET"), QSP_FMT("LET"), 0, 0, 0, 0);
	qspAddStatement(qspStatIf, QSP_FMT("IF"), 0, 0, 0, 1, 1, 2);
	qspAddStatement(qspStatAct, QSP_FMT("ACT"), 0, 0, 0, 1, 2, 1, 1);
	qspAddStatement(qspStatAddObj, QSP_FMT("ADDOBJ"), QSP_FMT("ADD OBJ"), 0, qspStatementAddObject, 1, 2, 1, 1);
	qspAddStatement(qspStatAddQst, QSP_FMT("ADDQST"), 0, 1, qspStatementOpenQst, 1, 1, 1);
	qspAddStatement(qspStatClA, QSP_FMT("CLA"), 0, 3, qspStatementClear, 0, 0);
	qspAddStatement(qspStatCloseAll, QSP_FMT("CLOSE ALL"), 0, 1, qspStatementCloseFile, 0, 0);
	qspAddStatement(qspStatClose, QSP_FMT("CLOSE"), 0, 0, qspStatementCloseFile, 0, 1, 1);
	qspAddStatement(qspStatClS, QSP_FMT("CLS"), 0, 4, qspStatementClear, 0, 0);
	qspAddStatement(qspStatCmdClear, QSP_FMT("CMDCLEAR"), QSP_FMT("CMDCLR"), 2, qspStatementClear, 0, 0);
	qspAddStatement(qspStatCopyArr, QSP_FMT("COPYARR"), 0, 0, qspStatementCopyArr, 2, 2, 1, 1);
	qspAddStatement(qspStatDelAct, QSP_FMT("DELACT"), QSP_FMT("DEL ACT"), 0, qspStatementDelAct, 1, 1, 1);
	qspAddStatement(qspStatDelObj, QSP_FMT("DELOBJ"), QSP_FMT("DEL OBJ"), 0, qspStatementDelObj, 1, 1, 1);
	qspAddStatement(qspStatDynamic, QSP_FMT("DYNAMIC"), 0, 0, qspStatementDynamic, 1, 1, 1);
	qspAddStatement(qspStatExec, QSP_FMT("EXEC"), 0, 0, qspStatementExec, 1, 1, 1);
	qspAddStatement(qspStatExit, QSP_FMT("EXIT"), 0, 0, qspStatementExit, 0, 0);
	qspAddStatement(qspStatGoSub, QSP_FMT("GOSUB"), QSP_FMT("GS"), 0, qspStatementGoSub, 1, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	qspAddStatement(qspStatGoTo, QSP_FMT("GOTO"), QSP_FMT("GT"), 1, qspStatementGoTo, 1, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	qspAddStatement(qspStatJump, QSP_FMT("JUMP"), 0, 0, qspStatementJump, 1, 1, 1);
	qspAddStatement(qspStatKillAll, QSP_FMT("KILLALL"), 0, 5, qspStatementClear, 0, 0);
	qspAddStatement(qspStatKillObj, QSP_FMT("KILLOBJ"), 0, 1, qspStatementDelObj, 0, 1, 2);
	qspAddStatement(qspStatKillQst, QSP_FMT("KILLQST"), 0, 6, qspStatementClear, 0, 0);
	qspAddStatement(qspStatKillVar, QSP_FMT("KILLVAR"), 0, 0, qspStatementKillVar, 0, 2, 1, 2);
	qspAddStatement(qspStatMenu, QSP_FMT("MENU"), 0, 0, qspStatementShowMenu, 1, 1, 1);
	qspAddStatement(qspStatMClear, QSP_FMT("*CLEAR"), QSP_FMT("*CLR"), 1, qspStatementClear, 0, 0);
	qspAddStatement(qspStatMNL, QSP_FMT("*NL"), 0, 5, qspStatementAddText, 0, 1, 1);
	qspAddStatement(qspStatMPL, QSP_FMT("*PL"), 0, 3, qspStatementAddText, 0, 1, 1);
	qspAddStatement(qspStatMP, QSP_FMT("*P"), 0, 1, qspStatementAddText, 1, 1, 1);
	qspAddStatement(qspStatClear, QSP_FMT("CLEAR"), QSP_FMT("CLR"), 0, qspStatementClear, 0, 0);
	qspAddStatement(qspStatNL, QSP_FMT("NL"), 0, 4, qspStatementAddText, 0, 1, 1);
	qspAddStatement(qspStatPL, QSP_FMT("PL"), 0, 2, qspStatementAddText, 0, 1, 1);
	qspAddStatement(qspStatP, QSP_FMT("P"), 0, 0, qspStatementAddText, 1, 1, 1);
	qspAddStatement(qspStatMsg, QSP_FMT("MSG"), 0, 0, qspStatementMsg, 1, 1, 1);
	qspAddStatement(qspStatOpenGame, QSP_FMT("OPENGAME"), 0, 0, qspStatementOpenGame, 0, 1, 1);
	qspAddStatement(qspStatOpenQst, QSP_FMT("OPENQST"), 0, 0, qspStatementOpenQst, 1, 1, 1);
	qspAddStatement(qspStatPlay, QSP_FMT("PLAY"), 0, 0, qspStatementPlayFile, 1, 2, 1, 2);
	qspAddStatement(qspStatRefInt, QSP_FMT("REFINT"), 0, 0, qspStatementRefInt, 0, 0);
	qspAddStatement(qspStatSaveGame, QSP_FMT("SAVEGAME"), 0, 0, qspStatementSaveGame, 0, 1, 1);
	qspAddStatement(qspStatSetTimer, QSP_FMT("SETTIMER"), 0, 0, qspStatementSetTimer, 1, 1, 2);
	qspAddStatement(qspStatShowActs, QSP_FMT("SHOWACTS"), 0, 0, qspStatementShowWin, 1, 1, 2);
	qspAddStatement(qspStatShowInput, QSP_FMT("SHOWINPUT"), 0, 3, qspStatementShowWin, 1, 1, 2);
	qspAddStatement(qspStatShowObjs, QSP_FMT("SHOWOBJS"), 0, 1, qspStatementShowWin, 1, 1, 2);
	qspAddStatement(qspStatShowVars, QSP_FMT("SHOWSTAT"), 0, 2, qspStatementShowWin, 1, 1, 2);
	qspAddStatement(qspStatUnSelect, QSP_FMT("UNSELECT"), QSP_FMT("UNSEL"), 0, qspStatementUnSelect, 0, 0);
	qspAddStatement(qspStatView, QSP_FMT("VIEW"), 0, 0, qspStatementView, 0, 1, 1);
	qspAddStatement(qspStatWait, QSP_FMT("WAIT"), 0, 0, qspStatementWait, 1, 1, 2);
	qspAddStatement(qspStatXGoTo, QSP_FMT("XGOTO"), QSP_FMT("XGT"), 0, qspStatementGoTo, 1, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static long qspGetStatCode(QSP_CHAR *s, QSP_BOOL isMultiline, QSP_CHAR **pos)
{
	long i, j, len, last;
	QSP_CHAR *uStr;
	if (!(*s)) return qspStatUnknown;
	if (*s == QSP_LABEL[0]) return qspStatLabel;
	if (*s == QSP_COMMENT[0]) return qspStatComment;
	/* ------------------------------------------------------------------ */
	last = (isMultiline ? qspStatFirst_NotMultilineStatement : qspStatLast_Statement);
	qspUpperStr(uStr = qspGetNewText(s, qspStatMaxLen));
	for (i = qspStatFirst_Statement; i < last; ++i)
	{
		for (j = 0; j < 2; ++j)
			if (qspStats[i].Names[j])
			{
				len = qspStats[i].NamesLens[j];
				if (qspIsEqual(uStr, qspStats[i].Names[j], len) && qspIsInListEOL(QSP_DELIMS, s[len]))
				{
					if (pos) *pos = s + len;
					free(uStr);
					return i;
				}
			}
	}
	free(uStr);
	return qspStatUnknown;
}

static long qspSearchElse(QSP_CHAR **s, long start, long end)
{
	long c = 1;
	while (start < end)
	{
		switch (qspGetStatCode(s[start], QSP_TRUE, 0))
		{
		case qspStatAct:
		case qspStatIf:
			if (*(qspStrEnd(s[start]) - 1) == QSP_COLONDELIM[0]) ++c;
			break;
		case qspStatElse:
			if (c == 1) return start;
			break;
		case qspStatEnd:
			if (!(--c)) return -1;
			break;
		}
		++start;
	}
	return -1;
}

static long qspSearchEnd(QSP_CHAR **s, long start, long end)
{
	long c = 1;
	while (start < end)
	{
		switch (qspGetStatCode(s[start], QSP_TRUE, 0))
		{
		case qspStatAct:
		case qspStatIf:
			if (*(qspStrEnd(s[start]) - 1) == QSP_COLONDELIM[0]) ++c;
			break;
		case qspStatEnd:
			if (!(--c)) return start;
			break;
		}
		++start;
	}
	return -1;
}

static long qspSearchLabel(QSP_CHAR **s, long start, long end, QSP_CHAR *str)
{
	QSP_CHAR *buf, *pos;
	while (start < end)
	{
		if (*s[start] == QSP_LABEL[0])
		{
			pos = QSP_STRCHR(s[start], QSP_STATDELIM[0]);
			if (pos)
			{
				*pos = 0;
				buf = qspDelSpc(s[start] + 1);
				*pos = QSP_STATDELIM[0];
			}
			else
				buf = qspDelSpc(s[start] + 1);
			qspUpperStr(buf);
			if (!QSP_STRCMP(buf, str))
			{
				free(buf);
				return start;
			}
			free(buf);
		}
		++start;
	}
	return -1;
}

long qspGetStatArgs(QSP_CHAR *s, long statCode, QSPVariant *args)
{
	QSP_CHAR *pos;
	char type;
	long count = 0, oldRefreshCount = qspRefreshCount;
	while (1)
	{
		s = qspSkipSpaces(s);
		if (!(*s))
		{
			if (count) qspSetError(QSP_ERR_SYNTAX);
			break;
		}
		if (count == qspStats[statCode].MaxArgsCount)
		{
			qspSetError(QSP_ERR_ARGSCOUNT);
			break;
		}
		pos = qspStrPos(s, QSP_COMMA, QSP_FALSE);
		if (pos)
		{
			*pos = 0;
			args[count] = qspExprValue(s);
			*pos = QSP_COMMA[0];
		}
		else
			args[count] = qspExprValue(s);
		if (qspRefreshCount != oldRefreshCount || qspErrorNum) break;
		type = qspStats[statCode].ArgsTypes[count];
		if (type && qspConvertVariantTo(args + count, type == 1))
		{
			qspSetError(QSP_ERR_TYPEMISMATCH);
			++count;
			break;
		}
		++count;
		if (!pos) break;
		s = pos + QSP_LEN(QSP_COMMA);
	}
	if (qspRefreshCount != oldRefreshCount || qspErrorNum)
	{
		qspFreeVariants(args, count);
		return 0;
	}
	if (count < qspStats[statCode].MinArgsCount)
	{
		qspSetError(QSP_ERR_ARGSCOUNT);
		qspFreeVariants(args, count);
		return 0;
	}
	return count;
}

static QSP_BOOL qspExecString(QSP_CHAR *s, QSP_CHAR **jumpTo)
{
	QSPVariant args[QSP_STATMAXARGS];
	long oldRefreshCount, statCode, count;
	QSP_BOOL isExit;
	QSP_CHAR *pos, *paramPos;
	s = qspSkipSpaces(s);
	if (!(*s)) return QSP_FALSE;
	pos = qspStrPos(s, QSP_STATDELIM, QSP_FALSE);
	statCode = qspGetStatCode(s, QSP_FALSE, &paramPos);
	if (pos)
	{
		switch (statCode)
		{
		case qspStatComment:
		case qspStatAct:
		case qspStatIf:
			break;
		default:
			oldRefreshCount = qspRefreshCount;
			*pos = 0;
			isExit = qspExecString(s, jumpTo);
			*pos = QSP_STATDELIM[0];
			if (isExit || qspRefreshCount != oldRefreshCount || qspErrorNum || **jumpTo) return isExit;
			return qspExecString(pos + 1, jumpTo);
		}
	}
	switch (statCode)
	{
	case qspStatLabel:
	case qspStatComment:
	case qspStatElse:
	case qspStatEnd:
		return QSP_FALSE;
	case qspStatUnknown:
		statCode = (qspStrPos(s, QSP_EQUAL, QSP_FALSE) ? qspStatSet : qspStatMPL);
		paramPos = s;
	default:
		switch (statCode)
		{
		case qspStatAct:
			qspStatementAddAct(paramPos);
			break;
		case qspStatIf:
			return qspStatementIf(paramPos, jumpTo);
		case qspStatSet:
			qspStatementSetVarValue(paramPos);
			break;
		default:
			oldRefreshCount = qspRefreshCount;
			count = qspGetStatArgs(paramPos, statCode, args);
			if (qspRefreshCount != oldRefreshCount || qspErrorNum) break;
			isExit = qspStats[statCode].Func(args, count, jumpTo, qspStats[statCode].ExtArg);
			qspFreeVariants(args, count);
			return isExit;
		}
		return QSP_FALSE;
	}
}

QSP_BOOL qspExecCode(QSP_CHAR **s, long startLine, long endLine, long codeOffset, QSP_CHAR **jumpTo, QSP_BOOL uLevel)
{
	QSPVariant args[2];
	QSP_CHAR *jumpToFake, *pos, *paramPos;
	long i, statCode, count, endPos, elsePos, oldRefreshCount;
	QSP_BOOL isExit = QSP_FALSE;
	oldRefreshCount = qspRefreshCount;
	/* Prepare temporary data */
	if (uLevel)
	{
		jumpToFake = qspGetNewText(QSP_FMT(""), 0);
		jumpTo = &jumpToFake;
	}
	/* Code execution */
	i = startLine;
	while (i < endLine)
	{
		if (codeOffset > 0) qspRealLine = i + codeOffset;
		statCode = qspGetStatCode(s[i], QSP_TRUE, &paramPos);
		if (statCode == qspStatAct || statCode == qspStatIf)
		{
			pos = qspStrEnd(s[i]) - 1;
			if (*pos == QSP_COLONDELIM[0]) /* Multiline */
			{
				endPos = qspSearchEnd(s, ++i, endLine);
				if (endPos < 0)
				{
					qspSetError(QSP_ERR_ENDNOTFOUND);
					break;
				}
				*pos = 0;
				count = qspGetStatArgs(paramPos, statCode, args);
				*pos = QSP_COLONDELIM[0];
				if (qspRefreshCount != oldRefreshCount || qspErrorNum) break;
				if (statCode == qspStatAct)
				{
					qspAddAction(args, count, s, i, endPos, QSP_TRUE);
					qspFreeVariants(args, count);
					if (qspErrorNum) break;
					i = endPos;
				}
				else if (statCode == qspStatIf)
				{
					elsePos = qspSearchElse(s, i, endLine);
					if (QSP_NUM(args[0]))
					{
						if (elsePos >= 0)
						{
							isExit = qspExecCode(s, i, elsePos, codeOffset, jumpTo, QSP_FALSE);
							if (isExit || qspRefreshCount != oldRefreshCount || qspErrorNum) break;
							if (**jumpTo)
							{
								i = qspSearchLabel(s, startLine, endLine, *jumpTo);
								if (i < 0)
								{
									if (uLevel) qspSetError(QSP_ERR_LABELNOTFOUND);
									break;
								}
								**jumpTo = 0;
							}
							else
								i = endPos;
						}
					}
					else
						i = (elsePos < 0 ? endPos : elsePos);
				}
				continue;
			}
		}
		isExit = qspExecString(s[i], jumpTo);
		if (isExit || qspRefreshCount != oldRefreshCount || qspErrorNum) break;
		if (**jumpTo)
		{
			i = qspSearchLabel(s, startLine, endLine, *jumpTo);
			if (i < 0)
			{
				if (uLevel) qspSetError(QSP_ERR_LABELNOTFOUND);
				break;
			}
			**jumpTo = 0;
			continue;
		}
		++i;
	}
	if (uLevel) free(jumpToFake);
	return isExit;
}

QSP_BOOL qspExecStringAsCode(QSP_CHAR *s, QSP_CHAR **jumpTo)
{
	QSP_BOOL isExit;
	QSP_CHAR **strs;
	long count = qspPreprocessData(s, &strs);
	isExit = qspExecCode(strs, 0, count, 0, jumpTo, QSP_FALSE);
	qspFreeStrs(strs, count, QSP_FALSE);
	return isExit;
}

static QSP_BOOL qspStatementIf(QSP_CHAR *s, QSP_CHAR **jumpTo)
{
	QSPVariant arg;
	QSP_BOOL isExit;
	long oldRefreshCount;
	QSP_CHAR *uStr, *ePos, *pos = qspStrPos(s, QSP_COLONDELIM, QSP_FALSE);
	if (!pos)
	{
		qspSetError(QSP_ERR_COLONNOTFOUND);
		return QSP_FALSE;
	}
	oldRefreshCount = qspRefreshCount;
	*pos = 0;
	qspGetStatArgs(s, qspStatIf, &arg);
	*pos = QSP_COLONDELIM[0];
	if (qspRefreshCount != oldRefreshCount || qspErrorNum) return QSP_FALSE;
	qspUpperStr(uStr = qspGetNewText(pos, -1));
	ePos = qspStrPos(uStr, qspStats[qspStatElse].Names[0], QSP_TRUE);
	free(uStr);
	if (QSP_NUM(arg))
	{
		if (ePos)
		{
			ePos = ePos - uStr + pos;
			*ePos = 0;
			isExit = qspExecString(pos + 1, jumpTo);
			*ePos = qspStats[qspStatElse].Names[0][0];
			return isExit;
		}
		else
			return qspExecString(pos + 1, jumpTo);
	}
	else if (ePos)
		return qspExecString(ePos - uStr + pos + qspStats[qspStatElse].NamesLens[0], jumpTo);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementAddText(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	switch (extArg)
	{
	case 0:
		if (*QSP_STR(args[0]))
		{
			qspCurVarsLen = qspAddText(&qspCurVars, QSP_STR(args[0]), qspCurVarsLen, -1, QSP_FALSE);
			qspIsVarsDescChanged = QSP_TRUE;
		}
		break;
	case 1:
		if (*QSP_STR(args[0]))
		{
			qspCurDescLen = qspAddText(&qspCurDesc, QSP_STR(args[0]), qspCurDescLen, -1, QSP_FALSE);
			qspIsMainDescChanged = QSP_TRUE;
		}
		break;
	case 2:
		if (count) qspCurVarsLen = qspAddText(&qspCurVars, QSP_STR(args[0]), qspCurVarsLen, -1, QSP_FALSE);
		qspCurVarsLen = qspAddText(&qspCurVars, QSP_STRSDELIM, qspCurVarsLen, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
		qspIsVarsDescChanged = QSP_TRUE;
		break;
	case 3:
		if (count) qspCurDescLen = qspAddText(&qspCurDesc, QSP_STR(args[0]), qspCurDescLen, -1, QSP_FALSE);
		qspCurDescLen = qspAddText(&qspCurDesc, QSP_STRSDELIM, qspCurDescLen, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
		qspIsMainDescChanged = QSP_TRUE;
		break;
	case 4:
		qspCurVarsLen = qspAddText(&qspCurVars, QSP_STRSDELIM, qspCurVarsLen, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
		if (count) qspCurVarsLen = qspAddText(&qspCurVars, QSP_STR(args[0]), qspCurVarsLen, -1, QSP_FALSE);
		qspIsVarsDescChanged = QSP_TRUE;
		break;
	case 5:
		qspCurDescLen = qspAddText(&qspCurDesc, QSP_STRSDELIM, qspCurDescLen, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
		if (count) qspCurDescLen = qspAddText(&qspCurDesc, QSP_STR(args[0]), qspCurDescLen, -1, QSP_FALSE);
		qspIsMainDescChanged = QSP_TRUE;
		break;
	}
	return QSP_FALSE;
}

static QSP_BOOL qspStatementClear(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	switch (extArg)
	{
	case 0:
		if (qspClearText(&qspCurVars, &qspCurVarsLen))
			qspIsVarsDescChanged = QSP_TRUE;
		break;
	case 1:
		if (qspClearText(&qspCurDesc, &qspCurDescLen))
			qspIsMainDescChanged = QSP_TRUE;
		break;
	case 2:
		qspClearText(&qspCurInput, &qspCurInputLen);
		qspCallSetInputStrText(0);
		break;
	case 3:
		qspClearActions(QSP_FALSE);
		break;
	case 4:
		if (qspClearText(&qspCurVars, &qspCurVarsLen))
			qspIsVarsDescChanged = QSP_TRUE;
		if (qspClearText(&qspCurDesc, &qspCurDescLen))
			qspIsMainDescChanged = QSP_TRUE;
		qspClearText(&qspCurInput, &qspCurInputLen);
		qspClearActions(QSP_FALSE);
		qspCallSetInputStrText(0);
		break;
	case 5:
		qspClearVars(QSP_FALSE);
		qspInitSpecialVars();
		qspClearObjectsWithNotify();
		break;
	case 6:
		qspClearIncludes(QSP_FALSE);
		if (qspCurLoc >= qspLocsCount) qspCurLoc = -1;
		break;
	}
	return QSP_FALSE;
}

static QSP_BOOL qspStatementExit(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	return QSP_TRUE;
}

static QSP_BOOL qspStatementGoSub(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	long oldRefreshCount;
	QSPVar local, *var;
	if (!(var = qspVarReference(QSP_FMT("ARGS"), QSP_TRUE))) return QSP_FALSE;
	qspMoveVar(&local, var);
	qspSetArgs(var, args + 1, count - 1);
	oldRefreshCount = qspRefreshCount;
	qspExecLocByName(QSP_STR(args[0]), QSP_FALSE);
	if (qspRefreshCount != oldRefreshCount || qspErrorNum)
	{
		qspEmptyVar(&local);
		return QSP_FALSE;
	}
	if (!(var = qspVarReference(QSP_FMT("ARGS"), QSP_TRUE)))
	{
		qspEmptyVar(&local);
		return QSP_FALSE;
	}
	qspEmptyVar(var);
	qspMoveVar(var, &local);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementGoTo(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	QSPVar *var;
	long locInd = qspLocIndex(QSP_STR(args[0]));
	if (locInd < 0)
	{
		qspSetError(QSP_ERR_LOCNOTFOUND);
		return QSP_FALSE;
	}
	if (!(var = qspVarReference(QSP_FMT("ARGS"), QSP_TRUE))) return QSP_FALSE;
	qspEmptyVar(var);
	qspSetArgs(var, args + 1, count - 1);
	qspCurLoc = locInd;
	qspRefreshCurLoc(extArg);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementJump(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	free(*jumpTo);
	qspUpperStr(*jumpTo = qspDelSpc(QSP_STR(args[0])));
	return QSP_FALSE;
}

static QSP_BOOL qspStatementWait(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	long num = QSP_NUM(args[0]);
	qspCallRefreshInt(QSP_TRUE);
	if (num < 0) num = 0;
	qspCallSleep(num);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementSetTimer(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	long num = QSP_NUM(args[0]);
	if (num < 0) num = 0;
	qspCallSetTimer(num);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementShowWin(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	QSP_BOOL val = QSP_NUM(args[0]) != 0;
	switch (extArg)
	{
	case 0:
		qspCallShowWindow(QSP_WIN_ACTS, qspCurIsShowActs = val);
		break;
	case 1:
		qspCallShowWindow(QSP_WIN_OBJS, qspCurIsShowObjs = val);
		break;
	case 2:
		qspCallShowWindow(QSP_WIN_VARS, qspCurIsShowVars = val);
		break;
	case 3:
		qspCallShowWindow(QSP_WIN_INPUT, qspCurIsShowInput = val);
		break;
	}
	return QSP_FALSE;
}

static QSP_BOOL qspStatementRefInt(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	qspCallRefreshInt(QSP_TRUE);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementView(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	QSP_CHAR *file;
	if (count == 1 && qspIsAnyString(QSP_STR(args[0])))
	{
		file = qspGetNewText(qspQstPath, qspQstPathLen);
		file = qspGetAddText(file, QSP_STR(args[0]), qspQstPathLen, -1);
		qspCallShowPicture(file);
		free(file);
	}
	else
		qspCallShowPicture(0);
	return QSP_FALSE;
}

static QSP_BOOL qspStatementMsg(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	qspCallShowMessage(QSP_STR(args[0]));
	return QSP_FALSE;
}

static QSP_BOOL qspStatementExec(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	QSP_CHAR *cmd;
	if (qspIsAnyString(QSP_STR(args[0])))
	{
		cmd = qspGetNewText(qspQstPath, qspQstPathLen);
		cmd = qspGetAddText(cmd, QSP_STR(args[0]), qspQstPathLen, -1);
		qspCallSystem(cmd);
		free(cmd);
	}
	return QSP_FALSE;
}

static QSP_BOOL qspStatementDynamic(QSPVariant *args, long count, QSP_CHAR **jumpTo, char extArg)
{
	return qspExecStringAsCode(QSP_STR(args[0]), jumpTo);
}

#include "main.h"
#ifdef FILELOG
ostream *GLOBAL::	m_lout = (new std::ofstream(SDCARDDIR "/memorum.log"));
#else
ostream *GLOBAL::	m_lout = &std::cerr;
#endif
CONFIG GLOBAL::		m_Config;
CARDLIST GLOBAL::	m_CardList;
CALENDAR GLOBAL::	m_Calendar;

static imenu		menu[] =
{
	{ ITEM_HEADER, 0, "@Menu", NULL },
	{ ITEM_ACTIVE, 2, "@Configuration", NULL },
	{ ITEM_SEPARATOR, 0, NULL, NULL },
	{ ITEM_ACTIVE, 3, "@Smpl_mode", NULL },
	{ ITEM_ACTIVE, 4, "@Memo_mode", NULL },
	{ ITEM_ACTIVE, 5, "@Test_mode", NULL },
	{ ITEM_INACTIVE, 6, "@Auto_mode", NULL },
	{ ITEM_SEPARATOR, 0, NULL, NULL },
	{ ITEM_ACTIVE, 7, "@Change_sides", NULL },
	{ ITEM_ACTIVE, 8, "@Change_sides2", NULL },
	{ ITEM_ACTIVE, 9, "@Rand", NULL },
	{ ITEM_ACTIVE, 10, "@Reset", NULL },
	{ ITEM_SEPARATOR, 0, NULL, NULL },
	{ ITEM_ACTIVE, 11, "@Statistic", NULL },
	{ ITEM_ACTIVE, 12, "@Calendar", NULL },
	{ ITEM_ACTIVE, 13, "@Calendar_add", NULL },
	{ ITEM_SEPARATOR, 0, NULL, NULL },
	{ ITEM_ACTIVE, 1, "@Exit", NULL },
	{ 0, 0, NULL, NULL }
};
int					calendar_handler(int type, int par1, int par2);
int					caladd_handler(int type, int par1, int par2);

/* */
void SetSState(int state)
{
	menu[9].type = state;
}

/* */
void menu_handler(int index)
{
	lout << "menu_handler:  " << index << endl;
	Config.MainHandlerDefenition[index](0, 0);
}

/* */
int main_handler(int type, int par1, int par2)
{
	lout << "main_handler:  " << type << " " << par1 << " " << par2 << " " << endl;

	static bool t = false;
	switch (type)
	{
	case EVT_POINTERUP:
		if (CardList.IsSell() && CardList.SetSell(par1, par2)) return 1;
		break;

	case EVT_INIT:
		srand(time(0));
		Config.Init();
		CardList.Init();
		t = Calendar.Init();
		lout << GetDeviceModel() << endl << QueryDeviceButtons() << endl;
		return 1;

	case EVT_SHOW:
		CardList.Repaint();
		if (t) Message(ICON_INFORMATION, "Memorum", T(@Need_to_repeat), 5000);
		t = false;
		return 1;

	case EVT_KEYRELEASE:
		switch (par1)
		{
		case KEY_RIGHT:
			if (par2 == 0 && CardList.SellNext()) return 1;

		case KEY_LEFT:
			if (par2 == 0 && CardList.SellPrev()) return 1;

		case KEY_UP:
		case KEY_DOWN:
			if (par2 == 0 && CardList.SellNext2()) return 1;

		case KEY_OK:
			if (par2 == 0 && CardList.SellOK()) return 1;
		}
	}

	for
	(
		std::vector < InputConfig * >::iterator iter = Config.MainHandlerDescription.begin();
		iter != Config.MainHandlerDescription.end();
		iter++
	) if ((*iter)->RunIf(type, par1, par2)) return 1;
	return 0;
}

/* */
void KA_mem_0(int par1, int par2)	//Menu
{
	OpenMenu(menu, 0, ScreenWidth(), ScreenHeight(), menu_handler);
}

/* */
void KA_mem_1(int par1, int par2)	//Exit
{
	CloseApp();
}

/* */
void KA_mem_2(int par1, int par2)	//Configuration
{
	Config.Show();
}

/* */
void KA_mem_3(int par1, int par2)	//Smpl_mode
{
	CardList.SetSmplImpl();
	CardList.Repaint();
}

/* */
void KA_mem_4(int par1, int par2)	//Memo_mode
{
	CardList.SetMemoImpl();
	CardList.Repaint();
}

/* */
void KA_mem_5(int par1, int par2)	//Test_mode
{
	CardList.SetTestImpl();
	CardList.Repaint();
}

/* */
void KA_mem_6(int par1, int par2)	//Auto_mode
{
}

/* */
void KA_mem_7(int par1, int par2)	//Change_sides
{
	CardList.ChangeSides();
	CardList.Repaint();
}

/* */
void KA_mem_8(int par1, int par2)	//Change_sides2
{
	CardList.ChangeSides2();
	CardList.Repaint();
}

/* */
void KA_mem_9(int par1, int par2)	//Rand
{
	CardList.Randomise();
	CardList.Repaint();
}

/* */
void KA_mem_10(int par1, int par2)	//Reset
{
	CardList.Reset();
	CardList.Repaint();
}

/* */
void KA_mem_11(int par1, int par2)	//Statistic
{
	char	buf[256] = "";
	if (CardList.GetPercent()) sprintf(buf, "%s: %d\n", T(@Cur_percent), CardList.GetPercent());

	int Know = 0;
	for
	(
		std::vector < BaseCard * >::iterator iter = CardList.m_vData.begin();
		iter != CardList.m_vData.end();
		iter++
	) Know += (*iter)->m_bKnowing;

	sprintf(buf, "%s%s: %d \t%d%%\n", buf, T(@Lerned_cards), Know, Know * 100 / CardList.m_vData.size());
	sprintf
	(
		buf,
		"%s%s: %d \t%d%%\n",
		buf,
		T(@Unlerned_cards),
		CardList.m_vData.size() - Know,
		(CardList.m_vData.size() - Know) * 100 / CardList.m_vData.size()
	);
	Message(ICON_INFORMATION, "Memorum", buf, 5000);
}

/* */
void KA_mem_12(int par1, int par2)	//Calendar
{
	SetEventHandler(calendar_handler);
}

/* */
void KA_mem_13(int par1, int par2)	//Calendar_add
{
	SetEventHandler(caladd_handler);
}

/* */
void KA_mem_14(int par1, int par2)	//MoveNext
{
	CardList.MoveNext();
}

/* */
void KA_mem_15(int par1, int par2)	//MovePrev
{
	CardList.MovePrev();
}

/* */
void KA_mem_16(int par1, int par2)	//Sound
{
	CardList.Sound();
}

/* */
void KA_mem_17(int par1, int par2)	//Expand
{
	CardList.Expand();
}

KeyHandlerFunc	MainHandlerDefenition[] =
{
	&KA_mem_0,						//"@KA_mem_0"	Menu
	&KA_mem_1,						//"@KA_mem_1"	Exit
	&KA_mem_2,						//"@KA_mem_2"	Configuration
	&KA_mem_3,						//"@KA_mem_3"	Smpl_mode
	&KA_mem_4,						//"@KA_mem_4"	Memo_mode
	&KA_mem_5,						//"@KA_mem_5"	Test_mode
	&KA_mem_6,						//"@KA_mem_6"	Test_mode
	&KA_mem_7,						//"@KA_mem_7"	Change_sides
	&KA_mem_8,						//"@KA_mem_8"	Change_sides2
	&KA_mem_9,						//"@KA_mem_9"	Rand
	&KA_mem_10,						//"@KA_mem_10"	Reset
	&KA_mem_11,						//"@KA_mem_11"	Statistic
	&KA_mem_12,						//"@KA_mem_12"	Calendar
	&KA_mem_13,						//"@KA_mem_13"	Calendar_add
	&KA_mem_14,						//"@KA_mem_14"	MoveNext
	&KA_mem_15,						//"@KA_mem_15"	MovePrev
	&KA_mem_16,						//"@KA_mem_16"	Sound
	&KA_mem_17,						//"@KA_mem_17"	Expand
};

/* */

/* */
int main(int argc, char **argv)
{
	Config.MainHandlerDefenition = MainHandlerDefenition;
	if (argc > 1)
	{
		CardList.SetFile(argv[1]);
		InkViewMain(main_handler);
	}
	else
		InkViewMain(calendar_handler);
	return 0;
}

#define CalAddRetok(time) \
	FillArea(RCTVAL(Footer), WHITE); \
	DrawLine(0, RCT(Footer).y, RCT(Footer).w, RCT(Footer).y, BLACK); \
	parts[0] = DateStr(time); \
	DrawTextRect(RCTVAL(Footer), T(@Select_date), ALIGN_LEFT | VALIGN_MIDDLE); \
	DrawTextRect(RCT(Footer).x, RCT(Footer).y, RCT(Footer).w - 10, RCT(Footer).h, parts[0], RCT(Footer).flags); \
	length = StringWidth(parts[0]) + 10; \
	DrawSymbol(StringWidth(T(@Select_date)) + 10, RCT(Footer).y + 10, SYMBOL_OK); \
	for (int i = 1; i < 4; i++) \
	{ \
		strtok(parts[i - 1], " "); \
		parts[i] = parts[i - 1] + strlen(parts[i - 1]) + 1; \
	} \
	width = parts_wdth[0] = StringWidth(parts[0]) + StringWidth(" "); \
	for (int i = 1; i < 4; i++) \
	{ \
		parts_wdth[i] = StringWidth(parts[i]) + StringWidth(" "); \
		width += parts_wdth[i]; \
		DrawSymbol(RCT(Footer).w - length + width - 10 - parts_wdth[i] / 2, RCT(Footer).y - 8, ARROW_UP); \
		DrawSymbol(RCT(Footer).w - length + width - 10 - parts_wdth[i] / 2, RCT(Footer).y + 29, ARROW_DOWN); \
	}
#define CalAddRedraw(level) \
	width = 0; \
	for (int i = 0; i < level; i++) width += parts_wdth[i]; \
	DrawSelection \
	( \
		RCT(Footer).w - length + width - 5, \
		RCT(SelButt1).y + 7, \
		parts_wdth[level], \
		RCT(SelButt1).h - 14, \
		BLACK \
	); \
	PartialUpdateBW(RCTVAL(Footer));
#define CalAddClrdraw(level) \
	DrawSelection \
	( \
		RCT(Footer).w - length + width - 5, \
		RCT(SelButt1).y + 7, \
		parts_wdth[level], \
		RCT(SelButt1).h - 14, \
		WHITE \
	);
#define Beetwen(x, x1, x2)	(x >= (x1) && x <= (x2))
#define Inside(x, w, y, h)	(Beetwen(par1, x, x + w) && Beetwen(par2, y, y + h))
#define SI(x, i, yy) \
	Inside \
	( \
		(RCT(Footer).w - length - 5 + (x)), \
		(parts_wdth[i]), \
		(RCT(Footer).y + (yy)), \
		(RCT(Footer).h / 2) \
	)

/* */
int caladd_handler(int type, int par1, int par2)
{
	lout << "caladd_handler:  " << type << " " << par1 << " " << par2 << endl;

	static int		level;
	static int		length;
	static int		width;
	static time_t	ts;
	static char		*parts[4];
	static int		parts_wdth[4];
	tm				*tmstr;

	switch (type)
	{
	case EVT_POINTERUP:
		if (Inside(0, StringWidth(T(@Select_date)) + 30, RCT(Footer).y, RCT(Footer).h))
		{
			Calendar.AddCurrCL(ts);
			Calendar.Save();
			SetEventHandler(main_handler);
			return 0;
		}
		else if (SI(parts_wdth[0], 1, 0))
			ts += 24 * 3600;
		else if (SI(parts_wdth[0], 1, RCT(Footer).h / 2))
			ts -= 24 * 3600;
		else if (SI(parts_wdth[0] + parts_wdth[1], 2, 0))
		{
			tmstr = gmtime(&ts);
			tmstr->tm_mon++;
			ts = mktime(tmstr);
		}
		else if (SI(parts_wdth[0] + parts_wdth[1], 2, RCT(Footer).h / 2))
		{
			tmstr = gmtime(&ts);
			tmstr->tm_mon--;
			ts = mktime(tmstr);
		}
		else if (SI(parts_wdth[0] + parts_wdth[1] + parts_wdth[2], 3, 0))
		{
			tmstr = gmtime(&ts);
			tmstr->tm_year++;
			ts = mktime(tmstr);
		}
		else if (SI(parts_wdth[0] + parts_wdth[1] + parts_wdth[2], 3, RCT(Footer).h / 2))
		{
			tmstr = gmtime(&ts);
			tmstr->tm_year--;
			ts = mktime(tmstr);
		}

		CalAddRetok(ts);
		CalAddRedraw(level);
		break;

	case EVT_POINTERHOLD:
		Calendar.AddCurrCL(ts);
		Calendar.Save();
		SetEventHandler(main_handler);
		break;

	case EVT_ORIENTATION:
		SetOrientation(par2);
		break;

	case EVT_SHOW:
		SetFont(Config.Footerfont, BLACK);
		level = 1;
		ts = time(0);
		CalAddRetok(ts);
		CalAddRedraw(level);
		break;

	case EVT_KEYRELEASE:
		switch (par1)
		{
		case KEY_RIGHT:
		case KEY_NEXT:
		case KEY_NEXT2:
			CalAddClrdraw(level);
			level++;
			if (level > 3) level = 1;
			CalAddRedraw(level);
			break;

		case KEY_LEFT:
		case KEY_PREV:
		case KEY_PREV2:
			CalAddClrdraw(level);
			level--;
			if (level < 1) level = 3;
			CalAddRedraw(level);
			break;

		case KEY_UP:
			switch (level)
			{
			case 1:
				ts += 24 * 3600;
				break;

			case 2:
				tmstr = gmtime(&ts);
				tmstr->tm_mon++;
				ts = mktime(tmstr);
				break;

			case 3:
				tmstr = gmtime(&ts);
				tmstr->tm_year++;
				ts = mktime(tmstr);
				break;
			}

			CalAddRetok(ts);
			CalAddRedraw(level);
			break;

		case KEY_DOWN:
			switch (level)
			{
			case 1:
				ts -= 24 * 3600;
				break;

			case 2:
				tmstr = gmtime(&ts);
				tmstr->tm_mon--;
				ts = mktime(tmstr);
				break;

			case 3:
				tmstr = gmtime(&ts);
				tmstr->tm_year--;
				ts = mktime(tmstr);
				break;
			}

			CalAddRetok(ts);
			CalAddRedraw(level);
			break;

		case KEY_OK:
			Calendar.AddCurrCL(ts);
			Calendar.Save();
			SetEventHandler(main_handler);
			break;

		case KEY_BACK:
			SetEventHandler(main_handler);
			break;
		}
		break;
	}

	return 0;
}

/* */
int calendar_handler(int type, int par1, int par2)
{
	lout << "calendar_handler:  " << type << " " << par1 << " " << par2 << endl;

	static bool t = false;
	static int	PointerX, PointerY;
	switch (type)
	{
	case EVT_POINTERDOWN:
		PointerX = par1;
		PointerY = par2;
		break;

	case EVT_POINTERUP:
		Calendar.MoveToXY(par1, par2);
		break;

	case EVT_ORIENTATION:
		SetOrientation(par1);
		break;

	case EVT_INIT:
		srand(time(0));
		Config.Init();
		t = Calendar.Init(true);

		char	**act0, **act1;
		act0 = new char *[50];
		act1 = new char *[50];
		GetKeyMapping(act0, act1);

		int i;
		i = 0;
		while (act0[i]) i++;
		break;

	case EVT_SHOW:
		Calendar.Repaint();
		if (t) Message(ICON_INFORMATION, "Memorum", T(@Need_to_repeat), 5000);
		t = false;
		break;

	case EVT_KEYRELEASE:
		switch (par1)
		{
		case KEY_BACK:
			if (par2 > 0 || Calendar.Start())
				CloseApp();
			else
				SetEventHandler(main_handler);
			break;

		case KEY_PREV:
		case KEY_PREV2:
			Calendar.MovePrevM();
			break;

		case KEY_NEXT:
		case KEY_NEXT2:
			Calendar.MoveNextM();
			break;

		case KEY_RIGHT:
			if (par2 > 0)
				Calendar.MoveNextM();
			else
				Calendar.MoveRight();
			break;

		case KEY_LEFT:
			if (par2 > 0)
				Calendar.MovePrevM();
			else
				Calendar.MoveLeft();
			break;

		case KEY_UP:
			Calendar.MoveUp();
			break;

		case KEY_DOWN:
			if (par2 > 0)
				CloseApp();
			else
				Calendar.MoveDown();
			break;

		case KEY_OK:
			Calendar.Go();
			break;
		}
		break;
	}

	return 0;
}

unsigned int	csel;

/* */
bool pred(Unit &u)
{
	return &u == (*Calendar.CurUnits)[csel];
}

/* */
void delete_unit(int button)
{
	if (button == 1)
	{
		Calendar.m_vData.erase(std::find_if(Calendar.m_vData.begin(), Calendar.m_vData.end(), pred));
		Calendar.CurUnits->erase(Calendar.CurUnits->begin() + csel);
		Calendar.Save();
		SetEventHandler(day_handler);
	}
}

/* */
int day_handler(int type, int par1, int par2)
{
	lout << "day_handler:  " << type << " " << par1 << " " << par2 << endl;

	static int	PointerX, PointerY;
	static int	hei = Config.Delftfont->height + 1;
	switch (type)
	{
	case EVT_POINTERDOWN:
		PointerX = par1;
		PointerY = par2;
		break;

	case EVT_POINTERUP:
		par2 -= RCT(Header).h;
		if (par2 < 0 || par2 / hei >= (int) Calendar.CurUnits->size()) break;
		DrawSelection(0, RCT(Header).h + hei * csel, RCT(FullScreen).w, hei, WHITE);
		csel = par2 / hei;
		DrawSelection(0, RCT(Header).h + hei * csel, RCT(FullScreen).w, hei, BLACK);
		PartialUpdateBW(RCTVAL(FullScreen));
		CardList.SetFile((*Calendar.CurUnits)[csel]->m_sFullFileName);
		CardList.Init();
		SetEventHandler(main_handler);
		break;

	case EVT_ORIENTATION:
		SetOrientation(par1);
		break;

	case EVT_SHOW:
		if (Calendar.CurUnits->empty())
		{
			SetEventHandler(calendar_handler);
			return 0;
		}

		ClearScreen();
		Calendar.UpdateHeader();

		char	buf[5];
		for (unsigned int i = 0; i < Calendar.CurUnits->size(); i++)
		{
			for (unsigned int j = 0; j < (*Calendar.CurUnits)[i]->m_vDays.size(); j++)
			{
				if
				(
					(*Calendar.CurUnits)[i]->m_vDays[j] -
						Calendar.m_iMonthStartDay +
						Calendar.m_iMonthStartWeekDay == Calendar.m_Cur
				)
				{
					(*Calendar.CurUnits)[i]->m_iTempState = j;
					break;
				}
			}

			SetFont(Config.Delftfont, BLACK);
			sprintf(buf, "%d ", (*Calendar.CurUnits)[i]->m_iCardsCount);
			DrawTextRect
			(
				5,
				RCT(Header).h + hei * i,
				RCT(FullScreen).w,
				hei,
				sz((*Calendar.CurUnits)[i]->m_sCardListName),
				ALIGN_LEFT | VALIGN_MIDDLE
			);
			if ((*Calendar.CurUnits)[i]->m_iState <= (*Calendar.CurUnits)[i]->m_iTempState)
				SetFont(Config.Delftfontb, BLACK);
			DrawTextRect(0, RCT(Header).h + hei * i, RCT(FullScreen).w, hei, buf, ALIGN_RIGHT | VALIGN_MIDDLE);
		}

		csel = 0;
		DrawSelection(0, RCT(Header).h + hei * csel, RCT(FullScreen).w, hei, BLACK);
		FullUpdate();
		break;

	case EVT_KEYRELEASE:
		switch (par1)
		{
		case KEY_LEFT:
		case KEY_BACK:
			if (par2 > 0)
				CloseApp();
			else
				SetEventHandler(calendar_handler);
			break;

		case KEY_UP:
			DrawSelection(0, RCT(Header).h + hei * csel, RCT(FullScreen).w, hei, WHITE);
			if (csel == 0)
				csel = Calendar.CurUnits->size() - 1;
			else
				csel--;
			DrawSelection(0, RCT(Header).h + hei * csel, RCT(FullScreen).w, hei, BLACK);
			PartialUpdateBW(RCTVAL(FullScreen));
			break;

		case KEY_DOWN:
			if (par2 > 0)
				Dialog(ICON_QUESTION, "Memorum", "@Conifurm_delete", "@Yes", "@No", delete_unit);
			else
			{
				DrawSelection(0, RCT(Header).h + hei * csel, RCT(FullScreen).w, hei, WHITE);
				if (csel == Calendar.CurUnits->size() - 1)
					csel = 0;
				else
					csel++;
				DrawSelection(0, RCT(Header).h + hei * csel, RCT(FullScreen).w, hei, BLACK);
				PartialUpdateBW(RCTVAL(FullScreen));
			}
			break;

		case KEY_OK:
			CardList.SetFile((*Calendar.CurUnits)[csel]->m_sFullFileName);
			CardList.Init();
			if ((*Calendar.CurUnits)[csel]->m_iState <= (*Calendar.CurUnits)[csel]->m_iTempState)
			{
				(*Calendar.CurUnits)[csel]->m_iState = (*Calendar.CurUnits)[csel]->m_iTempState + 1;
				Calendar.Save();
			}

			SetEventHandler(main_handler);
			break;

		case KEY_DELETE:
			Dialog(ICON_QUESTION, "Memorum", "@Conifurm_delete", "@Yes", "@No", delete_unit);
			break;
		}
	}

	return 0;
}

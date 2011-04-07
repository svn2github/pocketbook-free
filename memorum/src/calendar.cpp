#include "main.h"

const int CALENDAR::	MonthDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
char *CALENDAR::		WeekDays[7] = { "@Mon", "@Tue", "@Wed", "@Thu", "@Fri", "@Sat", "@Sun" };

/* */
std::ostream & operator <<(std::ostream & out, Unit & u)
{
	out << u.m_sCardListName << ends;
	out << u.m_sFullFileName << ends;
	out << u.m_iCardsCount << endl;
	out << u.m_iState << endl;
	out << u.m_vDays[0];
	return out;
}

/* */
std::istream & operator >>(std::istream & in, Unit & u)
{
	int temp;
	std::getline(in, u.m_sCardListName, '\0');
	std::getline(in, u.m_sFullFileName, '\0');
	in >> u.m_iCardsCount;
	in >> u.m_iState;
	if (u.m_iState > 100)
	{
		temp = u.m_iState;
		u.m_iState = 0;
	}
	else
		in >> temp;
	for (unsigned int i = 0; i < Config.Graphic.size(); i++)
	{
		u.m_vDays.push_back(temp);
		temp += Config.Graphic[i];
	}
	u.m_iTempState = -1;
	return in;
}

/* */
bool CALENDAR::Init(bool start)
{
	m_bStart = start;

	m_iCurrDay = time(0) / 3600 / 24;

	time_t	timt;
	time(&timt);

	tm	*tmstr = gmtime(&timt);
	m_iCurrMonth = tmstr->tm_mon;
	m_iMonthStartDay = m_iCurrDay - tmstr->tm_mday;
	m_iMonthStartWeekDay = (m_iMonthStartDay + 3) % 7;
	m_Cur = (m_iCurrDay - m_iMonthStartDay + m_iMonthStartWeekDay);
	std::ifstream fin(CONFIGPATH "/memorum.calendar");

	bool	Unrepeated = false;
	if (fin.is_open())
	{
		int s;
		fin >> s;
		for (int i = 0; i < s; i++)
		{
			Unit	u;
			fin >> (u);
			m_vData.push_back(u);
			for (unsigned int j = 0; j < u.m_vDays.size() /* && !Unrepeated*/ ; j++)
			{
				if (u.m_vDays[j] - Calendar.m_iMonthStartDay + Calendar.m_iMonthStartWeekDay == Calendar.m_Cur)
				{
					u.m_iTempState = j;
					break;
				}
			}

			if (u.m_iState <= u.m_iTempState) Unrepeated = true;
		}

		if (m_vData.empty() && m_bStart) Message(ICON_INFORMATION, "Memorum", T(@First_lunch), 10000);

		fin.close();
	}

	return Unrepeated;
}

/* */
void CALENDAR::AddCurrCL(time_t ts)
{
	Unit	u;
	u.m_iState = 0;
	u.m_iCardsCount = CardList.m_vData.size();
	u.m_sCardListName = CardList.m_sCardListName;
	u.m_sFullFileName = CardList.m_sFullFileName;

	int temp = ts / 3600 / 24;
	for (unsigned int i = 0; i < Config.Graphic.size(); i++)
	{
		int tc = 0;
		for (std::vector<Unit>::iterator it = m_vData.begin(); it != m_vData.end(); it++)
			for (std::vector<int>::iterator j = it->m_vDays.begin(); j != it->m_vDays.end(); j++)
				if (*j == temp) tc++;
		if (tc >= 13)
		{
			char	buf[256];
			sprintf(buf, T(@Calendar_notadded), DateStr(temp * 3600 * 24));
			Message(ICON_ERROR, "Memorum", buf, 5000);
			return;
		}

		u.m_vDays.push_back(temp);
		temp += Config.Graphic[i];
	}

	m_vData.push_back(u);
	Message(ICON_INFORMATION, "Memorum", "@Calendar_added", 2000);
}

/* */
void CALENDAR::Save()
{
	std::ofstream fout(CONFIGPATH "/memorum.calendar");
	fout << m_vData.size(); //<<endl;
	for (std::vector<Unit>::iterator i = m_vData.begin(); i != m_vData.end(); i++) fout << (*i);	//<<endl;	
	fout.close();
}

/* */
void CALENDAR::RecalcWid()
{
	int ScrW = RCT(FullScreen).w;
	int ScrH = RCT(FullScreen).h - RCT(Header).h;
	WidX = ScrW / 7;
	WidY = ScrH / 7;
}

/* */
void CALENDAR::Repaint()
{
	char	buf[10];
	RecalcWid();
	for (int i = 0; i < 42; i++) m_Cells[i].m_vUnits.clear();
	for (std::vector<Unit>::iterator i = m_vData.begin(); i != m_vData.end(); i++)
	{
		for (std::vector<int>::iterator j = i->m_vDays.begin(); j != i->m_vDays.end(); j++)
		{
			if
			(
				*j >= m_iMonthStartDay - m_iMonthStartWeekDay
			&&	(*j) < m_iMonthStartDay - m_iMonthStartWeekDay + 42
			) m_Cells[(*j) - m_iMonthStartDay + m_iMonthStartWeekDay].m_vUnits.push_back(&*i);
		}
	}

	ClearScreen();
	SetFont(Config.Delftfontb, BLACK);
	for (int i = 0; i < 7; i++)
	{
		DrawTextRect
		(
			0,
			RCT(Header).h + i * (WidY),
			WidX,
			WidY,
			GetLangText(WeekDays[i]),
			ALIGN_CENTER | VALIGN_MIDDLE
		);
	}

	for (int i = 0; i < 42; i++)
	{
		int		color = BLACK;
		ifont	*font = Config.Delftfont;
		int		t = i - m_iMonthStartWeekDay;
		if (t < 1)
		{
			color = LGRAY;
			m_Cells[i].m_iDay = MonthDays[m_iCurrMonth ? m_iCurrMonth - 1 : 11] + t;
		}
		else if (t > MonthDays[m_iCurrMonth])
		{
			color = LGRAY;
			m_Cells[i].m_iDay = t - MonthDays[m_iCurrMonth];
		}
		else
			m_Cells[i].m_iDay = t;
		if (!m_Cells[i].m_vUnits.empty()) font = Config.Delftfontb;
		sprintf(buf, ((m_iMonthStartDay + t == m_iCurrDay) ? "[%d]" : "%d"), m_Cells[i].m_iDay);
		SetFont(font, color);
		m_Cells[i].m_irRect.x = (i / 7 + 1) * WidX;
		m_Cells[i].m_irRect.y = RCT(Header).h + i % 7 * WidY;
		m_Cells[i].m_irRect.w = WidX;
		m_Cells[i].m_irRect.h = WidY;
		m_Cells[i].m_irRect.flags = ALIGN_CENTER | VALIGN_MIDDLE;
		DrawTextRect2(&m_Cells[i].m_irRect, buf);
	}

	InvertAreaBW(CurCelRectM);
	DrawSelection(CurCelRect, BLACK);
	UpdateHeader();
	FullUpdate();
}

/* */
bool CALENDAR::Start()
{
	return m_bStart;
}

/* */
void CALENDAR::UpdateHeader()
{
	FillArea(RCTVAL(Header), WHITE);
	DrawSelection(RCTVAL(Header), DGRAY);
	SetFont(Config.Headerfont, BLACK);
	DrawTextRect2(&RCT(Header), DateStr((m_Cur - m_iMonthStartWeekDay + m_iMonthStartDay) * 3600 * 24));
}

/* */
void CALENDAR::MoveNextM(int day)
{
	m_iMonthStartDay += MonthDays[m_iCurrMonth];
	if (m_iCurrMonth == 11)
		m_iCurrMonth = 0;
	else
		m_iCurrMonth++;
	m_iMonthStartWeekDay = (m_iMonthStartDay + 3) % 7;

	m_Cur = m_iMonthStartWeekDay + day;
	Repaint();
}

/* */
void CALENDAR::MovePrevM(int day)
{
	if (m_iCurrMonth == 0)
		m_iCurrMonth = 11;
	else
		m_iCurrMonth--;
	m_iMonthStartDay -= MonthDays[m_iCurrMonth];
	m_iMonthStartWeekDay = (m_iMonthStartDay + 3) % 7;
	m_Cur = m_iMonthStartWeekDay + day;
	Repaint();
}

/* */
void CALENDAR::MoveUp()
{
	InvertAreaBW(CurCelRectM);
	DrawSelection(CurCelRect, WHITE);
	if (m_Cur == m_iMonthStartWeekDay + 1)
		MovePrevM(m_Cells[m_Cur - 1].m_iDay);
	else
	{
		m_Cur--;
		InvertAreaBW(CurCelRectM);
		DrawSelection(CurCelRect, BLACK);
		UpdateHeader();
		PartialUpdateBW(RCTVAL(FullScreen));
	}
}

/* */
void CALENDAR::MoveDown()
{
	InvertAreaBW(CurCelRectM);
	DrawSelection(CurCelRect, WHITE);
	if (m_Cur + 1 - m_iMonthStartWeekDay > MonthDays[m_iCurrMonth])
		MoveNextM(1);
	else
	{
		m_Cur++;
		InvertAreaBW(CurCelRectM);
		DrawSelection(CurCelRect, BLACK);
		UpdateHeader();
		PartialUpdateBW(RCTVAL(FullScreen));
	}
}

/* */
void CALENDAR::MoveLeft()
{
	InvertAreaBW(CurCelRectM);
	DrawSelection(CurCelRect, WHITE);
	if (m_Cur - 7 <= m_iMonthStartWeekDay)
		MovePrevM(m_Cells[m_iMonthStartWeekDay].m_iDay - (7 + m_iMonthStartWeekDay - m_Cur));
	else
	{
		m_Cur -= 7;
		InvertAreaBW(CurCelRectM);
		DrawSelection(CurCelRect, BLACK);
		UpdateHeader();
		PartialUpdateBW(RCTVAL(FullScreen));
	}
}

/* */
void CALENDAR::MoveRight()
{
	InvertAreaBW(CurCelRectM);
	DrawSelection(CurCelRect, WHITE);
	if (m_Cur + 7 > m_iMonthStartWeekDay + MonthDays[m_iCurrMonth])
		MoveNextM(m_Cur - (m_iMonthStartWeekDay + MonthDays[m_iCurrMonth]) + 7);
	else
	{
		m_Cur += 7;
		InvertAreaBW(CurCelRectM);
		DrawSelection(CurCelRect, BLACK);
		UpdateHeader();
		PartialUpdateBW(RCTVAL(FullScreen));
	}
}

/* */
void CALENDAR::MoveToXY(int x, int y)
{
	if (!(x / WidX) || y < RCT(Header).h) return;

	int temp = (x / WidX - 1) * 7 + (y - RCT(Header).h) / WidY;
	InvertAreaBW(CurCelRectM);
	DrawSelection(CurCelRect, WHITE);
	if (temp <= m_iMonthStartWeekDay || temp - m_iMonthStartWeekDay > MonthDays[m_iCurrMonth])
		MovePrevM(m_Cells[temp].m_iDay);
	else
	{
		m_Cur = temp;
		InvertAreaBW(CurCelRectM);
		DrawSelection(CurCelRect, BLACK);
		UpdateHeader();
		PartialUpdateBW(RCTVAL(FullScreen));
	}

	Go();
}

/* */
void CALENDAR::Go()
{
	if (m_Cells[m_Cur].m_vUnits.empty()) return;
	CurUnits = &m_Cells[m_Cur].m_vUnits;
	SetEventHandler(day_handler);
}

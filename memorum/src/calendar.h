struct Unit
{
	std::vector<int>	m_vDays;
	std::string			m_sFullFileName;
	std::string			m_sCardListName;
	int m_iCardsCount;
	int m_iState;
	int m_iTempState;
};
std::ostream & operator <<(std::ostream & out, Unit & u);
std::istream & operator >>(std::istream & in, Unit & u);
int day_handler(int type, int par1, int par2);

struct Cell
{
	int m_iDay;
	irect m_irRect;
	std::vector<Unit *> m_vUnits;
};
#define CurCelRect	m_Cells[m_Cur].m_irRect.x, m_Cells[m_Cur].m_irRect.y, m_Cells[m_Cur].m_irRect.w, m_Cells[m_Cur]. \
			m_irRect.h
#define CurCelRectM m_Cells[m_Cur].m_irRect.x + \
		1, m_Cells[m_Cur].m_irRect.y + 1, m_Cells[m_Cur].m_irRect.w - 2, m_Cells[m_Cur].m_irRect.h - 2
class CALENDAR
{
	friend int	day_handler(int type, int par1, int par2);
	static const int MonthDays[12];
	static char *WeekDays[7];
	bool m_bStart;
	Cell m_Cells[42];
	int m_iCurrDay;
	int m_iCurrMonth;
	int m_iMonthStartDay;
	int m_iMonthStartWeekDay;
	int m_Cur;
	int WidY;
	int WidX;

/* */
public:
	std::vector<Unit>	m_vData;
	std::vector<Unit *> *CurUnits;
	bool				Init(bool start = false);
	void				AddCurrCL(time_t ts);
	void				Save();
	void				RecalcWid();
	void				Repaint();
	void				UpdateHeader();
	void				MoveNextM(int day = 1);
	void				MovePrevM(int day = 1);
	void				MoveUp();
	void				MoveDown();
	void				MoveLeft();
	void				MoveRight();
	void				MoveToXY(int x, int y);
	void				Go();
	bool				Start();
};

#pragma once

struct Unit
{
	vector<int> m_vDays;
	string m_sFullFileName;
	string m_sCardListName;
	int m_iCardsCount;
};
ostream& operator<<(ostream& out, Unit& u);
istream& operator>>(istream& in, Unit& u);
int day_handler(int type, int par1, int par2);

class CALENDAR
{
	static const int MonthDays[12];
	static char* WeekDays[7]; 
	bool m_bStart;
	bool m_bBold[42];
	int m_iCurrDay;
	int m_iCurrMonth;
	int m_iMonthStartDay;
	int m_iMonthStartWeekDay;
	int m_CurY;
	int m_CurX;
	int WidY;
	int WidX;
public:
	std::vector<Unit> m_vData;
	std::vector<Unit*> CurUnits;
	void Init(bool start = false);
	void AddCurrCL();
	void Save();
	void RecalcWid();
	void Repaint();
	void UpdateHeader();
	void MoveNextM(int mode = 0);
	void MovePrevM(int mode = 0);
	void MoveUp();
	void MoveDown();
	void MoveLeft();
	void MoveRight();
	void MoveToXY(int x, int y);
	void Go();
	bool Start();
};

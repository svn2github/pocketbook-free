#include "main.h"

const int CALENDAR::MonthDays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
char* CALENDAR::WeekDays[7] = {"@Mon", "@Tue", "@Wed", "@Thu", "@Fri", "@Sat", "@Sun"}; 

ostream& operator<<(ostream& out, Unit& u)
{
	out<<u.m_sCardListName<<ends;
	out<<u.m_sFullFileName<<ends;
	out<<u.m_iCardsCount<<endl;
	out<<u.m_vDays[0];
	return out;
}
istream& operator>>(istream& in, Unit& u)
{
	int temp;
	std::getline(in,u.m_sCardListName,'\0');
	lout<<"1  "<<u.m_sCardListName<<endl;
	std::getline(in,u.m_sFullFileName,'\0');
	lout<<"2  "<<u.m_sFullFileName<<endl;
	in>>u.m_iCardsCount;
	lout<<"3  "<<u.m_iCardsCount<<endl;
	in>>temp;
	lout<<"4  "<<temp<<endl;
	for(unsigned int i=0; i<Config.Graphic.size(); i++)
	{
		u.m_vDays.push_back(temp);
		temp+=Config.Graphic[i];
	}
	return in;
}

void CALENDAR::Init(bool start)
{
	m_bStart=start;
	std::ifstream fin(CONFIGPATH"/memorum.calendar");
	if(fin.is_open())
	{
		int s;
		fin>>s;
		for(int i=0; i<s; i++)
		{
			Unit u;
			fin>>(u);
			m_vData.push_back(u);
		}
		fin.close();
	}

	m_iCurrDay = time(0)/3600/24;
	time_t timt;
	time(&timt);
	tm* tmstr = gmtime(&timt);
	m_iCurrMonth=tmstr->tm_mon;
	m_iMonthStartDay = m_iCurrDay-tmstr->tm_mday;
	m_iMonthStartWeekDay = (m_iMonthStartDay+3)%7;
	m_CurX=(m_iCurrDay-m_iMonthStartDay+m_iMonthStartWeekDay)/7+1;
	m_CurY=(m_iCurrDay-m_iMonthStartDay+m_iMonthStartWeekDay)%7;
}
void CALENDAR::AddCurrCL()
{
	
	Unit u;
	u.m_iCardsCount = CardList.m_vData.size();
	u.m_sCardListName = CardList.m_sCardListName;
	u.m_sFullFileName = CardList.m_sFullFileName;
	int temp = Calendar.m_iCurrDay;
	for(unsigned int i=0; i<Config.Graphic.size(); i++)
	{
		int tc=0;
		for(vector<Unit>::iterator it=m_vData.begin(); it!=m_vData.end(); it++)
			for(vector<int>::iterator j=it->m_vDays.begin(); j!=it->m_vDays.end(); j++)
				if(*j==temp) tc++;
		if(tc>=13)
		{
			char buf[256];
			sprintf(buf, T(@Calendar_notadded), DateStr(temp*3600*24));
			Message(ICON_ERROR,"Memorum",buf,5000);
			return;
		}
		u.m_vDays.push_back(temp);
		temp+=Config.Graphic[i];
	}
	m_vData.push_back(u);
	Message(ICON_INFORMATION,"Memorum","@Calendar_added",2000);
}
void CALENDAR::Save()
{
	ofstream fout(CONFIGPATH"/memorum.calendar");
	fout<<m_vData.size();//<<endl;
	for(vector<Unit>::iterator i=m_vData.begin(); i!=m_vData.end(); i++)
		fout<<(*i);//<<endl;	
	fout.close();
}
void CALENDAR::RecalcWid()
{
	int ScrW = RCT(FullScreen).w;
	int ScrH = RCT(FullScreen).h-RCT(Header).h;
	WidX = ScrW/7;
	WidY = ScrH/7;
}
void CALENDAR::Repaint()
{
	char buf[10];
	RecalcWid();
	for(int i=0; i<42;i++)
		m_bBold[i]=false;
	for(vector<Unit>::iterator i=m_vData.begin(); i!=m_vData.end(); i++)
		for(vector<int>::iterator j=i->m_vDays.begin(); j!=i->m_vDays.end(); j++)
			if(*j>m_iMonthStartDay-m_iMonthStartWeekDay && (*j)<m_iMonthStartDay-m_iMonthStartWeekDay+42)
				m_bBold[(*j)-m_iMonthStartDay+m_iMonthStartWeekDay]=true;

	ClearScreen();
	SetFont(Config.Delftfontb, BLACK);
	for(int i=0; i<7; i++)
		DrawTextRect(0, RCT(Header).h+i*(WidY), WidX, WidY, GetLangText(WeekDays[i]), ALIGN_CENTER|VALIGN_MIDDLE);
	for(int x=0; x<6; x++)
		for(int y=0; y<7; y++)
		{
			int color = BLACK;
			ifont* font = Config.Delftfont;
			int d, t = x*7+y-m_iMonthStartWeekDay;
			if(t<1)	
			{
				color = LGRAY;
				d = MonthDays[m_iCurrMonth?m_iCurrMonth-1:11]+t;
			}
			else if(t>MonthDays[m_iCurrMonth])	
			{
				color = LGRAY;
				d = t-MonthDays[m_iCurrMonth];
			}
			else d=t;
			if(m_bBold[x*7+y]) font = Config.Delftfontb;
			sprintf(buf, ((m_iMonthStartDay+t==m_iCurrDay)?"[%d]":"%d"), d);
			SetFont(font, color);
			DrawTextRect((x+1)*(WidX), RCT(Header).h+y*(WidY), WidX, WidY, buf, ALIGN_CENTER|VALIGN_MIDDLE);
		}
	InvertAreaBW(m_CurX*(WidX)+1, RCT(Header).h+m_CurY*(WidY)+1, WidX-2, WidY-2);
	DrawSelection(m_CurX*(WidX), RCT(Header).h+m_CurY*(WidY), WidX, WidY, BLACK);
	UpdateHeader();
	FullUpdate();
}
bool CALENDAR::Start()
{
	return m_bStart;
}
void CALENDAR::UpdateHeader()
{	
	FillArea(RCTVAL(Header),WHITE);
	DrawSelection(RCTVAL(Header), DGRAY);
	SetFont(Config.Headerfont, BLACK);
	DrawTextRect2(&RCT(Header), DateStr(((m_CurX-1)*7+m_CurY-m_iMonthStartWeekDay+m_iMonthStartDay)*3600*24));
}
void CALENDAR::MoveNextM(int mode)
{
	if(mode) m_CurX=6-(MonthDays[m_iCurrMonth]+m_iMonthStartWeekDay)/7;
	m_iMonthStartDay+=MonthDays[m_iCurrMonth];
	if(m_iCurrMonth==11) m_iCurrMonth=0;
	else m_iCurrMonth++;
	m_iMonthStartWeekDay=(m_iMonthStartDay+3)%7;
	switch(mode)
	{
	case 0:
		m_CurX=(m_iMonthStartWeekDay+1)/7+1;
		m_CurY=(m_iMonthStartWeekDay+1)%7;
		break;
	case 1:
		m_CurX=(m_iMonthStartWeekDay+1)/7+1+((m_iMonthStartDay+3)%7>=m_CurY);
		break;
	default:
		m_CurX=(m_iMonthStartWeekDay+mode-1)/7+1;
		m_CurY=(m_iMonthStartWeekDay+mode-1)%7;
	}
	Repaint();
}
void CALENDAR::MovePrevM(int mode)
{
	if(m_iCurrMonth==0) m_iCurrMonth=11;
	else m_iCurrMonth--;
	m_iMonthStartDay-=MonthDays[m_iCurrMonth];
	m_iMonthStartWeekDay=(m_iMonthStartDay+3)%7;
	switch(mode)
	{
	case 0:
		m_CurX=(m_iMonthStartWeekDay+MonthDays[m_iCurrMonth])/7+1;
		m_CurY=(m_iMonthStartWeekDay+MonthDays[m_iCurrMonth])%7;
		break;
	case 1:
		m_CurX=(m_iMonthStartWeekDay+MonthDays[m_iCurrMonth])/7+m_CurX-1;
		break;
	}
	Repaint();
}
void CALENDAR::MoveUp()
{
	InvertAreaBW(m_CurX*(WidX)+1, RCT(Header).h+m_CurY*(WidY)+1, WidX-2, WidY-2);
	DrawSelection(m_CurX*(WidX), RCT(Header).h+m_CurY*(WidY), WidX, WidY, WHITE);
	if((m_CurX-1)*7+(m_CurY-1)-m_iMonthStartWeekDay < 1) MovePrevM(0);
	else
	{
		m_CurY--;
		InvertAreaBW(m_CurX*(WidX)+1, RCT(Header).h+m_CurY*(WidY)+1, WidX-2, WidY-2);
		DrawSelection(m_CurX*(WidX), RCT(Header).h+m_CurY*(WidY), WidX, WidY, BLACK);
		UpdateHeader();
		PartialUpdateBW(RCTVAL(FullScreen));
	}
}
void CALENDAR::MoveDown()
{
	InvertAreaBW(m_CurX*(WidX)+1, RCT(Header).h+m_CurY*(WidY)+1, WidX-2, WidY-2);
	DrawSelection(m_CurX*(WidX), RCT(Header).h+m_CurY*(WidY), WidX, WidY, WHITE);
	if((m_CurX-1)*7+(m_CurY+1)-m_iMonthStartWeekDay > MonthDays[m_iCurrMonth]) MoveNextM(0);
	else
	{
		m_CurY++;
		InvertAreaBW(m_CurX*(WidX)+1, RCT(Header).h+m_CurY*(WidY)+1, WidX-2, WidY-2);
		DrawSelection(m_CurX*(WidX), RCT(Header).h+m_CurY*(WidY), WidX, WidY, BLACK);
		UpdateHeader();
		PartialUpdateBW(RCTVAL(FullScreen));
	}
}
void CALENDAR::MoveLeft()
{
	InvertAreaBW(m_CurX*(WidX)+1, RCT(Header).h+m_CurY*(WidY)+1, WidX-2, WidY-2);
	DrawSelection(m_CurX*(WidX), RCT(Header).h+m_CurY*(WidY), WidX, WidY, WHITE);
	if((m_CurX-2)*7+m_CurY-m_iMonthStartWeekDay < 1) MovePrevM(1);
	else
	{
		m_CurX--;
		InvertAreaBW(m_CurX*(WidX)+1, RCT(Header).h+m_CurY*(WidY)+1, WidX-2, WidY-2);
		DrawSelection(m_CurX*(WidX), RCT(Header).h+m_CurY*(WidY), WidX, WidY, BLACK);
		UpdateHeader();
		PartialUpdateBW(RCTVAL(FullScreen));
	}
}
void CALENDAR::MoveRight()
{
	InvertAreaBW(m_CurX*(WidX)+1, RCT(Header).h+m_CurY*(WidY)+1, WidX-2, WidY-2);
	DrawSelection(m_CurX*(WidX), RCT(Header).h+m_CurY*(WidY), WidX, WidY, WHITE);
	if(m_CurX*7+m_CurY-m_iMonthStartWeekDay > MonthDays[m_iCurrMonth]) MoveNextM(1);
	else
	{
		m_CurX++;
		InvertAreaBW(m_CurX*(WidX)+1, RCT(Header).h+m_CurY*(WidY)+1, WidX-2, WidY-2);
		DrawSelection(m_CurX*(WidX), RCT(Header).h+m_CurY*(WidY), WidX, WidY, BLACK);
		UpdateHeader();
		PartialUpdateBW(RCTVAL(FullScreen));
	}
}
void CALENDAR::MoveToXY(int x, int y)
{
	y-=RCT(Header).h;
	int tempX = x/WidX;
	int tempY = y/WidY;
	lout<<tempX<<" "<<tempY<<endl;
	if(!tempX || y<0) return;
	InvertAreaBW(m_CurX*(WidX)+1, RCT(Header).h+m_CurY*(WidY)+1, WidX-2, WidY-2);
	DrawSelection(m_CurX*(WidX), RCT(Header).h+m_CurY*(WidY), WidX, WidY, WHITE);
	if((tempX-1)*7+(tempY)-m_iMonthStartWeekDay < 1)
	{
		m_CurX = tempX+1;
		m_CurY = tempY;
		MovePrevM(1);
	}
	else if((tempX-1)*7+(tempY)-m_iMonthStartWeekDay > MonthDays[m_iCurrMonth])
	{
		MoveNextM(1+(tempX-1)*7+(tempY)-m_iMonthStartWeekDay-MonthDays[m_iCurrMonth]);
	}
	else
	{
		m_CurX = tempX;
		m_CurY = tempY;
		InvertAreaBW(m_CurX*(WidX)+1, RCT(Header).h+m_CurY*(WidY)+1, WidX-2, WidY-2);
		DrawSelection(m_CurX*(WidX), RCT(Header).h+m_CurY*(WidY), WidX, WidY, BLACK);
		UpdateHeader();
		PartialUpdateBW(RCTVAL(FullScreen));
	}
	Go();
}
void CALENDAR::Go()
{
	if(!m_bBold[(m_CurX-1)*7+m_CurY])
		return;
	CurUnits.clear();
	for(vector<Unit>::iterator i=m_vData.begin(); i!=m_vData.end(); i++)
		for(vector<int>::iterator j=i->m_vDays.begin(); j!=i->m_vDays.end(); j++)
			if(m_iMonthStartDay-m_iMonthStartWeekDay+(m_CurX-1)*7+m_CurY==*j)
				CurUnits.push_back(&(*i));
	
	SetEventHandler(day_handler);
}

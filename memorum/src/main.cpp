#include "main.h"
#ifdef FILELOG
	ostream *GLOBAL::m_lout = (new ofstream(SDCARDDIR"/memorum.log"));
#else
	ostream *GLOBAL::m_lout = &cerr;
#endif
CONFIG   GLOBAL::m_Config;
CARDLIST GLOBAL::m_CardList;
CALENDAR GLOBAL::m_Calendar;

static imenu menu[] = {
	{ ITEM_HEADER,  0, "@Menu", NULL },
	{ ITEM_ACTIVE,  1, "@Configuration", NULL },
	{ ITEM_SEPARATOR, 0, NULL, NULL },
	{ ITEM_ACTIVE,  2, "@Smpl_mode", NULL },
	{ ITEM_ACTIVE,  3, "@Memo_mode", NULL },
	{ ITEM_ACTIVE,  4, "@Test_mode", NULL },
	{ ITEM_SEPARATOR, 0, NULL, NULL },
	{ ITEM_ACTIVE,  5, "@Change_sides", NULL },
	{ ITEM_ACTIVE,  6, "@Rand", NULL },
	{ ITEM_ACTIVE,  7, "@Reset", NULL },
	{ ITEM_SEPARATOR, 0, NULL, NULL },
//	{ ITEM_INACTIVE,  8, "@Statistic", NULL },
	{ ITEM_ACTIVE,  9, "@Calendar", NULL },
	{ ITEM_ACTIVE, 10, "@Calendar_add", NULL },
	{ ITEM_SEPARATOR, 0, NULL, NULL },
	{ ITEM_ACTIVE, 11, "@About_program", NULL },
	{ ITEM_SEPARATOR, 0, NULL, NULL },
	{ ITEM_ACTIVE, 99, "@Exit", NULL },
	{ 0, 0, NULL, NULL }
};
int calendar_handler(int type, int par1, int par2);
void menu_handler(int index)
{
	lout<<"menu_handler:  "<<index<<endl;
	switch(index)
	{
	case 1: //	@Configuration
		Config.Show();
		break;
	case 2: //	@Smpl_mode
		CardList.SetSmplImpl();
		CardList.Repaint();
		break;
	case 3: //	@Memo_mode
		CardList.SetMemoImpl();
		CardList.Repaint();
		break;
	case 4: //	@Test_mode
		CardList.SetTestImpl();
		CardList.Repaint();
		break;
	case 5: //	@Change_sides
		CardList.ChangeSides();
		CardList.Repaint();
		break;
	case 6: //	@Rand
		CardList.Randomise();
		CardList.Repaint();
		break;
	case 7: //	@Reset
		CardList.Reset();
		CardList.Repaint();
		break;
	case 8: //	@Statistic
		break;
	case 9: //	@Calendar
		SetEventHandler(calendar_handler);
		break;
	case 10: //	@Calendar_add
		Calendar.AddCurrCL();
		Calendar.Save();
		break;
	case 11: //	@About_program
		Message(ICON_INFORMATION, "Memorum", "Memorum v1.0 RC\n(c) Naydenov Ivan (aka Samogot)",5000);
		break;
	case 99: //	@Exit
		CloseApp();
		break;
	}
}
int main_handler(int type, int par1, int par2)
{
	lout<<"main_handler:  "<<type<<" "<<par1<<" "<<par2<<endl;
	static int PointerX, PointerY;
	switch(type)
	{
	case EVT_POINTERDOWN:
		PointerX=par1;
		PointerY=par2;
		break;
	case EVT_POINTERUP:
		if(CardList.IsSell() && par2>Config.m_Rects[Footer].y)
			CardList.SetSell(par1/(ScreenWidth()/4));
		else if((PointerY-par2<150&&par2-PointerY<150) && par1-PointerX>200)
			CardList.MoveNext();
		else if((PointerY-par2<150&&par2-PointerY<150) && par1-PointerX<-200)
			CardList.MovePrev();
		break;
	case EVT_ORIENTATION:
		SetOrientation(par1);
		break;
	case EVT_INIT:
		srand(time(0));
		Config.Init();
		CardList.Init();
		Calendar.Init();
		break;
	case EVT_SHOW:
		CardList.Repaint();
		break;
	case EVT_KEYPRESS:
		switch(par1)
		{
		case KEY_BACK:
			CloseApp();
			break;
		case KEY_UP:
			OpenMenu(menu,0,ScreenWidth(),ScreenHeight(),menu_handler);
			break;
		case KEY_RIGHT:
			if(!CardList.SellNext())
				CardList.MoveNext();
			break;
		case KEY_LEFT:
			if(!CardList.SellPrev())
				CardList.MovePrev();
			break;
		case KEY_OK:
			if(!CardList.SellOK())
				CardList.Sound();
			break;
		case KEY_DOWN:
			CardList.Expand();
			break;
		}
		break;
	}
	return 0;
}

int main(int argc, char **argv)
{
	if(argc>1) 
	{
		CardList.SetFile(argv[1]);
		InkViewMain(main_handler);
	}
	else InkViewMain(calendar_handler);	
	return 0;
}
int calendar_handler(int type, int par1, int par2)
{
	lout<<"calendar_handler:  "<<type<<" "<<par1<<" "<<par2<<endl;
	static int PointerX, PointerY;
	switch(type)
	{
	case EVT_POINTERDOWN:
		PointerX=par1;
		PointerY=par2;
		break;
	case EVT_POINTERUP:
		Calendar.MoveToXY(par1,par2);
		break;
	case EVT_ORIENTATION:
		SetOrientation(par1);
		break;	
	case EVT_INIT:
		srand(time(0));
		Config.Init();
		Calendar.Init(true);
		break;
	case EVT_SHOW:
		Calendar.Repaint();
		break;	
	case EVT_KEYRELEASE:
		switch(par1)
		{
		case KEY_BACK:
			if(par2>0 || Calendar.Start())
				CloseApp();
			else
				SetEventHandler(main_handler);
			break;
		case KEY_RIGHT:
			if(par2>0) Calendar.MoveNextM();
			else Calendar.MoveRight();
			break;
		case KEY_LEFT:
			if(par2>0) Calendar.MovePrevM();
			else Calendar.MoveLeft();
			break;
		case KEY_UP:
			Calendar.MoveUp();
			break;
		case KEY_DOWN:
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
unsigned int csel;
bool pred(Unit& u) { return &u==Calendar.CurUnits[csel]; }
void delete_unit(int button)
{
	if(button==1)
	{
		Calendar.m_vData.erase(std::find_if(Calendar.m_vData.begin(), Calendar.m_vData.end(), pred));
		Calendar.CurUnits.erase(Calendar.CurUnits.begin()+csel);
		Calendar.Save();
		SetEventHandler(day_handler);
	}
}
int day_handler(int type, int par1, int par2)
{
	lout<<"day_handler:  "<<type<<" "<<par1<<" "<<par2<<endl;
	static int PointerX, PointerY;
	static int hei=Config.Delftfont->height+1;
	switch(type)
	{
	case EVT_POINTERDOWN:
		PointerX=par1;
		PointerY=par2;
		break;
	case EVT_POINTERUP:
		par2-=RCT(Header).h;
		if(par2<0 || par2/hei>=(int)Calendar.CurUnits.size()) break;
		DrawSelection(0, RCT(Header).h+hei*csel, RCT(FullScreen).w, hei, WHITE);
		csel=par2/hei;
		DrawSelection(0, RCT(Header).h+hei*csel, RCT(FullScreen).w, hei, BLACK);
		PartialUpdateBW(RCTVAL(FullScreen));
		CardList.SetFile(Calendar.CurUnits[csel]->m_sFullFileName);
		CardList.Init();
		SetEventHandler(main_handler);
		break;
	case EVT_ORIENTATION:
		SetOrientation(par1);
		break;
	case EVT_SHOW:
		if(Calendar.CurUnits.empty())
		{
			SetEventHandler(calendar_handler);
			return 0;
		}
		ClearScreen();
		Calendar.UpdateHeader();
		SetFont(Config.Delftfont,BLACK);
		char buf[5];
		for(unsigned int i=0; i<Calendar.CurUnits.size(); i++)
		{
			sprintf(buf,"%d ",Calendar.CurUnits[i]->m_iCardsCount);
			DrawTextRect(5, RCT(Header).h+hei*i, RCT(FullScreen).w, hei, sz(Calendar.CurUnits[i]->m_sCardListName), ALIGN_LEFT|VALIGN_MIDDLE);
			DrawTextRect(0, RCT(Header).h+hei*i, RCT(FullScreen).w, hei, buf, ALIGN_RIGHT|VALIGN_MIDDLE);
		}
		csel=0;
		DrawSelection(0, RCT(Header).h+hei*csel, RCT(FullScreen).w, hei, BLACK);
		FullUpdate();
		break;
	case EVT_KEYRELEASE:
		switch(par1)
		{
		case KEY_BACK:
			if(par2>0)
				CloseApp();
			else
				SetEventHandler(calendar_handler);
			break;
		case KEY_UP:
			DrawSelection(0, RCT(Header).h+hei*csel, RCT(FullScreen).w, hei, WHITE);
			if(csel==0) csel=Calendar.CurUnits.size()-1;
			else csel--;
			DrawSelection(0, RCT(Header).h+hei*csel, RCT(FullScreen).w, hei, BLACK);
			PartialUpdateBW(RCTVAL(FullScreen));
			break;
		case KEY_DOWN:
			DrawSelection(0, RCT(Header).h+hei*csel, RCT(FullScreen).w, hei, WHITE);
			if(csel==Calendar.CurUnits.size()-1) csel=0;
			else csel++;
			DrawSelection(0, RCT(Header).h+hei*csel, RCT(FullScreen).w, hei, BLACK);
			PartialUpdateBW(RCTVAL(FullScreen));
			break;
		case KEY_OK:
			CardList.SetFile(Calendar.CurUnits[csel]->m_sFullFileName);
			CardList.Init();
			SetEventHandler(main_handler);
			break;
		case KEY_DELETE:
			Dialog(ICON_QUESTION, "Memorum", "@Conifurm_delete", "@Yes", "@No", delete_unit);
			break;
		}
	}
	return 0;
}

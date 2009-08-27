#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <dlfcn.h>
#include "inkview.h"

#define CELLWIDE 29
#define ROWHEIGHT 20
#define MAXFIELDWIDE 19

static int BOMBSCOUNT ;
static int FIELDWIDE;

static int oldOrientation;
extern const ibitmap mine00,mine01,mine02,mine03,mine04,mine05,mine06,mine07,mine08,minecl,minefl,mbomb,mcursor,mcursor1;
static int cursor_x; 
static int cursor_y; 

static int flags_count=0;
static int opened_fields=0;
static int screen_offs;

static int visible[MAXFIELDWIDE][MAXFIELDWIDE];
static int bombs[MAXFIELDWIDE][MAXFIELDWIDE];

static const ibitmap *cellbmp[] = {&mbomb,&minecl,&mine01,&mine02,&mine03,&mine04,&mine05,&mine06,&mine07,&mine08,&mcursor,&mine00,&minefl,&mcursor1};

static int screen_x0;
static int screen_y0;
static int screen_x1;
static int screen_y1;

void BuildGameField( void);
void StartNewGameDialog( int button);
void ChooseLevel( int button);


//==============

ifont *arial8n, *arial12, *arialb12, *cour16, *cour24, *times20;
int main_handler(int type, int par1, int par2);

void mainscreen_repaint() {
	ClearScreen();
	StartNewGameDialog( 1);
}


void ScreenUpdate( void)
{
	int dx = screen_x1-screen_x0;
	int dy = screen_y1-screen_y0;
	if( dx && dy)
		PartialUpdateBW( screen_x0, screen_y0, dx, dy);
		
	screen_x0 = -1;
	screen_y0 = -1;
	screen_x1 = -1;
	screen_y1 = -1;
}

void DrawTile( int scr_x, int scr_y, const ibitmap *bmp)
{
	DrawBitmap( scr_x-1, scr_y-1, bmp);
	if( screen_x0 == -1 || screen_x0 > scr_x-CELLWIDE)
		screen_x0 = scr_x-CELLWIDE;
	if( screen_y0 == -1 || screen_y0 > scr_y-CELLWIDE)
		screen_y0 = scr_y-CELLWIDE;
	if( screen_x1 == -1 || screen_x1 < scr_x+bmp->width)
		screen_x1 = scr_x+bmp->width;
	if( screen_y1 == -1 || screen_y1 < scr_y+bmp->height)
		screen_y1 = scr_y+bmp->height;
}

void DrawFieldCell( int x, int y, int cursor)
{
	if(!cursor)
	{
		if (visible[x][y]==1) DrawTile( screen_offs+CELLWIDE*x,screen_offs+CELLWIDE*y , cellbmp[bombs[x][y]+1]);
		else if (visible[x][y]==-1)	 DrawTile( screen_offs+CELLWIDE*x, screen_offs+CELLWIDE*y, cellbmp[12]);
		else DrawTile( screen_offs+CELLWIDE*x, screen_offs+CELLWIDE*y, cellbmp[11]);
	}
	if(cursor) 
	{
		if (visible[x][y]==-1) DrawTile( screen_offs+CELLWIDE*x, screen_offs+CELLWIDE*y, &mcursor);
		else DrawTile( screen_offs+CELLWIDE*x, screen_offs+CELLWIDE*y, &mcursor1);
	}
}

void PrepareGameField( void)
{
	char buffer [2];
	cursor_x = FIELDWIDE/2;
	cursor_y = FIELDWIDE/2;
	screen_offs = (ScreenWidth()-CELLWIDE*FIELDWIDE)/2; //screen offset from top left
	FillArea(0, 0, ScreenWidth(),ScreenWidth(), WHITE);	
	SetFont( cour24, BLACK);
  	sprintf(buffer,"%i",BOMBSCOUNT);
	DrawString( ScreenWidth()-30, 25, buffer);
	flags_count=0;
	opened_fields=0;
}

void ClearGameTable( void)
{
	int i, j;
	for( j = 0; j <FIELDWIDE; j++)
		for( i = 0; i <FIELDWIDE; i++)
		{
			bombs[i][j] = 0;
			visible[i][j] = 0;
		}
}

void GenerateBombs( void)
{
	int i,j,k=0;
	srand(time(0));
	while (k<BOMBSCOUNT)
	{
			i=rand() % FIELDWIDE;
			j=rand() % FIELDWIDE;
			if (bombs[i][j]==0)
			{
				bombs[i][j]=-1;
				k++;
			}
	}
}

void CalculateBombsCount( void)
{
	int i, j,i1,j1;
	for( j = 0; j < FIELDWIDE; j++)
		for( i = 0; i < FIELDWIDE; i++)
		{
			if (bombs[i][j]!=-1)
			for( j1 = -1; j1 <=1; j1++)
				for( i1 = -1; i1 <=1 ; i1++)
					if ((j+j1<FIELDWIDE)&&(j+j1>=0)&&(i+i1<FIELDWIDE)&&(i+i1>=0)&&(bombs[i+i1][j+j1]==-1)) 
						bombs[i][j]++;
			DrawFieldCell( i, j, 0);
		}
}

void BuildGameField( void)
{
	PrepareGameField();
	ClearGameTable();
	GenerateBombs();
	CalculateBombsCount();
	cursor_x = FIELDWIDE/2;
	cursor_y = FIELDWIDE/2;
	DrawFieldCell( cursor_x, cursor_y, 1);
}


void StartNewGameDialog( int button)
{
	if( button == 1)
		Dialog(ICON_QUESTION, "Choose dificulty", "Choose dificulty for next game:", "Novice", "Average", ChooseLevel);
	if( button == 2)
		CloseApp();
}

void ChooseLevel( int button)
{
	if( button == 1)
	{
		BOMBSCOUNT=10;
		FIELDWIDE=9;
	}
	if( button == 2)
	{
		BOMBSCOUNT=40;
		FIELDWIDE=16;
	}
	BuildGameField();
	FullUpdate();

}

void StartNewGame( void)
{
	Dialog(ICON_QUESTION, "Start New Game", "Do you wish to start a new game?", "Yes", "No", StartNewGameDialog);
}

void MoveCursor( int dx, int dy)
{
	DrawFieldCell( cursor_x, cursor_y, 0);	
	cursor_x += dx;
	cursor_y += dy;
	if( cursor_x >= FIELDWIDE)
		cursor_x -= FIELDWIDE;
	if( cursor_y >= FIELDWIDE)
		cursor_y -= FIELDWIDE;
	if( cursor_x < 0)
		cursor_x += FIELDWIDE;
	if( cursor_y < 0)
		cursor_y += FIELDWIDE;
	DrawFieldCell( cursor_x, cursor_y, 1);
	ScreenUpdate();
}

void AddMove( )
{
	if (visible[cursor_x][cursor_y]==0)
	{
		opened_fields++;
		visible[cursor_x][cursor_y]=1;
		if (bombs[cursor_x][cursor_y]==-1)
		{
			FillArea( screen_offs, 25-CELLWIDE, ScreenWidth()-screen_offs*2-CELLWIDE*2, 2*CELLWIDE, WHITE);
			DrawString( screen_offs+CELLWIDE/2+CELLWIDE, 25, "YOU LOSE!!!");
			PartialUpdateBW( screen_offs,  25-CELLWIDE, ScreenWidth()-screen_offs*2-CELLWIDE*2, 2*CELLWIDE);
			DrawFieldCell( cursor_x, cursor_y, 0);
			ScreenUpdate();
			sleep(1);
			StartNewGame();
		}
		int n,i,j;
		n=1;
		if (bombs[cursor_x][cursor_y]==0)
			while (n>0)
			{
				n=0;
				for( j = 0; j <FIELDWIDE; j++)
					for( i = 0; i <FIELDWIDE; i++)
						if ((bombs[i][j]!=-1)&&(visible[i][j]==0)&&(
						((visible[i+1][j]==1)&&(bombs[i+1][j]==0)&&(i+1!=FIELDWIDE))||
						((visible[i-1][j]==1)&&(bombs[i-1][j]==0)&&(i!=0))||
						((visible[i][j+1]==1)&&(bombs[i][j+1]==0)&&(j+1!=FIELDWIDE))||
						((visible[i][j-1]==1)&&(bombs[i][j-1]==0)&&(j!=0))||
						((visible[i+1][j+1]==1)&&(bombs[i+1][j+1]==0)&&(i+1!=FIELDWIDE)&&(j+1!=FIELDWIDE))||
						((visible[i+1][j-1]==1)&&(bombs[i+1][j-1]==0)&&(i+1!=FIELDWIDE)&&(j!=0))||
						((visible[i-1][j+1]==1)&&(bombs[i-1][j+1]==0)&&(i!=0)&&(j+1!=FIELDWIDE))||
						((visible[i-1][j-1]==1)&&(bombs[i-1][j-1]==0)&&(i!=0)&&(j!=0)))) 
						{
							visible[i][j]=1;
							opened_fields++;
							DrawFieldCell(i,j, 0);
							n++;
						}
			}
		DrawFieldCell( cursor_x, cursor_y, 0);
	}
	DrawFieldCell( cursor_x, cursor_y, 1);
	ScreenUpdate();
	if (opened_fields==FIELDWIDE*FIELDWIDE-BOMBSCOUNT)
	{
		FillArea( screen_offs, 25-CELLWIDE, ScreenWidth()-screen_offs*2-CELLWIDE*2, 2*CELLWIDE, WHITE);
		DrawString( screen_offs+CELLWIDE/2+CELLWIDE, 25, "YOU WON!!!");
		PartialUpdateBW( screen_offs,  25-CELLWIDE, ScreenWidth()-screen_offs*2-CELLWIDE*2, 2*CELLWIDE);
		StartNewGame();
	}
}

void RedrawBoard( void)
{
	int i, j;
	for( j = 0; j < FIELDWIDE; j++)
		for( i = 0; i < FIELDWIDE; i++)
			DrawFieldCell( i, j, 0);
	DrawFieldCell( cursor_x, cursor_y, 1);
	ScreenUpdate();
}

void PutFlag( void)
{
	if (visible[cursor_x][cursor_y]==0) 
	{
		visible[cursor_x][cursor_y]=-1;
		flags_count++;
		//screen_x0, screen_y0
	}
	else if (visible[cursor_x][cursor_y]==-1) 
	{
		visible[cursor_x][cursor_y]=0;
		flags_count--;
	}
	DrawFieldCell( cursor_x, cursor_y, 0);
	DrawFieldCell( cursor_x, cursor_y, 1);
	if(!((visible[cursor_x][cursor_y]==0)||(visible[cursor_x][cursor_y]==-1))) ScreenUpdate();
	else 
	{
		char buffer [2];
		sprintf(buffer,"%i",BOMBSCOUNT-flags_count);
		FillArea( ScreenWidth()-30, 25, 30, 25, WHITE);
		DrawString( ScreenWidth()-30, 25, buffer);	
		PartialUpdateBW(screen_offs+CELLWIDE*cursor_x, 25, ScreenWidth()-screen_offs+CELLWIDE*cursor_x, screen_offs+CELLWIDE*cursor_y);	
	}
}

int main_handler(int type, int par1, int par2)
{
//	fprintf(stderr, "[%i %i %i]\n", type, par1, par2);

	if (type == EVT_INIT) {
		// occurs once at startup, only in main handler
		arial8n = OpenFont("DroidSans", 8, 0);
		arial12 = OpenFont("DroidSans", 12, 1);
		arialb12 = OpenFont("DroidSans", 12, 1);
		cour16 = OpenFont("cour", 16, 1);
		cour24 = OpenFont("cour", 24, 1);
		times20 = OpenFont("times", 20, 1);
		oldOrientation = GetOrientation();
		SetOrientation(ROTATE0);

	}

	if (type == EVT_SHOW) {
		// occurs when this event handler becomes active
		mainscreen_repaint();
	//	RedrawBoard();
	}

	if (type == EVT_KEYPRESS || type == EVT_KEYREPEAT) {
		int mv = par2>0 ? 4 : 1;
		switch (par1) {

			case KEY_OK:
				if( par2==0)
				{
					AddMove();
				}
				//else if( par2>=1)
				//{
				//	RedrawBoard();
			//}
			//	break;

			case KEY_BACK:
			    PutFlag();
				break;
			case KEY_LEFT:
				MoveCursor( -mv, 0);
				break;
			case KEY_RIGHT:
				MoveCursor( +mv, 0);
				break;
			case KEY_UP:
				MoveCursor( 0, -mv);
				break;
			case KEY_DOWN:
				MoveCursor( 0, +mv);
				break;
			case KEY_MENU:
				CloseApp();
				break;
			case KEY_DELETE:
				if( par2==1)
					StartNewGame();
				else if( !par2)
					CloseApp();
				break;

		}
	}

	if (type == EVT_EXIT) {
		SetOrientation(oldOrientation);
	}

	return 0;
}

int main(int argc, char **argv)
{
	InkViewMain(main_handler);
	return 0;
}


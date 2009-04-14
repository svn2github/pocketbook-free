#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <dlfcn.h>
#include "inkview.h"
//#include <stddef.h>
//#include "json.h"

#define SCREENWIDE 600
#define FIELDWIDE 19
#define CELLWIDE 29

extern const ibitmap goe01, goe02, goe03, goe04, goe05, goe05dot, goe06, goe07, goe08, goe09, goeblack, goecursor, goewhite;

//static char filename_state[] = "/mnt/ext1/system/state/games/goe.state";
static char filename_state[] = "./goe.state";

static int cursor_x = FIELDWIDE/2; //number of current move
static int cursor_y = FIELDWIDE/2; //number of current move
static char str_move_black[] = "Black's move";
static char str_move_white[] = "White's move";

static int screen_offs = (SCREENWIDE-CELLWIDE*FIELDWIDE)/2; //screen offset from top left

//static const int field_wide = 19;
static int move_x[FIELDWIDE*FIELDWIDE*3];
static int move_y[FIELDWIDE*FIELDWIDE*3];
static int move_t[FIELDWIDE*FIELDWIDE*3];
static int current_move_number = 0; //number of current move
static int current_move_color = 0; //0=black, 1=white
static int stones_white = 0;
static int stones_white_captured = 0;
static int stones_black = 0;
static int stones_black_captured = 0;
static unsigned short int field[FIELDWIDE][FIELDWIDE];//[x][y]
static unsigned short int stones[FIELDWIDE][FIELDWIDE];//[x][y]
static const ibitmap *cellbmp[] = {&goe01, &goe02, &goe03, &goe04, &goe05, &goe06, &goe07, &goe08, &goe09, &goe05dot, &goeblack, &goewhite};

void BuildGameField( void);
void ShowWhichPlayerMove( void);
void StartNewGameDialog( int button);
void ScoreCalcProc( int t, int dir);

//==============

static iconfig *testcfg = NULL;
static char *choice_variants[] = { "qqq", "www", "@Contents", "rrr", NULL };
static char *choice_variants2[] = { "q1", "q2", "q3", "q4", "w1", "w2", "w3", "w4", "e1", "e2", "e3", "e4", "r1", "r2", "r3", "r4", "t1", "t2", "t3", "t4", NULL };
static char *choice_variants3[] = { "q1", "q2", "q3", "q4", "w's", ":w1", ":w2", ":w3", ":w4", "e1", "e2", "e3", "e4", "r1", "r2", "r3", "r4", "t1", "t2", "t3", "t4", NULL };
static char *index_variants[] = { "value1", "value2", "value3", NULL };
static iconfigedit testce[] = {

	{ "About device", NULL, CFG_INFO, "Name: PocketBook 310\nSerial: 123456789", NULL },
	{ "Text edit", "param.textedit", CFG_TEXT, "qwerty", NULL },
	{ "Choice", "param.choice", CFG_CHOICE, "eee", choice_variants },
	{ "Many variants", "param.choice2", CFG_CHOICE, "qqq", choice_variants2 },
	{ "Multi-level", "param.choice3", CFG_CHOICE, "cvb", choice_variants3 },
	{ "Font", "param.font", CFG_FONT, "Arial,24", NULL },
	{ "Font face", "param.fontface", CFG_FONTFACE, "Arial", NULL },
	{ "Index", "param.index", CFG_INDEX, "2", index_variants },
	{ "Time", "param.time", CFG_TIME, "1212396151", NULL },
	{ NULL, NULL, 0, NULL, NULL}

};

ifont *arial8n, *arial12, *arialb12, *cour16, *cour24, *times20;

int main_handler(int type, int par1, int par2);

#define ROWHEIGHT 20
void msg( char *s, int row, int mv_color)
{
	int x = screen_offs+CELLWIDE/2+CELLWIDE;
	int y = SCREENWIDE+row*ROWHEIGHT;
	int dx = SCREENWIDE-screen_offs*2-CELLWIDE*2;
	int dy = ROWHEIGHT;

	if( mv_color==2)
		DrawBitmap( screen_offs, y-CELLWIDE/3, &goewhite);
	else if( mv_color==1)
		DrawBitmap( screen_offs, y-CELLWIDE/3, &goeblack);
	else
		FillArea( screen_offs, y-CELLWIDE/3, CELLWIDE, CELLWIDE, WHITE);
	//PartialUpdate( screen_offs, y-CELLWIDE/3, CELLWIDE, CELLWIDE);
		
	FillArea( x, y, dx, dy, WHITE);
	SetFont( arialb12, BLACK);
	DrawString( x, y, s);
	//PartialUpdate( x, y, dx, dy);

	PartialUpdate( screen_offs,  y-CELLWIDE/3, dx, CELLWIDE);
}

void mainscreen_repaint() {

	//char buf[64];
	//ibitmap *b;
	//int i;

	ClearScreen();
/*
	SetFont(arialb12, BLACK);
	DrawString(5, 2, "InkView library demo, press OK to open menu");

	DrawBitmap(0, 20, &background);
	DrawBitmap(120, 30, &books);

	DrawLine(5, 500, 595, 500, BLACK);
	DrawLine(5, 502, 595, 502, DGRAY);
	DrawLine(5, 504, 595, 504, LGRAY);
	DrawLine(19, 516, 20, 517, BLACK);
	DrawLine(22, 516, 23, 516, BLACK);

	for (i=5; i<595; i+=3) DrawPixel(i, 507, BLACK);

	DrawRect(5, 510, 590, 10, BLACK);

	for (i=0; i<256; i++) FillArea(35+i*2, 524, 2, 12, i | (i << 8) | (i << 16));

	b = BitmapFromScreen(0, 520, 600, 20);
	DrawBitmap(0, 550, b);
	free(b);
	InvertArea(0, 550, 600, 20);
	DimArea(0, 575, 600, 10, BLACK);

	if (! orient) {
		Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 10, 600,  6,  6, 0);
		Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 20, 600, 10,  6, 0);
		Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 35, 600, 30,  6, 0);
		Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 10, 610,  6, 10, 0);
		Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 20, 610, 10, 10, 0);
		Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 35, 610, 30, 10, 0);
		Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 10, 625,  6, 30, 0);
		Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 20, 625, 10, 30, 0);
		Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 35, 625, 30, 30, 0);
	}

	SetFont(arial8n, BLACK);
	DrawString(350, 600, "Arial 8 with no antialiasing");
	SetFont(arial12, BLACK);
	DrawString(350, 615, "Arial 12 regular");
	SetFont(arialb12, BLACK);
	DrawString(350, 630, "Arial 12 bold");
	SetFont(cour16, BLACK);
	DrawString(350, 645, "Courier 16");
	SetFont(cour24, BLACK);
	DrawString(350, 660, "Courier 24");
	DrawSymbol(500, 660, ARROW_LEFT);
	DrawSymbol(520, 660, ARROW_RIGHT);
	DrawSymbol(540, 660, ARROW_UP);
	DrawSymbol(560, 660, ARROW_DOWN);
	SetFont(times20, BLACK);
	DrawString(350, 680, "Times 20");
	DrawSymbol(450, 680, ARROW_LEFT);
	DrawSymbol(470, 680, ARROW_RIGHT);
	DrawSymbol(490, 680, ARROW_UP);
	DrawSymbol(510, 680, ARROW_DOWN);

	//DrawTextRect(25, 400, 510, 350, sometext, ALIGN_LEFT);
//*/	
	StartNewGameDialog( 1);
}

void config_ok() {
	SaveConfig(testcfg);
}

/*
void menu1_handler(int index) {
	switch (index) {

		case 102:
//			orient++; if (orient > 2) orient = 0;
//			SetOrientation(orient);
			mainscreen_repaint();
			break;

		case 104:
			Message(ICON_INFORMATION, "Message", "This is a message.\n"
				"It will disappear after 5 seconds, or press any key", 5000);
			break;

		case 105:
			Dialog(ICON_QUESTION, "Dialog", "This is a dialog.\n"
				"Do you like it?", "Yes", "No", dialog_handler);
			break;

		case 112:
			OpenConfigEditor("Configuration", testcfg, testce, config_ok, NULL);
			break;
	}

}
*/

void ShowWhichPlayerMove( void)
{
	char str[512] = "";
	
	if( current_move_color)
		strcpy( str, str_move_white);
	else
		strcpy( str, str_move_black);

	sprintf( str, "%s        B:[ %i, %i ]      W:[ %i, %i ]", str, stones_black, stones_black_captured, stones_white, stones_white_captured);
	
	msg( str, 0, current_move_color+1);
}

static int screen_x0;
static int screen_y0;
static int screen_x1;
static int screen_y1;

void ScreenUpdate( void)
{
	int dx = screen_x1-screen_x0;
	int dy = screen_y1-screen_y0;

	if( dx && dy)
		PartialUpdate( screen_x0, screen_y0, dx, dy);
		
	screen_x0 = -1;
	screen_y0 = -1;
	screen_x1 = -1;
	screen_y1 = -1;
}

void DrawTile( int scr_x, int scr_y, const ibitmap *bmp)
{
	DrawBitmap( scr_x, scr_y, bmp);
	if( screen_x0 == -1 || screen_x0 > scr_x)
		screen_x0 = scr_x;
	if( screen_y0 == -1 || screen_y0 > scr_y)
		screen_y0 = scr_y;
	if( screen_x1 == -1 || screen_x1 < scr_x+bmp->width)
		screen_x1 = scr_x+bmp->width;
	if( screen_y1 == -1 || screen_y1 < scr_y+bmp->height)
		screen_y1 = scr_y+bmp->height;
}

void DrawFieldCell( int x, int y, int cursor)
{
	DrawTile( screen_offs+CELLWIDE*x, screen_offs+CELLWIDE*y, cellbmp[field[x][y]]);
	if( stones[x][y])
		DrawTile( screen_offs+CELLWIDE*x, screen_offs+CELLWIDE*y, cellbmp[stones[x][y]+9]);
	if( cursor)
		DrawTile( screen_offs+CELLWIDE*x, screen_offs+CELLWIDE*y, &goecursor);
}

void BuildGameField( void)
{
	int i, j;
	
	for( j = 0; j < FIELDWIDE; j++)
		for( i = 0; i < FIELDWIDE; i++)
		{
			if( i==0)
			{
				if( j==0)
					field[i][j] = 0;
				else if( j==FIELDWIDE-1)
					field[i][j] = 6;
				else
					field[i][j] = 3;
			}
			else if( i==FIELDWIDE-1)
			{
				if( j==0)
					field[i][j] = 2;
				else if( j==FIELDWIDE-1)
					field[i][j] = 8;
				else
					field[i][j] = 5;
			}
			else
			{
				if( j==0)
					field[i][j] = 1;
				else if( j==FIELDWIDE-1)
					field[i][j] = 7;
				else
					field[i][j] = 4;
			}
			if( (i==3 || i==FIELDWIDE-4 || i==FIELDWIDE/2) && (j==3 || j==FIELDWIDE-4 || j==FIELDWIDE/2))
				field[i][j] = 9;
			stones[i][j] = 0;
			DrawFieldCell( i, j, 0);
		}
	
	cursor_x = FIELDWIDE/2;
	cursor_y = FIELDWIDE/2;
	DrawFieldCell( cursor_x, cursor_y, 1);
	current_move_number = 0;
	current_move_color = 0;
	
	stones_white = 180;
	stones_white_captured = 0;
	stones_black = 181;
	stones_black_captured = 0;
}

void StartNewGameDialog( int button)
{
	if( button == 1)
	{
		BuildGameField();
		ShowWhichPlayerMove();
		FullUpdate();
	}
}

void StartNewGame( void)
{
	Dialog(ICON_QUESTION, "Start New Game", "Do you wish to start a new game (clear game field)?", "Yes", "No", StartNewGameDialog);
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

void AddMove( int flg)
{
	if( stones[cursor_x][cursor_y] && !flg)
		return;
	if( !stones[cursor_x][cursor_y] && flg)
		return;
	move_x[current_move_number] = cursor_x;
	move_y[current_move_number] = cursor_y;
	if( !flg)
	{
		stones[cursor_x][cursor_y] = current_move_color+1;
		move_t[current_move_number] = stones[cursor_x][cursor_y];
	}
	else
	{
		move_t[current_move_number] = stones[cursor_x][cursor_y]|0x10;
		stones[cursor_x][cursor_y] = 0;
	}
	ScoreCalcProc( move_t[current_move_number], 1);
	current_move_number++;
	DrawFieldCell( cursor_x, cursor_y, 1);
	ScreenUpdate();
	if( !flg)
		current_move_color = current_move_color ? 0 : 1;
	ShowWhichPlayerMove();
}

void DelMove( void)
{
	if( !current_move_number)
		return;
	current_move_number--;
	ScoreCalcProc( move_t[current_move_number], -1);
	if( move_t[current_move_number]&0x10)
		stones[move_x[current_move_number]][move_y[current_move_number]] = move_t[current_move_number]&0x0f;
	else
	{
		stones[move_x[current_move_number]][move_y[current_move_number]] = 0;
		current_move_color = current_move_color ? 0 : 1;
	}
	DrawFieldCell( cursor_x, cursor_y, 0);
	cursor_x = move_x[current_move_number];
	cursor_y = move_y[current_move_number];
	DrawFieldCell( cursor_x, cursor_y, 1);
	ScreenUpdate();
	ShowWhichPlayerMove();
}

void WriteState( void)
{
	FILE *f;
	int i;
	f = fopen( filename_state, "wb");
	if( f == NULL)
		return;

	i = FIELDWIDE;
	fwrite( &i, sizeof(int), 1, f);
	fwrite( &cursor_x, sizeof(int), 1, f);
	fwrite( &cursor_y, sizeof(int), 1, f);
	fwrite( &current_move_color, sizeof(int), 1, f);
	fwrite( &current_move_number, sizeof(int), 1, f);
	for( i=0; i<current_move_number; i++)
	{
		fwrite( &move_x[i], sizeof(int), 1, f);
		fwrite( &move_y[i], sizeof(int), 1, f);
		fwrite( &move_t[i], sizeof(int), 1, f);
	}
	
	fclose( f);
}

void ScoreCalcProc( int t, int dir)
{
	if( dir>=0)
		dir = 1;
	else
		dir = -1;
	
	if( t&0x10)
	{
		if( (t&0x0f)==1)
			stones_white_captured += dir;
		else if( (t&0x0f)==2)
			stones_black_captured += dir;
	}
	else if( t==1)
		stones_black -= dir;
	else if( t==2)
		stones_white -= dir;
}

void ParseHistory(void)
{
	int i;

	stones_white = 180;
	stones_white_captured = 0;
	stones_black = 181;
	stones_black_captured = 0;

	for( i=0; i<current_move_number; i++)
	{
		ScoreCalcProc( move_t[i], 1);
		if( move_t[i]&0x10)
			stones[move_x[i]][move_y[i]] = 0;//move_t[i];
		else
			stones[move_x[i]][move_y[i]] = move_t[i]&0x0f;
	}
}

void ReadState( void)
{
	FILE *f;
	int i;
	f = fopen( filename_state, "rb");
	if( f == NULL)
		return;

	fread( &i, sizeof(int), 1, f);//fieldwide
	fread( &cursor_x, sizeof(int), 1, f);
	fread( &cursor_y, sizeof(int), 1, f);
	fread( &current_move_color, sizeof(int), 1, f);
	fread( &current_move_number, sizeof(int), 1, f);
	fprintf(stderr, "ReadState: history[%i]\n", current_move_number);
	for( i=0; i<current_move_number; i++)
	{
		fread( &move_x[i], sizeof(int), 1, f);
		fread( &move_y[i], sizeof(int), 1, f);
		fread( &move_t[i], sizeof(int), 1, f);
	}
	
	fclose( f);
	ParseHistory();
}

void RedrawBoard( void)
{
	int i, j;
	
	for( j = 0; j < FIELDWIDE; j++)
		for( i = 0; i < FIELDWIDE; i++)
		{
			DrawFieldCell( i, j, 0);
		}
	
	DrawFieldCell( cursor_x, cursor_y, 1);
	ScreenUpdate();
	ShowWhichPlayerMove();
}

int main_handler(int type, int par1, int par2)
{
	fprintf(stderr, "[%i %i %i]\n", type, par1, par2);

	if (type == EVT_INIT) {
		// occurs once at startup, only in main handler
		testcfg = OpenConfig("/mnt/ext1/goe_the_game.cfg", testce);
		arial8n = OpenFont("DroidSans", 8, 0);
		arial12 = OpenFont("DroidSans", 12, 1);
		arialb12 = OpenFont("DroidSans", 12, 1);
		cour16 = OpenFont("cour", 16, 1);
		cour24 = OpenFont("cour", 24, 1);
		times20 = OpenFont("times", 20, 1);
	}

	if (type == EVT_SHOW) {
		// occurs when this event handler becomes active
		mainscreen_repaint();
		ReadState();
		RedrawBoard();
	}

	if (type == EVT_KEYPRESS || type == EVT_KEYREPEAT) {
		int mv = par2>0 ? 4 : 1;
		switch (par1) {

			case KEY_OK:
				if( par2==0)
				{
					//msg( "AddMove()", 0);
					AddMove( 0);
				}
				else if( par2==1)
				{
					ParseHistory();
					RedrawBoard();
				}
				//OpenMenu(menu1, cindex, 20, 20, menu1_handler);
				break;

			case KEY_BACK:
				//msg( "DelMove()", 0);
				DelMove();
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

			case KEY_MUSIC:
				msg("KEY_MUSIC", 0, 0);
				break;

			case KEY_MENU:
				WriteState();
				msg("State saved", 0, 0);
//				sleep(1);
				CloseApp();
				break;

			case KEY_DELETE:
				//msg("NewGame()");
				if( par2==1)
					StartNewGame();
				else if( !par2)
					AddMove( 1);
				break;

		}
	}

	if (type == EVT_EXIT) {
		// occurs only in main handler when exiting or when SIGINT received.
		// save configuration here, if needed
	}

	return 0;
}

int main(int argc, char **argv)
{
//fprintf(stderr, "%lld\n", 0x7fffffffffffffffLL);

	InkViewMain(main_handler);
	return 0;
}


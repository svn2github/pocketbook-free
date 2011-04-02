#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <dlfcn.h>
#include <math.h>
#include <sys\timeb.h>
#include "inkview.h"

#define SCRW 600
#define SCRH 800

typedef struct{
  short width;
  short height;
  unsigned char data[];
} trubitmap;

extern const ibitmap weapon01, weapon02, weapon03, weapon04, weapon05, weapon06, weapon07, weapon08;
extern const ibitmap weapon09, weapon10, weapon11, weapon12, weapon13;
extern const ibitmap fighterleft, fighterright, windleft, windright, selectplayer;
extern const trubitmap tank1pic, tank2pic, tank3pic;

typedef struct {
	int cplayer;  // 0 - human player, 1 - computer player
	int ammo[13]; // we have [0..12] types of weapons, [x] - count of ammo, -1 - infinite
	int angle;    // angle of firing
	int power;    // power of firing
	int life;     // life of player
	int x,y;      // x,y of player
	int dead;     // dead (=1) or alive (=0) :)
	int money;    // money
	int wins;     // amount of wins
} infoplayer; // struct for changable variables of player, except names

int PLAYERS=2, DIFFICUILTY=1; // PLAYERS [2..8], DIFFICUILTY [1..3]
char *NPLAYERS [8];  //names of players
int WIND=0; //global wind variable
int CHOICE = 1; //global number of player to edit (edit_player_handler)
int ARENA [800]; // area of fighting. we have 800 y coordinates of peaks (y=0 in left up corner!!!)
short BOOLARENA [800][545];//representation of arena in 2-array, where =1 - pixel, =0 - empty ([0][0] in left down corner!!!)
// handler flags
int STARTFROMBEGINNING = 1; // global flag for beginning of new game (uses in game_handler). 1 - we are beginning from scratch, 0 - returning from handler to game_handler
int GAMESHOPBEGINNING = 1; // global flag for gameshop, ^^^^
int FLAGSHTOKGAME = 1; // handlers are very cool
int FLAGSHTOKSHOP = 1; // maybe they are ignoring return 0 ???
int FLAGSHTOKINGAMEMENU = 1; // more and more of them.. fuck
// options flags
int IMMEDIATETRAECTORY = 0; //immediate traectory drawing, 1 - yes, 0 - no
int IMMEDIATEDROP = 0; //immediate drop of tanks in the begin of round
int REFLECTWALLS = 1; //reflect from side walls

infoplayer IPLAYER[8];


// stuff functions
char itoc (int i);
int minint (int num1, int num2);
int maxint (int num1, int num2);
void itos (char *strn, int i);
int fround (float x);
int iabs (int num);
float flabs (float num);
void draw_trubitmap(int x, int y, trubitmap *pic);
unsigned long ret_4num (unsigned char byte);
// end of stuff functions

// initing functiuons
void init_NPLAYERS ();
void init_players_info ();
// end of initing functions

// gameshop functions
int shop_handler(int type, int par1, int par2);
void shop_init_draw ();
void shop_player_info (int playa, int update);
// end of gameshop functions

// gameplay functions
int game_handler(int type, int par1, int par2);
void show_arena();
void fill_arena();
void copy_ARENA_to_BOOLARENA ();
void copy_BOOLARENA_to_ARENA ();
void show_mainpanel();
void show_player_info(int playa);
void draw_angle (int ang);
void clear_info_panel();
void drop_tanks();
void draw_tank (int x, int y);
void show_tanks();
void invert_weapon_choice (int num);
void draw_tank_wbarrel (int num);
void redraw_tank_and_angle (int turn);
void show_partial_arena (int x, int w);
void show_parabollic_traectory (int *xret, int *yret, int playa, int update);
void calc_best_fire (int *retv, int *retalpha, int playa);
void return_coordinates (int *retx, int *rety, float x0, float y0, float v0, float alpha);
void explosion (int x, int y, int r);
void draw_circle (int x, int y, int r, int color);
void fall_landscape ();
void show_roll_traectory (int *retx, int *rety, int x, int y);
void main_fire (int playa, int weapon);
void explode_dead ();
void triple_traectory (int *x1ret, int *y1ret, int *x2ret, int *y2ret, int *x3ret, int *y3ret, int playa);
void walking (int *xret, int *yret, int playa);
void laser (int playa);
void show_winner(int playa);
int winner_handler(int type, int par1, int par2); // not working!
// end of gameplay functions

// menu and redactor functions
void main_draw (int xl, int yl, int hgt);
void init_menu_choice (int xl, int yl, int wdt, int hgt, int posmenu);
void init_mainmenu_choice (int xl, int yl, int wdt, int hgt, int posmenu, int *mainwd);
void menu_update (int xl, int yl, int wdt, int hgt, int posmenu, int posmenupr);
void mainmenu_update (int xl, int yl, int wdt, int hgt, int posmenu, int posmenupr);
int main_handler (int type, int par1, int par2);

int instruction_handler(int type, int par1, int par2);

int start_handler(int type, int par1, int par2);
void redraw_choice_menu (int xl, int yl, int wdt, int hgt, int posmenu, int *mainwd);
void redraw_players_menu (int xl, int yl, int wdt, int hgt, int posmenu);
int edit_player_handler (int type, int par1, int par2);
char return_key (int num);
void draw_keyboard (int xp, int yp);
void init_keyboard_kursor (int xp, int yp, int wkl, int hkl, int posx, int posy);
void update_keyboard_kursor (int xp, int yp, int wkl, int hkl, int posx, int posy, int posxpr, int posypr);
void redraw_player_name (int xp, int yp);

int options_handler (int type, int par1, int par2);
void show_options_init ();
// end of menu and redactor functions

//////////////////////////////////////////////////////////////////////
// STUFF FUNCTIONS
//////////////////////////////////////////////////////////////////////

//make from 1-byte color 3-bytes for DrawPixel, DrawLine etc.
unsigned long ret_4num (unsigned char byte) {
	return (byte*256*256+byte*256+byte);
}

//draws tru 16-color bitmap
void draw_trubitmap(int x, int y, trubitmap *pic) {
	int i,j;
	for (i=0; i<pic->height; i++) {
		for (j=0; j<pic->width; j++) {
			DrawPixel (x+j, y+(pic->height-i), ret_4num(pic->data[pic->width*i+j]));
		}
	}
}


// return char representation of [0..9] integer
char itoc (int i) {
	if (i == 0) return '0';
	if (i == 1) return '1';
	if (i == 2) return '2';
	if (i == 3) return '3';
	if (i == 4) return '4';
	if (i == 5) return '5';
	if (i == 6) return '6';
	if (i == 7) return '7';
	if (i == 8) return '8';
	if (i == 9) return '9';
	return 'F';
}

int minint (int num1, int num2) {
	if (num1 >= num2) return num2;
	if (num1 <= num2) return num1;
	return 0;
}

int maxint (int num1, int num2) {
	if (num1 >= num2) return num1;
	if (num1 <= num2) return num2;
	return 0;
}
int iabs (int num) {
	if (num > 0) return num;
	if (num < 0) return -num;
	return 0;
}

float flabs (float num) {
	if (num > 0) return num;
	if (num < 0) return -num;
	return 0;	
}

// float round function, returns integer
int fround (float x) {
	float r=0;
	r = (float)(x-(int)x);
	if (x > 0) {
		if (r < 0.5 ) return (int)x;
		if (r >= 0.5 ) return 1+(int)x;
	}
	if (x < 0){
		if (r <= -0.5 ) return -1+(int)x;
		if (r > -0.5 ) return (int)x;
	}
	return 0;
}

// fill strn with string representation of number [0..32764], ends with '\0'
void itos (char *strn, int i) {
	int wc,j,k,l;
//	wc=i;
	j=10;
	k=0;
	l=i;
	if (i < 10) k = 1;
	if (i > 9 && i < 100) k=2;
	if (i > 99 && i < 1000) k=3;
	if (i > 999 && i < 10000) k=4;
	if (i > 9999 && i < 32765 ) k=5;
	
//	while (wc > 0) {
//		wc = (int)i/j;
//		j=j*10;
//		k++;
//	}
	wc = i;
	for (j = k-1; j >= 0; j--) {
		strn[j] = itoc (wc%10);
		wc = (wc-wc%10)/10;
	}
	strn[k] = '\0';
}

//////////////////////////////////////////////////////////////////////
// ENF OF STUFF FUNCTIONS
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// GAMESHOP CODE
//////////////////////////////////////////////////////////////////////


//show info for playa with deleting previous pictures and partialupdate
// playa [1..PLAYERS]
// update: 1 - PartialUpdate, 0 - no
void shop_player_info (int playa, int update) {
	char strammo [10];
	char strmoney[10];
	int i;
	
	FillArea (451, 101, 58, 500, WHITE);
	FillArea (511, 101, 289, 500, WHITE);
	
	SetFont(OpenFont("LiberationSans", 24, 0), BLACK);
	for (i=0; i<13; i++) {
		strcpy (strammo, "");
		if (IPLAYER[playa-1].ammo[i] >= 0) itos (strammo, IPLAYER[playa-1].ammo[i]);
		if (IPLAYER[playa-1].ammo[i] == -1) strcpy (strammo, "...");
		DrawTextRect (451, i*35+107, 60, 33, strammo, ALIGN_CENTER);
	}
	
	strcpy (strmoney, "");
	itos (strmoney, IPLAYER[playa-1].money);
	strcat (strmoney, "$");
	
	SetFont(OpenFont("LiberationSans", 50, 0), BLACK);
	DrawTextRect (511, 400, 289, 200, strmoney, ALIGN_CENTER);
	
	SetFont(OpenFont("LiberationSans", 42, 0), BLACK);
	DrawTextRect (511, 150, 289, 200, NPLAYERS[playa-1], ALIGN_CENTER);
		
	if (update == 1) PartialUpdateBW (451, 101, 349, 500);
}

void shop_init_draw () {
	int widweapon=35, xo=30, yh=110;
	int prices[13] = {0, 10, 20, 30, 50, 70, 40, 70, 100, 150, 400, 300, 500};
	int i;
	char strprice[10];
	
	ClearScreen();
	DrawRect (0,0,800, 100, BLACK);
	DrawLine (450, 100, 450, 599, BLACK);
	DrawLine (510, 100, 510, 599, BLACK);
	
	SetFont(OpenFont("LiberationSans", 24, 0), BLACK);

	DrawBitmap (xo, widweapon*0+yh, &weapon01);
	DrawBitmap (xo, widweapon*1+yh, &weapon02);
	DrawBitmap (xo, widweapon*2+yh, &weapon03);
	DrawBitmap (xo, widweapon*3+yh, &weapon04);
	DrawBitmap (xo, widweapon*4+yh, &weapon05);
	DrawBitmap (xo, widweapon*5+yh, &weapon06);
	DrawBitmap (xo, widweapon*6+yh, &weapon07);
	DrawBitmap (xo, widweapon*7+yh, &weapon08);
	DrawBitmap (xo, widweapon*8+yh, &weapon09);
	DrawBitmap (xo, widweapon*9+yh, &weapon10);
	DrawBitmap (xo, widweapon*10+yh, &weapon11);
	DrawBitmap (xo, widweapon*11+yh, &weapon12);
	DrawBitmap (xo, widweapon*12+yh, &weapon13);
	
	DrawString (xo+50, 35*0+yh, "Gun");
	DrawString (xo+50, 35*1+yh, "Barrell");
	DrawString (xo+50, 35*2+yh, "Grenade");
	DrawString (xo+50, 35*3+yh, "Napalm");
	DrawString (xo+50, 35*4+yh, "Magnetoid");
	DrawString (xo+50, 35*5+yh, "Gigamagnetoid");
	DrawString (xo+50, 35*6+yh, "Plazmoseed");
	DrawString (xo+50, 35*7+yh, "Blazer");
	DrawString (xo+50, 35*8+yh, "Deadrain");
	DrawString (xo+50, 35*9+yh, "Pluk");
	DrawString (xo+50, 35*10+yh, "Nuke");
	DrawString (xo+50, 35*11+yh, "Tachion canon");
	DrawString (xo+50, 35*12+yh, "Last hope");
	DrawString (xo+50, 35*13+yh, "Done");
	
	for (i=0; i<13; i++) {
		strcpy(strprice, "");
		itos (strprice, prices[i]);
		DrawString (380, i*35+yh, strprice);
	}
	
}

int shop_handler(int type, int par1, int par2) {
	static int playa=1; // [1..PLAYERS]
	static int posmenu=2;
	int posmenutmp=2;
	int xmenu=10, ymenu=107, wmenu=430, hmenu=35;
	int prices[13] = {0, 10, 20, 30, 50, 70, 40, 70, 100, 150, 400, 300, 500};
	int dmoney;
	int compmenu;
	int i;
//	struct timeb t;
		
	if (type == EVT_SHOW && FLAGSHTOKSHOP == 1) {
		if (GAMESHOPBEGINNING == 1) {
			shop_init_draw();
			init_menu_choice (xmenu, ymenu, wmenu, hmenu, posmenu);
			FullUpdate();
			shop_player_info(playa, 1);
		}
		if (GAMESHOPBEGINNING == 0) {
			shop_player_info(playa, 1);
			init_menu_choice (xmenu, ymenu, wmenu, hmenu, posmenu);
		}
	}
	while (IPLAYER[playa-1].cplayer == 1 && FLAGSHTOKSHOP == 1) {
		ClearScreen();
		shop_init_draw();
		PartialUpdateBW (0,0,800,600);
		shop_player_info(playa, 1);
//		init_menu_choice (xmenu, ymenu, wmenu, hmenu, posmenu);
		while (IPLAYER[playa-1].money >= 10) {
			compmenu = rand()%10+2;
			dmoney = IPLAYER[playa-1].money - prices[compmenu-1];
			if (dmoney >= 0) {
				init_menu_choice (xmenu, ymenu, wmenu, hmenu, compmenu);
				IPLAYER[playa-1].money = dmoney;
				IPLAYER[playa-1].ammo[compmenu-1]++;
				if (IPLAYER[playa-1].ammo[compmenu-1] > 999) IPLAYER[playa-1].ammo[compmenu-1] = 999;
				shop_player_info(playa, 1);
				init_menu_choice (xmenu, ymenu, wmenu, hmenu, compmenu);
			}
		}
		playa++;
		if (playa > PLAYERS) {
			for (i=0; i<PLAYERS; i++) {
				IPLAYER[i].life = 100;
				IPLAYER[i].dead = 0;
			}
			STARTFROMBEGINNING = 1;
			GAMESHOPBEGINNING = 1;
			FLAGSHTOKGAME = 1;
			FLAGSHTOKSHOP = 0;
			playa = 1;
			posmenu = 2;
			fill_arena();
			SetEventHandler (game_handler);
			return 0;
		}
		if (IPLAYER[playa-1].cplayer == 0) {
			posmenu = 2;
			shop_player_info(playa, 1);
			//init_menu_choice (xmenu, ymenu, wmenu, hmenu, posmenu);			
		}
	}
	
	if (type == EVT_KEYPRESS) {
		if (par1 == KEY_UP) {
			posmenutmp = posmenu;
			posmenu--;
			if (posmenu < 2) posmenu = 14;
			menu_update (xmenu, ymenu, wmenu, hmenu, posmenu, posmenutmp);
		}
		if (par1 == KEY_DOWN) {
			posmenutmp = posmenu;
			posmenu++;
			if (posmenu > 14) posmenu = 2;
			menu_update (xmenu, ymenu, wmenu, hmenu, posmenu, posmenutmp);
		}
		if (par1 == KEY_OK) {
			if (posmenu >= 2 && posmenu <= 13) {
				dmoney = IPLAYER[playa-1].money - prices[posmenu-1];
				if (dmoney >= 0) {
					IPLAYER[playa-1].money = dmoney;
					IPLAYER[playa-1].ammo[posmenu-1]++;
					if (IPLAYER[playa-1].ammo[posmenu-1] > 999) IPLAYER[playa-1].ammo[posmenu-1] = 999;
					shop_player_info(playa, 1);
				}
			}
			if (posmenu == 14) {
				init_menu_choice (xmenu, ymenu, wmenu, hmenu, posmenu);
				posmenu = 2;
				playa++;
				if (playa > PLAYERS) {
					for (i=0; i<PLAYERS; i++) {
						IPLAYER[i].life = 100;
						IPLAYER[i].dead = 0;
					}
					STARTFROMBEGINNING = 1;
					GAMESHOPBEGINNING = 1;
					FLAGSHTOKGAME = 1;
					FLAGSHTOKSHOP = 0;
					playa = 1;
					posmenu = 2;
					fill_arena();
					SetEventHandler (game_handler);
					return 0;
				}
				if (playa <= PLAYERS) {
					GAMESHOPBEGINNING = 0;
					SetEventHandler (shop_handler);
					return 0;
				}
			}
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////
// ENF OF GAMESHOP CODE
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// GAMEPLAY CODE
//////////////////////////////////////////////////////////////////////

// filling ARENA with landscape
void fill_arena () {
	float t;
	float koef1, koef2, koef3, koef4, koef5, koef6, koef7;
	int i;
	koef1 = (float)(rand()%250+50);
	koef2 = (float)(rand()%70+200);
	koef3 = (float)(rand()%250+50);
	koef4 = (float)(rand()%250+50);
	koef5 = (float)(rand()%250+50);
	koef6 = (float)(rand()%250+50);
	koef7 = (float)(rand()%250+50);
	for (i=0; i<800; i++) {
		t = 325+koef2*sin((float)i/koef1)*cos((float)i/koef3)*cos((float)i/koef4)*sin((float)i/koef5);
		ARENA[i] = (int)t;
	}
}

// copy ARENA to BOOLARENA
void copy_ARENA_to_BOOLARENA () {
	int i,j;
	for (i=0; i<800; i++) {
		for (j=0; j<545; j++) {
			if (j <= (599-ARENA[i])) BOOLARENA[i][j] = 1;
			if (j > (599-ARENA[i])) BOOLARENA[i][j] = 0;
		}
	}
}

// copy BOOLARENA to ARENA. uses only from inside fall_landscape()
void copy_BOOLARENA_to_ARENA () {
	int i,flag=1,j=0;
	
	for (i=0; i<800; i++) {
		flag = 1;
		j = 0;
		while (flag == 1) {
			j++;
			if (BOOLARENA[i][j] == 0 || j == 545) flag = 0;
		}
		ARENA[i]=599-(j-1);
	}
}

// show ARENA on screen
void show_arena() {
	int i=0;
	for (i=0; i<800; i++) {
		DrawLine(i, ARENA[i], i, 599, BLACK);
	}
}

// show ARENA around x coordinate, w - width of showing
void show_partial_arena (int x, int w) {
	int i;
	for (i=x-w; i<=x+w; i++) {
		if (i >= 0 && i <= 799) DrawLine (i, ARENA[i], i, 600, BLACK);
	}
}

// draw line with ang (in degrees!) angle in angle box in info panel
void draw_angle (int ang) {
	float truang, tempang=0;
	float a=0, b=0;
	float w=40;
	float c=20, alpha = M_PI/30; //lenght and angle of pointer
	float cycdraw=0;
	float x1str=0, y1str=0;
	int x1=0, y1=0, x2=0, y2=0;
	float coss=0, sinn=0;
	
	truang = (float)ang*M_PI/180;
	if (truang > 0 && truang < M_PI/4) {
		a = w/2;
		b = a*tan(truang);
		x1 = 460;
		y1 = 5+(int)(w/2+b);
		x2 = 500;
		y2 = 5+(int)(w/2-b);
	}
	if (truang > M_PI/4 && truang < M_PI/2) {
		b = w/2;
		a = b/tan(truang);
		x1 = 460+(int)(w/2-a);
		y1 = 45;
		x2 = 460+(int)(w/2+a);
		y2 = 5;
	}
	if (truang > M_PI/2 && truang < 3*M_PI/4) {
		tempang = truang - M_PI/2;
		a = w/2;
		b = a*tan(tempang);
		x1 = 460+(int)(w/2+b);
		y1 = 45;
		x2 = 460+(int)(w/2-b);
		y2 = 5;
	}
	if (truang > 3*M_PI/4 && truang < M_PI) {
		tempang = truang - M_PI/2;
		b = w/2;
		a = b/tan(tempang);
		x1 = 500;
		y1 = 5+(int)(w/2+a);
		x2 = 460;
		y2 = 5+(int)(w/2-a);
	}
	if (ang == 0)  { x1=460;          y1=5+(int)w/2; x2=500;          y2=5+(int)w/2; }
	if (ang == 45) { x1=460;          y1=5+(int)w;   x2=500;          y2=5; }
	if (ang == 90) { x1=460+(int)w/2; y1=45;         x2=460+(int)w/2; y2=5; }
	if (ang == 135){ x1=460+(int)w;   y1=5+(int)w;   x2=460;          y2=5; }
	if (ang == 180){ x1=500;          y1=5+(int)w/2; x2=460;          y2=5+(int)w/2; }
	
	for (cycdraw=-alpha; cycdraw <= 1.5*alpha; cycdraw=cycdraw+M_PI/180) {
		y1str = (float)(y2+c*sin(truang+cycdraw));
		x1str = (float)(x2-c*cos(truang+cycdraw));
		DrawLine (x2, y2, fround(x1str), fround(y1str), BLACK);
	}
	
	coss = cos(truang);
	sinn = sin(truang);
	for (cycdraw=-1.4; cycdraw <= 1.4; cycdraw=cycdraw+0.2) {
		DrawLine (x1+fround(sinn*cycdraw), y1+fround(coss*cycdraw), x2, y2, BLACK);
	}
	DrawLine (x1, y1, x2, y2, BLACK);
}

// clear panel before putting new info
void clear_info_panel() {
	int i;
	for (i=0; i<13; i++) FillArea(i*35+1, 1, 33, 14, WHITE);
	FillArea (456, 1, 48, 49, WHITE);
	FillArea (507, 2, 46, 28, WHITE);
	FillArea (557, 2, 46, 28, WHITE);
	FillArea (606, 1, 38, 14, WHITE);
	FillArea (606, 16, 38, 14, WHITE);
	FillArea (646, 31, 72, 18, WHITE);
	FillArea (721, 31, 78, 18, WHITE);
//	FillArea (646, 1, 152, 28, WHITE);
	FillArea (646, 1, 48, 28, WHITE);
	FillArea (696, 1, 102, 28, WHITE);
	
}

// shows all player's info (dont forget to call clear_info_panel() before that!). № of player [0..7], NOT [1..8]
void show_player_info(int playa) {
	int i;
	char strammo[8];
	char strangpower[8];
	char strwind[8];
	char strlife[12];
	char tempstrlife[12];
	char strmoney[6];
	
	SetFont(OpenFont("LiberationSans", 14, 0), BLACK);
	for (i=0; i<13; i++) {
//		DrawLine (i*35, 0, i*35, 50, BLACK);
		strcpy (strammo, "");
		if (IPLAYER[playa].ammo[i] >= 0) itos (strammo, IPLAYER[playa].ammo[i]);
		if (IPLAYER[playa].ammo[i] == -1) strcpy (strammo, "...");
		DrawTextRect (i*35+1, 0, 33, 15, strammo, ALIGN_CENTER);
	}
	SetFont(OpenFont("LiberationSans", 24, 0), BLACK);
	
	strcpy (strangpower, "");
	itos (strangpower, IPLAYER[playa].angle);
	DrawTextRect (507, 2, 46, 28, strangpower, ALIGN_CENTER);
	
	strcpy (strangpower, "");
	itos (strangpower, IPLAYER[playa].power);
	DrawTextRect (557, 2, 46, 28, strangpower, ALIGN_CENTER);
	
	draw_angle (IPLAYER[playa].angle);
	
	SetFont(OpenFont("LiberationSans", 14, 0), BLACK);
	strcpy (strwind, "");
	itos (strwind, iabs(WIND));
	DrawTextRect (606, 0, 38, 14, strwind, ALIGN_CENTER);
	if (WIND > 0) DrawBitmap (607, 19, &windright);
	if (WIND < 0) DrawBitmap (607, 19, &windleft);
	
	SetFont(OpenFont("LiberationSans", 16, 0), BLACK);
	strcpy (strlife, "Men: ");
	strcpy (tempstrlife, "");
	itos (tempstrlife, IPLAYER[playa].life);
	strcat (strlife, tempstrlife);
	DrawTextRect (646, 31, 74, 20, strlife, ALIGN_CENTER);
	
	FillArea (723, 33, fround((float)(0.74*IPLAYER[playa].life)), 14, BLACK); //Lifebar
	
	strcpy(strmoney, "");
	itos (strmoney, IPLAYER[playa].money);
	strcat(strmoney, "$");
	SetFont(OpenFont("LiberationSans", 18, 0), BLACK);
	DrawTextRect (647, 4, 47, 24, strmoney, ALIGN_CENTER);
	
	SetFont(OpenFont("LiberationSans", 24, 0), BLACK);
	DrawTextRect (696, 0, 104, 28, NPLAYERS[playa], ALIGN_CENTER);
	
}

// shows grid and sprites of main panel, WITHOUT player's info
void show_mainpanel() {
	int i;
	int widweapon=35, xo=3, yh=18; // values for weapon pictures to place them correctly in main panel grid
	
	SetFont(OpenFont("LiberationSans", 16, 0), BLACK);
	DrawRect(0,0, SCRH, 51, BLACK);
	DrawLine (0, 15, 455, 15, BLACK);
	for (i=0; i<14; i++) {
		DrawLine (i*widweapon, 0, i*widweapon, 50, BLACK);
	}
	
	DrawLine (505, 0, 505, 50, BLACK);
	DrawLine (555, 0, 555, 50, BLACK);
	DrawLine (505, 30, 555, 30, BLACK);
	
	DrawTextRect (505, 30, 50, 20, "Angle", ALIGN_CENTER);
	
	DrawLine (605, 0, 605, 50, BLACK);
	DrawLine (555, 30, 605, 30, BLACK);
	
	DrawTextRect (555, 30, 50, 20, "Power", ALIGN_CENTER);
	
	DrawLine (645, 0, 645, 50, BLACK);
	DrawLine (605, 30, 800, 30, BLACK);
	DrawLine (605, 15, 645, 15, BLACK);
	DrawLine (720, 30, 720, 50, BLACK);
	
	DrawLine (695, 0, 695, 30, BLACK);
	
	DrawTextRect (606, 30, 40, 20, "Wind", ALIGN_CENTER);
			
	DrawBitmap (widweapon*0+xo, yh, &weapon01);
	DrawBitmap (widweapon*1+xo, yh, &weapon02);
	DrawBitmap (widweapon*2+xo, yh, &weapon03);
	DrawBitmap (widweapon*3+xo, yh, &weapon04);
	DrawBitmap (widweapon*4+xo, yh, &weapon05);
	DrawBitmap (widweapon*5+xo, yh, &weapon06);
	DrawBitmap (widweapon*6+xo, yh, &weapon07);
	DrawBitmap (widweapon*7+xo, yh, &weapon08);
	DrawBitmap (widweapon*8+xo, yh, &weapon09);
	DrawBitmap (widweapon*9+xo, yh, &weapon10);
	DrawBitmap (widweapon*10+xo, yh, &weapon11);
	DrawBitmap (widweapon*11+xo, yh, &weapon12);
	DrawBitmap (widweapon*12+xo, yh, &weapon13);
	
}

// draws tank
void draw_tank (int x, int y) {
	FillArea (x-10, y-5, 21, 5, BLACK);
	FillArea (x-7, y-8, 15, 3, BLACK);
	FillArea (x-3, y-10, 7, 2, BLACK);
	DrawLine (x-2, y-11, x+2, y-11, BLACK);
	DrawPixel (x-4, y-5, WHITE);
	DrawPixel (x+4, y-5, WHITE);
}

// shows all tanks except dead
void show_tanks () {
	int i;
	for (i=0; i<PLAYERS; i++) if (IPLAYER[i].life > 0) draw_tank (IPLAYER[i].x, IPLAYER[i].y);
}

// drop tanks on arena
void drop_tanks () {
	int numofplayer=1;
	int yofplayer=60;
	int xofplayer;
	int i,j;
	int temp=0;
	struct timespec t;
	struct timespec tret;
	int maxarena=0;

	t.tv_sec = 0;
	for (numofplayer=1; numofplayer <= PLAYERS; numofplayer++) {
		xofplayer = 30+(numofplayer-1)*740/(PLAYERS-1);
		temp = (ARENA[xofplayer]-60)/5;
		for (j=0; j<6; j++) {
			yofplayer = 63+j*temp;
			if (j > 0) FillArea (xofplayer-10, yofplayer-11-temp, 21, 11, WHITE);
			draw_tank (xofplayer, yofplayer);
			PartialUpdateBW (xofplayer-10, yofplayer-11-temp, 21, 11+temp);
			t.tv_nsec = 100000000;
			nanosleep(&t, &tret);
		}
		for (j=xofplayer-6; j<=xofplayer+6; j++) if (maxarena <= ARENA[j]) maxarena = ARENA[j];
//		yofplayer = ARENA[xofplayer];
		yofplayer = maxarena;
		maxarena = 0;
		IPLAYER[numofplayer-1].x = xofplayer;
		IPLAYER[numofplayer-1].y = yofplayer;
		for (i=xofplayer-10; i <= xofplayer+10; i++) {
			if (ARENA[i] < yofplayer) ARENA[i] = yofplayer;
		}
		FillArea (xofplayer-10, 55, 21, yofplayer-55, WHITE);
		draw_tank (xofplayer, yofplayer);
		PartialUpdateBW (xofplayer-10, 55, 21, 1+yofplayer-55);
	}
	FillArea (0, 55, 800, 545, WHITE);
	//debug
	show_arena();
	show_tanks();
	draw_tank_wbarrel(1);
	PartialUpdateBW (0, 55, 800, 545);
}

// inverting weapon in main panel, num [1..13]
void invert_weapon_choice (int num) {
	InvertAreaBW (2+35*(num-1), 17, 32, 32);
}

// draw tank with barrel, num [1..PLAYER]
void draw_tank_wbarrel (int num) {
	float truang;
	int xb1, yb1;
	int xb2, yb2;
	int l = 15;//lenght of barrel
	float c = 1;// width of barrel
	
	truang = (float)(IPLAYER[num-1].angle*M_PI/180);
	xb1 = IPLAYER[num-1].x;
	yb1 = IPLAYER[num-1].y-6;
	FillArea (IPLAYER[num-1].x-l, IPLAYER[num-1].y-l-6, 2*l+1, l+6, WHITE);
	draw_tank (IPLAYER[num-1].x, IPLAYER[num-1].y);
	xb2 = xb1 + fround((float)(l*cos(truang)));
	yb2 = yb1 - fround((float)(l*sin(truang)));
	DrawLine (xb1, yb1, xb2, yb2, BLACK);
	DrawLine (xb1-fround(sin(truang)*c), yb1-fround(cos(truang)*c), xb2-fround(sin(truang)*c), yb2-fround(cos(truang)*c), BLACK);
	DrawLine (xb1+fround(sin(truang)*c), yb1+fround(cos(truang)*c), xb2+fround(sin(truang)*c), yb2+fround(cos(truang)*c), BLACK);	
	show_partial_arena (IPLAYER[num-1].x, l);
}

// redraws tank and angle information after EVT_KEYPRESS == LEFT(RIGHT) in phase 2 in game_handler
// turn - number of player [1..PLAYERS]
void redraw_tank_and_angle (int turn) {
	int xupd=0, yupd=0, wupd=0, hupd=0; // values for partial update
	
	clear_info_panel();
	show_player_info(turn-1);
	draw_tank_wbarrel (turn);
	xupd = minint (IPLAYER[turn-1].x-15, 456);
	yupd = 2;
	if (456-(IPLAYER[turn-1].x-15) > 0) wupd = 100+456-(IPLAYER[turn-1].x-15);
	if (456-(IPLAYER[turn-1].x-15) <= 0) wupd = iabs (456-(IPLAYER[turn-1].x+15));
	if (wupd < 100) wupd = 100;
	hupd = IPLAYER[turn-1].y;
	PartialUpdateBW (xupd, yupd, wupd, hupd);
}

// returns final coordinates after firing from x0, y0 with speed=v0 and angle=alpha
void return_coordinates (int *retx, int *rety, float x0, float y0, float v0, float alpha) {
	float t=0;
	float xt, yt;
	float w, g=9.8;
	float coss, sinn;
	float vch;// speed at time of impact the wall
	float dt=0.1;
	int flag = 1;
	
	w = -(float)(WIND/50);	
	coss = cos(alpha);
	sinn = sin(alpha);
	while (flabs(w*dt*dt/2-v0*dt*coss) + flabs(v0*dt*sinn-g*dt*dt/2) > 3) dt = dt/2;
	while ( flag == 1 ) {
		xt = x0 + v0*t*coss - w*t*t/2;
		yt = y0 - v0*t*sinn + g*t*t/2;
		if (xt > 799 || xt < 0) {
			vch = sqrt((v0*coss-w*t)*(v0*coss-w*t) + (g*t-v0*sinn)*(g*t-v0*sinn));
			sinn = -(g*t-v0*sinn)/vch;
			coss = -(v0*coss-w*t)/vch;
			x0 = xt;
			y0 = yt;
			v0 = vch;
			t = 0;
			xt = x0 + v0*t*coss - w*t*t/2;
			yt = y0 - v0*t*sinn + g*t*t/2;
		}
		if (xt >= 0 && xt <=799 && t > 0) {
			if (fround(yt) >= ARENA[fround(xt)]) { *retx = fround(xt); *rety = fround(yt); flag=0; }
		}
		t = t+dt;
	}
}

// calculates best angle and power for playa's tank to shoot random tank
// playa [1..PLAYERS]
void calc_best_fire (int *retv, int *retalpha, int playa) {
	int ang, pow;
	int xtemp, ytemp;
	int vtemp, atemp;
	int rd; //temp variable for random choice
	int cnt=0;
	int i;
	int dpow, dang;
	int *rv;
	int *ra;
	// 1 - 
	// 2 - 
	// 3 - dpow = 50-((d-1)*10);
	rv = (int*)malloc(sizeof(int));
	ra = (int*)malloc(sizeof(int));
	dpow = 50-((DIFFICUILTY-1)*10);
	dang = 25-((DIFFICUILTY-1)*5);
	for (ang=0; ang <=180; ang=ang+dang) {
		for (pow=10; pow <= 990; pow=pow+dpow) {
			return_coordinates (&xtemp, &ytemp, (float)IPLAYER[playa-1].x, (float)IPLAYER[playa-1].y-6, (float)(pow*0.18), (float)(ang*M_PI/180));
			for (i=0; i<PLAYERS; i++) {
				if (i != playa-1 && IPLAYER[i].life > 0) {
					if (xtemp >= IPLAYER[i].x-16-(3-DIFFICUILTY)*20 && xtemp <= IPLAYER[i].x+16+(3-DIFFICUILTY)*20 && ytemp <= IPLAYER[i].y+5+(3-DIFFICUILTY)*20 && ytemp >= IPLAYER[i].y-15-(3-DIFFICUILTY)*20 ) {
						cnt++;
						rv = (int*)realloc(rv, cnt*sizeof(int));
						ra = (int*)realloc(ra, cnt*sizeof(int));
						rv[cnt-1] = pow;
						ra[cnt-1] = ang;
						
					}
				}
			}
		}
	}
	if (cnt > 0) {
		rd = rand()%cnt;
		vtemp = rv[rd];
		atemp = ra[rd];
		*retv = vtemp;
		*retalpha = atemp;
	}
	if (cnt == 0) {
		*retv = 800;
		*retalpha = 90;
	}
	free (rv);
	rv = NULL;
	free (ra);
	ra = NULL;
}

// draws circle in x,y with radius r and color
void draw_circle (int x, int y, int r, int color) {
	float xtemp;
	float l;
	if (r > 0) {
		for (l=0; l<r; l=l+1) {
			xtemp = (float)(r*sqrt(1-l*l/(r*r)));
//		fprintf(stderr, "x1=%i y1=%i x2=%i y2=%i xtemp=%f\n", x-fround(xtemp), y-(int)l, x+fround(xtemp), y-(int)l, xtemp);
			if (y-(int)l >= 55)  DrawLine (x-fround(xtemp), y-(int)l, x+fround(xtemp), y-(int)l, color);
			if (y+(int)l <= 599) DrawLine (x-fround(xtemp), y+(int)l, x+fround(xtemp), y+(int)l, color);
		}
	}
}

// here we falling landscape USING BOOLARENA
void fall_landscape () {
	int i,j;
	int flag = 1;
	int counter=1;
	int xmin=799, xmax=0; //for partial update and x-cycle
	int maxarena=0;

	for (j=0; j <=799; j++) {
		for (i = 1; i < 545; i++) {
			if (BOOLARENA[j][i-1] == 0 && BOOLARENA[j][i] == 1) {
				xmin = minint (j, xmin);
				xmax = maxint (j, xmax);
				break;
			}
		}
	}	
	while (flag == 1) {
		for (j=xmin; j <=xmax; j++) {
			for (i = 1; i < 545; i++) {
				if (BOOLARENA[j][i-1] == 0 && BOOLARENA[j][i] == 1) {
					BOOLARENA[j][i] = 0;
					BOOLARENA[j][i-1] = 1;
					DrawPixel (j, 599-i, WHITE);
					DrawPixel (j, 599-(i-1), BLACK);
				}
			}
		}
		counter++;
		flag = 0;
		for (j=xmin; j <=xmax; j++) {
			for (i=1; i<545; i++) {
				if (BOOLARENA[j][i-1] == 0 && BOOLARENA[j][i] == 1) flag = 1;
			}
		}
		if (counter%20 == 0) PartialUpdateBW (xmin, 55, iabs(xmax-xmin), 545);
	}
	PartialUpdateBW (xmin, 55, iabs(xmax-xmin), 545);
	copy_BOOLARENA_to_ARENA();
	for (i = 0; i < PLAYERS; i++) {
		if (IPLAYER[i].dead != 1) {
			for (j=IPLAYER[i].x-6; j<=IPLAYER[i].x+6; j++) if (maxarena < ARENA[j]) maxarena = ARENA[j];
//			if (IPLAYER[i].y < ARENA[IPLAYER[i].x]) {
			if (IPLAYER[i].y < maxarena) {
				IPLAYER[i].y = maxarena;
				FillArea (IPLAYER[i].x-10, 55, 21, IPLAYER[i].y-55, WHITE);
				for (j = IPLAYER[i].x-10; j <= IPLAYER[i].x+10; j++) if (ARENA[j] < IPLAYER[i].y) ARENA[j] = IPLAYER[i].y;
				draw_tank (IPLAYER[i].x, IPLAYER[i].y);
			}
			maxarena = 0;
		}
	}
	show_tanks();
	PartialUpdateBW (0, 55, 800, 545);
}

// shows explosion of weapon r [1..6,9,10,11,13] in x,y
// and change BOOLARENA
void explosion (int x, int y, int r) {
	int i,j,dr=1;
	int radexplosion=10;
	float xtemp;
//	struct timeb t;
	struct timespec t;
	struct timespec tret;
	float rd,l;
	float k = 0.6; //koeff of deadliness
	float minl; //minus lives
	char strminl [10];
	
	copy_ARENA_to_BOOLARENA();
	t.tv_sec = 0;
	if (r == 1)  { radexplosion = 10;  dr = 1; }
	if (r == 2 || r == 5 || r == 9)  { radexplosion = 20; dr = 1; }
	if (r == 3 || r == 6 || r == 10)  { radexplosion = 30; dr = 2; }
	if (r == 4 || r == 13)  { radexplosion = 40; dr = 2; }
	if (r == 11) { radexplosion = 180;dr = 3; }
	
	for (j = radexplosion/dr; j <= radexplosion; j=j+(radexplosion/dr)) {
		draw_circle (x, y, j, BLACK);
		PartialUpdateBW (x-j, y-j, 2*j, 2*j);
		t.tv_nsec = 100000000; //nanoseconds, lol ))))))
		nanosleep(&t, &tret);
	}
	draw_circle(x, y, radexplosion, WHITE);
	PartialUpdateBW (x-radexplosion, y-radexplosion, radexplosion*2, radexplosion*2);
	// recalc lives
	for (i = 0; i < PLAYERS; i++) {
		strcpy (strminl, "");
		if (IPLAYER[i].life > 0) {
			rd = (float)1.5*radexplosion;
			l = (float)sqrt((IPLAYER[i].x-x)*(IPLAYER[i].x-x) + (IPLAYER[i].y-y)*(IPLAYER[i].y-y));
			minl = 0;
			if (l <= rd) minl = (float)(20+rd*k)*(1-l/rd);
			IPLAYER[i].life = IPLAYER[i].life - fround(minl);
			if (IPLAYER[i].life <= 0) IPLAYER[i].life = 0;
			if (fround(minl) > 0) {
				itos (strminl, fround(minl));
				
				SetFont(OpenFont("LiberationSans", 20, 0), BLACK);
				DrawString (IPLAYER[i].x-10, IPLAYER[i].y-40, strminl);
				PartialUpdateBW (IPLAYER[i].x-10, IPLAYER[i].y-40, 50, 30);
				t.tv_nsec = 300000000;
				nanosleep(&t, &tret);
				SetFont(OpenFont("LiberationSans", 20, 0), WHITE);
				DrawString (IPLAYER[i].x-10, IPLAYER[i].y-40, strminl);
				PartialUpdateBW (IPLAYER[i].x-10, IPLAYER[i].y-40, 50, 30);				
			}
		}
	}
	//change BOOLARENA
	for (l=0; l<radexplosion; l=l+1) {
		xtemp = (float)(radexplosion*sqrt(1-l*l/(radexplosion*radexplosion)));
		for (i=x-fround(xtemp); i <= x+fround(xtemp); i++) {
			if (i >= 0 && i <= 799) {
				if (y-(int)l >= 55) BOOLARENA[i][599-(y-(int)l)]=0;
				if (y+(int)l <= 599) BOOLARENA[i][599-(y+(int)l)]=0;
			}
		}
	}
}

// shows traectory for weapons 5,6 AFTER impact to landscape
// returns final retx, rety
void show_roll_traectory (int *retx, int *rety, int x, int y) {
	int dx;
	int i;
	int flag=1;
	int rdir=-1, ldir=-1; //direction on the right and left. 1 - going up, 0 - going down, -1 - ERROR
	int direction=0; // final direction of rolling; 1 - right, -1 - left
	int counter = 1; //counter for partial update
	
	if (y >= 599) y = 599;
	
	for (i = x; i <= 799; i++) {
		if (ARENA[i] > ARENA[x]) { rdir = 0; break; }
		if (ARENA[i] < ARENA[x]) { rdir = 1; break; }
	}
	for (i = x; i >= 0; i--) {
		if (ARENA[i] > ARENA[x]) { ldir = 0; break; }
		if (ARENA[i] < ARENA[x]) { ldir = 1; break; }
	}
	if (rdir == ldir) direction = rand()%2;
	if (rdir == 0 && ldir == 1) direction = 1;
	if (rdir == 1 && ldir == 0) direction = -1;
	if (direction == 0) direction = -1;
	dx = x;
	
	while (flag == 1) {
		dx = dx + direction;
		if ( dx > 0 && dx < 799) {
			if (dx - direction != x) DrawLine (dx+2*direction, ARENA[dx]-2, dx-direction+2*direction, ARENA[dx-direction]-2, BLACK);
			if (dx - direction == x) DrawLine (dx-direction, ARENA[dx-direction], dx+2*direction, ARENA[dx]-2, BLACK);
			if (ARENA[dx] < ARENA[dx-direction]) {
				*retx = dx;
				*rety = ARENA[dx];
				flag = 0;
			}
			for (i = 0; i<PLAYERS; i++) {
				if (dx >= IPLAYER[i].x-10 && dx <= IPLAYER[i].x+10 && ARENA[dx] >= IPLAYER[i].y-10 && ARENA[dx] <= IPLAYER[i].y+1 && IPLAYER[i].life > 0) {
					*retx = dx;
					*rety = ARENA[dx];
					flag = 0;					
				}
			}
		}
		if (dx == 0 || dx == 799) {
			*retx = dx;
			*rety = ARENA[dx];
			flag = 0;			
		}
		if (counter%15 == 0) PartialUpdateBW (0, 55, 800, 545);
	}
	PartialUpdateBW (0, 55, 800, 545);
}

// function for showing playa traectory of fire, returns xret,yret of final pixel
// playa - [1..PLAYERS]
// update: 1 - clear screen and redraw landscape + tanks = partialupdate, 0 - without clear and partialupdate
void show_parabollic_traectory (int *xret, int *yret, int playa, int update) {
	float t=0;
	float x0, y0, v0, alpha;
	float xt, yt;
	float w, g=9.8;
	float coss, sinn;
	float vch;// speed at time of impact the wall
	float dt=0.1;
	int flag = 1, i;
	float xcounter, ycounter, rcounter;  // difference between xy coords now and at the begin of counting. use for partial update
/* Experimental, details are further
//	float vxcounter, vycounter;
//	float xtx0, yty0; //x(t) and y(t), where dx/dt=0 or dy/dt=0 - for partial update
//	int xmaxupd, ymaxupd, xminupd, yminupd; // ^^^^
*/
	int maxARENA=0; //for partial update, not optimal, but less brainfucking, details are further
	
	
	
	x0 = (float)IPLAYER[playa-1].x;
	y0 = (float)IPLAYER[playa-1].y-6;
	v0 = (float)(IPLAYER[playa-1].power*0.18);
	alpha = (float)(IPLAYER[playa-1].angle*M_PI/180);
	w = -(float)(WIND/50);
	coss = cos(alpha);
	sinn = sin(alpha);

	for (i=0; i<800; i++) if (ARENA[i] > maxARENA) maxARENA = ARENA[i];
	xcounter = x0;
	ycounter = y0;
//	vxcounter = v0*coss; //experimental
//	vycounter = -v0*sinn;//experimental
	
	while (flabs(w*dt*dt/2-v0*dt*coss) + flabs (v0*dt*sinn-g*dt*dt/2) > 2) dt = dt/2;
	while ( flag == 1 ) {
		xt = x0 + v0*t*coss - w*t*t/2;
		yt = y0 - v0*t*sinn + g*t*t/2;
		if (xt > 799 || xt < 0) {
			vch = sqrt((v0*coss-w*t)*(v0*coss-w*t) + (g*t-v0*sinn)*(g*t-v0*sinn));
			sinn = -(g*t-v0*sinn)/vch;
			coss = -(v0*coss-w*t)/vch;
			x0 = xt;
			y0 = yt;
			v0 = vch;
			t = 0;
			xt = x0 + v0*t*coss - w*t*t/2;
			yt = y0 - v0*t*sinn + g*t*t/2;
//			vxcounter = v0*coss;//experimental
//			vycounter = -v0*sinn;//experimental
			PartialUpdateBW (0, 55, 800, maxARENA-55);
			xcounter = xt;
			ycounter = yt;			
		}
		if (xt >= 0 && xt <=799 && t > 0) {
			for (i=0; i<PLAYERS; i++) {
				if (fround(xt) >= IPLAYER[i].x-10 && fround(xt) <= IPLAYER[i].x+10 && fround(yt) >= IPLAYER[i].y-7 && fround(yt) <= IPLAYER[i].y && IPLAYER[i].life > 0 && i != playa-1) {
					flag = 0;
				}
			}
			if (fround(yt) >= ARENA[fround(xt)] || fround(yt) >= 599) flag=0;
		}
		if (yt <= 599 && yt >= 56 && xt >= 0 && xt <= 799) {
//			DrawLine (fround(xt), fround(yt), x0 + v0*(t+dt)*coss - w*(t+dt)*(t+dt)/2, y0 - v0*(t+dt)*sinn + g*(t+dt)*(t+dt)/2, BLACK);
			DrawRect (fround(xt)-1, fround(yt)-1, 2, 2, BLACK);
			rcounter = sqrt((xt-xcounter)*(xt-xcounter) + (yt-ycounter)*(yt-ycounter));
			if (rcounter > sqrt((v0*coss-w*t)*(v0*coss-w*t) + (g*t-v0*sinn)*(g*t-v0*sinn))) {
//experimental, working, but result is not faster on PocketBook301+, than PartialUpdateBW (0,55,800,max(ARENA[])-55), strange
/*				xmaxupd = maxint (fround(xcounter), fround(xt));
				xminupd = minint (fround(xcounter), fround(xt));
				ymaxupd = maxint (fround(ycounter), fround(yt));
				yminupd = minint (fround(ycounter), fround(yt));
				if ((vxcounter > 0 && (v0*coss-w*t)<0) || (vxcounter < 0 && (v0*coss-w*t)>0)) {
					xtx0 = x0+v0*v0*coss*coss/(2*w);
					xmaxupd = maxint (maxint(fround(xcounter), fround(xtx0)), fround(xt));
					xminupd = minint (minint(fround(xcounter), fround(xtx0)), fround(xt));
				}
				if ((vycounter > 0 && (g*t-v0*sinn)<0) || (vycounter < 0 && (g*t-v0*sinn)>0)) {
					yty0 = y0-v0*v0*sinn*sinn/(2*g);
					ymaxupd = maxint (maxint(fround(ycounter), fround(yty0)), fround(yt));
					yminupd = minint (minint(fround(ycounter), fround(yty0)), fround(yt));
				}

				PartialUpdateBW (xminupd, yminupd, xmaxupd-xminupd, ymaxupd-yminupd);
				vxcounter = v0*coss-w*t;
				vycounter = g*t-v0*sinn;
*/
				if (IMMEDIATETRAECTORY == 0) PartialUpdateBW (0, 55, 800, maxARENA-55);
				xcounter = xt;
				ycounter = yt;

			}
		}
		t = t+dt;
	}
	PartialUpdateBW (0, 55, 800, 545);
	if (update == 1) {
		FillArea (0, 55, 800, 545, WHITE);
		show_arena();
		show_tanks();
		PartialUpdateBW (0, 55, 800, 545);
	}
	*xret = fround(xt);
	*yret = fround(yt);	
}

// draws acid from x,y and recalculates landscape and lives
// weapon - 7,8
void acid (int x, int y, int weapon) {
	int dx, dy;
	int counter, counter2=1;
	int i;
	int x0, y0;
	int xt, yt;
	int j,k,l;
	
	copy_ARENA_to_BOOLARENA();
	counter = 800;
	x0 = x;
	y0 = ARENA[x];
	
	for (i=1; i<=counter; i++) {
		dx = (rand()%3-1)*(weapon-6)*2;
		dy = (rand()%3-1)*(weapon-6)*2;
		while (dx == dy && dx == 0) {
			dx = (rand()%3-1)*(weapon-6)*2;
			dy = (rand()%3-1)*(weapon-6)*2;
		}
		xt = x0 + dx;
		yt = y0 + dy;
		if (xt <= 799 && xt >= 0) {
			if (yt <= 599 && yt >= ARENA[xt]) {
				if (dx > 0 && dy > 0) {
					for (j = x0; j <= xt; j = j + 1) {
						for (k = y0; k <= yt; k = k + 1) {
							BOOLARENA[j][599-k] = 0;
							DrawPixel (j, k, WHITE);
							for (l=0; l<PLAYERS; l++) {
								if (j >= IPLAYER[l].x-10 && j <= IPLAYER[l].x+10 && k >= IPLAYER[l].y && k <= IPLAYER[l].y+10 && IPLAYER[l].life > 0) {
									IPLAYER[l].life--;
									if (IPLAYER[l].life <= 0) IPLAYER[l].life = 0;
								}
							}
						}
					}
				}
				if (dx > 0 && dy < 0) {
					for (j = x0; j <= xt; j = j + 1) {
						for (k = y0; k >= yt; k = k - 1) {
							BOOLARENA[j][599-k] = 0;
							DrawPixel (j, k, WHITE);
							for (l=0; l<PLAYERS; l++) {
								if (j >= IPLAYER[l].x-10 && j <= IPLAYER[l].x+10 && k >= IPLAYER[l].y && k <= IPLAYER[l].y+10 && IPLAYER[l].life > 0) {
									IPLAYER[l].life--;
									if (IPLAYER[l].life <= 0) IPLAYER[l].life = 0;
								}
							}
						}
					}
				}
				if (dx < 0 && dy > 0) {
					for (j = x0; j >= xt; j = j - 1) {
						for (k = y0; k <= yt; k = k + 1) {
							BOOLARENA[j][599-k] = 0;
							DrawPixel (j, k, WHITE);
							for (l=0; l<PLAYERS; l++) {
								if (j >= IPLAYER[l].x-10 && j <= IPLAYER[l].x+10 && k >= IPLAYER[l].y && k <= IPLAYER[l].y+10 && IPLAYER[l].life > 0) {
									IPLAYER[l].life--;
									if (IPLAYER[l].life <= 0) IPLAYER[l].life = 0;
								}
							}
						}
					}
				}
				if (dx < 0 && dy < 0) {
					for (j = x0; j >= xt; j = j - 1) {
						for (k = y0; k >= yt; k = k - 1) {
							BOOLARENA[j][599-k] = 0;
							DrawPixel (j, k, WHITE);
							for (l=0; l<PLAYERS; l++) {
								if (j >= IPLAYER[l].x-10 && j <= IPLAYER[l].x+10 && k >= IPLAYER[l].y && k <= IPLAYER[l].y+10 && IPLAYER[l].life > 0) {
									IPLAYER[l].life--;
									if (IPLAYER[l].life <= 0) IPLAYER[l].life = 0;
								}
							}
						}
					}
				}
				counter2++;
				x0 = xt;
				y0 = yt;
			}
		}
		if (counter2%20 == 0) PartialUpdateBW (0, 55, 800, 545);
	}
	PartialUpdateBW (0, 55, 800, 545);
}

// for weapons 9,10 - shows triple traectory and returns x.ret,y.ret - 3 pairs of coordinates
// playa [1..PLAYERS]
void triple_traectory (int *x1ret, int *y1ret, int *x2ret, int *y2ret, int *x3ret, int *y3ret, int playa) {
	float t1=0, t2=0, t3=0;
	float x01, y01, v01, alpha1;
	float x02, y02, v02, alpha2;
	float x03, y03, v03, alpha3;
	
	float xt1, yt1, xt2, yt2, xt3, yt3;
	float w, g=9.8;
	float coss1, sinn1, coss2, sinn2, coss3, sinn3;
	float vch1, vch2, vch3;// speed at time of impact the wall
	float dt1=0.1, dt2=0.1, dt3=0.1;
	int flag = 3;
	int flag1=1, flag2=1, flag3=1;
	int flagforupdate = 0; // 0 - not updating (for outscreen traectories)
	int i;
	float xcounter1, ycounter1, rcounter1=0; // difference between xy coords now and at the begin of counting. use for partial update
	float xcounter2, ycounter2, rcounter2=0;
	float xcounter3, ycounter3, rcounter3=0;
	
	
	x01 = (float)IPLAYER[playa-1].x;
	y01 = (float)IPLAYER[playa-1].y-6;
	v01 = (float)(IPLAYER[playa-1].power*0.18);
	
	x02 = (float)IPLAYER[playa-1].x;
	y02 = (float)IPLAYER[playa-1].y-6;
	v02 = (float)(IPLAYER[playa-1].power*0.18);
	
	x03 = (float)IPLAYER[playa-1].x;
	y03 = (float)IPLAYER[playa-1].y-6;
	v03 = (float)(IPLAYER[playa-1].power*0.18);
	
	alpha1 = (float)(IPLAYER[playa-1].angle*M_PI/180);
	alpha2 = (float)((IPLAYER[playa-1].angle+15)*M_PI/180);
	alpha3 = (float)((IPLAYER[playa-1].angle-15)*M_PI/180);
	
	w = -(float)(WIND/50);
	
	coss1 = cos(alpha1);
	sinn1 = sin(alpha1);
	
	coss2 = cos(alpha2);
	sinn2 = sin(alpha2);

	coss3 = cos(alpha3);
	sinn3 = sin(alpha3);	

	xcounter1 = x01;
	ycounter1 = y01;
	xcounter2 = x02;
	ycounter2 = y02;
	xcounter3 = x03;
	ycounter3 = y03;
	
	
	while (flabs(w*dt1*dt1/2-v01*dt1*coss1) + flabs(v01*dt1*sinn1-g*dt1*dt1/2) > 2) dt1 = dt1/2;
	while (flabs(w*dt2*dt2/2-v02*dt2*coss2) + flabs(v02*dt2*sinn2-g*dt2*dt2/2) > 2) dt2 = dt2/2;
	while (flabs(w*dt3*dt3/2-v03*dt3*coss3) + flabs(v03*dt3*sinn3-g*dt3*dt3/2) > 2) dt3 = dt3/2;
	
	while ( flag > 0 ) {
		xt1 = x01 + v01*t1*coss1 - w*t1*t1/2;
		yt1 = y01 - v01*t1*sinn1 + g*t1*t1/2;

		xt2 = x02 + v02*t2*coss2 - w*t2*t2/2;
		yt2 = y02 - v02*t2*sinn2 + g*t2*t2/2;

		xt3 = x03 + v03*t3*coss3 - w*t3*t3/2;
		yt3 = y03 - v03*t3*sinn3 + g*t3*t3/2;		
		
		if (xt1 > 799 || xt1 < 0) {
			vch1 = sqrt((v01*coss1-w*t1)*(v01*coss1-w*t1) + (g*t1-v01*sinn1)*(g*t1-v01*sinn1));
			sinn1 = -(g*t1-v01*sinn1)/vch1;
			coss1 = -(v01*coss1-w*t1)/vch1;
			x01 = xt1;
			y01 = yt1;
			v01 = vch1;
			t1 = 0;
			xt1 = x01 + v01*t1*coss1 - w*t1*t1/2;
			yt1 = y01 - v01*t1*sinn1 + g*t1*t1/2;
			PartialUpdateBW (0, 55, 800, 545);
			xcounter1 = xt1;
			ycounter1 = yt1;			
		}	
		if (xt2 > 799 || xt2 < 0) {
			vch2 = sqrt((v02*coss2-w*t2)*(v02*coss2-w*t2) + (g*t2-v02*sinn2)*(g*t2-v02*sinn2));
			sinn2 = -(g*t2-v02*sinn2)/vch2;
			coss2 = -(v02*coss2-w*t2)/vch2;
			x02 = xt2;
			y02 = yt2;
			v02 = vch2;
			t2 = 0;
			xt2 = x02 + v02*t2*coss2 - w*t2*t2/2;
			yt2 = y02 - v02*t2*sinn2 + g*t2*t2/2;
			PartialUpdateBW (0, 55, 800, 545);
			xcounter2 = xt2;
			ycounter2 = yt2;			
		}
		if (xt3 > 799 || xt3 < 0) {
			vch3 = sqrt((v03*coss3-w*t3)*(v03*coss3-w*t3) + (g*t3-v03*sinn3)*(g*t3-v03*sinn3));
			sinn3 = -(g*t3-v03*sinn3)/vch3;
			coss3 = -(v03*coss3-w*t3)/vch3;
			x03 = xt3;
			y03 = yt3;
			v03 = vch3;
			t3 = 0;
			xt3 = x03 + v03*t3*coss3 - w*t3*t3/2;
			yt3 = y03 - v03*t3*sinn3 + g*t3*t3/2;
			PartialUpdateBW (0, 55, 800, 545);
			xcounter3 = xt3;
			ycounter3 = yt3;			
		}

		if (xt1 >= 0 && xt1 <=799 && t1 > 0 && flag1 == 1) {
			for (i=0; i<PLAYERS; i++) {
				if (fround(xt1) >= IPLAYER[i].x-10 && fround(xt1) <= IPLAYER[i].x+10 && fround(yt1) >= IPLAYER[i].y-7 && fround(yt1) <= IPLAYER[i].y && IPLAYER[i].life > 0 && i != playa-1) {
					flag--;
					flag1 = 0;
				}
			}
			if ((fround(yt1) >= ARENA[fround(xt1)] || fround(yt1) >= 599) && flag1 == 1) { flag--; flag1 = 0; }
		}
		if (xt2 >= 0 && xt2 <=799 && t2 > 0 && flag2 == 1) {
			for (i=0; i<PLAYERS; i++) {
				if (fround(xt2) >= IPLAYER[i].x-10 && fround(xt2) <= IPLAYER[i].x+10 && fround(yt2) >= IPLAYER[i].y-7 && fround(yt2) <= IPLAYER[i].y && IPLAYER[i].life > 0 && i != playa-1) {
					flag--;
					flag2 = 0;
				}
			}
			if ((fround(yt2) >= ARENA[fround(xt2)] || fround(yt2) >= 599) && flag2 == 1) { flag--; flag2 = 0; }
		}
		if (xt3 >= 0 && xt3 <=799 && t3 > 0 && flag3 == 1) {
			for (i=0; i<PLAYERS; i++) {
				if (fround(xt3) >= IPLAYER[i].x-10 && fround(xt3) <= IPLAYER[i].x+10 && fround(yt3) >= IPLAYER[i].y-7 && fround(yt3) <= IPLAYER[i].y && IPLAYER[i].life > 0 && i != playa-1) {
					flag--;
					flag3 = 0;
				}
			}
			if ((fround(yt3) >= ARENA[fround(xt3)] || fround(yt3) >= 599) && flag3 == 1) { flag--; flag3 = 0; }
		}
		flagforupdate = 0;
		if (yt1 <= 599 && yt1 >= 56 && xt1 >= 0 && xt1 <= 799) {
//			DrawLine (fround(xt), fround(yt), x0 + v0*(t+dt)*coss - w*(t+dt)*(t+dt)/2, y0 - v0*(t+dt)*sinn + g*(t+dt)*(t+dt)/2, BLACK);
			DrawRect (fround(xt1)-1, fround(yt1)-1, 2, 2, BLACK);
			rcounter1 = sqrt((xt1-xcounter1)*(xt1-xcounter1) + (yt1-ycounter1)*(yt1-ycounter1));
			if (rcounter1 > sqrt((v01*coss1-w*t1)*(v01*coss1-w*t1) + (g*t1-v01*sinn1)*(g*t1-v01*sinn1))) flagforupdate = 1;
		}
		if (yt2 <= 599 && yt2 >= 56 && xt2 >= 0 && xt2 <= 799) {
			DrawRect (fround(xt2)-1, fround(yt2)-1, 2, 2, BLACK);
			rcounter2 = sqrt((xt2-xcounter2)*(xt2-xcounter2) + (yt2-ycounter2)*(yt2-ycounter2));
			if (rcounter2 > sqrt((v02*coss2-w*t2)*(v02*coss2-w*t2) + (g*t2-v02*sinn2)*(g*t2-v02*sinn2))) flagforupdate = 1;
		}
		if (yt3 <= 599 && yt3 >= 56 && xt3 >= 0 && xt3 <= 799) {
			DrawRect (fround(xt3)-1, fround(yt3)-1, 2, 2, BLACK);
			rcounter3 = sqrt((xt3-xcounter3)*(xt3-xcounter3) + (yt3-ycounter3)*(yt3-ycounter3));
			if (rcounter3 > sqrt((v03*coss3-w*t3)*(v03*coss3-w*t3) + (g*t3-v03*sinn3)*(g*t3-v03*sinn3))) flagforupdate = 1;
		}
		if (flagforupdate == 1) {
			if (IMMEDIATETRAECTORY == 0) PartialUpdateBW (0, 55, 800, 545);
			xcounter1 = xt1;
			ycounter1 = yt1;
			xcounter2 = xt2;
			ycounter2 = yt2;
			xcounter3 = xt3;
			ycounter3 = yt3;
		}
		
		if (flag1 == 1) t1 = t1+dt1;
		if (flag2 == 1) t2 = t2+dt2;
		if (flag3 == 1) t3 = t3+dt3;
	}
	PartialUpdateBW (0, 55, 800, 545);
	FillArea (0, 55, 800, 545, WHITE);
	show_arena();
	show_tanks();
	PartialUpdateBW (0, 55, 800, 545);
	
	*x1ret = fround(xt1);
	*y1ret = fround(yt1);
	*x2ret = fround(xt2);
	*y2ret = fround(yt2);
	*x3ret = fround(xt3);
	*y3ret = fround(yt3);
}

// shows weapon 12, recalculates BOOLARENA and lives
// playa [1..PLAYERS]
void laser (int playa) {
	copy_ARENA_to_BOOLARENA();
	float r=15, dr = 1;
	float dpmax=10, dp=1, p;
	int flag = 1, i;
	float x0, y0;
	float xt, yt;
	float coss, sinn;
	float xn, yn;
	float xu, yu;
	
	x0 = (float)IPLAYER[playa-1].x;
	y0 = (float)IPLAYER[playa-1].y-6;
	coss = (float)(cos(IPLAYER[playa-1].angle*M_PI/180));
	sinn = (float)(sin(IPLAYER[playa-1].angle*M_PI/180));
	
	while (flag == 1) {
		r = r + dr;
		xt = x0 + r*coss;
		yt = y0 - r*sinn;
		for (p = dpmax; p >= 0; p = p - dp) {
			xn = p*sinn;
			yn = p*coss;
			xu = -xn;
			yu = -yn;
			if (fround(xt+xn) >= 0 && fround(xt+xn) <= 799 && fround(yt+yn) > 55 && fround(yt+yn) <= 599) {
				BOOLARENA[fround(xt+xn)][599-fround(yt+yn)] = 0;
				if (rand()%2 == 0) DrawPixel (fround(xt+xn), fround(yt+yn), BLACK);
					else DrawPixel (fround(xt+xn), fround(yt+yn), WHITE);
				if (p == dpmax) DrawPixel (fround(xt+xn), fround(yt+yn), WHITE);
				for (i=0; i<PLAYERS; i++) {
					if (fround (xt+xn) >= IPLAYER[i].x-3 && fround (xt+xn) <= IPLAYER[i].x+3 && fround (yt+yn) <= IPLAYER[i].y-1 && fround (yt+yn) >= IPLAYER[i].y-6 && IPLAYER[i].life > 0 && i != playa-1) {
						IPLAYER[i].life = IPLAYER[i].life - 1;
						if (IPLAYER[i].life <= 0) IPLAYER[i].life = 0;
					}
				}
			}
			if (fround(xt+xu) >= 0 && fround(xt+xu) <= 799 && fround(yt+yu) > 55 && fround(yt+yu) <= 599) {
				BOOLARENA[fround(xt+xu)][599-fround(yt+yu)] = 0;
				if (rand()%2 == 0) DrawPixel (fround(xt+xu), fround(yt+yu), BLACK);
					else DrawPixel (fround(xt+xu), fround(yt+yu), WHITE);
				if (p == dpmax) DrawPixel (fround(xt+xu), fround(yt+yu), WHITE);
				for (i=0; i<PLAYERS; i++) {
					if (fround (xt+xu) >= IPLAYER[i].x-3 && fround (xt+xu) <= IPLAYER[i].x+3 && fround (yt+yu) <= IPLAYER[i].y-1 && fround (yt+yu) >= IPLAYER[i].y-6 && IPLAYER[i].life > 0 && i != playa-1) {
						IPLAYER[i].life = IPLAYER[i].life - 1;
						if (IPLAYER[i].life <= 0) IPLAYER[i].life = 0;
					}
				}
			}
		}
		if (xt >= 799 || xt <= 0 || yt >= 599 || yt <= 55) flag = 0;
	}
	PartialUpdateBW (0, 55, 800, 545);
	flag = 1;
	r = 15;
	
	while (flag == 1) {
		r = r + dr;
		xt = x0 + r*coss;
		yt = y0 - r*sinn;
		for (p = dpmax; p >= 0; p = p - dp) {
			xn = p*sinn;
			yn = p*coss;
			xu = -xn;
			yu = -yn;
			if (fround(xt+xn) >= 0 && fround(xt+xn) <= 799 && fround(yt+yn) > 55 && fround(yt+yn) <= 599) {
				DrawPixel (fround(xt+xn), fround(yt+yn), WHITE);
			}
			if (fround(xt+xu) >= 0 && fround(xt+xu) <= 799 && fround(yt+yu) > 55 && fround(yt+yu) <= 599) {
				DrawPixel (fround(xt+xu), fround(yt+yu), WHITE);
			}
		}
		if (xt >= 799 || xt <= 0 || yt >= 599 || yt <= 55) flag = 0;
	}
	PartialUpdateBW (0, 55, 800, 545);
}

// show walking men for weapon 13
// playa [1..PLAYERS]
// returns xret,yret - final coords of men
void walking (int *xret, int *yret, int playa) {
	int direction=1; // 1 - right, -1 - left
	int flag = 1;
	int xt;
	int i;
	int counter = 1;
	
	xt = IPLAYER[playa-1].x;
	
	if (IPLAYER[playa-1].angle >= 90) direction = -1;
	if (IPLAYER[playa-1].angle < 90) direction = 1;
	
	while (flag == 1) {
		xt = xt + direction;
		if (xt > 0 && xt < 799) {
			if (xt-direction != IPLAYER[playa-1].x) DrawLine (xt+2*direction, ARENA[xt]-2, xt-direction+2*direction, ARENA[xt-direction]-2, BLACK);
			if (xt-direction == IPLAYER[playa-1].x) DrawLine (xt-direction, ARENA[xt-direction], xt+2*direction, ARENA[xt]-2, BLACK);
		}
		for (i=0; i<PLAYERS; i++) {
			if (xt <= 0 || xt >= 799 || (xt >= IPLAYER[i].x-4 && xt <= IPLAYER[i].x+4 && i != playa-1 && IPLAYER[i].life > 0)) {
				*xret = xt;
				*yret = ARENA[xt];
				flag = 0;
			}
		}
		counter++;
		if (counter%50 == 0) {
			if (direction == 1) DrawBitmap (xt-15, ARENA[xt]-32, &fighterright);
			if (direction == -1) DrawBitmap (xt-15, ARENA[xt]-32, &fighterleft);
			PartialUpdateBW (0, 55, 800, 545);
		}
	}
	if (direction == 1) DrawBitmap (xt-15, ARENA[xt]-32, &fighterright);
	if (direction == -1) DrawBitmap (xt-15, ARENA[xt]-32, &fighterleft);	
	
	PartialUpdateBW (0, 55, 800, 545);
	FillArea (0, 55, 800, 545, WHITE);
	show_arena();
	show_tanks();
	PartialUpdateBW (0, 55, 800, 545);
}

// explodes dead tanks and set IPLAYER[].dead = 1
void explode_dead () {
	int i;
	for (i=0; i<PLAYERS; i++) {
		if (IPLAYER[i].life <= 0 && IPLAYER[i].dead != 1) {
			explosion (IPLAYER[i].x, IPLAYER[i].y, 3);
			IPLAYER[i].dead = 1;
			fall_landscape();
		}
	}
}

// main function to fire by playa with weapon
// playa [1..PLAYERS]
// weapon [1..13]
void main_fire (int playa, int weapon) {
	int xr, yr;
	int x2, y2;
	int xr1, yr1, xr2, yr2, xr3, yr3;
	
	if ((weapon >= 1 && weapon <= 4) || weapon == 11) {
		show_parabollic_traectory (&xr, &yr, playa, 1);
		explosion (xr, yr, weapon);
		fall_landscape();
		explode_dead();
		if (IPLAYER[playa-1].ammo[weapon-1] != -1) IPLAYER[playa-1].ammo[weapon-1]--;
	}
	if (weapon == 5 || weapon == 6) {
		show_parabollic_traectory (&xr, &yr, playa, 0);
		show_roll_traectory (&x2, &y2, xr, yr);
		FillArea (0, 55, 800, 545, WHITE);
		show_arena();
		show_tanks();
		PartialUpdateBW (0, 55, 800, 545);
		explosion (x2, y2, weapon);
		fall_landscape();
		explode_dead();
		if (IPLAYER[playa-1].ammo[weapon-1] != -1) IPLAYER[playa-1].ammo[weapon-1]--;
	}
	if (weapon == 7 || weapon == 8) {
		show_parabollic_traectory (&xr, &yr, playa, 1);
		acid (xr, yr, weapon);
		fall_landscape();
		explode_dead();
		if (IPLAYER[playa-1].ammo[weapon-1] != -1) IPLAYER[playa-1].ammo[weapon-1]--;
	}
	if (weapon == 9 || weapon == 10) {
		triple_traectory (&xr1, &yr1, &xr2, &yr2, &xr3, &yr3, playa);
		explosion (xr1, yr1, weapon);
		fall_landscape();
		explode_dead();
		explosion (xr2, yr2, weapon);
		fall_landscape();
		explode_dead();
		explosion (xr3, yr3, weapon);
		fall_landscape();
		explode_dead();
		if (IPLAYER[playa-1].ammo[weapon-1] != -1) IPLAYER[playa-1].ammo[weapon-1]--;
	}
	if (weapon == 12) {
		laser (playa);
		fall_landscape();
		explode_dead();
		if (IPLAYER[playa-1].ammo[weapon-1] != -1) IPLAYER[playa-1].ammo[weapon-1]--;
	}
	if (weapon == 13) {
		walking (&xr, &yr, playa);
		explosion (xr, yr, weapon);
		fall_landscape();
		explode_dead();
		if (IPLAYER[playa-1].ammo[weapon-1] != -1) IPLAYER[playa-1].ammo[weapon-1]--;
	}
}

// handler only for press any key (with FLAGSHTOKs it's finally working!!!)
int winner_handler(int type, int par1, int par2) {
	if (type == EVT_KEYPRESS) {
		SetEventHandler(shop_handler);
		return 0;
	}
	return 0;
}

// initial draw in game menu
void draw_menuingame() {
	int xmenu=660, ymenu=58, hmenu=25;
	FillArea (650, 55, 150, 150, WHITE);
	SetFont(OpenFont("LiberationSans", 18, 0), BLACK);
	DrawString (xmenu, ymenu+hmenu*0, "FullUpdate");
	DrawString (xmenu, ymenu+hmenu*1, "Save");
	DrawString (xmenu, ymenu+hmenu*2, "Sudden Death");
	DrawString (xmenu, ymenu+hmenu*3, "Return to game");
	DrawString (xmenu, ymenu+hmenu*4, "Exit to menu");
	PartialUpdateBW(650, 55, 150, 150);
}

// menu in game
int menuingame_handler(int type, int par1, int par2) {
	static int posmenu=1;
	int posmenupr;
	int i;
	int xmenu=655, ymenu=58, wmenu=140, hmenu=25;
	
	if (type == EVT_SHOW && FLAGSHTOKINGAMEMENU == 1) {
		draw_menuingame();
		init_menu_choice (xmenu, ymenu, wmenu, hmenu, posmenu);
	}
	if (type == EVT_KEYPRESS) {
		if (par1 == KEY_UP) {
			posmenupr = posmenu;
			posmenu--;
			if (posmenu < 1) posmenu = 5;
			menu_update (xmenu, ymenu, wmenu, hmenu, posmenu, posmenupr);
		}
		if (par1 == KEY_DOWN) {
			posmenupr = posmenu;
			posmenu++;
			if (posmenu > 5) posmenu = 1;
			menu_update (xmenu, ymenu, wmenu, hmenu, posmenu, posmenupr);
		}
		if (par1 == KEY_OK) {
			if (posmenu == 1) {
				FillArea (650, 55, 150, 150, WHITE);
				FullUpdate();
				FLAGSHTOKGAME = 1;
				FLAGSHTOKINGAMEMENU = 0;
				STARTFROMBEGINNING = 0;
				posmenu = 1;
				SetEventHandler(game_handler);
				return 0;				
			}
			if (posmenu == 3) {
				STARTFROMBEGINNING = 0;
				posmenu = 1;
				FLAGSHTOKGAME = 1;
				FLAGSHTOKINGAMEMENU = 0;
				for (i=0; i<PLAYERS; i++) if (IPLAYER[i].life > 0) IPLAYER[i].life = 1;
				SetEventHandler(game_handler);
				return 0;
			}
			if (posmenu == 4) {
				FLAGSHTOKGAME = 1;
				FLAGSHTOKINGAMEMENU = 0;
				STARTFROMBEGINNING = 0;
				posmenu = 1;
				SetEventHandler(game_handler);
				return 0;
			}
			if (posmenu == 5) {
				GAMESHOPBEGINNING = 1;
				STARTFROMBEGINNING = 1;
				posmenu = 1;
				FLAGSHTOKGAME = 1;
				FLAGSHTOKSHOP = 1;
				FLAGSHTOKINGAMEMENU = 0;
				for (i=0; i<PLAYERS; i++) {
					IPLAYER[i].life = 100;
					IPLAYER[i].dead = 0;
				}
				SetEventHandler(main_handler);
				return 0;
			}
		}
		
	}
	return 0;
}

// show the winner. if playa = -1 - draw
// playa [1..PLAYERS]
void show_winner(int playa) {
	FillArea (0,55,800,545,WHITE);
	SetFont(OpenFont("LiberationSans", 60, 0), BLACK);
	if (playa > 0) {
		DrawTextRect (200, 100, 400, 100, "Winner is", ALIGN_CENTER);
		DrawTextRect (200, 200, 400, 100, NPLAYERS[playa-1], ALIGN_CENTER);
	}
	if (playa == -1) DrawTextRect (200, 100, 400, 100, "How did you do that!?", ALIGN_CENTER);
	SoftUpdate();
}

int game_handler(int type, int par1, int par2) {
	static int turn=1; //number of player to fire, [1..PLAYERS]
	static int phase=1; //phase of firing, 1 - choose weapon, 2 - choose angle and power
	static int weapon=1; //number of weapon in phase 1 [1..13]
	int tempphase=1;
	int acomp, vcomp, wcomp;// for comp firing
//	struct timeb t;
	int templives[8];
	int i;
	int count = 0; //counter to determine, if current player is winner (when all other players have 0 lives)
	int cycler = 1; // cycler for determining DRAW (when all players have 0 lives)
	int tempflag = 1; // flag for fucking with handlers, summoned from cycles (maybe it will help us to break cycle and stop brainfucking)
					  // now we are using global FLAGSHTOKs, brainfucking has ended
	
//	int xr, yr; //debug
	
	if (type == EVT_SHOW && FLAGSHTOKGAME == 1) {
		if (STARTFROMBEGINNING == 1) {
			turn = 1;
			phase = 1;
			weapon = 1;
			ClearScreen();
			WIND = rand()%1998-999;
			show_mainpanel();
			show_player_info(turn-1);
			show_arena();
			invert_weapon_choice(weapon);
			FullUpdate();
			drop_tanks();
		}
		if (STARTFROMBEGINNING == 0) {
			FillArea (0, 55, 800, 545, WHITE);
			show_arena();
			show_tanks();
			draw_tank_wbarrel(turn);
			clear_info_panel();
			show_player_info(turn-1);
			PartialUpdateBW (0, 0, 800, 600);
		}
	}
	// cycle for computer players
	tempflag = 1;
	while (IPLAYER[turn-1].cplayer == 1 && FLAGSHTOKGAME == 1) {
		if (IPLAYER[turn-1].life > 0) {
			FillArea (0,0,800,50,WHITE);
			show_mainpanel();
			show_player_info(turn-1);
			PartialUpdateBW (0,0,800,50);
		
			wcomp = rand()%11+1;
			while (IPLAYER[turn-1].ammo[wcomp-1] == 0) wcomp = rand()%11+1;
			invert_weapon_choice(wcomp);
			PartialUpdateBW (0,0,800,50);
			
			SetFont(OpenFont("LiberationSans", 24, 0), BLACK);
			DrawString(IPLAYER[turn-1].x-5, IPLAYER[turn-1].y-50, "?");
			PartialUpdateBW(IPLAYER[turn-1].x-5, IPLAYER[turn-1].y-50, 60, 60);
			
			calc_best_fire (&vcomp, &acomp, turn);
			IPLAYER[turn-1].angle = acomp;
			IPLAYER[turn-1].power = vcomp;
			
			SetFont(OpenFont("LiberationSans", 24, 0), WHITE);
			DrawString(IPLAYER[turn-1].x-5, IPLAYER[turn-1].y-50, "?");
			PartialUpdateBW(IPLAYER[turn-1].x-5, IPLAYER[turn-1].y-50, 60, 60);
			
			clear_info_panel();
			show_player_info(turn-1);
			PartialUpdateBW (0,0,800,50);
			redraw_tank_and_angle (turn);
			
			for (i=0; i<PLAYERS; i++) {
				templives[i] = IPLAYER[i].life;
			}
			
			main_fire (turn, wcomp);
			
			invert_weapon_choice(wcomp);
			PartialUpdateBW (0,0,800,50);
//			show_parabollic_traectory (&xcomp, &ycomp, turn, 1);
//			explosion(xcomp, ycomp, wcomp);
//			fall_landscape();
//			explode_dead();
			
			for (i=0; i<PLAYERS; i++) {
				IPLAYER[turn-1].money = IPLAYER[turn-1].money + (templives[i]-IPLAYER[i].life);
			}
			if (IPLAYER[turn-1].money > 999) IPLAYER[turn-1].money = 999;
			
			count = 0;
			for (i=0; i<PLAYERS; i++) if (IPLAYER[i].life <= 0 && i != turn-1) count ++;
			if (count == PLAYERS-1) { // we dont calculate draw among computers, it's impossible, but...
				show_winner (turn);
				IPLAYER[turn-1].money = IPLAYER[turn-1].money + 50;
				if (IPLAYER[turn-1].money > 999) IPLAYER[turn-1].money = 999;
				GAMESHOPBEGINNING = 1;
				STARTFROMBEGINNING = 1;
				turn = 1;
				phase = 1;
				weapon = 1;
				tempflag = 0;
				FLAGSHTOKGAME = 0;
				FLAGSHTOKSHOP = 1;
				SetEventHandler(winner_handler);
				return 0;
			}
		}
		turn++;
		if (turn > PLAYERS) turn = 1;
		WIND = rand()%1998-999;
		weapon = 1;
		if (IPLAYER[turn-1].cplayer == 0 && IPLAYER[turn-1].life > 0) {
			FillArea (0,0,800,50,WHITE);
			show_mainpanel();
			show_player_info(turn-1);
			invert_weapon_choice(weapon);
			draw_tank_wbarrel(turn);
			PartialUpdateBW (0,0,800,50);
		}
	}
	cycler = 1;
	tempflag = 1;
	while (IPLAYER[turn-1].life == 0 && FLAGSHTOKGAME == 1) {
		turn++;
		cycler++;
		if (turn > PLAYERS) turn = 1;
		if (cycler > 3*PLAYERS) {
			GAMESHOPBEGINNING = 1;
			STARTFROMBEGINNING = 1;
			turn = 1;
			phase = 1;
			weapon = 1;
			tempflag = 0;
			FLAGSHTOKGAME = 0;
			FLAGSHTOKSHOP = 1;
			show_winner(-1);
			SetEventHandler(winner_handler);
			return 0;
		}
	}
	if (IPLAYER[turn-1].cplayer == 1 && FLAGSHTOKGAME == 1) {
		STARTFROMBEGINNING = 0;
		SetEventHandler(game_handler);
		return 0;
	}
	
	if (type == EVT_KEYPRESS) {
		if (par1 == KEY_OK) {
			tempphase = phase;
			if (tempphase == 1) {
				invert_weapon_choice(weapon);
				PartialUpdateBW (0,0,455,50);
				phase = 2;
			}
			if (tempphase == 2) {
				for (i=0; i<PLAYERS; i++) {
					templives[i] = IPLAYER[i].life;
				}
				main_fire (turn, weapon);
				for (i=0; i<PLAYERS; i++) {
					IPLAYER[turn-1].money = IPLAYER[turn-1].money + (templives[i]-IPLAYER[i].life);
				}
				if (IPLAYER[turn-1].money > 999) IPLAYER[turn-1].money = 999;
				
				count = 0;
				for (i=0; i<PLAYERS; i++) if (IPLAYER[i].life <= 0 && i != turn-1) count ++;
				if (count == PLAYERS-1) {
					show_winner (turn);
					IPLAYER[turn-1].money = IPLAYER[turn-1].money + 50;
					if (IPLAYER[turn-1].money > 999) IPLAYER[turn-1].money = 999;
					GAMESHOPBEGINNING = 1;
					STARTFROMBEGINNING = 1;
					turn = 1;
					phase = 1;
					weapon = 1;
					FLAGSHTOKGAME = 0;
					FLAGSHTOKSHOP = 1;
					SetEventHandler(winner_handler);
					return 0;
				}
				
				phase = 1;
				weapon = 1;
				WIND = rand()%1998-999;
				turn++;
				if (turn > PLAYERS) turn = 1;
				
				cycler = 1;
				tempflag = 1;
				while (IPLAYER[turn-1].life == 0 && FLAGSHTOKGAME == 1) {
					turn++;
					cycler++;
					if (turn > PLAYERS) turn = 1;
					if (cycler > 3*PLAYERS) {
						GAMESHOPBEGINNING = 1;
						STARTFROMBEGINNING = 1;
						turn = 1;
						phase = 1;
						weapon = 1;
						tempflag = 0;
						show_winner(-1);
						FLAGSHTOKGAME = 0;
						FLAGSHTOKSHOP = 1;
						SetEventHandler(winner_handler);
						return 0;
					}
				}
				
				if (IPLAYER[turn-1].cplayer == 1) {
					STARTFROMBEGINNING = 0;
					SetEventHandler(game_handler);
					return 0;
				}
				clear_info_panel();
				show_player_info(turn-1);
				invert_weapon_choice(weapon);
				draw_tank_wbarrel(turn);
				PartialUpdateBW (0,0,800,IPLAYER[turn-1].y);
			}
		}
		if (par1 == KEY_PLUS) {
			FLAGSHTOKGAME = 0;
			FLAGSHTOKINGAMEMENU = 1;
			SetEventHandler(menuingame_handler);			
			return 0;
		}
		if (par1 == KEY_LEFT) {
			if (phase == 1) {
				invert_weapon_choice(weapon);
				weapon--;
				if (weapon < 1) weapon = 13;
				while (IPLAYER[turn-1].ammo[weapon-1] == 0) {
					weapon--;
					if (weapon < 1) weapon = 13;
				}
				invert_weapon_choice(weapon);
				PartialUpdateBW (2, 17, 453, 32);
			}
			if (phase == 2) {
				IPLAYER[turn-1].angle++;
				if (IPLAYER[turn-1].angle > 180 ) IPLAYER[turn-1].angle = 180;
				redraw_tank_and_angle (turn);
			}
		}
		if (par1 == KEY_RIGHT) {
			if (phase == 1) {
				invert_weapon_choice(weapon);
				weapon++;
				if (weapon > 13) weapon = 1;
				while (IPLAYER[turn-1].ammo[weapon-1] == 0) {
					weapon++;
					if (weapon > 13) weapon = 1;
				}
				invert_weapon_choice(weapon);
				PartialUpdateBW (2, 17, 453, 32);
			}
			if (phase == 2) {
				IPLAYER[turn-1].angle--;
				if (IPLAYER[turn-1].angle < 0 ) IPLAYER[turn-1].angle = 0;
				redraw_tank_and_angle (turn);
			}
		}
		if (par1 == KEY_UP) {
			if (phase == 2) {
				IPLAYER[turn-1].power = IPLAYER[turn-1].power + 10;
				if (IPLAYER[turn-1].power > 990) IPLAYER[turn-1].power = 990;
				clear_info_panel();
				show_player_info(turn-1);
				PartialUpdateBW (555, 1, 50, 30);
			}
		}
		if (par1 == KEY_DOWN) {
			if (phase == 2) {
				IPLAYER[turn-1].power = IPLAYER[turn-1].power - 10;
				if (IPLAYER[turn-1].power < 10) IPLAYER[turn-1].power = 10;
				clear_info_panel();
				show_player_info(turn-1);
				PartialUpdateBW (555, 1, 50, 30);
			}
		}
	}
	if (type == EVT_KEYREPEAT) {
		if (par1 == KEY_LEFT) {
			if (phase == 2) {
				IPLAYER[turn-1].angle = IPLAYER[turn-1].angle + 10;
				if (IPLAYER[turn-1].angle > 180 ) IPLAYER[turn-1].angle = 180;
				redraw_tank_and_angle (turn);
			}		
		}
		if (par1 == KEY_RIGHT) {
			if (phase == 2) {
				IPLAYER[turn-1].angle = IPLAYER[turn-1].angle - 10;
				if (IPLAYER[turn-1].angle <0 ) IPLAYER[turn-1].angle = 0;
				redraw_tank_and_angle (turn);
			}	
		}
		if (par1 == KEY_UP) {
			if (phase == 2) {
				IPLAYER[turn-1].power = IPLAYER[turn-1].power + 100;
				if (IPLAYER[turn-1].power > 990) IPLAYER[turn-1].power = 990;
				clear_info_panel();
				show_player_info(turn-1);
				PartialUpdateBW (555, 1, 50, 30);
			}
		}
		if (par1 == KEY_DOWN) {
			if (phase == 2) {
				IPLAYER[turn-1].power = IPLAYER[turn-1].power - 100;
				if (IPLAYER[turn-1].power < 10) IPLAYER[turn-1].power = 10;
				clear_info_panel();
				show_player_info(turn-1);
				PartialUpdateBW (555, 1, 50, 30);
			}
		}		
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////
// END OF GAMEPLAY CODE
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// MENU AND REDACTOR CODE
//////////////////////////////////////////////////////////////////////

// draw main menu on EVT_SHOW
void main_draw (int xl, int yl, int hgt) {
	char diff[10];
	char stnofpl[30] = "Number of Players: ";
	char stdiff[25] = "Difficuilty: ";
	char nofpl[2];
	
	nofpl[0] = itoc (PLAYERS);
	nofpl[1] = '\0';
	if (DIFFICUILTY == 1) strcpy (diff, "Easy");
	if (DIFFICUILTY == 2) strcpy (diff, "Medium");
	if (DIFFICUILTY == 3) strcpy (diff, "Hard");
	
	strcat (stdiff, diff);
	strcat (stnofpl, nofpl);
	SetOrientation (2);
//	DrawBitmap (0,0,&tank1);
//	DrawBitmap (400,0,&tank2);
	draw_trubitmap (400, 0, &tank2pic);
	draw_trubitmap (0, 0, &tank1pic);
	draw_trubitmap (0, 300, &tank3pic);
//	DrawBitmap (1,300,&tank3);
	DrawLine (0, SCRW/2, SCRH, SCRW/2, BLACK);
	DrawLine (SCRH/2, 0, SCRH/2, SCRW/2, BLACK);
	SetFont(OpenFont("LiberationSans", 40, 0), BLACK);
	DrawString (xl+15, yl-50, "TANK WAR");
	SetFont(OpenFont("LiberationSans", 24, 0), BLACK);
	DrawString (xl, yl, "Start");
	DrawString (xl, yl+hgt, stnofpl);
	DrawString (xl, yl+2*hgt, stdiff);
	DrawString (xl, yl+3*hgt, "Instructions");
	DrawString (xl, yl+4*hgt, "Exit");
}

// xl,yl - left uppder corner of menu, wdt,hgt - width and height of 1 item of menu, posmenu - number of choice, posmenupr - previous number of choice

// redraw one item in main menu (on change difficuilty or number of players)
void redraw_choice_menu (int xl, int yl, int wdt, int hgt, int posmenu, int *mainwd) {
	char diff[10];
	char stnofpl[30] = "Number of Players: ";
	char stdiff[25] = "Difficuilty: ";
	char nofpl[2];
	int newx,newy;
	
	nofpl[0] = itoc (PLAYERS);
	nofpl[1] = '\0';	
	if (DIFFICUILTY == 1) strcpy (diff, "Easy");
	if (DIFFICUILTY == 2) strcpy (diff, "Medium");
	if (DIFFICUILTY == 3) strcpy (diff, "Hard");
	newx = xl;
	newy = yl+(posmenu-1)*hgt;
	strcat (stdiff, diff);
	strcat (stnofpl, nofpl);
	if (posmenu == 2) {
		InvertAreaBW (xl, yl+(posmenu-1)*hgt, mainwd[posmenu-1], hgt);
		FillArea (newx+213, newy, 24, 24, WHITE);
		SetFont(OpenFont("LiberationSans", 24, 0), BLACK);
		DrawString (newx, newy, stnofpl);
		init_mainmenu_choice (xl, yl, wdt, hgt, posmenu, mainwd);
	}
	if (posmenu == 3) {
		InvertAreaBW (xl, yl+(posmenu-1)*hgt, mainwd[posmenu-1], hgt);
		FillArea (newx+100, newy, 120, 27, WHITE);
		SetFont(OpenFont("LiberationSans", 24, 0), BLACK);
		DrawString (newx, newy, stdiff);
		init_mainmenu_choice (xl, yl, wdt, hgt, posmenu, mainwd);
	}
}

// redraw one item in players menu (on change computer-human)
void redraw_players_menu (int xl, int yl, int wdt, int hgt, int posmenu) {
	char comporhuman [15];
	
	SetFont(OpenFont("LiberationSans", 24, 0), BLACK);
	if (IPLAYER[posmenu-1].cplayer == 1) strcpy (comporhuman, "Computer");
	if (IPLAYER[posmenu-1].cplayer == 0) strcpy (comporhuman, "Human");
	FillArea (xl, yl+(posmenu-1)*hgt, wdt, hgt, WHITE);
	DrawString (xl, yl+(posmenu-1)*hgt, NPLAYERS[posmenu-1]);
	DrawString (xl+130, yl+(posmenu-1)*hgt, comporhuman);
	init_menu_choice (xl, yl, wdt, hgt, posmenu);
//	PartialUpdateBW (xl, yl+(posmenu-1)*hgt, wdt, hgt);
}

// initial invert of one intem in any menu
void init_menu_choice (int xl, int yl, int wdt, int hgt, int posmenu) {
	InvertAreaBW (xl, yl+(posmenu-1)*hgt, wdt, hgt);
	PartialUpdateBW (xl, yl+(posmenu-1)*hgt, wdt, hgt);
}

// initial invert of one intem in main menu
void init_mainmenu_choice (int xl, int yl, int wdt, int hgt, int posmenu, int *mainwd) {
	InvertAreaBW (xl, yl+(posmenu-1)*hgt, mainwd[posmenu-1], hgt);
//	InvertAreaBW (xl, yl+(posmenu-1)*hgt, wdt, hgt);
	PartialUpdateBW (xl, yl+(posmenu-1)*hgt, mainwd[posmenu-1], hgt);
}

// moves choice in main menu from posmenupr to posmenu
void mainmenu_update (int xl, int yl, int wdt, int hgt, int posmenu, int posmenupr) {
	int oldx, oldy;
	int newx, newy;
	int miny, maxsumy;
	int mainwd[5] = {60, 235, 205, 130, 45};
		
	oldx = xl;
	newx = xl;
	oldy = yl+(posmenupr-1)*hgt;
	newy = yl+(posmenu-1)*hgt;
	InvertAreaBW (oldx, oldy, mainwd[posmenupr-1], hgt);
	InvertAreaBW (newx, newy, mainwd[posmenu-1], hgt);
//	InvertAreaBW (oldx, oldy, wdt, hgt);
//	InvertAreaBW (newx, newy, wdt, hgt);
	
	miny = minint(newy, oldy);
	maxsumy = maxint (newy+hgt, oldy+hgt);
	
	PartialUpdateBW (xl, miny, maxint(mainwd[posmenupr-1], mainwd[posmenu-1]), maxsumy-miny);
}

// moves choice in menu from posmenupr to posmenu
void menu_update (int xl, int yl, int wdt, int hgt, int posmenu, int posmenupr) {
	int oldx, oldy;
	int newx, newy;
	int miny, maxsumy;
	
	oldx = xl;
	newx = xl;
	oldy = yl+(posmenupr-1)*hgt;
	newy = yl+(posmenu-1)*hgt;
	InvertAreaBW (oldx, oldy, wdt, hgt);
	InvertAreaBW (newx, newy, wdt, hgt);
	
	miny = minint(newy, oldy);
	maxsumy = maxint (newy+hgt, oldy+hgt);
	
	PartialUpdateBW (xl, miny, wdt, maxsumy-miny);
}

// returns char representation of integer [1..40] (for virtual keyboard)
char return_key (int num) {
	if (num == 1) return '1';
	if (num == 2) return '2';
	if (num == 3) return '3';
	if (num == 4) return '4';
	if (num == 5) return '5';
	if (num == 6) return '6';
	if (num == 7) return '7';
	if (num == 8) return '8';
	if (num == 9) return '9';
	if (num == 10) return '0';	
	if (num == 11) return 'q';
	if (num == 12) return 'w';
	if (num == 13) return 'e';
	if (num == 14) return 'r';
	if (num == 15) return 't';
	if (num == 16) return 'y';
	if (num == 17) return 'u';
	if (num == 18) return 'i';
	if (num == 19) return 'o';
	if (num == 20) return 'p';	
	if (num == 21) return 'a';
	if (num == 22) return 's';
	if (num == 23) return 'd';
	if (num == 24) return 'f';
	if (num == 25) return 'g';
	if (num == 26) return 'h';
	if (num == 27) return 'j';
	if (num == 28) return 'k';
	if (num == 29) return 'l';
	if (num == 30) return '_';	
	if (num == 31) return 'z';
	if (num == 32) return 'x';
	if (num == 33) return 'c';
	if (num == 34) return 'v';
	if (num == 35) return 'b';
	if (num == 36) return 'n';
	if (num == 37) return 'm';
	if (num == 38) return ',';
	if (num == 39) return '.';
	if (num == 40) return ' ';
	return '%';
}

//draws keyboard on EVT_SHOW
void draw_keyboard (int xp, int yp) {
	int i, j;
	char strkey [2];
	int wkl = 30;
	int hkl = 30;
	SetFont(OpenFont("LiberationSans", 24, 0), BLACK);
	
	for (i=1; i<11; i++) {
		for (j=1; j<5; j++) {
			strkey[0] = return_key(i+(j-1)*10);
			strkey[1] = '\0';
			DrawString (xp+(i-1)*wkl, yp+(j-1)*hkl, strkey);
		}
	}
	SetFont(OpenFont("LiberationSans", 34, 0), BLACK);
	DrawString (xp-3*wkl, yp+2*hkl, "OK");
	DrawString (xp+11*wkl, yp+2*hkl, "Backspace");
	
}

// initializing virtual keyboard cursor (during EVT_SHOW)
void init_keyboard_kursor (int xp, int yp, int wkl, int hkl, int posx, int posy) {
	int truposx=1, truposy=1, truwkl=30, truhkl=30;
	
	if (posx < 11 && posx > 0) {
		truposx = xp+(posx-1)*wkl-8;
		truposy = yp+(posy-1)*hkl;
		truwkl = wkl;
		truhkl = hkl;
	}
	if (posx == 11) { 
		truposx = xp+11*wkl;
		truposy = yp+2*hkl;
		truwkl = 175;
		truhkl = 40;
	}
	if (posx == 0) {
		truposx = xp-3*wkl;
		truposy = yp+2*hkl;
		truwkl = 50;
		truhkl = 40;		
	}
	
	InvertAreaBW (truposx, truposy, truwkl, truhkl);
	PartialUpdateBW (truposx, truposy, truwkl, truhkl);
}

// update cursor in virtual keyboard after EVT_KEYPRESS
void update_keyboard_kursor (int xp, int yp, int wkl, int hkl, int posx, int posy, int posxpr, int posypr) {
	int truposx=1, truposy=1, truposxpr=1, truposypr=1, truwkl=30, truhkl=30, truwklpr=30, truhklpr=30;
	int minx, miny;
	int maxsumx;
	int maxsumy;
	
	if (posx < 11 && posx > 0) {
		truposx = xp+(posx-1)*wkl-8;
		truposy = yp+(posy-1)*hkl;
		truwkl = wkl;
		truhkl = hkl;
	}
	if (posx == 11) { 
		truposx = xp+11*wkl;
		truposy = yp+2*hkl;
		truwkl = 175;
		truhkl = 40;
	}
	if (posx == 0) {
		truposx = xp-3*wkl;
		truposy = yp+2*hkl;
		truwkl = 50;
		truhkl = 40;		
	}

	if (posxpr < 11 && posxpr > 0) {
		truposxpr = xp+(posxpr-1)*wkl-8;
		truposypr = yp+(posypr-1)*hkl;
		truwklpr = wkl;
		truhklpr = hkl;
	}
	if (posxpr == 11) { 
		truposxpr = xp+11*wkl;
		truposypr = yp+2*hkl;
		truwklpr = 175;
		truhklpr = 40;
	}
	if (posxpr == 0) {
		truposxpr = xp-3*wkl;
		truposypr = yp+2*hkl;
		truwklpr = 50;
		truhklpr = 40;		
	}
		
	InvertAreaBW (truposxpr, truposypr, truwklpr, truhklpr);
	InvertAreaBW (truposx, truposy, truwkl, truhkl);
	minx = minint(truposx, truposxpr);
	miny = minint(truposy, truposypr);
	maxsumx = maxint (truposx+truwkl, truposxpr+truwklpr);
	maxsumy = maxint (truposy+truhkl, truposypr+truhklpr);
	PartialUpdateBW (minx, miny, maxsumx-minx, maxsumy-miny);
}

// redraws player name in virtual keyboard mode (after BACKSPACE)
void redraw_player_name (int xp, int yp) {
	SetFont(OpenFont("LiberationSans", 46, 0), BLACK);
	FillArea (xp, yp, 250, 60, WHITE);
	DrawString (xp, yp, NPLAYERS[CHOICE-1]);
	PartialUpdateBW (xp, yp, 250, 60);
}

// handler for editing player name
int edit_player_handler (int type, int par1, int par2) {
	static int posx = 1;
	static int posy = 1;
	static int posxpr = 1;
	static int posypr = 1;
	static int len = 0;
	int xkb, ykb, wkl, hkl;
	char buff[10];
	char dump[2];
	
	xkb = SCRH/4;
	ykb = 300;
	wkl = 30;
	hkl = 30;
	
	if (type == EVT_SHOW) {
		ClearScreen();
		SetFont(OpenFont("LiberationSans", 46, 0), BLACK);
		DrawString (SCRH/4, 150, NPLAYERS[CHOICE-1]);
		draw_keyboard (xkb, ykb);
		init_keyboard_kursor(xkb, ykb, wkl, hkl, posx, posy);
		len = strlen (NPLAYERS[CHOICE-1]);		
		FullUpdate();
	}
	if (type == EVT_KEYPRESS) {
		if (par1 == KEY_RIGHT) {
			posxpr = posx;
			posypr = posy;
			posx++;
			if (posx > 11) posx = 0;
			update_keyboard_kursor(xkb, ykb, wkl, hkl, posx, posy, posxpr, posypr);
		}
		if (par1 == KEY_LEFT) {
			posxpr = posx;
			posypr = posy;
			posx--;
			if (posx < 0) posx = 11;
			update_keyboard_kursor(xkb, ykb, wkl, hkl, posx, posy, posxpr, posypr);	
		}
		if (par1 == KEY_UP) {
			posypr = posy;
			posxpr = posx;
			posy--;
			if (posy < 1) posy = 1;
			update_keyboard_kursor(xkb, ykb, wkl, hkl, posx, posy, posxpr, posypr);
		}
		if (par1 == KEY_DOWN) {
			posypr = posy;
			posxpr = posx;
			posy++;
			if (posy > 4) posy = 4;
			update_keyboard_kursor(xkb, ykb, wkl, hkl, posx, posy, posxpr, posypr);
		}
		if (par1 == KEY_OK) {
			if (posx == 0) { SetEventHandler(start_handler); return 0; }
			if (posx == 11) {
				len--;
				if (len < 0) len=0;
				strcpy (buff, "");
				strncat (buff, NPLAYERS[CHOICE-1], len);
				strcpy (NPLAYERS[CHOICE-1], buff);
				redraw_player_name (SCRH/4, 150);
			}
			if (posx <11 && posx > 0) {
				if (len < 8) {
					dump[0] = return_key (posx+(posy-1)*10);
					dump[1] = '\0';
					strcat (NPLAYERS[CHOICE-1], dump);
					redraw_player_name (SCRH/4, 150);
					len++;
				}
			}
			
		}
	}
	return 0;
}

// initial draw of options screen
void show_options_init () {
	ClearScreen();
	SetFont(OpenFont("LiberationSans", 36, 0), BLACK);
	DrawString (300, 300, "Options");
}

// handler for options screen
int options_handler (int type, int par1, int par2) {
	int xmenu, ymenu, wmenu, hmenu;
	if (type == EVT_SHOW) {
		show_options_init();
		FullUpdate();
		FineUpdate();
	}
	return 0;
}

// handler for screen after pressing START in main menu or exiting from virtual keyboard
int start_handler (int type, int par1, int par2) {
	int xmenu, ymenu, wmenu, hmenu;
	static int posmenu = 1;
	static int posmenupr = 1;
	int i, corh=0;
	char comporhuman [15];
	xmenu = 550;
	ymenu = 75;
	wmenu = 240;
	hmenu = 30;
	
	if (type == EVT_SHOW) {
		ClearScreen();
		DrawBitmap (0,0,&selectplayer);
		SetFont(OpenFont("LiberationSans", 36, 0), BLACK);		
		DrawString (SCRH/2-150, 10, "Choose your destiny, Kao-Shahn!");
		SetFont(OpenFont("LiberationSans", 24, 0), BLACK);
		posmenu = 1;
		posmenupr = 1;
		for (i=1; i<=PLAYERS; i++) {
			if (IPLAYER[i-1].cplayer == 1) strcpy (comporhuman, "Computer");
			if (IPLAYER[i-1].cplayer == 0) strcpy (comporhuman, "Human");
			DrawString (xmenu, ymenu+(i-1)*hmenu, NPLAYERS[i-1]);
			DrawString (xmenu+130, ymenu+(i-1)*hmenu, comporhuman);
		}
		DrawString (xmenu, ymenu+PLAYERS*hmenu, "START BATTLE");
		DrawString (xmenu, ymenu+(PLAYERS+1)*hmenu, "Exit to Menu");
		init_menu_choice (xmenu, ymenu, wmenu, hmenu, posmenu);
		FullUpdate();
	}
	if (type == EVT_KEYPRESS) {
		if ((par1 == KEY_LEFT || par1 == KEY_RIGHT) && posmenu <= PLAYERS) {
			if (IPLAYER[posmenu-1].cplayer == 0) corh=1;
			if (IPLAYER[posmenu-1].cplayer == 1) corh=0;
			IPLAYER[posmenu-1].cplayer = corh;
			redraw_players_menu (xmenu, ymenu, wmenu, hmenu, posmenu);
		}
		if (par1 == KEY_UP) {
			posmenupr = posmenu;
			posmenu--;
			if (posmenu < 1) posmenu = PLAYERS+2;
			menu_update (xmenu, ymenu, wmenu, hmenu, posmenu, posmenupr);
		}
		if (par1 == KEY_DOWN) {
			posmenupr = posmenu;
			posmenu++;
			if (posmenu > PLAYERS+2) posmenu = 1;
			menu_update (xmenu, ymenu, wmenu, hmenu, posmenu, posmenupr);
		}
		if (par1 == KEY_OK) {
			if (posmenu == PLAYERS+2) SetEventHandler(main_handler);
			if (posmenu == PLAYERS+1) {
				fill_arena();
				STARTFROMBEGINNING = 1;
				SetEventHandler(game_handler);
			}
			if (posmenu <= PLAYERS) {
				CHOICE = posmenu;
				SetEventHandler (edit_player_handler);
				return 0;
			}
		}
	}
	return 0;
}

// handler for instruction screen
int instruction_handler(int type, int par1, int par2) {
	if (type == EVT_SHOW) {
		ClearScreen();
		SetFont(OpenFont("LiberationSans", 14, 0), BLACK);
		DrawTextRect(SCRH/10, SCRW/10, SCRH*4/5, SCRW/4*5, "Instruction \n Тут будет инструкция", ALIGN_CENTER);
		FullUpdate();
	}
	if (type == EVT_KEYPRESS) { SetEventHandler(main_handler); return 0; }
	return 0;
}

// most fucking important handler in the program
int main_handler (int type, int par1, int par2) {
	static int posmenu=1;
	static int posmenupr=1;
	int xmenu = 50;
	int ymenu = 400;
	int wmenu = 250;
	int hmenu = 30;
	int mainwd[5] = {60, 235, 205, 130, 45};
	
	if (type == EVT_SHOW) {
		ClearScreen();
		main_draw(xmenu, ymenu, hmenu);
		init_mainmenu_choice(xmenu, ymenu, wmenu, hmenu, posmenu, mainwd);
		FullUpdate();
		FineUpdate();
	}
	if (type == EVT_KEYPRESS) 	{
		switch (par1) {
			
			case KEY_UP:
				posmenupr = posmenu;
				posmenu--;
				if (posmenu < 1) posmenu = 5;
				mainmenu_update (xmenu, ymenu, wmenu, hmenu, posmenu, posmenupr);
				break;
			case KEY_DOWN:
				posmenupr = posmenu;
				posmenu++;
				if (posmenu > 5) posmenu = 1;
				mainmenu_update (xmenu, ymenu, wmenu, hmenu, posmenu, posmenupr);
				break;
			case KEY_OK:
				if (posmenu == 5) CloseApp();
				if (posmenu == 4) { SetEventHandler(instruction_handler); return 0; }
				if (posmenu == 1) { SetEventHandler(start_handler); return 0; }
				break;
			case KEY_RIGHT:
				if (posmenu == 2) {
					PLAYERS++;
					if (PLAYERS > 8) PLAYERS = 2;
					redraw_choice_menu (xmenu, ymenu, wmenu, hmenu, posmenu, mainwd);
				}
				if (posmenu == 3) {
					DIFFICUILTY++;
					if (DIFFICUILTY > 3) DIFFICUILTY = 1;
					redraw_choice_menu (xmenu, ymenu, wmenu, hmenu, posmenu, mainwd);
				}
				break;
			case KEY_LEFT:
				if (posmenu == 2) {
					PLAYERS--;
					if (PLAYERS < 2) PLAYERS = 8;
					redraw_choice_menu (xmenu, ymenu, wmenu, hmenu, posmenu, mainwd);
				}
				if (posmenu == 3) {
					DIFFICUILTY--;
					if (DIFFICUILTY < 1) DIFFICUILTY = 3;
					redraw_choice_menu (xmenu, ymenu, wmenu, hmenu, posmenu, mainwd);
				}
				break;
		}
	}
	return 0;	
}
//////////////////////////////////////////////////////////////////////
// END OF MENU AND REDACTOR CODE
//////////////////////////////////////////////////////////////////////

// mallocing and filling NPLAYERS global array of pointers to names
// where can I use free() to free allocated memory???
void init_NPLAYERS () {
	int i;
	for (i=0; i<8; i++) NPLAYERS[i] = (char*) malloc(10);
	strcpy (NPLAYERS[0], "player 1");
	strcpy (NPLAYERS[1], "player 2");
	strcpy (NPLAYERS[2], "player 3");
	strcpy (NPLAYERS[3], "player 4");
	strcpy (NPLAYERS[4], "player 5");
	strcpy (NPLAYERS[5], "player 6");
	strcpy (NPLAYERS[6], "player 7");
	strcpy (NPLAYERS[7], "player 8");
}

// initing changable players' info, except names and x,y
void init_players_info() {
	int i,j;
	for (i=0; i<8; i++) {
		IPLAYER[i].angle = 90;
		IPLAYER[i].power = 150;
		IPLAYER[i].cplayer = 0;
		IPLAYER[i].life = 100;
		IPLAYER[i].dead = 0;
		IPLAYER[i].money = 0;
		IPLAYER[i].wins = 0;
	}
	for (i=0; i<8; i++) {
		for (j=0; j<13; j++) {
			IPLAYER[i].ammo[j] = 0;
			if (j == 0) IPLAYER[i].ammo[j] = -1;
			if (j == 1) IPLAYER[i].ammo[j] = 3;
			if (j == 2) IPLAYER[i].ammo[j] = 2;
		}
	}
	//for debugging
//	IPLAYER[0].money = 999;
	for (i=0; i<13; i++) IPLAYER[0].ammo[i] = 574;
	IPLAYER[0].ammo[0] = -1;
}
// main
int main(int argc, char **argv)
{
	srand(time(NULL));
	init_NPLAYERS();
	init_players_info();
	InkViewMain(main_handler);
	return 0;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <dlfcn.h>
#include <math.h>
#include <sys/timeb.h>
#include "inkview.h"

#define SCRW 600
#define SCRH 800
#define MY_MASK 0777


typedef struct{
  short width;
  short height;
  unsigned char data[];
} trubitmap;

extern const ibitmap weapon01, weapon02, weapon03, weapon04, weapon05, weapon06, weapon07, weapon08;
extern const ibitmap weapon09, weapon10, weapon11, weapon12, weapon13;
extern const ibitmap fighterleft, fighterright, windleft, windright;
extern const ibitmap bonusbarrel;
extern const ibitmap weapon02transparent, weapon03transparent, weapon04transparent, weapon05transparent, weapon06transparent;
extern const ibitmap weapon07transparent, weapon08transparent, weapon09transparent, weapon10transparent;
extern const trubitmap tank1pic, tank2pic, tank3pic, instructionpic, optionspic;
extern const trubitmap nameplayerspic, salespic;

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
	int turned;	  // had this player his turn this round?
} infoplayer; // struct for changable variables of player, except names

typedef struct {
	int x,y; //x,y of bonus, if 0,0 - bonus is disabled
//	int weapon; //weapon in bonus
} bonusstruct;

typedef struct {
	char name[10]; //name of player in hiscores table
	int cplayer; //0 - human player, 1 - computer player
	int wins; //amound of wins of player
} hiscoresstruct;

typedef struct {
	int h; //height of sprite
	int w; //width of sprite
	unsigned char data[]; //0 or 1
} spritestruct;

int PLAYERS=2, DIFFICUILTY=1; // PLAYERS [2..8], DIFFICUILTY [1..3]
char *NPLAYERS [8];  //names of players
int WIND=0; //global wind variable
int CHOICE = 1; //global number of player to edit (edit_player_handler)
int ARENA [800]; // area of fighting. we have 800 y coordinates of peaks (y=0 in left up corner!!!)
short BOOLARENA [800][545];//representation of arena in 2-array, where =1 - pixel, =0 - empty ([0][0] in left down corner!!!)
int FLAGCOMPUTERBATTLES = 1; //determining, if we already chose to watch computer battle. it's nullfied (mean =1), if you go from game to main menu or to shop! (feature :)
int TURNFORSAVE = 1; //player's turn (for save-load process)
int PHASEFORSAVE = 1; //player's phase (for save-load process). I've decided no to use phase of firing, so this is (for now) not needed variable, but we'll keep it just in case, but! we are saving it to file
// hiscores variables
int COMPWINS = 0; //computer comand wins
int HUMANWINS = 0; //human comand wins
// handler flags
int STARTFROMBEGINNING = 1; // global flag for beginning of new game (uses in game_handler). 1 - we are beginning from scratch, 0 - returning from handler to game_handler
int GAMESHOPBEGINNING = 1; // global flag for gameshop, ^^^^
int FLAGSHTOKGAME = 1; // handlers are very cool
int FLAGSHTOKSHOP = 1; // maybe they are ignoring return 0 ???
int FLAGSHTOKINGAMEMENU = 1; // more and more of them.. fuck
int FLAGSHTOKALLCOMPS = 1; //maybe the last one
// options flags
int IMMEDIATETRAECTORY = 0; //immediate traectory drawing, 1 - yes, 0 - no
int IMMEDIATEDROP = 1; //immediate drop of tanks in the begin of round
int REFLECTWALLS = 1; //reflect from side walls (1 - reflects, 0 - no(shot appears on the other side of screen))
int ORIENTATION = 2; //orientation of screen
int BONUSFLAG = 1; //do we have bonuses in game? 0 - no
int WINDKOEF = 1; //koeff for wind (1 - normal wind, 0 - disabled)
int TRANSPARENTSIDEWALLS = 0; //can our shot fly off the side walls, 0 - cannot, 1 - can, if we have this =1 => REFLECTWALLS has no effect
int COMMANDBATTLES = 0; //does computer shoot to other computers (=0) or only to humans (=1)

infoplayer IPLAYER[8]; // we have maximum 8 players
bonusstruct BONUSES[2]; // we have maximum 2 bonuses on the field
hiscoresstruct HISCORES[10]; //we have 10 hiscores
spritestruct poup = { //pointer up
	40, 19, 
	{
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,
		0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
		0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
		0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
		0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,
		0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
		0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
		0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
		0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
		0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};


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
void save_options();
void load_options();
void load_game();
void save_game();
void load_hiscores();
void save_hiscores();
void recalc_hiscores();
void draw_filltriangle (int x1, int y1, int x2, int y2, int x3, int y3, int color, int invert);
void draw_invert_sprite (int x, int y, spritestruct *sprite);
void draw_touchpointer (int x, int y, int w, int h, int dirc, int color);
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
void show_parabollic_traectory (int numof, int *xstart, int *ystart, int *powstart, int *angstart, int *xret, int *yret, int playa, int update);
void calc_best_fire (int *retv, int *retalpha, int playa);
void return_coordinates (int *retx, int *rety, float x0, float y0, float v0, float alpha);
void float_return_coordinates (int *retx, int *rety, float x0, float y0, float v0, float alpha, int draw);
void explosion (int numof, int *x, int *y, int r, int playa);
void draw_circle (int x, int y, int r, int color);
void fall_landscape ();
void show_roll_traectory (int *retx, int *rety, int x, int y);
void main_fire (int playa, int weapon);
void explode_dead ();
void walking (int *xret, int *yret, int playa);
void laser (int playa);
void show_winner(int playa);
int winner_handler(int type, int par1, int par2);
int allcomps_handler(int type, int par1, int par2);
void show_names_of_players(int playa);
void set_bonuses_in_game();
void show_bonuses_in_game();
void show_inside_bonus (int numofbonus, int numofweapon);

void temp_return_coordinates (float x0, float y0, float v0, float alpha);
// end of gameplay functions

// menu and redactor functions
void main_draw (int xl, int yl, int hgt);
void init_menu_choice (int xl, int yl, int wdt, int hgt, int posmenu);
void init_mainmenu_choice (int xl, int yl, int hgt, int posmenu, int *mainwd);
void menu_update (int xl, int yl, int wdt, int hgt, int posmenu, int posmenupr);
void mainmenu_update (int xl, int yl, int hgt, int posmenu, int posmenupr, int *mainwd);
void redraw_choice_menu (int xl, int yl, int hgt, int posmenu, int *mainwd);
int main_handler (int type, int par1, int par2);

int instruction_handler(int type, int par1, int par2);
void show_instruction (int x, int y, int w, int h, int page);

int start_handler(int type, int par1, int par2);
int validate_start();
void redraw_players_menu (int xl, int yl, int wdt, int hgt, int posmenu);
int edit_player_handler (int type, int par1, int par2);
char return_key (int num);
void draw_keyboard (int xp, int yp);
void init_keyboard_kursor (int xp, int yp, int wkl, int hkl, int posx, int posy);
void update_keyboard_kursor (int xp, int yp, int wkl, int hkl, int posx, int posy, int posxpr, int posypr);
void redraw_player_name (int xp, int yp);

int options_handler (int type, int par1, int par2);
void show_options_init ();
void draw_options_menu (int x, int y, int h, int posmenu, int *wd);

int hiscores_handler(int type, int par1, int par2);
// end of menu and redactor functions

//////////////////////////////////////////////////////////////////////
// STUFF FUNCTIONS
//////////////////////////////////////////////////////////////////////

// recalcs hiscores and add players to table
void recalc_hiscores() {
	int i,j,k;
	
	for (i=0; i<PLAYERS; i++) {
		for (j=0; j<10; j++) {
			if (strcmp (NPLAYERS[i], HISCORES[j].name) == 0 && IPLAYER[i].wins == HISCORES[j].wins) break;
			if (IPLAYER[i].wins > HISCORES[j].wins) {
				for (k=8; k>=j; k--) {
					strcpy (HISCORES[k+1].name, HISCORES[k].name);
					HISCORES[k+1].wins = HISCORES[k].wins;
					HISCORES[k+1].cplayer = HISCORES[k].cplayer;
				}
				strcpy (HISCORES[j].name, NPLAYERS[i]);
				HISCORES[j].wins = IPLAYER[i].wins;
				HISCORES[j].cplayer = IPLAYER[i].cplayer;
				break;			
			}
		}
	}
}

// saves hiscores
void save_hiscores() {
	FILE *fl;
	int i;
	
	fl = fopen ("/mnt/ext1/system/tankwar/tankwar.personal", "w");
	if (fl == NULL) fprintf (stderr, "ERROR opening tankwar.personal");
	else for (i=0; i<10; i++) fprintf(fl, "%s %i %i\n", HISCORES[i].name, HISCORES[i].cplayer, HISCORES[i].wins);
	fclose(fl);
	
	fl = fopen ("/mnt/ext1/system/tankwar/tankwar.comand", "w");
	if (fl == NULL) fprintf (stderr, "ERROR opening tankwar.comand");
	else {
		fprintf(fl, "%s %i\n", "COMPWINS", COMPWINS);
		fprintf(fl, "%s %i\n", "HUMANWINS", HUMANWINS);		
	}
	fclose(fl);
}

//load hiscores
void load_hiscores() {
	FILE *fl;
	int i;
	char temp[50];
	
	fl = fopen ("/mnt/ext1/system/tankwar/tankwar.personal", "r");
	if (fl == NULL) fprintf (stderr, "ERROR opening file");
	else for (i=0; i<10; i++) fscanf(fl, "%s %i %i", HISCORES[i].name, &HISCORES[i].cplayer, &HISCORES[i].wins);
	fclose(fl);
	
	fl = fopen ("/mnt/ext1/system/tankwar/tankwar.comand", "r");
	if (fl == NULL) fprintf (stderr, "ERROR opening file");
	else {
		fscanf (fl, "%s %i", temp, &COMPWINS);
		fscanf (fl, "%s %i", temp, &HUMANWINS);
	}
	fclose(fl);
	
}

// saves options to file
void save_options() {
	FILE *fl;
	
	fl = fopen ("/mnt/ext1/system/tankwar/tankwar.ini", "w");
	if (fl == NULL) fprintf (stderr, "ERROR opening file");
	else {
		fprintf (fl, "%s %i\n", "IMMEDIATETRAECTORY", IMMEDIATETRAECTORY);
		fprintf (fl, "%s %i\n", "IMMEDIATEDROP", IMMEDIATEDROP);
		fprintf (fl, "%s %i\n", "REFLECTWALLS", REFLECTWALLS);
		fprintf (fl, "%s %i\n", "ORIENTATION", ORIENTATION);
		fprintf (fl, "%s %i\n", "BONUSFLAG", BONUSFLAG);
		fprintf (fl, "%s %i\n", "WINDKOEF", WINDKOEF);
		fprintf (fl, "%s %i\n", "TRANSPARENTSIDEWALLS", TRANSPARENTSIDEWALLS);
		fprintf (fl, "%s %i\n", "COMMANDBATTLES", COMMANDBATTLES);
		fprintf (fl, "%s %i\n", "DIFFICUILTY", DIFFICUILTY);
		fprintf (fl, "%s %i\n", "PLAYERS", PLAYERS);
	}
	fclose(fl);
}

void load_options() {
	FILE *fl;
	char temp[50];
	int tmp;
	int i,j;
	
	tmp = umask(0);
	if ((tmp = mkdir("/mnt/ext1/system/tankwar", MY_MASK)) == 0) {
		// if we created directory (tmp == 0), it was not here before and we must create default tankwar.ini and tankwar.sav, tankwar.comand and tankwar.personal
		fl = fopen ("/mnt/ext1/system/tankwar/tankwar.ini", "w");
		fprintf (fl, "%s %i\n", "IMMEDIATETRAECTORY", IMMEDIATETRAECTORY);
		fprintf (fl, "%s %i\n", "IMMEDIATEDROP", IMMEDIATEDROP);
		fprintf (fl, "%s %i\n", "REFLECTWALLS", REFLECTWALLS);
		fprintf (fl, "%s %i\n", "ORIENTATION", ORIENTATION);
		fprintf (fl, "%s %i\n", "BONUSFLAG", BONUSFLAG);
		fprintf (fl, "%s %i\n", "WINDKOEF", WINDKOEF);
		fprintf (fl, "%s %i\n", "TRANSPARENTSIDEWALLS", TRANSPARENTSIDEWALLS);
		fprintf (fl, "%s %i\n", "COMMANDBATTLES", COMMANDBATTLES);
		fprintf (fl, "%s %i\n", "DIFFICUILTY", DIFFICUILTY);
		fprintf (fl, "%s %i\n", "PLAYERS", PLAYERS);
		fclose(fl);
		
		fl = fopen ("/mnt/ext1/system/tankwar/tankwar.sav", "w");
		fprintf(fl, "%i ", PLAYERS);
//		fprintf(fl, "%i ", DIFFICUILTY);
		fprintf(fl, "%i ", WIND);
//		fprintf(fl, "%i ", IMMEDIATETRAECTORY);
//		fprintf(fl, "%i ", IMMEDIATEDROP);
//		fprintf(fl, "%i ", REFLECTWALLS);
		fprintf(fl, "%i ", ORIENTATION);
//		fprintf(fl, "%i ", BONUSFLAG);
//		fprintf(fl, "%i ", WINDKOEF);
//		fprintf(fl, "%i ", TRANSPARENTSIDEWALLS);
		fprintf(fl, "%i ", COMMANDBATTLES);
		fprintf(fl, "%i ", TURNFORSAVE);
		fprintf(fl, "%i ", PHASEFORSAVE);
		for (i=0; i<800; i++) fprintf (fl, "%i ", ARENA[i]);
		for (i=0; i<8; i++) {
			fprintf (fl, "%s ", NPLAYERS[i]);
			fprintf (fl, "%i ", IPLAYER[i].cplayer);
			for (j=0; j<13; j++) fprintf (fl, "%i ", IPLAYER[i].ammo[j]);
			fprintf (fl, "%i ", IPLAYER[i].angle);
			fprintf (fl, "%i ", IPLAYER[i].power);
			fprintf (fl, "%i ", IPLAYER[i].life);
			fprintf (fl, "%i ", IPLAYER[i].x);
			fprintf (fl, "%i ", IPLAYER[i].y);
			fprintf (fl, "%i ", IPLAYER[i].dead);
			fprintf (fl, "%i ", IPLAYER[i].money);
			fprintf (fl, "%i ", IPLAYER[i].wins);
		}
		fclose(fl);
		
		fl = fopen ("/mnt/ext1/system/tankwar/tankwar.comand", "w");
		fprintf(fl, "%s %i\n", "COMPWINS", COMPWINS);
		fprintf(fl, "%s %i\n", "HUMANWINS", HUMANWINS);
		fclose(fl);
		
		fl = fopen ("/mnt/ext1/system/tankwar/tankwar.personal", "w");
		for (i=0; i<10; i++) fprintf(fl, "%s %i %i\n", HISCORES[i].name, HISCORES[i].cplayer, HISCORES[i].wins);
		fclose(fl);
	}
	else {
		fl = fopen ("/mnt/ext1/system/tankwar/tankwar.ini", "r");
		if (fl == NULL) fprintf (stderr, "ERROR opening file");
		else fscanf (fl, "%s %i %s %i %s %i %s %i %s %i %s %i %s %i %s %i %s %i %s %i", temp, &IMMEDIATETRAECTORY, temp, &IMMEDIATEDROP, temp, &REFLECTWALLS, temp, &ORIENTATION, temp, &BONUSFLAG, temp, &WINDKOEF, temp, &TRANSPARENTSIDEWALLS, temp, &COMMANDBATTLES, temp, &DIFFICUILTY, temp, &PLAYERS);
		fclose(fl);		
	}
}

void load_game() {
	FILE *fl;
	int i,j;
	int temp;
	
	fl = fopen ("/mnt/ext1/system/tankwar/tankwar.sav", "r");
	if (fl == NULL) fprintf (stderr, "ERROR opening file");
	else { 
		fscanf(fl, "%i", &PLAYERS);
//		fscanf(fl, "%i", &DIFFICUILTY);
		fscanf(fl, "%i", &WIND);
//		fscanf(fl, "%i", &IMMEDIATETRAECTORY);
//		fscanf(fl, "%i", &IMMEDIATEDROP);
//		fscanf(fl, "%i", &REFLECTWALLS);
		fscanf(fl, "%i", &ORIENTATION);
//		fscanf(fl, "%i", &BONUSFLAG);
//		fscanf(fl, "%i", &WINDKOEF);
//		fscanf(fl, "%i", &TRANSPARENTSIDEWALLS);
		fscanf(fl, "%i", &COMMANDBATTLES);		
		fscanf(fl, "%i", &TURNFORSAVE);
		fscanf(fl, "%i", &PHASEFORSAVE);
		for (i=0; i<800; i++) { fscanf (fl, "%i", &temp); ARENA[i] = temp; }
		for (i=0; i<8; i++) {
			fscanf (fl, "%s", NPLAYERS[i]);
			fscanf (fl, "%i", &IPLAYER[i].cplayer);
			for (j=0; j<13; j++) fscanf (fl, "%i", &IPLAYER[i].ammo[j]);
			fscanf (fl, "%i", &IPLAYER[i].angle);
			fscanf (fl, "%i", &IPLAYER[i].power);
			fscanf (fl, "%i", &IPLAYER[i].life);
			fscanf (fl, "%i", &IPLAYER[i].x);
			fscanf (fl, "%i", &IPLAYER[i].y);
			fscanf (fl, "%i", &IPLAYER[i].dead);
			fscanf (fl, "%i", &IPLAYER[i].money);
			fscanf (fl, "%i", &IPLAYER[i].wins);
		}
	}
	fclose(fl);
}

void save_game() {
	FILE *fl;
	int i,j;
	
	fl = fopen ("/mnt/ext1/system/tankwar/tankwar.sav", "w");
	if (fl == NULL) fprintf (stderr, "ERROR opening file");
	else { 
		fprintf(fl, "%i ", PLAYERS);
//		fprintf(fl, "%i ", DIFFICUILTY);
		fprintf(fl, "%i ", WIND);
//		fprintf(fl, "%i ", IMMEDIATETRAECTORY);
//		fprintf(fl, "%i ", IMMEDIATEDROP);
//		fprintf(fl, "%i ", REFLECTWALLS);
		fprintf(fl, "%i ", ORIENTATION);
//		fprintf(fl, "%i ", BONUSFLAG);
//		fprintf(fl, "%i ", WINDKOEF);
//		fprintf(fl, "%i ", TRANSPARENTSIDEWALLS);
		fprintf(fl, "%i ", COMMANDBATTLES);
		fprintf(fl, "%i ", TURNFORSAVE);
		fprintf(fl, "%i ", PHASEFORSAVE);
		for (i=0; i<800; i++) fprintf (fl, "%i ", ARENA[i]);
		for (i=0; i<8; i++) {
			fprintf (fl, "%s ", NPLAYERS[i]);
			fprintf (fl, "%i ", IPLAYER[i].cplayer);
			for (j=0; j<13; j++) fprintf (fl, "%i ", IPLAYER[i].ammo[j]);
			fprintf (fl, "%i ", IPLAYER[i].angle);
			fprintf (fl, "%i ", IPLAYER[i].power);
			fprintf (fl, "%i ", IPLAYER[i].life);
			fprintf (fl, "%i ", IPLAYER[i].x);
			fprintf (fl, "%i ", IPLAYER[i].y);
			fprintf (fl, "%i ", IPLAYER[i].dead);
			fprintf (fl, "%i ", IPLAYER[i].money);
			fprintf (fl, "%i ", IPLAYER[i].wins);
		}
	}
	fclose(fl);	
}

// draws filled with color triangle, invert - will we use InvertAreaBW (=1) or not (=0)
void draw_filltriangle (int x1, int y1, int x2, int y2, int x3, int y3, int color, int invert) {
	int xt[3], yt[3]; //sorted x,y
	int nummin=0, nummax=0, i;
	float sy;
	float xd1, xd2, tmp=0;
	
	yt[0] = minint (y1, minint(y2, y3));
	if (yt[0] == y1) { xt[0] = x1; nummin = 1; }
	if (yt[0] == y2) { xt[0] = x2; nummin = 2; }
	if (yt[0] == y3) { xt[0] = x3; nummin = 3; }
	yt[2] = maxint (y1, maxint(y2, y3));
	if (yt[2] == y1) { xt[2] = x1; nummax = 1; }
	if (yt[2] == y2) { xt[2] = x2; nummax = 2; }
	if (yt[2] == y3) { xt[2] = x3; nummax = 3; }
	for (i=1; i<=3; i++) {
		if (i != nummin && i != nummax) {
			if (i == 1) { yt[1] = y1; xt[1] = x1; }
			if (i == 2) { yt[1] = y2; xt[1] = x2; }
			if (i == 3) { yt[1] = y3; xt[1] = x3; }
		}
	}
	for (sy = (float)yt[0]; sy <= yt[2]; sy=sy+1) {
		xd1 = (float)(xt[0] + (sy-yt[0])*(xt[2]-xt[0])/(yt[2] - yt[0]));
		if (sy < yt[1]) xd2 = (float)(xt[0] + (sy-yt[0])*(xt[1]-xt[0])/(yt[1]-yt[0]));
		else {
			if (yt[2] == yt[1]) xd2 = (float)xt[1];
			else xd2 = (float)(xt[1] + (sy-yt[1])*(xt[2]-xt[1])/(yt[2]-yt[1]));
		}
	if (xd1 > xd2) { tmp = xd1; xd1 = xd2; xd2 = tmp; }
	if (invert == 0) DrawLine (fround(xd1), fround(sy), fround(xd2), fround(sy), color);
	else InvertAreaBW (minint (fround(xd1), fround(xd2)), fround(sy), maxint(fround(xd1), fround(xd2)) - minint(fround(xd1), fround(xd2)), 1);
	}

}

// draws rectangle with pointer inside
// dirc - direction, =0 - up, =1 - right, =2 - down, =3 - left
void draw_touchpointer (int x, int y, int w, int h, int dirc, int color) {
	FillArea (x, y, w, h, WHITE);
	DrawRect (x, y, w, h, color);
	DrawPixel (x, y, WHITE);
	DrawPixel (x+w-1, y, WHITE);
	DrawPixel (x+w-1, y+h-1, WHITE);
	DrawPixel (x, y+h-1, WHITE);
	if (dirc == 0) {
		draw_filltriangle (x+w/2, y+3, x+3, y+h/2, x+w-3, y+h/2, color, 0);
		draw_filltriangle (x+w/3, y+h/2, x+2*w/3, y+h/2, x+w/3, y+h-3, color, 0);
		draw_filltriangle (x+2*w/3, y+h/2, x+w/3, y+h-3, x+2*w/3, y+h-3, color, 0);
	}
	if (dirc == 1) {
		draw_filltriangle (x+w-3, y+h/2, x+w/2, y+3, x+w/2, y+h-3, color, 0);
		draw_filltriangle (x+w/2, y+h/3, x+w/2, y+2*h/3, x+3, y+h/3, color, 0);
		draw_filltriangle (x+w/2, y+2*h/3, x+3, y+h/3, x+3, y+2*h/3, color, 0);
	}
	if (dirc == 2) {
		draw_filltriangle (x+w/2, y+h-3, x+3, y+h/2, x+w-3, y+h/2, color, 0);
		draw_filltriangle (x+w/3, y+h/2, x+2*w/3, y+h/2, x+w/3, y+3, color, 0);
		draw_filltriangle (x+2*w/3, y+h/2, x+w/3, y+3, x+2*w/3, y+3, color, 0);
	}
	if (dirc == 3) {
		draw_filltriangle (x+3, y+h/2, x+w/2, y+3, x+w/2, y+h-3, color, 0);
		draw_filltriangle (x+w/2, y+h/3, x+w/2, y+2*h/3, x+w-3, y+h/3, color, 0);
		draw_filltriangle (x+w/2, y+2*h/3, x+w-3, y+h/3, x+w-3, y+2*h/3, color, 0);		
	}
}

//draws invertBW sprite
void draw_invert_sprite (int x, int y, spritestruct *sprite) {
	int i,j;
	for (j=0; j<sprite->h; j++) {
		for (i=0; i<sprite->w; i++) {
			if (sprite->data[j*sprite->w+i] == 1) InvertAreaBW(x+i, y+j, 1, 1);
//			if (sprite->data[j*sprite->w+i] == 1) DrawPixel(x+i, y+j, BLACK);
		}
	}
}

//make from 1-byte color 3-bytes for DrawPixel, DrawLine etc.
unsigned long ret_4num (unsigned char byte) {
	return (byte*256*256+byte*256+byte);
}

//draws tru 16-color bitmap
void draw_trubitmap(int x, int y, trubitmap *pic) {
	int i,j;
	int colleft;
	int colright;
	
	for (i=0; i<pic->height; i++) {
		for (j=0; j<(pic->width)/2; j++) {
			colleft = pic->data[(pic->width)/2*i+j]/16;
			colright = pic->data[(pic->width)/2*i+j]%16;
			DrawPixel (x+j*2, y+(pic->height-i), ret_4num(colleft*16));
			DrawPixel (x+j*2+1, y+(pic->height-i), ret_4num(colright*16));
			
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

// fill strn with string representation of number [0..100000], ends with '\0'
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
	if (i > 9999 && i < 100000 ) k=5;
	
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
	char strwins[50];
	char strtemp[10];
	int i;
	
	FillArea (451, 101, 58, 500, WHITE);
	FillArea (511, 101, 275, 500, WHITE);
	
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
	
	strcpy(strwins, "Побед - ");
	strcpy(strtemp, "");
	itos (strtemp, IPLAYER[playa-1].wins);
	strcat (strwins, strtemp);
	
	DrawTextRect (511, 220, 289, 200, strwins, ALIGN_CENTER);
		
	if (update == 1) PartialUpdateBW (451, 101, 335, 500);
}

void shop_init_draw () {
	int widweapon=35, xo=30, yh=110;
	int prices[13] = {0, 10, 20, 30, 50, 70, 70, 80, 80, 110, 300, 250, 400};
	int i;
	char strprice[10];
	
	ClearScreen();
//	DrawRect (0,0,800, 100, BLACK);
	draw_trubitmap(0,0,&salespic);
	for (i=0; i<=13; i++) DrawLine(i,i,799-i,i,ret_4num(16*(13-i)));
	for (i=0; i<=13; i++) DrawLine(i,i,i,599,ret_4num(16*(13-i)));
	for (i=0; i<=13; i++) DrawLine(799-i,i,799-i,599,ret_4num(16*(13-i)));
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
	DrawString (xo+50, 35*13+yh, "ОК");
	
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
	int xmenu=15, ymenu=107, wmenu=425, hmenu=35;
	int prices[13] = {0, 10, 20, 30, 50, 70, 70, 80, 80, 110, 300, 250, 400};
	int dmoney;
	int compmenu;
	int i;
		
	if (type == EVT_SHOW && FLAGSHTOKSHOP == 1) {
		if (GAMESHOPBEGINNING == 1) {
			shop_init_draw();
			shop_player_info(playa, 0);
			init_menu_choice (xmenu, ymenu, wmenu, hmenu, posmenu);
			for (i=0; i<PLAYERS; i++) {
				if (IPLAYER[i].ammo[1] < 3) IPLAYER[i].ammo[1] = 3;
				if (IPLAYER[i].ammo[2] < 2) IPLAYER[i].ammo[2] = 2;
			}
			FullUpdate();
			FineUpdate();
		}
		if (GAMESHOPBEGINNING == 0) {
			shop_player_info(playa, 1);
			init_menu_choice (xmenu, ymenu, wmenu, hmenu, posmenu);
		}
	}
	while (IPLAYER[playa-1].cplayer == 1 && FLAGSHTOKSHOP == 1) {
		shop_init_draw();
		shop_player_info(playa, 0);
		PartialUpdateBW (0,0,800,600);
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
			if (playa == 2) init_menu_choice (xmenu, ymenu, wmenu, hmenu, posmenu);	// fuck, this == 2 is working, but should work without it, but it isn't, if first buyer is comp, and second - human
		}
	}
	if (type == EVT_POINTERUP && FLAGSHTOKSHOP == 1 && par2 >= ymenu + hmenu && par2 <= ymenu + 14*hmenu && par1 >= xmenu && par1 <= xmenu + wmenu) {
		posmenutmp = posmenu;
		posmenu = 1 + (par2-ymenu) / hmenu;
		menu_update (xmenu, ymenu, wmenu, hmenu, posmenu, posmenutmp);
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
				for (i=0; i<PLAYERS; i++) {
					if (IPLAYER[i].ammo[1] < 3) IPLAYER[i].ammo[1] = 3;
					if (IPLAYER[i].ammo[2] < 2) IPLAYER[i].ammo[2] = 2;
				}
				for (i=0;i<2;i++) { BONUSES[i].x=0; BONUSES[i].y=0; }
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
					for (i=0; i<PLAYERS; i++) {
						if (IPLAYER[i].ammo[1] < 3) IPLAYER[i].ammo[1] = 3;
						if (IPLAYER[i].ammo[2] < 2) IPLAYER[i].ammo[2] = 2;
					}
					for (i=0;i<2;i++) { BONUSES[i].x=0; BONUSES[i].y=0; }
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
	koef2 = (float)(rand()%70+185);
	koef3 = (float)(rand()%250+50);
	koef4 = (float)(rand()%250+50);
	koef5 = (float)(rand()%250+50);
	koef6 = (float)(rand()%250+50);
	koef7 = (float)(rand()%250+50);
	for (i=0; i<800; i++) {
		t = 340+koef2*sin((float)i/koef1)*cos((float)i/koef3)*cos((float)i/koef4)*sin((float)i/koef5);
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
	float c=20, alpha = M_PI/20; //lenght and angle of pointer
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
	
/*	for (cycdraw=-alpha; cycdraw <= 1.5*alpha; cycdraw=cycdraw+M_PI/180) {
		y1str = (float)(y2+c*sin(truang+cycdraw));
		x1str = (float)(x2-c*cos(truang+cycdraw));
		DrawLine (x2, y2, fround(x1str), fround(y1str), BLACK);
	}
*/
	y1str = (float)(y2+c*sin(truang-alpha));
	x1str = (float)(x2-c*cos(truang-alpha));
	draw_filltriangle (fround(x1str), fround(y1str), x2, y2, fround((float)(x2-c*cos(truang+alpha))), fround((float)(y2+c*sin(truang+alpha))), BLACK, 0);
	
	coss = cos(truang);
	sinn = sin(truang);
	cycdraw = 1.5;
	draw_filltriangle (x1-fround(sinn*cycdraw), y1-fround(coss*cycdraw), x2, y2, x1+fround(sinn*cycdraw), y1+fround(coss*cycdraw), BLACK, 0);
/*	for (cycdraw=-1.4; cycdraw <= 1.4; cycdraw=cycdraw+0.2) {
		DrawLine (x1+fround(sinn*cycdraw), y1+fround(coss*cycdraw), x2, y2, BLACK);
	}
	DrawLine (x1, y1, x2, y2, BLACK);
*/
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

// shows all player's info (dont forget to call clear_info_panel() before that!). N of player [0..7], NOT [1..8]
void show_player_info(int playa) {
	int i;
	char strammo[8];
	char strangpower[8];
	char strwind[8];
	char strlife[30];
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
	strcpy (strlife, "Чел: ");
	strcpy (tempstrlife, "");
	itos (tempstrlife, IPLAYER[playa].life);
	strcat (strlife, tempstrlife);
	DrawTextRect (646, 31, 74, 20, strlife, ALIGN_CENTER);
	
	FillArea (723, 33, fround((float)(0.74*IPLAYER[playa].life)), 14, BLACK); //Lifebar
	
	strcpy(strmoney, "");
	itos (strmoney, IPLAYER[playa].money);
	//strcat(strmoney, "$");
	SetFont(OpenFont("LiberationSans", 16, 0), BLACK);
	DrawTextRect (647, -1, 47, 24, "$$", ALIGN_CENTER);
	DrawTextRect (647, 14, 47, 24, strmoney, ALIGN_CENTER);
	
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
	
	DrawTextRect (505, 30, 50, 20, "Угол", ALIGN_CENTER);
	
	DrawLine (605, 0, 605, 50, BLACK);
	DrawLine (555, 30, 605, 30, BLACK);
	
	DrawTextRect (555, 30, 50, 20, "Сила", ALIGN_CENTER);
	
	DrawLine (645, 0, 645, 50, BLACK);
	DrawLine (605, 30, 800, 30, BLACK);
	DrawLine (605, 15, 645, 15, BLACK);
	DrawLine (720, 30, 720, 50, BLACK);
	
	DrawLine (695, 0, 695, 30, BLACK);
	
	DrawTextRect (606, 30, 40, 20, "Вет.", ALIGN_CENTER);
			
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
	DrawLine (x-10, y, x+10, y, WHITE);
	DrawLine (x-11, y-1, x-11, y-5, WHITE);
	DrawLine (x+11, y-1, x+11, y-5, WHITE);
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
		if (IPLAYER[numofplayer-1].life > 0) {
			xofplayer = 30+(numofplayer-1)*740/(PLAYERS-1);
			temp = (ARENA[xofplayer]-60)/5;
			if (IMMEDIATEDROP == 0) {
				for (j=0; j<6; j++) {
					yofplayer = 63+j*temp;
					if (j > 0) FillArea (xofplayer-10, yofplayer-11-temp, 21, 11, WHITE);
					draw_tank (xofplayer, yofplayer);
					PartialUpdateBW (xofplayer-10, yofplayer-11-temp, 21, 11+temp);
					t.tv_nsec = 100000000;
					nanosleep(&t, &tret);
				}
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
	}
	//debug
	FillArea (0, 55, 800, 545, WHITE);
	show_arena();
	show_tanks();
//	draw_tank_wbarrel(1);
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
	xb2 = xb1 + fround((float)(l*cos(truang)));
	yb2 = yb1 - fround((float)(l*sin(truang)));
/*	DrawLine (xb1, yb1, xb2, yb2, BLACK);
	DrawLine (xb1-fround(sin(truang)*c), yb1-fround(cos(truang)*c), xb2-fround(sin(truang)*c), yb2-fround(cos(truang)*c), BLACK);
	DrawLine (xb1+fround(sin(truang)*c), yb1+fround(cos(truang)*c), xb2+fround(sin(truang)*c), yb2+fround(cos(truang)*c), BLACK);	
*/
	draw_filltriangle (xb1-fround(sin(truang)*c), yb1-fround(cos(truang)*c), xb2-fround(sin(truang)*c), yb2-fround(cos(truang)*c), xb2+fround(sin(truang)*c), yb2+fround(cos(truang)*c), BLACK, 0);
	draw_filltriangle (xb1+fround(sin(truang)*c), yb1+fround(cos(truang)*c), xb2+fround(sin(truang)*c), yb2+fround(cos(truang)*c), xb1-fround(sin(truang)*c), yb1-fround(cos(truang)*c), BLACK, 0);
	show_partial_arena (IPLAYER[num-1].x, l);
	draw_tank (IPLAYER[num-1].x, IPLAYER[num-1].y);
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

// function for float check of int calculations of comp's firing
// draw - will we draw traectory? =1 - yes, =0 - no
void float_return_coordinates (int *retx, int *rety, float x0, float y0, float v0, float alpha, int draw) {
	float t=0;
	float xt, yt;
	float w, g=9.8;
	float coss, sinn;
	float vch;// speed at time of impact the wall
	float dt=0.1;
	int cnt=0; //debug
	int flag = 1;
//	float tempv0=v0;
	
	w = -(float)WIND/100;
	coss = cos(alpha);
	sinn = sin(alpha);
	
//	while (flabs(w*dt*dt/2-v0*dt*coss) + flabs(v0*dt*sinn-g*dt*dt/2) > 2) dt = dt/2;
	while (flabs(w*dt*dt/2-v0*dt*coss) + flabs(v0*dt*sinn-g*dt*dt/2) > 4) dt = dt/2;
	
	while ( flag == 1 ) {
		xt = x0 + v0*t*coss - w*t*t/2;
		yt = y0 - v0*t*sinn + g*t*t/2;
//		while (flabs(w*dt*dt/2-v0*dt*coss+w*t*dt) + flabs (v0*dt*sinn-g*t*dt-g*dt*dt/2) > 2) {
//			dt = dt/2;
//		}
		if ((xt > 799 || xt < 0) && REFLECTWALLS == 1 && TRANSPARENTSIDEWALLS == 0) {
			if (xt < 0) xt = 0;
			if (xt > 799) xt = 799;
			vch = sqrt((v0*coss - w*t)*(v0*coss - w*t) + (g*t - v0*sinn)*(g*t - v0*sinn));
			sinn = -(g*t - v0*sinn)/vch;
			coss = -(v0*coss - w*t)/vch;
			x0 = xt;
			y0 = yt;
			v0 = vch;
			t = 0;
			xt = x0;
			yt = y0;
		}
		if ((xt > 799 || xt < 0) && REFLECTWALLS == 0 && TRANSPARENTSIDEWALLS == 0) {
			vch = sqrt((v0*coss-w*t)*(v0*coss-w*t) + (g*t-v0*sinn)*(g*t-v0*sinn));
			sinn = -(g*t-v0*sinn)/vch;
			coss = (v0*coss-w*t)/vch;
			x0 = 800-xt;
			if (x0 > 799) x0 = 799;
			if (x0 < 0) x0 = 0;
			y0 = yt;
			v0 = vch;
			t = 0;
			xt = x0;
			yt = y0;
		}
		if (xt >= 0 && xt <=799 && t > 0) {
			if (fround(yt) >= ARENA[fround(xt)] || fround(yt) >= 599) { *retx = fround(xt); *rety = fround(yt); flag=0; }
		}
		if (fround (yt) >= 599) { *retx = fround(xt); *rety = fround(yt); flag = 0; }
//		if (flag == 0 && draw == 1) fprintf (stderr, "retx=%i, rety=%i, v=%i, alpha=%i, w=%f, g=%f\n", fround(xt), fround(yt), fround((float)tempv0*5), fround((float)alpha/M_PI*180), w, g);
		if (draw == 1) {
			DrawPixel (xt, yt, BLACK);
			if (cnt%50 == 0) PartialUpdateBW (0,55,800,545);
			cnt++;
		}
		t = t+dt;
	}
}

// returns final coordinates after firing from x0, y0 with speed=v0 and angle=alpha (for comp firing)
// now - in longint!
void return_coordinates (int *retx, int *rety, float x0, float y0, float v0, float alpha) {
	long intv0; //v0 for int calcs
	long coefv0 = 1;
//	float t=0;
//	float xt, yt; //float calcs, prev version, slow
	long t=0; //coefdt=coeft
	long xt, yt;
	long sx1, sx2, sx3;
	long sy1, sy2, sy3;
	long coefsx2, coefsx3, coefsy2, coefsy3;
	
//	float w, g=9.8;
	long intw, intg=98;
	long coefw = 10, coefg = 10;
	
	float coss, sinn;
	long intcoss, intsinn; //sin and cos for int calculations
	long coefcossin = 1000;
	
	float vch;// speed at time of impact the wall
//	float dt=0.1;
	long intdt; //dt for int calculations
	long coefdt = 100; //NOT FOR CHANGE!
	int flag = 1;
	
//	w = -(float)(WIND/100);

	coss = cos(alpha);
	sinn = sin(alpha);

//	while (flabs(w*dt*dt/2-v0*dt*coss) + flabs(v0*dt*sinn-g*dt*dt/2) > 4) dt = dt/2;
//	dt = dt*coefdt;
//	intdt = fround(dt);
	intdt = 1;

	intw = -WIND/(100/coefw);
//	intw = -WIND;
	
	v0 = (float)v0*coefv0;
	intv0 = v0;	
	
	coss = coefcossin*coss;
	sinn = coefcossin*sinn;
	intcoss = coss;
	intsinn = sinn;
	
	coefsx2 = coefv0*coefdt*coefcossin;
	coefsx3 = coefw*coefdt*coefdt;
	coefsy2 = coefv0*coefdt*coefcossin;
	coefsy3 = coefg*coefdt*coefdt;
	
	while ( flag == 1 ) {
//		xt = x0 + v0*t*coss - w*t*t/2;
//		yt = y0 - v0*t*sinn + g*t*t/2;
		sx1 = x0;
		sx2 = intv0*t*intcoss;
		sx3 = -intw*t*t/2;
		sy1 = y0;
		sy2 = -intv0*t*intsinn;
		sy3 = intg*t*t/2;
		xt = sx1 + sx2/coefsx2 + sx3/coefsx3;
		yt = sy1 + sy2/coefsy2 + sy3/coefsy3;
//		fprintf (stderr, "xt=%li, sx2=%li, sx3=%li, yt=%li, t=%li, intdt=%li, dt=%f\n", xt, sx2, sx3, yt, t, intdt, dt);
//		fprintf (stderr, "v0=%li, coss=%li, w=%li, sinn=%li, g=%li\n", intv0, intcoss, intw, intsinn, intg);
		if ((xt > 799 || xt < 0) && REFLECTWALLS == 1 && TRANSPARENTSIDEWALLS == 0){
			if (xt < 0) xt = 0;
			if (xt > 799) xt = 799;
			vch = (float)sqrt((intv0/coefv0 * intcoss/coefcossin - intw/coefw * t/coefdt)*(intv0/coefv0 * intcoss/coefcossin - intw/coefw * t/coefdt) + (intg/coefg * t/coefdt - intv0/coefv0 * intsinn/coefcossin)*(intg/coefg * t/coefdt - intv0/coefv0 * intsinn/coefcossin));
//			sinn = -(g*t-v0*sinn)/vch;
//			coss = -(v0*coss-w*t)/vch;
			sinn = (float)-coefcossin*(intg/coefg * t/coefdt - intv0/coefv0 * intsinn/coefcossin)/vch;
			coss = (float)-coefcossin*(intv0/coefv0 * intcoss/coefcossin - intw/coefw * t/coefdt)/vch;
			intsinn = sinn;
			intcoss = coss;
			x0 = xt;
			y0 = yt;
			v0 = vch*coefv0;
			intv0 = v0;
			t = 0;
			xt = x0;
			yt = y0;
		}
		if ((xt > 799 || xt < 0) && REFLECTWALLS == 0 && TRANSPARENTSIDEWALLS == 0){
//			vch = sqrt((v0*coss-w*t)*(v0*coss-w*t) + (g*t-v0*sinn)*(g*t-v0*sinn));
			vch = (float)sqrt((intv0/coefv0 * intcoss/coefcossin - intw/coefw * t/coefdt)*(intv0/coefv0 * intcoss/coefcossin - intw/coefw * t/coefdt) + (intg/coefg * t/coefdt - intv0/coefv0 * intsinn/coefcossin)*(intg/coefg * t/coefdt - intv0/coefv0 * intsinn/coefcossin));
//			sinn = -(g*t-v0*sinn)/vch;
//			coss = (v0*coss-w*t)/vch;
			sinn = (float)-coefcossin*(intg/coefg * t/coefdt - intv0/coefv0 * intsinn/coefcossin)/vch;
			coss = (float)coefcossin*(intv0/coefv0 * intcoss/coefcossin - intw/coefw * t/coefdt)/vch;
			intsinn = sinn;
			intcoss = coss;
			x0 = 800-xt;
			if (x0 > 799) x0 = 799;
			if (x0 < 0) x0 = 0;
			y0 = yt;
			v0 = vch*coefv0;
			intv0 = v0;
			t = 0;
			xt = x0;
			yt = y0;
		}
		if (xt >= 0 && xt <=799 && t > 0) {
//			if (fround(yt) >= ARENA[fround(xt)] || fround(yt) >= 599) { *retx = fround(xt); *rety = fround(yt); flag=0; }
			if (yt >= ARENA[xt] || yt >= 599) { *retx = xt; *rety = yt; flag=0; }
		}
//		if (fround (yt) >= 599) { *retx = fround(xt); *rety = fround(yt); flag = 0; }
		if (yt >= 599) { *retx = xt; *rety = yt; flag = 0; }
		t = t+intdt;
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
	int i,j,k;
	int dpow, dang;
	int *rv;
	int *ra;
	int flag_if_failed = 1; //flag for determining of failure of int calcs, =1 - if failed, =0 - otherwise
	// 1 - 
	// 2 - 
	// 3 - dpow = 50-((d-1)*10);
	rv = (int*)malloc(sizeof(int));
	ra = (int*)malloc(sizeof(int));
	dpow = 40-((DIFFICUILTY-1)*10);
	dang = 15-((DIFFICUILTY-1)*5);
	ang = 90; pow = 10;
	while (pow <= 990) {
		ang = 90;
		while (ang <= 180) {
			for (j=0; j<=1; j++) {
				return_coordinates (&xtemp, &ytemp, (float)IPLAYER[playa-1].x, (float)IPLAYER[playa-1].y-6, (float)(pow/5), (float)((ang-j*90)*M_PI/180));
				for (i=0; i<PLAYERS; i++) {
					if (i != playa-1 && IPLAYER[i].life > 0 && COMMANDBATTLES == 0) {
						if (xtemp >= IPLAYER[i].x-16-(3-DIFFICUILTY)*20 && xtemp <= IPLAYER[i].x+16+(3-DIFFICUILTY)*20 && ytemp <= IPLAYER[i].y+5+(3-DIFFICUILTY)*20 && ytemp >= IPLAYER[i].y-15-(3-DIFFICUILTY)*20 ) {
							cnt++;
							rv = (int*)realloc(rv, cnt*sizeof(int));
							ra = (int*)realloc(ra, cnt*sizeof(int));
							rv[cnt-1] = pow;
							ra[cnt-1] = ang-j*90;
//							fprintf (stderr, "ang=%i, pow=%i, xcalced=%i, ycalced=%i\n", ang-j*90, pow, xtemp, ytemp);
							if (cnt >= 6) { goto brcycle; }
						}
					}
					if (i != playa-1 && IPLAYER[i].life > 0 && COMMANDBATTLES == 1 && IPLAYER[i].cplayer == 0) {
						if (xtemp >= IPLAYER[i].x-16-(3-DIFFICUILTY)*20 && xtemp <= IPLAYER[i].x+16+(3-DIFFICUILTY)*20 && ytemp <= IPLAYER[i].y+5+(3-DIFFICUILTY)*20 && ytemp >= IPLAYER[i].y-15-(3-DIFFICUILTY)*20 ) {
							cnt++;
							rv = (int*)realloc(rv, cnt*sizeof(int));
							ra = (int*)realloc(ra, cnt*sizeof(int));
							rv[cnt-1] = pow;
							ra[cnt-1] = ang-j*90;
							if (cnt >= 6) { goto brcycle; }
						}
					}
				}
			}
			ang = ang + dang;
		}
		pow = pow + dpow;
	}
	brcycle: 
	if (cnt > 0) {
		rd = rand()%cnt;
		vtemp = rv[rd];
		atemp = ra[rd];
		*retv = vtemp;
		*retalpha = atemp;
		//check int calced traectory in float
		float_return_coordinates (&xtemp, &ytemp, (float)IPLAYER[playa-1].x, (float)IPLAYER[playa-1].y-6, (float)(vtemp/5), (float)(atemp*M_PI/180), 0);
		for (i=0; i<PLAYERS; i++) {
			if (i != playa-1 && IPLAYER[i].life > 0 && COMMANDBATTLES == 0) {
				if (xtemp >= IPLAYER[i].x-16-(3-DIFFICUILTY)*20 && xtemp <= IPLAYER[i].x+16+(3-DIFFICUILTY)*20 && ytemp <= IPLAYER[i].y+5+(3-DIFFICUILTY)*20 && ytemp >= IPLAYER[i].y-15-(3-DIFFICUILTY)*20 ) {
					flag_if_failed = 0;
					break;
				}
			}
			if (i != playa-1 && IPLAYER[i].life > 0 && COMMANDBATTLES == 1 && IPLAYER[i].cplayer == 0) {
				if (xtemp >= IPLAYER[i].x-16-(3-DIFFICUILTY)*20 && xtemp <= IPLAYER[i].x+16+(3-DIFFICUILTY)*20 && ytemp <= IPLAYER[i].y+5+(3-DIFFICUILTY)*20 && ytemp >= IPLAYER[i].y-15-(3-DIFFICUILTY)*20 ) {
					flag_if_failed = 0;
					break;
				}
			}
		}
		if (flag_if_failed == 1) {
			for (k=atemp-2; k<=atemp+2; k=k+1) {
				for (j=vtemp-20; j<=vtemp+20; j=j+10) {
					float_return_coordinates (&xtemp, &ytemp, (float)IPLAYER[playa-1].x, (float)IPLAYER[playa-1].y-6, (float)(j/5), (float)(k*M_PI/180), 0);
					for (i=0; i<PLAYERS; i++) {
						if (i != playa-1 && IPLAYER[i].life > 0 && COMMANDBATTLES == 0) {
							if (xtemp >= IPLAYER[i].x-16-(3-DIFFICUILTY)*20 && xtemp <= IPLAYER[i].x+16+(3-DIFFICUILTY)*20 && ytemp <= IPLAYER[i].y+5+(3-DIFFICUILTY)*20 && ytemp >= IPLAYER[i].y-15-(3-DIFFICUILTY)*20 ) {
								flag_if_failed = 0;
								if (j >=10 && j <= 990) *retv = j;
								if (k >= 0 && k <= 180) *retalpha = k;
								break;
							}
						}
						if (i != playa-1 && IPLAYER[i].life > 0 && COMMANDBATTLES == 1 && IPLAYER[i].cplayer == 0) {
							if (xtemp >= IPLAYER[i].x-16-(3-DIFFICUILTY)*20 && xtemp <= IPLAYER[i].x+16+(3-DIFFICUILTY)*20 && ytemp <= IPLAYER[i].y+5+(3-DIFFICUILTY)*20 && ytemp >= IPLAYER[i].y-15-(3-DIFFICUILTY)*20 ) {
								flag_if_failed = 0;
								if (j >=10 && j <= 990) *retv = j;
								if (k >= 0 && k <= 180) *retalpha = k;
								break;
							}
						}
					}
					if (flag_if_failed == 0) break;
				}
				if (flag_if_failed == 0) break;
			}
		}
//		for (i=0; i<cnt; i++) fprintf (stderr, "rv(%i)=%i, ra(%i)=%i\n", i, rv[i], i, ra[i]);
//		fprintf (stderr, "rd=%i, truang=%i, trupow=%i\n", rd, atemp, vtemp);
//		fprintf (stderr, "longlong=%i\n", sizeof(long long));
//		temp_return_coordinates(IPLAYER[playa-1].x, IPLAYER[playa-1].y-6, (float)vtemp*0.2, (float)atemp*M_PI/180);
	}
	if (cnt == 0) {
		*retv = 800;
		*retalpha = 90+(2*(rand()%2)-1)*45;
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
				if (BOOLARENA[j][i-1] == 0 && BOOLARENA[j][i] == 1) { flag = 1; break; }
			}
			if (flag == 1) break;
		}
		if (counter%20 == 0) PartialUpdateBW (xmin, 55, iabs(xmax-xmin)+1, 545);
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
	if (BONUSFLAG == 1) {
		for (i=0; i<2; i++) { 
			if (BONUSES[i].x != 0 && BONUSES[i].y != 0) {
				if (BONUSES[i].y != ARENA[BONUSES[i].x]-10) {
					FillArea(BONUSES[i].x-10, BONUSES[i].y-15, 20, 30, WHITE); 
					BONUSES[i].y = ARENA[BONUSES[i].x]-10; 
				}
			}
		}
	}
	FillArea (0,55,800,545,WHITE);
	show_arena();
	show_tanks();
	show_bonuses_in_game();
	show_names_of_players(-1);
	PartialUpdateBW (0, 55, 800, 545);
}

// shows explosion of weapon r [1..6,9,10,11,13] in x,y
// and change BOOLARENA
// add bonus if hit to playa, [1..PLAYERS]
void explosion (int numof, int *x, int *y, int r, int playa) {
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
	int bonusweapon; //if hit bonus this is weapon to add
	int x1upd=799, y1upd=599, x2upd=0, y2upd=0; //coords for partialupdate
	
	copy_ARENA_to_BOOLARENA();
	t.tv_sec = 0;
	if (r == 1)  { radexplosion = 10;  dr = 1; k = 1.5;}
	if (r == 2 || r == 5)  { radexplosion = 20; dr = 1; }
	if (r == 3 || r == 6 || r == 9)  { radexplosion = 30; dr = 2; }
	if (r == 4 || r == 13 || r == 10)  { radexplosion = 40; dr = 2; }
	if (r == 11) { radexplosion = 180;dr = 3; k = 0.3;}
	
	for (i = 1; i <= numof; i++) {
		if (x[i-1]-radexplosion < x1upd) { x1upd = x[i-1]-radexplosion; if (x1upd < 0) x1upd = 0; }
		if (x[i-1]+radexplosion > x2upd) { x2upd = x[i-1]+radexplosion; if (x2upd > 799) x2upd = 799; }
		if (y[i-1]-radexplosion < y1upd) { y1upd = y[i-1]-radexplosion; if (y1upd < 0) y1upd = 0; }
		if (y[i-1]+radexplosion > y2upd) { y2upd = y[i-1]+radexplosion; if (y2upd > 599) y2upd = 599; }
	}
	for (j = radexplosion/dr; j <= radexplosion; j=j+(radexplosion/dr)) {
		for (i=1; i<=numof; i++) draw_circle (x[i-1], y[i-1], j, BLACK);
		PartialUpdateBW (x1upd, y1upd, x2upd-x1upd, y2upd-y1upd);
		t.tv_nsec = 80000000; //nanoseconds, lol ))))))
		nanosleep(&t, &tret);
	}
	for (i=1; i<=numof; i++) draw_circle (x[i-1], y[i-1], radexplosion, WHITE);
	PartialUpdateBW (x1upd, y1upd, x2upd-x1upd, y2upd-y1upd);
	//recalc bonus if hit
	for (i=0; i<2; i++) {
		for (j=1; j<=numof; j++) {
			if (BONUSFLAG == 1 && BONUSES[i].x != 0 && BONUSES[i].y != 0) {
				rd = (float)radexplosion+15;
				l = (float)sqrt((BONUSES[i].x-x[j-1])*(BONUSES[i].x-x[j-1]) + (BONUSES[i].y-y[j-1])*(BONUSES[i].y-y[j-1]));
				if (l <= rd) {
					bonusweapon = rand()%9+2;
					show_inside_bonus (i, bonusweapon);
					FillArea(BONUSES[i].x-10, BONUSES[i].y-15, 20, 30, WHITE);
					PartialUpdateBW (BONUSES[i].x-15, BONUSES[i].y-45, 30, 60);
					t.tv_nsec = 500000000;
					nanosleep(&t, &tret);
					BONUSES[i].x = 0;
					BONUSES[i].y = 0;
					IPLAYER[playa-1].ammo[bonusweapon-1]++;
				}
			}
		}
	}
	// recalc lives
	for (i = 0; i < PLAYERS; i++) {
		minl = 0;
		for (j=1; j<=numof; j++) {
			strcpy (strminl, "");
			if (IPLAYER[i].life > 0) {
				rd = (float)1.5*radexplosion;
				l = (float)sqrt((IPLAYER[i].x-x[j-1])*(IPLAYER[i].x-x[j-1]) + (IPLAYER[i].y-y[j-1])*(IPLAYER[i].y-y[j-1]));
				if (l <= rd) minl = minl + (float)(20+rd*k)*(1-l/rd);
			}
		}
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
	//change BOOLARENA
	for (j=1; j<=numof; j++) {
		for (l=0; l<radexplosion; l=l+1) {
			xtemp = (float)(radexplosion*sqrt(1-l*l/(radexplosion*radexplosion)));
			for (i=x[j-1]-fround(xtemp); i <= x[j-1]+fround(xtemp); i++) {
				if (i >= 0 && i <= 799) {
					if (y[j-1]-(int)l >= 55) BOOLARENA[i][599-(y[j-1]-(int)l)]=0;
					if (y[j-1]+(int)l <= 599) BOOLARENA[i][599-(y[j-1]+(int)l)]=0;
				}
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
		if (counter%5 == 0) PartialUpdateBW (0, 55, 800, 545);
	}
	PartialUpdateBW (0, 55, 800, 545);
}

// function for showing playa traectory of fire, returns xret,yret - massive of final pixels
// numof - number of start points, xstart, ystart, powstart, angstart - massive of x,y,power,angle of start points
// playa - [1..PLAYERS]
// update: 1 - clear screen and redraw landscape + tanks = partialupdate, 0 - without clear and partialupdate
void show_parabollic_traectory (int numof, int *xstart, int *ystart, int *powstart, int *angstart, int *xret, int *yret, int playa, int update) {
	float t0=0, tglob=0;
	float tupd = 2.5; //if (t-t0 > tupd) => partialupdate
	float *t;
	float *x0, *y0, *v0, *alpha;
	float *xt, *yt, *xdt, *ydt;
	int *xpo, *ypo; //x,y for pointer if our shot is out of sight
	float w=-(float)(WIND*0.01), g=9.8, mindt=1;
	float *coss, *sinn;
	float vch;// speed at time of impact the wall
	float *dt;
	int glflag = 1, i, mcyc;
	int *flag;

	int maxARENA=0; //for partial update, not optimal, but less brainfucking
	
	t = (float*)malloc(numof*sizeof(float));
	x0 = (float*)malloc(numof*sizeof(float));
	y0 = (float*)malloc(numof*sizeof(float));
	v0 = (float*)malloc(numof*sizeof(float));
	alpha = (float*)malloc(numof*sizeof(float));
	xt = (float*)malloc(numof*sizeof(float));
	yt = (float*)malloc(numof*sizeof(float));
	xdt = (float*)malloc(numof*sizeof(float));
	ydt = (float*)malloc(numof*sizeof(float));
	coss = (float*)malloc(numof*sizeof(float));
	sinn = (float*)malloc(numof*sizeof(float));
	dt = (float*)malloc(numof*sizeof(float));
	flag = (int*)malloc(numof*sizeof(int));
	xpo = (int*)malloc(numof*sizeof(int));
	ypo = (int*)malloc(numof*sizeof(int));
/*
	x0 = (float)IPLAYER[playa-1].x;
	y0 = (float)IPLAYER[playa-1].y-6;
	v0 = (float)(IPLAYER[playa-1].power*0.18);
	alpha = (float)(IPLAYER[playa-1].angle*M_PI/180);
	coss = cos(alpha);
	sinn = sin(alpha);
	x0 = x0 + 15 * cos(alpha);
	y0 = y0 - 15 * sin(alpha);
*/	
//	w = -(float)(WIND/100);
	for (i=1; i<=numof; i++) {
		t[i-1] = 0;
		x0[i-1] = (float)xstart[i-1];
		y0[i-1] = (float)ystart[i-1];
		v0[i-1] = (float)(powstart[i-1]*0.2);
		alpha[i-1] = (float)(angstart[i-1]*M_PI/180);
		coss[i-1] = (float)(cos(alpha[i-1]));
		sinn[i-1] = (float)(sin(alpha[i-1]));
		flag[i-1] = 1;
		xpo[i-1] = -1;
		ypo[i-1] = -1;
		dt[i-1] = 0.1;
		while (flabs(w*dt[i-1]*dt[i-1]/2-v0[i-1]*dt[i-1]*coss[i-1]) + flabs (v0[i-1]*dt[i-1]*sinn[i-1]-g*dt[i-1]*dt[i-1]/2) > 2) dt[i-1] = dt[i-1]/2;
	}
	for (i=0; i<800; i++) if (ARENA[i] > maxARENA) maxARENA = ARENA[i];
	
	while ( glflag == 1 ) {
		for (mcyc = 1; mcyc <= numof; mcyc++) {
			xt[mcyc-1] = x0[mcyc-1] + v0[mcyc-1]*t[mcyc-1]*coss[mcyc-1] - w*t[mcyc-1]*t[mcyc-1]/2;
			yt[mcyc-1] = y0[mcyc-1] - v0[mcyc-1]*t[mcyc-1]*sinn[mcyc-1] + g*t[mcyc-1]*t[mcyc-1]/2;
			while (flabs(w*dt[mcyc-1]*dt[mcyc-1]/2-v0[mcyc-1]*dt[mcyc-1]*coss[mcyc-1]+w*t[mcyc-1]*dt[mcyc-1]) + flabs (v0[mcyc-1]*dt[mcyc-1]*sinn[mcyc-1]-g*t[mcyc-1]*dt[mcyc-1]-g*dt[mcyc-1]*dt[mcyc-1]/2) > 2) {
				dt[mcyc-1] = dt[mcyc-1]/2;
			}
			if ((xt[mcyc-1] > 799 || xt[mcyc-1] < 0) && REFLECTWALLS == 1 && TRANSPARENTSIDEWALLS == 0) {
				if (xt[mcyc-1] < 0) xt[mcyc-1] = 0;
				if (xt[mcyc-1] > 799) xt[mcyc-1] = 799;
				vch = sqrt((v0[mcyc-1]*coss[mcyc-1]-w*t[mcyc-1])*(v0[mcyc-1]*coss[mcyc-1]-w*t[mcyc-1]) + (g*t[mcyc-1]-v0[mcyc-1]*sinn[mcyc-1])*(g*t[mcyc-1]-v0[mcyc-1]*sinn[mcyc-1]));
				sinn[mcyc-1] = -(g*t[mcyc-1]-v0[mcyc-1]*sinn[mcyc-1])/vch;
				coss[mcyc-1] = -(v0[mcyc-1]*coss[mcyc-1]-w*t[mcyc-1])/vch;
				x0[mcyc-1] = xt[mcyc-1];
				y0[mcyc-1] = yt[mcyc-1];
				v0[mcyc-1] = vch;
				t[mcyc-1] = 0;
				xt[mcyc-1] = x0[mcyc-1];
				yt[mcyc-1] = y0[mcyc-1];
//			if (IMMEDIATETRAECTORY == 0) PartialUpdateBW (0, 55, 800, maxARENA-55);
			}
			if ((xt[mcyc-1] > 799 || xt[mcyc-1] < 0) && REFLECTWALLS == 0 && TRANSPARENTSIDEWALLS == 0) {
				vch = sqrt((v0[mcyc-1]*coss[mcyc-1]-w*t[mcyc-1])*(v0[mcyc-1]*coss[mcyc-1]-w*t[mcyc-1]) + (g*t[mcyc-1]-v0[mcyc-1]*sinn[mcyc-1])*(g*t[mcyc-1]-v0[mcyc-1]*sinn[mcyc-1]));
				sinn[mcyc-1] = -(g*t[mcyc-1]-v0[mcyc-1]*sinn[mcyc-1])/vch;
				coss[mcyc-1] = (v0[mcyc-1]*coss[mcyc-1]-w*t[mcyc-1])/vch;
				x0[mcyc-1] = 800-xt[mcyc-1];
				if (x0[mcyc-1] < 0) x0[mcyc-1] = 0;
				if (x0[mcyc-1] > 799) x0[mcyc-1] = 799;
				y0[mcyc-1] = yt[mcyc-1];
				v0[mcyc-1] = vch;
				t[mcyc-1] = 0;
				xt[mcyc-1] = x0[mcyc-1];
				yt[mcyc-1] = y0[mcyc-1];
//			if (IMMEDIATETRAECTORY == 0) PartialUpdateBW (0, 55, 800, maxARENA-55);
			}
			if (xt[mcyc-1] >= 0 && xt[mcyc-1] <=799 && t[mcyc-1] > 0) {
				for (i=0; i<PLAYERS; i++) {
					if (fround(xt[mcyc-1]) >= IPLAYER[i].x-10 && fround(xt[mcyc-1]) <= IPLAYER[i].x+10 && fround(yt[mcyc-1]) >= IPLAYER[i].y-7 && fround(yt[mcyc-1]) <= IPLAYER[i].y && IPLAYER[i].life > 0) {
//						fprintf (stderr, "trux=%i, truy=%i, v=%i, alpha=%i, w=%f, g=%f \n\n", fround(xt[mcyc-1]), fround(yt[mcyc-1]), powstart[0], angstart[0], w, g);
						flag[mcyc-1] = 0;
					}
				}
				if (fround(yt[mcyc-1]) >= ARENA[fround(xt[mcyc-1])] || fround(yt[mcyc-1]) >= 599) {
//					fprintf (stderr, "trux=%i, truy=%i, v=%i, alpha=%i, w=%f, g=%f \n\n", fround(xt[mcyc-1]), fround(yt[mcyc-1]), powstart[0], angstart[0], w, g);
					flag[mcyc-1] = 0;
				}
			}
			if (fround (yt[mcyc-1]) >= 599) flag[mcyc-1] = 0;
			if (yt[mcyc-1] <= 599 && yt[mcyc-1] >= 56 && xt[mcyc-1] >= 0 && xt[mcyc-1] <= 799) {
				DrawRect (fround(xt[mcyc-1])-1, fround(yt[mcyc-1])-1, 2, 2, BLACK);
				if (xpo[mcyc-1] != -1 && ypo[mcyc-1] != -1) {
					draw_invert_sprite (xpo[mcyc-1]-poup.w/2, ypo[mcyc-1], &poup);
					xpo[mcyc-1] = -1;
					ypo[mcyc-1] = -1;
				}
			}
			if (yt[mcyc-1] < 56 && xt[mcyc-1] >= 0 && xt[mcyc-1] <= 799 && IMMEDIATETRAECTORY == 0 && tglob-t0 > tupd) {
				if (xpo[mcyc-1] == -1 && ypo[mcyc-1] == -1) {
					draw_invert_sprite (xt[mcyc-1]-poup.w/2, 57, &poup);
					xpo[mcyc-1] = xt[mcyc-1];
					ypo[mcyc-1] = 57;
				}
				else {
					draw_invert_sprite (xpo[mcyc-1]-poup.w/2, ypo[mcyc-1], &poup);
					draw_invert_sprite (xt[mcyc-1]-poup.w/2, 57, &poup);
					xpo[mcyc-1] = xt[mcyc-1];
					ypo[mcyc-1] = 57;
				}
			}
			if (flag[mcyc-1] == 1) t[mcyc-1] = t[mcyc-1]+dt[mcyc-1];
		}
		if (tglob-t0 > tupd && IMMEDIATETRAECTORY == 0) {
			PartialUpdateBW (0, 55, 800, maxARENA-55);
			t0 = tglob;
		}
		glflag = 0;
		for (i=1; i<=numof; i++) if (flag[i-1] == 1) glflag = 1;
		mindt = 1;
		for (i=1; i<=numof; i++) if (dt[i-1] < mindt) mindt = dt[i-1];
		tglob=tglob+mindt;
	}
	PartialUpdateBW (0, 55, 800, 545);
	if (update == 1) {
		FillArea (0, 55, 800, 545, WHITE);
		show_arena();
		show_tanks();
		show_bonuses_in_game();
		show_names_of_players(playa);
		PartialUpdateBW (0, 55, 800, 545);
	}
	for (i=1; i<=numof; i++) {
		xret[i-1] = fround(xt[i-1]);
		yret[i-1] = fround(yt[i-1]);	
	}
	free(t);
	free(x0);
	free(y0);
	free(v0);
	free(alpha);
	free(xt);
	free(yt);
	free(xdt);
	free(ydt);
	free(coss);
	free(sinn);
	free(dt);
	free(flag);
	free(xpo);
	free(ypo);
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
		dx = ((rand()%3)-1)*(weapon-6)*2;
		dy = ((rand()%3)-1)*(weapon-6)*2;
//		fprintf (stderr, "dx = %i, dy = %i \n", dx, dy);
		while (dx == dy && dx == 0) {
			dx = ((rand()%3)-1)*(weapon-6)*2;
			dy = ((rand()%3)-1)*(weapon-6)*2;
		}
		xt = x0 + dx;
		yt = y0 + dy;
		if (xt <= 799 && xt >= 0) {
			if (yt <= 599 && yt >= ARENA[xt]) {
				for (j = x0; j != xt; j = j + dx / iabs(dx)) {
					for (k = y0; k != yt; k = k + dy / iabs(dy)) {
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
				counter2++;
				x0 = xt;
				y0 = yt;
			}
		}
		if (counter2%20 == 0) PartialUpdateBW (0, 55, 800, 545);
	}
	PartialUpdateBW (0, 55, 800, 545);
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
	int xpr = -1, ypr = -1; //previous coords for walking man to delete it from screen, -1, -1 - not initialized
	
	xt = IPLAYER[playa-1].x;
	
	if (IPLAYER[playa-1].angle >= 90) direction = -1;
	if (IPLAYER[playa-1].angle < 90) direction = 1;
	
	while (flag == 1) {
		xt = xt + direction;
		//line, showing traectory of walking man. not looking good on screen
//		if (xt > 0 && xt < 799) {
//			if (xt-direction != IPLAYER[playa-1].x) DrawLine (xt+2*direction, ARENA[xt]-2, xt-direction+2*direction, ARENA[xt-direction]-2, BLACK);
//			if (xt-direction == IPLAYER[playa-1].x) DrawLine (xt-direction, ARENA[xt-direction], xt+2*direction, ARENA[xt]-2, BLACK);
//		}
		for (i=0; i<PLAYERS; i++) {
			if (xt <= 0 || xt >= 799 || (xt >= IPLAYER[i].x-4 && xt <= IPLAYER[i].x+4 && i != playa-1 && IPLAYER[i].life > 0)) {
				*xret = xt;
				*yret = ARENA[xt];
				flag = 0;
			}
		}
		counter++;
		if (counter%50 == 0) {
			if (xpr == -1 && ypr == -1) {
				if (direction == 1) DrawBitmap (xt-15, ARENA[xt]-32, &fighterright);
				else DrawBitmap (xt-15, ARENA[xt]-32, &fighterleft);
				xpr = xt-15;
				ypr = ARENA[xt]-32;
			}
			else {
				FillArea (xpr, ypr, 30, 34, WHITE);
				if (direction == 1) DrawBitmap (xt-15, ARENA[xt]-32, &fighterright);
				else DrawBitmap (xt-15, ARENA[xt]-32, &fighterleft);
				xpr = xt-15;
				ypr = ARENA[xt]-32;
				show_arena();
				show_tanks();
				show_bonuses_in_game();
				show_names_of_players(playa);
			}
			PartialUpdateBW (0, 55, 800, 545);
		}
	}
	if (xpr != -1 && ypr != -1) {
		FillArea (xpr, ypr, 30, 34, WHITE);
		show_arena();
		show_tanks();
		show_bonuses_in_game();
		show_names_of_players(playa);		
	}
	if (direction == 1) DrawBitmap (xt-15, ARENA[xt]-32, &fighterright);
	else DrawBitmap (xt-15, ARENA[xt]-32, &fighterleft);	
	
	PartialUpdateBW (0, 55, 800, 545);
	FillArea (0, 55, 800, 545, WHITE);
	show_arena();
	show_tanks();
	show_bonuses_in_game();
	show_names_of_players(playa);
	PartialUpdateBW (0, 55, 800, 545);
}

// explodes dead tanks and set IPLAYER[].dead = 1
void explode_dead () {
	int i;
	int xe[1], ye[1];
	for (i=0; i<PLAYERS; i++) {
		if (IPLAYER[i].life <= 0 && IPLAYER[i].dead != 1) {
			xe[0] = IPLAYER[i].x;
			ye[0] = IPLAYER[i].y;
			explosion (1, xe, ye, 3, i);
			IPLAYER[i].dead = 1;
			fall_landscape();
		}
	}
}

// main function to fire by playa with weapon
// playa [1..PLAYERS]
// weapon [1..13]
void main_fire (int playa, int weapon) {
	int x2, y2;
	int xr, yr;
//	int xr1, yr1, xr2, yr2, xr3, yr3;
	int xstart[20], ystart[20], powstart[20], angstart[20];
	int xret[20], yret[20];
	int xex[20], yex[20]; //x,y of explosions
	float alpha;
	int i;
	
	if ((weapon >= 1 && weapon <= 8) || weapon == 11) {
		alpha = (float)((IPLAYER[playa-1].angle)*M_PI/180);
		xstart[0] = IPLAYER[playa-1].x + (float)(15*cos(alpha));
		ystart[0] = IPLAYER[playa-1].y - 6 - (float)(15*sin(alpha));
		powstart[0] = IPLAYER[playa-1].power;
		angstart[0] = IPLAYER[playa-1].angle;
	}
	if (weapon == 9 || weapon == 10) {
		for (i = 8-weapon; i <= weapon-8; i++) { // 
			alpha = (float)((IPLAYER[playa-1].angle)*M_PI/180);
			xstart[i+(weapon-8)] = IPLAYER[playa-1].x + (float)(15*cos(alpha));
			ystart[i+(weapon-8)] = IPLAYER[playa-1].y - 6 - (float)(15*sin(alpha));
			powstart[i+(weapon-8)] = IPLAYER[playa-1].power;
			angstart[i+(weapon-8)] = IPLAYER[playa-1].angle + i*7;
		}
	}
	
	if ((weapon >= 1 && weapon <= 4) || weapon == 11) {
		show_parabollic_traectory (1, xstart, ystart, powstart, angstart, xret, yret, playa, 1);
//		fprintf (stderr, "xfire=%i, yfire=%i\n", xret[0], yret[0]);
		xex[0] = xret[0];
		yex[0] = yret[0];
		if (xex[0] >= 0 && xex[0] <= 799) {
			explosion (1, xex, yex, weapon, playa);
			fall_landscape();
			explode_dead();
		}
		if (IPLAYER[playa-1].ammo[weapon-1] != -1) IPLAYER[playa-1].ammo[weapon-1]--;
	}
	if (weapon == 5 || weapon == 6) {
		show_parabollic_traectory (1, xstart, ystart, powstart, angstart, xret, yret, playa, 0);
		if (xret[0] >= 0 && xret[0] <= 799) { 
			show_roll_traectory (&x2, &y2, xret[0], yret[0]);
			FillArea (0, 55, 800, 545, WHITE);
			show_arena();
			show_tanks();
			show_bonuses_in_game();
			show_names_of_players(playa);
			PartialUpdateBW (0, 55, 800, 545);
			xex[0] = x2;
			yex[0] = y2;
			explosion (1, xex, yex, weapon, playa);
			fall_landscape();
			explode_dead();
		}
		if (IPLAYER[playa-1].ammo[weapon-1] != -1) IPLAYER[playa-1].ammo[weapon-1]--;
	}
	if (weapon == 7 || weapon == 8) {
		show_parabollic_traectory (1, xstart, ystart, powstart, angstart, xret, yret, playa, 1);
		if (xret[0] >= 0 && xret[0] <= 799) {
			acid (xret[0], yret[0], weapon);
			fall_landscape();
			explode_dead();
		}
		if (IPLAYER[playa-1].ammo[weapon-1] != -1) IPLAYER[playa-1].ammo[weapon-1]--;
	}
	if (weapon == 9 || weapon == 10) {
		show_parabollic_traectory ((weapon-8)*2+1, xstart, ystart, powstart, angstart, xret, yret, playa, 1);
		for (i=1; i<=(weapon-8)*2+1; i++) {
			xex[i-1] = xret[i-1];
			yex[i-1] = yret[i-1];
		}
		explosion ((weapon-8)*2+1, xex, yex, weapon, playa);
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
		xex[0] = xr;
		yex[0] = yr;
		explosion (1, xex, yex, weapon, playa);
		fall_landscape();
		explode_dead();
		if (IPLAYER[playa-1].ammo[weapon-1] != -1) IPLAYER[playa-1].ammo[weapon-1]--;
	}
}

// handler only for press any key (with FLAGSHTOKs it's finally working!!!)
int winner_handler(int type, int par1, int par2) {
	if (type == EVT_KEYPRESS) {
		TURNFORSAVE = 1;
		PHASEFORSAVE = 1;
		SetEventHandler(shop_handler);
		return 0;
	}
	return 0;
}

// initial draw in game menu
void draw_menuingame() {
	int xmenu=660, ymenu=58, hmenu=25;
	FillArea (650, 55, 150, 156, WHITE);
	DrawRect (650, 55, 150, 156, BLACK);
	SetFont(OpenFont("LiberationSans", 18, 0), BLACK);
	DrawString (xmenu, ymenu+hmenu*0, "FullUpdate");
	DrawString (xmenu, ymenu+hmenu*1, "Сохранить");
	DrawString (xmenu, ymenu+hmenu*2, "Загрузить");
	DrawString (xmenu, ymenu+hmenu*3, "Sudden Death");
	DrawString (xmenu, ymenu+hmenu*4, "Вернуться");
	DrawString (xmenu, ymenu+hmenu*5, "Выйти в меню");
	PartialUpdateBW(650, 55, 150, 156);
}

// menu in game
int menuingame_handler(int type, int par1, int par2) {
	static int posmenu=1;
	int posmenupr;
	int i,j;
	int xmenu=655, ymenu=58, wmenu=140, hmenu=25;
	
	if (type == EVT_SHOW && FLAGSHTOKINGAMEMENU == 1) {
		draw_menuingame();
		init_menu_choice (xmenu, ymenu, wmenu, hmenu, posmenu);
	}
	if (type == EVT_POINTERUP && FLAGSHTOKINGAMEMENU == 1) {
		if (par1 >= xmenu && par1 <= xmenu+wmenu && par2 >= ymenu && par2 <= ymenu+6*hmenu) {
			posmenupr = posmenu;
			posmenu = 1 + (par2-ymenu)/hmenu;
			menu_update (xmenu, ymenu, wmenu, hmenu, posmenu, posmenupr);
			if (posmenu == 1) { // fullupdate
				FillArea (650, 55, 150, 156, WHITE);
				FullUpdate();
				FLAGSHTOKGAME = 1;
				FLAGSHTOKINGAMEMENU = 0;
				STARTFROMBEGINNING = 0;
				posmenu = 1;
				SetEventHandler(game_handler);
				return 0;				
			}
			if (posmenu == 2) { // save game
				save_game();
				FLAGSHTOKGAME = 1;
				FLAGSHTOKINGAMEMENU = 0;
				STARTFROMBEGINNING = 0;
				posmenu = 1;
				SetEventHandler(game_handler);
				return 0;
			}
			if (posmenu == 3) { // load game
				load_game();
				SetOrientation(ORIENTATION);
				FLAGSHTOKGAME = 1;
				FLAGSHTOKINGAMEMENU = 0;
				STARTFROMBEGINNING = 1; //important! not 0!!!
				posmenu = 1;
				for (i=0;i<2;i++) { BONUSES[i].x=0; BONUSES[i].y=0; }
				for (i=0; i<PLAYERS; i++) IPLAYER[i].turned = 0;
				SetEventHandler(game_handler);
				return 0;
			}			
			if (posmenu == 4) { // sudden death
				STARTFROMBEGINNING = 0;
				posmenu = 1;
				FLAGSHTOKGAME = 1;
				FLAGSHTOKINGAMEMENU = 0;
				for (i=0; i<PLAYERS; i++) if (IPLAYER[i].life > 0) IPLAYER[i].life = 1;
				SetEventHandler(game_handler);
				return 0;
			}
			if (posmenu == 5) { // nothing, just return to game
				FLAGSHTOKGAME = 1;
				FLAGSHTOKINGAMEMENU = 0;
				STARTFROMBEGINNING = 0;
				posmenu = 1;
				SetEventHandler(game_handler);
				return 0;
			}
			if (posmenu == 6) {// to menu, nullify lives and dead status, ammo etc. (except names), nullify "watch computer battles" status and bonuses and .turned status
				GAMESHOPBEGINNING = 1;
				STARTFROMBEGINNING = 1;
				posmenu = 1;
				FLAGSHTOKGAME = 1;
				FLAGSHTOKSHOP = 1;
				FLAGSHTOKINGAMEMENU = 0;
				FLAGCOMPUTERBATTLES = 1;
				TURNFORSAVE = 1;
				PHASEFORSAVE = 1;
				for (i=0; i<PLAYERS; i++) {
					IPLAYER[i].life = 100;
					IPLAYER[i].dead = 0;
					for (j=0; j<13; j++) IPLAYER[i].ammo[j] = 0;
					IPLAYER[i].ammo[0] = -1;
					IPLAYER[i].ammo[1] = 3;
					IPLAYER[i].ammo[2] = 2;
					IPLAYER[i].money = 0;
					IPLAYER[i].wins = 0;
				}
				for (i=0;i<2;i++) { BONUSES[i].x=0; BONUSES[i].y=0; }
				for (i=0; i<PLAYERS; i++) IPLAYER[i].turned = 0;
				SetEventHandler(main_handler);
				return 0;
			}
		}
	}
	if (type == EVT_KEYPRESS && FLAGSHTOKINGAMEMENU == 1) {
		if (par1 == KEY_UP) {
			posmenupr = posmenu;
			posmenu--;
			if (posmenu < 1) posmenu = 6;
			menu_update (xmenu, ymenu, wmenu, hmenu, posmenu, posmenupr);

		}
		if (par1 == KEY_DOWN) {
			posmenupr = posmenu;
			posmenu++;
			if (posmenu > 6) posmenu = 1;
			menu_update (xmenu, ymenu, wmenu, hmenu, posmenu, posmenupr);
		}
		if (par1 == KEY_OK) {
			if (posmenu == 1) { // fullupdate
				FillArea (650, 55, 150, 156, WHITE);
				FullUpdate();
				FLAGSHTOKGAME = 1;
				FLAGSHTOKINGAMEMENU = 0;
				STARTFROMBEGINNING = 0;
				posmenu = 1;
				SetEventHandler(game_handler);
				return 0;				
			}
			if (posmenu == 2) { // save game
				save_game();
				FLAGSHTOKGAME = 1;
				FLAGSHTOKINGAMEMENU = 0;
				STARTFROMBEGINNING = 0;
				posmenu = 1;
				SetEventHandler(game_handler);
				return 0;
			}
			if (posmenu == 3) { // load game
				load_game();
				SetOrientation(ORIENTATION);
				FLAGSHTOKGAME = 1;
				FLAGSHTOKINGAMEMENU = 0;
				STARTFROMBEGINNING = 1; //important! not 0!!!
				posmenu = 1;
				for (i=0;i<2;i++) { BONUSES[i].x=0; BONUSES[i].y=0; }
				for (i=0; i<PLAYERS; i++) IPLAYER[i].turned = 0;
				SetEventHandler(game_handler);
				return 0;
			}			
			if (posmenu == 4) { // sudden death
				STARTFROMBEGINNING = 0;
				posmenu = 1;
				FLAGSHTOKGAME = 1;
				FLAGSHTOKINGAMEMENU = 0;
				for (i=0; i<PLAYERS; i++) if (IPLAYER[i].life > 0) IPLAYER[i].life = 1;
				SetEventHandler(game_handler);
				return 0;
			}
			if (posmenu == 5) { // nothing, just return to game
				FLAGSHTOKGAME = 1;
				FLAGSHTOKINGAMEMENU = 0;
				STARTFROMBEGINNING = 0;
				posmenu = 1;
				SetEventHandler(game_handler);
				return 0;
			}
			if (posmenu == 6) {// to menu, nullify lives and dead status, ammo etc. (except names), nullify "watch computer battles" status and bonuses and .turned status
				GAMESHOPBEGINNING = 1;
				STARTFROMBEGINNING = 1;
				posmenu = 1;
				FLAGSHTOKGAME = 1;
				FLAGSHTOKSHOP = 1;
				FLAGSHTOKINGAMEMENU = 0;
				FLAGCOMPUTERBATTLES = 1;
				TURNFORSAVE = 1;
				PHASEFORSAVE = 1;
				for (i=0; i<PLAYERS; i++) {
					IPLAYER[i].life = 100;
					IPLAYER[i].dead = 0;
					for (j=0; j<13; j++) IPLAYER[i].ammo[j] = 0;
					IPLAYER[i].ammo[0] = -1;
					IPLAYER[i].ammo[1] = 3;
					IPLAYER[i].ammo[2] = 2;
					IPLAYER[i].money = 0;
					IPLAYER[i].wins = 0;
				}
				for (i=0;i<2;i++) { BONUSES[i].x=0; BONUSES[i].y=0; }
				for (i=0; i<PLAYERS; i++) IPLAYER[i].turned = 0;
				SetEventHandler(main_handler);
				return 0;
			}
		}
		
	}
	return 0;
}

// show the winner. if playa = -1 - draw
// playa [1..PLAYERS]
// adding possibilities for COMMANDBATTLES = 1
// -2 - computer command wins
// -3 - human comand wins
void show_winner(int playa) {
	FillArea (0,55,800,545,WHITE);
	SetFont(OpenFont("LiberationSans", 60, 1), BLACK);
	if (playa > 0) {
		DrawTextRect (200, 100, 400, 100, "Победил", ALIGN_CENTER);
		DrawTextRect (200, 200, 400, 100, NPLAYERS[playa-1], ALIGN_CENTER);
	}
	if (playa == -1) DrawTextRect (200, 100, 450, 300, "FATALITY! В смысле, ничья", ALIGN_CENTER);
	if (playa == -2) DrawTextRect (200, 100, 450, 300, "Победила команда супермозгов", ALIGN_CENTER);
	if (playa == -3) DrawTextRect (200, 100, 450, 300, "Победила команда людей", ALIGN_CENTER);
	PartialUpdate(0,55,800,545);
}

// handler for question - will we watch computer battles further?
int allcomps_handler(int type, int par1, int par2) {
	static int posmenu=1;
	int posmenupr;
	int xmenu=350, ymenu=220, wmenu=55, hmenu=38;
	int i;
	
	if (type == EVT_SHOW && FLAGSHTOKALLCOMPS == 1) {
		FillArea (150, 100, 500, 220, WHITE);
		DrawRect (150, 100, 500, 220, BLACK);
		SetFont(OpenFont("LiberationSans", 30, 0), BLACK);
		DrawTextRect (150, 100, 500, 150, "На поле остались только компьютерные игроки. Желаете посмотреть их бой?", ALIGN_CENTER);
		DrawString (xmenu, ymenu+hmenu*0, "Да");
		DrawString (xmenu, ymenu+hmenu*1, "Нет");
		init_menu_choice (xmenu, ymenu, wmenu, hmenu, posmenu);
		PartialUpdateBW (150, 100, 500, 220);
	}
	if (type == EVT_POINTERUP && FLAGSHTOKALLCOMPS == 1) {
		if (par1 >= xmenu && par1 <= xmenu + 80 && par2 >= ymenu && par2 <= ymenu + hmenu) {
			posmenupr = posmenu;
			posmenu = 1;
			menu_update (xmenu, ymenu, wmenu, hmenu, posmenu, posmenupr);
			FLAGSHTOKALLCOMPS = 0;
			FLAGSHTOKGAME = 1;
			STARTFROMBEGINNING = 0;
			FLAGCOMPUTERBATTLES = 0; // yes, its 0
			SetEventHandler(game_handler);
			return 0;			
		}
		if (par1 >= xmenu && par1 <= xmenu + 110 && par2 > ymenu + hmenu && par2 <= ymenu + 2*hmenu) {
			posmenupr = posmenu;
			posmenu = 2;
			menu_update (xmenu, ymenu, wmenu, hmenu, posmenu, posmenupr);
			posmenu = 1;
			FLAGSHTOKALLCOMPS = 0;
			FLAGSHTOKSHOP = 1;
			FLAGSHTOKGAME = 1;
			STARTFROMBEGINNING = 1;
			GAMESHOPBEGINNING = 1;
			FLAGCOMPUTERBATTLES = 1;
			TURNFORSAVE = 1;
			PHASEFORSAVE = 1;
			for (i=0;i<2;i++) { BONUSES[i].x=0; BONUSES[i].y=0; }
			for (i=0; i<PLAYERS; i++) IPLAYER[i].turned = 0;
			SetEventHandler(shop_handler);
			return 0;			
		}
	}
	if (type == EVT_KEYPRESS) {
		if (par1 == KEY_UP || par1 == KEY_DOWN) {
			posmenupr = posmenu;
			if (posmenu == 1) posmenu = 2;
			else posmenu = 1;
			menu_update (xmenu, ymenu, wmenu, hmenu, posmenu, posmenupr);
		}
		if (par1 == KEY_OK) {
			if (posmenu == 1) {
				FLAGSHTOKALLCOMPS = 0;
				FLAGSHTOKGAME = 1;
				STARTFROMBEGINNING = 0;
				FLAGCOMPUTERBATTLES = 0; // yes, its 0
				SetEventHandler(game_handler);
				return 0;
			}
			if (posmenu == 2) {
				FLAGSHTOKALLCOMPS = 0;
				FLAGSHTOKSHOP = 1;
				FLAGSHTOKGAME = 1;
				STARTFROMBEGINNING = 1;
				GAMESHOPBEGINNING = 1;
				FLAGCOMPUTERBATTLES = 1;
				TURNFORSAVE = 1;
				PHASEFORSAVE = 1;
				for (i=0;i<2;i++) { BONUSES[i].x=0; BONUSES[i].y=0; }
				for (i=0; i<PLAYERS; i++) IPLAYER[i].turned = 0;
				SetEventHandler(shop_handler);
				return 0;
			}
		}
	}
	return 0;
}

// show whats inside bonus after hit, numofweapon [2..10]
// numofbonus [0..1]
void show_inside_bonus (int numofbonus, int numofweapon) {
	
	if (numofweapon == 2) DrawBitmap (BONUSES[numofbonus].x-15, BONUSES[numofbonus].y-45, &weapon02transparent);
	if (numofweapon == 3) DrawBitmap (BONUSES[numofbonus].x-15, BONUSES[numofbonus].y-45, &weapon03transparent);
	if (numofweapon == 4) DrawBitmap (BONUSES[numofbonus].x-15, BONUSES[numofbonus].y-45, &weapon04transparent);
	if (numofweapon == 5) DrawBitmap (BONUSES[numofbonus].x-15, BONUSES[numofbonus].y-45, &weapon05transparent);
	if (numofweapon == 6) DrawBitmap (BONUSES[numofbonus].x-15, BONUSES[numofbonus].y-45, &weapon06transparent);
	if (numofweapon == 7) DrawBitmap (BONUSES[numofbonus].x-15, BONUSES[numofbonus].y-45, &weapon07transparent);
	if (numofweapon == 8) DrawBitmap (BONUSES[numofbonus].x-15, BONUSES[numofbonus].y-45, &weapon08transparent);
	if (numofweapon == 9) DrawBitmap (BONUSES[numofbonus].x-15, BONUSES[numofbonus].y-45, &weapon09transparent);
	if (numofweapon == 10) DrawBitmap (BONUSES[numofbonus].x-15, BONUSES[numofbonus].y-45, &weapon10transparent);
	
}

// after every fire we calc our luck and set bonus coords if we are lucky
void set_bonuses_in_game() {
	int tempbonus0x, tempbonus0y, tempbonus1x, tempbonus1y; //for calc bonus coords
	
	if (BONUSFLAG == 1 && rand()%35 == 0) {
		tempbonus0x = BONUSES[0].x; tempbonus0y = BONUSES[0].y;
		tempbonus1x = BONUSES[1].x; tempbonus1y = BONUSES[1].y;
		if (tempbonus0x == 0 && tempbonus0y == 0 && tempbonus1x == 0 && tempbonus1y == 0) {
			BONUSES[0].x = rand()%740+30;
			BONUSES[0].y = ARENA[BONUSES[0].x]-10;
		}
		if (tempbonus0x != 0 && tempbonus0y != 0 && tempbonus1x == 0 && tempbonus1y == 0) {
			BONUSES[1].x = rand()%740+30;
			BONUSES[1].y = ARENA[BONUSES[1].x]-10;					
		}
		if (tempbonus0x == 0 && tempbonus0y == 0 && tempbonus1x != 0 && tempbonus1y != 0) {
			BONUSES[0].x = rand()%740+30;
			BONUSES[0].y = ARENA[BONUSES[0].x]-10;					
		}				
	}
}

//shows bonuses
void show_bonuses_in_game() {
	int i,j;
	if (BONUSFLAG == 1) {
		for (i=0; i<2; i++) { 
			if (BONUSES[i].x != 0 && BONUSES[i].y != 0) {
				for (j=0; j<PLAYERS; j++) {
					if (BONUSES[i].x >= IPLAYER[j].x-10 && BONUSES[i].x <= IPLAYER[j].x+10 && IPLAYER[j].life > 0) {
						BONUSES[i].x = 0;
						BONUSES[i].y = 0;
					}
				}
			}
			if (BONUSES[i].x != 0 && BONUSES[i].y != 0) DrawBitmap (BONUSES[i].x-15, BONUSES[i].y-15, &bonusbarrel);
		}
	}
}

//show names of players, playa - BOLD
//playa [1..PLAYERS], if playa == -1 - no BOLD
void show_names_of_players(int playa) {
	int i;
	
	SetFont(OpenFont("LiberationSans", 14, 0), BLACK);
	for (i=0; i<PLAYERS; i++) if (IPLAYER[i].life > 0) { 
		if (i == playa-1) {
			SetFont(OpenFont("LiberationSans-Bold", 14, 0), BLACK);
			DrawString (IPLAYER[i].x-25, 55, NPLAYERS[i]);
			SetFont(OpenFont("LiberationSans", 14, 0), BLACK);
			DrawRect (IPLAYER[i].x-25, 73, 50, 5, BLACK);
			FillArea (IPLAYER[i].x-25, 73, fround((float)IPLAYER[i].life/2), 4, BLACK);
		}
		else {
			DrawString (IPLAYER[i].x-25, 55, NPLAYERS[i]);
			DrawRect (IPLAYER[i].x-25, 73, 50, 5, BLACK);
			FillArea (IPLAYER[i].x-25, 73, fround((float)IPLAYER[i].life/2), 4, BLACK);
		}
	}
}

int game_handler(int type, int par1, int par2) {
	static int turn=1; //number of player to fire, [1..PLAYERS]
	static int phase=1; //phase of firing, 1 - choose weapon, 2 - choose angle and power
	static int weapon=1; //number of weapon in phase 1 [1..13]
	int tempphase=1;
	int acomp, vcomp, wcomp;// for comp firing
//	struct timeb t;
	int templives[8]; //for counting money (idea is to compare all lives before shot and after)
	int i,j;
	int count = 0; //counter to determine, if current player is winner (when all other players have 0 lives)
	int cycler = 1; // cycler for determining DRAW (when all players have 0 lives)
	int tempflag = 1; // flag for fucking with handlers, summoned from cycles (maybe it will help us to break cycle and stop brainfucking)
					  // now we are using global FLAGSHTOKs, brainfucking has ended
	int countlives; //counters for determine, if all remaining tanks are comps
	
	static int xdown, ydown; //x,y when we press on touchscreen
	static int xup, yup; //x,y when we release pressing touchscreen
	int tempweapon; //variable for temprorary weapon choice for touchscreen
	int temppower; //variable for temprorary power for touchscreen
	static int touchphase=0; // =0 - not angle, not power, =1 - angle, =2 - power
	struct timespec t;
	struct timespec tret;
	
	t.tv_sec = 0;
	t.tv_nsec = 300000000;
	
//	int xr, yr; //debug
	
	if (type == EVT_SHOW && FLAGSHTOKGAME == 1) {
		if (STARTFROMBEGINNING == 1) {
			turn = TURNFORSAVE;
			phase = 1; //here we can use PHASEFORSAVE, but we are not saving weapon number, so let it be =1
			weapon = 1;
			ClearScreen();
			WIND = WINDKOEF*(rand()%1998-999);
			show_mainpanel();
			show_player_info(turn-1);
			show_arena();	
			if (phase == 1) invert_weapon_choice(weapon);
			FullUpdate();
			drop_tanks();
			set_bonuses_in_game();
			show_bonuses_in_game();
			show_names_of_players(turn);
			draw_tank_wbarrel(turn);
			PartialUpdateBW(0,0,800,600);
		}
		if (STARTFROMBEGINNING == 0) {
			FillArea (0, 55, 800, 545, WHITE);
			show_arena();
			show_tanks();
			set_bonuses_in_game();
			show_bonuses_in_game();
			if (IPLAYER[turn-1].life > 0) {
				draw_tank_wbarrel(turn);
				show_names_of_players(turn);
			}
			clear_info_panel();
			show_player_info(turn-1);
			PartialUpdateBW (0, 0, 800, 600);
		}
	}
	// cycle for computer players
	tempflag = 1;
	while (IPLAYER[turn-1].cplayer == 1 && FLAGSHTOKGAME == 1) {
		if (IPLAYER[turn-1].life > 0 && ((COMMANDBATTLES == 1 && IPLAYER[turn-1].turned == 0) || COMMANDBATTLES == 0)) {
			FillArea (0,0,800,600,WHITE);
			show_mainpanel();
			show_player_info(turn-1);
			show_arena();
			show_tanks();
			set_bonuses_in_game();
			show_bonuses_in_game();
			show_names_of_players(turn);
			wcomp = rand()%11+1;
			while (IPLAYER[turn-1].ammo[wcomp-1] == 0) wcomp = rand()%11+1;
			invert_weapon_choice(wcomp);	
			SetFont(OpenFont("LiberationSans", 24, 0), BLACK);
			DrawString(IPLAYER[turn-1].x-5, IPLAYER[turn-1].y-50, "?");
			
			PartialUpdateBW (0,0,800,600);
			
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
			
			for (i=0; i<PLAYERS; i++) {
				if (i != turn-1) IPLAYER[turn-1].money = IPLAYER[turn-1].money + (templives[i]-IPLAYER[i].life);
			}
			if (IPLAYER[turn-1].money > 9999) IPLAYER[turn-1].money = 9999;
			if (COMMANDBATTLES == 1) {
				IPLAYER[turn-1].turned = 1;
				j = 0;
				for (i=0; i<PLAYERS; i++) if (IPLAYER[i].cplayer == 1 && IPLAYER[i].turned == 0 && IPLAYER[i].life > 0) j++;
				if (j == 0) for (i=0; i<PLAYERS; i++) if (IPLAYER[i].cplayer == 1 && IPLAYER[i].life > 0) IPLAYER[i].turned = 0; //if all alive comps had their turns - we must nullify .turned status
				
				j = 0;
				for (i=0; i<PLAYERS; i++) if (IPLAYER[i].cplayer == 0 && IPLAYER[i].turned == 0 && IPLAYER[i].life > 0) j++;
				if (j == 0) for (i=0; i<PLAYERS; i++) if (IPLAYER[i].cplayer == 0 && IPLAYER[i].life > 0) IPLAYER[i].turned = 0; //if all alive humans had their turns - we must nullify .turned status
			}
			//determine, if all remaining tanks are comps && COMMANDBATTLES == 1
			countlives = 0;
			for (i=0; i<PLAYERS; i++) if (IPLAYER[i].life > 0 && IPLAYER[i].cplayer == 0) countlives++;
			if (countlives == 0 && COMMANDBATTLES == 1) {
				show_winner(-2); //computer comand wins
				COMPWINS++;
				recalc_hiscores();
				save_hiscores();
				GAMESHOPBEGINNING = 1;
				STARTFROMBEGINNING = 1;
				turn = 1;
				phase = 1;
				weapon = 1;
				FLAGSHTOKGAME = 0;
				FLAGSHTOKSHOP = 1;
				FLAGCOMPUTERBATTLES = 1;
				for (i=0; i<PLAYERS; i++) if (IPLAYER[i].cplayer == 1) {
					IPLAYER[i].money = IPLAYER[i].money + 20;
					if (IPLAYER[i].money > 9999) IPLAYER[i].money = 9999;
				}
				for (i=0; i<PLAYERS; i++) IPLAYER[i].turned = 0;
				SetEventHandler(winner_handler);
				return 0;
			}
			count = 0;
			for (i=0; i<PLAYERS; i++) if (IPLAYER[i].life <= 0 && i != turn-1) count ++;
			if (count == PLAYERS-1) { // we dont calculate draw among computers, it's impossible, but...
				show_winner (turn);
				IPLAYER[turn-1].money = IPLAYER[turn-1].money + 50;
				if (IPLAYER[turn-1].money > 9999) IPLAYER[turn-1].money = 9999;
				IPLAYER[turn-1].wins++;
				recalc_hiscores();
				save_hiscores();
				GAMESHOPBEGINNING = 1;
				STARTFROMBEGINNING = 1;
				turn = 1;
				phase = 1;
				weapon = 1;
				FLAGSHTOKGAME = 0;
				FLAGSHTOKSHOP = 1;
				FLAGCOMPUTERBATTLES = 1;
				for (i=0;i<2;i++) { BONUSES[i].x=0; BONUSES[i].y=0; }
				for (i=0; i<PLAYERS; i++) IPLAYER[i].turned = 0;
				SetEventHandler(winner_handler);
				return 0;
			}
			//determine, if all remaining tanks are comps
			countlives = 0;
			for (i=0; i<PLAYERS; i++) if (IPLAYER[i].life > 0 && IPLAYER[i].cplayer == 0) countlives++;
			if (countlives == 0 && FLAGCOMPUTERBATTLES == 1 && COMMANDBATTLES == 0) {
				FLAGSHTOKGAME = 0;
				FLAGSHTOKALLCOMPS = 1;
				SetEventHandler(allcomps_handler);
				return 0;
			}
		}
		cycler = 1;
		if (COMMANDBATTLES == 0) {
			turn++;
			if (turn > PLAYERS) turn = 1;
		}
		else {
			while ( 1 == 1 ) {
				turn++;
				cycler++;
				if (turn > PLAYERS) turn = 1;
				if (IPLAYER[turn-1].turned == 0 && IPLAYER[turn-1].cplayer == 0 && IPLAYER[turn-1].life > 0) break;
				if (cycler > 3*PLAYERS) {
					show_winner(-2); //computer comand wins
					COMPWINS++;
					recalc_hiscores();
					save_hiscores();
					GAMESHOPBEGINNING = 1;
					STARTFROMBEGINNING = 1;
					turn = 1;
					phase = 1;
					weapon = 1;
					FLAGSHTOKGAME = 0;
					FLAGSHTOKSHOP = 1;
					FLAGCOMPUTERBATTLES = 1;
					for (i=0; i<PLAYERS; i++) if (IPLAYER[i].cplayer == 1) {
						IPLAYER[i].money = IPLAYER[i].money + 20;
						if (IPLAYER[i].money > 9999) IPLAYER[i].money = 9999;
					}
					for (i=0; i<PLAYERS; i++) IPLAYER[i].turned = 0;
					SetEventHandler(winner_handler);
					return 0;					
				}
			}
		}
		WIND = WINDKOEF*(rand()%1998-999);
		weapon = 1;
		// if next player in human, not checking his life!!! it will cause trouble in panel grid
		if (IPLAYER[turn-1].cplayer == 0) {
			FillArea (0,0,800,50,WHITE);
			show_mainpanel();
			show_player_info(turn-1);
			invert_weapon_choice(weapon);
			if (IPLAYER[turn-1].life > 0) {
				draw_tank_wbarrel(turn);// and here we are checking lives to avoid flashing tank, if after comp is dead human
				show_names_of_players(turn);
				set_bonuses_in_game();
				show_bonuses_in_game();
			}
			PartialUpdateBW (0,0,800,600);
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
			show_winner(-1); //draw
			FLAGCOMPUTERBATTLES = 1;
			for (i=0;i<2;i++) { BONUSES[i].x=0; BONUSES[i].y=0; }
			for (i=0; i<PLAYERS; i++) IPLAYER[i].turned = 0;
			SetEventHandler(winner_handler);
			return 0;
		}
	}
	// determining, if all remaining tanks are humans
	countlives = 0;
	for (i=0; i<PLAYERS; i++) if (IPLAYER[i].life > 0 && IPLAYER[i].cplayer == 1) countlives++;
	if (countlives == 0 && COMMANDBATTLES == 1 && FLAGSHTOKGAME == 1) {
		show_winner(-3); //human comand wins
		HUMANWINS++;
		recalc_hiscores();
		save_hiscores();
		GAMESHOPBEGINNING = 1;
		STARTFROMBEGINNING = 1;
		turn = 1;
		phase = 1;
		weapon = 1;
		FLAGSHTOKGAME = 0;
		FLAGSHTOKSHOP = 1;
		FLAGCOMPUTERBATTLES = 1;
		for (i=0; i<PLAYERS; i++) if (IPLAYER[i].cplayer == 0) {
			IPLAYER[i].money = IPLAYER[i].money + 20;
			if (IPLAYER[i].money > 9999) IPLAYER[i].money = 9999;
		}
		for (i=0; i<PLAYERS; i++) IPLAYER[i].turned = 0;
		SetEventHandler(winner_handler);
		return 0;
	}
	if (IPLAYER[turn-1].cplayer == 1 && FLAGSHTOKGAME == 1) {
		STARTFROMBEGINNING = 0;
		SetEventHandler(game_handler);
		return 0;
	}
	
	if (type == EVT_POINTERDOWN && FLAGSHTOKGAME == 1) {
		if (phase == 2 && par2 >= 51) {
			xdown = par1;
			ydown = par2;
		}
	}
	
	if (type == EVT_POINTERLONG || type == EVT_POINTERHOLD) {
		if (par1 >= 505 && par1 <= 555 && par2 > 50 && par2 <= 152 && touchphase == 1) {
			if (par2 <= 101) {
				IPLAYER[turn-1].angle = IPLAYER[turn-1].angle + 10;
				if (IPLAYER[turn-1].angle > 180) IPLAYER[turn-1].angle = 180;
				redraw_tank_and_angle (turn);
			}
			if (par2 > 101) {
				IPLAYER[turn-1].angle = IPLAYER[turn-1].angle - 10;
				if (IPLAYER[turn-1].angle < 0) IPLAYER[turn-1].angle = 0;
				redraw_tank_and_angle (turn);
			}
		}
		if (par1 > 555 && par1 <= 605 && par2 > 50 && par2 <= 152 && touchphase == 2) {
			if (par2 <= 101) {
				IPLAYER[turn-1].power = IPLAYER[turn-1].power + 100;
				if (IPLAYER[turn-1].power > 990) IPLAYER[turn-1].power = 990;
				clear_info_panel();
				show_player_info(turn-1);
				PartialUpdateBW (555, 1, 50, 30);				
			}
			if (par2 > 101) {
				IPLAYER[turn-1].power = IPLAYER[turn-1].power - 100;
				if (IPLAYER[turn-1].power < 10) IPLAYER[turn-1].power = 10;
				clear_info_panel();
				show_player_info(turn-1);
				PartialUpdateBW (555, 1, 50, 30);				
			}			
		}
	}
	
	if (type == EVT_POINTERUP && FLAGSHTOKGAME == 1) {
		if (phase == 1 && par1 >= 0 && par1 <= 455 && par2 >= 0 && par2 <= 50) {
			invert_weapon_choice(weapon);
			tempweapon = 1 + par1/35;
			if (IPLAYER[turn-1].ammo[tempweapon-1] != 0) weapon = tempweapon;
			invert_weapon_choice(weapon);
			PartialUpdateBW (0, 0, 455, 51);
		}
		if (phase == 2 && par2 >= 51 && touchphase == 0) { //pressing outside info panel - choosing power ang angle, touchphase MUST be ==0
			xup = par1;
			yup = par2;
			//DrawLine (xdown, ydown, xup, yup, BLACK);
			temppower = 2 * sqrt((xdown-xup)*(xdown-xup) + (ydown-yup)*(ydown-yup));
			if (temppower >= 20) {
				temppower = temppower - 30;
				if (temppower <= 10) temppower = 10;
				if (temppower >= 990) temppower = 990;
				temppower = temppower / 10;
				temppower = temppower * 10;
				IPLAYER[turn-1].power = temppower;
				clear_info_panel();
				show_player_info (turn-1);
				//draw_filltriangle (xdown, ydown, xup-1, yup, xup+6, yup, WHITE, 0);
				//draw_filltriangle (xdown, ydown, xup, yup, xup+5, yup, BLACK, 0);
				DrawLine (xdown-2, ydown, xup-2, yup, WHITE); DrawLine (xdown+2, ydown, xup+2, yup, WHITE);
				DrawLine (xdown, ydown-2, xup, yup-2, WHITE); DrawLine (xdown, ydown+2, xup, yup+2, WHITE);
				DrawLine (xdown-1, ydown, xup-1, yup, BLACK); DrawLine (xdown+1, ydown, xup+1, yup, BLACK);
				DrawLine (xdown, ydown-1, xup, yup-1, BLACK); DrawLine (xdown, ydown+1, xup, yup+1, BLACK);	
				PartialUpdateBW (0,51,800,549);
				nanosleep (&t, &tret);
				FillArea (0,51,800,549,WHITE);
				show_arena();
				show_tanks();
				draw_tank_wbarrel(turn);
				show_bonuses_in_game();
				show_names_of_players(turn);
				PartialUpdateBW (0,0,800,600);
			}
			else {
				if (xup > IPLAYER[turn-1].x) IPLAYER[turn-1].angle = (float)atan(((float)IPLAYER[turn-1].y - (float)yup) / ((float)xup - (float)IPLAYER[turn-1].x))/((float)M_PI/(float)180);
				if (xup < IPLAYER[turn-1].x) IPLAYER[turn-1].angle = 180 - (float)atan(((float)IPLAYER[turn-1].y - (float)yup) / ((float)IPLAYER[turn-1].x - (float)xup))/((float)M_PI/(float)180);
				if (xup == IPLAYER[turn-1].x) IPLAYER[turn-1].angle = 90;
				if (xup >= IPLAYER[turn-1].x && yup > IPLAYER[turn-1].y) IPLAYER[turn-1].angle = 0;
				if (xup < IPLAYER[turn-1].x && yup > IPLAYER[turn-1].y) IPLAYER[turn-1].angle = 180;
				if (yup > IPLAYER[turn-1].y) yup = IPLAYER[turn-1].y;
				
				DrawLine (IPLAYER[turn-1].x-1, IPLAYER[turn-1].y-4, xup-1, yup, WHITE); DrawLine (IPLAYER[turn-1].x+1, IPLAYER[turn-1].y-4, xup+1, yup, WHITE);
				DrawLine (IPLAYER[turn-1].x, IPLAYER[turn-1].y-5, xup, yup-1, WHITE); DrawLine (IPLAYER[turn-1].x, IPLAYER[turn-1].y-3, xup, yup+1, WHITE);
				DrawLine (IPLAYER[turn-1].x, IPLAYER[turn-1].y-4, xup, yup, BLACK);
				
				if (IPLAYER[turn-1].angle <= 90) xup = 799;
				if (IPLAYER[turn-1].angle > 90) xup = 0;
				
				DrawLine (IPLAYER[turn-1].x, IPLAYER[turn-1].y-5, xup, IPLAYER[turn-1].y-5, WHITE); DrawLine (IPLAYER[turn-1].x, IPLAYER[turn-1].y-3, xup, IPLAYER[turn-1].y-3, WHITE);
				DrawLine (IPLAYER[turn-1].x, IPLAYER[turn-1].y-4, xup, IPLAYER[turn-1].y-4, BLACK);
				PartialUpdateBW (0,51,800,549);
				redraw_tank_and_angle (turn);
				FillArea (0,51,800,549,WHITE);
				show_arena();
				show_tanks();
				draw_tank_wbarrel(turn);
				show_bonuses_in_game();
				show_names_of_players(turn);
				PartialUpdateBW (0,0,800,600);
			}
			//PartialUpdateBW (minint(xdown, xup), minint(ydown, yup), maxint (xdown, xup) - minint (xdown, xup), maxint (ydown, yup) - minint (ydown, yup));
		}
		// pressing on pointers (power or angle) using touch
		if (phase == 2 && touchphase == 1 && par1 >= 505 && par1 <= 555 && par2 >= 50 && par2 <= 152) { //press on angle pointers for change
			if (par2 <= 101) {
				IPLAYER[turn-1].angle++;
				if (IPLAYER[turn-1].angle > 180) IPLAYER[turn-1].angle = 180;
				redraw_tank_and_angle (turn);
			}
			if (par2 > 101) {
				IPLAYER[turn-1].angle--;
				if (IPLAYER[turn-1].angle < 0) IPLAYER[turn-1].angle = 0;
				redraw_tank_and_angle (turn);				
			}
		}
		if (phase == 2 && touchphase == 2 && par1 >= 555 && par1 <= 605 && par2 >= 50 && par2 <= 152) { //pressing on power pointers for change
			if (par2 <= 101) {
				IPLAYER[turn-1].power = IPLAYER[turn-1].power + 10;
				if (IPLAYER[turn-1].power > 990) IPLAYER[turn-1].power = 990;
				clear_info_panel();
				show_player_info(turn-1);
				PartialUpdateBW (555, 1, 50, 30);
			}
			if (par2 > 101) {
				IPLAYER[turn-1].power = IPLAYER[turn-1].power - 10;
				if (IPLAYER[turn-1].power < 10) IPLAYER[turn-1].power = 10;
				clear_info_panel();
				show_player_info(turn-1);
				PartialUpdateBW (555, 1, 50, 30);				
			}
		}
		// here we press on angle or power and then - pointers for cnange show up! if we press again - pointers will disappear
		if (phase == 2 && par2 <= 50 && par1 >= 505 && par1 <= 605) {
			if (touchphase == 0) {
				if (par1 <= 555) {
					touchphase = 1;
					draw_touchpointer (505, 51, 51, 51, 0, BLACK);
					draw_touchpointer (505, 101, 51, 51, 2, BLACK);
					PartialUpdateBW (505, 0, 51, 152);
				}
				if (par1 > 555) {
					touchphase = 2;
					draw_touchpointer (555, 51, 51, 51, 0, BLACK);
					draw_touchpointer (555, 101, 51, 51, 2, BLACK);
					PartialUpdateBW (555, 0, 51, 152);
				}
			}
			else {
				if (touchphase == 1 && par1 <= 555) {
					touchphase = 0;
					FillArea (505, 51, 51, 102, WHITE);
					show_names_of_players(turn);
					show_arena();
					show_tanks();
					draw_tank_wbarrel(turn);
					PartialUpdateBW (505, 0, 51, 152);
				}
				if (touchphase == 2 && par1 > 555) {
					touchphase = 0;
					FillArea (555, 51, 51, 102, WHITE);
					show_names_of_players(turn);
					show_arena();
					show_tanks();
					draw_tank_wbarrel(turn);
					PartialUpdateBW (555, 0, 51, 152);					
				}
			}
		}
	}
	
	if (type == EVT_KEYPRESS) {
		if (par1 == KEY_OK || par1 == KEY_BACK) {
			tempphase = phase;
			if (tempphase == 1) {
				invert_weapon_choice(weapon);
				PartialUpdateBW (0,0,455,50);
				phase = 2;
			}
			if (tempphase == 2 && touchphase == 0) { //fire, touchphase MUST be ==0
				for (i=0; i<PLAYERS; i++) {
					templives[i] = IPLAYER[i].life;
				}
				main_fire (turn, weapon);
				for (i=0; i<PLAYERS; i++) {
					if (i != turn-1) IPLAYER[turn-1].money = IPLAYER[turn-1].money + (templives[i]-IPLAYER[i].life);
				}
				if (IPLAYER[turn-1].money > 9999) IPLAYER[turn-1].money = 9999;
				if (COMMANDBATTLES == 1) {
					IPLAYER[turn-1].turned = 1;
					j = 0;
					for (i=0; i<PLAYERS; i++) if (IPLAYER[i].cplayer == 0 && IPLAYER[i].turned == 0 && IPLAYER[i].life > 0) j++;
					if (j == 0) for (i=0; i<PLAYERS; i++) if (IPLAYER[i].cplayer == 0 && IPLAYER[i].life > 0) IPLAYER[i].turned = 0; //if all alive humans had their turns - we must nullify .turned status
					
					j = 0;
					for (i=0; i<PLAYERS; i++) if (IPLAYER[i].cplayer == 1 && IPLAYER[i].turned == 0 && IPLAYER[i].life > 0) j++;
					if (j == 0) for (i=0; i<PLAYERS; i++) if (IPLAYER[i].cplayer == 1 && IPLAYER[i].life > 0) IPLAYER[i].turned = 0; //if all alive comps had their turns - we must nullify .turned status
				}
				// determining, if all remaining tanks are humans
				countlives = 0;
				for (i=0; i<PLAYERS; i++) if (IPLAYER[i].life > 0 && IPLAYER[i].cplayer == 1) countlives++;
				if (countlives == 0 && COMMANDBATTLES == 1) {
					show_winner(-3); //human comand wins
					HUMANWINS++;
					recalc_hiscores();
					save_hiscores();
					GAMESHOPBEGINNING = 1;
					STARTFROMBEGINNING = 1;
					turn = 1;
					phase = 1;
					weapon = 1;
					FLAGSHTOKGAME = 0;
					FLAGSHTOKSHOP = 1;
					FLAGCOMPUTERBATTLES = 1;
					for (i=0; i<PLAYERS; i++) if (IPLAYER[i].cplayer == 0) {
						IPLAYER[i].money = IPLAYER[i].money + 20;
						if (IPLAYER[i].money > 9999) IPLAYER[i].money = 9999;
					}
					for (i=0; i<PLAYERS; i++) IPLAYER[i].turned = 0;
					SetEventHandler(winner_handler);
					return 0;
				}
				// determining, if all remaining tanks are comps
				countlives = 0;
				for (i=0; i<PLAYERS; i++) if (IPLAYER[i].life > 0 && IPLAYER[i].cplayer == 0) countlives++;
				if (countlives == 0 && COMMANDBATTLES == 1) {
					show_winner(-2); //computer comand wins
					COMPWINS++;
					recalc_hiscores();
					save_hiscores();
					GAMESHOPBEGINNING = 1;
					STARTFROMBEGINNING = 1;
					turn = 1;
					phase = 1;
					weapon = 1;
					FLAGSHTOKGAME = 0;
					FLAGSHTOKSHOP = 1;
					FLAGCOMPUTERBATTLES = 1;
					for (i=0; i<PLAYERS; i++) if (IPLAYER[i].cplayer == 1) {
						IPLAYER[i].money = IPLAYER[i].money + 20;
						if (IPLAYER[i].money > 9999) IPLAYER[i].money = 9999;
					}
					for (i=0; i<PLAYERS; i++) IPLAYER[i].turned = 0;
					SetEventHandler(winner_handler);
					return 0;
				}
				// determining wining of certain player if COMMANDBATTLES == 0
				count = 0;
				for (i=0; i<PLAYERS; i++) if (IPLAYER[i].life <= 0 && i != turn-1) count ++;
				if (count == PLAYERS-1 && COMMANDBATTLES == 0) {
					show_winner (turn);
					IPLAYER[turn-1].money = IPLAYER[turn-1].money + 50;
					if (IPLAYER[turn-1].money > 9999) IPLAYER[turn-1].money = 9999;
					IPLAYER[turn-1].wins++;
					recalc_hiscores();
					save_hiscores();
					GAMESHOPBEGINNING = 1;
					STARTFROMBEGINNING = 1;
					turn = 1;
					phase = 1;
					weapon = 1;
					FLAGSHTOKGAME = 0;
					FLAGSHTOKSHOP = 1;
					FLAGCOMPUTERBATTLES = 1;
					for (i=0;i<2;i++) { BONUSES[i].x=0; BONUSES[i].y=0; }
					for (i=0; i<PLAYERS; i++) IPLAYER[i].turned = 0;
					SetEventHandler(winner_handler);
					return 0;
				}
				
				phase = 1;
				weapon = 1;
				WIND = WINDKOEF*(rand()%1998-999);
				cycler = 0;
				if (COMMANDBATTLES == 0) {
					turn++;
					if (turn > PLAYERS) turn = 1;
				}
				else {
					while ( 1 == 1 ) {
						turn++;
						cycler++;
						if (turn > PLAYERS) turn = 1;
						if (IPLAYER[turn-1].turned == 0 && IPLAYER[turn-1].cplayer == 1 && IPLAYER[turn-1].life > 0) break;
						if (cycler > 3*PLAYERS) {
							show_winner(-3); //human comand wins
							COMPWINS++;
							recalc_hiscores();
							save_hiscores();
							GAMESHOPBEGINNING = 1;
							STARTFROMBEGINNING = 1;
							turn = 1;
							phase = 1;
							weapon = 1;
							FLAGSHTOKGAME = 0;
							FLAGSHTOKSHOP = 1;
							FLAGCOMPUTERBATTLES = 1;
							for (i=0; i<PLAYERS; i++) if (IPLAYER[i].cplayer == 0) {
								IPLAYER[i].money = IPLAYER[i].money + 20;
								if (IPLAYER[i].money > 9999) IPLAYER[i].money = 9999;
							}
							for (i=0; i<PLAYERS; i++) IPLAYER[i].turned = 0;
							SetEventHandler(winner_handler);
							return 0;					
						}
					}					
				}
				
				cycler = 1;
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
						show_winner(-1);//draw
						FLAGSHTOKGAME = 0;
						FLAGSHTOKSHOP = 1;
						FLAGCOMPUTERBATTLES = 1;
						for (i=0;i<2;i++) { BONUSES[i].x=0; BONUSES[i].y=0; }
						for (i=0; i<PLAYERS; i++) IPLAYER[i].turned = 0;
						SetEventHandler(winner_handler);
						return 0;
					}
				}
				
				
				if (IPLAYER[turn-1].cplayer == 1) {
					STARTFROMBEGINNING = 0;
					SetEventHandler(game_handler);
					return 0;
				}
				ClearScreen();
				show_mainpanel();
				show_arena();
				show_tanks();
				//clear_info_panel();
				show_player_info(turn-1);
				invert_weapon_choice(weapon);
				draw_tank_wbarrel(turn);
				show_names_of_players(turn);
				set_bonuses_in_game();
				show_bonuses_in_game();
				PartialUpdateBW (0,0,800,600);
			}
		}
		if ((par1 == KEY_PLUS || par1 == KEY_DELETE || par1 == KEY_PREV2 || par1 == KEY_PREV) && touchphase == 0) {
			FLAGSHTOKGAME = 0;
			FLAGSHTOKINGAMEMENU = 1;
			TURNFORSAVE = turn;
			PHASEFORSAVE = phase;
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
	char diff[20];
	char stnofpl[50] = "Игроков: ";
	char stdiff[50] = "Сложность: ";
	char nofpl[2];
	
	nofpl[0] = itoc (PLAYERS);
	nofpl[1] = '\0';
	if (DIFFICUILTY == 1) strcpy (diff, "Легко");
	if (DIFFICUILTY == 2) strcpy (diff, "Средне");
	if (DIFFICUILTY == 3) strcpy (diff, "Тяжело");
	
	strcat (stdiff, diff);
	strcat (stnofpl, nofpl);
	SetOrientation (ORIENTATION);
//	DrawBitmap (0,0,&tank1);
//	DrawBitmap (400,0,&tank2);
	draw_trubitmap (400, 0, &tank2pic);
	draw_trubitmap (0, 0, &tank1pic);
	draw_trubitmap (0, 300, &tank3pic);
//	DrawBitmap (1,300,&tank3);
	DrawLine (0, SCRW/2, SCRH, SCRW/2, BLACK);
	DrawLine (SCRH/2, 0, SCRH/2, SCRW/2, BLACK);
	SetFont(OpenFont("LiberationSans", 40, 0), BLACK);
	DrawString (xl+15, yl-60, "TANK WAR");
	SetFont(OpenFont("LiberationSans", 24, 0), BLACK);
	//DrawString (xl, yl, "Start");
	DrawString (xl, yl, "Старт");
	DrawString (xl, yl+hgt, stnofpl);
	DrawString (xl, yl+2*hgt, stdiff);
//	DrawString (xl, yl+3*hgt, "Инструкция");
	DrawString (xl, yl+3*hgt, "Предыстория");
	DrawString (xl, yl+4*hgt, "Настройки");
	DrawString (xl, yl+5*hgt, "Статистика");
	DrawString (xl, yl+6*hgt, "Выход");
}

// xl,yl - left uppder corner of menu, wdt,hgt - width and height of 1 item of menu, posmenu - number of choice, posmenupr - previous number of choice

// redraw one item in main menu (on change difficuilty or number of players)
void redraw_choice_menu (int xl, int yl, int hgt, int posmenu, int *mainwd) {
	char diff[20];
	char stnofpl[50] = "Игроков: ";
	char stdiff[50] = "Сложность: ";
	char nofpl[2];
	int newx,newy;
	
	nofpl[0] = itoc (PLAYERS);
	nofpl[1] = '\0';	
	if (DIFFICUILTY == 1) strcpy (diff, "Легко");
	if (DIFFICUILTY == 2) strcpy (diff, "Средне");
	if (DIFFICUILTY == 3) strcpy (diff, "Тяжело");
	newx = xl;
	newy = yl+(posmenu-1)*hgt;
	strcat (stdiff, diff);
	strcat (stnofpl, nofpl);
	if (posmenu == 2) {
		InvertAreaBW (xl, yl+(posmenu-1)*hgt, mainwd[posmenu-1], hgt);
		FillArea (newx, newy, mainwd[posmenu-1], 28, WHITE);
		SetFont(OpenFont("LiberationSans", 24, 0), BLACK);
		DrawString (newx, newy, stnofpl);
		init_mainmenu_choice (xl, yl, hgt, posmenu, mainwd);
	}
	if (posmenu == 3) {
		InvertAreaBW (xl, yl+(posmenu-1)*hgt, mainwd[posmenu-1], hgt);
		FillArea (newx, newy, mainwd[posmenu-1], 28, WHITE);
		SetFont(OpenFont("LiberationSans", 24, 0), BLACK);
		DrawString (newx, newy, stdiff);
		init_mainmenu_choice (xl, yl, hgt, posmenu, mainwd);
	}
}

// redraw one item in players menu (on change computer-human)
void redraw_players_menu (int xl, int yl, int wdt, int hgt, int posmenu) {
	char comporhuman [32];
	
	SetFont(OpenFont("LiberationSans", 24, 0), BLACK);
	if (IPLAYER[posmenu-1].cplayer == 1) strcpy (comporhuman, "Супермозг");
	if (IPLAYER[posmenu-1].cplayer == 0) strcpy (comporhuman, "Человек");
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
void init_mainmenu_choice (int xl, int yl, int hgt, int posmenu, int *mainwd) {
	InvertAreaBW (xl, yl+(posmenu-1)*hgt, mainwd[posmenu-1], hgt);
//	InvertAreaBW (xl, yl+(posmenu-1)*hgt, wdt, hgt);
	PartialUpdateBW (xl, yl+(posmenu-1)*hgt, mainwd[posmenu-1], hgt);
}

// moves choice in main menu from posmenupr to posmenu, mainwd - massive of widths of lines
void mainmenu_update (int xl, int yl, int hgt, int posmenu, int posmenupr, int *mainwd) {
	int oldx, oldy;
	int newx, newy;
	int miny, maxsumy;
		
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
	if (num == 40) return '-';
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
	int i;
	int xkb, ykb, wkl, hkl;
	char buff[10];
	char dump[2];
	
	xkb = SCRH/4;
	ykb = 300;
	wkl = 30;
	hkl = 30;
	
	if (type == EVT_SHOW) {
		ClearScreen();
		SetFont(OpenFont("LiberationSans", 28, 0), BLACK);
		DrawString (50, 50, "Долгое нажатие на кнопку Backspace - очистить имя");
		SetFont(OpenFont("LiberationSans", 46, 0), BLACK);
		DrawString (SCRH/4, 150, NPLAYERS[CHOICE-1]);
		draw_keyboard (xkb, ykb);
		init_keyboard_kursor(xkb, ykb, wkl, hkl, posx, posy);
		len = strlen (NPLAYERS[CHOICE-1]);
		for (i=0; i<=13; i++) DrawRect(0+i,0+i,800-2*i,600-2*i,ret_4num(16*(13-i)));
		FullUpdate();
	}
	if (type == EVT_POINTERUP) {
		if (par1 >= xkb && par1 <= xkb + 10*wkl && par2 >= ykb && par2 <= ykb+4*hkl) {
			posxpr = posx;
			posypr = posy;
			posx = 1+(par1-xkb)/wkl;
			posy = 1+(par2-ykb)/hkl;
			update_keyboard_kursor(xkb, ykb, wkl, hkl, posx, posy, posxpr, posypr);
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
		if (par1 >= xkb + 11*wkl && par2 >= ykb + 2*hkl && par2 <= ykb + 3*hkl) {
			len--;
			if (len < 0) len=0;
			strcpy (buff, "");
			strncat (buff, NPLAYERS[CHOICE-1], len);
			strcpy (NPLAYERS[CHOICE-1], buff);
			redraw_player_name (SCRH/4, 150);			
		}
		if (par1 >= xkb - 3*wkl && par1 <= xkb && par2 >= ykb + 2*hkl && par2 <= ykb + 3*hkl) {
			if (len > 0) {
				posx=1; posy=1;
				posxpr=1; posypr=1;
				SetEventHandler(start_handler);
				return 0;
			}
		}
	}
	
	if (type == EVT_POINTERLONG || type == EVT_POINTERHOLD) {
		if (par1 >= xkb + 11*wkl && par2 >= ykb + 2*hkl && par2 <= ykb + 3*hkl) {
			len = 0;
			strcpy (NPLAYERS[CHOICE-1], "");
			redraw_player_name (SCRH/4, 150);			
		}
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
			if (posx == 0) {
				if (len > 0) {
					posx=1; posy=1;
					posxpr=1; posypr=1;
					SetEventHandler(start_handler);
					return 0;
				}
			}
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
	if (type == EVT_KEYREPEAT && par1 == KEY_OK && posx == 11) {
		len = 0;
		strcpy (NPLAYERS[CHOICE-1], "");
		redraw_player_name (SCRH/4, 150);
	}
	return 0;
}

//redraws options menu, wd - widths of lines
void draw_options_menu (int x, int y, int h, int posmenu, int *wd) {
	char strfinal[90];
	char yes[10]="Да";
	char no[10]="Нет";
	
	SetFont(OpenFont("LiberationSans", 24, 0), BLACK);
	DrawString (x+130, y-48, "Настройки");
	
	strcpy (strfinal, "");
	if (IMMEDIATEDROP == 0) { strcpy(strfinal, "Мгновенное падение танков: "); strcat (strfinal, no); }
	if (IMMEDIATEDROP == 1) { strcpy(strfinal, "Мгновенное падение танков: "); strcat (strfinal, yes); }
	FillArea (x, y+h*0, wd[0], h, WHITE);
	DrawString (x, y+h*0, strfinal);
	
	strcpy (strfinal, "");
	if (IMMEDIATETRAECTORY == 0) { strcpy(strfinal, "Мгновенная отрисовка траекторий: "); strcat (strfinal, no); }
	if (IMMEDIATETRAECTORY == 1) { strcpy(strfinal, "Мгновенная отрисовка траекторий: "); strcat (strfinal, yes); }
	FillArea (x, y+h*1, wd[1], h, WHITE);
	DrawString (x, y+h*1, strfinal);
	
	strcpy (strfinal, "");
	if (REFLECTWALLS == 0) { strcpy(strfinal, "Отражение от боковых стен: "); strcat (strfinal, no); }
	if (REFLECTWALLS == 1) { strcpy(strfinal, "Отражение от боковых стен: "); strcat (strfinal, yes); }
	FillArea (x, y+h*2, wd[2], h, WHITE);
	DrawString (x, y+h*2, strfinal);
	
	strcpy (strfinal, "");
	if (TRANSPARENTSIDEWALLS == 0) { strcpy(strfinal, "Замкнутое игровое поле: "); strcat (strfinal, yes); }
	if (TRANSPARENTSIDEWALLS == 1) { strcpy(strfinal, "Замкнутое игровое поле: "); strcat (strfinal, no); }
	FillArea (x, y+h*3, wd[3], h, WHITE);
	DrawString (x, y+h*3, strfinal);
	
	strcpy (strfinal, "");
	if (COMMANDBATTLES == 0) { strcpy(strfinal, "Командная игра компьютера: "); strcat (strfinal, no); }
	if (COMMANDBATTLES == 1) { strcpy(strfinal, "Командная игра компьютера: "); strcat (strfinal, yes); }
	FillArea (x, y+h*4, wd[4], h, WHITE);
	DrawString (x, y+h*4, strfinal);	
	
	strcpy (strfinal, "");
	if (WINDKOEF == 0) { strcpy(strfinal, "Ветер: "); strcat (strfinal, no); }
	if (WINDKOEF == 1) { strcpy(strfinal, "Ветер: "); strcat (strfinal, yes); }
	FillArea (x, y+h*5, wd[5], h, WHITE);
	DrawString (x, y+h*5, strfinal);
	
	
	strcpy (strfinal, "");
	if (BONUSFLAG == 0) { strcpy(strfinal, "Бонусы: "); strcat (strfinal, no); }
	if (BONUSFLAG == 1) { strcpy(strfinal, "Бонусы: "); strcat (strfinal, yes); }
	FillArea (x, y+h*6, wd[6], h, WHITE);
	DrawString (x, y+h*6, strfinal);	
	
	
	DrawString (x, y+h*7, "Ориентация экрана");
	DrawString (x, y+h*8, "Назад");

}

// initial draw of options screen
void show_options_init () {
	int i;
	ClearScreen();
	draw_trubitmap(0,0,&optionspic);
	for (i=0; i<=13; i++) DrawRect(0+i,0+i,800-2*i,600-2*i,ret_4num(16*(13-i)));

}

// here we check equal number of players, if COMMANDBATTLES == 1
// and if all players are comps
// returns 1, if we can start or prints warning and returns 0, if we cannot
int validate_start() {
	int i;
	int cnthuman=0, cntcomputer=0;
	
	SetFont(OpenFont("LiberationSans", 22, 1), BLACK);
	for (i=0; i<PLAYERS; i++) 
		if (IPLAYER[i].cplayer == 0) cnthuman++;
		else cntcomputer++;
	if (COMMANDBATTLES == 1 && cnthuman != cntcomputer) {
		DrawTextRect (362, 20, 800-362-15, 60, "При командной игре у команд должно быть равное количество игроков!", ALIGN_CENTER);
		PartialUpdate (362, 20, 800-362-15, 60);
		return 0;
	}
	if (cntcomputer == PLAYERS) {
		DrawTextRect (362, 20, 800-362-15, 60, "На поле одни компьютерные игроки! Супермозг перегружен!", ALIGN_CENTER);
		PartialUpdate (362, 20, 800-362-15, 60);
		return 0;
	}
	return 1;
}

// handler for options screen
int options_handler (int type, int par1, int par2) {
	int xmenu=120, ymenu=70, hmenu=28;
	static int posmenu = 1;
	int posmenupr;
	int wd[9] = {387, 459, 380, 342, 392, 129, 143, 232, 75};//widths of lines of menu
	int maxwidth = 0, i;
	
	for (i=0; i<5; i++) if (wd[i] > maxwidth) maxwidth = wd[i];	
	if (type == EVT_SHOW) {
		show_options_init();
		draw_options_menu(xmenu, ymenu, hmenu, posmenu, wd);
		init_mainmenu_choice (xmenu, ymenu, hmenu, posmenu, wd);
		FullUpdate();
		FineUpdate();
	}
	if (type == EVT_POINTERUP) {
		if (par1 >= xmenu && par1 <= xmenu+maxwidth && par2 >= ymenu && par2 <= ymenu+9*hmenu) {
			posmenupr = posmenu;
			posmenu = 1+(par2-ymenu)/hmenu;
			mainmenu_update (xmenu, ymenu, hmenu, posmenu, posmenupr, wd);
			if (posmenu == 1) {
				if (IMMEDIATEDROP == 0) IMMEDIATEDROP = 1;
				else IMMEDIATEDROP = 0;
				draw_options_menu(xmenu, ymenu, hmenu, posmenu, wd);
				init_mainmenu_choice (xmenu, ymenu, hmenu, posmenu, wd);
				PartialUpdateBW (xmenu, ymenu, maxwidth, hmenu*9);
			}
			if (posmenu == 2) {
				if (IMMEDIATETRAECTORY == 0) IMMEDIATETRAECTORY = 1;
				else IMMEDIATETRAECTORY = 0;
				draw_options_menu(xmenu, ymenu, hmenu, posmenu, wd);
				init_mainmenu_choice (xmenu, ymenu, hmenu, posmenu, wd);
				PartialUpdateBW (xmenu, ymenu, maxwidth, hmenu*9);
			}
			if (posmenu == 3) {
				if (REFLECTWALLS == 0) REFLECTWALLS = 1;
				else REFLECTWALLS = 0;
				draw_options_menu(xmenu, ymenu, hmenu, posmenu, wd);
				init_mainmenu_choice (xmenu, ymenu, hmenu, posmenu, wd);
				PartialUpdateBW (xmenu, ymenu, maxwidth, hmenu*9);
			}
			if (posmenu == 4) {
				if (TRANSPARENTSIDEWALLS == 0) TRANSPARENTSIDEWALLS = 1;
				else TRANSPARENTSIDEWALLS = 0;
				draw_options_menu(xmenu, ymenu, hmenu, posmenu, wd);
				init_mainmenu_choice (xmenu, ymenu, hmenu, posmenu, wd);
				PartialUpdateBW (xmenu, ymenu, maxwidth, hmenu*9);				
			}
			if (posmenu == 5) {
				if (COMMANDBATTLES == 0) COMMANDBATTLES = 1;
				else COMMANDBATTLES = 0;
				draw_options_menu(xmenu, ymenu, hmenu, posmenu, wd);
				init_mainmenu_choice (xmenu, ymenu, hmenu, posmenu, wd);
				PartialUpdateBW (xmenu, ymenu, maxwidth, hmenu*9);				
			}
			if (posmenu == 6) {
				if (WINDKOEF == 0) WINDKOEF = 1;
				else WINDKOEF = 0;
				draw_options_menu(xmenu, ymenu, hmenu, posmenu, wd);
				init_mainmenu_choice (xmenu, ymenu, hmenu, posmenu, wd);
				PartialUpdateBW (xmenu, ymenu, maxwidth, hmenu*9);				
			}
			if (posmenu == 7) {
				if (BONUSFLAG == 0) BONUSFLAG = 1;
				else BONUSFLAG = 0;
				draw_options_menu(xmenu, ymenu, hmenu, posmenu, wd);
				init_mainmenu_choice (xmenu, ymenu, hmenu, posmenu, wd);
				PartialUpdateBW (xmenu, ymenu, maxwidth, hmenu*9);				
			}
			if (posmenu == 8) {
				if (ORIENTATION == 2) ORIENTATION = 1;
				else ORIENTATION = 2;
				SetOrientation (ORIENTATION);
				show_options_init();
				draw_options_menu(xmenu, ymenu, hmenu, posmenu, wd);
				init_mainmenu_choice (xmenu, ymenu, hmenu, posmenu, wd);
				FullUpdate();
				FineUpdate();				
			}
			if (posmenu == 9) { save_options(); SetEventHandler(main_handler); }
		}
	}
	if (type == EVT_KEYPRESS) {
		if (par1 == KEY_DOWN) {
			posmenupr = posmenu;
			posmenu++;
			if (posmenu > 9) posmenu = 1;
			mainmenu_update (xmenu, ymenu, hmenu, posmenu, posmenupr, wd);
		}
		if (par1 == KEY_UP) {
			posmenupr = posmenu;
			posmenu--;
			if (posmenu < 1) posmenu = 9;
			mainmenu_update (xmenu, ymenu, hmenu, posmenu, posmenupr, wd);
		}
		if (par1 == KEY_RIGHT || par1 == KEY_LEFT) {
			if (posmenu == 1) {
				if (IMMEDIATEDROP == 0) IMMEDIATEDROP = 1;
				else IMMEDIATEDROP = 0;
				draw_options_menu(xmenu, ymenu, hmenu, posmenu, wd);
				init_mainmenu_choice (xmenu, ymenu, hmenu, posmenu, wd);
				PartialUpdateBW (xmenu, ymenu, maxwidth, hmenu*9);
			}
			if (posmenu == 2) {
				if (IMMEDIATETRAECTORY == 0) IMMEDIATETRAECTORY = 1;
				else IMMEDIATETRAECTORY = 0;
				draw_options_menu(xmenu, ymenu, hmenu, posmenu, wd);
				init_mainmenu_choice (xmenu, ymenu, hmenu, posmenu, wd);
				PartialUpdateBW (xmenu, ymenu, maxwidth, hmenu*9);
			}
			if (posmenu == 3) {
				if (REFLECTWALLS == 0) REFLECTWALLS = 1;
				else REFLECTWALLS = 0;
				draw_options_menu(xmenu, ymenu, hmenu, posmenu, wd);
				init_mainmenu_choice (xmenu, ymenu, hmenu, posmenu, wd);
				PartialUpdateBW (xmenu, ymenu, maxwidth, hmenu*9);
			}
			if (posmenu == 4) {
				if (TRANSPARENTSIDEWALLS == 0) TRANSPARENTSIDEWALLS = 1;
				else TRANSPARENTSIDEWALLS = 0;
				draw_options_menu(xmenu, ymenu, hmenu, posmenu, wd);
				init_mainmenu_choice (xmenu, ymenu, hmenu, posmenu, wd);
				PartialUpdateBW (xmenu, ymenu, maxwidth, hmenu*9);				
			}
			if (posmenu == 5) {
				if (COMMANDBATTLES == 0) COMMANDBATTLES = 1;
				else COMMANDBATTLES = 0;
				draw_options_menu(xmenu, ymenu, hmenu, posmenu, wd);
				init_mainmenu_choice (xmenu, ymenu, hmenu, posmenu, wd);
				PartialUpdateBW (xmenu, ymenu, maxwidth, hmenu*9);				
			}
			if (posmenu == 6) {
				if (WINDKOEF == 0) WINDKOEF = 1;
				else WINDKOEF = 0;
				draw_options_menu(xmenu, ymenu, hmenu, posmenu, wd);
				init_mainmenu_choice (xmenu, ymenu, hmenu, posmenu, wd);
				PartialUpdateBW (xmenu, ymenu, maxwidth, hmenu*9);				
			}
			if (posmenu == 7) {
				if (BONUSFLAG == 0) BONUSFLAG = 1;
				else BONUSFLAG = 0;
				draw_options_menu(xmenu, ymenu, hmenu, posmenu, wd);
				init_mainmenu_choice (xmenu, ymenu, hmenu, posmenu, wd);
				PartialUpdateBW (xmenu, ymenu, maxwidth, hmenu*9);				
			}
		}
		if (par1 == KEY_OK) {
			if (posmenu == 8) {
				if (ORIENTATION == 2) ORIENTATION = 1;
				else ORIENTATION = 2;
				SetOrientation (ORIENTATION);
				show_options_init();
				draw_options_menu(xmenu, ymenu, hmenu, posmenu, wd);
				init_mainmenu_choice (xmenu, ymenu, hmenu, posmenu, wd);
				FullUpdate();
				FineUpdate();				
			}
			if (posmenu == 9) { save_options(); SetEventHandler(main_handler); }
		}
	}
	return 0;
}

// handler for screen after pressing START in main menu or exiting from virtual keyboard
int start_handler (int type, int par1, int par2) {
	int xmenu, ymenu, wmenu, hmenu;
	static int posmenu = 1;
	static int posmenupr = 1;
	int i, corh=0;
	char comporhuman [90];
	xmenu = 60;
	ymenu = 72;
	wmenu = 262;
	hmenu = 27;
	
	if (type == EVT_SHOW) {
		ClearScreen();
		draw_trubitmap (0,0,&nameplayerspic);
		SetFont(OpenFont("LiberationSans", 17, 0), BLACK);
		DrawTextRect (xmenu, ymenu-63, wmenu, 70, "влево/вправо/нажать - человек-компьютер, ОК или длинное нажатие - сменить имя", ALIGN_CENTER);
		SetFont(OpenFont("LiberationSans", 40, 0), BLACK);		
		DrawTextRect (xmenu+wmenu+30, ymenu+30, 800-(xmenu+wmenu+30+15), 120, "Профессор смотрит за тобой!", ALIGN_CENTER);
		SetFont(OpenFont("LiberationSans", 24, 0), BLACK);
		posmenu = 1;
		posmenupr = 1;
		for (i=1; i<=PLAYERS; i++) {
			if (IPLAYER[i-1].cplayer == 1) strcpy (comporhuman, "Супермозг");
			if (IPLAYER[i-1].cplayer == 0) strcpy (comporhuman, "Человек");
			DrawString (xmenu, ymenu+(i-1)*hmenu, NPLAYERS[i-1]);
			DrawString (xmenu+130, ymenu+(i-1)*hmenu, comporhuman);
		}
		DrawString (xmenu, ymenu+PLAYERS*hmenu, "Начать бой");
		DrawString (xmenu, ymenu+(PLAYERS+1)*hmenu, "Загрузить игру");
		DrawString (xmenu, ymenu+(PLAYERS+2)*hmenu, "Выйти в меню");
		init_menu_choice (xmenu, ymenu, wmenu, hmenu, posmenu);
		for (i=0; i<=13; i++) DrawRect(0+i,0+i,800-2*i,600-2*i,ret_4num(16*(13-i)));
		FullUpdate();
		FineUpdate();
	}
	
	if (type == EVT_POINTERUP) {
		if (par1 >= xmenu && par1 <= xmenu+wmenu && par2 >= ymenu && par2 <= ymenu + (PLAYERS+3)*hmenu) {
			posmenupr = posmenu;
			posmenu = 1+(par2-ymenu)/hmenu;
			menu_update (xmenu, ymenu, wmenu, hmenu, posmenu, posmenupr);
			if (posmenu <= PLAYERS) {
				if (IPLAYER[posmenu-1].cplayer == 0) corh=1;
				if (IPLAYER[posmenu-1].cplayer == 1) corh=0;
				IPLAYER[posmenu-1].cplayer = corh;
				redraw_players_menu (xmenu, ymenu, wmenu, hmenu, posmenu);				
			}
			if (posmenu == PLAYERS+3) { SetEventHandler(main_handler); return 0; }
			if (posmenu == PLAYERS+2) {
				load_game();
				SetOrientation(ORIENTATION);
				STARTFROMBEGINNING = 1;
				posmenu = 1;
				for (i=0;i<2;i++) { BONUSES[i].x=0; BONUSES[i].y=0; }
				SetEventHandler(game_handler);
				return 0;				
			}
			if (posmenu == PLAYERS+1) {
				if (validate_start() == 1) {
					fill_arena();
					posmenu = 1;
					STARTFROMBEGINNING = 1;
					SetEventHandler(game_handler);
					return 0;
				}
			}			
		}
	}
	
	if (type == EVT_POINTERLONG || type == EVT_POINTERHOLD) {
		if (par1 >= xmenu && par1 <= xmenu+wmenu && par2 >= ymenu && par2 <= ymenu + (PLAYERS+3)*hmenu) {
			posmenupr = posmenu;
			posmenu = 1+(par2-ymenu)/hmenu;
			menu_update (xmenu, ymenu, wmenu, hmenu, posmenu, posmenupr);
			if (posmenu <= PLAYERS) {
				CHOICE = posmenu;
				SetEventHandler (edit_player_handler);
				return 0;
			}			
		}
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
			if (posmenu < 1) posmenu = PLAYERS+3;
			menu_update (xmenu, ymenu, wmenu, hmenu, posmenu, posmenupr);
		}
		if (par1 == KEY_DOWN) {
			posmenupr = posmenu;
			posmenu++;
			if (posmenu > PLAYERS+3) posmenu = 1;
			menu_update (xmenu, ymenu, wmenu, hmenu, posmenu, posmenupr);
		}
		if (par1 == KEY_OK || par1 == KEY_BACK) {
			if (posmenu == PLAYERS+3) { SetEventHandler(main_handler); return 0; }
			if (posmenu == PLAYERS+2) {
				load_game();
				SetOrientation(ORIENTATION);
				STARTFROMBEGINNING = 1;
				posmenu = 1;
				for (i=0;i<2;i++) { BONUSES[i].x=0; BONUSES[i].y=0; }
				SetEventHandler(game_handler);
				return 0;				
			}
			if (posmenu == PLAYERS+1) {
				if (validate_start() == 1) {
					fill_arena();
					posmenu = 1;
					STARTFROMBEGINNING = 1;
					SetEventHandler(game_handler);
					return 0;
				}
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

void show_instruction (int x, int y, int w, int h, int page) {
	char *strup;
	char str[5];
		
	strup = (char*)malloc(sizeof(char)*100);
	strcpy(str, "");
	strcpy(strup, "вправо/влево - листать, ОК - выход в меню    стр.");
	itos (str, page);
	strcat(strup, str);
	strcat(strup, " из 21");
	
	FillArea(x,y-20,w,h+20,WHITE);
//	DrawRect(x,y,w,h,BLACK);
	
	SetFont(OpenFont("LiberationSerif", 14, 0), BLACK);
	DrawString (x+100, y-20, strup);
	SetFont(OpenFont("LiberationSerif", 20, 0), BLACK);

	if (page == 1) DrawTextRect (x, y, w, h, "    Не все ладно на планете Дезоксирибонуклотрон. Ужасные войны, разруха и голод, "
	"невозможность с первого раза выговорить свое название, а также отвратительная погода сделали свое дело - мир вышел из Земной Империи, "
	"объявил о независимости и даже начал печатать свои, полные патриотического гнева и пикантных подробностей, желтые газетенки. Улицы были "
	"полны людьми, роботами и даже местными полуразумными туземцами, смахивающими на волосатые вантузы, когда громкоговорители объявили о выходе планеты "
	"из под власти земного правительства. Радости не было конца, все орали как сумасшедшие, обнимались и надеялись, что уж теперь-то прекратятся все их беды, "
	"жизнь начнет бить ключом в этом космическом Мухосранске, а местные бордели откроются и для роботов.", ALIGN_LEFT);
	if (page == 2) DrawTextRect (x, y, w, h, "    Радость эта было недолгой. Космические слизни из соседней системы прознали про то, что планета "
	"уже не находится под защитой земных дредноутов и нагрянули в гости. Потоки зеленой каши, напалма и субстационарной квазистенции обрушились на все "
	"значимые города. По улицам носились обезумевшие люди, выла авиационная сирена, а роботы, воспользовавшись суматохой, занимали бордели. Воцарился хаос. "
	"И никто не знал, что все происходящее является частью коварного замысла сумасшедшего профессора, живущего в горах и лелеющего единственную мечту - наконец-то "
	"испытать свое адское изобретение, темпоральный пульсатор. Выход из империи, наводка для слизней - все имело под собой цель восстановить закрытый военными "
	"проект и начать испытания.", ALIGN_LEFT);
	if (page == 3) DrawTextRect (x, y, w, h, "    И вот профессор, надев свой лучший халат и причесав лысину, направился в бункер правительства. Он не боялся слизней, "
	"ибо кто же их боится с плазменных дезинтегратором за плечами? По пути его ждало много приключений, живые зловонные лужи, смердящие ховер-танки пришельцев и "
	"даже один голубой робот. Но наконец, грязный, уставший и злой он добрался до бункера. Посмотрел на черное от дыма небо, вздохнул, отбросил опостылевший дезинтегратор "
	"и начал переговоры с чиновниками. Они с радостью приняли его предложение, лишь только услышав, что новое оружие раз и навсегда выбьет слизней с планеты. "
	"И профессор начал подготовку...", ALIGN_LEFT);
	if (page == 4) DrawTextRect (x, y, w, h, "    Из объяснений ученого выходило, что его пульсатор закольцовывает тринадцатое измерение таким образом, что "
	"объект фактически замыкает причинно-следственную связь и выпадает из текущей временной плоскости. Поскольку никто не понимал, ни что это значит, ни, самое главное, "
	"что это даст в военном плане, подготовка и испытания полностью легли на хрупкие плечи профессора. Он затребовал - и получил - полигон для испытаний, "
	"дюжину роботов и кошку (по его словам, для проверки квазистационарной теории эллиптического микроколлапсара). Застучали молотки, загудели дрели, грузовики с "
	"песком и глиной возводили насыпи - работа кипела. И вот настал день, знаменательный день, когда профессор запросил для экспериментов человека - оператора "
	"управления.", ALIGN_LEFT);
	if (page == 5) DrawTextRect (x, y, w, h, "    Этот человек - вы. Младший сержант 317-й танковой дивизии, холост, родственников нет, сирота, выращенный дикими роботами. "
	"Из путанных объяснений профессора вы вынесли несколько важных моментов. 1-й: ученый спит (во всех смыслах) с кошкой. 2-й: управлять техникой придется дистанционно, "
	"из рубки на вершине полигона. 3-й: пульсатор замораживает время, потому условные противники не стреляют, пока наводишься и стреляешь ты. Условности эти "
	"призваны как можно более точно приблизить картину боя к настоящей, потому профессор добавил еще одну трудность - пульсатор срабатывает на все танки по очереди. ", ALIGN_LEFT);
	if (page == 6) DrawTextRect (x, y, w, h, "    Создается некая очередность стрельбы, когда все танки заморожены, а один оставшийся ведет бой. Изучение рубки управления "
	"и механики боя выявило несколько дополнительных подробностей. Условный выстрел состоит из двух фаз: 1-я - выбор оружия (в свой ход это можно сделать только один раз), "
	"и 2-я - выбор угла и силы выстрела соответственно ландшафту, силе ветра и местонахождению противника. Танк стреляет, и... Либо попадает или нет. До начала боя можно "
	"выбрать, какие из танков будут управляться человеком, а какие - нейронным супермозгом полигона. Также за попадания выдаются условные денежные единицы, которые между "
	"раундами можно тратить на улучшение вооружения (по словам ученого, денежная основа улучшения танка будет способствовать более рациональному "
	"расходу боеприпасов).", ALIGN_LEFT);
	if (page == 7) DrawTextRect (x, y, w, h, "    Итак, вперед, сержант! Настало время отточить свое мастерство и надрать зеленые задницы тупым пришельцам! "
	"Мы, свободное человечество Дезоксирибонуклотрона, верим в тебя!\n"
	"_______________________________________________________", ALIGN_LEFT);
	if (page == 8) DrawTextRect (x, y, w, h, "    Теперь поговорим о самой игре и ее механике. Как и любая игра, Tank War начинается с главного меню. По меню можно ходить с "
	"помощью кнопок \"вверх\" и \"вниз\", менять какие-либо значения - \"вправо\" и \"влево\", зайти внутрь пункта - кнопка \"ОК\". Для начала вам стоит определиться со сложностью. "
	"Если вы никогда в жизни не играли в баллистические танки, советуем поставить сложность в \"Легко\", в противном случае стоит начать со \"Средне\". "
	"Количество игроков выбирайте на свое усмотрение, но если их 2 - играть не очень интересно, ибо приходится целиться через все поле и партии растягиваются на довольно "
	"длительное время.\n    Отдельного рассмотрения стоят пункты \"Настройка\" и \"Статистика\". Начнем с первого.", ALIGN_LEFT);
	if (page == 9) DrawTextRect (x, y, w, h, "    Настройка.\n\n"
	"    Пройдемся последовательно по всем настройкам и опишем, за что они отвечают.\n"
	"    Мгновенное падение танков. Отвечает за то, будут ли танки в начале раунда оказываться мгновенно на ландшафте - или постепенно на него падать. Можно отключить без какого-"
	"либо ущерба геймплею.\n"
	"    Мгновенная отрисовка траекторий. Тут все немного сложнее. Как можно догадаться, чем быстрее скорость у снаряда, тем быстрее он должен перемещаться по экрану. Если эту опцию "
	"поставить в \"да\" - траектории полета будут рисоваться сразу же после выстрела и ни о какой эмуляции скорости снаряда речи быть", ALIGN_LEFT);
	if (page == 10) DrawTextRect (x, y, w, h, "не может. Если же опцию изменить на \"нет\" - "
	"снаряд полетит к цели постепенно, причем именно визуально скорость перемещения будет зависеть от скорости снаряда. Фактически эта настройка тоже не влияет на геймплей, однако "
	"частенько бывает удобно судить о промахах и попаданиях еще и по скорости в тот или иной момент времени. Так что - палка о двух концах. Порекомендуем все-таки поставить в \"нет\"\n"
	"    Отражение от боковых стен. Игровое поле у нас ограничено слева и справа, так что эта настройка отвечает за поведение снаряда при ударе о боковую стенку. "
	"\"да\" - снаряд отскакивает от боковой стены и продолжает полет, и \"нет\" - снаряд появляется с противоположной стороны игрового поля и опять таки продолжает свой смертоносный путь. "
	"Каких-либо рекомендаций", ALIGN_LEFT);
	if (page == 11) DrawTextRect (x, y, w, h, "тут дать невозможно, каждый решает для себя сам.\n"
	"    Замкнутое игровое поле. Настройка, значительно меняющая весь игровой процесс. Если ее поставить в \"нет\" - она \"выключает\" боковые стены, так что теперь снаряды "
	"могут навсегда улететь из зоны видимости. Нельзя стрелять рикошетом, нельзя подлавливать игроков с противоположной стороны поля - гораздо больше \"нельзя\", чем \"можно\", однако "
	"и тут можно привести некоторые доводы в пользу выключения боковых стен. Например - реалистичность, отскоки все-таки попахивают аркадой, а не баллистическими изысками. Однако если вы "
	"хотите побольше драйва и неожиданностей, рекомендуем здесь поставить \"да\".\n"
	"    Командная игра компьютера. Определяет стиль игры с компьютерными оппонентами. Если включено - компьютерные", ALIGN_LEFT);
	if (page == 12) DrawTextRect (x, y, w, h, "танки объединяются против людских и счет игры идет не на личные победы, а на победы той или иной команды. "
	"Если здесь стоит \"нет\" - счет идет на личные победы и каждый игрок сражается сам за себя. Статистику командных и личных побед можно посмотреть в пункте \"Статистика\" главного меню.\n"
	"    Ветер. Включает или отключает ветер. С ветром играть гораздо интереснее, но и сложнее, без ветра - скучнее и проще.\n"
	"    Бонусы. Будут ли появляться на игровом поле случайные бонусы. Бонус выглядит как мятая псевдо-банка, и если в него "
	"попасть взрывающимся снарядом, это даст попавшему то, что было внутри. Рекомендуется включить - так интереснее.\n"
	"    Ориентация экрана. Меняет ориентацию экрана.", ALIGN_LEFT);
	if (page == 13) DrawTextRect (x, y, w, h, "    Теперь заглянем внутрь пункта \"Статистика\" главного меню.\n"
	"    Здесь можно посмотреть как личную, так и командную статистику игры. Обратим внимание на то, что при выходе из игры в главное меню личная статистика игроков обнуляется и остается только вот "
	"в этой табличке, тогда как командная учитывается постоянно. Так что сохраняйтесь перед выходом, если не хотите личный счет побед начать заново.", ALIGN_LEFT);
	if (page == 14) DrawTextRect (x, y, w, h, "    Если в главном меню выбрать пункт \"Старт\" - мы перейдем к экрану настройки игроков. "
	"Тут можно поменять имя каждого игрока и кто им будет управлять - человек или компьютер. "
	"Ну и наконец - пункт \"Начать бой\"! Он начинает саму игру.\n"
	"    Tank War - игра пошаговая, каждому игроку в порядке очереди "
	"предлагается сделать выстрел с учетом скорости ветра. Предварительно необходимо выбрать оружие, силу и угол наклона дула танка. Каждый ход состоит из двух фаз:\n"
	"    1-я фаза - выбор оружия. Для этого используются кнопки \"вправо\" и \"влево\". Индикатор вверху экрана показывает текущий выбор, а кнопка \"ОК\" - подтверждает его. "
	"Внимание! Выбрав оружие, мы не будем иметь возможность передумать и выбрать другое, пока не закончится ход, так что будьте внимательны.", ALIGN_LEFT);
	if (page == 15) DrawTextRect (x, y, w, h, "    Если же у вас модель PocketBook 302 (с тачскрином), тогда выбор оружия производится нажатием на соответствующую "
	"иконку вверху экрана. Подтверждение выбора - большая кнопка на лицевой панели устройства.", ALIGN_LEFT);
	if (page == 16) DrawTextRect (x, y, w, h, "    2-я фаза - выбор угла наклона и силы выстрела. Кнопки \"вправо\" и \"влево\" отвечают на угол, кнопки \"вверх\" и \"вниз\" - за силу. Если нажать и удерживать любую из этих кнопок, "
	"значение начнет меняться в 10 раз быстрее. Индикацию угла и силы можно увидеть на панели вверху экрана. Там же можно найти силу ветра и его направление, количество денег у текущего игрока, "
	"количество оставшихся в живых людей в танке (аналог жизни) и имя. Кроме того над каждым из оставшихся в живых игроков вверху экрана видно его имя и полоску жизни.", ALIGN_LEFT);
	if (page == 17) DrawTextRect (x, y, w, h, "    И опять про тачскрин - угол (как и силу выстрела) можно выбрать двумя способами. 1-й - нажатие на любой части экрана, кроме верхней панели, "
	"задает угол выстрела, который отображается на экране в виде двух отрезков, символизирующих угол. Если же нажать на эран, провести пальцем(стилусом) некоторое расстояние "
	"и отпустить - длина проведенного отрезка будет соответствовать силе выстрела. 2-й способ - нажатие на соответствующую иконку на верхней панели (угол или сила) вызывает "
	"на экран стрелки более точной регулировки силы или угла, нажимая на которые, мы меняем соответствующие значения. Повторное нажатие на иконку стрелки убирает. "
	"Выстрел с выбранными значениями происходит после нажатия на большую кнопку на лицевой панели устройства.", ALIGN_LEFT);
	if (page == 18) DrawTextRect (x, y, w, h, "    По окончании настройки танка мы нажимаем на \"ОК\" - и происходит выстрел. Если выстрел был удачным - "
	"к денежному запасу игрока прибавляется соответствующая сумма. Далее в порядке очереди начинает стрелять следующий игрок. "
	"Раунд заканчивается, если на поле осталось меньше двух игроков, либо (при командном режиме игры) если остались игроки только одной команды. Победитель получает денежную компенсацию "
	"и возможно запись в таблицу рекордов, а игроки переходят в магазин.\n"
	"    В магазине каждый из игроков имеет возможность на заработанные деньги прикупить себе пушку помощнее. Тут стоит помнить, что иногда не стоит покупать себе кучу дешевого барахла, "
	"а вместо этого лучше сыграть еще пару раундов", ALIGN_LEFT);
	if (page == 19) DrawTextRect (x, y, w, h, "и накопить на что-нибудь более значительное. Компьютерные игроки своими финансами распоряжаются самостоятельно.\n"
	"Когда все игроки завершают покупки, стартует следующий раунд.\n    В игре есть игровое меню, его можно вызвать (если текущий игрок - человек) кнопками \"Плюс\" или \"Корзина\", "
	"кому как удобнее в зависимости от ориентации экрана. Для PocketBook 360 и Pocketbook 302 добавлена дополнительная кнопка - \"Предыдущая страница\", поскольку ни \"Плюс\", ни \"Корзина\" там попросту нет.\n"
	"    В меню мы имеем возможность выбрать:\n"
	"    FullUpdate - делает полное обновление экрана. Актуально, если раунд сильно затянулся и экран уже весь исчерчен призраками траекторий и взрывов.", ALIGN_LEFT);
	if (page == 20) DrawTextRect (x, y, w, h, "    Сохранить - сохраняет текущую игру, чтобы потом была возможность продолжить с текущего момента.\n"
	"    Загрузить - загружает предварительно сохраненную игру.\n"
	"    Sudden Death - кто знаком с хоккеем, знает, что термин этот - \"игра до первого гола\". При его выборе у всех без исключения игроков счетчик жизней сбрасывается до единицы.\n"
	"    Вернуться - вернуться в игру.\n"
	"    Выйти в меню - выйти в главное меню. Счетчики личных побед сбрасывается (они остаются только в таблице рекордов), так что не забудьте сохраниться перед этим!\n"
	"    Вот вроде и все. Играйте и побеждайте!\n"
	"    __________________________________________________\n    Далее благодарности и контакты с автором.", ALIGN_LEFT);
	if (page == 21) DrawTextRect (x, y, w, h, "   Во-первых, хочется выразить благодарность Курганову Эдуарду за его идею по оптимизации подсчета всех траекторий полета снаряда при "
	"выстреле компьютерным игроком. Далее выражу благодарность Яковлеву Николаю, моему брату, который нашел несколько интересных багов в игре и предложил пару идей. Ну и  "
	"остальным (Кускову Александру, Лебедеву Антону и пр.), которые верили в это проект и поддерживали автора, а также предложили несколько идей по улучшению геймплея.\n"
	"    Автор игры, Евгений Яковлев. Со мной можно связаться посредством e-mail: jackovle@mail.ru или katehizis@mail.ru, сюда же можно слать замечания и предложения.\n"
	"    Коммерчески распространять эту работу без ведома автора строго запрещено!"
	"", ALIGN_LEFT);
	PartialUpdateBW (x, y-20, w, h+20);
	free(strup);
}

// handler for instruction screen
int instruction_handler(int type, int par1, int par2) {
	static int pos=1;
	int x=50,y=50,w=600,h=305;
	int xl=50, yl=360, wl=50, hl=50; //coords for pointer left for touch
	int xr=600, yr=360, wr=50, hr=50; //coords for pointer right for touch
	
	
	if (type == EVT_SHOW) {
		ClearScreen();
		draw_trubitmap (0,0,&instructionpic);
		show_instruction (x, y, w, h, pos);
		draw_touchpointer (xl, yl, wl, hl, 3, BLACK);
		draw_touchpointer (xr, yr, wr, hr, 1, BLACK);
		FullUpdate();
		FineUpdate();
	}
	if (type == EVT_POINTERUP) {
		if (par1 >= xl && par1 <= xl+wl && par2 >= yl && par2 <= yl+hl) {
			pos--;
			if (pos < 1) pos = 1;
			show_instruction (x, y, w, h, pos);
		}
		if (par1 >= xr && par1 <= xr+wr && par2 >= yr && par2 <= yr+hr) {
			pos++;
			if (pos > 21) pos = 21;
			show_instruction (x, y, w, h, pos);
		}
	}
	if (type == EVT_KEYPRESS) { 
		if (par1 == KEY_RIGHT) {
			pos++;
			if (pos > 19) pos = 19;
			show_instruction (x, y, w, h, pos);
		}
		if (par1 == KEY_LEFT) {
			pos--;
			if (pos < 1) pos = 1;
			show_instruction (x, y, w, h, pos);
		}
		if (par1 == KEY_OK || par1 == KEY_BACK) {  pos = 1; SetEventHandler(main_handler); return 0; }
	}
	return 0;
}

//handler for hiscores
int hiscores_handler(int type, int par1, int par2) {
	int i,j;
	char strhumanwins[60], strcomputerwins[60];
	char temp[40];
	int tablex=100, tabley=140, tableh=40;
	
	if (type == EVT_SHOW) {
		ClearScreen();
		for (i=0; i<=15; i++) DrawRect(0+i,0+i,800-2*i,600-2*i,ret_4num(16*i));
		strcpy (strcomputerwins, "Побед компьютерной команды - ");
		strcpy (strhumanwins, "Побед человеческой команды - ");
		SetFont(OpenFont("LiberationSans", 30, 1), BLACK);
		strcpy (temp, "");
		itos (temp, COMPWINS);
		strcat (strcomputerwins, temp);
		strcpy (temp, "");
		itos (temp, HUMANWINS);
		strcat (strhumanwins, temp);
		for (i=15; i<=400; i++) {
			
		}
		DrawTextRect (100, 30, 600, 100, strcomputerwins, ALIGN_CENTER);
		DrawTextRect (100, 70, 600, 100, strhumanwins, ALIGN_CENTER);
		
		for (i=0; i<10; i++) {
//			DrawRect (tablex, tabley+i*tableh, 600, 40, BLACK);
			for (j=0; j<=3; j++) {
				if (i > 0) {
					DrawLine (tablex+j, tabley+i*tableh+j, tablex+600-j, tabley+i*tableh+j, ret_4num(j*5*16));
					DrawLine (tablex+j, tabley+i*tableh-j, tablex+600-j, tabley+i*tableh-j, ret_4num(j*5*16));
					DrawLine (tablex+600-j, tabley+(i-1)*tableh+j, tablex+600-j, tabley+i*tableh-j, ret_4num(j*5*16));
					DrawLine (tablex+600+j, tabley+(i-1)*tableh-j, tablex+600+j, tabley+i*tableh-j, ret_4num(j*5*16));
					DrawLine (tablex+j, tabley+(i-1)*tableh+j, tablex+j, tabley+i*tableh-j, ret_4num(j*5*16));
					DrawLine (tablex-j, tabley+(i-1)*tableh-j, tablex-j, tabley+i*tableh-j, ret_4num(j*5*16));
				}
			}
			strcpy(temp, HISCORES[i].name);
			DrawTextRect (tablex, tabley+i*tableh, 180, 40, temp, ALIGN_CENTER);
			if (HISCORES[i].cplayer == 0) strcpy (temp, "Человек");
			if (HISCORES[i].cplayer == 1) strcpy (temp, "Супермозг");
			DrawTextRect (tablex+180, tabley+i*tableh, 220, 40, temp, ALIGN_CENTER);
			strcpy (temp, "");
			itos (temp, HISCORES[i].wins);
			if (HISCORES[i].wins%10 == 0 || (HISCORES[i].wins%10 >= 5 && HISCORES[i].wins%10 <= 9)) strcat(temp, " побед");
			if (HISCORES[i].wins%10 == 1) strcat(temp, " победа");
			if (HISCORES[i].wins%10 >= 2 && HISCORES[i].wins%10 <= 4) strcat(temp, " победы");
			if (HISCORES[i].wins >= 11 && HISCORES[i].wins <= 19) { strcpy (temp, ""); itos (temp, HISCORES[i].wins); strcat(temp, " побед"); }
			DrawTextRect (tablex+400, tabley+i*tableh, 200, 40, temp, ALIGN_CENTER);
		}
		for (j=0; j<=3; j++) {
			DrawLine (tablex+j, tabley+j, tablex+600-j, tabley+j, ret_4num(j*5*16));
			DrawLine (tablex-j, tabley-j, tablex+600+j, tabley-j, ret_4num(j*5*16));
			DrawLine (tablex+j, tabley+10*tableh-j, tablex+600-j, tabley+10*tableh-j, ret_4num(j*5*16));
			DrawLine (tablex-j, tabley+10*tableh+j, tablex+600+j, tabley+10*tableh+j, ret_4num(j*5*16));
			DrawLine (tablex+j, tabley+9*tableh+j, tablex+j, tabley+10*tableh-j, ret_4num(j*5*16));
			DrawLine (tablex-j, tabley+9*tableh-j, tablex-j, tabley+10*tableh+j, ret_4num(j*5*16));
			DrawLine (tablex+600+j, tabley+9*tableh-j, tablex+600+j, tabley+10*tableh+j, ret_4num(j*5*16));
			DrawLine (tablex+600-j, tabley+9*tableh+j, tablex+600-j, tabley+10*tableh-j, ret_4num(j*5*16));
		}
		for (i=1; i<=10; i++) {
			for (j=0; j<=3; j++) {
				DrawLine (tablex+180+j, tabley+(i-1)*tableh+j, tablex+180+j, tabley+i*tableh-j, ret_4num(j*5*16));
				DrawLine (tablex+180-j, tabley+(i-1)*tableh+j, tablex+180-j, tabley+i*tableh-j, ret_4num(j*5*16));
				DrawLine (tablex+400+j, tabley+(i-1)*tableh+j, tablex+400+j, tabley+i*tableh-j, ret_4num(j*5*16));
				DrawLine (tablex+400-j, tabley+(i-1)*tableh+j, tablex+400-j, tabley+i*tableh-j, ret_4num(j*5*16));
			}
		}
		FullUpdate();
		FineUpdate();
	}
	if (type == EVT_KEYPRESS || type == EVT_POINTERUP) { SetEventHandler(main_handler); return 0; }
	return 0;
}

// most fucking important handler in the program
int main_handler (int type, int par1, int par2) {
	static int posmenu=1;
	static int posmenupr=1;
	int xmenu = 70;
	int ymenu = 368;
//	int wmenu = 250; //not used
	int hmenu = 28;
	int mainwd[7] = {70, 120, 225, 155, 122, 135, 75}; //width of each line in main menu
	int wmenu=0;
	int i;
	
	for (i=0; i<7; i++) if (mainwd[i] > wmenu) wmenu = mainwd[i];
	
	if (type == EVT_SHOW) {
		ClearScreen();
		main_draw(xmenu, ymenu, hmenu);
		init_mainmenu_choice(xmenu, ymenu, hmenu, posmenu, mainwd);
		FullUpdate();
		FineUpdate();
	}
	if (type == EVT_POINTERUP) {
		if (par1 >= xmenu && par1 <= xmenu+wmenu && par2 >= ymenu && par2 <= ymenu + 7*hmenu) {
			posmenupr = posmenu;
			posmenu = 1 + (par2-ymenu)/hmenu;
			mainmenu_update (xmenu, ymenu, hmenu, posmenu, posmenupr, mainwd);
			if (posmenu == 7) { save_options(); CloseApp(); }
			if (posmenu == 6) { SetEventHandler(hiscores_handler); return 0; }
			if (posmenu == 5) { SetEventHandler(options_handler); return 0; }
			if (posmenu == 4) { SetEventHandler(instruction_handler); return 0; }
			if (posmenu == 2) {
				PLAYERS++;
				if (PLAYERS > 8) PLAYERS = 2;
				redraw_choice_menu (xmenu, ymenu, hmenu, posmenu, mainwd);
			}
			if (posmenu == 3) {
				DIFFICUILTY++;
				if (DIFFICUILTY > 3) DIFFICUILTY = 1;
				redraw_choice_menu (xmenu, ymenu, hmenu, posmenu, mainwd);
			}
			if (posmenu == 1) { SetEventHandler(start_handler); return 0; }			
		}
	}
	if (type == EVT_KEYPRESS) 	{
		switch (par1) {
			
			case KEY_UP:
				posmenupr = posmenu;
				posmenu--;
				if (posmenu < 1) posmenu = 7;
				mainmenu_update (xmenu, ymenu, hmenu, posmenu, posmenupr, mainwd);
				break;
			case KEY_DOWN:
				posmenupr = posmenu;
				posmenu++;
				if (posmenu > 7) posmenu = 1;
				mainmenu_update (xmenu, ymenu, hmenu, posmenu, posmenupr, mainwd);
				break;
			case KEY_OK:
				if (posmenu == 7) { save_options(); CloseApp(); }
				if (posmenu == 6) { SetEventHandler(hiscores_handler); return 0; }
				if (posmenu == 5) { SetEventHandler(options_handler); return 0; }
				if (posmenu == 4) { SetEventHandler(instruction_handler); return 0; }
				if (posmenu == 1) { SetEventHandler(start_handler); return 0; }
				break;
			case KEY_RIGHT:
				if (posmenu == 2) {
					PLAYERS++;
					if (PLAYERS > 8) PLAYERS = 2;
					redraw_choice_menu (xmenu, ymenu, hmenu, posmenu, mainwd);
				}
				if (posmenu == 3) {
					DIFFICUILTY++;
					if (DIFFICUILTY > 3) DIFFICUILTY = 1;
					redraw_choice_menu (xmenu, ymenu, hmenu, posmenu, mainwd);
				}
				break;
			case KEY_LEFT:
				if (posmenu == 2) {
					PLAYERS--;
					if (PLAYERS < 2) PLAYERS = 8;
					redraw_choice_menu (xmenu, ymenu, hmenu, posmenu, mainwd);
				}
				if (posmenu == 3) {
					DIFFICUILTY--;
					if (DIFFICUILTY < 1) DIFFICUILTY = 3;
					redraw_choice_menu (xmenu, ymenu, hmenu, posmenu, mainwd);
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
// additional
// filling HISCORES array
void init_NPLAYERS () {
	int i;
	for (i=0; i<8; i++) NPLAYERS[i] = (char*) malloc(10);
	strcpy (NPLAYERS[0], "player1");
	strcpy (NPLAYERS[1], "player2");
	strcpy (NPLAYERS[2], "player3");
	strcpy (NPLAYERS[3], "player4");
	strcpy (NPLAYERS[4], "player5");
	strcpy (NPLAYERS[5], "player6");
	strcpy (NPLAYERS[6], "player7");
	strcpy (NPLAYERS[7], "player8");
	
	for (i=0; i<10; i++) { HISCORES[i].wins = 0; HISCORES[i].cplayer = 0; }
	strcpy (HISCORES[0].name, "player1");
	strcpy (HISCORES[1].name, "player2");
	strcpy (HISCORES[2].name, "player3");
	strcpy (HISCORES[3].name, "player4");
	strcpy (HISCORES[4].name, "player5");
	strcpy (HISCORES[5].name, "player6");
	strcpy (HISCORES[6].name, "player7");
	strcpy (HISCORES[7].name, "player8");
	strcpy (HISCORES[8].name, "player8");
	strcpy (HISCORES[9].name, "player8");
}

// initing changable players' info, except names and x,y
void init_players_info() {
	int i,j;
	for (i=0; i<8; i++) {
		IPLAYER[i].angle = 90;
		IPLAYER[i].power = 150;
		IPLAYER[i].cplayer = 0;
		IPLAYER[i].life = 100;
		IPLAYER[i].x = 0; //recalculates during first drop_tanks()
		IPLAYER[i].y = 0; //recalculates during first drop_tanks()
		IPLAYER[i].dead = 0;
		IPLAYER[i].money = 0;
		IPLAYER[i].wins = 0;
		IPLAYER[i].turned = 0;
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
//	for (i=0; i<13; i++) IPLAYER[0].ammo[i] = 574;
//	IPLAYER[0].ammo[0] = -1;
}
// main
int main(int argc, char **argv)
{
	srand(time(NULL));
	printf("1\n");
	init_NPLAYERS();
	printf("2\n");
	init_players_info();
	printf("3\n");
	fill_arena();
	printf("4\n");
	load_options();
	printf("5\n");
	load_hiscores();
	printf("6\n");
	InkViewMain(main_handler);
	return 0;
}

#ifndef INKINTERNAL_H
#define INKINTERNAL_H

#include "inkview.h"

#ifndef EMULATOR
#include <dlfcn.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#else
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define iverror(x...) fprintf(stderr, x);
//#define IVDBG(x...) fprintf(stderr, x);
#define IVDBG(x...)

#define MAXPATHLENGTH 1024
#define MAXEVENTS 64
#define MAXPLAYLIST 1024
#define MAXPLSTRINGS 65536
#define MAXDICTS 64
#define SLEEPDELAY 1500LL
#define EINKDELAY 1500LL
#define MAXVOL 36

#define SHOWCLOCK_ONTURN 0
#define SHOWCLOCK_ALWAYS 1
#define SHOWCLOCK_OFF 2

#define C24TO8(c) (((((c) >> 16) & 255) * 77 + (((c) >> 8) & 255) * 151 + ((c) & 255) * 28) >> 8)

typedef struct iv_glyph_s {

	char left;
	char top;
	unsigned char width;
	unsigned char height;
	unsigned char data[0];

} iv_glyph;

typedef struct iv_state_s {

	int isopen;
	char *theme;
	char *lang;
	char *kbdlang;
	int powerofftm;
	unsigned char showclock;
	unsigned char reserved_01a;
	unsigned char reserved_01b;
	unsigned char reserved_01c;
	int antialiasing;

	pid_t pid;
	iv_handler hproc;
	iv_handler mainhproc;

	icanvas *framebuffer;
	icanvas *canvas;
	int orientation;

	iconfig *cfg;
	FT_Library ftlibrary;
	ihash *fonthash;
	ifont *current_font;
	FT_Face current_face;
	int current_color;
	int current_aa;
	unsigned char **cw_cache;
	unsigned short **kerning_cache;
	iv_glyph **glyph_cache;

	char *font;
	char *fontb;
	char *fonti;
	char *fontbi;

	long long reserved_02;
	int allowkeyup;
	int currentkey;
	int keyrepeats;
	int initialized;
	int exiting;
	int usbmode;
	char inplayer;
	char inlastopen;
	char in_dialog;
	char in_menu;
	int nohourglass;
	int needupdate;
	int poweroffenable;
	int sleepenable;
	long long waketime;
	long long sleeptime;
	long long measuretime;
	
	itimer *timers;
	int ntimers;

	ievent *events;
	int evtstart;
	int evtend;

} iv_state;

typedef struct iv_caches_s {

	int size;
	int refs;
	unsigned char **cw_cache;
	unsigned short **kerning_cache;
	iv_glyph **glyph_cache_aa;
	iv_glyph **glyph_cache_noaa;

} iv_caches;

typedef struct iv_fontdata_s {

	int refs;
	FT_Face face;
	int nglyphs;
	int nsizes;
	iv_caches *caches;

} iv_fontdata;

typedef struct iv_mpctl_s {

	unsigned char pclink;
	unsigned char poweroff;
	unsigned char lockdisplay;
	unsigned char remountfs;
	unsigned char restart;
	unsigned char wakeup;
	unsigned char safe_mode;
	unsigned char inkflags;
	unsigned char reserved_01;
	unsigned char reserved_02;
	unsigned char reserved_03;
	unsigned char reserved_04;

	int monpid;
	int bspid;
	int apppid;
	int mppid;
	int reserved_05;
	int reserved_06;
	int reserved_07;
	int reserved_08;
	int pkey;
	int vid;

	short state;
	short mode;
	short track;
	short reserved_0b;

	int tracksize;
	int position;
	int newposition;

	short volume;
	short equalizer[32];
	short filter_changed;
	short reserved_0c;

	int plcount;
	int playlist[MAXPLAYLIST];
	char plstrings[MAXPLSTRINGS];

} iv_mpctl;

typedef struct eink_cmd_s {

	int owner;
	int command;
	int nwrite;
	int nread;
	unsigned char data[0];

} eink_cmd;

extern iv_state ivstate;

extern ifont *title_font, *window_font, *header_font, *menu_s_font, *menu_n_font;
extern ifont *butt_n_font, *butt_s_font;
extern int window_color, header_color, menu_n_color, menu_s_color, menu_d_color;
extern int butt_n_color, butt_s_color;
extern ibitmap *button_normal, *button_selected;
extern int title_height;

extern ibitmap def_icon_question, def_icon_information, def_icon_warning, def_icon_error;
extern ibitmap def_fileicon;
extern ibitmap def_hourglass;
extern ibitmap def_button_normal, def_button_selected;
extern ibitmap def_pageentry;
extern ibitmap def_bmk_panel, def_bmk_active, def_bmk_inactive, def_bmk_actnew;
extern ibitmap def_tab_panel, def_tab_active, def_tab_inactive;
extern ibitmap def_leaf_open, def_leaf_closed;
extern ibitmap def_battery_0, def_battery_1, def_battery_2, def_battery_3, def_battery_4;
extern ibitmap def_battery_5, def_battery_c;
extern ibitmap keyboard_arrow;
extern ibitmap def_playing, def_usbex;
extern ibitmap def_dicmenu;

void hw_init();
void hw_close();
int hw_safemode();
void hw_rotate(int n);
icanvas *hw_getframebuffer();
int hw_depth();
void hw_update(int x, int y, int w, int h, int bw);
void hw_fullupdate();
void hw_refine16();
int hw_capable16();
void hw_suspend_display();
void hw_resume_display();
int hw_useraction();
int hw_keystate(int key);
int hw_nextevent(int *type, int *par1, int *par2);
int hw_isplaying();
int hw_ischarging();
int hw_isusbconnected();
int hw_issdinserted();
void hw_remountfs();
int hw_power();
int hw_temperature();
int hw_sleep(int ms);
time_t hw_gettime();
void hw_settime(time_t newt);
long long hw_timeinms();
void hw_setalarm(int ms);
void hw_say_poweroff();
void hw_restart();
int hw_isbookshelf();
void hw_registerapp(pid_t pid);
char *hw_serialnumber();
char *hw_password();
char *hw_waveform_filename();
char *hw_getmodel();
char *hw_getplatform();
int hw_writelogo(ibitmap *bm);
int hw_usbready();
void hw_usblink();
int hw_exiting();

void hw_mp_loadplaylist(char **pl);
char **hw_mp_getplaylist();
void hw_mp_playtrack(int n);
int hw_mp_currenttrack();
int hw_mp_tracksize();
void hw_mp_settrackposition(int pos);
int hw_mp_trackposition();
void hw_mp_setstate(int state);
int hw_mp_getstate();
void hw_mp_setmode(int mode);
int hw_mp_getmode();
void hw_mp_setvolume(int n);
int hw_mp_getvolume();
void hw_mp_setequalizer(int *eq);
void hw_mp_getequalizer(int *eq);

const char *iv_evttype(int type);
iv_handler iv_seteventhandler(iv_handler hproc);
void iv_enqueue(iv_handler hproc, int type, int par1, int par2);
char **enum_files(char ***list, char *path1, char *path2, char *extension);
void iv_drawsymbol(int x, int y, int size, int symbol, int color);
void iv_windowframe(int x, int y, int w, int h, int bordercolor, int windowcolor, char *title, int button);
void iv_scrollbar(int x, int y, int w, int h, int percent);
void iv_textblock(int x, int y, int w, unsigned short *p, int len, int color, int angle);
void def_openplayer();
bookinfo *def_getbookinfo(char *path);
ibitmap *def_getbookcover(char *path, int width, int height);
int iv_player_handler(int type, int par1, int par2);
void iv_actualize_panel(int update);
void iv_update_panel(int with_time);
void iv_rise_poweroff_timer();
void iv_key_timer();
void iv_poweroff_timer();
void iv_panelupdate_timer();
void iv_exit_handler();

#ifdef __cplusplus
}
#endif

#endif

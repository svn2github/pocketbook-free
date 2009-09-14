#ifndef INKVIEW_H
#define INKVIEW_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sysinfo.h>
#include <signal.h>
#include <errno.h>
#include <zlib.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#ifdef __cplusplus
extern "C"
{
#endif

#define APP_UID 101
#define APP_GID 101

#ifndef IVSAPP
#define FLASHDIR "/mnt/ext1"
#define SDCARDDIR "/mnt/ext2"
#define SYSTEMDATA "/ebrmain"
#define USERDATA FLASHDIR "/system"
#define USERDATA2 SDCARDDIR "/system"
#define TEMPDIR "/tmp"
#else
#define FLASHDIR "./system/mnt/ext1"
#define SDCARDDIR "./system/mnt/ext2"
#define SYSTEMDATA "./system"
#define USERDATA SYSTEMDATA
#define USERDATA2 SDCARDDIR "/system"
#define TEMPDIR "./system/tmp"
#endif


#define SYSTEMFONTDIR SYSTEMDATA "/fonts"
#define USERFONTDIR USERDATA "/fonts"
#define TEMPFONTPATH TEMPDIR "/fonts"
#define PHOTOTEMPDIR TEMPDIR "/photo"
#define CONFIGPATH USERDATA "/config"
#define STATEPATH USERDATA "/state"
#define SYSTEMTHEMESPATH SYSTEMDATA "/themes"
#define USERTHEMESPATH USERDATA "/themes"
#define GLOBALCONFIGFILE CONFIGPATH "/global.cfg"
#define SYSTEMLANGPATH SYSTEMDATA "/language"
#define USERLANGPATH USERDATA "/language"
#define SYSTEMKBDPATH SYSTEMDATA "/language/keyboard"
#define USERKBDPATH USERDATA "/language/keyboard"
#define SYSTEMDICTPATH SYSTEMDATA "/dictionaries"
#define USERDICTPATH1 USERDATA "/dictionaries"
#define USERDICTPATH2 USERDATA2 "/dictionaries"
#define SYSTEMLOGOPATH SYSTEMDATA "/logo"
#define USERLOGOPATH USERDATA "/logo"
#define NOTESPATH FLASHDIR "/notes"
#define GAMEPATH FLASHDIR "/games"
#define USERAPPDIR USERDATA "/bin"
#define CACHEPATH USERDATA "/cache"
#define USERBOOKSHELF USERDATA "/bin/bookshelf.app"
#define SYSTEMBOOKSHELF SYSTEMDATA "/bin/bookshelf.app"
#define USERMPD USERDATA "/bin/mpd.app"
#define SYSTEMMPD SYSTEMDATA "/bin/mpd.app"
#define USERMPLAYER USERDATA "/bin/mplayer.so"
#define USERBOOKINFO USERDATA "/bin/bookinfo.so"
#define POCKETBOOKSIG USERDATA "/.pocketbook"
#define LASTOPENBOOKS STATEPATH "/lastopen.txt"
#define FAVORITES USERDATA "/favorite"
#define CURRENTBOOK TEMPDIR "/.current"
#define BOOKSHELFSTATE TEMPDIR"/.bsstate"
#define BOOKSHELFSTATE_NV STATEPATH "/.bsstate"
#define HISTORYFILE TEMPDIR "/history.txt"
#define DICKEYBOARD TEMPDIR "/dictionary.kbd"

#define DEFAULTFONT "LiberationSans"
#define DEFAULTFONTB "LiberationSans-Bold"
#define DEFAULTFONTI "LiberationSans-Italic"
#define DEFAULTFONTBI "LiberationSans-BoldItalic"

#define SYSTEMDEPTH 8

#define EVT_STARTOFLIST 20 
#define EVT_INIT 21
#define EVT_EXIT 22
#define EVT_SHOW 23
#define EVT_REPAINT 23
#define EVT_HIDE 24
#define EVT_KEYDOWN 25
#define EVT_KEYPRESS 25
#define EVT_KEYUP 26
#define EVT_KEYRELEASE 26
#define EVT_KEYREPEAT 28
#define EVT_POINTERUP 29
#define EVT_POINTERDOWN 30
#define EVT_POINTERMOVE 31
#define EVT_ORIENTATION 32
#define EVT_ENDOFLIST 33

#define EVT_MP_STATECHANGED 81
#define EVT_MP_TRACKCHANGED 82

#undef KEY_UP
#undef KEY_DOWN
#undef KEY_LEFT
#undef KEY_RIGHT
#undef KEY_OK
#undef KEY_BACK
#undef KEY_MENU
#undef KEY_DELETE
#undef KEY_MUSIC
#undef KEY_POWER
#undef KEY_PREV
#undef KEY_NEXT
#undef KEY_MINUS
#undef KEY_PLUS
#undef KEY_0
#undef KEY_1
#undef KEY_2
#undef KEY_3
#undef KEY_4
#undef KEY_5
#undef KEY_6
#undef KEY_7
#undef KEY_8
#undef KEY_9

#define KEY_BACK 0x1b
#define KEY_DELETE 0x08
#define KEY_OK 0x0a
#define KEY_UP 0x11
#define KEY_DOWN 0x12
#define KEY_LEFT 0x13
#define KEY_RIGHT 0x14
#define KEY_MINUS 0x15
#define KEY_PLUS 0x16
#define KEY_MENU 0x17
#define KEY_MUSIC 0x1e
#define KEY_POWER 0x01
#define KEY_PREV 0x18
#define KEY_NEXT 0x19

#define KEY_0 0x30
#define KEY_1 0x31
#define KEY_2 0x32
#define KEY_3 0x33
#define KEY_4 0x34
#define KEY_5 0x35
#define KEY_6 0x36
#define KEY_7 0x37
#define KEY_8 0x38
#define KEY_9 0x39

#define BLACK 0x000000
#define DGRAY 0x555555
#define LGRAY 0xaaaaaa
#define WHITE 0xffffff

#define ITEM_HEADER 1
#define ITEM_ACTIVE 2
#define ITEM_INACTIVE 3
#define ITEM_SUBMENU 5
#define ITEM_SEPARATOR 6
#define ITEM_BULLET 7

#define KBD_UPPER 1
#define KBD_LOWER 2
#define KBD_FIRSTUPPER 3

#define ICON_INFORMATION 1
#define ICON_QUESTION 2
#define ICON_WARNING 3
#define ICON_ERROR 4

#define DEF_BUTTON1 0
#define DEF_BUTTON2 0x1000

#define LIST_BEGINPAINT 1
#define LIST_PAINT 2
#define LIST_ENDPAINT 3
#define LIST_OPEN 4
#define LIST_MENU 5
#define LIST_DELETE 6
#define LIST_EXIT 7
#define LIST_ORIENTATION 8

#define BMK_CLOSED -1
#define BMK_SELECTED 1
#define BMK_ADDED 2
#define BMK_REMOVED 3
#define BMK_PAINT 4

#define CFG_TEXT 1
#define CFG_CHOICE 2
#define CFG_INDEX 3
#define CFG_TIME 4
#define CFG_FONT 5
#define CFG_FONTFACE 6
#define CFG_INFO 7
#define CFG_SUBMENU 31

#define CFG_HIDDEN 128

#define ALIGN_LEFT 1
#define ALIGN_CENTER 2
#define ALIGN_RIGHT 4
#define ALIGN_FIT 8
#define VALIGN_TOP 16
#define VALIGN_MIDDLE 32
#define VALIGN_BOTTOM 64
#define ROTATE 128
#define HYPHENS 256
#define DOTS 512

#define ARROW_LEFT    1
#define ARROW_RIGHT   2
#define ARROW_UP      3
#define ARROW_DOWN    4
#define SYMBOL_OK     5
#define SYMBOL_PAUSE  6
#define SYMBOL_BULLET 7
#define ARROW_UPDOWN  8

#define IMAGE_BW    1
#define IMAGE_GRAY2 2
#define IMAGE_GRAY4 4
#define IMAGE_GRAY8 8
#define IMAGE_RGB   24

#define ROTATE0    0
#define ROTATE90   1
#define ROTATE270  2
#define ROTATE180  3

#define XMIRROR    4
#define YMIRROR    8

#define DITHER_THRESHOLD 0
#define DITHER_PATTERN 1
#define DITHER_DIFFUSION 2

#define MP_STOPPED 0
#define MP_REQUEST_FOR_PLAY 1
#define MP_PLAYING 2
#define MP_PAUSED 3
#define MP_PREVTRACK 4
#define MP_NEXTTRACK 5

#define MP_ONCE 0
#define MP_CONTINUOUS 1
#define MP_RANDOM 2

#define FTYPE_UNKNOWN 0
#define FTYPE_BOOK 1
#define FTYPE_PICTURE 2
#define FTYPE_MUSIC 3
#define FTYPE_APPLICATION 4
#define FTYPE_FOLDER 255

#define GSENSOR_OFF 0
#define GSENSOR_ON 1
#define GSENSOR_INTR 2

typedef struct irect_s {

	short x;
	short y;
	short w;
	short h;
	int flags;

} irect;

typedef struct ibitmap_s {

	unsigned short width;
	unsigned short height;
	unsigned short depth;
	unsigned short scanline;
	unsigned char* data;

} ibitmap;

typedef int (*iv_handler)(int type, int par1, int par2);
typedef void (*iv_timerproc)();

typedef void (*iv_menuhandler)(int index);
typedef void (*iv_keyboardhandler)(char *text);
typedef void (*iv_dialoghandler)(int button);
typedef void (*iv_timeedithandler)(long newtime);
typedef void (*iv_fontselecthandler)(char *fontr, char *fontb, char *fonti, char *fontbi);
typedef void (*iv_dirselecthandler)(char *path);
typedef void (*iv_confighandler)();
typedef void (*iv_itemchangehandler)(char *name);
typedef void (*iv_pageselecthandler)(int page);
typedef void (*iv_bmkhandler)(int action, int page, long long position);
typedef void (*iv_tochandler)(long long position);
//!: Not used
//typedef void (*iv_itempaint)(int x, int y, int index, int selected);
typedef void (*iv_listhandler)(int action, int x, int y, int idx, int state);
typedef void (*iv_rotatehandler)(int direction);
//!: Not used
//typedef int (*iv_turnproc)(int direction);
//!: Not used?
//typedef int (*iv_recurser)(char *path, int type, void *data);

typedef int (*iv_hashenumproc)(char *name, void *value, void *userdata);
typedef int (*iv_hashcmpproc)(char *name1, void *value1, char *name2, void *value2);
typedef void * (*iv_hashaddproc)(void *data);
typedef void (*iv_hashdelproc)(void *data);

typedef struct ihash_item_s {

	char *name;
	void *value;
	struct ihash_item_s *next;

} ihash_item;

typedef struct ihash_s {

	int prime;
	int count;
	iv_hashaddproc addproc;
	iv_hashdelproc delproc;
	struct ihash_item_s **items;

} ihash;

typedef struct imenu_s {
	short type;
	short index;
	char *text;
	struct imenu_s *submenu;
} imenu;

typedef struct icanvas_s {

	int width;
	int height;
	int scanline;
	int depth;
	int clipx1, clipx2;
	int clipy1, clipy2;
	unsigned char *addr;

} icanvas;

typedef struct ifont_s {

	char *name;
	char *family;
	int size;
	unsigned char aa;
	unsigned char isbold;
	unsigned char isitalic;
	unsigned char _r1;
	unsigned short charset;
	unsigned short _r2;
	int color;
	int height;
	int linespacing;
	int baseline;
	void *fdata;

} ifont;

//!: Not used
//~ typedef struct ievent_s {

	//~ iv_handler hproc;
	//~ unsigned short type;
	//~ unsigned short _reserved;
	//~ unsigned short par1;
	//~ unsigned short par2;

//~ } ievent;

typedef struct iconfig_s {

	char *filename;
	ihash *hash;
	int changed;

} iconfig;

typedef struct iconfigedit_s {

	int type;
	const ibitmap *icon;
	char *text;
	char *hint;
	char *name;
	char *deflt;
	char **variants;
	struct iconfigedit_s *submenu;

} iconfigedit;

//!: Not used
//~ typedef struct oldconfigedit_s {

	//~ char *text;
	//~ char *name;
	//~ int type;
	//~ char *deflt;
	//~ char **variants;

//~ } oldconfigedit;

typedef struct tocentry_s {

	int level;
	int page;
	long long position;
	char *text;

} tocentry;

//!: Not used
//~ typedef struct itimer_s {

	//~ iv_timerproc tp;
	//~ int weak;
	//~ long long extime;
	//~ char name[16];

//~ } itimer;

typedef struct bookinfo_s {

	int type;
	char *typedesc;
	char *path;
	char *filename;
	char *title;
	char *author;
	char *series;
	char *genre[10];
	ibitmap *icon;
	int year;
	long size;
	time_t ctime;

} bookinfo;

typedef struct iv_filetype_s {

	char *extension;
	char *description;
	int type;
	char *program;
	ibitmap *icon;

} iv_filetype;

//!: Not used
//~ typedef struct iv_template_s {

	//~ int width;
	//~ int height;
	//~ ibitmap *background;
	//~ ibitmap *bg_folder;
	//~ ibitmap *bg_folder_a;
	//~ ibitmap *bg_file;
	//~ ibitmap *bg_file_a;
	//~ irect iconpos;
	//~ irect mediaiconpos;
	//~ irect line1pos;
	//~ irect line2pos;
	//~ irect line3pos;
	//~ ifont *line1font;
	//~ ifont *line2font;
	//~ ifont *line3font;

//~ } iv_template;

typedef struct iv_wlist_s {

	char *word;
	short x1;
	short y1;
	short x2;
	short y2;

} iv_wlist;

void OpenScreen();
void InkViewMain(iv_handler h);
void CloseApp();

// Screen information

int ScreenWidth();
int ScreenHeight();

// Orientation and g-sensor
// Set screen orientation: 0=portrait, 1=landscape 90, 2=landscape 270, 3=portrait 180
// For global settings: -1=auto (g-sensor)

void SetOrientation(int n);
int GetOrientation();
void SetGlobalOrientation(int n);
int GetGlobalOrientation(int n);
int QueryGSensor();
void SetGSensor(int mode);
int ReadGSensor(int *x, int *y, int *z);

// Graphic functions. Color=0x00RRGGBB

void ClearScreen();
void SetClip(int x, int y, int w, int h);
void DrawPixel(int x, int y, int color);
void DrawLine(int x1, int y1, int x2, int y2,int color);
void DrawRect(int x, int y, int w, int h, int color);
void FillArea(int x, int y, int w, int h, int color);
void InvertArea(int x, int y, int w, int h);
void InvertAreaBW(int x, int y, int w, int h);
void DimArea(int x, int y, int w, int h, int color);
void DrawSelection(int x, int y, int w, int h, int color);
void DitherArea(int x, int y, int w, int h, int levels, int method);
void Stretch(const unsigned char *src, int format, int sw, int sh, int scanline, int dx, int dy, int dw, int dh, int rotate);
//TODO: !!!
void SetCanvas(icanvas *c);
icanvas *GetCanvas();
void Repaint();

// Bitmap functions

ibitmap *LoadBitmap(char *filename);
ibitmap *BitmapFromScreen(int x, int y, int w, int h);
ibitmap *NewBitmap(int w, int h);
ibitmap *LoadJPEG(char *path, int w, int h, int br, int co, int proportional);
void DrawBitmap(int x, int y, const ibitmap *b);
void DrawBitmapArea(int x, int y, const ibitmap *b, int bx, int by, int bw, int bh);
void DrawBitmapRect(int x, int y, int w, int h, ibitmap *b, int flags);
void DrawBitmapRect2(irect *rect, ibitmap *b);
void StretchBitmap(int x, int y, int w, int h, ibitmap *src, int flags);

// Text functions

char **EnumFonts();
ifont *OpenFont(char *name, int size, int aa);
void CloseFont(ifont *f);
void SetFont(ifont *font, int color);
void DrawString(int x, int y, const char *s);
void DrawStringR(int x, int y, const char *s);
int TextRectHeight(int width, char *s, int flags);
char *DrawTextRect(int x, int y, int w, int h, char *s, int flags);
char *DrawTextRect2(irect *rect, char *s);
int CharWidth(unsigned  short c);
int StringWidth(char *s);
int DrawSymbol(int x, int y, int symbol);
void RegisterFontList(ifont **fontlist, int count);

// Screen update functions

void FullUpdate();
void SoftUpdate();
void PartialUpdate(int x, int y, int w, int h);
void PartialUpdateBW(int x, int y, int w, int h);
void FineUpdate();
int FineUpdateSupported();

// Event handling functions

iv_handler SetEventHandler(iv_handler hproc);
iv_handler GetEventHandler();
void SendEvent(iv_handler hproc, int type, int par1, int par2);

// Timer functions

void SetHardTimer(char *name, iv_timerproc tproc, int ms);
void SetWeakTimer(char *name, iv_timerproc tproc, int ms);
void ClearTimer(iv_timerproc tproc);

// UI functions

void OpenMenu(imenu *menu, int pos, int x, int y, iv_menuhandler hproc);
void OpenMenu3x3(const ibitmap *mbitmap, const char *strings[9], iv_menuhandler hproc);
char **EnumKeyboards();
void OpenList(char *title, ibitmap *background, int itemw, int itemh, int itemcount, int cpos, iv_listhandler hproc);
void OpenDummyList(char *title, ibitmap *background, char *text, iv_listhandler hproc);
void LoadKeyboard(char *kbdlang);
void OpenKeyboard(char *title, char *buffer, int maxlen, int flags, iv_keyboardhandler hproc);
void OpenCustomKeyboard(char *filename, char *title, char *buffer, int maxlen, int flags, iv_keyboardhandler hproc);
void CloseKeyboard();
void OpenPageSelector(iv_pageselecthandler hproc);
void OpenTimeEdit(char *title, int x, int y, long intime, iv_timeedithandler hproc);
void OpenDirectorySelector(char *title, char *buf, int len, iv_dirselecthandler hproc);
void OpenFontSelector(char *title, char *font, int with_size, iv_fontselecthandler hproc);
void OpenBookmarks(int page, long long position, int *bmklist, long long *poslist,
		int *bmkcount, int maxbmks, iv_bmkhandler hproc);
void SwitchBookmark(int page, long long position, int *bmklist, long long *poslist,
		int *bmkcount, int maxbmks, iv_bmkhandler hproc);
void OpenContents(tocentry *toc, int count, long long position, iv_tochandler hproc);
void OpenRotateBox(iv_rotatehandler hproc);
void Message(int icon, char *title, char *text, int timeout);
void Dialog(int icon, char *title, char *text, char *button1, char *button2, iv_dialoghandler hproc);
void CloseDialog();
void ShowHourglass();
void ShowHourglassAt(int x, int y);
void HideHourglass();
void DisableExitHourglass();
int DrawPanel(ibitmap *icon, char *text, char *title, int percent);
void DrawTabs(ibitmap *icon, int current, int total);
int PanelHeight();
void SetKeyboardRate(int t1, int t2);

// Configuration functions

iconfig * GetGlobalConfig();
iconfig * OpenConfig(char *path, iconfigedit *ce);
int SaveConfig(iconfig *cfg);
void CloseConfig(iconfig *cfg);
int ReadInt(iconfig *cfg, char *name, int deflt);
char *ReadString(iconfig *cfg, char *name, char *deflt);
void WriteInt(iconfig *cfg, char *name, int value);
void WriteString(iconfig *cfg, char *name, char *value);
void OpenConfigEditor(char *header, iconfig *cfg, iconfigedit *ce, iv_confighandler hproc, iv_itemchangehandler cproc);
void GetKeyMapping(char *act0[], char *act1[]);
unsigned long QueryDeviceButtons();

// String hash functions

ihash * hash_new(int prime);
void hash_add(ihash *h, char *name, char *value);
void hash_delete(ihash *h, char *name);
char *hash_find(ihash *h, char *name);

// Object hash functions

ihash * vhash_new(int prime, iv_hashaddproc addproc, iv_hashdelproc delproc);
void vhash_add(ihash *h, char *name, void *value);
void vhash_delete(ihash *h, char *name);
void *vhash_find(ihash *h, char *name);

// Common hash functions
void hash_clear(ihash *h);
void hash_destroy(ihash *h);
int  hash_count(ihash *h);
void hash_enumerate(ihash *h, iv_hashcmpproc cmpproc, iv_hashenumproc enumproc, void *userdata);

// filesystem functions

//!: Not used
//~ int iv_stat(char *name, struct stat *st);
//~ FILE *iv_fopen(char *name, char *mode);
//~ int iv_fread(void *buffer, int size, int count, FILE *f);
//~ int iv_fwrite(const void *buffer, int size, int count, FILE *f);
//~ int iv_fseek(FILE *f, long offset, int whence);
//~ long iv_ftell(FILE *f); 
//~ int iv_fclose(FILE *f);
//~ int iv_fgetc(FILE *f);
//~ char *iv_fgets(char *string, int n, FILE *f);
//~ int iv_mkdir(char *pathname, mode_t mode);
//~ void iv_buildpath(char *filename);
//~ DIR *iv_opendir(const char *dirname);
//~ struct dirent *iv_readdir(DIR *dir);
//~ int iv_closedir(DIR *dir);
//~ int iv_unlink(char *name);
//~ int iv_rmdir(char *name);
//~ int iv_truncate(char *name, int length);
//~ int iv_rename(char *oldname, char *newname);
//~ void iv_preload(char *name, int count);
//~ void iv_sync();

// Language functions

char ** EnumLanguages();
void LoadLanguage(char *lang);
void AddTranslation(char *label, char *trans);
char *GetLangText(char *s);
char *GetLangTextF(char *s, ...);

#define T(x) GetLangText(#x)
#define TF(x...) GetLangTextF(x)

// Theme functions

char ** EnumThemes();
void OpenTheme(char *path);
ibitmap *GetResource(char *name, ibitmap *deflt);
int GetThemeInt(char *name, int deflt);
char *GetThemeString(char *name, char *deflt);
ifont *GetThemeFont(char *name, char *deflt);
void GetThemeRect(char *name, irect *rect, int x, int y, int w, int h, int flags);

// Book functions

iv_filetype *GetSupportedFileTypes();
bookinfo *GetBookInfo(char *name);
ibitmap *GetBookCover(char *name, int width, int height);
char *GetAssociatedFile(char *name, int index);
char *CheckAssociatedFile(char *name, int index);
void SetReadMarker(char *name, int isread);
iv_filetype *FileType(char *path);
void OpenBook(char *path, char *position, int addtolast);
void BookReady(char *path);
char **GetLastOpen();
void AddLastOpen(char *path);
void OpenLastBooks();


// Media functions 

//!: not needed :)
//void OpenPlayer();
//void PlayFile(char *filename);
//void LoadPlaylist(char **pl);
//char **GetPlaylist();
//void PlayTrack(int n);
//void PreviousTrack();
//void NextTrack();
//int GetCurrentTrack();
//int GetTrackSize();
//void SetTrackPosition(int pos);
//int GetTrackPosition();
//void SetPlayerState(int state);
//int GetPlayerState();
//void SetPlayerMode(int mode);
//int GetPlayerMode();
//void TogglePlaying();
//void SetVolume(int n);
//int GetVolume();
//void SetEqualizer(int *eq);
//void GetEqualizer(int *eq);

// Notepad functions

char **EnumNotepads();
void OpenNotepad(char *name);
void CreateNote(char *filename, char *title, long long position);
void CreateNoteFromImages(char *filename, char *title, long long position, ibitmap *img1, ibitmap *img2);
void CreateNoteFromPage(char *filename, char *title, long long position);
void OpenNotesMenu(char *filename, char *title, long long position);

// Dictionary functions

char **EnumDictionaries();
int OpenDictionary(char *name);
void CloseDictionary();
int LookupWord(char *what, char **word, char **trans);
int LookupWordExact(char *what, char **word, char **trans);
int LookupPrevious(char **word, char **trans);
int LookupNext(char **word, char **trans);
void OpenDictionaryView(iv_wlist *wordlist, char *dicname);

// Text reflow API

void iv_reflow_start(int x, int y, int w, int h, int scale);
void iv_reflow_bt();
void iv_reflow_et();
void iv_reflow_div();
void iv_reflow_addchar(int code, int x, int y, int w, int h);
void iv_reflow_addimage(int x, int y, int w, int h, int flags);
int iv_reflow_subpages();
void iv_reflow_render(int spnum);
int iv_reflow_getchar(int *x, int *y);
int iv_reflow_getimage(int *x, int *y, int *scale);
int iv_reflow_words();
char *iv_reflow_getword(int n, int *spnum, int *x, int *y, int *w, int *h);
void iv_reflow_clear();

// Additional functions

int GetBatteryPower();
int GetTemperature();
int IsCharging();
int IsUSBconnected();
int IsSDinserted();
int IsPlayingMP3();
int IsKeyPressed(int key);
void SetRTCtime(time_t newt);
char *GetDeviceModel();
char *GetHardwareType();
char *GetSoftwareVersion();
int GetHardwareDepth();
char *GetSerialNumber();
char *GetWaveformFilename();
char *GetDeviceKey();
char *CurrentDateStr();
char *DateStr(time_t t);
int GoSleep(int ms, int deep);
void SetAutoPowerOff(int en);
void PowerOff();
int SafeMode();
void OpenMainMenu();
int WriteStartupLogo(ibitmap *bm);

//!: Not used ?
//~ int escape(char *val, char *buf, int size);
//~ int unescape(char *val, char *buf, int size);
//~ int utf2ucs(const char *s, unsigned short *us, int maxlen);
//~ int utf2ucs4(const char *s, unsigned int *us, int maxlen);
//~ int ucs2utf(const unsigned short *us, char *s, int maxlen);
//~ int utfcasecmp(char *sa, char *sb);
//~ int utfncasecmp(char *sa, char *sb, int n);
//~ char *utfcasestr(char *sa, char *sb);
//~ void utf_toupper(char *s);
//~ void utf_tolower(char *s);
//~ int base64_decode(char *in, unsigned char *out, int len);
//~ int recurse_action(char *path, iv_recurser proc, void *data, int creative, int this_too);

#ifdef __cplusplus
}
#endif

#endif


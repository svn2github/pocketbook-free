/*
 *  inkview front end for Simon Tatham's puzzle collection.
 */

#include <stdarg.h>
#include <math.h>
#include "inkview.h"

#include "../puzzles.h"
#include "inkcol.h"

int pbSW, pbSH, pbOrient;

typedef struct {
  int x;
  int y;
} MWPOINT;

struct frontend {
  int inkSW;
  int inkSH;
  int FH;
  int FW;
  int statusbar;
  ifont *statusfont;
  char *statustext;
#ifdef GUESS_GAME
  ifont *guessfont;
#endif
  struct timeval last_time;
  int MenuP;
  int time_int;
  int isTimer;
#ifdef PBKEYB
  int is_pbkeyb;
  int pbkeybx, pbkeyby;
  int ndig;
#endif
#ifdef INERTIA_GAME
  int inerdir;
#endif
  game_params *pparams;
};

struct blitter {
  int width;
  int height;
  ibitmap *ibit;
};

struct savefile_write_ctx {
    FILE *fp;
    int error;
};

imenu *TypeMenu;

imenu StateMenu[] = {
    {ITEM_ACTIVE, 103, "Load", NULL},
    {ITEM_ACTIVE, 104, "Save", NULL},
    {0, 0, NULL, NULL }
};

imenu RotateMenu[] = {
    {ITEM_ACTIVE, 111, "clockwise", NULL},
    {ITEM_ACTIVE, 112, "anticlockwise", NULL},
    {0, 0, NULL, NULL }
};

imenu GameParMenu[] = {
    {ITEM_SUBMENU, 0, "Rotate", RotateMenu},
//    {ITEM_INACTIVE, 0, "Language", NULL},
    {0, 0, NULL, NULL }
};

imenu MainMenu[] = {
    {ITEM_SUBMENU, 0, "Game", GameParMenu},
    {ITEM_ACTIVE, 101, "New", NULL},
    {ITEM_ACTIVE, 102, "Restart", NULL},
    {ITEM_SEPARATOR, 0, NULL, NULL },
    {ITEM_SUBMENU, 0, "Type", NULL},
    {ITEM_SUBMENU, 0, "State", StateMenu},
    {ITEM_SEPARATOR, 0, NULL, NULL },
    {ITEM_ACTIVE, 105, "Undo", NULL},
    {ITEM_ACTIVE, 106, "Redo", NULL},
    {ITEM_ACTIVE, 107, "Solve", NULL},
    {ITEM_SEPARATOR, 0, NULL, NULL },
    {ITEM_ACTIVE, 108, "About", NULL},
    {ITEM_ACTIVE, 109, "Help", NULL},
    {ITEM_SEPARATOR, 0, NULL, NULL },
    {ITEM_ACTIVE, 110, "Exit", NULL },
    {0, 0, NULL, NULL }
};

frontend *fe;
midend *me;
drawing *dr;

void ink_draw_text(void *handle, int x, int y, int fonttype, int fontsize,
               int align, int colour, char *text) {

  ifont *tempfont;
  int sw, sh;

  switch (fonttype) {
    case FONT_FIXED:
            tempfont = OpenFont("LiberationMono", fontsize, 0);
            break;
    case FONT_VARIABLE:
    default:
            tempfont = OpenFont("LiberationSans", fontsize, 0);
            break;
  }

  SetFont(tempfont, inkcolors[colour]);
  sw=StringWidth(text);
  sh=TextRectHeight(sw, text, 0);
  if (align & ALIGN_VNORMAL) y -= sh;
  else if (align & ALIGN_VCENTRE) y -= sh/2;
  if (align & ALIGN_HCENTRE) x -= sw/2;
  else if (align & ALIGN_HRIGHT) x -= sw;

  DrawString(x, y, text);
  CloseFont(tempfont);
}

void ink_draw_rect(void *handle, int x, int y, int w, int h, int colour) {

#ifdef PEGS_GAME
  if (colour==2) colour=4;
#endif

  int i;

  if (inkcolors[colour] & DOTTED) {
    for (i=0;i<h;i++) {
      if ((y+i) & 1) DrawLine(x,y+i,x+w-1,y+i,inkcolors[colour]&WHITE);
      else DrawLine(x,y+i,x+w-1,y+i,inkcolors[0]);
    }
  }
  else for (i=0;i<h;i++) DrawLine(x,y+i,x+w-1,y+i,inkcolors[colour]);
//???????? FillArea(x, y, w, h, inkcolors[colour]);

}

void ink_draw_rect_outline(void *handle, int x, int y, int w, int h, int colour) {

    DrawRect(x, y, w, h, inkcolors[colour]);
}

void ink_draw_line(void *handle, int x1, int y1, int x2, int y2, int colour) {

  DrawLine(x1, y1, x2, y2, inkcolors[colour]);
}


/* polygon fill routine was borrowed from the Nano-X Window System. */

static void extendrow(int y, int x1, int y1, int x2, int y2, int *minxptr, int *maxxptr) {

  int x;	
  typedef long NUM;
  NUM num;	

  if (((y < y1) || (y > y2)) && ((y < y2) || (y > y1)))
	return;

  if (y1 == y2) {
	if (*minxptr > x1) *minxptr = x1;
	if (*minxptr > x2) *minxptr = x2;
	if (*maxxptr < x1) *maxxptr = x1;
	if (*maxxptr < x2) *maxxptr = x2;
	return;
  }

  if (x1 == x2) {
	if (*minxptr > x1) *minxptr = x1;
	if (*maxxptr < x1) *maxxptr = x1;
	return;
  }

  num = ((NUM) (y - y1)) * (x2 - x1);
  x = x1 + num / (y2 - y1);
  if (*minxptr > x) *minxptr = x;
  if (*maxxptr < x) *maxxptr = x;
}

void ink_draw_polygon(void *handle, int *icoords, int npoints,
                  int fillcolour, int outlinecolour) {

MWPOINT *coords = (MWPOINT *)icoords;

#ifdef PEGS_GAME
  if (fillcolour==2) fillcolour=4;
#endif

  MWPOINT *pp;
  int miny;
  int maxy;
  int minx;
  int maxx;
  int i;

  if (fillcolour!=-1) {
    if (npoints <= 0) return;

    pp = coords;
    miny = pp->y;
    maxy = pp->y;
    for (i = npoints; i-- > 0; pp++) {
  	if (miny > pp->y) miny = pp->y;
  	if (maxy < pp->y) maxy = pp->y;
    }

    for (; miny <= maxy; miny++) {
  	minx = 32767;
	maxx = -32768;
	pp = coords;
	for (i = npoints; --i > 0; pp++)
		extendrow(miny, pp[0].x, pp[0].y, pp[1].x, pp[1].y,
			&minx, &maxx);
	extendrow(miny, pp[0].x, pp[0].y, coords[0].x, coords[0].y,
		&minx, &maxx);

	if (minx <= maxx) {
          if (inkcolors[fillcolour] & DOTTED) {
            if (miny & 1) DrawLine(minx, miny, maxx, miny, inkcolors[fillcolour]&WHITE);
            else DrawLine(minx, miny, maxx, miny, inkcolors[0]);
          }
          else DrawLine(minx, miny, maxx, miny, inkcolors[fillcolour]);
        }
    }
  }

  for (i = 0; i < npoints-1; i++) {
    DrawLine(coords[i].x, coords[i].y, coords[i+1].x, coords[i+1].y, inkcolors[outlinecolour]);
  }
  DrawLine(coords[i].x, coords[i].y, coords[0].x, coords[0].y, inkcolors[outlinecolour]);
}

void ink_draw_circle(void *handle, int cx, int cy, int radius, int fillcolour, int outlinecolour) {

  int i,x,y,yy=0-radius,xx=0;

  for (i=0; i<=2*radius; i++) {
    y=i-radius;
    x=lround(sqrt(radius*radius-y*y));

    DrawLine(cx+xx, cy+yy, cx+x, cy+y, inkcolors[outlinecolour]&WHITE);
    DrawLine(cx-xx, cy+yy, cx-x, cy+y, inkcolors[outlinecolour]&WHITE);

#ifdef GUESS_GAME
  if (inkcolors[fillcolour]&DOTTED) {
char nnn[2];
    int sw, sh;

    sprintf(nnn, "%i", fillcolour-6);
    SetFont(fe->guessfont, BLACK);
    sw=StringWidth(nnn);
    sh=TextRectHeight(sw, nnn, 0);
    DrawString(cx-sw/2, cy-sh/2, nnn);
  }
  else
#endif

    if (fillcolour!=-1) {
      if (!(inkcolors[fillcolour]&DOTTED)) {
        DrawLine(cx-x, cy+y, cx+x, cy+y, inkcolors[fillcolour]&WHITE);
      }
      else if ((cy+y)%2) DrawLine(cx-x, cy+y, cx+x, cy+y, inkcolors[fillcolour]&WHITE);
    }

    xx=x; yy=y;
  }
}

void ink_clip(void *handle, int x, int y, int w, int h) {

    SetClip(x, y, w, h);
}

void ink_unclip(void *handle) {

    SetClip(0, 0, pbSW, pbSH);
}

void ink_start_draw(void *handle) {
//??
}

void ink_draw_update(void *handle, int x, int y, int w, int h) {
//??
}

void ink_end_draw(void *handle) {

  PartialUpdate(0, 0, fe->inkSW, fe->inkSH);
}

blitter *ink_blitter_new(void *handle, int w, int h) {

  blitter *bl = snew(blitter);
  bl->width = w;
  bl->height = h;
  bl->ibit = NULL;
  return bl;
}

void ink_blitter_free(void *handle, blitter *bl) {

  sfree(bl->ibit);
  sfree(bl);
}

void ink_blitter_save(void *handle, blitter *bl, int x, int y) {

  bl->ibit = BitmapFromScreen(x, y, bl->width, bl->height);
}

void ink_blitter_load(void *handle, blitter *bl, int x, int y) {

  DrawBitmap(x, y, bl->ibit);
}

void ink_status_bar(void *handle, char *text) {

  size_t tlen;

  if (fe->statusbar) {
    sfree(fe->statustext);
    tlen=strlen(text)+1;
    fe->statustext = snewn(tlen, char);
    strcpy(fe->statustext, text);
    ink_draw_rect(0, 10, pbSH-30, pbSW-20, 30, 0);
    SetFont(fe->statusfont, BLACK);
    DrawString(10, pbSH-30, fe->statustext);
    PartialUpdateBW(0, pbSH-50, pbSW, 50);
  }
}

void get_random_seed(void **randseed, int *randseedsize) {

    struct timeval *tvp = snew(struct timeval);
    gettimeofday(tvp, NULL);
    *randseed = (void *)tvp;
    *randseedsize = sizeof(struct timeval);
}

void tproc() {

  if (fe->isTimer) {
    struct timeval now;
    float elapsed;

    gettimeofday(&now, NULL);
    elapsed = ((now.tv_usec - fe->last_time.tv_usec) * 0.000001F + (now.tv_sec - fe->last_time.tv_sec));
    midend_timer(me, elapsed);
    fe->last_time = now;
    SetWeakTimer("timername", tproc, fe->time_int);
  }
}

void activate_timer(frontend *fe) {

  fe->isTimer=1;
  gettimeofday(&fe->last_time, NULL);
  SetWeakTimer("timername", tproc, fe->time_int);

};

void deactivate_timer(frontend *fe) {

  fe->isTimer=0;
  ClearTimer(tproc);
};

void fatal(char *fmt, ...) {
    va_list ap;
    fprintf(stderr, "fatal error: ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    exit(1);
}

void frontend_default_colour(frontend *fe, float *output) {

    output[0] = 1.0;
    output[1] = 1.0;
    output[2] = 1.0;
}

static void savefile_write(void *wctx, void *buf, int len) {

  struct savefile_write_ctx *ctx = (struct savefile_write_ctx *)wctx;
  if (fwrite(buf, 1, len, ctx->fp) < len) ctx->error = errno;
}

static int savefile_read(void *wctx, void *buf, int len) {

  FILE *fp = (FILE *)wctx;
  int ret;

  ret = fread(buf, 1, len, fp);
  return (ret == len);
}

static void save_state() {

  char name[128];
  FILE *fp;
  struct savefile_write_ctx ctx;

  sprintf(name, STATEPATH"/%s.sav", thegame.name);
  fp = fopen(name, "w");
  if (!fp) {
    Message(ICON_ERROR, "ERROR", "Unable to open state file",10000);
    return;
  }

  ctx.fp = fp;
  ctx.error = 0;
  midend_serialise(me, savefile_write, &ctx);
  fclose(fp);
  if (ctx.error) {
    char boxmsg[512];
    sprintf(boxmsg, "Error writing save file: %.400s",
    strerror(errno));
    Message(ICON_ERROR, "ERROR", boxmsg,10000);
    return;
  }
}

void load_state() {

  char name[128];
  char *err;
  FILE *fp;
    
  sprintf(name, STATEPATH"/%s.sav", thegame.name);
  fp = fopen(name, "r");
  if (!fp) {
    Message(ICON_ERROR, "ERROR", "Unable to open state file",10000);
    return;
  }

  err = midend_deserialise(me, savefile_read, fp);
  fclose(fp);
  if (err) {
    Message(ICON_ERROR, "ERROR", err,10000);
    return;
  }
}

const struct drawing_api ink_drawing = {
    ink_draw_text,
    ink_draw_rect,
    ink_draw_line,
    ink_draw_polygon,
    ink_draw_circle,
    ink_draw_update,
    ink_clip,
    ink_unclip,
    ink_start_draw,
    ink_end_draw,
    ink_status_bar,
    ink_blitter_new,
    ink_blitter_free,
    ink_blitter_save,
    ink_blitter_load,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

#ifdef INERTIA_GAME
static void ink_drawarr(int dir) {

  int i = pbSW/2;
  ink_draw_rect(0,i-25, fe->FH, 51, 51, 0);
  if (dir) {
    DrawLine(i-18, fe->FH+7, i-3, fe->FH+7,BLACK);
    DrawLine(i-18, fe->FH+7, i-18, fe->FH+22,BLACK);
    DrawLine(i-18, fe->FH+43, i-3, fe->FH+43,BLACK);
    DrawLine(i-18, fe->FH+43, i-18, fe->FH+28,BLACK);
    DrawLine(i+18, fe->FH+7, i+3, fe->FH+7,BLACK);
    DrawLine(i+18, fe->FH+7, i+18, fe->FH+22,BLACK);
    DrawLine(i+18, fe->FH+43, i+3, fe->FH+43,BLACK);
    DrawLine(i+18, fe->FH+43, i+18, fe->FH+28,BLACK);
  }
  else {
    DrawLine(i, fe->FH, i-10, fe->FH+10,BLACK);
    DrawLine(i, fe->FH, i+10, fe->FH+10,BLACK);
    DrawLine(i, fe->FH+50, i-10, fe->FH+40,BLACK);
    DrawLine(i, fe->FH+50, i+10, fe->FH+40,BLACK);
    DrawLine(i-25, fe->FH+25, i-15, fe->FH+15,BLACK);
    DrawLine(i-25, fe->FH+25, i-15, fe->FH+35,BLACK);
    DrawLine(i+25, fe->FH+25, i+15, fe->FH+15,BLACK);
    DrawLine(i+25, fe->FH+25, i+15, fe->FH+35,BLACK);
  }
  ink_draw_circle(0, i, fe->FH+25, 10, 6, 6);
  PartialUpdateBW(i-25, fe->FH, 51, 51);
}
#endif

static void PrepareGame(void) {

  ClearScreen();
  fe->inkSW = pbSW;
  fe->inkSH = pbSH;

  if ((fe->statusbar = midend_wants_statusbar(me))) {
    DrawLine(0,pbSH-40,pbSW,pbSH-40,BLACK);
    fe->inkSH -= 50;
    if (fe->statustext) {
      SetFont(fe->statusfont, BLACK);
      DrawString(10, pbSH-30, fe->statustext);
    }
  }
#ifdef INERTIA_GAME
  fe->inkSH -= 60;
  fe->FH = fe->inkSH;
  ink_drawarr(fe->inerdir);

#elif defined PBKEYB
  if (pbOrient%3) {
    fe->inkSW -= 200; //?
    fe->FW = fe->inkSW;
    fe->pbkeybx = pbSW-100; fe->pbkeyby = pbSH/2;
  }
  else {
    fe->inkSH -= 200;
    fe->FH = fe->inkSH;
    fe->pbkeybx = pbSW/2; fe->pbkeyby = fe->FH+50;
  }
  fe->is_pbkeyb=0;
//  pbkeyb(pbSW/2, fe->FH, '0', fe->ndig);

#endif

  FullUpdate();
}

static void StartNewGame(void) {
  midend_new_game(me);
  midend_size(me, &fe->inkSW, &fe->inkSH, TRUE);
  fe->time_int=20;

#ifdef INERTIA_GAME
  fe->inerdir=0;
  ink_drawarr(fe->inerdir);

#elif defined PBKEYB
  char *wintitle;
  config_item *cfgit = midend_get_config(me, CFG_SETTINGS, &wintitle);

#ifdef FILLING_GAME
  fe->ndig=9;
#endif

#ifdef KEEN_GAME
  fe->ndig=atoi(cfgit[0].sval);
#endif

#ifdef SOLO_GAME
  fe->ndig=atoi(cfgit[0].sval)*atoi(cfgit[1].sval);
#endif

#ifdef TOWERS_GAME
  fe->ndig=atoi(cfgit[0].sval);
#endif

#ifdef UNEQUAL_GAME
  fe->ndig=atoi(cfgit[1].sval);
#endif
  fe->ndig++;
  pbkeyb(fe->pbkeybx, fe->pbkeyby, '0', fe->ndig);

  sfree(wintitle);
  free_cfg(cfgit);

#endif

  midend_redraw(me);
}

static void ChangeOrientation(void) {
  SetOrientation(pbOrient);
  pbSW=ScreenWidth(); 
  pbSH=ScreenHeight();
  PrepareGame();
#ifdef PBKEYB
  pbkeyb(fe->pbkeybx, fe->pbkeyby, '0', fe->ndig);
#endif
  midend_size(me, &fe->inkSW, &fe->inkSH, TRUE);
  midend_redraw(me);
}

void MainMenuHandler(int index) {
  char *mess;
  char *textbuf;
  fe->MenuP=index;

  switch (index) {
	case 101:
            PrepareGame();
            StartNewGame();
            break;
	case 102:
#ifdef PBKEYB
            if (fe->is_pbkeyb) {
              fe->is_pbkeyb=0;
              pbkeyb_sel(0);
            }
#endif
            midend_restart_game(me);
            FullUpdate();
            break;
	case 103:
            load_state();
            PrepareGame();
            midend_size(me, &fe->inkSW, &fe->inkSH, TRUE);
            midend_redraw(me);
            break;
	case 104:
            save_state();
            FullUpdate();
            break;
	case 105:
            midend_process_key(me, 0, 0, 'u');
            FullUpdate();
            break;
	case 106:
            midend_process_key(me, 0, 0, 'r');
            FullUpdate();
            break;                                
	case 107:
            mess = midend_solve(me);
            if (mess) Message(ICON_WARNING, "Unable to solve", mess,5000);
            FullUpdate();
            break;
	case 108:
            textbuf = snewn(128, char);
            sprintf(textbuf,"%s\n\nfrom Simon Tatham's Portable Puzzle Collection\n%s\nport for PocketBook by mnk", thegame.name, ver);
            Message(ICON_INFORMATION, "About", textbuf, 10000);
            sfree(textbuf);
            FullUpdate();
            break;
	case 109:
            Message(0, "Help", HelpMessage, 100000);
            FullUpdate();
            break;
	case 110:
            CloseApp();
            break;
	case 200 ... 250:
            TypeMenu[midend_which_preset(me)].type = ITEM_ACTIVE;
            midend_fetch_preset(me, index-200, &TypeMenu[index-200].text, &fe->pparams);
            midend_set_params(me, fe->pparams);
            TypeMenu[index-200].type = ITEM_BULLET;
            PrepareGame();
            StartNewGame();
            break;
        case 111:
            pbOrient=(pbOrient==1)?3:(pbOrient==2)?0:pbOrient^1;
            ChangeOrientation();
            break;
        case 112:
            pbOrient=(pbOrient==2)?3:(pbOrient==1)?0:pbOrient^2;
            ChangeOrientation();
            break;
        default:
            FullUpdate();
            break;
	}
}

int main_handler(int type, int par1, int par2) {

  int i, np;

  if (type == EVT_INIT) {
//CalibrateTouchpanel();
    pbSW=ScreenWidth(); 
    pbSH=ScreenHeight();
    pbOrient=GetOrientation();
    fe = snew(frontend);
    fe->statusfont = OpenFont("LiberationSans", 25, 0);
    fe->statustext = NULL;

#ifdef GUESS_GAME
    fe->guessfont = OpenFont("LiberationSans", 25, 0);
#endif

    fe->MenuP=0;
    fe->isTimer=0;

#ifdef PBKEYB
    fe->is_pbkeyb=0;
#endif

#ifdef INERTIA_GAME
    fe->inerdir=0;
#endif

    me = midend_new(fe, &thegame, &ink_drawing, fe);
    np = midend_num_presets(me);

    TypeMenu=snewn(np+1, imenu);
  
    for (i=0; i<np; i++) {
      midend_fetch_preset(me, i, &TypeMenu[i].text, &fe->pparams);
      TypeMenu[i].type = ITEM_ACTIVE;
      TypeMenu[i].index = 200+i;
      TypeMenu[i].submenu = NULL;
    }
    TypeMenu[i].type = 0;
    TypeMenu[i].index = 0;
    TypeMenu[i].text = NULL;
    TypeMenu[i].submenu = NULL;
    TypeMenu[midend_which_preset(me)].type = ITEM_BULLET;

    MainMenu[4].submenu = TypeMenu;

    PrepareGame();
    StartNewGame();
    return 0;
  }
  else if (type == EVT_SHOW) {
    midend_redraw(me);
    return 0;
  }
  else if (type == EVT_ORIENTATION) {
    if (pbOrient!=par1) {
      pbOrient=par1;
      ChangeOrientation();
    }
    return 0;
  }
  else if (type == EVT_POINTERDOWN) {
//fprintf(stderr, "pointer: %i, %i\n", par1, par2);
    midend_process_key(me, par1, par2, LEFT_BUTTON);
  }
  else if (type == EVT_POINTERUP) {
    midend_process_key(me, par1, par2, LEFT_RELEASE);
  }
  else if (type == EVT_POINTERMOVE) {
    midend_process_key(me, par1, par2, LEFT_DRAG);
  }
  else if (type == EVT_KEYPRESS) {
    switch (par1) {
#ifdef INERTIA_GAME
      int KEYKEY;
      case KEY_LEFT:
          if (fe->inerdir) KEYKEY=MOD_NUM_KEYPAD | '1';
          else KEYKEY=CURSOR_LEFT;
          midend_process_key(me, 0, 0, KEYKEY);
          break;
      case KEY_RIGHT:
          if (fe->inerdir) KEYKEY=MOD_NUM_KEYPAD | '9';
          else KEYKEY=CURSOR_RIGHT;
          midend_process_key(me, 0, 0, KEYKEY);
          break;
      case KEY_UP:
          if (fe->inerdir) KEYKEY=MOD_NUM_KEYPAD | '7';
          else KEYKEY=CURSOR_UP;
          midend_process_key(me, 0, 0, KEYKEY);
          break;
      case KEY_DOWN:
          if (fe->inerdir) KEYKEY=MOD_NUM_KEYPAD | '3';
          else KEYKEY=CURSOR_DOWN;
          midend_process_key(me, 0, 0, KEYKEY);
          break;

#elif defined PBKEYB

      case KEY_LEFT:
          if (fe->is_pbkeyb) {
            pbkeyb_sel(1);
            break;
          }
          else {
            midend_process_key(me, 0, 0, CURSOR_LEFT);
            break;
          }
      case KEY_RIGHT:
          if (fe->is_pbkeyb) {
            pbkeyb_sel(2);
            break;
          }
          else {
            midend_process_key(me, 0, 0, CURSOR_RIGHT);
            break;
          }
      case KEY_UP:
          if (fe->is_pbkeyb) {
            pbkeyb_sel(3);
            break;
          }
          else {
            midend_process_key(me, 0, 0, CURSOR_UP);
            break;
          }
      case KEY_DOWN:
          if (fe->is_pbkeyb) {
            pbkeyb_sel(4);
            break;
          }
          else {
            midend_process_key(me, 0, 0, CURSOR_DOWN);
            break;
          }
         
#else
      case KEY_LEFT:
          midend_process_key(me, 0, 0, CURSOR_LEFT);
          break;
      case KEY_RIGHT:
          midend_process_key(me, 0, 0, CURSOR_RIGHT);
          break;
      case KEY_UP:
          midend_process_key(me, 0, 0, CURSOR_UP);
          break;
      case KEY_DOWN:
          midend_process_key(me, 0, 0, CURSOR_DOWN);
          break;
#endif

#ifdef GUESS_GAME
      case KEY_NEXT:
      case KEY_PLUS:
          midend_process_key(me, 0, 0, 'h');
          break;

#elif defined NET_GAME
      case KEY_NEXT:
      case KEY_PLUS:
          midend_process_key(me, 0, 0, 'd');
          break;
      case KEY_PREV:
      case KEY_MINUS:
          midend_process_key(me, 0, 0, ' ');
          break;

#else
      case KEY_NEXT:
      case KEY_PLUS:
          midend_process_key(me, 0, 0, ' ');
          break;
#endif

    }
  }
  else if (type == EVT_KEYREPEAT) {
    switch (par1) {
      case KEY_BACK:
      case KEY_OK:
          OpenMenu(MainMenu, fe->MenuP, 10, 10, MainMenuHandler);
          break;
      case KEY_LEFT:
          midend_process_key(me, 0, 0, CURSOR_LEFT);
          break;
      case KEY_RIGHT:
          midend_process_key(me, 0, 0, CURSOR_RIGHT);
          break;
      case KEY_UP:
          midend_process_key(me, 0, 0, CURSOR_UP);
          break;
      case KEY_DOWN:
          midend_process_key(me, 0, 0, CURSOR_DOWN);
          break;
      case KEY_PREV:
          CloseApp();
          return 0;
    }
  }
  else if ((type == EVT_KEYRELEASE)&&(par1==KEY_OK)) {

#ifdef PBKEYB
    if (fe->is_pbkeyb) {
      pbkeyb_sel(0);
      midend_process_key(me, 0, 0, pbkeyb_sel(5));
      fe->is_pbkeyb=0;
    }
    else {
      fe->is_pbkeyb=1;
      pbkeyb_sel(0);
    }
#elif defined INERTIA_GAME
    fe->inerdir=(fe->inerdir)?0:1;
    ink_drawarr(fe->inerdir);
#else
    midend_process_key(me, 0, 0, CURSOR_SELECT);
#endif
  }
  return 0;
}

int main(int argc, char **argv) {

  InkViewMain(main_handler);
  return 0;
}

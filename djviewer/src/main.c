/*
 * Copyright (C) 2008 Most Publishing
 * Copyright (C) 2008 Dmitry Zakharov <dmitry-z@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <stdio.h>
#include <inkview.h>
#include <inkinternal.h>
#include "djviewer.h"
#include <math.h>
#include <sys/wait.h>

#ifdef WIN32
#undef WIN32
#include "libdjvu/ddjvuapi.h"
#define WIN32
#else
#include "libdjvu/ddjvuapi.h"
#endif

#include "zoomer.h"

extern const ibitmap zoombm;
extern const ibitmap hgicon;

#define MAXRESULTS 200

#define WORKAROUND_OF_CRASH_ON_EMPTY_DICTIONARY

// #define die(x...) { fprintf(stderr, x); exit(1); }

static imenu zoom_menu_p[] = {

	{ ITEM_HEADER, 0, "@Zoom", NULL },
	{ ITEM_ACTIVE, 33, "@9_pages", NULL },
	{ ITEM_ACTIVE, 50, "@4_pages", NULL },
	{ ITEM_ACTIVE, 90, "90%", NULL },
	{ ITEM_ACTIVE, 100, "100%", NULL },
	{ ITEM_ACTIVE, 110, "110%", NULL },
	{ ITEM_ACTIVE, 120, "120%", NULL },
	{ ITEM_ACTIVE, 200, "@2_columns", NULL },
	{ ITEM_ACTIVE, 300, "@3_columns", NULL },
	{ ITEM_ACTIVE, 400, "@4_columns", NULL },
	{ 0, 0, NULL, NULL }

};

static imenu zoom_menu_l[] = {

	{ ITEM_HEADER, 0, "@Zoom", NULL },
	{ ITEM_ACTIVE, 33, "@6_pages", NULL },
	{ ITEM_ACTIVE, 50, "@2_pages", NULL },
	{ ITEM_ACTIVE, 90, "90%", NULL },
	{ ITEM_ACTIVE, 100, "100%", NULL },
	{ ITEM_ACTIVE, 110, "110%", NULL },
	{ ITEM_ACTIVE, 120, "120%", NULL },
	{ ITEM_ACTIVE, 200, "@2_columns", NULL },
	{ ITEM_ACTIVE, 300, "@3_columns", NULL },
	{ ITEM_ACTIVE, 400, "@4_columns", NULL },
	{ 0, 0, NULL, NULL }

};


static const char *def_menutext[9] = {
	"@Goto_page", "@Exit", "",
	"@Bookmarks", "@Menu", "@Rotate",
	"@Dictionary", "@Zoom", "@Settings"
};

static const char *def_menuaction[9] = {
	"@KA_goto", "@KA_exit", "@KA_none",
	"@KA_obmk", "@KA_none", "@KA_rtte",
	"@KA_dict", "@KA_zoom", "@KA_stgs"
};

static char *yes_no_variants[] = { "No", "Yes", NULL };

static char *level_of_black_variants[] = { "Default", "+10%", "+20%", "+30%", "+40%", "+50%", "+60%", "+70%", "+80%", "+90%", "+100%", NULL};

static iconfigedit djvu_config[] = {
	{ "Draw end of page", "draw_end_of_page", CFG_INDEX, "0", yes_no_variants },
	{ "Level of black", "level_of_black", CFG_INDEX, "0", level_of_black_variants },
	{ NULL, NULL, 0, NULL, NULL}
};

char *strings3x3[9];

ddjvu_context_t *ctx;
ddjvu_document_t *doc;

tdocstate docstate;

pid_t bgpid = 0;

static iconfig* djvucfg;

static int SCALES[9] = { 33, 50, 90, 100, 110, 120, 200, 300, 400};
#define NSCALES ((int)(sizeof(SCALES)/sizeof(int)))

static char *book_title="";
static int orient=0;
static int cpage=1, npages=1;
static int cpagew, cpageh;
static int offx, offy, oldoffy;
static int scale=100;
static int offset;
static int calc_optimal_zoom;
static int thx, thy, thw, thh, thix, thiy, thiw, thih, panh, pgbottom;
static int zoom_mode=0;
static char *FileName;
static char *DataFile;
static int ready_sent = 0;
static int bmkrem;

static long long bmkpos[32];

static char *keyact0[32], *keyact1[32];

static ibitmap *m3x3;
static ibitmap *bmk_flag;

static inline int is_portrait() { return (orient == 0 || orient == 3); }

static void find_off(int step)
{
	int sw=ScreenWidth();
	int sh=ScreenHeight();

        oldoffy = offy;

	offy+=((sh-panh*2)*step);

	if (step > 0 && pgbottom == 1)
 	{
 	 offy=0;
 	 offx+=sw;
 	 if (scale < 200 || offx>=cpagew)
 	 	{
		 if (cpage == npages) return;
 	 	 offx=0;
 	 	 cpage++;
 	 	}
 	 	else
 	 	if (offx>=cpagew-sw)
			offx=cpagew-sw;
 	}
	else if (offy>cpageh-(sh-panh)) {
		offy = cpageh-(sh-panh);
	}

	if (offy<0 && offy>-(sh-panh*2)) {
		offy = 0;
	} else if (offy==-(sh-panh*2))
 	{
 	 offy=cpageh-(sh-panh);
 	 offx-=sw;
 	 if (scale < 200 || offx<=-sw)
 	 	{

		 if (cpage == 1) return;
 	 	 offx=cpagew-sw;
 	 	 cpage--;
 	 	}
 	 	else
 	 	if (offx<=0)
			offx=0;
 	}

  //if (offy <= 24) offy = 0;
  //if (offy >= cpageh-sh-24) offy = cpageh-sh;

}

/* Djvuapi events */

static void handle(int wait)
{
  const ddjvu_message_t *msg;
  if (!ctx)
    return;
  if (wait)
    msg = ddjvu_message_wait(ctx);
  while ((msg = ddjvu_message_peek(ctx)))
    {
      switch(msg->m_any.tag)
        {
        case DDJVU_ERROR:
          fprintf(stderr,"ddjvu: %s\n", msg->m_error.message);
	  break;
        default:
          break;
        }
      ddjvu_message_pop(ctx);
    }
}


static void draw_page_image() {

	int sw, sh, pw, ph, scrx, scry, w, h, rowsize;
	ddjvu_page_t *page;
	ddjvu_rect_t prect;
	ddjvu_rect_t rrect;
	ddjvu_format_style_t style;
	ddjvu_render_mode_t mode;
	ddjvu_format_t *fmt;
	unsigned char *data;

	pgbottom=0;
	FillArea(thx, thy, thw, thh, WHITE);
	DrawBitmap(thx+(thw-hgicon.width)/2, thy+(thh-hgicon.height)/2, &hgicon);
	PartialUpdate(thx, thy, thw, thh);

	SetOrientation(orient);
	sw = ScreenWidth();
	sh = ScreenHeight();

	if (! (page = ddjvu_page_create_by_pageno(doc, cpage-1))) {
		fprintf(stderr, "Cannot access page %d.\n", cpage);
		return;
	}
	while (! ddjvu_page_decoding_done(page))
		handle(TRUE);
	if (ddjvu_page_decoding_error(page)) {
		fprintf(stderr, "Cannot decode page %d.\n", cpage);
		ddjvu_page_release(page);
		return;
	}

	pw = ddjvu_page_get_width(page);
	ph = ddjvu_page_get_height(page);

	scrx = scry = 0;
	w = sw;
	h = sh;

	prect.x = 0;
	prect.y = 0;
	prect.w = cpagew = (sw*scale)/100;
	cpageh = (prect.w * ph) / pw;
	prect.h = (prect.w * (ph - panh - 1)) / pw;

	if (scale > 50 && scale < 200) {
		offx = (cpagew - sw) / 2;
		//if (is_portrait()) offy = (cpageh - sh) / 2;
	}


	rrect.w = sw;
	rrect.h = sh;

	if (prect.w < sw) {
		scrx = (sw-prect.w) / 2;
		offx = 0;
		rrect.w = prect.w;
	} else {
		if (offx>prect.w-sw) offx=prect.w-sw;
		if (offx < 0) offx = 0;
	}
	if (prect.h < sh) {
		scry = (sh-prect.h) / 2;
		offy = 0;
		rrect.h = prect.h;
	} else {
		if (offy>=prect.h-sh) {
			offy=prect.h-sh;
			pgbottom=1;
		}
		if (offy < 0) offy = 0;
	}

	rrect.x = offx;
	rrect.y = offy;

	mode = DDJVU_RENDER_COLOR;
	style = DDJVU_FORMAT_GREY8;

	if (! (fmt = ddjvu_format_create(style, 0, 0))) {
		fprintf(stderr, "Cannot determine pixel style\n");
	}
	ddjvu_format_set_row_order(fmt, 1);
	ddjvu_format_set_y_direction(fmt, 1);
	rowsize = rrect.w;
	if (! (data = (unsigned char *)malloc(rowsize * rrect.h))) {
		fprintf(stderr, "Cannot allocate image buffer\n");
	}

        rrect.x += offset;

        if (rrect.x < 0) rrect.x=0;

        if (rrect.x + rrect.w >= prect.w)
        {
            rrect.x = prect.w - rrect.w;
        }

	if (! ddjvu_page_render(page, mode, &prect, &rrect, fmt, rowsize, (char *)data)) {
		fprintf(stderr, "Cannot render image\n");
		ddjvu_page_release(page);
		return;
	}

        if (ReadInt(djvucfg, "draw_end_of_page", 0) != 0)
        {
            if (scale > 50 && scale < 200 && offy > oldoffy)
            {
                int final_offset_y = thy - (offy - oldoffy);

                if (final_offset_y > 100)
                {
                    int x;
                    for (x =0; x < ScreenWidth(); x += 10)
                    {
                        data[(thy - (offy - oldoffy)) * ScreenWidth() + x] = 0;
                    }
                }
            }
        }

        int level_of_black = ReadInt(djvucfg, "level_of_black", 0);

        if (level_of_black != 0)
        {
            level_of_black = 10 * level_of_black;

            int i;
            for (i = 0; i < rowsize * rrect.h; ++i)
            {
                if (data[i] < 200)
                {
                    if (data[i] < level_of_black)
                    {
                        data[i] = 0;
                    }
                    else
                    {
                        data[i] -= level_of_black;
                    }
                }
            }
        }

	w = rrect.w;
	h = rrect.h;
	Stretch(data, IMAGE_GRAY8, w, h, rowsize, scrx, scry, w, h, 0);
	
	ddjvu_format_release(fmt);
	free(data);
	ddjvu_page_release(page);

	thiw = (sw * 100) / cpagew; if (thiw > 100) thiw = 100;
	thih = (sh * 100) / cpageh; if (thih > 100) thih = 100;
	thix = (offx * 100) / cpagew; if (thix < 0) thix = 0; if (thix+thiw>100) thix = 100-thiw;
	thiy = (offy * 100) / cpageh; if (thiy < 0) thiy = 0; if (thiy+thih>100) thiy = 100-thih;

}

static void bg_monitor() {

	char buf[64];
	struct stat st;
	int status;

	if (bgpid == 0) return;
	waitpid(-1, &status, WNOHANG);
	sprintf(buf, "/proc/%i/cmdline", bgpid);
	if (stat(buf, &st) == -1) {
		bgpid = 0;
		return;
	}
	SetHardTimer("BGPAINT", bg_monitor, 500);

}

static int draw_pages() {

	int sw, sh, pw, ph, nx, ny, boxx, boxy, boxw, boxh, xx, yy, n, rowsize, w, h, size;
	ddjvu_page_t *page;
	ddjvu_rect_t prect;
	ddjvu_rect_t rrect;
	ddjvu_format_t *fmt;
	unsigned char *data;
	pid_t pid;

	sw = ScreenWidth();
	sh = ScreenHeight();

	if (is_portrait() && scale == 50) {
		nx = ny = 2;
	} else if (is_portrait() && scale == 33) {
		nx = ny = 3;
	} else if (!is_portrait() && scale == 50) {
		nx = 2; ny = 1;
	} else if (!is_portrait() && scale == 33) {
		nx = 3; ny = 2;
	} else {
		return 1;
	}
	boxw = sw/nx;
	boxh = (sh-30)/ny;

	for (yy=0; yy<ny; yy++) {
		for (xx=0; xx<nx; xx++) {
			if (cpage+yy*nx+xx > npages) break;
			boxx = xx*boxw;
			boxy = yy*boxh;
			DrawRect(boxx+6, boxy+boxh-4, boxw-8, 2, DGRAY);
			DrawRect(boxx+boxw-4, boxy+6, 2, boxh-8, DGRAY);
			DrawRect(boxx+4, boxy+4, boxw-8, boxh-8, BLACK);
		}
	}



#ifndef EMULATOR

	if ((pid = fork()) == 0) {

#else

	FullUpdate();

#endif

		if (! (fmt = ddjvu_format_create(DDJVU_FORMAT_GREY8, 0, 0))) {
			fprintf(stderr, "Cannot determine pixel style\n");
		}
		ddjvu_format_set_row_order(fmt, 1);
		ddjvu_format_set_y_direction(fmt, 1);

		w = boxw-10;
		h = boxh-10;
		size = w*h;
		data = (unsigned char *) malloc(size);

		for (yy=0; yy<ny; yy++) {
			for (xx=0; xx<nx; xx++) {

				n = cpage+yy*nx+xx;
				if (n > npages) break;
				boxx = xx*boxw;
				boxy = yy*boxh;
				w = boxw-10;
				h = boxh-10;
				memset(data, 0xff, size);

				if (! (page = ddjvu_page_create_by_pageno(doc, n-1))) {
					fprintf(stderr, "Cannot access page %d.\n", cpage);
					return 1;
				}
				while (! ddjvu_page_decoding_done(page))
					handle(TRUE);
				if (ddjvu_page_decoding_error(page)) {
					fprintf(stderr, "Cannot decode page %d.\n", cpage);
					ddjvu_page_release(page);
					break;
				}

				pw = ddjvu_page_get_width(page);
				ph = ddjvu_page_get_height(page);

				prect.x = 0;
				prect.y = 0;
				prect.w = w;
				prect.h = h;

				rrect.x = 0;
				rrect.y = 0;
				rrect.w = w;
				rrect.h = h;

				rowsize = w;

				if (! ddjvu_page_render(page, DDJVU_RENDER_BLACK, &prect, &rrect, fmt, rowsize, (char *)data)) {
					fprintf(stderr, "Cannot render image\n");
					ddjvu_page_release(page);
					return 1;
				}

				ddjvu_page_release(page);

				Stretch(data, IMAGE_GRAY8, w, h, rowsize, boxx+5, boxy+5, w, h, 0);
				PartialUpdate(boxx+5, boxy+5, boxw, boxh);

			}
		}

		ddjvu_format_release(fmt);
		free(data);


#ifndef EMULATOR

		exit(0);

	} else {

		bgpid = pid;

	}

#endif

	SetHardTimer("BGPAINT", bg_monitor, 500);
	return cpage+nx*ny > npages ? npages+1-cpage : nx*ny;


}

static void kill_bgpainter() {

#ifndef EMULATOR

	int status;

	if (bgpid != 0) {
		kill(bgpid, SIGKILL);
		usleep(100000);
		waitpid(-1, &status, WNOHANG);
		bgpid = 0;
	}

#endif

}

static void draw_zoomer() {
	DrawBitmap(ScreenWidth()-zoombm.width-10, ScreenHeight()-zoombm.height-35, &zoombm);
}

static void draw_bmk_flag(int update) {
	int i, x, y, bmkset=0;
	if (! bmk_flag) return;
	for (i=0; i<docstate.nbmk; i++) {
		if (docstate.bmk[i] == cpage) bmkset = 1;
	}
	if (! bmkset) return;
	x = ScreenWidth()-bmk_flag->width;
	y = 0;
	DrawBitmap(x, y, bmk_flag);
	if (update) PartialUpdate(x, y, bmk_flag->width, bmk_flag->height);
}

static void out_page(int full) {

	char buf[48];
	int n=1, h;

	ClearScreen();

        if (calc_optimal_zoom)
        {
            CalculateOptimalZoom(doc, cpage, &scale, &offset);
        }

	if (scale>50)
			draw_page_image();
		else
			n = draw_pages();
	if (zoom_mode) {
		draw_zoomer();
	}
	if (n == 1) {
		sprintf(buf, "    %i / %i    %i%%", cpage, npages, scale);
	} else {
		sprintf(buf, "    %i-%i / %i    %i%%", cpage, cpage+n-1, npages, scale);
	}
	h = DrawPanel(NULL, buf, book_title, (cpage * 100) / npages);
	thx = 4;
	thy = ScreenHeight()-h+4;
	thh = h-6;
	thw = (thh*6)/8;
	FillArea(thx, thy, thw, thh, WHITE);
	thx+=1; thy+=1; thw-=2; thh-=2;
	if (1/*scale >= 200*/) {
		FillArea(thx+(thw*thix)/100, thy+(thh*thiy)/100, (thw*thiw)/100, (thh*thih)/100, BLACK);
	}
	draw_bmk_flag(0);

	if (full) {
		FullUpdate();
	} else {
		PartialUpdate(0, 0, ScreenWidth(), ScreenHeight());
	}
}

static void open_notes_menu() {

	OpenNotesMenu(FileName, book_title, cpage);

}

static void page_selected(int page) {

	if (page < 1) page = 1;
	if (page > npages) page = npages;
	cpage = page;
	offx = offy = 0;
	out_page(1);

}

static void bmk_added(int page) {

	char buf[256];

	sprintf(buf,"%s %i", GetLangText("@Add_Bmk_P"), page);
	Message(ICON_INFORMATION, GetLangText("@Bookmarks"), buf, 1500);
	draw_bmk_flag(1);

}

static void bmk_handler(int action, int page, long long pos) {

	switch (action) {
		case BMK_ADDED: bmk_added(page); break;
		case BMK_REMOVED: bmkrem = 1; break;
		case BMK_SELECTED: page_selected(page); break;
		case BMK_CLOSED: if (bmkrem) out_page(0); break;
	}

}

static void open_bookmarks() {

	int i;

	bmkrem = 0;
	for (i=0; i<docstate.nbmk; i++) bmkpos[i] = docstate.bmk[i];
	OpenBookmarks(cpage, cpage, docstate.bmk, bmkpos, &docstate.nbmk, 30, bmk_handler);

}

static void new_bookmark() {

	int i;

	bmkrem = 0;
	for (i=0; i<docstate.nbmk; i++) bmkpos[i] = docstate.bmk[i];
	SwitchBookmark(cpage, cpage, docstate.bmk, bmkpos, &docstate.nbmk, 30, bmk_handler);
	if (bmkrem) out_page(0);

}

static void zoom_menu_handler(int index) {

	if (index < 0) return;
	scale = index;
	out_page(1);

}

static void rotate_handler(int n) {

	orient = n;
	offx = offy = 0;
	out_page(1);

}

static void do_zoom(int add) {

	int i;

	for (i=0; i<NSCALES; i++) {
		if (SCALES[i] == scale) break;
	}
	if (i == NSCALES) {
		scale = 100;
		out_page(0);
	} else {
		if (i+add >= 0 && i+add < NSCALES) {
			i+=add;
			scale = SCALES[i];
			out_page(0);
		}
	}

}

static void turn_page(int n) {

	offx = 0;
	offy = 0;
	if (n > 0 && cpage < npages) {
		cpage += n;
		if (cpage < 1) cpage = 1;
		out_page(1);
	}
	if (n < 0 && cpage > 1) {
		cpage += n;
		if (cpage > npages) cpage = npages;
		out_page(1);
	}

}

static void jump_pages(int n) {

	char buf[16];
	int x, y, w, h;

	cpage += n;
	if (cpage < 1) cpage = 1;
	if (cpage > npages) cpage = npages;
	offx = offy = 0;
	w = 50;
	h = (menu_n_font->height*3) / 2;
	x = ScreenWidth()-w-5;
	y = ScreenHeight()-h-30;
	FillArea(x+1, y+1, w-2, h-2, WHITE);
	DrawRect(x+1, y, w-2, h, BLACK);
	DrawRect(x, y+1, w, h-2, BLACK);
	sprintf(buf, "%i", cpage);
	SetFont(menu_n_font, BLACK);
	DrawTextRect(x, y, w, h, buf, ALIGN_CENTER | VALIGN_MIDDLE);
	PartialUpdate(x, y, w, h);

}

static int zoom_handler(int type, int par1, int par2) {

	if (type == EVT_SHOW) {
		//out_page(0);
	}

	if (type == EVT_KEYPRESS) {

		kill_bgpainter();

		if (par1 == KEY_UP) {
			do_zoom(+1);
		}
		if (par1 == KEY_DOWN) {
			do_zoom(-1);
		}
		if (par1 == KEY_LEFT) {
			turn_page(-1);
		}
		if (par1 == KEY_RIGHT) {
			turn_page(+1);
		}
		if (par1 == KEY_OK || par1 == KEY_BACK) {
			zoom_mode = 0;
			SetEventHandler(main_handler);
		}

	}

	return 0;

}

static void open_quickmenu() { OpenMenu3x3(m3x3, (const char **)strings3x3, menu_handler); }
static void prev_page() { turn_page(-1); }
static void next_page() { turn_page(+1); }
static void jump_pr10() { jump_pages(-10); }
static void jump_nx10() { jump_pages(+10); }
static void stop_jump() { out_page(1); }
static void open_pageselector()  { OpenPageSelector(page_selected); }
static void first_page() { page_selected(1); }
static void last_page() { page_selected(npages); }
static void new_note() { CreateNote(FileName, book_title, cpage); }
static void save_page_note() { CreateNoteFromPage(FileName, book_title, cpage); }
static void open_notes() { OpenNotepad(NULL); }
static void open_dictionary()
{
#ifdef WORKAROUND_OF_CRASH_ON_EMPTY_DICTIONARY
    static iv_wlist diclist[2];
    diclist[0].word="";
    diclist[0].x1=0;
    diclist[0].x2=0;
    diclist[0].y1=0;
    diclist[0].y2=0;
    diclist[1].word=0;
    OpenDictionaryView(diclist, NULL);
#else
    OpenDictionaryView(NULL, NULL);
#endif
}
static void open_zoomer() { OpenMenu(is_portrait() ? zoom_menu_p : zoom_menu_l, scale, -1, -1, zoom_menu_handler); }
static void zoom_in() { do_zoom(+1); }
static void zoom_out() { do_zoom(-1); }
static void open_rotate() { OpenRotateBox(rotate_handler); }
static void main_menu() { OpenMainMenu(); }
static void exit_reader() { CloseApp(); }
static void open_mp3() { OpenPlayer(); }
static void mp3_pause() { TogglePlaying(); }
static void volume_up() { int r = GetVolume(); SetVolume(r+3); }
static void volume_down() { int r = GetVolume(); SetVolume(r-3); }

static void on_close_zoomer(ZoomerParameters* params)
{
    if (scale != params->zoom || offset != params->offset)
    {
        scale = params->zoom;
        offset = params->offset;
        calc_optimal_zoom = params->optimal_zoom;
        out_page(1);
    }
}

static void open_new_zoomer()
{
    ZoomerParameters params = {0};
    params.zoom = scale;
    params.doc = doc;
    params.cpage = cpage;
    params.orient = orient;
    params.optimal_zoom = calc_optimal_zoom;

    ShowZoomer(&params, on_close_zoomer);
}

void config_ok()
{
    SaveConfig(djvucfg);
    SetEventHandler(main_handler);
}

static void open_settings()
{
    OpenConfigEditor("Configuration", djvucfg, djvu_config, config_ok, NULL);
}

static void handle_navikey(int key) {

	int pageinc=1;

	switch (scale) {
		case 50: pageinc = is_portrait() ? 4 : 2; break;
		case 33: pageinc = is_portrait() ? 9 : 6; break;
	}

	switch (key) {

		case KEY_LEFT:
			turn_page(-pageinc);
			break;

		case KEY_RIGHT:
			turn_page(pageinc);
			break;

		case KEY_UP:
			if (scale<=50) {
				turn_page(-1);
			} else {
			 	find_off(-1);
				out_page(1);
			}
			break;

		case KEY_DOWN:
			if (scale<=50) {
				turn_page(1);
			} else {
				find_off(1);
				out_page(1);
			}
			break;

	}

}


static int act_on_press(char *a0, char *a1) {

	if (a0 == NULL || a1 == NULL || strcmp(a1, "@KA_none") == 0) return 1;
	if (strcmp(a0, "@KA_prev") == 0 || strcmp(a0, "@KA_next") == 0) {
		if (strcmp(a1, "@KA_pr10") == 0) return 1;
		if (strcmp(a1, "@KA_nx10") == 0) return 1;
	}
	return 0;

}

static const struct {
	char *action;
	void (*f1)();
	void (*f2)();
	void (*f3)();
} KA[] = {
	{ "@KA_menu", open_quickmenu, NULL, NULL },
	{ "@KA_prev", prev_page, prev_page, NULL },
	{ "@KA_next", next_page, next_page, NULL },
	{ "@KA_pr10", jump_pr10, jump_pr10, stop_jump },
	{ "@KA_nx10", jump_nx10, jump_nx10, stop_jump },
	{ "@KA_goto", open_pageselector, NULL, NULL },
	{ "@KA_frst", first_page, NULL, NULL },
	{ "@KA_last", last_page, NULL, NULL },
	//{ "@KA_prse", prev_section, NULL, NULL },
	//{ "@KA_nxse", next_section, NULL, NULL },
	{ "@KA_obmk", open_bookmarks, NULL, NULL },
	{ "@KA_nbmk", new_bookmark, NULL, NULL },
	{ "@KA_nnot", new_note, NULL, NULL },
	{ "@KA_savp", save_page_note, NULL, NULL },
	{ "@KA_onot", open_notes, NULL, NULL },
	//{ "@KA_olnk", open_links, NULL, NULL },
	//{ "@KA_blnk", back_link, NULL, NULL },
	//{ "@KA_cnts", open_contents, NULL, NULL },
	//{ "@KA_srch", start_search, NULL, NULL },
	{ "@KA_dict", open_dictionary, NULL, NULL },
	{ "@KA_zoom", open_new_zoomer, NULL, NULL },
	{ "@KA_stgs", open_settings, NULL, NULL },
	{ "@KA_zmin", zoom_in, NULL, NULL },
	{ "@KA_zout", zoom_out, NULL, NULL },
	{ "@KA_rtte", open_rotate, NULL, NULL },
	{ "@KA_mmnu", main_menu, NULL, NULL },
	{ "@KA_exit", exit_reader, NULL, NULL },
	{ "@KA_mp3o", open_mp3, NULL, NULL },
	{ "@KA_mp3p", mp3_pause, NULL, NULL },
	{ "@KA_volp", volume_up, NULL, NULL },
	{ "@KA_volm", volume_down, NULL, NULL },
	{ NULL, NULL, NULL, NULL }
};

static void menu_handler(int pos) {

	char buf[32], *act;
	int i;

	if (pos < 0) return;
	sprintf(buf, "qmenu.djviewer.%i.action", pos);
	act = GetThemeString(buf, (char *)def_menuaction[pos]);

        if (pos == 8)
        {
            act = "@KA_stgs";
        }

	for (i=0; KA[i].action != NULL; i++) {
		if (strcmp(act, KA[i].action) != 0) continue;
		if (KA[i].f1 != NULL) (KA[i].f1)();
		if (KA[i].f3 != NULL) (KA[i].f3)();
		break;
	}

}

static int key_handler(int type, int par1, int par2) {

	char *act0, *act1, *act=NULL;
	int i;

	if (type == EVT_KEYPRESS) kill_bgpainter();

	act0 = keyact0[par1];
	act1 = keyact1[par1];

	if (par1 >= 0x20) {

		// aplhanumeric keys
		return 0;

	} else if (par1 == KEY_BACK) {

		CloseApp();
		return 0;

	} else if (par1 == KEY_POWER)  {

		return 0;

	} else if ((par1==KEY_UP || par1== KEY_DOWN || par1==KEY_LEFT || par1==KEY_RIGHT)
	  && (type == EVT_KEYPRESS || (type == EVT_KEYRELEASE && par2 == 0))
	  ) {
		if (act_on_press(act0, act1)) {
			if (type == EVT_KEYPRESS) handle_navikey(par1);
		} else {
			if (type == EVT_KEYRELEASE) handle_navikey(par1);
		}
		return 0;

	} else {

		if (type == EVT_KEYPRESS && act_on_press(act0, act1)) {
			act = act0;
		} else if (type == EVT_KEYRELEASE && par2 == 0 && !act_on_press(act0, act1)) {
			act = act0;
		} else if (type == EVT_KEYREPEAT) {
			act = act1;
		} else if (type == EVT_KEYRELEASE && par2 > 0) {
			act = act1;
			par2 = -1;
		}
		if (act == NULL) return 1;

		for (i=0; KA[i].action != NULL; i++) {
			if (strcmp(act, KA[i].action) != 0) continue;
			if (par2 == -1) {
				if (KA[i].f3 != NULL) (KA[i].f3)();
			} else if (par2 <= 1) {
				if (KA[i].f1 != NULL) (KA[i].f1)();
				if (act == act0 && KA[i].f3 != NULL) (KA[i].f3)();
			} else {
				if (KA[i].f2 != NULL) (KA[i].f2)();
			}
			break;
		}
		return 1;

	}

}


static int main_handler(int type, int par1, int par2) {

	if (type == EVT_INIT)
		SetOrientation(orient);

	if (type == EVT_EXIT)
		save_settings();

	if (type == EVT_SHOW) {
		out_page(1);
		if (! ready_sent) BookReady(FileName);
		ready_sent = 1;
	}

	if (type == EVT_KEYPRESS || type == EVT_KEYREPEAT || type == EVT_KEYRELEASE) {
		return key_handler(type, par1, par2);
	}

	return 0;

}

static void save_settings() {

  if (docstate.magic != 0x9751) return;
  fprintf(stderr, "djviewer: saving settings\n");
  docstate.page = cpage;
  docstate.offx = offx;
  docstate.offy = offy;

  docstate.scale = scale | (abs(offset) << 16);

  if (offset > 0)
  {
      docstate.scale |= 1 << 31;
  }

  if (calc_optimal_zoom)
  {
      docstate.scale |= 1 << 30;
  }

  FILE *f = iv_fopen(DataFile, "wb");
  if (f != NULL) {
	iv_fwrite(&docstate, 1, sizeof(tdocstate), f);
	iv_fclose(f);
  }
  fprintf(stderr, "djviewer: saving settings done\n");

  CloseConfig(djvucfg);

}

#define die(x...) { \
	fprintf(stderr, x); \
	fprintf(stderr, "\n"); \
	Message(ICON_WARNING, "DejaVu Viewer", "@Cant_open_file", 2000); \
	exit(1); \
}

int main(int argc, char **argv) {

  char buf[1024];
  int i;
  bookinfo *bi;
  FILE *f = NULL;

  OpenScreen();

  m3x3 = GetResource("djviewer_menu", NULL);
  if (m3x3 == NULL) m3x3 = NewBitmap(128, 128);
  bmk_flag = GetResource("bmk_flag", NULL);

  for (i=0; i<9; i++) {
	sprintf(buf, "qmenu.djviewer.%i.text", i);
	strings3x3[i] = GetThemeString(buf, (char *)def_menutext[i]);
  }

  GetKeyMapping(keyact0, keyact1);

  if (argc < 2)
	die("Missing filename");

  FileName=argv[1];
  bi = GetBookInfo(FileName);
  if (bi->title) book_title = strdup(bi->title);

  djvucfg = OpenConfig("/mnt/ext1/system/config/djvu.cfg", djvu_config);

  /* Create context and document */
  if (! (ctx = ddjvu_context_create(argv[0])))
    die("Cannot create djvu context.");
  if (! (doc = ddjvu_document_create_by_filename(ctx, FileName, TRUE)))
    die("Cannot open djvu document '%s'.", FileName);
  while (! ddjvu_document_decoding_done(doc))
    handle(TRUE);

  npages = ddjvu_document_get_pagenum(doc);

  DataFile = GetAssociatedFile(FileName, 0);
  f = fopen(DataFile, "rb");
  
  if (f == NULL || fread(&docstate, 1, sizeof(tdocstate), f) != sizeof(tdocstate) || docstate.magic != 0x9751) {
		docstate.magic = 0x9751;
		docstate.page = 1;
		docstate.offx = 0;
		docstate.offy = 0;
		docstate.scale = 100;
		docstate.orient = 0;
		docstate.nbmk = 0;
  }
  if (f != NULL) fclose(f);

  cpage = docstate.page;
  offx = docstate.offx;
  offy = docstate.offy;
  scale = docstate.scale & 0xFFFF;
  offset = (docstate.scale >> 16) & 0x3FFF;

  if (!(docstate.scale >> 31)) offset =-offset;
  if (((docstate.scale >> 30) & 0x1)) calc_optimal_zoom = 1;

  orient = docstate.orient;
  panh = PanelHeight();

  if (argc >= 3 && argv[2][0] == '=') cpage = atoi(argv[2]+1);
  if (cpage < 1) cpage = 1;
  if (cpage > npages) cpage = npages;

  InkViewMain(main_handler);
  return 0;
}
	

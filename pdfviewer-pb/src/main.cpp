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

#include "pdfviewer.h"

static char *strings3x3[9];

int cpage=1, npages=1, subpage=0, nsubpages=1;
int scale=100, rscale=150;
int calc_optimal_zoom;
int reflow_mode=0;
int orient=0;
int clock_left=0;
int offx, offy;
int scrx, scry;
int flowpage=-1, flowscale, flowwidth, flowheight, savereflow;
int thx, thy, thw, thh, thix, thiy, thiw, thih, panelh;
int search_mode=0;
int zoom_mode=0;
static int bmkrem;
struct sresult results[MAXRESULTS];
int nresults;

char *book_title="";
PDFDoc *doc;
MySplashOutputDev *splashOut;
TextOutputDev *textout;
SearchOutputDev *searchout;
tdocstate docstate;

ibitmap *m3x3;
ibitmap *bmk_flag;


static int SCALES[12] = { 33, 50, 90, 100, 110, 120, 130, 140, 150, 200, 300, 400};
#define NSCALES ((int)(sizeof(SCALES)/sizeof(int)))

static const char *def_menutext[9] = {
	"@Goto_page", "@Exit", "@Search",
	"@Bookmarks", "@Menu", "@Rotate",
	"@Dictionary", "@Zoom", "@Contents"
};

static const char *def_menuaction[9] = {
	"@KA_goto", "@KA_exit", "@KA_srch",
	"@KA_obmk", "@KA_none", "@KA_rtte",
	"@KA_dict", "@KA_zoom", "@KA_cnts"
};

static long long bmkpos[32];

static char kbdbuffer[32];

static int savescale;
static char *FileName;
static char *DataFile;
static int ready_sent = 0;

static char *stext;
static int spage, sdir;

static tocentry *TOC=NULL;
static int tocsize=0, toclen=0;
static char **named_dest=NULL;
static int named_size=0, named_count=0;
static int no_save_state=0;

static char *keyact0[32], *keyact1[32];

static iv_wlist *diclist=NULL;
static int diclen=0;

static int main_handler(int type, int par1, int par2);
static int zoom_handler(int type, int par1, int par2);
static void menu_handler(int pos);
static void save_settings();

static void open_notes_menu() {

	OpenNotesMenu(FileName, book_title, cpage);

}

static void page_selected(int page) {

	if (page < 1) page = 1;
	if (page > npages) page = npages;
	cpage = page;
	subpage = 0;
	offx = offy = 0;
	out_page(1);

}

static void wordlist_normal() {

	double fx1, fx2, fy1, fy2;
	int x1, x2, y1, y2;
	char *s;
	int i, sw, sh, pw, ph, marginx, marginy, len;
	double sres;

	sw = ScreenWidth();
	sh = ScreenHeight();

	textout = new TextOutputDev (NULL, gFalse, gFalse, gFalse);
	getpagesize(cpage, &pw, &ph, &sres, &marginx, &marginy);
	doc->displayPage(textout, cpage, sres, sres, 0, false, true, false );
	TextWordList *wlist = textout->makeWordList();
	if (wlist == NULL) {
		fprintf(stderr, "wlist=NULL\n");
		len = 0;
	} else {
		len = wlist->getLength();
	}
	diclist = (iv_wlist *) malloc((len+1) * sizeof(iv_wlist));
	diclen = 0;
	fprintf(stderr, "len=%i\n", len);
	for (i=0; i<len; i++) {
		TextWord *tw = wlist->get(i);
		s = tw->getText()->getCString();
		tw->getBBox(&fx1, &fy1, &fx2, &fy2);
		x1 = (int)fx1 + scrx - offx;
		x2 = (int)fx2 + scrx - offx;
		y1 = (int)fy1 + scry - offy;
		y2 = (int)fy2 + scry - offy;

		if (s != NULL && strlen(s) > 1 && x1 < sw && x2 > 0 && y1 < sh && y2 > 0) {
			diclist[diclen].word = strdup(s);
			diclist[diclen].x1 = x1-1;
			diclist[diclen].y1 = y1-1;
			diclist[diclen].x2 = x2+1;
			diclist[diclen].y2 = y2+1;
			diclen++;
			// fprintf(stderr, "[%s %i,%i %i,%i] ", s, x1, y1, x2, y2);
		}

	}
	diclist[diclen].word = NULL;
	delete wlist;
	delete textout;


}

static void wordlist_reflow() {

	diclist = splashOut->getWordList(subpage);
	if (diclist) {
		for (diclen=0; diclist[diclen].word != NULL; diclen++) ;
	}
	OpenDictionaryView(diclist, NULL);

}

void open_dictionary() {

	int i;

	if (diclist != NULL) {
		for (i=0; i<diclen; i++) free(diclist[i].word);
		free(diclist);
		diclist = NULL;
	}
	reflow_mode ? wordlist_reflow() : wordlist_normal();
	OpenDictionaryView(diclist, NULL);

}

static void search_timer() {

	double xMin=0, yMin=0, xMax=9999999, yMax=9999999;
	unsigned int ucs4[32];
	unsigned int ucs4_len;
	int i, pw, ph, marginx, marginy;
	double sres;

	if (stext == NULL || ! search_mode) return;
	//fprintf(stderr, "%i\n", spage);

	if (spage < 1 || spage > npages) {
		HideHourglass();
		Message(ICON_INFORMATION, GetLangText("@Search"), GetLangText("@No_more_matches"), 2000);
		nresults = 0;
		return;
	}

	getpagesize(spage, &pw, &ph, &sres, &marginx, &marginy);

/*
  void displayPage(OutputDev *out, int page,
		   double hDPI, double vDPI, int rotate,
		   GBool useMediaBox, GBool crop, GBool printing,
		   GBool (*abortCheckCbk)(void *data) = NULL,
		   void *abortCheckCbkData = NULL,
                   GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data) = NULL,
                   void *annotDisplayDecideCbkData = NULL);
*/

	searchout->found = 0;
	doc->displayPage(searchout, spage, sres, sres, 0, false, false, false );
	if (! searchout->found) {
		spage += sdir;
		SetHardTimer("SEARCH", search_timer, 1);
		return;
	}

	doc->displayPage(textout, spage, sres, sres, 0, false, true, false );
	TextPage *textPage=textout->takeText();

	xMin=yMin=0;
	ucs4_len = utf2ucs4 (stext, (unsigned int *)ucs4, 32);

	nresults = 0;
	while (textPage->findText(ucs4, ucs4_len, 
            gFalse, gTrue, gTrue, gFalse, gFalse, gFalse, &xMin, &yMin, &xMax, &yMax ))
	{
		i=nresults++;
		results[i].x = (int)xMin - 2;
		results[i].y = (int)yMin - 2;
		results[i].w = (int)(xMax-xMin) + 4;
		results[i].h = (int)(yMax-yMin) + 4;
		if (i == MAXRESULTS-1) break;
	}

	if (nresults > 0) {
		cpage = spage;
		out_page(1);
	} else {
		spage += sdir;
		SetHardTimer("SEARCH", search_timer, 1);
	}

	delete textPage;

}


static void do_search(char *text, int frompage, int dir) {

	search_mode = 1;
	stext = strdup(text);
	spage = frompage;
	sdir = dir;
	ShowHourglass();
	search_timer();

}

static void stop_search() {

	ClearTimer(search_timer);
	delete textout;
	delete searchout;
	free(stext);
	stext = NULL;
	search_mode = 0;
	scale = savescale;
	reflow_mode = savereflow;
	SetEventHandler(main_handler);

}

static int search_handler(int type, int par1, int par2) {

	if (type == EVT_SHOW) {
		//out_page(0);
	}

	if (type == EVT_KEYPRESS) {

		if (par1 == KEY_OK || par1 == KEY_BACK) {
			stop_search();
		}
		if (par1 == KEY_LEFT) {
			do_search(stext, spage-1, -1);
		}
		if (par1 == KEY_RIGHT) {
			do_search(stext, spage+1, +1);
		}

	}

	return 0;

}

static void search_enter(char *text) {

	if (text == NULL || text[0] == 0) return;
	savescale = scale;
	savereflow = reflow_mode;
	scale = 105;
	reflow_mode = 0;
	textout = new TextOutputDev (NULL, gFalse, gFalse, gFalse);
	searchout = new SearchOutputDev (text);
	if (! textout->isOk()) {
		fprintf(stderr, "cannot create text output device\n");
		return;
	}

	SetEventHandler(search_handler);
	do_search(text, cpage, +1);

}

static void bmk_added(int page) {

	char buf[256];

	sprintf(buf, "%s %i", GetLangText("@Add_Bmk_P"), page);
	Message(ICON_INFORMATION,GetLangText("@Bookmarks"), buf, 1500);
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

static void toc_handler(long long position) {

	if (position < 100000) {
		cpage = position;
	} else {
		LinkDest *dest = doc->findDest(new GooString(named_dest[position-100000]));
		if (dest && dest->isOk()) {
			Ref page_ref = dest->getPageRef();
			cpage = doc->findPage(page_ref.num, page_ref.gen);
		}
	}
	if (cpage < 1) cpage = 1;
	if (cpage > npages) cpage = npages;
	subpage = 0;
	offx = offy = 0;
	out_page(1);

}

static void zoom_menu_handler(int index) {

	if (index < 0) return;
	if (index == 999) {
		if (rscale < 150) rscale = 150;
		reflow_mode = 1;
	} else {
		scale = index;
		reflow_mode = 0;
	}
	out_page(1);

}

static void rotate_handler(int n) {

	orient = n;
	offx = offy = 0;
	out_page(1);

}

static void turn_page(int n) {

	offx = 0;
	offy = 0;
	if (reflow_mode && n == -1 && subpage > 0) {
		subpage--;
		n = 0;
	}
	if (reflow_mode && n == 1 && subpage+1 < nsubpages) {
		subpage++;
		n = 0;
	}
	if (n > 0 && cpage < npages) {
		cpage += n;
		subpage = 0;
	}
	if (n < 0 && cpage > 1) {
		cpage += n;
		subpage = (reflow_mode && n == -1) ? 9999999 : 0;
	}
	if (cpage < 1) { cpage = 1; subpage = 0; }
	if (cpage > npages) cpage = npages;
	out_page(1);

}

static void jump_pages(int n) {

	char buf[16];
	int x, y, w, h;

	cpage += n;
	subpage = 0;
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

static void open_quickmenu() { OpenMenu3x3(m3x3, (const char **)strings3x3, menu_handler); }
static void prev_page() { turn_page(-1); }
static void next_page() { turn_page(+1); }
static void jump_pr10() { jump_pages(-10); }
static void jump_nx10() { jump_pages(+10); }
static void stop_jump() { out_page(1); }
static void open_pageselector()  { OpenPageSelector(page_selected); }
static void first_page() { page_selected(1); }
static void last_page() { page_selected(npages); }
static void prev_section() { }
static void next_section() { }
static void new_note() { CreateNote(FileName, book_title, cpage); }
static void save_page_note() { CreateNoteFromPage(FileName, book_title, cpage); }
static void open_notes() { OpenNotepad(NULL); }
static void open_contents() {
	if (toclen == 0) {
		Message(ICON_INFORMATION, "PDF Viewer", "@No_contents", 2000);
	} else {
		OpenContents(TOC, toclen, cpage, toc_handler);
	}
}
static void start_search() { OpenKeyboard(GetLangText("@Search"), kbdbuffer, 30, 0, search_enter); }
static void zoom_in() {
	if (reflow_mode) {
		rscale = (rscale < 200) ? rscale+50 : rscale+100;
		if (rscale > 500) rscale = 150;
		out_page(1);
		return;
	}
	update_value(&scale, +1, SC_DIRECT);
	out_page(1);
}
static void zoom_out() {
	if (reflow_mode) {
		rscale = (rscale > 200) ? rscale-100 : rscale-50;
		if (rscale < 150) rscale = 500;
		out_page(1);
		return;
	}
	update_value(&scale, -1, SC_DIRECT);
	out_page(1);
}
static void pdf_mode() {
	if (reflow_mode) {
		reflow_mode = 0;
	} else {
		reflow_mode = 1;
		if (rscale < 150) rscale = 150;
	}
	out_page(1);
}
static void open_rotate() { OpenRotateBox(rotate_handler); }
static void main_menu() { OpenMainMenu(); }
static void exit_reader() { CloseApp(); }
static void open_mp3() { OpenPlayer(); }
static void mp3_pause() { TogglePlaying(); }
static void volume_up() { int r = GetVolume(); SetVolume(r+3); }
static void volume_down() { int r = GetVolume(); SetVolume(r-3); }
	
static void do_zoom(int add) {

	int i;

	if (reflow_mode) {
		if (add > 0) {
			rscale = (rscale < 200) ? rscale+50 : rscale+100;
			if (rscale > 500) rscale = 150;
		} else {
			rscale = (rscale > 200) ? rscale-100 : rscale-50;
			if (rscale < 150) rscale = 500;
		}
		out_page(0);
		return;
	}

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
			prev_page();
		}
		if (par1 == KEY_RIGHT) {
			next_page();
		}
		if (par1 == KEY_OK || par1 == KEY_BACK) {
			zoom_mode = 0;
			SetEventHandler(main_handler);
		}

	}

	return 0;

}

static void handle_navikey(int key) {

	int pageinc=1;

	switch (scale) {
		case 50: pageinc = is_portrait() ? 4 : 2; break;
		case 33: pageinc = is_portrait() ? 9 : 6; break;
	}
	if (reflow_mode) pageinc = 1;

	switch (key) {

		case KEY_LEFT:
			if (scale >= 200) {
				find_off_x(-1);
				out_page(1);
			} else {
				turn_page(-pageinc);
			}
			break;

		case KEY_RIGHT:
			if (scale >= 200) {
				find_off_x(+1);
				out_page(1);
			} else {
				turn_page(pageinc);
			}
			break;

		case KEY_UP:
		 	find_off(-1);
			out_page(1);
			break;

		case KEY_DOWN:
			find_off(+1);
			out_page(1);
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
	{ "@KA_prse", prev_section, NULL, NULL },
	{ "@KA_nxse", next_section, NULL, NULL },
	{ "@KA_obmk", open_bookmarks, NULL, NULL },
	{ "@KA_nbmk", new_bookmark, NULL, NULL },
	{ "@KA_nnot", new_note, NULL, NULL },
	{ "@KA_savp", save_page_note, NULL, NULL },
	{ "@KA_onot", open_notes, NULL, NULL },
	{ "@KA_mpdf", pdf_mode, NULL, NULL },
	//{ "@KA_olnk", open_links, NULL, NULL },
	//{ "@KA_blnk", back_link, NULL, NULL },
	{ "@KA_cnts", open_contents, NULL, NULL },
	{ "@KA_srch", start_search, NULL, NULL },
	{ "@KA_dict", open_dictionary, NULL, NULL },
	{ "@KA_zoom", open_zoomer, NULL, NULL },
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
	sprintf(buf, "qmenu.pdfviewer.%i.action", pos);
	act = GetThemeString(buf, (char *)def_menuaction[pos]);

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
	  && ! reflow_mode) {

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


	if (type == EVT_INIT) {
		SetOrientation(orient);
		panelh = PanelHeight();
	}

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

static void add_toc_item(int level, char *label, int page, long long position) {

	//printf("%i %i %s\n", level, page, label);

	if (tocsize <= toclen+1) {
		tocsize += 64;
		TOC = (tocentry *) realloc(TOC, tocsize * sizeof(tocentry));
	}
	TOC[toclen].level = level;
	TOC[toclen].page = page;
	TOC[toclen].position = position;
	TOC[toclen].text = strdup(label);
	toclen++;

}

static void update_toc(GooList *items, int level) {

	unsigned short ucs[256];
	char label[256];
	int i, j;

	if (! items) return;
	if (items->getLength() < 1) return;

	for (i=0; i<items->getLength(); i++) {

		OutlineItem *outlineItem = (OutlineItem *)items->get( i );
		Unicode *title = outlineItem->getTitle();
		int tlen = outlineItem->getTitleLength();
		if (tlen > sizeof(ucs)-1) tlen = sizeof(ucs)-1;
		for (j=0; j<tlen; j++) ucs[j] = (unsigned short)title[j];
		ucs[j] = 0;
		ucs2utf(ucs, label, sizeof(label));

		LinkAction *a = outlineItem->getAction();
		if (a && (a->getKind() == actionGoTo)) {

	                // page number is contained/referenced in a LinkGoTo
	                LinkGoTo * g = static_cast< LinkGoTo * >( a );
	                LinkDest * destination = g->getDest();
	                if ( !destination && g->getNamedDest() )
	                {
				GooString *s = g->getNamedDest();
				if (named_size <= named_count+1) {
					named_size += 64;
					named_dest = (char **) realloc(named_dest, named_size * sizeof(char *));
				}
				named_dest[named_count] = strdup(s->getCString());
				add_toc_item(level, label, -1, 100000+named_count);
				named_count++;
			}
			else if ( destination && destination->isOk() && destination->isPageRef() )
			{
				Ref page_ref = destination->getPageRef();
				int num = doc->findPage(page_ref.num, page_ref.gen);
				add_toc_item(level, label, num, num);
			}
			else
			{
				add_toc_item(level, label, -1, -1);
			}

		} else {
			add_toc_item(level, label, -1, -1);
		}

		outlineItem->open();
		GooList * children = outlineItem->getKids();
		if (children) update_toc(children, level+1);
		outlineItem->close();

	}

}

static void save_settings() {

  if (no_save_state) return;
  if (docstate.magic != 0x9751) return;
  fprintf(stderr, "pdfviewer: saving settings...\n");

  docstate.page = cpage;
  docstate.subpage = subpage;
  docstate.offx = offx;
  docstate.offy = offy;
  docstate.scale = scale;
  if (calc_optimal_zoom) docstate.scale |= 1 << 15;

  docstate.rscale = rscale;
  docstate.orient = orient;
  if (reflow_mode) docstate.orient |= 0x80;

  FILE *f = iv_fopen(DataFile, "wb");
  if (f != NULL) {
	iv_fwrite(&docstate, 1, sizeof(tdocstate), f);
	iv_fclose(f);
  }
  sync();
  fprintf(stderr, "pdfviewer: saving settings done\n");

}

// against compiler bugs
void x_ready_sent(int n) { ready_sent = n; }

static void sigsegv_handler(int signum) {

	x_ready_sent(ready_sent);
	if (! ready_sent) {
		fprintf(stderr, "SIGSEGV caught - clearing state\n");
		iv_unlink(DataFile);
		no_save_state = 1;
	} else {
		fprintf(stderr, "SIGSEGV caught\n");
	}
	exit(127);

}

static void sigfpe_handler(int signum) {

	fprintf(stderr, "SIGFPE\n");

}

int main(int argc, char **argv) {

  SplashColor paperColor;
  GooString *filename, *password;
  char *spwd;
  FILE *f = NULL;
  bookinfo *bi;
  char buf[1024];
  int i;

  mkdir(CACHEDIR, 0777);
  chmod(CACHEDIR, 0777);

  spwd = GetDeviceKey();

  if (setgid(102) != 0) fprintf(stderr, "warning: cannot set gid\n");
  if (setuid(102) != 0) fprintf(stderr, "warning: cannot set uid\n");

  if (spwd) {
fprintf(stderr, "password: %s\n", spwd);
	password = new GooString(spwd);
  } else {
	fprintf(stderr, "warning: cannot read password\n");
	password = NULL;
  }

  OpenScreen();

  signal(SIGFPE, sigfpe_handler);
  signal(SIGSEGV, sigsegv_handler);

  clock_left = GetThemeInt("panel.clockleft", 0);
  m3x3 = GetResource("pdfviewer_menu", NULL);
  if (m3x3 == NULL) m3x3 = NewBitmap(128, 128);
  bmk_flag = GetResource("bmk_flag", NULL);

  for (i=0; i<9; i++) {
	sprintf(buf, "qmenu.pdfviewer.%i.text", i);
	strings3x3[i] = GetThemeString(buf, (char *)def_menutext[i]);
  }

  if (argc < 2) {
	Message(ICON_WARNING, "PDF Viewer", "@Cant_open_file", 2000);
	return 0;
  }

  GetKeyMapping(keyact0, keyact1);
  for (i=0; i<32; i++) {
	if (keyact0[i]!=NULL && strcmp(keyact0[i],"@KA_olnk")==0) keyact0[i]="@KA_zout";
  }
  
  FileName=argv[1];
  bi = GetBookInfo(FileName);
  if (bi->title) book_title = strdup(bi->title);

  // read config file
  globalParams = new GlobalParams();

  globalParams->setEnableFreeType("yes");
  globalParams->setAntialias((char *)(ivstate.antialiasing ? "yes" : "no"));
  globalParams->setVectorAntialias("no");

  filename = new GooString(FileName);
  doc = new PDFDoc(filename, NULL, NULL);
  if (!doc->isOk()) {
	delete doc;
	filename = new GooString(FileName);
	doc = new PDFDoc(filename, NULL, password);
	if (!doc->isOk()) {
		Message(ICON_WARNING, "PDF Viewer", "@Cant_open_file", 2000);
		return 0;
	}
  }
  npages = doc->getNumPages();
  paperColor[0] = 255;
  paperColor[1] = 255;
  paperColor[2] = 255;
  splashOut = new MySplashOutputDev(USE4 ? splashModeMono4 : splashModeMono8, 4, gFalse, paperColor);
  splashOut->startDoc(doc->getXRef());

  Outline * outline = doc->getOutline();
  if (outline && outline->getItems()) {

	GooList * items = outline->getItems();
	if (items->getLength() == 1) {
		OutlineItem *first = (OutlineItem *)items->get(0);
		first->open();
		items = first->getKids();
		update_toc(items, 0);
		first->close();
	} else if (items->getLength() > 1) {
		update_toc(items, 0);
	}

  }

  DataFile = GetAssociatedFile(FileName, 0);
  f = fopen(DataFile, "rb");
  if (f == NULL || fread(&docstate, 1, sizeof(tdocstate), f) != sizeof(tdocstate) || docstate.magic != 0x9751) {
		docstate.magic = 0x9751;
		docstate.page = 1;
		docstate.offx = 0;
		docstate.offy = 0;
		docstate.scale = 100;
		docstate.rscale = 150;
		docstate.orient = 0;
		docstate.nbmk = 0;
  }
  if (f != NULL) fclose(f);

  cpage = docstate.page;
  subpage = docstate.subpage;
  offx = docstate.offx;
  offy = docstate.offy;
  scale = docstate.scale & 0x7FFF;
  rscale = docstate.rscale;
  reflow_mode = (docstate.orient & 0x80) ? 1 : 0;
  orient = docstate.orient & 0x7f;

  calc_optimal_zoom = docstate.scale >> 15;

  if (argc >= 3) {
	if (argv[2][0] == '=') {
		cpage = atoi(argv[2]+1);
	} else {
		LinkDest *dest = doc->findDest(new GooString(argv[2]));
		if (dest && dest->isOk()) {
			Ref page_ref = dest->getPageRef();
			cpage = doc->findPage(page_ref.num, page_ref.gen);
		}
	}
  }

  if (cpage < 1) cpage = 1;
  if (cpage > npages) cpage = npages;

  InkViewMain(main_handler);
  return 0;

}


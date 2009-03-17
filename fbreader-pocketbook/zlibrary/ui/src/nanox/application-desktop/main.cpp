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

#include <ZLibrary.h>
#include <ZLApplication.h>
#include "../../../../../fbreader/src/fbreader/FBReader.h"
#include "../../../../../fbreader/src/fbreader/BookTextView.h"
#include "../../../../../fbreader/src/fbreader/FBReaderActions.h"
#include "../../../../../fbreader/src/formats/FormatPlugin.h"
#include "../view/ZLNXViewWidget.h"

#include <stdio.h>
#include <stdlib.h>
#include <inkview.h>
#include <inkinternal.h>

#include "main.h"

extern char *encoding_override;
extern int book_open_ok;

extern "C" const ibitmap searchbm;
extern "C" const ibitmap arrow_back;
extern "C" const ibitmap count_pages;

static char *strings3x3[9];

static char *spacing_variants[] = { "90", "100", "120", "150", "200", NULL };
static char *border_variants[] = { "@border_small", "@border_medium", "@border_large", NULL };
static char *encoding_variants[] = { "auto", "WINDOWS-1251", "KOI8-R", "KOI8-U", "IBM866", "ISO-8859-1", "ISO-8859-5", "UTF-8", NULL };

static iconfigedit fbreader_ce[] = {

	{ "@Font", "font", CFG_FONT, DEFREADERFONT, NULL },
	{ "@Encoding", "encoding", CFG_CHOICE, "auto", encoding_variants },
	{ "@Linespacing", "linespacing", CFG_CHOICE, "100", spacing_variants },
	{ "@Pagemargins", "border", CFG_INDEX, "1", border_variants },
	{ NULL, NULL, 0, NULL, NULL}

};

static const char *def_menutext[9] = {
	"@Goto_page", "@Exit", "@Search",
	"@Bookmarks", "@Menu", "@Rotate",
	"@Dictionary", "@Contents", "@Settings"
};

static const char *def_menuaction[9] = {
	"@KA_goto", "@KA_exit", "@KA_srch",
	"@KA_obmk", "@KA_none", "@KA_rtte",
	"@KA_dict", "@KA_cnts", "@KA_conf"
};


tdocstate docstate;

static char *FileName;
static char *OriginalName;
static char *DataFile;
static char *book_title="";
static char *book_author="";
static int ready_sent = 0;

static iconfig *cfg;

static long long pagelist[10000];
static int npages;
static long long cpos;

static int soft_update = 0;

static tocentry *TOC;
static int toc_size;

static int bmkpages[32];

static char kbdbuffer[32];
static int search_mode = 0;

static char *keyact0[32], *keyact1[32];

typedef struct hlink_s {
	short x;
	short y;
	short w;
	short h;
	int kind;
	char *href;
} hlink;

static int links_mode = 0;
static int link_back = -1;
static hlink links[MAXLINKS];
static int nlinks=0, clink;

static int calc_in_progress = 0;
static int calc_position_changed = 0;
static int current_position_changed = 0;
static int calc_current_page;
static long long calc_current_position;

extern ZLApplication *mainApplication;
BookTextView *bookview;

unsigned char *screenbuf;
int use_antialiasing;
static int orient;
int imgposx, imgposy, imgwidth, imgheight, scanline;
int lock_drawing=0;
int no_save_state=0;

static ibitmap *m3x3;
static ibitmap *bmk_flag;
static ibitmap *bgnd_p, *bgnd_l;
static irect textarea;

std::vector<std::string> xxx_notes;

struct xxx_link {
	int x1, y1, x2, y2;
	int kind;
	std::string id;
	bool next;
};

std::vector<struct xxx_link> xxx_page_links;

struct xxx_toc_entry {
	int paragraph;
	int level;
	std::string text;
};

std::vector<xxx_toc_entry> xxx_myTOC;

iv_wlist *xxx_wordlist=NULL;
int xxx_wlistlen=0, xxx_wlistsize=0;

int glb_page;

static void draw_searchpan() {
	DrawBitmap(ScreenWidth()-searchbm.width-10, ScreenHeight()-searchbm.height-35, &searchbm);
}

static long long get_position() {

	ZLTextWordCursor cr = bookview->startCursor();
	return pack_position(cr.paragraphCursor().index(), cr.wordNumber(), cr.charNumber());

}

static long long get_end_position() {

	ZLTextWordCursor cr = bookview->endCursor();
	if (cr.isNull()) {
		mainApplication->refreshWindow();
		cr = bookview->endCursor();
	}
	if (cr.isNull()) return get_position();
	return pack_position(cr.paragraphCursor().index(), cr.wordNumber(), cr.charNumber());

}

static void draw_bmk_flag(int update) {
	int i, x, y, bmkset=0;
	long long s, e;
	if (! bmk_flag) return;
	s = get_position();
	e = get_end_position();
	for (i=0; i<docstate.nbmk; i++) {
		if (docstate.bmk[i] >= s && docstate.bmk[i] < e) bmkset = 1;
	}
	if (! bmkset) return;
	x = ScreenWidth()-bmk_flag->width;
	y = 0;
	DrawBitmap(x, y, bmk_flag);
	if (update) PartialUpdate(x, y, bmk_flag->width, bmk_flag->height);
}

static int is_footnote_mode() {

	return (((FBReader *)mainApplication)->getMode() == FBReader::FOOTNOTE_MODE);

}

static void printpos(char *s, long long n) {

	int p, w, l, cpage;
	unpack_position(cpos, &p, &w, &l);
	cpage = position_to_page(cpos);
	fprintf(stderr, "%s %i  %lld %i %i %i\n", s, cpage, cpos, p, w, l);

}

static void repaint(int full) {

	char buf[32];
	int cpage, p, w, l;
	ibitmap *icon, *bgnd;

	ClearScreen();

	bgnd = (orient == 0 || orient == 3) ? bgnd_p : bgnd_l;
	if (bgnd) DrawBitmap(0, 0, bgnd);

	restore_current_position();
//	mainApplication->refreshWindow();
	Stretch(screenbuf, IMAGE_GRAY2, imgwidth, imgheight, scanline,
		imgposx, imgposy, imgwidth, imgheight, 0);

	icon = NULL;
	if (calc_in_progress) {
		icon = (ibitmap *) &count_pages;
		sprintf(buf, "...       ");
	} else if (! is_footnote_mode()) {
		cpage = position_to_page(get_position());
		sprintf(buf, "  %i / %i", cpage, npages);
	} else {
		buf[0] = 0;
	}

	if (links_mode) {
		icon = NULL;
		if (link_back >= 0) DrawBitmap(0, 0, &arrow_back);
		invert_current_link();
		sprintf(buf, GetLangText("@choose_link"));
	}
	if (search_mode) {
		draw_searchpan();
	}


	shared_ptr<ZLTextModel> model = bookview->model();
	int npar = model->paragraphsNumber();
	if (npar <= 0) npar = 1;
	unpack_position(get_position(), &p, &w, &l);
	DrawPanel(icon, buf, book_title, (p * 100) / npar);
	draw_bmk_flag(0);

	if (full==1) {
		FullUpdate();
	} else if (full==0) {
		SoftUpdate();
	}

}

static void prev_page() {
	restore_current_position();
	mainApplication->refreshWindow();
	long long tmppos=get_position();
	int p = position_to_page(tmppos);
	if (is_footnote_mode() || p >= npages || p == 0) {
		mainApplication->doAction(ActionCode::LARGE_SCROLL_BACKWARD);
		mainApplication->refreshWindow();
	} else {
		set_position(page_to_position(p-1));
		mainApplication->refreshWindow();
	}
	cpos = get_position();
	printpos("< ", cpos);
	calc_position_changed = 1;
	if (cpos == tmppos && is_footnote_mode()) {
		mainApplication->doAction(ActionCode::CANCEL);
		mainApplication->refreshWindow();
	}
	repaint(1); 
}

static void next_page() {
	restore_current_position();
	long long tmppos=get_position();
	int p = position_to_page(tmppos);
	if (is_footnote_mode() || p >= npages) {
		mainApplication->doAction(ActionCode::LARGE_SCROLL_FORWARD);
	} else {
		set_position(page_to_position(p+1));
		mainApplication->refreshWindow();
	}
	cpos = get_position();
	printpos("> ", cpos);
	calc_position_changed = 1;
	repaint(1); 
}

static void prev_section() {
	restore_current_position();
	if (is_footnote_mode()) {
		mainApplication->doAction(ActionCode::CANCEL);
	}
	mainApplication->doAction(ActionCode::GOTO_PREVIOUS_TOC_SECTION);
	cpos = get_position();
	calc_position_changed = 1;
	repaint(1); 
}

static void next_section() {
	restore_current_position();
	if (is_footnote_mode()) {
		mainApplication->doAction(ActionCode::CANCEL);
	}
	mainApplication->doAction(ActionCode::GOTO_NEXT_TOC_SECTION);
	cpos = get_position();
	calc_position_changed = 1;
	repaint(1); 
}

static void jump_pages(int n) {

	char buf[16];
	int x, y, w, h, cpage;

	if (calc_in_progress) {
		(n < 0) ? prev_page() : next_page();
	} else {
		cpage = position_to_page(cpos);
		cpage += n;
		if (cpage < 1) cpage = 1;
		if (cpage > npages) cpage = npages;
		cpos = page_to_position(cpage);
		set_position(cpos);
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

}

static void font_change(int d) {

	char buf[256], *p;
	int size;

	restore_current_position();

	char *xfont = ReadString(cfg, "font", DEFREADERFONT);
	strcpy(buf, xfont);
	p = strchr(buf, ',');
	if (p) {
		size = atoi(p+1);
	} else {
		p = buf+strlen(buf);
		size = 12;
	}

	size += d;
	if (size >= 36) size = 15;
	if (size < 15) size = 36;
	sprintf(p, ",%i", size);
	WriteString(cfg, "font", buf);
	apply_config(1);
	repaint(1);

}

static void apply_config(int recalc) {

	char buf[256], *p, *newenc;
	int size;

	char *xfont = ReadString(cfg, "font", DEFREADERFONT);
	strcpy(buf, xfont);
	p = strchr(buf, ',');
	if (p) *(p++) = 0;
	size = p ? atoi(p)/2 : 12;
	ifont *f = OpenFont(buf, size, 1);

	ZLStringOption &ffoption = ZLTextStyleCollection::instance().baseStyle().FontFamilyOption;
	ffoption.setValue(f->family);

	ZLIntegerRangeOption &fsoption = ZLTextStyleCollection::instance().baseStyle().FontSizeOption;
	fsoption.setValue(size);

	ZLBooleanOption &boldoption = ZLTextStyleCollection::instance().baseStyle().BoldOption;
	boldoption.setValue(f->isbold ? true : false);

	ZLBooleanOption &italicoption = ZLTextStyleCollection::instance().baseStyle().ItalicOption;
	italicoption.setValue(f->isitalic ? true : false);

	int linespacing = ReadInt(cfg, "linespacing", 100);
	ZLIntegerOption &lspaceoption = ZLTextStyleCollection::instance().baseStyle().LineSpacePercentOption;
	lspaceoption.setValue(linespacing);

	FBIndicatorStyle &indicatorInfo = bookview->commonIndicatorInfo();
	indicatorInfo.ShowOption.setValue(false);

	newenc = ReadString(cfg, "encoding", "auto");
	if (strcmp(newenc, docstate.encoding) != 0) {
		ShowHourglass();
		xxx_myTOC.clear();
		FBReader *fbr = (FBReader *)mainApplication;
		strncpy(docstate.encoding, newenc, 15);
		fbr->openFile(FileName);
	}

	orient = ReadInt(cfg, "orientation", 0);
	int border = ReadInt(cfg, "border", 1);
	SetOrientation(orient);
	ZLNXViewWidget *widget = (ZLNXViewWidget *) mainApplication->myViewWidget;

	if (orient == 0 || orient == 3) {
		imgposx = textarea.x;
		imgposy = textarea.y;
		imgwidth = textarea.w;
		imgheight = textarea.h;
	} else {
		imgposx = textarea.y;
		imgposy = textarea.x;
		imgwidth = textarea.h;
		imgheight = textarea.w;
	}
	if (imgposy+imgheight > ScreenHeight()-PanelHeight()) {
		imgheight = ScreenHeight()-PanelHeight()-imgposy;
	}

	if (border == 0) {
		imgposx += 4;
		imgposy += 10;
		imgwidth -= 8;
		imgheight -= 10;
	} else if (border == 1) {
		imgposx += 18;
		imgposy += 24;
		imgwidth -= 36;
		imgheight -= 36;
	} else {
		imgposx += 36;
		imgposy += 36;
		imgwidth -= 72;
		imgheight -= 62;
	}
	imgwidth = (imgwidth+3) & ~3;
	imgheight = (imgheight+3) & ~3;

	scanline = imgwidth/4;
	widget->setSize(imgwidth, imgheight);

	bookview->clearCaches();
	mainApplication->refreshWindow();	

	CloseFont(f);

	if (recalc) calc_pages();

}


static void rotate_handler(int n) {

	restore_current_position();
	int cn = ReadInt(cfg, "orientation", 0);
	WriteInt(cfg, "orientation", n);
	apply_config((cn+n == 3) ? 0 : 1);
	repaint(1);

}

static void save_state() {

  if (no_save_state) return;
  if (docstate.magic != 0x9751) return;
  docstate.position = cpos;
  strncpy(docstate.encoding, ReadString(cfg, "encoding", "auto"), 15);

  fprintf(stderr, "fbreader - save settings...\n");

  FILE *f = iv_fopen(DataFile, "wb");
  if (f != NULL) {
	iv_fwrite(&docstate, 1, sizeof(tdocstate), f);
	iv_fclose(f);
  }

  CloseConfig(cfg);

  fprintf(stderr, "fbreader - save settings done\n");

}

static void calc_timer() {

	int p, w, l;
        
	if (! calc_in_progress) {
//		mainApplication->refreshWindow();
		if (GetEventHandler() == main_handler) repaint(0);
		return;
	}

	if (is_footnote_mode()) {
		SetWeakTimer("CalcPages", calc_timer, 200);
		return;
	}

	lock_drawing = 1;
	current_position_changed = 1;

	if (calc_position_changed) {
		set_position(calc_current_position);
		calc_position_changed = 0;
	}

	mainApplication->doAction(ActionCode::LARGE_SCROLL_FORWARD);

	calc_current_position = get_position();
	if (calc_current_position == pagelist[calc_current_page-1]) {

		long long pos2 = calc_current_position;
                mainApplication->doAction(ActionCode::GOTO_NEXT_TOC_SECTION);
		calc_current_position = get_position();
		if (pos2 == calc_current_position) {

			pos2 = get_end_position();
			unpack_position(pos2, &p, &w, &l);
			set_position(pack_position(p+1, 0, 0));
			calc_current_position = get_position();
			if (calc_current_position <= pos2) {

				calc_in_progress = 0;
				set_position(cpos);
#ifndef EMULATOR
				SetHardTimer("CalcPages", calc_timer, 0);
#else
				SetHardTimer("CalcPages", calc_timer, 100);
#endif
				lock_drawing = 0;
				return;

			}

		}

	}

	npages = ++calc_current_page;
	pagelist[npages-1] = calc_current_position;

	//int p, w, l;
	//unpack_position(calc_current_position, &p, &w, &l);
	//fprintf(stderr, "%i  %i %i %i\n", npages, p, w, l);

	SetHardTimer("CalcPages", calc_timer, 0 /*200*/);
	lock_drawing = 0;

}		

static void calc_pages() {

	calc_in_progress = 1;
	calc_position_changed = 1;
	current_position_changed = 1;
	calc_current_page = npages = 1;
	calc_current_position = pack_position(0, 0, 0);

	SetHardTimer("CalcPages", calc_timer, 1);


}

static long long pack_position(int para, int word, int letter) {

	return ((long long)para << 40) | ((long long)word << 16) | ((long long)letter);

}

static void unpack_position(long long pos, int *para, int *word, int *letter) {

	*para = (pos >> 40) & 0xffffff;
	*word = (pos >> 16) & 0xffffff;
	*letter = pos & 0xffff;

}

static long long page_to_position(int page) {

	if (page < 1) page = 1;
	if (page > npages) page = npages;
	return pagelist[page-1];

}

static int position_to_page(long long position) {

	int i;

	for (i=1; i<=npages; i++) {
		if (position < pagelist[i]) return i;
//		if (position <= pagelist[i-1]) return i;
	}
	return npages;

}

static void set_position(long long pos) {

	int para, word, letter;

	unpack_position(pos, &para, &word, &letter);
	
	bookview->gotoPosition(para, word, letter);
	calc_position_changed = 1;

}

static void restore_current_position() {

	if (current_position_changed) {
		set_position(cpos);
		current_position_changed = 0;
		calc_position_changed = 1;
	}

}

static void wait_to_calc(int page) {

	int hgshown=0;
	while (calc_in_progress && calc_current_page < page) {
		if (! hgshown) ShowHourglass();
		hgshown=1;
		calc_timer();
	}
//	ClearTimer(calc_timer);


}

static void select_page(int page) {

	wait_to_calc(page);
	cpos = page_to_position(page);
	set_position(cpos);
	calc_position_changed = 1;
	mainApplication->refreshWindow();
	repaint(1);

}


static void bmk_paint(int page, long long pos) {

	set_position(pos);
	mainApplication->refreshWindow();
	repaint(2);
	set_position(cpos);

}	

static void bmk_selected(int page, long long pos) {

	cpos = pos;
	set_position(pos);
	calc_position_changed = 1;
	mainApplication->refreshWindow();
	repaint(1);

}

static void bmk_added(int page, long long pos) {

	char buf[256];

	if (page > 0 && page < 99999) {
		sprintf(buf, "%s %i", GetLangText("@Add_Bmk_P"), page);
	} else {
		sprintf(buf, "%s", GetLangText("@Add_Bmk_P"));
	}
	Message(ICON_INFORMATION, GetLangText("@Bookmarks"), buf, 1500);
	draw_bmk_flag(1);

}

static void bmk_handler(int action, int page, long long pos) {

	switch (action) {
		case BMK_PAINT: bmk_paint(page, pos); break;
		case BMK_ADDED: bmk_added(page, pos); break;
		case BMK_SELECTED: bmk_selected(page, pos); break;
		case BMK_CLOSED: repaint(0); break;
	}

}

static void open_bookmarks() {

	int i;

	for (i=0; i<docstate.nbmk; i++) {
		bmkpages[i] = calc_in_progress ? i+1 : position_to_page(docstate.bmk[i]);
	}
	int cpage = calc_in_progress ? -1 : position_to_page(cpos);
	OpenBookmarks(cpage, cpos, bmkpages, docstate.bmk, &docstate.nbmk, 30, bmk_handler);

}

static void new_bookmark() {

	int i;

	for (i=0; i<docstate.nbmk; i++) {
		bmkpages[i] = calc_in_progress ? i+1 : position_to_page(docstate.bmk[i]);
	}
	int cpage = calc_in_progress ? -1 : position_to_page(cpos);
	SwitchBookmark(cpage, cpos, bmkpages, docstate.bmk, &docstate.nbmk, 30, bmk_handler);
	repaint(0);

}

static void toc_selected(long long position) {

	cpos = position;
	set_position(cpos);
	calc_position_changed = 1;
	mainApplication->refreshWindow();
	repaint(1);

}

static void open_contents() {

	long long pos;
	char *p;
	int i;

	if (TOC) {
		for (i=0; i<toc_size; i++) free(TOC[i].text);
		free(TOC);
		TOC = NULL;
	}

	toc_size = xxx_myTOC.size();
	if (toc_size == 0) {
		Message(ICON_INFORMATION, "FBReader", "This book has no contents", 2000);
		return;
	}

	TOC = (tocentry *) malloc((toc_size+1) * sizeof(tocentry));
	for (i=0; i<toc_size; i++) {
		TOC[i].level = xxx_myTOC[i].level;
		if (xxx_myTOC[i].paragraph == -1) {
			TOC[i].position = -1;
			TOC[i].page = 0;
		} else {
			pos = pack_position(xxx_myTOC[i].paragraph, 0, 0);
			TOC[i].position = pos;
			TOC[i].page = calc_in_progress ? -1 : position_to_page(pos);
		}
		TOC[i].text = strdup((char *)(xxx_myTOC[i].text.c_str()));
		p = TOC[i].text;
		while (*p) {
			if (*p == '\r' || *p == '\n') *p = ' ';
			p++;
		}

		//int p, w, l;
		//unpack_position(pos, &p, &w, &l);
		//fprintf(stderr, "%i %i %i\n", p, w, l);
		//fprintf(stderr, "%llx %i %s\n", pos, TOC[i].page, TOC[i].text);
	}

	restore_current_position();
	pos = get_end_position();
	OpenContents(TOC, toc_size, pos, toc_selected);

}

static void open_notes_menu() {

	OpenNotesMenu(OriginalName, book_title, cpos);

}

static void configuration_updated() {

	SaveConfig(cfg);
	SetEventHandler(main_handler);
	apply_config(1);

}

static void invert_current_link() {

	hlink *cl = &(links[clink]);
	InvertAreaBW(cl->x, cl->y, cl->w, cl->h);

}

static void end_link_mode() {

	int i;
	links_mode = 0;
	for (i=0; i<nlinks; i++) {
		free(links[i].href);
		links[i].href = 0;
	}

}

static void jump_ref(char *ref) {

	if (is_footnote_mode()) {
		mainApplication->doAction(ActionCode::CANCEL);
	}
	if (ref[0] == '=') {
		cpos = strtoll(ref+1, NULL, 0);
	} else {
		((FBReader *)mainApplication)->tryShowFootnoteView(ref, false);
		cpos = get_position();
	}
	set_position(cpos);

}

static void open_int_ext_link(char *href) {

	char buf[1024], *p, *pp;

	fprintf(stderr, "%s\n", href);

	strcpy(buf, href);
	p = strchr(buf, '#');
	if (p != NULL) {
		*(p++) = 0;
		if (strcasecmp(buf, FileName) == 0) {
			jump_ref(p);
			mainApplication->refreshWindow();
			return;
		}
	}

	if (strncasecmp(href, "http:", 5) == 0 ||
	    strncasecmp(href, "ftp:", 4) == 0 ||
	    strncasecmp(href, "mailto:", 7) == 0
	) {
		Message(ICON_WARNING, "FBReader", "@Is_ext_link", 2000);
		return;
	} else if (href[0] == '/') {
		pp = strrchr(href, '/') + 1;
	} else {
		pp = href;
	}
	strcpy(buf, OriginalName);
	p = strrchr(buf, '/');
	if (p == NULL) p = buf; else p++;
	strcpy(p, pp);
	pp = strchr(p, '#');
	if (pp != NULL) {
		*(pp++) = 0;
	}
	OpenBook(buf, pp, 1);

}

static void process_link(int num) {

	hlink *cl = &(links[num]);
	FILE *f;

	restore_current_position();
	if (link_back >= 0 && num == 0) {
		iv_truncate(HISTORYFILE, link_back);
	} else {
		f = iv_fopen(HISTORYFILE, "a");
		if (f != NULL) {
			fprintf(f, "%s#=%lld\n", OriginalName, get_position());
			iv_fclose(f);
		}
	}

	if (cl->kind == 15) { // int. hyperlink

		if (is_footnote_mode()) {
				mainApplication->doAction(ActionCode::CANCEL);
		}
		((FBReader *)mainApplication)->tryShowFootnoteView(cl->href, false);

	} else if (cl->kind == 16) { // footnote

		((FBReader *)mainApplication)->tryShowFootnoteView(cl->href, false);

	} else if (cl->kind == 37) { // ext. hyperlink

		open_int_ext_link(cl->href);

	}

	end_link_mode();
	SetEventHandler(main_handler);

}


static int links_handler(int type, int par1, int par2) {

	hlink *cl = &(links[clink]);
	int y0a, y0b, y1a, y1b;

	switch (type) {

		case EVT_SHOW:
			repaint(0);
			break;

		case EVT_KEYPRESS:
		case EVT_KEYREPEAT:
			invert_current_link();
			y0a = cl->y;
			y1a = cl->y+cl->h+1;
			switch (par1) {

				case KEY_UP:
					clink--;
					if (type == EVT_KEYREPEAT) clink = 0;
					if (clink < 0) goto lk_exit;
					goto lk_update;
				case KEY_DOWN:
					clink++;
					if (type == EVT_KEYREPEAT) clink = nlinks-1;
					if (clink >= nlinks) clink = 0;
				lk_update:
					y0b = links[clink].y;
					y1b = links[clink].y+links[clink].h+1;
					if (y0b < y0a) y0a = y0b;
					if (y1b > y1a) y1a = y1b;
					invert_current_link();
					PartialUpdate(0, y0a, ScreenWidth(), y1a-y0a);
					break;

				case KEY_LEFT:
				case KEY_BACK:
				lk_exit:
					end_link_mode();
					soft_update = 1;
					SetEventHandler(main_handler);
					break;

				case KEY_OK:
					process_link(clink);
					break;

			}

	}
	return 0;

}

static char *get_backlink() {

	static char buf[1024], *p;
	FILE *f;
	int fp=0, cfp=0;

	link_back = -1;
	buf[0] = 0;
	f = iv_fopen(HISTORYFILE, "r");
	if (f != NULL) {
		while (1) {
			cfp = iv_ftell(f);
			if (iv_fgets(buf, sizeof(buf)-1, f) == NULL || (buf[0] & 0xe0) == 0) break;
			fp = cfp;
		}
		iv_fseek(f, fp, SEEK_SET);
		iv_fgets(buf, sizeof(buf)-1, f);
		iv_fclose(f);
		p = buf + strlen(buf);
		while (p > buf && (*(p-1) & 0xe0) == 0) *(--p) = 0;
	}

	if (buf[0] != 0) {
		link_back = fp;
		return buf;
	}
	return NULL;

}

static void back_link() {

	char *s = get_backlink();
	if (s != NULL) {
		nlinks = 1;
		links[0].kind = 37; // external link
		links[0].href = strdup(s);
		process_link(0);
	}

}

static void open_links() {

	int i;
	char *s;

	nlinks = 0;

	s = get_backlink();
	if (s != NULL) {
		links[nlinks].x = 2;
		links[nlinks].y = 2;
		links[nlinks].w = 30;
		links[nlinks].h = 14;
		links[nlinks].kind = 37; // external link
		links[nlinks].href = strdup(s);
		nlinks++;
	}		

	for (i=0; i<xxx_page_links.size(); i++) {
		xxx_link cl = xxx_page_links.at(i);
		links[nlinks].x = imgposx+cl.x1;
		links[nlinks].y = imgposy+cl.y1;
		links[nlinks].w = cl.x2-cl.x1+1;
		links[nlinks].h = cl.y2-cl.y1+1;
		links[nlinks].kind = cl.kind;
		links[nlinks].href = strdup(cl.id.c_str());
		nlinks++;
		if (nlinks >= MAXLINKS) break;
	}

	if (nlinks == 0) {
		Message(ICON_INFORMATION, "FBReader", "@No_links", 2000);
		return;
	}

	links_mode = 1;
	clink = 0;
	invert_current_link();
	SetEventHandler(links_handler);

}

static void open_dictionary() {

	OpenDictionaryView(xxx_wordlist, NULL);

}

static void adjust_position() {

/*
	int cpage = position_to_page(get_position());
	cpos = page_to_position(cpage);
*/

}

static int search_handler(int type, int par1, int par2) {

	long long xpos;

	if (type == EVT_SHOW) {
		//out_page(0);
	}

	if (type == EVT_KEYPRESS) {

		if (par1 == KEY_OK || par1 == KEY_BACK) {
			search_mode = 0;
			shared_ptr<ZLTextModel> model = bookview->model();
			model->removeAllMarks();
			SetEventHandler(main_handler);
		}
		if (par1 == KEY_LEFT) {
			restore_current_position();
			mainApplication->refreshWindow();
			xpos = cpos;
			ShowHourglass();
			bookview->findPrevious();
			cpos = get_position();
			if (cpos != xpos) {
				adjust_position();
				mainApplication->refreshWindow();
				repaint(1);
			} else {
				Message(ICON_INFORMATION, GetLangText("@Search"), GetLangText("@No_more_matches"), 2000);
			}
		}
		if (par1 == KEY_RIGHT) {
			restore_current_position();
			mainApplication->refreshWindow();
			xpos = cpos;
			ShowHourglass();
			bookview->findNext();
			cpos = get_position();
			if (cpos != xpos) {
				adjust_position();
				mainApplication->refreshWindow();
				repaint(1);
			} else {
				Message(ICON_INFORMATION, GetLangText("@Search"), GetLangText("@No_more_matches"), 2000);
			}
		}

	}

	return 0;

}

static void search_enter(char *text) {

	if (text == NULL || text[0] == 0) return;
	std::string s = text;

	restore_current_position();
	mainApplication->refreshWindow();
	SetEventHandler(search_handler);
	search_mode = 1;
	ShowHourglass();
	bookview->search(s, true, false, false, false);
	adjust_position();
	repaint(1);

}

static void open_quickmenu() { OpenMenu3x3(m3x3, (const char **)strings3x3, menu_handler); }
static void jump_pr10() { jump_pages(-10); }
static void jump_nx10() { jump_pages(+10); }
static void stop_jump() {
	if (! calc_in_progress) {
		mainApplication->refreshWindow();
		repaint(1);
	}
}
static void open_pageselector() { OpenPageSelector(select_page); }
static void first_page() { select_page(1); }
static void last_page() { select_page(999999); }
static void new_note() { CreateNote(OriginalName, book_title, cpos); }
static void save_page_note() { CreateNoteFromPage(OriginalName, book_title, cpos); }
static void open_notes() { OpenNotepad(NULL); }
static void start_search() { OpenKeyboard("@Search", kbdbuffer, 30, 0, search_enter); }
static void font_bigger() { font_change(+5); }
static void font_smaller() { font_change(-5); }
static void open_rotate() { OpenRotateBox(rotate_handler); }
static void main_menu() { OpenMainMenu(); }
static void exit_reader() { CloseApp(); }
static void open_mp3() { OpenPlayer(); }
static void mp3_pause() { TogglePlaying(); }
static void volume_up() { int r = GetVolume(); SetVolume(r+3); }
static void volume_down() { int r = GetVolume(); SetVolume(r-3); }
static void open_config() { OpenConfigEditor("@FB_config", cfg, fbreader_ce, configuration_updated, NULL); }

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
	{ "@KA_olnk", open_links, NULL, NULL },
	{ "@KA_blnk", back_link, NULL, NULL },
	{ "@KA_cnts", open_contents, NULL, NULL },
	{ "@KA_srch", start_search, NULL, NULL },
	{ "@KA_dict", open_dictionary, NULL, NULL },
	{ "@KA_zmin", font_bigger, NULL, NULL },
	{ "@KA_zout", font_smaller, NULL, NULL },
	{ "@KA_rtte", open_rotate, NULL, NULL },
	{ "@KA_mmnu", main_menu, NULL, NULL },
	{ "@KA_exit", exit_reader, NULL, NULL },
	{ "@KA_mp3o", open_mp3, NULL, NULL },
	{ "@KA_mp3p", mp3_pause, NULL, NULL },
	{ "@KA_volp", volume_up, NULL, NULL },
	{ "@KA_volm", volume_down, NULL, NULL },
	{ "@KA_conf", open_config, NULL, NULL },
	{ NULL, NULL, NULL, NULL }
};

static void menu_handler(int pos) {

	char buf[32], *act;
	int i;

	if (pos < 0) return;
	sprintf(buf, "qmenu.fbreader.%i.action", pos);
	act = GetThemeString(buf, (char *)def_menuaction[pos]);

	for (i=0; KA[i].action != NULL; i++) {
		if (strcmp(act, KA[i].action) != 0) continue;
		if (KA[i].f1 != NULL) (KA[i].f1)();
		if (KA[i].f3 != NULL) (KA[i].f3)();
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

static int key_handler(int type, int par1, int par2) {

	char *act0, *act1, *act=NULL;
	int i;

	if (par1 >= 0x20) {

		// aplhanumeric keys
		return 0;

	} else if (par1 == KEY_BACK) {

		CloseApp();
		return 0;

	} else if (par1 == KEY_POWER)  {

		return 0;

	} else {

		act0 = keyact0[par1];
		act1 = keyact1[par1];
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

// against compiler bugs
static void x_ready_sent(int n) { ready_sent = n; }

static int main_handler(int type, int par1, int par2) {

	if (type == EVT_EXIT) {
		fprintf(stderr,"EVT_EXIT\n");
		save_state();
	}

	if (type == EVT_SHOW) {
		repaint(soft_update ? 0 : 1);
		soft_update = 0;
		x_ready_sent(ready_sent);
		if (! ready_sent) BookReady(OriginalName);
		ready_sent = 1;
	}

	if (type == EVT_KEYPRESS || type == EVT_KEYREPEAT || type == EVT_KEYRELEASE) {

		return key_handler(type, par1, par2);

	}

	return 0;

}

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

//	ZLIntegerRangeOption &option = ZLTextStyleCollection::instance().baseStyle().FontSizeOption;

static int is_html(char *name) {

	char *p = strrchr(name, '.');
	if (! p) return 0;
	if (strcasecmp(p, ".html") == 0) return 1;
	if (strcasecmp(p, ".htm") == 0) return 1;
	if (strcasecmp(p, ".shtml") == 0) return 1;
	if (strcasecmp(p, ".asp") == 0) return 1;
	if (strcasecmp(p, ".php") == 0) return 1;
	if (strcasecmp(p, ".cgi") == 0) return 1;
	if (strcasecmp(p, ".jsp") == 0) return 1;
	return 0;

}

static int is_doc(char *name) {

	char *p = strrchr(name, '.');
	if (! p) return 0;
	if (strcasecmp(p, ".doc") == 0) return 1;
	return 0;

}

static void cleanup_temps() {

	system("rm -rf /tmp/fbreader.temp");

}

int main(int argc, char **argv) {

	FILE *f;
	char buf[1024];
	char *tbuf, *p, *pp;
	bookinfo *bi;
	int i, n;

	cleanup_temps();
	OpenScreen();

	m3x3 = GetResource("fbreader_menu", NULL);
	if (m3x3 == NULL) m3x3 = NewBitmap(128, 128);
	bmk_flag = GetResource("bmk_flag", NULL);

	bgnd_p = GetResource("book_background_p", NULL);
	bgnd_l = GetResource("book_background_l", NULL);
	GetThemeRect("book.textarea", &textarea, 0, 0, ScreenWidth(), ScreenHeight(), 0);

	for (i=0; i<9; i++) {
		sprintf(buf, "qmenu.fbreader.%i.text", i);
		strings3x3[i] = GetThemeString(buf, (char *)def_menutext[i]);
	}

	if (argc < 2) {
		Message(ICON_WARNING, "FBReader", "@Cant_open_file", 2000);
		return 0;
	}
  
	FileName=argv[1];
	bi = GetBookInfo(FileName);
	if (bi->title) book_title = strdup(bi->title);
	if (bi->author) book_author = strdup(bi->author);

	use_antialiasing = ivstate.antialiasing;

	cfg = OpenConfig(USERDATA"/config/fbreader.cfg", fbreader_ce);

	DataFile = GetAssociatedFile(FileName, 0);
	f = iv_fopen(DataFile, "rb");
	if (f == NULL || iv_fread(&docstate, 1, sizeof(tdocstate), f) != sizeof(tdocstate) || docstate.magic != 0x9751) {
		docstate.magic = 0x9751;
		docstate.position = pack_position(0, 0, 0);
		strcpy(docstate.encoding, "auto");
		docstate.nbmk = 0;
	}
	if (f != NULL) iv_fclose(f);

	signal(SIGSEGV, sigsegv_handler);

	OriginalName = FileName;
	if (is_doc(FileName)) {
		system("mkdir " CONVTMP);
		sprintf(buf, "/ebrmain/bin/antiword.app -x db \"%s\" >%s", FileName, CONVTMP "/index.html");
		fprintf(stderr, "%s\n", buf);
		system(buf);
		FileName = CONVTMP "/index.html";
	}

	if (strcmp(docstate.encoding, "auto") == 0 && is_html(FileName)) {
		f = iv_fopen(FileName, "rb");
		if (f != NULL) {
			tbuf = (char *) malloc(10001);
			n = iv_fread(tbuf, 1, 10000, f);
			tbuf[n] = 0;
			p = strstr(tbuf, "charset=");
			if (!p) p = strstr(tbuf, "CHARSET=");
			if (p) {
				p += 8;
				pp = strchr(p, '\"');
				if (pp != NULL && pp-p <= 12) {
					*pp = 0;
					for (pp=p; *pp; pp++) {
						if (*pp>='a' && *pp<='z') *pp -= 0x20;
					}
					strcpy(docstate.encoding, p);
				}
			}
			free(tbuf);
			iv_fclose(f);
		}
	}

	WriteString(cfg, "encoding", docstate.encoding);
	encoding_override = docstate.encoding;

	screenbuf = (unsigned char *)malloc(800*800/4);
	npages = 0;

	GetKeyMapping(keyact0, keyact1);
	for (i=0; i<32; i++) {
		if (keyact0[i]!=NULL && strcmp(keyact0[i],"@KA_mpdf")==0) keyact0[i]="@KA_none";
		if (keyact1[i]!=NULL && strcmp(keyact1[i],"@KA_mpdf")==0) keyact1[i]="@KA_none";
	}

	printf("opening %s...\n", argv[1]);

	int zargc = 1;
	char **zargv = NULL;
	if (!ZLibrary::init(zargc, zargv)) {
		fprintf(stderr, "Zlibrary::init() failed\n");
		Message(ICON_WARNING, "FBReader", "@Cant_open_file", 2000);
		return 0;
	}

	ZLibrary::run(new FBReader(FileName));
	if (! book_open_ok) {
		fprintf(stderr, "Zlibrary::run() failed\n");
		Message(ICON_WARNING, "FBReader", "@Cant_open_file", 2000);
		return 0;
	}
	bookview = &(((FBReader *)mainApplication)->bookTextView());

	apply_config(1);
	if (argc >= 3) {
		jump_ref(argv[2]);
	} else {
		set_position(cpos = docstate.position);
	}
	printpos(": ", cpos);
	mainApplication->refreshWindow();

	InkViewMain(main_handler);

	cleanup_temps();
	return 0;

}


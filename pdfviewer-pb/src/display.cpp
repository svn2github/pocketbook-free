#include "pdfviewer.h"

extern "C" const ibitmap zoombm;
extern "C" const ibitmap searchbm;
extern "C" const ibitmap hgicon;

static pid_t bgpid = 0;

void bg_monitor() {

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

void kill_bgpainter() {

	int status;

	if (bgpid != 0) {
		kill(bgpid, SIGKILL);
		usleep(100000);
		waitpid(-1, &status, WNOHANG);
		bgpid = 0;
	}

}

void getpagesize(int n, int *w, int *h, double *res, int *marginx, int *marginy) {

	int mx, my, realscale;
	int pw, ph;
	if (reflow_mode) {
		pw = (int)ceil(doc->getPageMediaWidth(n));
		ph = (int)ceil(doc->getPageMediaHeight(n));
	} else {
		pw = (int)ceil(doc->getPageCropWidth(n));
		ph = (int)ceil(doc->getPageCropHeight(n));
	}
//	int rt = (int)ceil(doc->getPageRotate(n));
	int sw = ScreenWidth();
	int ush = ScreenHeight() - panelh - 15;

//fprintf(stderr, "(%i,%i) (%i,%i) %i\n", pw, ph, cw, ch, rt);
	if (reflow_mode) {
		realscale = rscale;
	} else {
		realscale = (scale < 200) ? scale : (scale*19)/20;
	}
	*w = ((sw * realscale) / 100);
	*h = (((*w) * ph) / pw);
	*res = ((((double)realscale * 72.0) / 100.0) * sw) / pw;

	if (scale > 50 && scale < 200) {
		mx = (*w-sw)/2;
		if (mx < 0) mx = 0;
		if (*h < ush) {
			my = 0;
		} else {
			my = (*h-ush)/2;
			if (my > mx*2) my = mx;
			if (my < 0) my = 0;
		}
	} else {
		mx = my = 0;
	}

	n = *h/ush;
	if (n > 0 && *h-(n*ush) < *h/10) {
		my = (*h-(n*ush))/2+2;
	}

	*marginx = mx;
	*marginy = my;

}

void find_off(int step)
{
 int sw=ScreenWidth();
 int sh=ScreenHeight();
 int pw, ph, marginx, marginy;
 double res;

 getpagesize(cpage, &pw, &ph, &res, &marginx, &marginy);

 step *= (sh-panelh-15);

 //fprintf(stderr, "step=%i offx=%i offy=%i mx=%i ,my=%i pw=%i ph=%i\n", step,offx,offy,marginx,marginy,pw,ph);

 if (step < 0) {
	if (offy <= 1) {
		if (scale < 200 || offx <= marginx) {
			if (cpage == 1) return;
			cpage--;
			offx = pw-sw;
		} else {
			offx -= (sw*19)/20;
		}
		offy = ph;
	} else {
		offy += step;
	}
 } else {
	if (offy+(sh-panelh) >= ph-1) {
		if (scale < 200 || offx >= pw-sw-marginx) {
			if (cpage >= npages) return;
			cpage++;
			offx = 0;
		} else {
			offx += (sw*19)/20;
		}
		offy = 1;
	} else {
		offy += step;
	}
  }

}

void find_off_x(int step)
{
 int sw=ScreenWidth();
 int sh=ScreenHeight();
 int pw, ph, marginx, marginy;
 double res;

 getpagesize(cpage, &pw, &ph, &res, &marginx, &marginy);

 step *= (sw/3);

 //fprintf(stderr, "step=%i offx=%i offy=%i mx=%i ,my=%i pw=%i ph=%i\n", step,offx,offy,marginx,marginy,pw,ph);

 if (step < 0) {
	if (scale < 200 || offx <= marginx) {
		if (cpage == 1) return;
		cpage--;
		offx = pw-sw;
	} else {
		offx += step;
	}
 } else {
	if (scale < 200 || offx >= pw-sw-marginx) {
		if (cpage >= npages) return;
		cpage++;
		offx = 0;
	} else {
		offx += step;
	}
  }

}

static int center_image(int w, int h, int sw, int *rw) {

	unsigned char *data, *p, mask;
	int row, x1, x2, y, pxinbyte;
	int xleft, xright, ytop, ybottom;

	data = (unsigned char *)(splashOut->getBitmap()->getDataPtr());
	row = splashOut->getBitmap()->getRowSize();

	//Stretch(data, USE4 ? IMAGE_GRAY4 : IMAGE_GRAY8, sw, sh-panelh, row, scrx, scry, sw, sh-panelh, 0);

	if (USE4) {
		mask = 0x77;
		pxinbyte = 2;
	} else {
		mask = 0x7f;
		pxinbyte = 1;
	}

	xleft = w/4;
	xright = w - w/4;
	ytop = h/8;
	ybottom = h - h/8;

	for (x1=0; x1<xleft; x1+=pxinbyte) {
		p = data + x1/pxinbyte + row * ytop;
		for (y=ytop; y<ybottom; y++) {
			if ((*p | mask) != 0xff) break;
			p += row;
		}
		if (y != ybottom) break;
	}
	for (x2=w-pxinbyte*2; x2>xright; x2-=pxinbyte) {
		p = data + x2/pxinbyte + row * ytop;
		for (y=ytop; y<ybottom; y++) {
			if ((*p | mask) != 0xff) break;
			p += row;
		}
		if (y != ybottom) break;
	}
	//fprintf(stderr, "w=%i h=%i (%i,%i)=%i\n", w, h, x2, x1, x1+(x2-x1)/2);
	if (rw) *rw = x2-x1;
	return (x1 + x2) / 2 - sw / 2;

}

int get_fit_scale() {

	int sw, sh, pw, ph, marginx, marginy, rw;
	double res;

	scale = 100;
	sw = ScreenWidth();
	sh = ScreenHeight();
	getpagesize(cpage, &pw, &ph, &res, &marginx, &marginy);
	splashOut->setup(gFalse, 0, 0, 0, sw, sh-panelh, 0, 0, 0, 0, res);
	doc->displayPageSlice(splashOut, cpage, res, res, 0, gTrue, gFalse, gFalse, 0, 0, pw, ph);
	center_image(pw, ph, sw, &rw);
	return (sw * 95) / rw;

}

static void draw_page_image() {

	int sw, sh, pw, ph, x, y, w, h, dx, row, i;
	double tx, ty, tw, th, cw, mw;
	int marginx, marginy;
	double res;
	unsigned char *data;

	SetOrientation(orient);
	sw = ScreenWidth();
	sh = ScreenHeight();
	getpagesize(cpage, &pw, &ph, &res, &marginx, &marginy);

	scrx = scry = dx = 0;

	if (pw-sw-marginx < offx) offx = pw-sw-marginx;
	if (offx < marginx) offx = marginx;
	//if (offy < marginy) offy = marginy;
	//if (ph-(sh-panelh-15)-marginy < offy) offy = ph-(sh-panelh-15)-marginy;
	if (ph-(sh-panelh) < offy) offy = ph-(sh-panelh);
	if (offy < 0) offy = 1;
	if (offy == 0) offy = marginy;

	/*
	  void displayPageSlice(OutputDev *out, int page,
			double hDPI, double vDPI, int rotate, 
			GBool useMediaBox, GBool crop, GBool printing,
			int sliceX, int sliceY, int sliceW, int sliceH,
			GBool (*abortCheckCbk)(void *data) = NULL,
			void *abortCheckCbkData = NULL,
                        GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data) = NULL,
                        void *annotDisplayDecideCbkData = NULL);
        */

	if (! reflow_mode) {

		if (scale > 50 && scale <= 99) {
			scrx = (sw-pw)/2;
			offx = 0;
		}
		splashOut->setup(gFalse, 0, 0, 0, sw, sh-panelh, 0, 0, 0, 0, res);
		if (scale > 100 && scale <= 199) {
			doc->displayPageSlice(splashOut, cpage, res, res, 0, gFalse, gTrue/*gFalse*/, gFalse, 0, offy, pw, sh);
			dx = center_image(pw, sh-panelh, sw, NULL);
			offx = dx;
			if (dx < 0) dx = 0;
		} else {
			doc->displayPageSlice(splashOut, cpage, res, res, 0, gFalse, gTrue/*gFalse*/, gFalse, offx, offy, sw, sh);
		}
		nsubpages = 1;

	} else {

		w = sw - sw/17;
		h = sh - panelh;
		x = w/34;
		y = 0;
		tw = (doc->getPageMediaWidth(cpage) / pw) * w;
		tx = tw / 34.0;
		th = ((h * 72.0) / res);
		ty = doc->getPageMediaHeight(cpage);
		//fprintf(stderr, "pw=%i ph=%i cw=%i mw=%i tx=%i ty=%i tw=%i th=%i\n", (int)pw, (int)ph, (int)cw, (int)mw, (int)tx, (int)ty, (int)tw, (int)th);

		if (flowpage != cpage || flowscale != rscale || flowwidth != w || flowheight != h) {
			splashOut->setup(gTrue, -1, x, y, w, h, tx, ty, tw, th, res);
			doc->displayPageSlice(splashOut, cpage, res, res, 0, gTrue, gFalse, gFalse, 0, 0, sw, sh);
			flowpage = cpage;
			flowscale = rscale;
			flowwidth = w;
			flowheight = h;
		}
		nsubpages = splashOut->subpageCount();
		if (subpage >= nsubpages) subpage = nsubpages - 1;
		splashOut->setup(gTrue, subpage, x, y, w, h, tx, ty, tw, th, res);
		doc->displayPageSlice(splashOut, cpage, res, res, 0, gTrue, gFalse, gFalse, 0, 0, sw, sh);

	}

	data = (unsigned char *)(splashOut->getBitmap()->getDataPtr());
	row = splashOut->getBitmap()->getRowSize();

        printf("dx=%d\n",dx);

	Stretch(data+(USE4?dx/2:dx), USE4 ? IMAGE_GRAY4 : IMAGE_GRAY8, sw, sh-panelh, row, scrx, scry, sw, sh-panelh, 0);
	//DitherArea(scrx, scry, w, h, 4, DITHER_PATTERN);

	//scrx -= dx;

	if (search_mode) {
		for (i=0; i<nresults; i++) {
			InvertArea(results[i].x-offx, results[i].y-offy, results[i].w, results[i].h);
		}
	}

	thiw = (sw * 100) / pw; if (thiw > 100) thiw = 100;
	thih = (sh * 100) / ph; if (thih > 100) thih = 100;
	thix = (offx * 100) / pw; if (thix < 0) thix = 0; if (thix+thiw>100) thix = 100-thiw;
	thiy = (offy * 100) / ph; if (thiy < 0) thiy = 0; if (thiy+thih>100) thiy = 100-thih;

}

static int draw_pages() {

	int sw, sh, pw, ph, nx, ny, boxx, boxy, boxw, boxh, xx, yy, n, row, marginx, marginy;
	double res;
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

		for (yy=0; yy<ny; yy++) {
			for (xx=0; xx<nx; xx++) {
				n = cpage+yy*nx+xx;
				if (n > npages) break;
				boxx = xx*boxw;
				boxy = yy*boxh;
				getpagesize(n, &pw, &ph, &res, &marginx, &marginy);
				splashOut->setup(gFalse, 0, 0, 0, boxw-10, boxh-10, 0, 0, 0, 0, res);
				doc->displayPageSlice(splashOut, n, res, res, 0, gTrue, gFalse, gFalse,
					5, (scale==33 && !is_portrait()) ? 15 : 5, boxw-10, boxh-10);
				data = (unsigned char *)(splashOut->getBitmap()->getDataPtr());
				row = splashOut->getBitmap()->getRowSize();
				Stretch(data, USE4 ? IMAGE_GRAY4 : IMAGE_GRAY8, boxw-10, boxh-10, row,
					boxx+5, boxy+5, boxw-10, boxh-10, 0);
				//DitherArea(boxx+5, boxy+5, boxw-10, boxh-10, 2, DITHER_DIFFUSION);
				PartialUpdate(boxx+5, boxy+5, boxw, boxh);
			}
		}

#ifndef EMULATOR
		exit(0);

	} else {

		bgpid = pid;

	}

#endif

	return cpage+nx*ny > npages ? npages+1-cpage : nx*ny;


}

static void draw_zoomer() {
	DrawBitmap(ScreenWidth()-zoombm.width-10, ScreenHeight()-zoombm.height-35, &zoombm);
}

static void draw_searchpan() {
	DrawBitmap(ScreenWidth()-searchbm.width-10, ScreenHeight()-searchbm.height-35, &searchbm);
}


void draw_bmk_flag(int update) {
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

void out_page(int full) {

	char buf[32];
	int n=1, h;

	if (reflow_mode || scale>50) {
		FillArea(thx, thy, thw, thh, WHITE);
		DrawBitmap(thx+(thw-hgicon.width)/2, thy+(thh-hgicon.height)/2, &hgicon);
		PartialUpdate(thx, thy, thw, thh);
		ClearScreen();
		draw_page_image();
	} else {
		ClearScreen();
		n = draw_pages();
		SetHardTimer("BGPAINT", bg_monitor, 500);
	}

	if (zoom_mode) {
		draw_zoomer();
	}
	if (search_mode) {
		draw_searchpan();
	}
	if (n == 1) {
		if (clock_left) {
			sprintf(buf, "    %i%%    %i / %i", reflow_mode ? rscale : scale, cpage, npages);
		} else {
			sprintf(buf, "    %i / %i    %i%%", cpage, npages, reflow_mode ? rscale : scale);
		}
	} else {
		if (clock_left) {
			sprintf(buf, "    %i%%    %i-%i / %i", scale, cpage, cpage+n-1, npages);
		} else {
			sprintf(buf, "    %i-%i / %i    %i%%", cpage, cpage+n-1, npages, scale);
		}
	}
	h = DrawPanel(NULL, buf, book_title, (cpage * 100) / npages);

	thh = h-6;
	thw = (thh*6)/8;
	thx = clock_left ? ScreenWidth()-(thw+4) : 4;
	thy = ScreenHeight()-h+4;
	FillArea(thx, thy, thw, thh, WHITE);
	thx+=1; thy+=1; thw-=2; thh-=2;
	if ((scale >= 200 || (scale >= 100 && !is_portrait())) && !reflow_mode) {
		FillArea(thx+(thw*thix)/100, thy+(thh*thiy)/100, (thw*thiw)/100, (thh*thih)/100, BLACK);
	}

	draw_bmk_flag(0);
	if (full) {
		FullUpdate();
	} else {
		PartialUpdate(0, 0, ScreenWidth(), ScreenHeight());
	}
}


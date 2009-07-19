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

#ifndef DJVIEWER_H
#define DJVIEWER_H

#define MAXNOTEPADS 15

typedef struct tdocstate_s {

	int magic;
	int page;
	int offx;
	int offy;
	int scale;
	int orient;
	int reserved2;
	int nbmk;
	int bmk[30];

} tdocstate;

static void draw_page_image();
static int main_handler(int type, int par1, int par2);
static void menu_handler(int pos);
static void save_settings();





#endif


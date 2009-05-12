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

#ifndef FB_MAIN_H
#define FB_MAIN_H

#define MAXNOTEPADS 15

#define MAXLINKS 100
#define DEFREADERFONT "default,25"

#define CONVTMP "/tmp/fbreader.temp"

typedef struct tdocstate_s {

	int magic;
	long long position;
	char preformatted;
	char  reserved11;
	short reserved2;
	short reserved3;
	short reserved4;
	char encoding[16];
	int nbmk;
	long long bmk[30];

} tdocstate;

static long long pack_position(int para, int word, int letter);
static void unpack_position(long long pos, int *para, int *word, int *letter);
static long long page_to_position(int page);
static int position_to_page(long long position);
static long long get_position();
static long long get_end_position();
static void set_position(long long pos);
static void restore_current_position();
static void select_page(int page);
static void invert_current_link();
static void menu_handler(int pos);
static void calc_pages();
static void apply_config(int recalc, int canrestart);
static void save_state();

static int main_handler(int type, int par1, int par2);


#endif

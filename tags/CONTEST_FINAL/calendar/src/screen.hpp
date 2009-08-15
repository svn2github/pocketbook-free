/*
 * Screen.h
 *
 *  Created on: Jun 9, 2009
 *      Author: fax
 */

#ifndef SCREEN_H_
#define SCREEN_H_

#include <string>

#include "rect.hpp"
#include "font.hpp"
#include "bitmap.hpp"

// DONE: depending on rotation, screen width and height may differ

class Font;
class Bitmap;

class Screen {
	int background;

	//! Update coordinages
	Rect update_view;
public:
	Screen();

	unsigned width() const {
		return ScreenWidth();
	}
	unsigned height() const {
		return ScreenHeight();
	}

	/**
	 * Redraws the screen
	 *
	 * @param fullscreen when true, cause fullscreen redraw
	 */
	void update(bool fullscreen = false);
	void partial_update(const Rect& r);
	void full_update();

	virtual ~Screen();

	void set_color(int color);
	void set_background(int color);

	/* DRAWING FUNCTIONS */
	void clear();
	void clear_rect(const Rect& r);
	void fill_rect(const Rect& r, int color);
	void draw_rect(const Rect& r, int color);
	void draw_hline(int y, int xs, int xe, int color);
	void draw_vline(int x, int ys, int ye, int color);
	void invert_area(const Rect& r);

	/* TEXT DRAWING FUNCTIONS */
	void draw_string(int x, int y, const std::string& s);
	void draw_string_r(int x, int y, const std::string& s);

	/**
	 * @return Apparantly, part of the string that did not fit to rectangle
	 */
	std::string draw_text_rect(const Rect& r, const std::string& s, int flags);
	std::string draw_shaded_rect(const Rect& r, const std::string& s,
			int flags, int fg, int bg, const Font& font);

	void draw_bitmap(int x, int y, const Bitmap& b);
	int draw_symbol(int x, int y, int symbol);

	int string_width(const std::string& s);
	short char_width(char c);
	int rect_height(int width, const std::string& s, int flags);

	void rotate();

	Font open_font(const std::string& name, int size, bool aa);
	void close_font(const Font& f);
	/**
	 * @return old selected font
	 */
	Font set_font(const Font& f, int color);

	/*	int TextRectHeight(int width, char *s, int flags);
	 char *DrawTextRect(int x, int y, int w, int h, char *s, int flags);
	 char *DrawTextRect2(irect *rect, char *s);
	 int CharWidth(unsigned  short c);
	 int StringWidth(char *s);
	 int DrawSymbol(int x, int y, int symbol);
	 void RegisterFontList(ifont **fontlist, int count);
	 */

	static Screen& get_default();

private:
	static Screen defscreen;

	Font cur_font;

	int fg, bg;

private:
	Rect update_area;

};

#endif /* SCREEN_H_ */

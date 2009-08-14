/*
 * Screen.cpp
 *
 *  Created on: Jun 9, 2009
 *      Author: fax
 */

#include <boost/shared_ptr.hpp>

#include <inkview.h>
#include "screen.hpp"

#include <string>
#include <iostream>

using namespace std;

#define dbg cout
Screen Screen::defscreen;

Screen::Screen() :
	background(WHITE), cur_font(0) {

}

Screen::~Screen() {
}

void Screen::clear() {
	//FillArea(0,0, width(), height(), background);
	ClearScreen();
}

void Screen::draw_string(int x, int y, const std::string & s) {
	DrawString(x, y, s.c_str());
}

void Screen::draw_string_r(int x, int y, const std::string& s) {
	DrawStringR(x, y, s.c_str());
}

void Screen::clear_rect(const Rect& r) {
	fill_rect(r, background);
	update_area.union_with(r);
}

void Screen::fill_rect(const Rect& r, int color) {
	FillArea(r.x, r.y, r.w, r.h, color);
	update_area.union_with(r);
}

void Screen::draw_rect(const Rect& r, int color) {
	DrawRect(r.x, r.y, r.w, r.h, color);
	update_area.union_with(r);
}

void Screen::draw_hline(int y, int xs, int xe, int color) {
	DrawLine(xs, y, xe, y, color);
}

void Screen::draw_vline(int x, int ys, int ye, int color) {
	DrawLine(x, ys, x, ye, color);
}

std::string Screen::draw_text_rect(const Rect& r, const std::string& s,
		int flags) {
	update_area.union_with(r);
	return std::string(DrawTextRect(r.x, r.y, r.w, r.h,
			const_cast<char*> (s.c_str()), flags));
}

std::string Screen::draw_shaded_rect(const Rect& r, const std::string& s,
		int flags, int fg, int bg, const Font& font) {
	set_font(font, bg);

	draw_text_rect(r.shifted_by(-1, -1), s, flags);
	draw_text_rect(r.shifted_by(-1, 1), s, flags);
	draw_text_rect(r.shifted_by(1, -1), s, flags);
	draw_text_rect(r.shifted_by(1, 1), s, flags);

	set_font(font, fg);
	return draw_text_rect(r, s, flags);
}

int Screen::draw_symbol(int x, int y, int symbol) {
	return DrawSymbol(x, y, symbol);
}

void Screen::partial_update(const Rect& r) {
	PartialUpdate(r.x, r.y, r.w, r.h);
	dbg << "Updating (" << r.left() << ", " << r.top() << ", " << r.right()
			<< ", " << r.bottom() << ")" << endl;
	// TODO: think, if we need to reset update_area here
}

void Screen::full_update() {
	update_area.reset();
	FullUpdate();
}

void Screen::update(bool fullscreen) {
	if (fullscreen || update_area.is_empty()) {
		full_update();
		update_area.reset();
		return;
	}
	partial_update(update_area);
}

void Screen::draw_bitmap(int x, int y, const Bitmap& bmp) {

	update_area.union_with(Rect(x, y, bmp.width(), bmp.height()));
	DrawBitmap(x, y, bmp.bitmap());
}

Screen& Screen::get_default() {
	return defscreen;
}

Font Screen::set_font(const Font& f, int color) {
	Font old = cur_font;
	cur_font = f;
	SetFont(f.font(), color);
	return old;
}

Font Screen::open_font(const std::string& name, int size, bool aa) {
	ifont* fnt = OpenFont(const_cast<char*> (name.c_str()), size, aa);

	return Font(fnt, true);
}

int Screen::string_width(const std::string& s) {
	return StringWidth(const_cast<char*> (s.c_str()));
}

short Screen::char_width(char c) {
	return CharWidth(c);

}

int Screen::rect_height(int width, const std::string& s, int flags) {
	return TextRectHeight(width, const_cast<char*> (s.c_str()), flags);
}

void Screen::invert_area(const Rect& r) {
	InvertArea(r.x, r.y, r.w, r.h);
}

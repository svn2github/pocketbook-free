#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sstream>
#include <inkview.h>

#include "NumericTable.hh"

ifont *timesNN = NULL;
NumericTable table(5, 5);

static void repaint() {
	ClearScreen();

	int rows = table.getRows();
	int cols = table.getCols();

	int w = ScreenWidth();
	int h = ScreenHeight();
	int border = 5;
	int size = std::min(w, h) - border * 2;
	int table_x = border;
	int table_y = border;
	int cell_h = size / rows;
	int cell_w = size / cols;
	size = std::min(cell_w * cols, cell_h * rows);

	int y = table_y;
	for (int i = 0; i <= rows; ++i) {
		DrawLine(table_x, y, table_x + size, y, BLACK);
		y += cell_h;
	}
	int x = table_x;
	for (int i = 0; i <= cols; ++i) {
		DrawLine(x, table_y, x, table_y + size, BLACK);
		x += cell_w;
	}

	{
		SetFont(timesNN, BLACK);
		int y = table_y;
		for (int r = 0; r < rows; ++r) {
			int x = table_x;
			for (int c = 0; c < cols; ++c) {
				std::stringstream s;
				s << table.getNumber(r, c) << '\0';
				DrawTextRect(x, y, cell_w, cell_h,
						const_cast<char*>(s.str().c_str()),
						ALIGN_CENTER| VALIGN_MIDDLE);
				x += cell_w;
			}
			y += cell_h;
		}
	}
}

static int main_handler(int type, int par1, int par2) {
	if (type == EVT_INIT) {
		std::srand(std::time(NULL));
		table.regenerate();
		timesNN = OpenFont("times", 60, 1);
	}

	if (type == EVT_SHOW) {
		repaint();
		FullUpdate();
		FineUpdate();
	}

	if (type == EVT_KEYPRESS) {
		switch (par1) {
		case KEY_LEFT:
		case KEY_RIGHT:
		case KEY_UP:
		case KEY_DOWN:
		case KEY_OK:
		case KEY_NEXT:
		case KEY_PREV:
			table.regenerate();
			Repaint();
			break;
		case KEY_BACK:
			CloseApp();
			break;
		}
	}
	if (type == EVT_KEYREPEAT) {
		switch (par1) {
		case KEY_MENU:
		case KEY_HOME:
			CloseApp();
			break;
		}
	}

	if (type == EVT_EXIT) {
		if (timesNN != NULL) {
			CloseFont(timesNN);
			timesNN = NULL;
		}
	}
	return 0;
}

int main(int argc, char *argv[]) {
	InkViewMain(main_handler);
	return 0;
}

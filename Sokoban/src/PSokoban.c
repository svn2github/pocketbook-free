#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <dlfcn.h>
#include "inkview.h"

extern const ibitmap box, boxOnTheSpot, empty, player, playerOnTheSpot, spot,
		wall;

const char
		* level =
				"  ##### \n###   # \n# $ # ##\n# #    #\n#   .# #\n## #   #\n #@  ###\n #####  \n";

enum BoardElement {
	Box = 0x24,
	BoxOnTheSpot = 0x2a,
	Empty = 0x20,
	Player = 0x40,
	PlayerOnTheSpot = 0x2b,
	Spot = 0x2e,
	Wall = 0x23
};

const int tileSize = 26;
int playerX, playerY;
int boxCount = 0;
int boxesOnPlace = 0;

//This bunch of items creates something like incapsulated two-dimensional array - that's just because I've forget, and unable to find
//an example, how to manage dynamic two-dimensional array in C
const int boardHeight = 8;
const int boardWidth = 8;
char board[1000/* > boardHeight * boardWidth*/];
char getBoard(int x, int y) {
	return board[x + y * boardWidth];
}

void setBoard(int x, int y, char value) {
	board[x + y * boardWidth] = value;
}

//moves player to the position (+dx, +dy) if possible
//returns 1 if succeeded, 0 else
int movePlayer(int dx, int dy) {
	if (getBoard(playerX + dx, playerY + dy) == Empty || getBoard(playerX + dx,
			playerY + dy) == Spot) {
		if (getBoard(playerX + dx, playerY + dy) == Empty) {
			setBoard(playerX + dx, playerY + dy, Player);
		} else if (getBoard(playerX + dx, playerY + dy) == Spot) {
			setBoard(playerX + dx, playerY + dy, PlayerOnTheSpot);
		}
		if (getBoard(playerX, playerY) == Player) {
			setBoard(playerX, playerY, Empty);
		} else if (getBoard(playerX, playerY) == PlayerOnTheSpot) {
			setBoard(playerX, playerY, Spot);
		}
		return 1;
	}
	return 0;
}

//actually, if there is box and it is possible, moves box from position (+dx, +dy) to (+2dx, +2dy) relatively to the player position
//returns 1 if succeeded, 0 else
int moveBox(int dx, int dy) {
	if ((getBoard(playerX + dx, playerY + dy) == Box || getBoard(playerX + dx,
			playerY + dy) == BoxOnTheSpot) && (getBoard(playerX + 2 * dx,
			playerY + 2 * dy) == Empty || getBoard(playerX + 2 * dx, playerY
			+ 2 * dy) == Spot)) {
		if (getBoard(playerX + 2 * dx, playerY + 2 * dy) == Empty) {
			setBoard(playerX + 2 * dx, playerY + 2 * dy, Box);
		} else {// getBoard(playerX + 2 * dx, playerY + 2 * dy) == Spot
			setBoard(playerX + 2 * dx, playerY + 2 * dy, BoxOnTheSpot);
			boxesOnPlace++;
		}
		if (getBoard(playerX + dx, playerY + dy) == Box) {
			setBoard(playerX + dx, playerY + dy, Empty);
		} else {// getBoard(playerX + dx, playerY + dy) == BoxOnTheSpot
			setBoard(playerX + dx, playerY + dy, Spot);
			boxesOnPlace--;
		}
		return 1;
	}
	return 0;
}

void PrepareBoard() {
	int i, j;
	for (i = 0; i < boardWidth; i++) {
		for (j = 0; j < boardHeight; j++) {
			setBoard(i, j, level[i + j * (boardWidth + 1)]);
			if (getBoard(i, j) == Player || getBoard(i, j) == PlayerOnTheSpot) {
				playerX = i;
				playerY = j;
			}
			if (getBoard(i, j) == Box) {
				boxCount++;
			}
			if (getBoard(i, j) == BoxOnTheSpot) {
				boxCount++;
				boxesOnPlace++;
			}
		}
	}
}

void DrawCell(int i, int j) {
	switch (getBoard(i, j)) {
	case Box:
		DrawBitmap(i * tileSize, j * tileSize, &box);
		break;
	case BoxOnTheSpot:
		DrawBitmap(i * tileSize, j * tileSize, &boxOnTheSpot);
		break;
	case Empty:
		DrawBitmap(i * tileSize, j * tileSize, &empty);
		break;
	case Player:
		DrawBitmap(i * tileSize, j * tileSize, &player);
		break;
	case PlayerOnTheSpot:
		DrawBitmap(i * tileSize, j * tileSize, &playerOnTheSpot);
		break;
	case Spot:
		DrawBitmap(i * tileSize, j * tileSize, &spot);
		break;
	case Wall:
		DrawBitmap(i * tileSize, j * tileSize, &wall);
		break;
	}
}

void UpdateRegion(int x, int y, int dx, int dy) {
	int minx = dx > 0 ? x : x + dx;
	int width = 1 + (dx < 0 ? -dx : dx);
	int miny = dy > 0 ? y : y + dy;
	int height = 1 + (dy < 0 ? -dy : dy);
	PartialUpdate(minx * tileSize, miny * tileSize, width * tileSize, height
			* tileSize);
}

void DrawBoard() {
	int i, j;
	for (i = 0; i < boardWidth; i++) {
		for (j = 0; j < boardHeight; j++) {
			DrawCell(i, j);
		}
	}
	FullUpdate();
}

void Move(int dx, int dy) {
	//if there is box in front of player - it will be pushed one square further if possible.
	//Then, if there free space in front of player - it definitely will, if there was box and it was moved - he'll move himself
	//sum of this actions will inform us how much squares to update
	int shift = moveBox(dx, dy) + movePlayer(dx, dy);
	if (shift != 0) {
		DrawCell(playerX, playerY);
		DrawCell(playerX + dx, playerY + dy);
		if (shift == 2) {
			DrawCell(playerX + 2 * dx, playerY + 2 * dy);
		}
		//FullUpdate();
		UpdateRegion(playerX, playerY, shift * dx, shift * dy);
		playerX = playerX + dx;
		playerY = playerY + dy;
		//last value allows to distinguish, was box pushed on this stage.
		pushMove(dx, dy, shift - 1);
	}
}

void KeyPressed(int key) {
	switch (key) {
		case KEY_BACK:
		CloseApp();
		break;
		case KEY_UP:
		Move(0, -1);
		break;
		case KEY_DOWN:
		Move(0, 1);
		break;
		case KEY_LEFT:
		Move(-1, 0);
		break;
		case KEY_RIGHT:
		Move(1, 0);
		break;
	}
	if (boxesOnPlace == boxCount) {
		Message(ICON_INFORMATION, "Congratulations!", "You completed the level, press any key to continue", 10000);

	}
}

int main_handler(int type, int par1, int par2) {
	if (type == EVT_INIT) {
		PrepareBoard();
	} else if (type == EVT_SHOW) {
		DrawBoard();
	} else if (type == EVT_KEYPRESS) {
		KeyPressed(par1);
	}
	return 0;
}

int main(int argc, char **argv) {
	InkViewMain(main_handler);
	return 0;
}


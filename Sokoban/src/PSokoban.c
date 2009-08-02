//TODO: Store last played level
//TODO: Pick level packs from drive
//TODO: Add scrolling

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <dlfcn.h>
#include "inkview.h"

#define historyLimit 1000
#define boardMaxSize 15
#define rowMaxSize 12

extern const ibitmap box, boxOnTheSpot, empty, player, playerOnTheSpot, spot, wall;
ifont *font;

char *level =
				";MINICOSMOS 01\n\n  #####\n###   #\n# $ # ##\n# #  . #\n#    # #\n## #   #\n #@  ###\n #####\n\n;MINICOSMOS 02\n\n  #####\n###   #\n# $ # ##\n# #  . #\n#    # #\n##$#.  #\n #@  ###\n #####\n\n;MINICOSMOS 03\n\n  #####\n###   #\n# $ # ##\n# #  . #\n# .  # #\n##$#.$ #\n #@  ###\n #####\n\n;MINICOSMOS 04\n\n    ####\n#####  #\n#   $  #\n#  .#  #\n## ## ##\n#      #\n# @#   #\n#  #####\n####\n\n;MINICOSMOS 05\n\n    ####\n#####  #\n#   $  #\n# *.#  #\n## ## ##\n#      #\n# @#   #\n#  #####\n####\n\n;MINICOSMOS 06\n\n    ####\n#####  #\n#   *  #\n# *.#  #\n## ## ##\n# $    #\n# @#   #\n#  #####\n####\n\n;MINICOSMOS 07\n\n #####\n #   ##\n## #$ ##\n# $    #\n#. .#  #\n### @ ##\n  # # #\n  #   #\n  #####\n\n;MINICOSMOS 08\n\n #####\n #   ##\n##.#$ ##\n# $    #\n#. .#$ #\n### @ ##\n  # # #\n  #   #\n  #####\n\n;MINICOSMOS 09\n\n #####\n #   #\n##$# ###\n#   $@ #\n# #  # #\n# #. . #\n#   ####\n#####\n\n;MINICOSMOS 10\n\n #####\n #   #\n##$# ###\n#  .$@ #\n# #  # #\n# #..$ #\n#   ####\n#####\n\n;MINICOSMOS 11\n\n #####\n##   ###\n# . .  #\n# # ## ##\n#    $$@#\n### #   #\n  #   ###\n  #####\n\n;MINICOSMOS 12\n\n #####\n##   ###\n# . . .#\n# # ## ##\n#    $$@#\n### # $ #\n  #   ###\n  #####\n\n;MINICOSMOS 13\n\n    ####\n ####  #\n## $   #\n#  # #$#\n#.@.   ##\n## # #  #\n #      #\n #  #####\n ####\n\n;MINICOSMOS 14\n\n    ####\n ####  #\n## $   #\n#  # #$#\n#.@..  ##\n## # #  #\n #   $  #\n #  #####\n ####\n\n;MINICOSMOS 15\n\n   ####\n####  #\n# $   #\n#  .# ##\n## #.  #\n# @  $ #\n#   ####\n#####\n\n;MINICOSMOS 16\n\n   ####\n####  #\n# $ $ #\n#  .# ##\n## #.  #\n# @  $ #\n#.  ####\n#####\n\n;MINICOSMOS 17\n\n    #####\n   ##   ##\n  ## .#  #\n ##   @  #\n##    #  #\n#  $ #####\n# * ##\n#  ##\n####\n\n;MINICOSMOS 18\n\n    #####\n   ##   ##\n  ## .#  #\n ##   @  #\n##  * #  #\n#  $ #####\n# * ##\n#  ##\n####\n\n;MINICOSMOS 19\n\n####\n#  ####\n#     #\n#     #\n### ###\n# $$  ##\n# . .@ #\n####   #\n   #####\n\n;MINICOSMOS 20\n\n####\n#  ####\n#     #\n#     #\n### ###\n# $$$ ##\n# ...@ #\n####   #\n   #####\n\n;MINICOSMOS 21\n\n#####\n#   ###\n#     #\n##    #\n####$##\n#  $ ##\n# @   #\n###. .#\n  #####\n\n;MINICOSMOS 22\n\n#####\n#   ###\n# .   #\n## $  #\n####$##\n#  $ ##\n# @   #\n###. .#\n  #####\n\n;MINICOSMOS 23\n\n      ####\n#######  #\n#        #\n#  $ #.# #\n#  $## # ##\n###   @   #\n  ###  #  #\n    ##.  ##\n     #####\n\n;MINICOSMOS 24\n\n      ####\n#######  #\n#        #\n#  $ #.# #\n# $$## # ##\n### . @   #\n  ###  #  #\n    ##.  ##\n     #####\n\n;MINICOSMOS 25\n\n     ####\n   ###  #\n ### .. #\n # $$#  #\n## # #@##\n#       #\n#   #   #\n######  #\n     ####\n\n;MINICOSMOS 26\n\n     ####\n   ###  #\n ### .. #\n # $$#  #\n## # #@##\n#  *    #\n#   #   #\n######  #\n     ####\n\n;MINICOSMOS 27\n\n ####\n #  ####\n #     #\n # #.  #\n##*##$##\n#      #\n# # @  #\n#    ###\n######\n\n;MINICOSMOS 28\n\n ####\n #  ####\n #     #\n # #.  #\n##*##$##\n#    * #\n# # @  #\n#    ###\n######\n\n;MINICOSMOS 29\n\n########\n#   #  #\n#      #\n## #.  #\n#    ###\n# # . #\n# $$# #\n###  @#\n  #####\n\n;MINICOSMOS 30\n\n########\n#   #  #\n#      #\n## #. .#\n#    ###\n# # * #\n# $$# #\n###  @#\n  #####\n\n;MINICOSMOS 31\n\n#####\n#   ##\n# #  ##\n#. #$ #\n#  @  #\n#.##$##\n#    #\n######\n\n;MINICOSMOS 32\n\n####\n#  ###\n#    ##\n# .#$ #\n## @  #\n #.#$##\n #   #\n #####\n\n;MINICOSMOS 33\n\n #######\n #  #  ##\n## **$. #\n#   #   #\n#   @ ###\n#  ####\n####\n\n;MINICOSMOS 34\n\n #######\n##  #  #\n#  **$.##\n#   #   #\n### @   #\n  ####  #\n     ####\n\n;MINICOSMOS 35\n\n  ####\n###  ###\n#   *$ #\n# #  #@#\n# # *. #\n#   ####\n#####\n\n;MINICOSMOS 36\n\n#####\n#   ##\n# #  ###\n#   *$ #\n###  #@#\n  # *. #\n  #  ###\n  ####\n\n;MINICOSMOS 37\n\n  ####\n ##  #\n##   ##\n#  *$ #\n# # #@#\n#  *. #\n###  ##\n  #  #\n  ####\n\n;MINICOSMOS 38\n\n  ####\n  #  ###\n ## .  #\n##@$$$ #\n# . . ##\n#   ###\n#  ##\n####\n\n;MINICOSMOS 39\n\n  #####\n###   #\n#     #\n#  #.###\n##@$$$ #\n #.#.# #\n #     #\n #  ####\n ####\n\n;MINICOSMOS 40\n\n  ####\n ##  #\n## . ##\n#@$$$ #\n#. .# #\n# #   #\n#   ###\n#####\n";

int levelNo = 0;
int levels;

enum BoardElement {
	Box = '$',
	BoxOnTheSpot = '*',
	Empty = ' ',
	Player = '@',
	PlayerOnTheSpot = '+',
	Spot = '.',
	Wall = '#'
};

const int tileSize = 50;
int playerX, playerY;
int boxCount;
int boxesOnPlace;

///****This bunch of items creates something like incapsulated two-dimensional array - that's just because I've forget, and unable to find
//an example, how to manage dynamic two-dimensional array in C
int boardHeight;
int boardWidth;
int transpose;
char board[boardMaxSize * boardMaxSize/*max (currently) supported linear level size is size of screen*/];

int getCoordinateForXY(int x, int y) {
	return transpose ? y + x * boardHeight : x + y * boardWidth;
}

char getBoard(int x, int y) {
	if (x < 0 || x >= boardWidth || y < 0 || y >= boardHeight) {
		return Wall;
		//if someone is asking about the world around level - there are only bricks. This will help in case of open level
	}
	return board[getCoordinateForXY(x, y)];
}

void setBoard(int x, int y, char value) {
	if (x >= 0 && x < boardWidth && y >= 0 && y < boardHeight) {
		//we're writing data only while inbounds of board - also temporary helps when board is too big
		board[getCoordinateForXY(x, y)] = value;
	}
}

void boardReset() {
	//Set board size to (currently) max available - TODO: think what to do with this
	transpose = 0;
	boardWidth = boardMaxSize;
	boardHeight = boardMaxSize;
	int i, j;
	for (i = 0; i < boardMaxSize; i++) {
		for (j = 0; j < boardMaxSize; j++) {
			setBoard(i, j, Empty);
		}
	}
}

void boardCompress(int newWidth, int newHeight) {
	//old width was boardMaxSize - but in fact items are present only in first newWidth items of each row, and in first newHeight rows. so...
	int i, j;
	for (j = 0; j < newHeight; j++) {
		for (i = 0; i < newWidth; i++) {
			//order of cycles here has meaning - we're moving row by row, emptying space for next ones, not column by column - as they are stored in
			board[i + j * newWidth] = board[i + j * boardMaxSize];
		}
	}
	if (newWidth > rowMaxSize) {// && newHeight <= rowMaxSize) {
		transpose = 1;
		//prevention of too big level
		boardWidth = newHeight > rowMaxSize ? rowMaxSize : newHeight;
		boardHeight = newWidth;
	} else {
		transpose = 0;
		boardWidth = newWidth;
		boardHeight = newHeight;
	}
}
///****end of board

///****another pseudo-class - stack of moves, limited to 1000 moves, then starts to overwrite
int moves[3 * historyLimit];
int movesPointer = -1;
int undoTill = -1;
void resetMovesStack() {
	movesPointer = -1;
	undoTill = -1;
}

void pushMove(int dx, int dy, int pull) {
	movesPointer++;
	if (movesPointer >= historyLimit && movesPointer - historyLimit > undoTill) {
		undoTill = movesPointer - historyLimit;
	}
	moves[movesPointer % historyLimit * 3] = dx;
	moves[movesPointer % historyLimit * 3 + 1] = dy;
	moves[movesPointer % historyLimit * 3 + 2] = pull;
}

//if stack is not empty, returns 1
int popMove(int* dx, int* dy, int* pull) {
	if (movesPointer > undoTill) {
		*dx = moves[movesPointer % historyLimit * 3];
		*dy = moves[movesPointer % historyLimit * 3 + 1];
		*pull = moves[movesPointer % historyLimit * 3 + 2];
		movesPointer--;
		return 1;
	} else if (undoTill != -1) {
		Message(ICON_WARNING, "Sorry",
		"No more undo available", 10000);
	}
	return 0;
}
///****end of movement stack

///****nextLine This part of code will try to reproduce functionality of strtok(char*, "\r\n")
///again have no idea how to create new char* - so using global variable for response
int levelIndex = 0;
char currentLine[100];
int getNextLine() {
	if (level[levelIndex] == 0) {
		return 0;
	}
	int lineBegin = levelIndex;
	//seek for end
	while (level[levelIndex] != '\r' && level[levelIndex] != '\n'
			&& level[levelIndex] != 0) {
		levelIndex++;
	}
	int lineEnd = levelIndex;
	//this to remove trailing blanks
	while (level[lineEnd - 1] == Empty) {
		lineEnd--;
	}
	//seek for first character of next line
	while (level[levelIndex] == '\r' || level[levelIndex] == '\n') {
		levelIndex++;
	}
	int i;
	for (i = 0; i < lineEnd - lineBegin; i++) {
		currentLine[i] = level[lineBegin + i];
	}
	currentLine[lineEnd - lineBegin] = 0;
	return 1;
}

///****end of nextLine

int baseX;
int baseY;

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

//actually, if there is box and it is possible, moves box from position (x, y) to (x + dx, y + dy)
int moveBox(int x, int y, int dx, int dy) {
	if ((getBoard(x, y) == Box || getBoard(x, y) == BoxOnTheSpot) && (getBoard(
			x + dx, y + dy) == Empty || getBoard(x + dx, y + dy) == Spot)) {
		if (getBoard(x + dx, y + dy) == Empty) {
			setBoard(x + dx, y + dy, Box);
		} else {// getBoard(x + dx, y + dy) == Spot
			setBoard(x + dx, y + dy, BoxOnTheSpot);
			boxesOnPlace++;
		}
		if (getBoard(x, y) == Box) {
			setBoard(x, y, Empty);
		} else {// getBoard(x, y) == BoxOnTheSpot
			setBoard(x, y, Spot);
			boxesOnPlace--;
		}
		return 1;
	}
	return 0;
}

void DrawBoard();

void PrepareBoard() {
	boardReset();
	resetMovesStack();
	int currentLevel = -1;
	int previousLineHasData = 0;
	int currentLineIndex = 0;
	levelIndex = 0;
	int currentLevelWidth = 0;
	int currentLevelHeight = 0;
	int i;
	while (getNextLine()) {
		if (strlen(currentLine) != 0 && currentLine[0] != ';') {
			if (!previousLineHasData) {
				currentLevel++;
			}
			if (currentLevel == levelNo) {
				currentLevelHeight++;
				if (currentLevelWidth < strlen(currentLine)) {
					currentLevelWidth = strlen(currentLine);
				}
				for (i = 0; i < strlen(currentLine); i++) {
					setBoard(i, currentLineIndex, currentLine[i]);
					if (currentLine[i] == Player || currentLine[i]
							== PlayerOnTheSpot) {
						playerX = i;
						playerY = currentLineIndex;
					}
					if (currentLine[i] == Box) {
						boxCount++;
					}
					if (currentLine[i] == BoxOnTheSpot) {
						boxCount++;
						boxesOnPlace++;
					}
				}
				currentLineIndex++;
			}
			previousLineHasData = 1;
		} else {
			if (currentLevel == levelNo) {
				break;
			}
			previousLineHasData = 0;
		}
	}
	boardCompress(currentLevelWidth, currentLevelHeight);

	baseX = (rowMaxSize - boardWidth) * tileSize / 2;
	baseY = (boardMaxSize - boardHeight) * tileSize / 2;
}

void GetLevelCount() {
	levels = -1;
	int previousLineHasData = 0;
	levelIndex = 0;
	while (getNextLine()) {
		if (strlen(currentLine) != 0 && currentLine[0] != ';') {
			if (!previousLineHasData) {
				levels++;
			}
			previousLineHasData = 1;
		} else {
			previousLineHasData = 0;
		}
	}
}

void DrawCell(int i, int j) {
	//NOTE: failed usage of ibitmap *image = &name; and then DrawBitmap(,, image); - find out why?
	switch (getBoard(i, j)) {
	case Box:
		DrawBitmap(baseX + i * tileSize, baseY + j * tileSize, &box);
		break;
	case BoxOnTheSpot:
		DrawBitmap(baseX + i * tileSize, baseY + j * tileSize, &boxOnTheSpot);
		break;
	case Empty:
		DrawBitmap(baseX + i * tileSize, baseY + j * tileSize, &empty);
		break;
	case Player:
		DrawBitmap(baseX + i * tileSize, baseY + j * tileSize, &player);
		break;
	case PlayerOnTheSpot:
		DrawBitmap(baseX + i * tileSize, baseY + j * tileSize, &playerOnTheSpot);
		break;
	case Spot:
		DrawBitmap(baseX + i * tileSize, baseY + j * tileSize, &spot);
		break;
	case Wall:
		DrawBitmap(baseX + i * tileSize, baseY + j * tileSize, &wall);
		break;
	}
}

void UpdateRegion(int x, int y, int dx, int dy) {
	int minx = dx > 0 ? x : x + dx;
	int width = 1 + (dx < 0 ? -dx : dx);
	int miny = dy > 0 ? y : y + dy;
	int height = 1 + (dy < 0 ? -dy : dy);

	PartialUpdateBW(baseX + minx * tileSize, baseY + miny * tileSize, width * tileSize, height
			* tileSize);
}

void DrawBoard() {
	ClearScreen();
	int i, j;
	for (i = 0; i < boardWidth; i++) {
		for (j = 0; j < boardHeight; j++) {
			DrawCell(i, j);
		}
	}
	SetFont(font, WHITE);

	FillArea(0, boardMaxSize * tileSize, rowMaxSize * tileSize, tileSize, DGRAY);
	char buf[16];
	sprintf(buf, "Level %i/%i", levelNo + 1, levels + 1);
	DrawString(5, boardMaxSize * tileSize + 5, buf);
	DrawString(5, boardMaxSize * tileSize + 30, "Move: Up/Down/Left/Right. Undo: OK. Level navigation: +/-");
	FullUpdate();
}

void Move(int dx, int dy) {
	//if there is box in front of player - it will be pushed one square further if possible.
	//Then, if there free space in front of player - it definitely will, if there was box and it was moved - he'll move himself
	//sum of this actions will inform us how much squares to update
	int shift = moveBox(playerX + dx, playerY + dy, dx, dy) + movePlayer(dx, dy);
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

void Undo() {
	int dx, dy, pull;
	if (popMove(&dx, &dy, &pull)) {
		//This is reversed movement of player
		movePlayer(-dx, -dy);
		if (pull) {
			//and if there was push on previous movement - we're reverting it
			moveBox(playerX + dx, playerY + dy, -dx, -dy);
		}
		playerX = playerX - dx;
		playerY = playerY - dy;

		DrawCell(playerX, playerY);
		DrawCell(playerX + dx, playerY + dy);
		if (pull == 1) {
			DrawCell(playerX + 2 * dx, playerY + 2 * dy);
		}
		UpdateRegion(playerX, playerY, (pull + 1) * dx, (pull + 1) * dy);
	}
}

void KeyPressed(int key) {
	switch (key) {
	case KEY_BACK:
		CloseApp();
		return;
	case KEY_PLUS:
		if (levelNo < levels) {
			levelNo++;
			PrepareBoard();
			DrawBoard();
		}
		return;
	case KEY_MINUS:
		if (levelNo > 0) {
			levelNo--;
			PrepareBoard();
			DrawBoard();
		}
		return;
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
	case KEY_OK:
		Undo();
		break;
	}
	if (boxesOnPlace == boxCount) {
		if (levelNo < levels) {
			Message(ICON_INFORMATION, "Congratulations!",
					"You completed the level, press any key to continue", 10000);
			levelNo++;
			PrepareBoard();
			DrawBoard();
		}
		else {
			Message(ICON_INFORMATION, "Congratulations!",
					"You completed all the levels", 10000);
		}
	}
}

int main_handler(int type, int par1, int par2) {
	if (type == EVT_INIT) {
		font = OpenFont("LiberationSans", 16, 0);
		GetLevelCount();
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <dlfcn.h>
#include "inkview.h"

#define MAX_H_SIZE 15
#define MAX_W_SIZE 19
#define MIN_H_SIZE 10
#define MIN_W_SIZE 12
#define MODES_COUNT 4
#define MIN_BONUS_FOR_MIN_BOARD 50
#define MAX_BONUS_FOR_MIN_BOARD 1000
#define MIN_BONUS_FOR_MAX_BOARD 200
#define MAX_BONUS_FOR_MAX_BOARD 5000
#define BONUS_LEVEL 0.85
#define CELL_SIZE 40
#define CURSOR_DELTA 10
#define CURSOR_SIZE 10
#define ITEMS 5
#define EMPTY -1

int minX, maxX, minY, maxY;
extern const ibitmap item0, item1, item2, item3, item4;
ifont *font;

#ifndef EMULATOR
const char *configFileName = STATEPATH "/samegame.cfg";
#else
const char *configFileName = "samegame.cfg";
#endif

int board[MAX_H_SIZE][MAX_W_SIZE];
int hSize, wSize;
int mode = 3;
int baseX, baseY;
int cursorX, cursorY;
int briksErased;
int score;
int maxScores[MODES_COUNT];
int gameInProgress = 0;

static imenu levelSizesMenu[] = {
	{ ITEM_ACTIVE, 110, "19*15", NULL },
	{ ITEM_ACTIVE, 111, "16*14", NULL },
	{ ITEM_ACTIVE, 112, "14*12", NULL },
	{ ITEM_ACTIVE, 113, "12*10", NULL },
	{ 0, 0, NULL, NULL }
};

static int modesHeight[] = { 19, 16, 14, 12 };
static int modesWidth[] = { 15, 14, 12, 10 };

static imenu mainMenu[] = {
	{ ITEM_HEADER,   0, "Menu", NULL },
	{ ITEM_ACTIVE, 101, "Start New Game", NULL },
	{ ITEM_SUBMENU, 0, "Select Board Size", levelSizesMenu },
	{ ITEM_ACTIVE, 103, "Help", NULL },
	{ ITEM_ACTIVE, 104, "About", NULL },
	{ ITEM_ACTIVE, 105, "Exit", NULL },
	{ 0, 0, NULL, NULL }
};

void setMode()
{
    hSize = modesWidth[mode];
    wSize = modesHeight[mode];
    baseX = (ScreenWidth() - hSize * CELL_SIZE) / 2;
    baseY = (ScreenHeight() - wSize * CELL_SIZE - 40) / 2;
}

void prepareBoard()
{
	int i, j;
	for (i = 0; i < hSize; i++)
	{
		for (j = 0; j < wSize; j++)
		board[i][j] = rand() % ITEMS;
	}
	cursorX = cursorY = 0;
	score = 0;
}

void drawCell(int x, int y, int refresh);

void drawScores()
{
	SetFont(font, WHITE);

	FillArea(0, ScreenHeight() - 40, ScreenWidth(), 40, DGRAY);
	char buf[32];
	sprintf(buf, "Current Score: %i", score);
	DrawString(5, ScreenHeight() - 40, buf);
	sprintf(buf, "Highest Score: %i", maxScores[mode]);
	DrawString(5, ScreenHeight() - 20, buf);
	PartialUpdate(0, ScreenHeight() - 40, ScreenWidth(), 40);
}

void drawBoard()
{
	int i, j;
	ClearScreen();
	DrawRect(baseX - 2, baseY - 2, hSize * CELL_SIZE + 4, wSize * CELL_SIZE + 4, BLACK);
	for (i = 0; i < hSize; i++)
	{
		for (j = 0; j < wSize; j++)
		{
			drawCell(i, j, 0);
		}
	}
	FullUpdate();
	drawScores();
}

void updateCountedArea()
{
	int i, j;
	for (i = minX; i <= maxX; i++)
	{
		for (j = minY; j <= maxY; j++)
		{
			drawCell(i, j, 0);
		}
	}
	PartialUpdateBW(baseX + minX * CELL_SIZE, baseY + minY * CELL_SIZE, (maxX - minX + 1) * CELL_SIZE, (maxY - minY + 1) * CELL_SIZE);
}

void drawCell(int x, int y, int refresh)
{
	int xShift, yShift, item;
	FillArea(baseX + x * CELL_SIZE, baseY + y * CELL_SIZE, CELL_SIZE, CELL_SIZE, WHITE);

	item = board[x][y];
	xShift = baseX + x * CELL_SIZE;
	yShift = baseY + y * CELL_SIZE;

	if (item != EMPTY)
	{
		switch (item) {
		case 0:
			DrawBitmap(xShift, yShift, &item0);
			break;
		case 1:
			DrawBitmap(xShift, yShift, &item1);
			break;
		case 2:
			DrawBitmap(xShift, yShift, &item2);
			break;
		case 3:
			DrawBitmap(xShift, yShift, &item3);
			break;
		case 4:
			DrawBitmap(xShift, yShift, &item4);
			break;
		}
	}

	if (x == cursorX && y == cursorY)
	{
		FillArea(baseX + x * CELL_SIZE + CURSOR_DELTA, baseY + y * CELL_SIZE + CURSOR_DELTA, CURSOR_SIZE, CURSOR_SIZE, WHITE);
		DrawRect(baseX + x * CELL_SIZE + CURSOR_DELTA, baseY + y * CELL_SIZE + CURSOR_DELTA, CURSOR_SIZE, CURSOR_SIZE, BLACK);
	}
	if (refresh)
	{
		PartialUpdateBW(baseX + x * CELL_SIZE, baseY + y * CELL_SIZE, CELL_SIZE, CELL_SIZE);
	}
}

void saveSettings()
{
	int i = 0;
    FILE *config = fopen(configFileName, "w");
    if(config==NULL)return;
    fprintf(config, "%i\n", mode);
    for (i = 0; i < MODES_COUNT; i++)
	{
		fprintf(config, "%i\n", maxScores[i]);
	}
    fclose(config);
}

void updateMaxScore()
{
	if (maxScores[mode] < score)
	{
		maxScores[mode] = score;
	    saveSettings();
	}
}

void startNewGame()
{
    prepareBoard();
    drawBoard();
    gameInProgress = 0;
}

void gameOver()
{
	int i, j, bonus, count = 0;
	double clearanceCoefficient, boardCoefficeint, baseBonus, maxBonus;
	char buf[256];

	for (i = 0; i < hSize; i++)
	{
		for (j = 0; j < wSize; j++)
		{
			if (board[i][j] == EMPTY)
			{
				count++;
			}
		}
	}

	if (BONUS_LEVEL * wSize * hSize < count)
	{
		boardCoefficeint = ((double)hSize * wSize - MIN_H_SIZE * MIN_W_SIZE) / (MAX_H_SIZE * MAX_W_SIZE - MIN_H_SIZE * MIN_W_SIZE);
		clearanceCoefficient = ((double)count / (hSize * wSize) - BONUS_LEVEL) / (1 - BONUS_LEVEL);
		baseBonus = MIN_BONUS_FOR_MIN_BOARD + boardCoefficeint * (MIN_BONUS_FOR_MAX_BOARD - MIN_BONUS_FOR_MIN_BOARD);
		maxBonus = MAX_BONUS_FOR_MIN_BOARD + boardCoefficeint * (MAX_BONUS_FOR_MAX_BOARD - MAX_BONUS_FOR_MIN_BOARD);
		bonus = baseBonus + clearanceCoefficient * clearanceCoefficient * (maxBonus - baseBonus);
		score += bonus;
		sprintf(buf, "No more matches available! You scored %i, including %i for cleared %i%% of screen.", score, bonus, 100 * count / (hSize * wSize));
		updateMaxScore();
	}
	else
	{
		sprintf(buf, "No more matches available! You scored %i and cleared %i%% of screen.", score, 100 * count / (hSize * wSize));
	}

	Message(ICON_INFORMATION, "Game over!", buf, 10000);
    startNewGame();
}

int move_pointer(int cx, int cy)
{
	double ix=((double)(cx-baseX)/CELL_SIZE);
	double iy=((double)(cy-baseY)/CELL_SIZE);
	//int minX, minY;
	int oldX, oldY;
	if(ix<0 || ix>=hSize)return 0;
	if(iy<0 || iy>=wSize)return 0;
	oldX = cursorX;
	oldY = cursorY;
	cursorX = (int)ix;
	cursorY = (int)iy;
	drawCell(oldX, oldY, 1);
	drawCell(cursorX, cursorY, 1);
        return 1;
}

void move(int dx, int dy)
{
	int minX, minY;
	int oldX, oldY;
	if ((cursorX + dx < 0 || cursorX + dx >= hSize || cursorY + dy < 0 || cursorY + dy >= wSize))
	{
		oldX = cursorX;
		oldY = cursorY;
		cursorX = (hSize + cursorX + dx) % hSize;
		cursorY = (wSize + cursorY + dy) % wSize;
		drawCell(oldX, oldY, 1);
		drawCell(cursorX, cursorY, 1);
	}
	else
	{
		oldX = cursorX;
		oldY = cursorY;
		cursorX += dx;
		cursorY += dy;
		drawCell(oldX, oldY, 0);
		drawCell(cursorX, cursorY, 0);

		minX = oldX < cursorX ? oldX : cursorX;
		minY = oldY < cursorY ? oldY : cursorY;
		PartialUpdateBW(baseX + CELL_SIZE * minX, baseY + CELL_SIZE * minY, CELL_SIZE * (1 + abs(dx)), CELL_SIZE * (1 + abs(dy)));
	}
}

void resetCounters()
{
	minX = hSize;
	minY = wSize;
	maxX = maxY = -1;
}

void addCounter(int x, int y)
{
	if (minX > x)
	{
		minX = x;
	}
	if (minY > y)
	{
		minY = y;
	}
	if (maxX < x)
	{
		maxX = x;
	}
	if (maxY < y)
	{
		maxY = y;
	}
}

int hasSameNeighbours(int i, int j)
{
	if (board[i][j] != EMPTY &&
			((i > 0 && board[i - 1][j] == board[i][j]) ||
			(j > 0 && board[i][j - 1] == board[i][j]) ||
			(i < hSize - 1 && board[i + 1][j] == board[i][j]) ||
			(j < wSize - 1 && board[i][j + 1] == board[i][j])))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int areMatches()
{
	int i, j;
	for(i = 0; i < hSize; i++)
	{
		for (j = 0; j < wSize; j++)
		{
			if (hasSameNeighbours(i, j))
			{
				return 1;
			}
		}
	}
	return 0;
}

int erase(int x, int y)
{
	int briksErased = 1;
	int item = board[x][y];
	board[x][y] = EMPTY;
	addCounter(x, y);
	if (x > 0 && board[x - 1][y] == item)
	{
		briksErased += erase(x - 1, y);
	}
	if (y > 0 && board[x][y - 1] == item)
	{
		briksErased += erase(x, y - 1);
	}
	if (x < hSize - 1 && board[x + 1][y] == item)
	{
		briksErased += erase(x + 1, y);
	}
	if (y < wSize - 1 && board[x][y + 1] == item)
	{
		briksErased += erase(x, y + 1);
	}
	return briksErased;
}

void applyGravity()
{
	int i, j, k;
	for (i = 0; i < hSize; i++)
	{
		for (j = wSize - 1; j >= 0; j--)
		{
			if (board[i][j] == EMPTY)
			{
				int found = 0;
				for (k = j - 1; k >= 0; k--)
				{
					if (board[i][k] != EMPTY)
					{
						board[i][j] = board[i][k];
						board[i][k] = EMPTY;
						addCounter(i, j);
						addCounter(i, k);
						found = 1;
						k = -1;
					}
				}
				if (!found)
				{
					j = -1;
				}
			}
		}
	}
	for (i = hSize - 1; i >= 0; i--)
	{
		int found = 0;
		if (board[i][wSize - 1] == EMPTY)
		{
			for (k = i - 1; k >= 0; k--)
			{
				if (board[k][wSize - 1] != EMPTY)
				{
					found = 1;
					for (j = wSize - 1; j >= 0 && board[k][j] != EMPTY; j--)
					{
						board[i][j] = board[k][j];
						board[k][j] = EMPTY;
						addCounter(i, j);
						addCounter(k, j);
					}
					k = -1;
				}
			}
		}
	}
}

void trySelect()
{
	if (hasSameNeighbours(cursorX, cursorY))
	{
		gameInProgress = 1;
		resetCounters();
		int briksErased = erase(cursorX, cursorY);
		score += briksErased * (briksErased - 1);
		if (score > maxScores[mode])
		{
			updateMaxScore();
		}
		drawScores();
		applyGravity();
		updateCountedArea();
		if (!areMatches())
		{
			gameOver();
		}
	}
}

void tryExit();

int selectedIndex = 0;
void mainMenuHandler(int index) {
	selectedIndex = index;
	switch (index) {
	case 101:
		startNewGame();
		break;
	case 103:
		Message(0, "Help", "This game is played on a rectangular field initially filled with 5 kinds of blocks placed at random. If there are groups of adjoining blocks (more than one) of the same kind, you may remove them from the screen by pressing OK. Blocks that are no longer supported by removed blocks will fall down, and a column without any blocks will be trimmed away by other columns sliding to the left. The goal of the game is to remove as many blocks from the playing field as possible. \n(Wikipedia: http://en.wikipedia.org/wiki/SameGame)\n"
				"You are scoring points for each removed group, bonus is higher more tiles you remove. Also if you manage to clear more that 85% of board, you'll receive bonus up to... Try find out yourself, how big can it be ;)", 60000);
		break;
	case 104:
		Message(0, "About", "SameGame for Pocketbook\nDeveloped by Andriy Kvasnytsya (professor.kam at gmail.com), 2010\nOriginal idea by Kuniaki Moribe, 1985", 10000);
		break;
	case 105:
		tryExit();
		break;
	case 110:
	case 111:
	case 112:
	case 113:
		mode = index - 110;
		setMode();
		saveSettings();
		startNewGame();
	}
}

void showMenu()
{
	OpenMenu(mainMenu, selectedIndex, 20, 20, mainMenuHandler);
}

void exitDialog(int key)
{
	if (key == 1)
	{
		CloseApp();
	}
}

void tryExit()
{
	if (gameInProgress)
	{
		Dialog(ICON_QUESTION, "Exit", "Do you really want to leave application? Your progress won't be saved.", "Yes", "No", exitDialog);
	}
	else
	{
		CloseApp();
	}
}

int main_handler(int type, int par1, int par2)
{
	int i, temp;
	if (type == EVT_INIT)
	{
		srand(time(NULL));
		font = OpenFont("LiberationSans", 16, 0);

		FILE *config = fopen(configFileName, "r");
		if (config != NULL)
		{
			fscanf(config, "%i", &mode);
			for (i = 0; i < MODES_COUNT; i++)
			{
				fscanf(config, "%i", &temp);
				maxScores[i] = temp;
			}
			fclose(config);
		}
		else
		{
			mode = 3;
			for (i = 0; i < MODES_COUNT; i++)
			{
				maxScores[i] = 0;
			}
		}
	    setMode();

		prepareBoard();
	}
	else if (type == EVT_SHOW)
	{
		drawBoard();
	}
	else if (type == EVT_KEYPRESS && par1 != KEY_OK)
	{
		switch (par1)
		{
			case KEY_UP:
				move(0, -1);
				break;
			case KEY_DOWN:
				move(0, 1);
				break;
			case KEY_LEFT:
				move(-1, 0);
				break;
			case KEY_RIGHT:
				move(1, 0);
				break;
			case KEY_BACK:
				tryExit();
				break;
			case KEY_MENU:
				showMenu();
		}
	}
	else if (type == EVT_KEYREPEAT && par1 == KEY_OK)
	{
		showMenu();
	}
	else if (type == EVT_KEYRELEASE && par1 == KEY_OK && par2 == 0)
	{
		trySelect();
	}
	else if (EVT_POINTERDOWN == type){
	    if(move_pointer(par1,par2))
		trySelect();
	    else showMenu();
	}

	return 0;
}

int main(int argc, char **argv)
{
	InkViewMain(main_handler);
	return 0;
}

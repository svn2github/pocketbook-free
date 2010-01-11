#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <dlfcn.h>
#include "inkview.h"

#define H_SIZE 5
#define W_SIZE 6
#define BONUS_LEVEL 0.85
#define CELL_SIZE 46
#define ITEMS 5
#define CURSOR_DELTA 1
#define IMAGE_DELTA 3
#define EMPTY -1

int minX, maxX, minY, maxY;
int baseBonus = 50, maxBonus = 1000;
extern const ibitmap item0, item1, item2, item3, item4;
ifont *font;

const char *configFileName = STATEPATH "/samegame.cfg";

int board[H_SIZE][W_SIZE];
int baseX, baseY;
int cursorX, cursorY;
int briksErased;
int score;
int maxScore;
int gameInProgress = 0;

void prepareBoard()
{
	int i, j;
	for (i = 0; i < H_SIZE; i++)
	{
		for (j = 0; j < W_SIZE; j++)
		board[i][j] = rand() % ITEMS;
	}
	cursorX = cursorY = 0;
	score = 0;
}

void drawCell(int x, int y, int refresh);

void drawScores()
{
	SetFont(font, WHITE);

	FillArea(0, ScreenHeight() - 50, ScreenWidth(), 50, DGRAY);
	char buf[16];
	sprintf(buf, "Current Score: %i", score);
	DrawString(5, ScreenHeight() - 45, buf);
	sprintf(buf, "Highest Score: %i", maxScore);
	DrawString(5, ScreenHeight() - 20, buf);
	PartialUpdate(0, ScreenHeight() - 50, ScreenWidth(), 50);
}

void drawBoard()
{
	int i, j;
	ClearScreen();
	for (i = 0; i < H_SIZE; i++)
	{
		for (j = 0; j < W_SIZE; j++)
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
	FillArea(baseX + x * CELL_SIZE + 1, baseY + y * CELL_SIZE + 1, CELL_SIZE - 1, CELL_SIZE - 1, WHITE);

	item = board[x][y];
	xShift = baseX + x * CELL_SIZE + IMAGE_DELTA;
	yShift = baseY + y * CELL_SIZE + IMAGE_DELTA;

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
		DrawRect(baseX + x * CELL_SIZE + CURSOR_DELTA, baseY + y * CELL_SIZE + CURSOR_DELTA, CELL_SIZE - 2 * CURSOR_DELTA, CELL_SIZE - 2 * CURSOR_DELTA, BLACK);
	}
	if (refresh)
	{
		PartialUpdateBW(baseX + x * CELL_SIZE, baseY + y * CELL_SIZE, CELL_SIZE, CELL_SIZE);
	}
}


void updateMaxScore()
{
	if (maxScore < score)
	{
		maxScore = score;
		FILE *config = fopen(configFileName, "w");
		fprintf(config, "%i", maxScore);
		fclose(config);
	}
}

void gameOver()
{
	int i, j, bonus, count = 0;
	double coefficient;
	char buf[256];

	for (i = 0; i < H_SIZE; i++)
	{
		for (j = 0; j < W_SIZE; j++)
		{
			if (board[i][j] == EMPTY)
			{
				count++;
			}
		}
	}

	if (BONUS_LEVEL * W_SIZE * H_SIZE < count)
	{
		coefficient = (1.0 * count / (H_SIZE * W_SIZE) - BONUS_LEVEL) / (1 - BONUS_LEVEL);
		bonus = baseBonus + coefficient * coefficient * (maxBonus - baseBonus);
		score += bonus;
		sprintf(buf, "No more matches available! You scored %i, including %i for cleared %i%% of screen cleared.", score, bonus, 100 * count / (H_SIZE * W_SIZE));
		updateMaxScore();
	}
	else
	{
		sprintf(buf, "No more matches available! You scored %i and cleared %i%% of screen.", score, 100 * count / (H_SIZE * W_SIZE));
	}

	Message(ICON_INFORMATION, "Game over!", buf, 10000);
	prepareBoard();
	drawBoard();
	gameInProgress = 0;
}

void move(int dx, int dy)
{
	int minX, minY;
	int oldX, oldY;
	if ((cursorX + dx < 0 || cursorX + dx >= H_SIZE || cursorY + dy < 0 || cursorY + dy >= W_SIZE))
	{
		oldX = cursorX;
		oldY = cursorY;
		cursorX = (H_SIZE + cursorX + dx) % H_SIZE;
		cursorY = (W_SIZE + cursorY + dy) % W_SIZE;
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
	minX = H_SIZE;
	minY = W_SIZE;
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
			(i < H_SIZE - 1 && board[i + 1][j] == board[i][j]) ||
			(j < W_SIZE - 1 && board[i][j + 1] == board[i][j])))
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
	for(i = 0; i < H_SIZE; i++)
	{
		for (j = 0; j < W_SIZE; j++)
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
	if (x < H_SIZE - 1 && board[x + 1][y] == item)
	{
		briksErased += erase(x + 1, y);
	}
	if (y < W_SIZE - 1 && board[x][y + 1] == item)
	{
		briksErased += erase(x, y + 1);
	}
	return briksErased;
}

void applyGravity()
{
	int i, j, k;
	for (i = 0; i < H_SIZE; i++)
	{
		for (j = W_SIZE - 1; j >= 0; j--)
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
	for (i = H_SIZE - 1; i >= 0; i--)
	{
		int found = 0;
		if (board[i][W_SIZE - 1] == EMPTY)
		{
			for (k = i - 1; k >= 0; k--)
			{
				if (board[k][W_SIZE - 1] != EMPTY)
				{
					found = 1;
					for (j = W_SIZE - 1; j >= 0 && board[k][j] != EMPTY; j--)
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
		if (score > maxScore)
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
	if (type == EVT_INIT)
	{
		srand(time(NULL));
		font = OpenFont("LiberationSans", 16, 0);

		FILE *config = fopen(configFileName, "r");
		if (config != NULL)
		{
			fscanf(config, "%i", &maxScore);
			fclose(config);
		}
		else
		{
			maxScore = 0;
		}

		baseX = (ScreenWidth() - H_SIZE * CELL_SIZE) / 2;
		baseY = (ScreenHeight() - W_SIZE * CELL_SIZE) / 2;

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
		}
	}
	else if (type == EVT_KEYREPEAT && par1 == KEY_OK)
	{
		tryExit();
	}
	else if (type == EVT_KEYRELEASE && par1 == KEY_OK && par2 == 0)
	{
		trySelect();
	}

	return 0;
}

int main(int argc, char **argv)
{
	InkViewMain(main_handler);
	return 0;
}

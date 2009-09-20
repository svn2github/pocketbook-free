#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <dlfcn.h>
#include "inkview.h"

#define SIZE 9
#define CELL_SIZE 61
#define ITEMS 7
#define LINE_SIZE 5
#define STEP_EMITS 3
#define CURSOR_DELTA 2
#define SELECT_DELTA 3
#define IMAGE_DELTA 5
#define EMPTY -1

extern const ibitmap item0, item1, item2, item3, item4, item5, item6;

int board[SIZE][SIZE];
int lines[SIZE][SIZE];
int wave[SIZE][SIZE];
int baseX, baseY;
int cursorX, cursorY;
int selectedX, selectedY;
int emptyCount;

void prepareBoard()
{
	int i, j;
	for (i = 0; i < SIZE; i++)
	{
		for (j = 0; j < SIZE; j++)
		board[i][j] = EMPTY;
	}
	emptyCount = SIZE * SIZE;
	baseX = (ScreenWidth() - SIZE * CELL_SIZE) / 2;
	baseY = (ScreenHeight() - SIZE * CELL_SIZE) / 2;
	cursorX = cursorY = SIZE / 2;
	selectedX = selectedY = EMPTY;
}

void emit();
void drawCell(int x, int y, int refresh);

void drawBoard()
{
	int i;
	ClearScreen();
	// NOTE: <= is for draw also last line
	for (i = 0; i <= SIZE; i++)
	{
		DrawLine(baseX, baseY + i * CELL_SIZE, baseX + SIZE * CELL_SIZE, baseY + i * CELL_SIZE, BLACK);
		DrawLine(baseX + i * CELL_SIZE, baseY, baseX + i * CELL_SIZE, baseY + SIZE * CELL_SIZE, BLACK);
	}
	FullUpdate();
	drawCell(cursorX, cursorY, 1);
}

void drawCell(int x, int y, int refresh)
{
	int xShift, yShift;
	FillArea(baseX + x * CELL_SIZE + 1, baseY + y * CELL_SIZE + 1, CELL_SIZE - 1, CELL_SIZE - 1, WHITE);
	if (board[x][y] != EMPTY)
	{
		xShift = baseX + x * CELL_SIZE + IMAGE_DELTA;
		yShift = baseY + y * CELL_SIZE + IMAGE_DELTA;
		switch (board[x][y]) {
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
		case 5:
			DrawBitmap(xShift, yShift, &item5);
			break;
		case 6:
			DrawBitmap(xShift, yShift, &item6);
			break;
		}
	}

	if (x == cursorX && y == cursorY)
	{
		DrawRect(baseX + x * CELL_SIZE + CURSOR_DELTA, baseY + y * CELL_SIZE + CURSOR_DELTA, CELL_SIZE - 2 * CURSOR_DELTA + 1, CELL_SIZE - 2 * CURSOR_DELTA + 1, BLACK);
	}
	if (x == selectedX && y == selectedY)
	{
		DrawRect(baseX + x * CELL_SIZE + SELECT_DELTA, baseY + y * CELL_SIZE + SELECT_DELTA, CELL_SIZE - 2 * SELECT_DELTA + 1, CELL_SIZE - 2 * SELECT_DELTA + 1, BLACK);
	}
	if (refresh)
	{
		PartialUpdateBW(baseX + x * CELL_SIZE, baseY + y * CELL_SIZE, CELL_SIZE, CELL_SIZE);
	}
}

void gameOver()
{
	Message(ICON_INFORMATION, "Game over!",
			"Sorry, no more room for balls available", 10000);
	prepareBoard();
	drawBoard();
	emit();
}

int lookOver(int (*getFunc)(int, int), void (*setFunc)(int, int))
{
	int i, j, counter, found = 0;
	char item;
	for (i = 0; i < SIZE; i++)
	{
		//central element is always of kind - as proven below, in calling method
		item = getFunc(i, SIZE / 2);

		if (item == EMPTY)
		{
			//nothing to do in this row
			continue;
		}
		//else - we can find something
		counter = 1;
		for (j = SIZE / 2 - 1; j >= 0; j--)
		{
			if (getFunc(i, j) == item)
			{
				counter++;
			}
			else
			{
				break;
			}
		}
		for (j = SIZE / 2 + 1; j < SIZE; j++)
		{
			if (getFunc(i, j) == item)
			{
				counter++;
			}
			else
			{
				break;
			}
		}
		if (counter >= LINE_SIZE)
		{
			found = 1;
			setFunc(i, SIZE / 2);
			for (j = SIZE / 2 - 1; j >= 0; j--)
			{
				if (getFunc(i, j) == item)
				{
					setFunc(i, j);
				}
				else
				{
					break;
				}
			}
			for (j = SIZE / 2 + 1; j < SIZE; j++)
			{
				if (getFunc(i, j) == item)
				{
					setFunc(i, j);
				}
				else
				{
					break;
				}
			}
		}
	}
	return found;
}

int getBoardCols(int x, int y)
{
	return board[x][y];
}

void setForCols(int x, int y)
{
	lines[x][y] = 1;
}

int getBoardRows(int x, int y)
{
	return board[y][x];
}

void setForRows(int x, int y)
{
	lines[y][x] = 1;
}

int getBoardMainDiags(int x, int y)
{
	//This translates diagonals into square. Items that are out of borders are EMPTY for this
	if (x + y - SIZE / 2 < 0|| SIZE  < x + y - SIZE / 2)
	{
		return EMPTY;
	}
	return board[x + y - SIZE / 2][y];
}

void setForMainDiags(int x, int y)
{
	lines[x + y - SIZE / 2][y] = 1;
}

int getBoardSecDiags(int x, int y)
{
	if (x - y + SIZE / 2 < 0|| SIZE  < x - y + SIZE / 2)
	{
		return EMPTY;
	}
	return board[x - y + SIZE / 2][y];
}

void setForSecDiags(int x, int y)
{
	lines[x - y + SIZE / 2][y] = 1;
}

int matchLines()
{
	int i, j, found = 0;
	for (i = 0; i < SIZE; i++)
	{
		for (j = 0; j < SIZE; j++)
		{
			lines[i][j] = 0;
		}
	}

	// This algorithm is only valid when (SIZE / 2 + 1) = LINE_SIZE, and only for ODD sizes
	// In this case only one line is possible per row/column/diagonal - and it's color is definitely same as central item in it.
	// Besides we can handle diagonals as just special rows/columns because of it count of diagonals is exact.
	found |= lookOver(getBoardRows, setForRows);
	found |= lookOver(getBoardCols, setForCols);
	found |= lookOver(getBoardMainDiags, setForMainDiags);
	found |= lookOver(getBoardSecDiags, setForSecDiags);

	if (found)
	{
		//TODO: count bonuses
		for (i = 0; i < SIZE; i++)
		{
			for (j = 0; j < SIZE; j++)
			{
				if (lines[i][j])
				{
					emptyCount++;
					board[i][j] = EMPTY;
					drawCell(i, j, 1);
				}
			}
		}
	}
	return found;
}

void emit()
{
	int k, i, j;
	int address, currentAddress;
	int breakFlag;
	for (k = 0; k < STEP_EMITS; k++)
	{
		breakFlag = 0;
		address = rand() % emptyCount;
		currentAddress = 0;
		for (i = 0; i < SIZE; i++)
		{
			for (j = 0; j < SIZE; j++)
			{
				if (board[i][j] == EMPTY)
				{
					if (currentAddress == address)
					{
						board[i][j] = rand() % ITEMS;
						emptyCount--;
						drawCell(i, j, 1);

						if (emptyCount == 0)
						{
							gameOver();
						}
						breakFlag = 1;
						break;
					}
					else
					{
						currentAddress++;
					}
				}
			}
			if (breakFlag)
			{
				break;
			}
		}
	}
	matchLines();
}

void move(int dx, int dy)
{
	int minX, minY;
	if (dx != 0 && cursorX + dx >= 0 && cursorX + dx < SIZE)
	{
		cursorX += dx;
		drawCell(cursorX - dx, cursorY, 0);
		drawCell(cursorX, cursorY, 0);

		minX = dx < 0 ? cursorX : cursorX - dx;
		PartialUpdateBW(baseX + CELL_SIZE * minX, baseY + CELL_SIZE * cursorY, CELL_SIZE * 2, CELL_SIZE);
	}
	else if (dy != 0 && cursorY + dy >= 0 && cursorY + dy < SIZE)
	{
		cursorY += dy;
		drawCell(cursorX, cursorY - dy, 0);
		drawCell(cursorX, cursorY, 0);

		minY = dy < 0 ? cursorY : cursorY - dy;
		PartialUpdateBW(baseX + CELL_SIZE * cursorX, baseY + CELL_SIZE * minY, CELL_SIZE, CELL_SIZE * 2);
	}
	else if (dx != 0 && cursorX + dx < 0 && cursorX + dx >= SIZE)
	{

	}
}

int findPath()
{
	int i, j, generation, wasMovement;
	int pointX, pointY, oldPointX, oldPointY, stepsPassed;
	int minX, minY, dX, dY;
	//initialization: 0 - empty, -1 - obstacle, start point is 1
	for (i = 0; i < SIZE; i++)
	{
		for (j = 0; j < SIZE; j++)
		{
			if (board[i][j] == EMPTY)
			{
				wave[i][j] = 0;
			}
			else
			{
				wave[i][j] = -1;
			}
		}
	}
	wave[selectedX][selectedY] = generation = 1;

	do
	{
		wasMovement = 0;
		for (i = 0; i < SIZE; i++)
		{
			for (j = 0; j < SIZE; j++)
			{
				if ((wave[i][j] == 0) &&
						(((i > 0) && wave[i - 1][j] == generation) ||
						((i < SIZE - 1) && wave[i + 1][j] == generation) ||
						((j > 0) && wave[i][j - 1] == generation) ||
						((j < SIZE - 1) && wave[i][j + 1] == generation)))
				{
					wasMovement = 1;
					wave[i][j] = generation + 1;
				}
			}
		}
		generation++;
	}
	while (wasMovement && wave[cursorX][cursorY] == 0);

	//here we are walking path back to begin - everytime from end for each step. Not very rational, but no reason to optimize in such task.
	if (wave[cursorX][cursorY] != 0)
	{
		selectedX = selectedY = EMPTY;
		for (stepsPassed = 1; stepsPassed < wave[cursorX][cursorY]; stepsPassed++)
		{
			pointX = cursorX;
			pointY = cursorY;
			while(wave[pointX][pointY] > stepsPassed)
			{
				oldPointX = pointX;
				oldPointY = pointY;
				if ((pointX > 0) && wave[pointX - 1][pointY] == wave[pointX][pointY] - 1)
				{
					pointX--;
				}
				else if ((pointX < SIZE - 1) && wave[pointX + 1][pointY] == wave[pointX][pointY] - 1)
				{
					pointX++;
				}
				else if ((pointY > 0) && wave[pointX][pointY - 1] == wave[pointX][pointY] - 1)
				{
					pointY--;
				}
				else if ((pointY < SIZE - 1) && wave[pointX][pointY + 1] == wave[pointX][pointY] - 1)
				{
					pointY++;
				}

				if (wave[pointX][pointY] == stepsPassed)
				{
					board[oldPointX][oldPointY] = board[pointX][pointY];
					board[pointX][pointY] = EMPTY;
					minX = oldPointX < pointX ? oldPointX : pointX;
					minY = oldPointY < pointY ? oldPointY : pointY;
					dX = abs(oldPointX - pointX);
					dY = abs(oldPointY - pointY);
					drawCell(pointX, pointY, 0);
					drawCell(oldPointX, oldPointY, 0);
					PartialUpdateBW(baseX + CELL_SIZE * minX, baseY + CELL_SIZE * minY, (dX + 1) * CELL_SIZE, (dY + 1) * CELL_SIZE);
				}
			}
		}
	}
	return wasMovement;
}

void trySelect()
{
	if (selectedX == EMPTY && selectedY == EMPTY)
	{
		if (board[cursorX][cursorY] != EMPTY)
		{
			//selecting cell
			selectedX = cursorX;
			selectedY = cursorY;
			drawCell(selectedX, selectedY, 1);
		}
	}
	else if (selectedX == cursorX && selectedY == cursorY)
	{
		//unselecting cell
		selectedX = selectedY = EMPTY;
		drawCell(cursorX, cursorY, 1);
	}
	else
	{
		if (board[cursorX][cursorY] == EMPTY)
		{
			//moving item
			if (findPath())
			{
				if (!matchLines())
				{
					emit();
				}
			}
		}
	}
}

int main_handler(int type, int par1, int par2)
{
	if (type == EVT_INIT)
	{
		srand(time(NULL));
		prepareBoard();
	}

	if (type == EVT_SHOW)
	{
		drawBoard();
		emit();
	}

	if (type == EVT_KEYPRESS)
	{
		switch (par1)
		{
			case KEY_OK:
				trySelect();
				break;
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
				CloseApp();
				break;
		}
	}
	return 0;
}

int main(int argc, char **argv)
{
	InkViewMain(main_handler);
	return 0;
}


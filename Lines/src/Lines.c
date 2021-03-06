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
#define GAME_BEGIN_EMITS 5
#define CURSOR_DELTA 2
#define SELECT_DELTA 3
#define IMAGE_DELTA 6
#define EMPTY -1

extern const ibitmap item0, item1, item2, item3, item4, item5, item6;
ifont *font;

#ifndef EMULATOR
const char *configFileName = STATEPATH "/lines.cfg";
#else
const char *configFileName = "lines.cfg";
#endif

int board[SIZE][SIZE];
int lines[SIZE][SIZE];
int wave[SIZE][SIZE];
int cache[STEP_EMITS];
int baseX, baseY;
int previewBaseX, previewBaseY;
int cursorX, cursorY;
int selectedX, selectedY;
int emptyCount;

int score;
int maxScore;
int gameInProgress = 0;

static imenu mainMenu[] = {
	{ ITEM_HEADER,   0, "Menu", NULL },
	{ ITEM_ACTIVE, 101, "Start New Game", NULL },
	{ ITEM_ACTIVE, 102, "Help", NULL },
	{ ITEM_ACTIVE, 103, "About", NULL },
	{ ITEM_ACTIVE, 104, "Exit", NULL },
	{ 0, 0, NULL, NULL }
};
void prepareBoard()
{
	int i, j;
	for (i = 0; i < SIZE; i++)
	{
		for (j = 0; j < SIZE; j++)
		board[i][j] = EMPTY;
	}
	emptyCount = SIZE * SIZE;
	cursorX = cursorY = SIZE / 2;
	selectedX = selectedY = EMPTY;
	score = 0;
}

void emit(int items);
void drawCell(int x, int y, int refresh);

void drawScores()
{
	SetFont(font, WHITE);

	FillArea(0, ScreenHeight() - 50, ScreenWidth(), 50, DGRAY);
	char buf[32];
	sprintf(buf, "Current Score: %i", score);
	DrawString(5, ScreenHeight() - 45, buf);
	sprintf(buf, "Highest Score: %i", maxScore);
	DrawString(5, ScreenHeight() - 20, buf);
	PartialUpdate(0, ScreenHeight() - 50, ScreenWidth(), 50);
}

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
	DrawLine(previewBaseX, previewBaseY, previewBaseX + CELL_SIZE * STEP_EMITS, previewBaseY, BLACK);
	DrawLine(previewBaseX, previewBaseY + CELL_SIZE, previewBaseX + CELL_SIZE * STEP_EMITS, previewBaseY + CELL_SIZE, BLACK);
	for (i = 0; i <= STEP_EMITS; i++)
	{
		DrawLine(previewBaseX + CELL_SIZE * i, previewBaseY, previewBaseX + CELL_SIZE * i, previewBaseY + CELL_SIZE, BLACK);
	}
	FullUpdate();
	drawCell(cursorX, cursorY, 1);
	drawScores();
}

void drawCell(int x, int y, int refresh)
{
	int xShift, yShift, item;
	FillArea(baseX + x * CELL_SIZE + 1, baseY + y * CELL_SIZE + 1, CELL_SIZE - 1, CELL_SIZE - 1, WHITE);
	if (y != EMPTY)
	{
		item = board[x][y];
		xShift = baseX + x * CELL_SIZE + IMAGE_DELTA;
		yShift = baseY + y * CELL_SIZE + IMAGE_DELTA;
	}
	else
	{
		item = cache[x];
		xShift = previewBaseX + x * CELL_SIZE + IMAGE_DELTA;
		yShift = previewBaseY + IMAGE_DELTA;
	}
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
		if (x != cursorX || y != cursorY)
		{
			DrawRect(baseX + x * CELL_SIZE + SELECT_DELTA, baseY + y * CELL_SIZE + SELECT_DELTA, CELL_SIZE - 2 * SELECT_DELTA + 1, CELL_SIZE - 2 * SELECT_DELTA + 1, BLACK);
		}
		DrawRect(baseX + x * CELL_SIZE + SELECT_DELTA + 1, baseY + y * CELL_SIZE + SELECT_DELTA + 1, CELL_SIZE - 2 * SELECT_DELTA - 1, CELL_SIZE - 2 * SELECT_DELTA - 1, BLACK);
	}
	if (refresh)
	{
		PartialUpdateBW(baseX + x * CELL_SIZE, baseY + y * CELL_SIZE, CELL_SIZE, CELL_SIZE);
	}
}

void drawPreview()
{
	int i;
	for (i = 0; i < STEP_EMITS; i++)
	{
		drawCell(i, EMPTY, 0);
	}
	PartialUpdateBW(previewBaseX, previewBaseY, STEP_EMITS * CELL_SIZE, CELL_SIZE);
}

void startNewGame()
{
    prepareBoard();
    drawBoard();
    emit(GAME_BEGIN_EMITS);
    gameInProgress = 0;
}

void gameOver()
{
	Message(ICON_INFORMATION, "Game over!",
			"Sorry, no more room for balls available", 10000);
    startNewGame();
}

int lookOver(int (*getFunc)(int, int), void (*setFunc)(int, int))
{
	int i, j, counter, points = 0;
	signed char item;
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
			points = counter * counter + (1 - 2 * LINE_SIZE) * counter + LINE_SIZE * (LINE_SIZE + 1);
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
	return points;
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

int (*getFunctions[4])(int, int)  = {getBoardRows, getBoardCols, getBoardMainDiags, getBoardSecDiags};
void (*setFunctions[4])(int, int) = {setForRows, setForCols, setForMainDiags, setForSecDiags};

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

int pow3(int power)
{
	int i, result = 1;
	for (i = 1; i < power; i++)
	{
		result *= 3;
	}
	return result;
}

int matchLines()
{
	int i, j, curPoints, points = 0, linesFound = 0;
	for (i = 0; i < SIZE; i++)
	{
		for (j = 0; j < SIZE; j++)
		{
			lines[i][j] = 0;
		}
	}

	// This algorithm is only valid when LINE_SIZE > SIZE / 2, and only for ODD sizes
	// In this case only one line is possible per row/column/diagonal - and it's color is definitely same as central item in it.
	// Besides we can handle diagonals as just special rows/columns because of it count of diagonals is exact.
	for (i = 0; i < 4; i++)
	{
		curPoints = lookOver(getFunctions[i], setFunctions[i]);
		if (curPoints != 0)
		{
			linesFound++;
		}
		points += curPoints;
	}

	if (points)
	{
		points *= pow3(linesFound);

		score += points;
		if (score > maxScore)
		{
			updateMaxScore();
		}

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
		drawScores();
	}
	return points;
}

void emit(int items)
{
	int k, i, j;
	int address, currentAddress;
	int breakFlag;
	for (k = 0; k < items; k++)
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
						if (k < STEP_EMITS)
						{
							board[i][j] = cache[k];
							cache[k] = rand() % ITEMS;
						}
						else
						{
							board[i][j] = rand() % ITEMS;
						}
						emptyCount--;
						drawCell(i, j, 1);

						if (emptyCount == 0)
						{
							gameOver();
							return;
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
	drawPreview();
	matchLines();
}

int move_pointer(int cx, int cy)
{
	double ix=((double)(cx-baseX)/CELL_SIZE);
	double iy=((double)(cy-baseY)/CELL_SIZE);
	//int minX, minY;
	int oldX, oldY;
	if(ix<0 || ix>=SIZE)return 0;
	if(iy<0 || iy>=SIZE)return 0;
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
	if ((cursorX + dx < 0 || cursorX + dx >= SIZE || cursorY + dy < 0 || cursorY + dy >= SIZE))
	{
		oldX = cursorX;
		oldY = cursorY;
		cursorX = (SIZE + cursorX + dx) % SIZE;
		cursorY = (SIZE + cursorY + dy) % SIZE;
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

int findPath()
{
	int i, j, generation, wasMovement;
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

	return wave[cursorX][cursorY] != 0;
}

void trySelect()
{
	int oldX, oldY;
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
			if (findPath())
			{
				gameInProgress = 1;
				//moving item
				board[cursorX][cursorY] = board[selectedX][selectedY];
				oldX = selectedX;
				oldY = selectedY;
				selectedX = selectedY = EMPTY;
				board[oldX][oldY] = EMPTY;
				drawCell(oldX, oldY, 1);
				drawCell(cursorX, cursorY, 1);
				if (!matchLines())
				{
					emit(STEP_EMITS);
				}
			}
			else
			{
				//unselecting cell
				oldX = selectedX;
				oldY = selectedY;
				selectedX = selectedY = EMPTY;
				drawCell(oldX, oldY, 1);
			}
		}
		else
		{
			// selecting different item
			oldX = selectedX;
			oldY = selectedY;
			selectedX = cursorX;
			selectedY = cursorY;
			drawCell(oldX, oldY, 1);
			drawCell(selectedX, selectedY, 1);
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

int selectedIndex = 0;
void mainMenuHandler(int index) {
	selectedIndex = index;
	switch (index) {
	case 101:
		startNewGame();
		break;
	case 102:
		Message(0, "Help", "This game was originally created as Color Lines in 1992 by Oleg Demin for the Gamos company. It starts with a 9x9 board with five balls chosen out of seven different kinds. You can move one ball per turn, moving cursor, clicking OK on ball to move and again OK on empty cell to move. You may only move a ball to a particular place if there is a path (linked set of vertical and horizontal empty cells) between the current position of the ball and the desired destination. The goal is to remove balls by forming lines (horizontal, vertical or diagonal) of at least five balls of the same kind. If you do form such lines, the balls in them disappear, and you gain one turn, i.e. you can move another ball. If not, three new balls are added, and the game continues until the board is full.\n(Wikipedia: http://en.wikipedia.org/wiki/Lines_(video_game) )\n"
			   "You are scoring points for removing lines, points are higher for more balls in removed in one time.", 60000);
		break;
	case 103:
		Message(0, "About", "Lines for Pocketbook\n"
			   "Modifyed by Yury P. Fedorchenko, 2011"
			   "Developed by Andriy Kvasnytsya (professor.kam at gmail.com), 2010\nGraphics by Tetiana Sherbul, 2010\nOriginal idea by Oleg Demin, 1992", 10000);
		break;
	case 104:
		tryExit();
		break;
	}
}

void showMenu()
{
	OpenMenu(mainMenu, selectedIndex, 20, 20, mainMenuHandler);
}

int main_handler(int type, int par1, int par2)
{
	int i;
	if (type == EVT_INIT)
	{
		srand(time(NULL));
		font = OpenFont("LiberationSans", 16, 0);
                SetOrientation(0);
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

		baseX = (ScreenWidth() - SIZE * CELL_SIZE) / 2;
		baseY = (ScreenHeight() - SIZE * CELL_SIZE) / 2;
		previewBaseX = (ScreenWidth() - STEP_EMITS * CELL_SIZE) / 2;
		previewBaseY = (baseY - CELL_SIZE) / 2;
		for (i = 0; i < STEP_EMITS; i++)
		{
			cache[i] = rand() % ITEMS;
		}

		prepareBoard();
	}
	else if (type == EVT_SHOW)
	{
		drawBoard();
		emit(GAME_BEGIN_EMITS);
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
				showMenu();
				//tryExit();
				break;
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


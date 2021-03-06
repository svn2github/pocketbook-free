#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <dlfcn.h>
#include "inkview.h"

#define SIZE 5
#define CELL_SIZE 119
#define MAX SIZE * SIZE
int board[MAX];
ifont *f;

void prepareBoard()
{
	int i, j;
	int address;
	int currentAddress;
	int random;
	for (i = 0; i < MAX; i++)
	{
		board[i] = 0;
	}
	for (i = 0; i < MAX; i++)
	{
		random = rand();
		address = random % (MAX - i);
		currentAddress = 0;
		for (j = 0; j < MAX; j++)
		{
			if (board[j] == 0)
			{
				if (currentAddress == address)
				{
					board[j] = i + 1;
					break;
				}
				else
				{
					currentAddress++;
				}
			}
		}
	}
}

void timeOut()
{
	char buf[64];
	sprintf(buf, "%i seconds went out!", MAX);
	Message(ICON_INFORMATION, "Time Out", buf, 5000);
}

void drawBoard()
{
	SetFont(f, BLACK);
	int i, j;
	char buf[3];
	int baseX = (ScreenWidth() - SIZE * CELL_SIZE) / 2;
	int baseY = (ScreenHeight() - SIZE * CELL_SIZE) / 2;
	int xShift, yShift;
	ClearScreen();
	// NOTE: <= is for draw also last line
	for (i = 0; i <= SIZE; i++)
	{
		DrawLine(baseX, baseY + i * CELL_SIZE, baseX + SIZE * CELL_SIZE, baseY + i * CELL_SIZE, BLACK);
		DrawLine(baseX + i * CELL_SIZE, baseY, baseX + i * CELL_SIZE, baseY + SIZE * CELL_SIZE, BLACK);
	}
	for (i = 0; i < SIZE; i++)
	{
		for (j = 0; j < SIZE; j++)
		{
			sprintf(buf, "%i", board[SIZE * i + j]);
			xShift = (CELL_SIZE - StringWidth(buf)) / 2;
			yShift = (CELL_SIZE - f->height) / 2;
			DrawString(baseX + i * CELL_SIZE + xShift, baseY + j * CELL_SIZE + yShift, buf);
		}
	}
	FullUpdate();
	ClearTimer(timeOut);
	SetWeakTimer("timeOut", timeOut, MAX * 1000);
}

int main_handler(int type, int par1, int par2)
{
	if (type == EVT_INIT)
	{
		srand(time(NULL));
		f = OpenFont("LiberationSans", CELL_SIZE * 2 / 3, 0);
		prepareBoard();
	}

	if (type == EVT_SHOW)
	{
		drawBoard();
	}
	else if ((type == EVT_KEYPRESS && par1 == KEY_BACK) || (type == EVT_KEYREPEAT && (par1 == KEY_OK||par1 == KEY_MENU||par1 == KEY_HOME)))
	{
		CloseApp();
	}
	else if (type == EVT_KEYRELEASE && (par1 == KEY_OK||par1 == KEY_NEXT||par1 == KEY_PREV))
	{
		prepareBoard();
		drawBoard();
	}

	return 0;
}

int main(int argc, char **argv)
{
	InkViewMain(main_handler);
	return 0;
}

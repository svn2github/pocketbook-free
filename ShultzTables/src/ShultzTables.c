#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <dlfcn.h>
#include "inkview.h"

#define SIZE 5
#define CELL_SIZE 72
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

void drawBoard()
{
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
}

int main_handler(int type, int par1, int par2)
{
	if (type == EVT_INIT)
	{
		srand(time(NULL));
		f = OpenFont("LiberationSans", CELL_SIZE * 2 / 3, 0);
		SetFont(f, BLACK);
		prepareBoard();
	}

	if (type == EVT_SHOW)
	{
		drawBoard();
	}

	if (type == EVT_KEYPRESS)
	{
		switch (par1)
		{
			case KEY_OK:
				prepareBoard();
				drawBoard();
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

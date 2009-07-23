/*
 * PoTerm: primitive xterm for PocketBook - main
 * ---------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>

#include "inkview.h"
#include "poterm.h"
#include "Term.h"

///////////////////////////////////////////////////////////////////////
void rotate_handler(int direction)
{
    SetOrientation(direction);
    printf("orientation=%d, %dx%d\n", direction, ScreenWidth(), ScreenHeight());
    ClearScreen();
    FillArea(100, 100, 200, 200, BLACK);
    FullUpdate();
} // rotatehandler

int main_handler(int type, int par1, int)
{
    switch (type)
    {
        case EVT_INIT:
            ClearScreen();
            FillArea(300, 300, 20, 20, BLACK);
            FullUpdate();
            printf("initial orientation=%d\n", GetOrientation());
        case EVT_KEYPRESS:
            switch(par1)
            {
                case KEY_BACK:
                    CloseApp();
                    break;
                case KEY_MENU:
                case KEY_OK:
                    OpenRotateBox(rotate_handler);
                    break;
                default:
                    break;
            }
        default:
            break;
    }
    return 0;
}

int main()
{
    InkViewMain(main_handler);
    return 0;
} //  main

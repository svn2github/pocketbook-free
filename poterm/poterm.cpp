/*
 * PoTerm: primitive xterm for PocketBook - main
 * ---------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>

#include "inkview.h"
#include "poterm.h"
#include "Term.h"

static char kbuffer[KBUFFER_LEN];
Term        *term = 0;

static char timer_name[] = "poterm_timer";
///////////////////////////////////////////////////////////////////////
void keyboard_entry(char *s)
{
    if (s  && *s)
        term->command(s);
} // keyboard_entry

///////////////////////////////////////////////////////////////////////
void timer_handler()
{
    int was_read;
    term->check_output(was_read);
    if (was_read > 0)
        term->redraw();
    SetWeakTimer(timer_name, timer_handler, term->checkInterval());
} // keyboard_entry

///////////////////////////////////////////////////////////////////////
int main_handler(int type, int par1, int)
{
    int oldOrientation = 0;

    switch (type)
    {
        case EVT_INIT:
            oldOrientation = GetOrientation();
            term = new Term();
            if (!term  ||  !term->initialized())
                CloseApp();
            SetWeakTimer(timer_name, timer_handler, term->checkInterval());
            //{
            //    char **f = EnumFonts();
            //    for (int i=0; f[i]; i++)
            //        printf("F \"%s\"\n", f[i]);
            //}
            break;
        case EVT_SHOW:
            term->redraw();
            break;
        case EVT_EXIT:
            delete term;
            SetOrientation(oldOrientation);
            break;
        case EVT_KEYPRESS:
            switch(par1)
            {
                case KEY_OK:
                    OpenKeyboard(term->prompt(), kbuffer, KBUFFER_LEN, 0, keyboard_entry);
                    break;
                case KEY_BACK:
                    CloseApp();
                    break;
                case KEY_LEFT:
                    term->lineUp();
                    break;
                case KEY_RIGHT:
                    term->lineDown();
                    break;
                case KEY_UP:
                    term->pageUp();
                    break;
                case KEY_DOWN:
                    term->pageDown();
                    break;
                case KEY_MENU:
#ifndef DISABLE_ROTATION
                    term->rotate();
#endif
                    break;
                case KEY_MINUS:
                    term->decreaseFont();
                    break;
                case KEY_PLUS:
                    term->increaseFont();
                    break;
                default:
                    break;
            }
        default:
            break;
    }
    return 0;
} // main_handler

///////////////////////////////////////////////////////////////////////
int main()
{
    InkViewMain(main_handler);
    return 0;
} //  main

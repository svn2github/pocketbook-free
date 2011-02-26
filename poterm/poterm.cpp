/*
 * PoTerm: primitive xterm for PocketBook - main
 * ---------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>

#include "inkview.h"
#include "inkinternal.h"
#include "poterm.h"
#include "Term.h"

static char kbuffer[KBUFFER_LEN];
Term        *term = 0;

static char timer_name[] = "poterm_timer";
///////////////////////////////////////////////////////////////////////
void keyboard_entry(char *s)
{
    term->setHeightNoKbd(0);
    term->redrawAll();
    if (s  && *s)
        term->command(s);
} // keyboard_entry

///////////////////////////////////////////////////////////////////////
void timer_handler()
{
    P("Shell output polling timer\n");
    int was_read;
    term->check_output(was_read);
    if (was_read > 0)
        term->redraw();
    SetWeakTimer(timer_name, timer_handler, term->checkInterval());
} // keyboard_entry

static imenu menu1[]={
  {ITEM_HEADER,  0, "PoTerm",NULL},
  {ITEM_ACTIVE,101,"Command",NULL},
  {ITEM_SEPARATOR,0,NULL,NULL},
  {ITEM_ACTIVE,102,"Line Up",NULL},
  {ITEM_ACTIVE,103,"Line Down",NULL},
  {ITEM_ACTIVE,104,"Page Up",NULL},
  {ITEM_ACTIVE,105,"Page Down",NULL},
  {ITEM_SEPARATOR,0,NULL,NULL},
  {ITEM_ACTIVE,106,"Rotate",NULL},
  {ITEM_SEPARATOR,0,NULL,NULL},
  {ITEM_ACTIVE,107,"Decrease Font",NULL},
  {ITEM_ACTIVE,108,"Increase Font",NULL},
  {ITEM_SEPARATOR,0,NULL,NULL},
  {ITEM_ACTIVE,109,"Exit",NULL},
  {0,0,NULL,NULL}
};
static int cindex=0;
void menu_handler(int index){
  cindex = index;
  switch(index){
    case 101:
      OpenKeyboard(term->prompt(), kbuffer, KBUFFER_LEN, 0, keyboard_entry);
      term->setHeightNoKbd(iv_msgtop());
      term->redrawAll();
      break;
    case 109:
      CloseApp();
      break;
    case 102:
      term->lineUp();
      break;
    case 103:
      term->lineDown();
      break;
    case 104:
      term->pageUp();
      break;
    case 105:
      term->pageDown();
      break;
    case 106:
      term->rotate();
      break;
    case 107:
      term->decreaseFont();
      break;
    case 108:
      term->increaseFont();
      break;
    default:
      break;
  }
}
///////////////////////////////////////////////////////////////////////
int main_handler(int type, int par1, int par2)
{
    static int prevEvent = 0;
    int        oldOrientation = 0;

    P("main_handler(%d, %d, ...)\n", type, par1);
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
            if (prevEvent != EVT_INIT)
                term->redraw();
            break;
        case EVT_EXIT:
            delete term;
            SetOrientation(oldOrientation);
            break;
        /*case EVT_KEYREPEAT:
            if(KEY_OK == par1){
              OpenMenu(menu1,cindex,20,20,menu_handler);
            }
            break;*/
        case EVT_KEYPRESS:
            switch(par1)
            {
                case KEY_OK:
                    OpenKeyboard(term->prompt(), kbuffer, KBUFFER_LEN, 0, keyboard_entry);
                    term->setHeightNoKbd(iv_msgtop());
                    term->redrawAll();
                    break;
                case KEY_BACK:
                    //CloseApp();
                    OpenMenu(menu1,cindex,20,20,menu_handler);
                    break;
                case KEY_LEFT:
                    term->lineUp();
                    break;
                case KEY_RIGHT:
                    term->lineDown();
                    break;
                case KEY_UP:
                case KEY_PREV:
                    term->pageUp();
                    break;
                case KEY_DOWN:
                case KEY_NEXT:
                    term->pageDown();
                    break;
                case KEY_DELETE:
                    term->rotate();
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
            break;
        case EVT_POINTERDOWN:
            OpenMenu(menu1,cindex,par1,par2,menu_handler);
            break;
        default:
            break;
    }
    prevEvent = type;
    return 0;
} // main_handler

///////////////////////////////////////////////////////////////////////
int main()
{
    InkViewMain(main_handler);
    return 0;
} //  main

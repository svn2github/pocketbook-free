/*
 * PoTerm: primitive xterm for PocketBook - main
 * ---------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#include "inkview.h"
//#include "inkinternal.h"
extern "C"{
int iv_msgtop();
};
#include "poterm.h"
#include "Term.h"

static char kbuffer[KBUFFER_LEN];
Term        *term = 0;

static char timer_name[] = "poterm_timer";
///////////////////////////////////////////////////////////////////////
void keyboard_entry(const char *s)
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
  {ITEM_BULLET,110,"Auto Rotate",NULL},
  {ITEM_SEPARATOR,0,NULL,NULL},
  {ITEM_ACTIVE,107,"Decrease Font",NULL},
  {ITEM_ACTIVE,108,"Increase Font",NULL},
  {ITEM_SEPARATOR,0,NULL,NULL},
  {ITEM_ACTIVE,109,"Exit",NULL},
  {0,0,NULL,NULL}
};

#include "pbtk/pbdialog.h"
#include "pbtk/pbcombobox.h"
#include "pbtk/pbbutton.h"

class PBCommandDialog:public PBDialog{
  PBComboBox bx_history;
  PBButton bt_ok;
  PBButton bt_cancel;
  public:
  PBCommandDialog(const std::string& ttl):PBDialog(ttl),bx_history("",this),bt_ok("Ok",this),
  bt_cancel("Cancel",this){
    addWidget(&bx_history);
    addWidget(&bt_ok);
    addWidget(&bt_cancel);
    bt_ok.onPress.connect(sigc::mem_fun(this,&PBCommandDialog::run_command));
    bt_cancel.onPress.connect(sigc::mem_fun(this,&PBCommandDialog::run_command));
    std::ifstream hst(term->_config["HstFile"].c_str());
    if(!hst.fail()){
      std::vector<std::string> v;
      while(!hst.fail() || !hst.eof()){
        std::string a;
        std::getline(hst,a);
        if(!a.empty())v.push_back(a);
        if(v.size()>20)break;
      }
      bx_history.setItems(v);
    }
  }
  ~PBCommandDialog(){
    std::vector<std::string> v = bx_history.getItems();
    if(!v.empty()){
      std::ofstream hst(term->_config["HstFile"].c_str());
      for(int i=0;i<v.size();++i) hst<<v[i]<<std::endl;
    }
  }
  void placeWidgets(){
    int ch=captionHeight();
    setSize(5,ScreenHeight()-60-ch,ScreenWidth()-10,60+ch);
    bx_history.setSize(x()+5,y()+5+ch,w()-10,25);
    bt_ok.setSize(x()+5,y()+30+ch,w()/2-10,25);
    bt_cancel.setSize(x()+w()/2+5,y()+30+ch,w()/2-10,25);
  }
  void run_command(PBButton* bt){
    quit((bt==&bt_ok));
    keyboard_entry((bt==&bt_ok)?bx_history.getText().c_str():NULL);
  }
} *comdlg=NULL;

void do_command(){
#if NO_HISTORY
      OpenKeyboard(term->prompt(), kbuffer, KBUFFER_LEN, 0, (void (*)(char*))keyboard_entry);
      term->setHeightNoKbd(iv_msgtop());
      term->redrawAll();
#else
  if(!comdlg)comdlg=new PBCommandDialog("");
  comdlg->setText(term->prompt());
  comdlg->run();
  term->setHeightNoKbd(comdlg->y());
  term->redrawAll();
#endif
}

static int cindex=0;
void menu_handler(int index){
  cindex = index;
  switch(index){
    case 101:
      do_command();
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
            delete comdlg;
            delete term;
            SetOrientation(oldOrientation);
            break;
        case EVT_ORIENTATION:
            if(menu1[9].type==ITEM_BULLET)SetOrientation(par1);
            break;
        case EVT_KEYREPEAT:
            if(KEY_OK == par1){
              OpenMenu(menu1,cindex,20,20,menu_handler);
            }
            break;
        case EVT_KEYRELEASE:
            switch(par1)
            {
                case KEY_OK:
                    do_command();
                    /*OpenKeyboard(term->prompt(), kbuffer, KBUFFER_LEN, 0, keyboard_entry);
                    term->setHeightNoKbd(iv_msgtop());
                    term->redrawAll();*/
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

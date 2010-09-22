/*
 *   Copyright (C) 2009-2010 Yury P. Fedorchenko
 *   yuryfdr@users.sf.net
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include <FL/Fl_Double_Window.H>
#include <FL/Fl.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Pixmap.H>
#include <FL/fl_ask.H>

#include <iostream>

#include "chboard.h"

using std::cerr;
using std::endl;

#include "img/kingblack.xpm"
#include "img/kingwhite.xpm"
#include "img/manblack.xpm"
#include "img/manwhite.xpm"

//#if FL_MINOR_VERSION <3
  #define _(a) a
/*#else
  #include "src/nls.h"
namespace {
  struct locinit{
    locinit(){
#ifdef ENABLE_NLS
  setlocale(LC_ALL,"");
  //std::cerr<<MNW_LOC_DIR<<'\t'<<PACKAGE<<std::endl;
  const char* btd=bindtextdomain(PACKAGE,MNW_LOC_DIR);
  const char* tdc=bind_textdomain_codeset(PACKAGE,"UTF-8");
  textdomain(PACKAGE);
#endif
  }
}aaaa;
};

#endif*/

namespace{
  void update_menus();
  const char *skill_v[] = {"Beginner","Novice","Average","Good","Expert","Master", NULL };
  Fl_Pixmap king2(kingblack_xpm),king1(kingwhite_xpm);
  Fl_Pixmap man2(manblack_xpm),man1(manwhite_xpm);
};

class Fl_Board : public Fl_Widget, public CheckersBoard{
protected:  
  static const int csize=50;
  Fl_Pixmap *mn1,*mn2,*kg1,*kg2;
  virtual void set_checkers(){
    if(Checkers::RUSSIAN==rules){
      mn1 = &man1;
      mn2 = &man2;
      kg1 = &king1;
      kg2 = &king2;
    }else{
      mn1 = &man2;
      mn2 = &man1;
      kg1 = &king2;
      kg2 = &king1;
    }
  }
  void draw_chess(int t,int X,int Y){
    
    switch(t){
       	  case Checkers::MAN1:
          if(is_white)mn1->draw(X+9,Y+9);
          else mn2->draw(X+9,Y+9);
          return;
       	  case Checkers::MAN2:
          if(is_white)mn2->draw(X+9,Y+9);
          else mn1->draw(X+9,Y+9);
          return;
       	  case Checkers::KING1:
          if(is_white)kg1->draw(X+9,Y+9);
          else kg2->draw(X+9,Y+9);
          return;
       	  case Checkers::KING2:
          if(is_white)kg2->draw(X+9,Y+9);
          else kg1->draw(X+9,Y+9);
          return;
       	  default:
       	  ;
    }
  }
  // create new game
public:
  void new_game(bool h2)
  { 
    CheckersBoard::new_game(h2);
    redraw();
  }
  
  Fl_Board(int X,int Y,const char L=0) : Fl_Widget(X,Y,400,440){
    box(FL_ENGRAVED_BOX);
    set_checkers();
  }
  void draw(){
    draw_box();
    int item=0;
    for(int j=0;j<8;++j){
      for(int i=0;i<8;++i){
        fl_color(((i+j)%2)?FL_DARK3:FL_WHITE);
        int X=x()+i*csize
           ,Y=y()+j*csize;
	      fl_rectf(X,Y,csize,csize);
		
	      if((i+j)%2){
	        fl_color(FL_BLACK);
	        fl_font(FL_BOLD,8);
	        /*char str[32];
	        sprintf(str,"%d (%d,%d)",item,i,j);
	        fl_draw(str,x()+i*50+9,y()+j*50+9);*/
	        if(cp==item){
	          fl_color(FL_BLUE);
	          fl_rectf(X+4,Y+4,csize-8,csize-8);
	        }
	        draw_chess(game->item(item++),X,Y);
		    }
	  
      }
    }
    fl_color(FL_BLACK);
    fl_font(FL_BOLD,10);
    char str[512];
	  sprintf(str,_("Game type:%s    Level:%s "),Checkers::RUSSIAN==rules?_("Russian"):_("English"),skill_v[Checkers::skill2index(skill)]);
	  fl_draw(str,x()+5,y()+410);
	  if(game_over){
	   sprintf(str,_("Game Over! %s"),(winner==1)?_("Winner Computer"):((winner==2)?_("You Win!"):_("Nobody can move!")));
	   fl_draw(str,x()+5,y()+420);
	  }
  }
  int handle(int e){
    update_menus();
    static bool move = false;
    if(FL_PUSH == e && !move && !game_over){
      move=true;
      int X=(Fl::event_x()-x())/50;
      int Y=(Fl::event_y()-y())/50;
	    int gm_ind=((X+Y)%2)?(X/2+4*Y):-1;
      //cerr<<"match:"<<gm_ind<<" cp:"<<cp<<" free:"<< game->item(gm_ind)<<endl;
	    if(go1 && Checkers::FREE==game->item(gm_ind) && cp!=-1 && gm_ind !=-1 && game->go1(cp,gm_ind)){
	      if(!check_win(false)){
	        redraw();Fl::check();
                if(!ai2){
	          game->go2();
	          cp = -1;
	          check_win(true);
	        }else{
                  go1=false;
		      }
	      }
	    }else if(!go1 && Checkers::FREE==game->item(gm_ind) && cp!=-1 && gm_ind !=-1 && game->go2_human(cp,gm_ind)){
	      if(!check_win(true)){
	        redraw();Fl::check();
                go1=true;
	      }
      }else cp=gm_ind;
      move=false;
	    redraw();
      return 1;
    }
  }
};

static Fl_Board* brd = (Fl_Board*)0;

void cb_new(Fl_Widget*,long p){
  brd->new_game(p);
}

static Fl_Window* setw = (Fl_Window*)0;
static Fl_Choice* ch_rules;
static Fl_Choice* ch_level;
static Fl_Check_Button* chb_white;
static Fl_Return_Button* bt_ok;

void cb_setup(Fl_Widget*,void*){
  if(!setw){
    setw=new Fl_Window(155,125,_("Settings"));
    ch_rules = new Fl_Choice(50,5,100,25,_("Rules"));
    ch_rules->add(_("Russian|English"));
    ch_level = new Fl_Choice(50,35,100,25,_("Level"));
    ch_level->add(_("BEGINNER|NOVICE|AVERAGE|GOOD|EXPERT|MASTER"));
    chb_white= new Fl_Check_Button(5,65,150,25,_("Human is first"));
    bt_ok    = new Fl_Return_Button(100,95,50,25,_("Ok"));
    setw->set_modal();
  }
  ch_rules->value(Checkers::RUSSIAN==brd->rules?0:1);
  ch_level->value(Checkers::skill2index(brd->skill));
  chb_white->value(brd->is_white);
  setw->show();
  for (;;) {
    Fl_Widget *o = Fl::readqueue();
    if (!o) Fl::wait();
    else if (o == bt_ok){
    //0 1 2 3 4 5
    //2,4,6,7,8,9
//-4 -2,0,2,3,4,5
    setw->hide();
    brd->rules=(1==ch_rules->value())?Checkers::ENGLISH:Checkers::RUSSIAN;
    brd->is_white=chb_white->value();
    brd->skill=Checkers::index2skill(ch_level->value());
    brd->new_game(0);
    break;
    }
    else if (o == setw) {break;}
  }
}
void cb_rules(Fl_Widget*,void*){
        fl_message(
        "In the beginning of game you have 12 checkers (men).\n"
		    "The men move forward only. The men can capture:\n"
		    "-by jumping forward only (english rules);\n"
		    "-by jumping forward or backward (russian rules).\n"
		    "A man which reaches the far side of the board becomes a king.\n"
		    "The kings move forward or backward:\n"
		    "-to one square only (english rules);\n"
		    "-to any number of squares (russian rules).\n"
		    "The kings capture by jumping forward or backward.\n  "
		    "Whenever a player is able to make a capture he must do so.");
}
void cb_about(Fl_Widget*,void*){
        fl_message(
        _("Checkers game x 1.3.0\n"
				"by Yury P. Fedorchenko.\n"
				"AI based on kcheckers.\n"
        "This is free sowtware and distributed under terms\n of"
        "GNU GPL License\n"
        "www.fedorchenko.net") );
}
void cb_exit(Fl_Widget*,void*){
  if(fl_ask(_("Exit Game?")))exit(0);
}
void cb_undo(Fl_Widget*,void*){
  brd->game->undo();
  brd->redraw();
}
void cb_redo(Fl_Widget*,void*){
  cerr<<brd->game->can_redo()<<endl;
  brd->game->redo();
  brd->redraw();
}

Fl_Menu_Item mmenu[]={
  {_("Game"),0,0,0,FL_SUBMENU},
    {_("New"),FL_ALT+'n',(Fl_Callback*)cb_new,(void*)0},
    {_("New 2 player"),0,(Fl_Callback*)cb_new,(void*)1,FL_MENU_DIVIDER},
    {_("Undo"),FL_CTRL+'z',cb_undo,0,FL_MENU_INACTIVE},
    {_("Redo"),FL_CTRL+'y',cb_redo,0,FL_MENU_DIVIDER|FL_MENU_INACTIVE},
    {_("Settings"),0,cb_setup,0,FL_MENU_DIVIDER},
    {_("Exit"),FL_CTRL+'x',cb_exit},
  {0},
  {_("Help"),0,0,0,FL_SUBMENU},
    {_("Rules"),0,cb_rules},
    {_("About"),0,cb_about},
  {0},
  {0}
};

namespace{
  void update_menus(){
    if(brd->game->can_redo() && !brd->ai2){
      mmenu[4].activate();
    }else{
      mmenu[4].deactivate();
    }
    if(brd->game->can_undo() && !brd->ai2){
      mmenu[3].activate();
    }else{
      mmenu[3].deactivate();
    }
  }
};

int main(int argc,char** argv){
  Fl_Double_Window mainwnd(410,480,_("Shashki"));
  Fl_Menu_Bar menub(0,0,410,25);
  menub.menu(mmenu);
  brd = new Fl_Board(5,30);
  mainwnd.callback(cb_exit);
  mainwnd.show(argc,argv);
  return Fl::run();
}


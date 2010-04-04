/*
 *   Copyright (C) 2009 Yury P. Fedorchenko
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <dlfcn.h>
#include "inkview.h"

#include "chboard.h"

#define PartialUpdateA PartialUpdateBW
#include <iostream>
using std::endl; 
using std::cerr; 

extern ibitmap black_m,white_m,black_k,white_k,black_ms,white_ms,black_ks,white_ks;

static ifont* boardf;

static iconfig *pbchcfg = NULL;

static char *rules_v[] = {"Russian", "English", NULL };
static char *skill_v[] = {"Beginner","Novice","Average","Good","Expert","Master", NULL };
static char *color_v[] = {"1st player","2nd player",NULL};
static char *theme_v[] = {"simple","classic",NULL};
static char *fu_v[]    = {"no","yes",NULL};
static char *ndpl_v[]  = {"Pocket Book","human",NULL};

static iconfigedit pbchce[] = {
//int type;const ibitmap *icon;char *text;char *hint;char *name;char *deflt;char **variants;struct iconfigedit_s *submenu;
//{CFG_INFO,NULL               ,""          ,NULL      ,""        ,""         ,NULL           ,NULL},
  {CFG_INDEX,NULL             ,"Rules"       ,NULL     ,"ch.rules","0"        ,rules_v        ,NULL},
  {CFG_INDEX,NULL             ,"Skill"       ,NULL     ,"ch.skill","0"        ,skill_v        ,NULL},
  {CFG_INDEX,NULL             ,"First move",NULL     ,"ch.human","0"        ,color_v        ,NULL},
  {CFG_INDEX,NULL             ,"Theme"       ,NULL     ,"ch.theme","0"        ,theme_v        ,NULL},
  {CFG_INDEX,NULL             ,"Full Update" ,NULL     ,"ch.fu"   ,"0"        ,fu_v        ,NULL},
  {CFG_INDEX,NULL             ,"2nd player"  ,NULL     ,"ch.pl"   ,"0"        ,ndpl_v        ,NULL},
  {0}
}; 

class PBBoard : public CheckersBoard{
  int curs_x,curs_y;
  bool clicked;
  static const int bsize=8; //board size
  static const int csize=65;//cell size
  static const int xb=40;   // dx
  static const int yb=40;   // dy
  static int dibm;
protected:  
  ibitmap *mn1,*mn2,*kg1,*kg2;
public:
  virtual void set_checkers(){
    if( (Checkers::RUSSIAN==rules && is_white) || (Checkers::ENGLISH==rules && !is_white) ){
      if(theme==0){
      mn1 = &white_ms;
      mn2 = &black_ms;
      kg1 = &white_ks;
      kg2 = &black_ks;
      }else{
      mn1 = &white_m;
      mn2 = &black_m;
      kg1 = &white_k;
      kg2 = &black_k;
      }
    }else{
      if(theme==0){
      mn1 = &black_ms;
      mn2 = &white_ms;
      kg1 = &black_ks;
      kg2 = &white_ks;
      }else{
      mn1 = &black_m;
      mn2 = &white_m;
      kg1 = &black_k;
      kg2 = &white_k;
      }
    }
  }
  int theme;
  bool full;
  void cursor_up();
  void cursor_down();
  void cursor_left();
  void cursor_right();
  void click();
  int x2i(int i){
    int ret=(i-xb)/csize;
    if(ret <0 || ret>=8)return -1;
    return ret;
  }
  int y2j(int i){
    int ret=(i-yb)/csize;
    if(ret <0 || ret>=8)return -1;
    return ret;
  }
  void click(int i,int j){
    curs_x=i;
    curs_y=j;
    click();
  }
public:
  //
  void new_game(){
    CheckersBoard::new_game(ai2);
    clicked=false;
  }
  PBBoard():curs_x(0),curs_y(0),theme(0){
    set_checkers();
  }
  int ij_toind(int i,int j){
     return ((i+j)%2)?(i/2+4*j):-1;
  }
/*  void draw_cursor(int X,int Y,int W,int H ,int col){
    DrawRect(X,Y,W,H,col);
    DrawRect(X+1,Y+1,W-2,H-2,col);
  }*/
  void draw_checker(int X,int Y,ibitmap* bm){
    if(2!=theme)DrawBitmap(X,Y,bm);
    else {
      if(bm==&white_m){FillArea(X+8,Y+8,csize-16,csize-16,WHITE);FillArea(X+20,Y+20,csize-40,csize-40,BLACK);}
      if(bm==&black_m){FillArea(X+8,Y+8,csize-16,csize-16,BLACK);FillArea(X+20,Y+20,csize-40,csize-40,WHITE);}
      if(bm==&white_k){
        for(int i=0;i<4;++i)
        FillArea(X+8+i*4,Y+8+i*4,csize-16-i*8,csize-16-i*8,(i%2)?BLACK:WHITE);
      }
      if(bm==&black_k){//FillArea(X+8,Y+8,csize-16,csize-16,BLACK);FillArea(X+19,Y+20,csize-38,csize-40,WHITE);
        for(int i=0;i<4;++i)
        FillArea(X+8+i*4,Y+8+i*4,csize-16-i*8,csize-16-i*8,(i%2)?WHITE:BLACK);
      }
    }
  }
  void draw_item(int i,int j,int item,bool curs=false){
    int X=xb+i*csize;
    int Y=yb+j*csize;
    FillArea(X+3,Y+3,csize-6,csize-6,((i+j)%2)?LGRAY:WHITE);
    DrawRect(X,Y,csize,csize,BLACK);
    DrawRect(X+1,Y+1,csize-2,csize-2,(curs_x==i&&curs_y==j)?BLACK:WHITE);
    DrawRect(X+2,Y+2,csize-4,csize-4,(curs_x==i&&curs_y==j)?BLACK:WHITE);
    if((i+j)%2){
      if(cp==item){
        FillArea(X+5,Y+5,csize-10,csize-10,WHITE);
      }
      switch(game->item(item)) {
        case Checkers::MAN1:
        draw_checker(xb+i*csize+(csize-white_m.width)/2,yb+j*csize+(csize-white_m.height)/2,mn1);
            break;
            case Checkers::MAN2:
        draw_checker(xb+i*csize+(csize-white_m.width)/2,yb+j*csize+(csize-white_m.height)/2,mn2);
            break;
            case Checkers::KING1:
        draw_checker(xb+i*csize+(csize-white_m.width)/2,yb+j*csize+(csize-white_m.height)/2,kg1);
            break;
            case Checkers::KING2:
        draw_checker(xb+i*csize+(csize-white_m.width)/2,yb+j*csize+(csize-white_m.height)/2,kg2);
              break;
            default:
            ;
            }
    }
  }

  void draw(bool ds=true){
    if(ds){
      DrawRect(xb-1,yb-1,bsize*csize+2,bsize*csize+2,0x000000);
      char str[3];
      str[1]='\0';
      str[2]='\0';
      SetFont(boardf,0x000000);
      for(int i=0;i<bsize;++i){
        for(int j=0;j<bsize;++j){
          if(i==0){
            str[0]='1'+j;
            DrawString(xb-xb/2,yb+(bsize-j-1)*csize+csize/2-12,str);
          }
        }
        str[0]='A'+i;
        DrawString(xb+i*csize+csize/2-12,yb-yb/2-12,str);
      }
    }//ds
    //
    int item=0;
    for(int j=0;j<bsize;++j){
      for(int i=0;i<bsize;++i){
        if((i+j)%2){
          draw_item(i,j,item++);
        }else{
          draw_item(i,j,0);
        }
      }
    }
    //
    if(ds){
      char str2[512];
      sprintf(str2,"Game type:%s    Level:%s ",Checkers::RUSSIAN==rules?"Russian":"English",skill_v[Checkers::skill2index(skill)]);
      DrawString(xb,yb+8*csize,str2);
      if(game_over){
        if(winner>0){sprintf(str2,"Game Over! %s",(winner==1)?"Winner Computer":((winner==2)?"You Win!":"Nobody can move!"));
          DrawString(xb,yb+8.5*csize,str2);
        }
        sprintf(str2,"%s","Press Ok(or BACK) button for menu");
        DrawString(xb,yb+9*csize,str2);
      }
    }//ds
  }
};

PBBoard board;

void board_repaint(){
  ClearScreen();
  board.draw();
  if(board.game_over)FullUpdate();
  else SoftUpdate();
}

int PBBoard::dibm=(csize-white_m.width);

void PBBoard::cursor_down(){
  int co=curs_y;
  ++curs_y;
  if(curs_y==bsize)curs_y=0;
  draw_item(curs_x,co,ij_toind(curs_x,co));
  draw_item(curs_x,curs_y,ij_toind(curs_x,curs_y),true);
  if(curs_y==0)PartialUpdateA(xb+curs_x*csize,yb,csize,csize*bsize);
  else PartialUpdateA(xb+curs_x*csize,yb+(curs_y-1)*csize,csize,csize*2);
}
void PBBoard::cursor_up(){
  int co=curs_y;
  if(curs_y==0)curs_y=bsize;
  --curs_y;
  draw_item(curs_x,co,ij_toind(curs_x,co));
  draw_item(curs_x,curs_y,ij_toind(curs_x,curs_y),true);
  if(curs_y==bsize-1)PartialUpdateA(xb+curs_x*csize,yb,csize,csize*bsize);
  else PartialUpdateA(xb+curs_x*csize,yb+curs_y*csize,csize,csize*2);
}
void PBBoard::cursor_left(){
  int co=curs_x;
  if(curs_x==0)curs_x=bsize;
  --curs_x;
  draw_item(co,curs_y,ij_toind(co,curs_y));
  draw_item(curs_x,curs_y,ij_toind(curs_x,curs_y),true);
  if(curs_x==bsize-1)PartialUpdateA(xb,yb+curs_y*csize,csize*bsize,csize);
  else PartialUpdateA(xb+curs_x*csize,yb+curs_y*csize,csize*2,csize);
}
void PBBoard::cursor_right(){
  int co=curs_x;
  ++curs_x;
  if(curs_x==bsize)curs_x=0;
  draw_item(co,curs_y,ij_toind(co,curs_y));
  draw_item(curs_x,curs_y,ij_toind(curs_x,curs_y),true);
  if(curs_x==0)PartialUpdateA(xb,yb+curs_y*csize,csize*bsize,csize);
  else PartialUpdateA(xb+(curs_x-1)*csize,yb+curs_y*csize,csize*2,csize);
}
void PBBoard::click(){
  if(!clicked){
    clicked=true;
    int X=curs_x;
    int Y=curs_y;
    int gm_ind=((X+Y)%2)?(X/2+4*Y):-1;
    if(go1 && Checkers::FREE==game->item(gm_ind) && cp!=-1 && gm_ind !=-1 && game->go1(cp,gm_ind)){
      if(!check_win(false)){
        ClearScreen();
        draw();
        SoftUpdate();
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
        ClearScreen();
        draw();
        SoftUpdate();
        go1=true;
      }
    }else cp=gm_ind;
/*    if(Checkers::FREE==game->item(gm_ind) && cp!=-1 && gm_ind !=-1 && game->go1(cp,gm_ind)){
      if(!check_win(false)){
        //cerr<<"gmo:"<<game_over<<endl;
        ClearScreen();
        draw();
        SoftUpdate();
        //FineUpdate();
        //ShowHourglass();
        if(pl2nd==1)
        else game->go2();
        //HideHourglass();
        cp = -1;
        check_win(true);
        //cerr<<"gmo2:"<<game_over<<endl;
      }
    }
    else cp=gm_ind;*/
    ClearScreen();
    draw();
    if(board.full)FullUpdate();
    else SoftUpdate();
    if(game_over){
      FullUpdate();
    }
    clicked=false;
  }
}

#define msg(a) printf("%s\n",a);


int main_handler(int, int, int);
void menu1_handler(int index);
int cindex=0;

static imenu menu1[] = {

  { ITEM_HEADER,   0, "Menu", NULL },
  { ITEM_ACTIVE, 101, "New Game", NULL },
  { ITEM_SEPARATOR, 0, NULL, NULL },
  { ITEM_ACTIVE, 102, "Setup", NULL },
  { ITEM_SEPARATOR, 0, NULL, NULL },
  { ITEM_ACTIVE, 103, "Help", NULL },
  { ITEM_ACTIVE, 104, "About", NULL },
  { ITEM_SEPARATOR, 0, NULL, NULL },
  { ITEM_ACTIVE, 121, "Exit", NULL },
  { 0, 0, NULL, NULL }

};


int set_field_handler(int type, int par1, int par2);

void apply_config(){
  int get=ReadInt(pbchcfg,"ch.rules",0);
  board.rules=(0==get)?Checkers::RUSSIAN:Checkers::ENGLISH;
  get=ReadInt(pbchcfg,"ch.skill",0);
  board.skill=Checkers::index2skill(get);
  get=ReadInt(pbchcfg,"ch.human",0);
  board.is_white=(get==0);
  board.theme=ReadInt(pbchcfg,"ch.theme",0);
  board.set_checkers();
  board.full=ReadInt(pbchcfg,"ch.fu",0);
  board.ai2=ReadInt(pbchcfg,"ch.pl",0);
}

static void config_ok() {
  int ret=SaveConfig(pbchcfg);
  printf("seve config return %d file %s\n",ret,pbchcfg->filename);
  if(pbchcfg->changed){
    msg("config changed");
    apply_config();
  }
  ClearScreen();
  SendEvent(set_field_handler,EVT_SHOW,0,0);
}


void dialog_exit_handler(int button) {
  if(1 == button)CloseApp();
}


int set_field_handler(int type, int par1, int par2)
{
/*  if (type == EVT_INIT) {
    ClearScreen();
  }*/
  if ( EVT_SHOW == type ){
    board_repaint();
  }
  if (type == EVT_KEYPRESS || type == EVT_KEYREPEAT) {
  printf("t %d p1 %d p2 %d\n",type,par1,par2);
    switch (par1) {
    case KEY_MENU:
      OpenMenu(menu1, cindex, 20, 20, menu1_handler);
      return 1;
    case KEY_LEFT:
      board.cursor_left();
      break;
    case KEY_RIGHT:
      board.cursor_right();
      break;
    case KEY_UP:
      board.cursor_up();
      break;
    case KEY_DOWN:
      board.cursor_down();
      break;
    case KEY_OK:
      if(type == EVT_KEYREPEAT){
        OpenMenu(menu1, cindex, 20, 20, menu1_handler);
      }else{
        board.click();
        if(board.game_over)SetEventHandler(main_handler);
      }
      break;
    case KEY_BACK:
      if(type == EVT_KEYREPEAT){
        OpenMenu(menu1, cindex, 20, 20, menu1_handler);
      }
      break;
    }
    return 1;
    
  }
    if (type == EVT_POINTERDOWN ){
      int i = board.x2i(par1);
      if( i>=0 ){
	      int j = board.y2j(par2);
	      if( j>=0 ){
    	    board.click(i,j);
    	    if(board.game_over)SetEventHandler(main_handler);
    	    return 1;
	      }
      }
    }
  return 0;
}


void menu1_handler(int index) {
  cindex=index;
  switch (index) {
    case 101:
      board.new_game();
      SetEventHandler(set_field_handler);
      break;
    case 102:
      OpenConfigEditor("Configuration", pbchcfg, pbchce, config_ok, NULL);
      break;
    case 103:
      Message(ICON_INFORMATION, "Help", 
        "Checkers Game\n"
        "Support Russian and English checkers rules.\n"
        "In the beginning of game you have 12 checkers (men). "
        "The men move forward only. The men can capture:\n"
        "-by jumping forward only (english rules);\n"
        "-by jumping forward or backward (russian rules).\n"
        "A man which reaches the far side of the board becomes a king. "
        "The kings move forward or backward:\n"
        "-to one square only (english rules);\n"
        "-to any number of squares (russian rules).\n"
        "The kings capture by jumping forward or backward.\n"
        "Whenever a player is able to make a capture he must do so.\n"
        "Select checker by cursors button, press OK button, next select field to move and press OK."
        "If move is possible by rules your checkers will move.", 30000);    
      break;
    case 104:
      Message(ICON_INFORMATION, "About","Checkers game 1.2.0\n"
        "by Yury P. Fedorchenko.\n"
        "based on kcheckers.\n"
        "This is free sowtware and distributed under terms\n of "
        "GNU GPL License\n"
        "http://www.fedorchenko.net", 10000);    
      break;
    case 121:
      Dialog(ICON_QUESTION, "Quit", "Quit game?", "Yes", "No", dialog_exit_handler);
      break;
  }
  FullUpdate();
}


int main_handler(int type, int par1, int par2)
{
  if (type == EVT_INIT) {
    //CalibrateTouchpanel();
    boardf=OpenFont("cour",24,1);
    pbchcfg = OpenConfig(FLASHDIR "/pbchce.cfg", pbchce);
    apply_config();
  }

  if (type == EVT_SHOW) {
    ClearScreen();
    board_repaint();
  }

  if (type == EVT_KEYPRESS || type == EVT_KEYREPEAT ) {
    switch (par1) {

      case KEY_OK:
      case KEY_MENU:
      case KEY_BACK:
        msg("KEY_OK");
        OpenMenu(menu1, cindex, 20, 20, menu1_handler);
        return 1;

    }
  }
  if(type == EVT_POINTERDOWN){
    OpenMenu(menu1, cindex, 20, 20, menu1_handler);
    return 1;
  }

  if (type == EVT_EXIT) {
    // occurs only in main handler when exiting or when SIGINT received.
    // save configuration here, if needed
  }

  return 0;
}

int main(int argc, char **argv)
{
  InkViewMain(main_handler);
  return 0;
}


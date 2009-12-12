/*
 *   Copyright (C) 2009 Yury P. Fedorchenko
 *   yuryfdr@users.sf.net
 *   Copyright (C) 2002-2003 Andi Peredri                                  
 *   andi@ukr.net                                                          
 *   Copyright (C) 2004-2005 Artur Wiebe                                   
 *   wibix@gmx.de
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

#ifndef CHECKERS_H
#define CHECKERS_H

#ifndef NO_STL

#include <string>

#endif

class Checkers
{
public:
  // in some places used numbers instead of enums value - need fix
  // do not change - hard coded
  enum CheckerType {
    NONE=0,MAN1=1,KING1=2,FREE=3,KING2=4,MAN2=5,FULL=6
  };
  enum MoveDirection {
    UL=-6,UR=-5,DL=5,DR=6
  };
  enum GameRules {
    ENGLISH=21,RUSSIAN=25
  };
  enum GameSkill {
    BEGINNER=2,NOVICE=4,AVERAGE=6,GOOD=7,EXPERT=8,MASTER=9
  };
  Checkers();
  virtual ~Checkers() {}

  bool setup(int setupboard[]);// warn allways return true now
  // human side 
  virtual bool go1(int from, int to)=0;
  // human2 side
  virtual bool go2_human(int from, int to)=0;
  // ai side
  void go2();

  void setSkill(GameSkill i) { levelmax=i; };
  GameSkill skill() const { return (GameSkill)levelmax; }
  //conver index 0 - 5 to enum skill
  static GameSkill index2skill(int i){
    if(i>1)i+=4;
    else if(i==1)i=4;
    else i=2;
    return (GameSkill)i;
  }
  // convert skill to int 0 -5
  static int skill2index(GameSkill s){
    int skl = s;
    skl-=4;
    return (skl<0)?0:(skl==0)?1:skl;
  }
  
  virtual int type() const = 0;

  int item(int i) const { return board[internal(i)]; }
  void setItem(int i, int item) { board[internal(i)] = item; }

#ifndef NO_STL
  // string representation of the game board.
  // set rotate to switch player sides.
  // not used now need rewriten
  std::string toString(bool rotate) const;
  bool fromString(const std::string&);
#endif
  // checks for a capture/move for particular stone in external
  // representation. human player only.
  bool canCapture1(int i) { return checkCapture1(internal(i)); }
  bool canMove1(int i) { return checkMove1(internal(i)); }

  bool checkMove1() const;
  bool checkMove2() const;
  virtual bool checkCapture1() const = 0;
  virtual bool checkCapture2() const = 0;

protected:
  bool checkMove1(int) const;
  virtual bool checkCapture1(int) const = 0;

  int level;        // Current level
  int levelmax;     // Maximum level

  int  turn();
  void turn(int&, bool capture=false);

  int to;
  int board[54];
  int bestboard[54];
  int bestcounter;

  virtual void kingMove2(int,int &)=0;
  
  virtual bool manCapture2(int,int &)=0;
  virtual bool kingCapture2(int,int,int &)=0;

  virtual bool manCapture1(int,int,bool &)=0;
  virtual bool kingCapture1(int,int,bool &)=0;
  // for 2 player game
  virtual bool manCapture2(int,int,bool &)=0;
  virtual bool kingCapture2(int,int,bool &)=0;

  int internal(int) const;	// Return internal board position
};


#endif


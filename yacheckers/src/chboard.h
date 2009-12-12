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
#ifndef __checkers_board_h__
#define __checkers_board_h__

#include "rcheckers.h"
#include "echeckers.h"

class CheckersBoard {
public:
  Checkers* game;
  bool game_over;
  int  winner;
  // preferences
  int cp;
  Checkers::GameRules rules;
  bool is_white;
  Checkers::GameSkill skill;
  //
  bool ai2,go1;
protected:  
  bool check_win(bool human_move){
	  if(game_over) return true;
	  game_over = true;
	  bool player_can = game->checkMove1() || game->checkCapture1();
	  bool opp_can =    game->checkMove2() || game->checkCapture2();
	  // AI win
	  if(human_move && !player_can && opp_can){
	    winner=1;
	    return game_over;
	  }
	  // Human win
	  if(!human_move && player_can && !opp_can){
	    winner=2;
	    return game_over;
	  }
	  // Nobody cam move
	  if(!player_can && !opp_can) {
	    winner=3;
	    return game_over;
    }
	  game_over = false;
	  return game_over;
  };
  virtual void set_checkers() = 0;
public:
  // create new game
  void new_game(bool h2)
  {
    if(game) delete game;
    if(Checkers::RUSSIAN == rules ) game=new RCheckers();
    else game=new ECheckers();
    game->setSkill(skill);
    set_checkers();
    game_over = false;
    winner = 0;
    ai2=h2;
    if(!is_white){
      if(ai2){game->go2();go1=true;}
      else go1=false;
    }else go1=true;
    set_checkers();
  }
  
  CheckersBoard():game(NULL),game_over(true),cp(-1),rules(Checkers::RUSSIAN),
    is_white(true),skill(Checkers::BEGINNER){
    game=new RCheckers();
  }
  virtual ~CheckersBoard(){}
};

#endif // __checkers_board_h__


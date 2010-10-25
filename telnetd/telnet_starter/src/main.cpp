/**********************************************************************************************
    Copyright (C) 2010 Stephan Olbrich reader42@gmx.de

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

**********************************************************************************************/
//
// telnet_starter main.cpp
//

#include <inkview.h>
#include <stdio.h>

#include "MainView.h"

#define LOGFILE GAMEPATH "/telnet_starter.log"
#define DEFAULTTHEME SYSTEMTHEMESPATH "/default.pbt"

MainView *mview=0;

FILE * LogFile=0;

int main_handler(int type, int par1, int par2);

void quitHandler(int button)
{
  SetEventHandler(main_handler);
  if (button == 1)
  {
    CloseApp();
  }
  return;
}

int main_handler(int type, int par1, int par2)
{
  static int touch_x;
  static int touch_y;
  static int last_key;
  fprintf(LogFile, "main_handler(), type: %d\n", type);
  switch (type) {
    case EVT_INIT:
      fprintf(LogFile, "EVT_INIT\n");
//      OpenTheme(DEFAULTTHEME);
      mview = new MainView();
      mview->repaint();
      break;
    case EVT_SHOW:
      fprintf(LogFile, "EVT_SHOW\n");
      mview->repaint();
      break;
    case EVT_KEYPRESS:
      last_key = par1;
      break;
    case EVT_KEYRELEASE:
      fprintf(LogFile, "EVT_KEYRELEASE %d\n", par1);
      switch (par1) {
        case KEY_BACK:
          Dialog(2, "", "Do you really want to quit?", "Yes", "No", quitHandler);
          break;
        case KEY_PREV:
          break;
        case KEY_DOWN:
          break;
        case KEY_NEXT:
          break;
        case KEY_UP:
          break;
        case KEY_PREV2:
          break;
        case KEY_NEXT2:
          break;
      }
      break;
    case EVT_POINTERDOWN:
      fprintf(LogFile, "EVT_POINTERDOWN x=%d y=%d\n", par1, par2);
      touch_x = par1;
      touch_y = par2;
      break;
    case EVT_POINTERUP:
      fprintf(LogFile, "EVT_POINTERUP x=%d y=%d\n", par1, par2);
      if ( (abs(touch_x - par1) < 50) && (abs(touch_y - par2) < 50) )
      {
        int action;
        action = mview->hit(touch_x, touch_y);
        if ((action > 0) && (action == mview->hit(par1, par2)))
        {
          mview->highlight(action);
          switch (action) {
            case action_quit:
              Dialog(2, "", "Do you really want to quit?", "Yes", "No", quitHandler);
              break;
            case action_start_net:
              mview->NetworkConnect();
              break;
            case action_stop_net:
              mview->NetworkDisconnect();
              break;
            case action_start_telnet:
              mview->StartTelnetd();
              break;
            case action_stop_telnet:
              break;
          }
        }
      }
      break;
    case EVT_POINTERLONG:
      break;
  }
  return 0;
}

int main(int argc, char **argv) {
  LogFile = fopen(LOGFILE, "w");
  setbuf(LogFile, 0); // set buffer to 0 -> all data is written directly, so nothing is lost in case of a segfault
  fprintf(LogFile, "Start!\n");

  InkViewMain(main_handler);

  fclose(LogFile);
  return 0;
}

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
// telnet_starter MainView.h
//

#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <deque>
#include <inkview.h>

#define TELNETD "/mnt/ext1/games/utelnetd"

const int default_x_size = 200;
const int default_y_size = 75;

const int action_start_telnet = 1;
const int action_stop_telnet = 2;
const int action_start_net = 3;
const int action_stop_net = 4;
const int action_quit = 5;

class Button
{
  public:
    Button(char * text, int action, int x, int y, int sx = default_x_size, int sy = default_y_size);
    ~Button();
    void paint();
    int hit(int x, int y);
    bool operator==(const Button &other);
    bool operator!=(const Button &other);
    int button_action;
    void highlight();
    
  private:
    int pos_x;
    int pos_y;
    int size_x;
    int size_y;
    char * label;
};
  

class MainView
{
  public:
    MainView();
    ~MainView();
    void repaint();
    int hit(int x, int y);
    int NetworkConnect();
    int NetworkDisconnect();
    int StartTelnetd();
    void highlight(int action);
    
  private:
    char * defaultNetConnection();

    std::deque<Button> buttons;
    ifont * font;
};


#endif

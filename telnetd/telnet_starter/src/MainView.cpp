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
// telnet_starter MainView.cpp
//

#include <inkview.h>

#include "MainView.h"
#include <stdio.h>

extern FILE * LogFile;

Button::Button(char * text, int action, int x, int y, int sx, int sy)
{
  pos_x = x;
  pos_y = y;
  size_x = sx;
  size_y = sy;
  label = text;
  button_action = action;
  return;
}

Button::~Button()
{

}

void Button::paint()
{
  fprintf(LogFile, "Button->paint() %s\n", label);
  DrawRect(pos_x, pos_y, size_x, size_y, BLACK);
  DrawTextRect(pos_x, pos_y, size_x, size_y, label, ALIGN_CENTER | VALIGN_MIDDLE);
  return;
}

int Button::hit(int x, int y)
{
  fprintf(LogFile, "Button->hit() %s\n", label);
  if ( (x > pos_x) && (x < (pos_x + size_x)) && (y > pos_y) && (y < (pos_y + size_y)) )
  {
    fprintf(LogFile, "Button hit! %s\n", label);
    return button_action;
  }
  else
    return 0;
}

bool Button::operator==(const Button& other)
{
  if (button_action == other.button_action)
    return true;
  else
    return false;
}

bool Button::operator!=(const Button& other)
{
  return !(*this == other);
}

MainView::MainView()
{
  const int frame = 20;
  int w = (ScreenWidth() - 3*frame)/2;
  int h = (ScreenHeight() - 4*frame)/3;
  h = (h < default_y_size ? h : default_y_size);
  
  Button button1("Start Telnetd", action_start_telnet, frame, frame, w, h);
  Button button2("Stop Telnetd", action_stop_telnet, 2*frame + w, frame, w, h);
  Button button3("Start Network", action_start_net, frame, 2*frame + h, w, h);
  Button button4("Stop Network", action_stop_net, 2*frame + w, 2*frame + h, w, h);
  Button button5("Quit", action_quit, (3*frame + w)/2, 3*frame + 2*h, w, h);
  buttons.push_front(button1);
  buttons.push_front(button2);
  buttons.push_front(button3);
  buttons.push_front(button4);
  buttons.push_front(button5);
  font = OpenFont(DEFAULTFONT, 16, 1);
  SetFont(font, BLACK);
  return;
}

void Button::highlight()
{
  InvertAreaBW(pos_x, pos_y, size_x, size_y);
  PartialUpdateBW(pos_x, pos_y, size_x, size_y);
  InvertAreaBW(pos_x, pos_y, size_x, size_y);
  PartialUpdateBW(pos_x, pos_y, size_x, size_y);
}


MainView::~MainView()
{
  return;
}


void MainView::repaint()
{
  std::deque<Button>::iterator button_iterator;
  fprintf(LogFile, "MainView->repaint()\n");
  ClearScreen();		// Bildschirm löschen
  button_iterator = buttons.begin();
  while ( button_iterator != buttons.end() )
  {
    fprintf(LogFile, "MainView->repaint() loop\n");
    button_iterator->paint();
    button_iterator++;
  }
  fprintf(LogFile, "MainView->repaint() end\n");
  FullUpdate();
  return;
}

int MainView::hit(int x, int y)
{
  std::deque<Button>::reverse_iterator button_iterator;
  int action;
  fprintf(LogFile, "MainView->hit()\n");
  button_iterator = buttons.rbegin();
  while ( button_iterator != buttons.rend() )
  {
    action = button_iterator->hit(x,y);
    if (action > 0)
      return action;
    button_iterator++;
  }
  fprintf(LogFile, "MainView->hit() miss\n");
  return 0;
}

void MainView::highlight(int action)
{
  std::deque<Button>::iterator button_iterator;
  button_iterator = buttons.begin();
  while ( button_iterator != buttons.end() )
  {
    if (button_iterator->button_action == action)
    {
      button_iterator->highlight();;
      return;
    }
    button_iterator++;
  }
  return;
}


char* MainView::defaultNetConnection()
{
  iconfig * config;
  config = OpenConfig(NETWORKCONFIGFILE, NULL);
  return ReadString(config, "preferred", "default");
}

int MainView::NetworkConnect()
{
  if (!((QueryNetwork() & NET_WIFIREADY) || (QueryNetwork() & NET_BTREADY)))
  {
    fprintf(LogFile, "Connecting ... (%d)\n", QueryNetwork());
    return NetConnect(defaultNetConnection());
  }
  return 0;
}

int MainView::NetworkDisconnect()
{
  return NetDisconnect();
}

int MainView::StartTelnetd()
{
  int pid;
  fprintf(LogFile, "MainView->StartTelnetd()\n");
  if ((pid = fork()) < 0)
  {
    fprintf(LogFile, "Error forking\n");
    return -1;
  }
  if (pid == 0)
  {  /* this is the child */
    int uid = getuid();
    setgid(getgid());
    setuid(uid);
//    fclose(LogFile);
    execlp(TELNETD, TELNETD, "-p 10000", "-d", NULL);
    fprintf(LogFile, "Error execlp\n");
    exit(1);
  }
  fprintf(LogFile, "PID: %d\n", pid);
}

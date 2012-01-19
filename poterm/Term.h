/*
 * PoTerm: primitive xterm for PocketBook - Term class definitions
 * ---------------------------------------------------------------
 */

#ifndef TERM_H
#define TERM_H

#include <string>
#include <vector>
#include <map>

#include <stdio.h>

class Term
{
public:
    Term();
    ~Term();

    void command(const char *cmd);
    bool initialized() { return _initialized; }
    char *prompt();
    void check_output(int &was_read);
    void redraw();
    void redrawAll();
    void rotate();
    void lineUp()              { redraw_backward(_lastVisible - 1);  }
    void lineDown()            { redraw_forward(_firstVisible + 1);  }
    void pageUp()              { redraw_backward(_firstVisible - 1); }
    void pageDown()            { redraw_forward(_lastVisible + 1);   }
    void setHeightNoKbd(int h) { _heightNoKbd = h;                   }
    void log(const char *format, va_list args);
    void log(const char *format, ...) __attribute__ ((format (printf, 2, 3)));
    int  checkInterval();
    void decreaseFont();
    void increaseFont();

private:
    static void     trim(std::string &l);
    unsigned        toInt(const char *s, const char *k);
    void            output(const char c);
    void            readConfig();
    void            redraw_backward(int index);
    bool            redrawLine_backward(const std::string &line, int &y);
    void            redraw_forward(int index);
    bool            redrawLine_forward(const std::string &line, int &y);
    void            updateScroll();

    bool            _initialized;
    FILE            *_logf;
    int             _fromShell;
    int             _toShell;
    int             _width;
    int             _height;
    int             _offsx;
    int             _offsy;
    int             _scrollx;
    int             _scrolly;
    int             _scrollwidth;
    int             _scrollheight;
    ifont           *_blineFont;
    ifont           *_lineFont;
    int             _fontSize;
    int             _orientation;
    unsigned        _firstVisible;
    unsigned        _lastVisible;
    bool            _currVisible;
    int             _heightNoKbd;
    std::string                        _currLine;
    std::vector<std::string>           _lines;
  public:
    std::map<std::string, std::string> _config;
};

#endif

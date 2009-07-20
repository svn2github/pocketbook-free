/*
 * PoTerm: primitive xterm for PocketBook - Term class implementation
 * ------------------------------------------------------------------
 */
#include <iostream>
#include <fstream>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "inkview.h"
#include "Term.h"
#include "poterm.h"

using namespace std;

static int orientations[] = {0, 2, 3, 1};

static struct {
    const char *name;
    const char *value;
} defs[] = {
    { "BorderWidth",   "2"              },
    { "BorderColor",   "0xaaaaaa"       },
    { "BFontName",     "LiberationSans" },
    { "BFontSize",     "14"             },
    { "FontName",      "LiberationMono" },
    { "FontSize",      "16"             },
    { "LineGap",       "5"              },
    { "CheckInterval", "1000"           },
    { "ScrollBW",      "1"              },
    { "ScrollW",       "10"             },
    { "ScrollOutC",    "0xaaaaaa"       },
    { "ScrollBC",      "0x000000"       },
    { "ScrollInC",     "0x555555"       },
#if defined(HOST_X86)
    { "Shell",         "/bin/bash"      },
    { "LogFile",       "./poterm.log"   },
#elif defined(HOST_ARM)
    { "Shell",         "/bin/ash"       },
    { "LogFile",       "/mnt/ext1/games/poterm.log" },
#else
#error "Invalid architecture"
#endif
    { "Tab",           "8"              },
    { "Orientation",   "1"              }
};

//#define P(args...)      do { fprintf(stderr, args); fflush(stderr); } while (0)
#define P(args...)

#define BORDER_WIDTH    toInt(_config["BorderWidth"].c_str(), "BorderWidth")
#define BORDER_COLOR    toInt(_config["BorderColor"].c_str(), "BorderColor")
#define BFONT_NAME      _config["BFontName"].c_str()
#define BFONT_SIZE      toInt(_config["BFontSize"].c_str(), "BFontSize")
#define FONT_NAME       _config["FontName"].c_str()
#define FONT_SIZE       toInt(_config["FontSize"].c_str(), "FontSize")
#define LINE_GAP        toInt(_config["LineGap"].c_str(), "LineGap")
#define CHECK_INTERVAL  toInt(_config["CheckInterval"].c_str(), "CheckInterval")
#define TAB             toInt(_config["Tab"].c_str(), "Tab")
#define ORIENTATION     toInt(_config["Orientation"].c_str(), "Orientation")
#define SHELL           _config["Shell"].c_str()
#define LOG_FILE        _config["LogFile"].c_str()
#define SCROLL_BWIDTH   toInt(_config["ScrollBW"].c_str(), "ScrollBW")
#define SCROLL_WIDTH    toInt(_config["ScrollW"].c_str(), "ScrollW")
#define SCROLL_BCOLOR   toInt(_config["ScrollBC"].c_str(), "ScrollBC")
#define SCROLL_OUTCOLOR toInt(_config["ScrollOutC"].c_str(), "ScrollOutC")
#define SCROLL_INCOLOR  toInt(_config["ScrollInC"].c_str(), "ScrollInC")

int Term::checkInterval() { return CHECK_INTERVAL; }

///////////////////////////////////////////////////////////////////////
static int comm_pid;
static void catch_child()
{
    if (wait(0) == comm_pid)
        CloseApp();
} // catch_child

///////////////////////////////////////////////////////////////////////
static void catch_sig(int sig)
{
    signal(sig, SIG_DFL);
    setuid(getuid());
    kill(getpid(), sig);
} // catch_sig

///////////////////////////////////////////////////////////////////////
void Term::trim(string &l)
{
    while (!l.empty()  &&  isspace(l[l.size()-1]))
        l.erase(l.size()-1);
    while (!l.empty()  &&  isspace(l[0]))
        l.erase(0, 1);
} // trim

///////////////////////////////////////////////////////////////////////
unsigned Term::toInt(const char *s, const char *k)
{
    char     *endp;
    unsigned rc = strtol(s, &endp, 0);

    if (*endp)
    {
        log("---ERROR: \"%s\" is invalid format for \"%s\". Should be interger.\n", s, k);
        CloseApp();
    }
    return rc;
} // Term::toInt

///////////////////////////////////////////////////////////////////////
void Term::readConfig()
{
    // Init configuration
    for (unsigned i=0; i<sizeof(defs)/sizeof(defs[0]); i++)
        _config[defs[i].name] = defs[i].value;

    ifstream fp(INI_FILE);
    string   l;

    if (fp)
    {
        int lnumb = 0;
        while(getline(fp, l))
        {
            lnumb++;
            trim(l);
            if (l.empty()  ||  l[0] == '#')
                continue;
            string::size_type n = l.find(':');
            if (n == string::npos)
            {
                log("---ERROR: Init file \"%s\":%d - syntax error, should be at least one delimiter (;)\n",
                    INI_FILE, lnumb);
                continue;
            }
            string key = l.substr(0, n);
            string val = l.substr(n+1);
            trim(key);
            trim(val);
            _config[key] = val;
        }
    }
} // Term::readConfig

///////////////////////////////////////////////////////////////////////
Term::Term()
    : _initialized(false)
    , _logf(0)
    , _blineFont(0)
    , _lineFont(0)
    , _firstVisible(0)
    , _lastVisible(0)
    , _currVisible(false)

{
    readConfig();

    // Orienattion index
    _orientation = -1;
    for (unsigned i=0; i<sizeof(orientations)/sizeof(orientations[0]); i++)
        if (orientations[i] == (int)ORIENTATION)
        {
            _orientation = i;
            break;
        }
    if (_orientation == -1)
    {
        log("---ERROR: %d is invalid orientation\n", ORIENTATION);
        CloseApp();
    }
    SetOrientation(orientations[_orientation]);

    // Initial font size
    _fontSize = FONT_SIZE;

    /*
     * Open log file
     */
    _logf = fopen(LOG_FILE, "w");
    if (!_logf)
    {
        fprintf(stderr, "Can't open \"%s\" file for write: %s\n", LOG_FILE, strerror(errno));
        CloseApp();
    }

#if defined(PTY_SHELL)  &&  defined(PIPE_SHELL)
#error "PTY_SHELL and PIPE_SHELL are mutually exclusive"

#elif defined(PTY_SHELL)
    int         ptyfd, ttyfd;
    int         uid, gid;
    char        *s3, *s4;
    int         i;
    static char ptynam[] = "/dev/ptyxx";
    static char ttynam[] = "/dev/ttyxx";
    static char ptyc3[]  = "pqrstuvwxyz";
    static char ptyc4[]  = "0123456789abcdef";
    char *argv[2];
    argv[0] = (char *)SHELL;
    argv[1] = 0;

    /*  First find a master pty that we can open.
     */
    ptyfd = -1;
    for (s3 = ptyc3; *s3 != 0; s3++)
    {
        for (s4 = ptyc4; *s4 != 0; s4++)
        {
            ptynam[8] = ttynam[8] = *s3;
            ptynam[9] = ttynam[9] = *s4;
            if ((ptyfd = open(ptynam, O_RDWR)) >= 0) {
                if (geteuid() == 0 || access(ttynam, R_OK|W_OK) == 0)
                    break;
                else {
                    close(ptyfd);
                    ptyfd = -1;
                }
            }
            else
                log("Can't open \"%s\": %s\n", ptynam, strerror(errno));
        }
        if (ptyfd >= 0)
            break;
    }

    if (ptyfd < 0) {
        log("---ERROR: Can't open a pseudo teletype\n");
        return;
    }
    fcntl(ptyfd, F_SETFL, FNDELAY);

    for (i = 1; i <= 15; i++)
        signal(i, catch_sig);
    signal(SIGCHLD, (void (*)(int))catch_child);
    comm_pid = fork();
    if (comm_pid < 0)
    {
        log("---ERROR: Can't fork");
        return;
    }
    if (comm_pid == 0)
    {
        struct group *gr;

        if (setsid() < 0)
            log("---ERROR: failed to set process group\n");

        if ((ttyfd = open(ttynam,O_RDWR)) < 0)
        {
            log("---ERROR: could not open slave tty %s\n",ttynam);
            exit(1);
        }
        uid = getuid();
        if ((gr = getgrnam("tty")) != NULL)
            gid = gr->gr_gid;
        else
            gid = -1;
        fchown(ttyfd,uid, gid);
        fchmod(ttyfd, 0600);
        for (i = 0; i < getdtablesize(); i++)
            if (i != ttyfd)
                close(i);
        dup(ttyfd);
        dup(ttyfd);
        dup(ttyfd);
        if (ttyfd > 2)
            close(ttyfd);
        setgid(getgid());
        setuid(uid);
        execvp(SHELL, argv);
        log("---ERROR: Couldn't execute %s\n", SHELL);
        exit(1);
    }
    _fromShell = _toShell = ptyfd;

#elif defined(PIPE_SHELL)
    int  uid, fdes[2];

    for (int i = 1; i <= 15; i++)
        signal(i, catch_sig);
    signal(SIGCHLD, (void (*)(int))catch_child);

    if (pipe(fdes) < 0)
    {
        log("---ERROR: pipe - %s", strerror(errno));
        CloseApp();
    }
    int parent_inp = fdes[0];
    int child_out  = fdes[1];
    if (pipe(fdes) < 0)
    {
        log("---ERROR: second pipe - %s", strerror(errno));
        CloseApp();
    }
    int child_inp  = fdes[0];
    int parent_out = fdes[1];

    comm_pid = fork();
    if (comm_pid < 0)
    {
        log("---ERROR: Can't fork");
        return;
    }
    if (comm_pid == 0)
    {
        if (setsid() < 0)
            log("---ERROR: failed to set process group\n");

        uid = getuid();
        if (dup2(child_inp, 0) < 0)
        {
            log("---ERROR: dup2 (stdin): %s", strerror(errno));
            exit(1);
        }
        if (dup2(child_out, 1) < 0)
        {
            log("---ERROR: dup2 (stdout): %s", strerror(errno));
            exit(1);
        }
        if (dup2(child_out, 2) < 0)
        {
            log("---ERROR: dup2 (stderr): %s", strerror(errno));
            exit(1);
        }
        setgid(getgid());
        setuid(uid);
        for (int i = 3; i < getdtablesize(); i++)
            close(i);
        execlp(SHELL, SHELL, "-i", NULL);
        log("---ERROR: Couldn't execute %s\n", SHELL);
        exit(1);
    }
    _fromShell = parent_inp;
    _toShell   = parent_out;

#else
#error "PTY_SHELL or PIPE_SHELL should be defined"
#endif

    initBorders();

    _initialized = true;
} // Term::Term

///////////////////////////////////////////////////////////////////////
Term::~Term()
{
    close(_fromShell);
    close(_toShell);
    if (!_currLine.empty())
        log("%s\n", _currLine.c_str());
    fclose(_logf);
} // Term::~Term

///////////////////////////////////////////////////////////////////////
void Term::rotate()
{
    _orientation++;
    _orientation = _orientation % (sizeof(orientations)/sizeof(orientations[0]));
    SetOrientation(orientations[_orientation]);
    initBorders();
} // Term::rotate

///////////////////////////////////////////////////////////////////////
void Term::decreaseFont()
{
    int   newSize = _fontSize - 1;
    ifont *newFont = OpenFont((char *)FONT_NAME, newSize, 1);

    if (newFont)
    {
        _fontSize = newSize;
        _lineFont = newFont;
        redraw_forward(_firstVisible);
    }
} // Term::decreaseFont

///////////////////////////////////////////////////////////////////////
void Term::increaseFont()
{
    int   newSize = _fontSize + 1;
    ifont *newFont = OpenFont((char *)FONT_NAME, newSize, 1);

    if (newFont)
    {
        _fontSize = newSize;
        _lineFont = newFont;
        redraw_forward(_firstVisible);
    }
} // Term::increaseFont

///////////////////////////////////////////////////////////////////////
void Term::initBorders()
{
    ClearScreen();

    // Initialize screen parameters
    _width = ScreenWidth();
    _height = ScreenHeight();

    // Draw border
    FillArea(0, 0, _width, BORDER_WIDTH, BORDER_COLOR);
    FillArea(0, 0, BORDER_WIDTH, _height, BORDER_COLOR);
    FillArea(0, _height-BORDER_WIDTH, _width, BORDER_WIDTH, BORDER_COLOR);
    FillArea(_width-BORDER_WIDTH, 0,  BORDER_WIDTH, _height, BORDER_COLOR);
    _offsx = BORDER_WIDTH;
    _offsy = BORDER_WIDTH;
    _width -= 2*BORDER_WIDTH;
    _height -= 2*BORDER_WIDTH;

    // Draw botton line
    char text1[] = ": Command";
#ifndef DISABLE_ROTATION
    char text2[] = "Menu: Rotate";
#endif
    char text3[] = ": Page up/down";
    char text4[] = "<>: Line up/down";
    char text5[] = "+/-: Incr./Decr. font";
    if (!_blineFont)
        _blineFont = OpenFont((char *)BFONT_NAME, BFONT_SIZE, 0);
    if (!_blineFont)
    {
        log("---ERROR: Can't open font \"%s\", size: %d\n", (char *)BFONT_NAME, BFONT_SIZE);
        CloseApp();
    }
    SetFont(_blineFont, BLACK);
    int symbol_heigh = 16;
    int symbol_width = 12;
#ifndef DISABLE_ROTATION
    int gap = 15;
    int x = _offsx + (_width - (StringWidth(text1) + StringWidth(text2) + StringWidth(text3)
                                + StringWidth(text4) + StringWidth(text5)
                                + 4*gap + 2*symbol_width )) /2;
#else
    int gap = 30;
    int x = _offsx + (_width - (StringWidth(text1) + StringWidth(text3) + StringWidth(text4)
                                + StringWidth(text5)
                                + 3*gap + 2*symbol_width )) /2;
#endif
    int h = TextRectHeight(StringWidth(text1), text1, 0);
    FillArea(_offsx, _offsy + _height - h, _width, h, LGRAY);
    DrawSymbol(x, _offsy + _height-symbol_heigh, SYMBOL_OK);
    x += symbol_width;
    DrawString(x, _offsy + _height - h, text1);
    x += StringWidth(text1) + gap;
#ifndef DISABLE_ROTATION
    DrawString(x, _offsy + _height - h, text2);
    x += StringWidth(text2) + gap;
#endif
    DrawSymbol(x, _offsy + _height-symbol_heigh, ARROW_UPDOWN);
    x += symbol_width;
    DrawString(x, _offsy + _height - h, text3);
    x += StringWidth(text3) + gap;
    DrawString(x, _offsy + _height - h, text4);
    x += StringWidth(text4) + gap;
    DrawString(x, _offsy + _height - h, text5);
    x += StringWidth(text5) + gap;
    _height -= h;

    // Draw scrollbar
    _width += BORDER_WIDTH;
    FillArea(_offsx + _width-SCROLL_BWIDTH, _offsy,  SCROLL_BWIDTH, _height, SCROLL_BCOLOR);
    _width -= SCROLL_BWIDTH;
    FillArea(_offsx + _width-SCROLL_WIDTH, _offsy,  SCROLL_WIDTH, _height, SCROLL_OUTCOLOR);
    _scrollx = _offsx + _width-SCROLL_WIDTH;
    _scrolly = _offsy;
    _scrollwidth = SCROLL_WIDTH;
    _scrollheight = _height;
    _width -= SCROLL_WIDTH;
    FillArea(_offsx + _width-SCROLL_BWIDTH, _offsy,  SCROLL_BWIDTH, _height, SCROLL_BCOLOR);
    _width -= SCROLL_BWIDTH;

    if (!_lineFont)
        _lineFont = OpenFont((char *)FONT_NAME, _fontSize, 1);
    if (!_lineFont)
    {
        log("---ERROR: Can't open font \"%s\", size: %d\n", (char *)FONT_NAME, FONT_SIZE);
        CloseApp();
    }

    redraw();
} // Term::initBorders

///////////////////////////////////////////////////////////////////////
void Term::updateScroll()
{
    unsigned total = _lines.size() + (_currLine.empty() ? 0 : 1);
    if (total <= 0  || (
            _firstVisible <= 0 && _lastVisible >= _lines.size()-1 && _currVisible)
        )
    {
        FillArea(_scrollx, _scrolly,  _scrollwidth, _scrollheight, SCROLL_INCOLOR);
        return;
    }

    int   lastVisible = _currVisible ? _lastVisible+1 : _lastVisible;
    float pixelsPerLine = (float)_scrollheight / total;
    int   visStart = (int)(pixelsPerLine * _firstVisible + 0.5);
    int   visHeight = (int)(pixelsPerLine * (lastVisible - _firstVisible) + 0.5);

    FillArea(_scrollx, _scrolly,  _scrollwidth, _scrollheight, SCROLL_OUTCOLOR);
    FillArea(_scrollx, _scrolly + visStart, _scrollwidth, visHeight, SCROLL_INCOLOR);
} // Term::updateScroll

///////////////////////////////////////////////////////////////////////
void Term::check_output(int &was_read)
{
    static const int BUFLEN = 1024;
    static char      buf[BUFLEN];
    int              cnt, nread, readnow;

    was_read=0;
    for (;;)
    {
        if (ioctl(_fromShell, FIONREAD, &nread) < 0)
        {
            log("---ERROR: ioctl on shell pipe - %s\n", strerror(errno));
            return;
        }
        if (nread <= 0)
            return;
        do
        {
            readnow = nread > BUFLEN ? BUFLEN : nread;
            do
                cnt = read(_fromShell, buf, readnow);
            while (cnt < 0  &&  errno == EAGAIN);
            if (cnt != readnow)
            {
                log("---ERROR: read from shell pipe - %s\n", strerror(errno));
                return;
            }
            for (int i=0; i<readnow; i++)
            {
                was_read++;
                output(buf[i]);
            }
            nread -= readnow;
        } while (nread > 0);
    }
} // Term::check_output

///////////////////////////////////////////////////////////////////////
void Term::output(const char c)
{
    switch (c)
    {
        case '\r':
            _currLine.clear();
            break;
        case '\b':
            if (!_currLine.empty())
                _currLine.erase(_currLine.size()-1);
            break;
        case '\t':
        {
            int n = TAB - (_currLine.size() % TAB);
            for (int i=0; i<n; i++)
                _currLine += ' ';
            break;
        }
        case '\n':
            _lines.push_back(_currLine);
            log("%s\n", _currLine.c_str());
            _currLine.clear();
            break;
        default:
            _currLine += c;
    }
} // Term::output

///////////////////////////////////////////////////////////////////////
void Term::log(const char *format, va_list args)
{
    vfprintf(_logf, format, args);
} // Term::log

///////////////////////////////////////////////////////////////////////
void Term::log(const char *format, ...)
{
    va_list   args;

    va_start(args, format);
    log(format, args);
    va_end(args);
} // Term::log

///////////////////////////////////////////////////////////////////////
void Term::command(const char *cmd)
{
    write(_toShell, cmd, strlen(cmd));
    write(_toShell, "\n", 1);
    while (*cmd)
        output(*cmd++);
    output('\n');
} // Term::command

///////////////////////////////////////////////////////////////////////
char *Term::prompt()
{
    char *rc;
    if (_currLine.empty())
        rc = (char *)"Next command";
    else
        rc = (char *)_currLine.c_str();
    return rc;
} // Term::prompt

///////////////////////////////////////////////////////////////////////
void Term::redraw()
{
    SetFont(_lineFont, BLACK);
    FillArea(_offsx, _offsy, _width, _height, WHITE);

    int y = _height;
    int i = (int)_lines.size()-1;
    bool rc = _currLine.empty() ? true : redrawLine_backward(_currLine, y);
    if (rc)
    {
        for (; i >= 0; i--)
            if (!redrawLine_backward(_lines[i], y))
                break;
    }

    _currVisible = true;
    _firstVisible = i < 0 ? 0 : (i + 1);
    _lastVisible = _lines.size() > 0 ? _lines.size()-1 : 0;
    updateScroll();

    P("Term::redraw size:%d, capacity:%d _firstVisible:%d _lastVisible:%d _currVisible:%d\n",
      _lines.size(), _lines.capacity(), _firstVisible, _lastVisible, _currVisible);
    FullUpdate();
} // Term::redraw

///////////////////////////////////////////////////////////////////////
void Term::redraw_backward(int ind)
{
    if (ind <= 0)
        return;

    SetFont(_lineFont, BLACK);
    FillArea(_offsx, _offsy, _width, _height, WHITE);

    int y = _height;
    int i = ind;
    for (; i >= 0; i--)
        if (!redrawLine_backward(_lines[i], y))
            break;

    _currVisible = false;
    _firstVisible = i < 0 ? 0 : (i + 1);
    _lastVisible = ind;
    updateScroll();

    P("Term::redraw_backward(%d) size:%d, capacity:%d _firstVisible:%d _lastVisible:%d _currVisible:%d\n",
      ind, _lines.size(), _lines.capacity(), _firstVisible, _lastVisible, _currVisible);
    FullUpdate();
} // Term::redraw_backward

///////////////////////////////////////////////////////////////////////
void Term::redraw_forward(int ind)
{
    if (ind >= (int)_lines.size())
        return;

    SetFont(_lineFont, BLACK);
    FillArea(_offsx, _offsy, _width, _height, WHITE);

    int      y = _offsy;
    unsigned i = ind;
    for (; i < _lines.size(); i++)
        if (!redrawLine_forward(_lines[i], y))
            break;
    if (i >= _lines.size()  &&  !_currLine.empty())
        _currVisible = redrawLine_forward(_currLine, y);
    else
        _currVisible = false;

    _firstVisible = ind;
    _lastVisible = i < _lines.size() ? i : _lines.size()-1;
    updateScroll();

    P("Term::redraw_forward(%d) size:%d, capacity:%d _firstVisible:%d _lastVisible:%d _currVisible:%d\n",
      ind, _lines.size(), _lines.capacity(), _firstVisible, _lastVisible, _currVisible);
    FullUpdate();
} // Term::redraw_forward

///////////////////////////////////////////////////////////////////////
bool Term::redrawLine_backward(const string &line, int &y)
{
    if (y < _offsy)
        return false;
    if (StringWidth((char *)line.c_str()) > _width)
    {
        unsigned n = line.size()-1;
        for (; n && StringWidth((char *)line.substr(0, n).c_str()) > _width; n--);
        if (!redrawLine_backward(line.substr(n), y))
            return false;
        return redrawLine_backward(line.substr(0, n), y);
    }
    else
    {
        y -= TextRectHeight(StringWidth((char *)line.c_str()), (char *)line.c_str(), 0);
        if (y < _offsy)
            return false;
        DrawString(_offsx, y, (char *)line.c_str());
        y -= LINE_GAP;
        return true;
    }
} // Term::redrawLine_backward

///////////////////////////////////////////////////////////////////////
bool Term::redrawLine_forward(const string &line, int &y)
{
    if (y >= _height)
        return false;
    if (StringWidth((char *)line.c_str()) > _width)
    {
        unsigned n = line.size()-1;
        for (; n && StringWidth((char *)line.substr(0, n).c_str()) > _width; n--);
        if (!redrawLine_forward(line.substr(0, n), y))
            return false;
        return redrawLine_forward(line.substr(n), y);
    }
    else
    {
        int n = TextRectHeight(StringWidth((char *)line.c_str()), (char *)line.c_str(), 0);
        if (y+n > _height)
            return false;
        DrawString(_offsx, y, (char *)line.c_str());
        y += n + LINE_GAP;
        return true;
    }
} // Term::redrawLine_forward

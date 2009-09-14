import sys
import time
import inkview
from inkview import *
from traceback import print_exc

#TODO: Generate python arrays
#~ extern const ibitmap background, books, m3x3
background = cvar.background
books = cvar.books
m3x3 = cvar.m3x3
#~ extern const ibitmap item1, item2
item1 = cvar.item1
item2 = cvar.item2


sometext = ("Portability Concerns\n\n" 
    + "Of course, there are quite a few concerns that need to be thought about when considering bringing a program built for the desktop over to an embedded environment.\n"
    + "12345 67890")

#char[36]
kbuffer = "Some text"

#char[1024]
dirbuf = ""

#tocentry
contents = [
    [ 1,  1,   1,  "Annotation" ],
    [ 1,  5,   5,  "Chapter 1" ],
    [ 2, 10,  10,  "Level 2 chapter 1" ],
    [ 2, 12,  12,  "Level 2 chapter 2" ],
    [ 2, 15,  15,  "Level 2 chapter 3" ],
    [ 3, 15,  15,  "Level 3 chapter 1 ejfhew wefdhwehwehwejh wehfdjkwehfkjwehkjf wenkjf nwekj" ],
    [ 3, 17,  17,  "Level 3 chapter 2 ejfhew wefdhwehwehwejh wehfdjkwehfkjwehkjf wenkjf nwekj" ],
    [ 3, 20,  20,  "Level 3 chapter 3 ejfhew wefdhwehwehwejh wehfdjkwehfkjwehkjf wenkjf nwekj" ],
    [ 1, 25,  25,  "Chapter 2" ],
    [ 2, 30,  30,  "C2 level 2 chapter 1" ],
    [ 3, 34,  34,  "C2 level 3 chapter 1" ],
    [ 4, 37,  37,  "C2 level 4 chapter 1" ],
    [ 4, 38,  38,  "C2 level 4 chapter 2" ],
    [ 3, 41,  41,  "C2 level 3 chapter 2" ],
    [ 3, 45,  45,  "C2 level 3 chapter 3" ],
    [ 2, 51,  51,  "index 1" ],
    [ 2, 55,  55,  "index 2" ],
    [ 2, 60,  60,  "index 3" ],
    [ 1, 102, 102,  "Chapter 3" ],
    [ 1, 105, 105,  "Chapter 4" ],
    [ 1, 110, 110,  "Chapter 5" ],
    [ 1, 115, 115,  "Chapter 6" ],
    [ 1, 130, 130,  "Chapter 7" ],
    [ 1, 150, 150,  "Chapter 8" ],
    [ 1, 200, 200,  "Chapter 9" ],
    [ 1, 250, 250,  "Chapter 10" ],
    [ 1, 300, 300,  "Chapter 11" ],
    [ 1, 350, 350,  "Chapter 12" ],
    [ 1, 400, 400,  "Chapter 13" ],
    [ 1, 410, 410,  "Chapter 14" ],
    [ 1, 420, 420,  "Chapter 15" ],
    [ 1, 430, 430,  "Chapter 16" ],
    [ 1, 440, 440,  "Chapter 17" ],
    [ 2, 445, 445,  "Ch 17 level 2 1" ],
    [ 2, 446, 446,  "Ch 17 level 2 2" ],
    [ 1, 450, 450,  "Chapter 18" ],
    [ 1, 460, 460,  "Chapter 19" ],
    [ 1, 470, 470,  "Chapter 20" ],
    [ 1,  70,  70,  "Conclusion" ],
    [ 1,  90,  90,  "End" ],
    [ 1,  95,  95,  "Whole end" ],
    [ 1,  12345,12345,  "Full end" ],
]

#iconfig 
testcfg = None

choice_variants = [ "qqq", "www", "@Contents", "rrr", None ]
choice_variants2 = [ "q1", "q2", "q3", "q4", "w1", "w2", "w3", "w4", "e1", "e2", "e3", "e4", "r1", "r2", "r3", "r4", "t1", "t2", "t3", "t4", None ]
choice_variants3 = [ "q1", "q2", "q3", "q4", "w's", ":w1", ":w2", ":w3", ":w4", "e1", "e2", "e3", "e4", "r1", "r2", "r3", "r4", "t1", "t2", "t3", "t4", None ]
index_variants = [ "value1", "value2", "value3", None ]

#oldconfigedit  -???
#~ testce = [
    #~ ["About device", None, CFG_INFO, "Name: PocketBook 310\nSerial: 123456789", None],
    #~ [ "Text edit", "param.textedit", CFG_TEXT, "qwerty", None ],
    #~ [ "Choice", "param.choice", CFG_CHOICE, "eee", choice_variants ],
    #~ [ "Many variants", "param.choice2", CFG_CHOICE, "qqq", choice_variants2 ],
    #~ [ "Multi-level", "param.choice3", CFG_CHOICE, "cvb", choice_variants3 ],
    #~ [ "Font", "param.font", CFG_FONT, "Arial,24", None ],
    #~ [ "Font face", "param.fontface", CFG_FONTFACE, "Arial", None ],
    #~ [ "Index", "param.index", CFG_INDEX, "2", index_variants ],
    #~ [ "Time", "param.time", CFG_TIME, "1212396151", None ],
    #~ [ None, None, 0, None, None]
#~ ]

#iconfigedit 
testce2 = [
    [ CFG_INFO, item1, "About device", "Information about electronic book", None, None, None, None ],
    [ CFG_TEXT, item2, "Text edit", "Example of text edit control", "param.textedit", "qwerty", None, None ],
    [ 0, None, None, None, None, None, None]
]

cindex = 0
orient = 0
#timer_on = 0

current_page = 5

#int[25]
bmklist = [ 1, 12, 25, 35, 79 ]
# long long[25]
poslist = [ 1, 12, 25, 35, 79 ]

bmkcount = 5

arial8n = None
arial12 = None
arialb12 = None
cour16 = None
cour24 = None
times20 = None

langs = []
themes = []

strings3x3 = [
    "Exit",
    "Open page",
    "Search...",
    "Goto",
    "Menu",
    "Quote",
    "Thumbnails",
    "Settings",
    "Something else"
]

#imenu[16] 
menu1_lang = [] 

#imenu[16]
menu1_theme = []

#imenu 
menu1_notes = [
    [ ITEM_ACTIVE, 119, "Create note", None ],
    [ ITEM_ACTIVE, 120, "View notes", None ],
]

#imenu 
menu1 = [
    [ ITEM_HEADER,   0, "Menu", None ],
    [ ITEM_ACTIVE, 101, "Screens", None ],
    [ ITEM_ACTIVE, 102, "Rotate", None ],
    [ ITEM_ACTIVE, 103, "TextRect", None ],
    [ ITEM_ACTIVE, 104, "Message", None ],
    [ ITEM_ACTIVE, 105, "Dialog", None ],
#    [ ITEM_ACTIVE, 106, "Timer", None ],
    [ ITEM_ACTIVE, 107, "Menu3x3", None ],
    [ ITEM_ACTIVE, 108, "Hourglass", None ],
    [ ITEM_ACTIVE, 111, "Keyboard", None ],
    [ ITEM_ACTIVE, 112, "Configuration", None ],
    [ ITEM_ACTIVE, 113, "Page selector", None ],
    [ ITEM_ACTIVE, 116, "Bookmarks", None ],
    [ ITEM_ACTIVE, 117, "Contents", None ],
    [ ITEM_ACTIVE, 114, "Dither", None ],
    [ ITEM_ACTIVE, 115, "16-grays", None ],
    [ ITEM_ACTIVE, 125, "Dirselector", None ],
    [ ITEM_SUBMENU, 0, "Notes", menu1_notes ],
    [ ITEM_SEPARATOR, 0, None, None ],
    [ ITEM_SUBMENU, 0, "Language", menu1_lang ],
    [ ITEM_SUBMENU, 0, "Theme", menu1_theme ],
    [ ITEM_SEPARATOR, 0, None, None ],
    [ ITEM_ACTIVE, 121, "Exit", None ],
]

#unsigned char 
pic_example = [
    255,255,255,  0,  0,  0,  0,255,255,255,
    255,170,  0,255,255,255,255,  0,170,255,
    255,  0,255,255,255,255,255,255,  0,255,
      0,255,255,170,170,170,170,255,255,  0,
      0,255,255,170, 80, 80,170,255,255,  0,
      0,255,255,170, 80, 80,170,255,255,  0,
      0,255,255,170,170,170,170,255,255,  0,
    255,  0,255,255,255,255,255,255,  0,255,
    255,170,  0,255,255,255,255,  0,170,255,
    255,255,255,  0,  0,  0,  0,255,255,255,
]


def msg(s):
    FillArea(350, 770, 250, 20, WHITE)
    SetFont(arialb12, BLACK)
    DrawString(350, 770, s)
    PartialUpdate(350, 770, 250, 20)

def mainscreen_repaint():
    ClearScreen()

    SetFont(arialb12, BLACK)
    DrawString(5, 2, "InkView library demo, press OK to open menu")

    DrawBitmap(0, 20, background)
    DrawBitmap(120, 30, books)

    DrawLine(5, 500, 595, 500, BLACK)
    DrawLine(5, 502, 595, 502, DGRAY)
    DrawLine(5, 504, 595, 504, LGRAY)
    DrawLine(19, 516, 20, 517, BLACK)
    DrawLine(22, 516, 23, 516, BLACK)

    for i in range(5, 595, 3):
        DrawPixel(i, 507, BLACK)

    DrawRect(5, 510, 590, 10, BLACK)

    for i in range(0, 256):
        FillArea(35+i*2, 524, 2, 12, i | (i << 8) | (i << 16))

    b = BitmapFromScreen(0, 520, 600, 20)
    DrawBitmap(0, 550, b)
    del b
    InvertArea(0, 550, 600, 20)
    DimArea(0, 575, 600, 10, BLACK)

    if not orient:
        Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 10, 600,  6,  6, 0)
        Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 20, 600, 10,  6, 0)
        Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 35, 600, 30,  6, 0)
        Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 10, 610,  6, 10, 0)
        Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 20, 610, 10, 10, 0)
        Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 35, 610, 30, 10, 0)
        Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 10, 625,  6, 30, 0)
        Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 20, 625, 10, 30, 0)
        Stretch(pic_example, IMAGE_GRAY8, 10, 10, 10, 35, 625, 30, 30, 0)

    SetFont(arial8n, BLACK)
    DrawString(350, 600, "Arial 8 with no antialiasing")
    SetFont(arial12, BLACK)
    DrawString(350, 615, "Arial 12 regular")
    SetFont(arialb12, BLACK)
    DrawString(350, 630, "Arial 12 bold")
    SetFont(cour16, BLACK)
    DrawString(350, 645, "Courier 16")
    SetFont(cour24, BLACK)
    DrawString(350, 660, "Courier 24")
    DrawSymbol(500, 660, ARROW_LEFT)
    DrawSymbol(520, 660, ARROW_RIGHT)
    DrawSymbol(540, 660, ARROW_UP)
    DrawSymbol(560, 660, ARROW_DOWN)
    SetFont(times20, BLACK)
    DrawString(350, 680, "Times 20")
    DrawSymbol(450, 680, ARROW_LEFT)
    DrawSymbol(470, 680, ARROW_RIGHT)
    DrawSymbol(490, 680, ARROW_UP)
    DrawSymbol(510, 680, ARROW_DOWN)

    #DrawTextRect(25, 400, 510, 350, sometext, ALIGN_LEFT)

    FullUpdate()

def example_textrect():
    ClearScreen()
    SetFont(arial12, BLACK)

    DrawRect(5, 5, 190, 190, LGRAY)
    DrawTextRect(5, 5, 190, 190, sometext, ALIGN_LEFT | VALIGN_TOP)
    DrawRect(200, 5, 190, 190, LGRAY)
    DrawTextRect(200, 5, 190, 190, sometext, ALIGN_CENTER | VALIGN_TOP)
    DrawRect(395, 5, 190, 190, LGRAY)
    DrawTextRect(395, 5, 190, 190, sometext, ALIGN_RIGHT | VALIGN_TOP)

    DrawRect(5, 200, 190, 190, LGRAY)
    DrawTextRect(5, 200, 190, 190, sometext, ALIGN_LEFT | VALIGN_MIDDLE)
    DrawRect(200, 200, 190, 190, LGRAY)
    DrawTextRect(200, 200, 190, 190, sometext, ALIGN_CENTER | VALIGN_MIDDLE)
    DrawRect(395, 200, 190, 190, LGRAY)
    DrawTextRect(395, 200, 190, 190, sometext, ALIGN_RIGHT | VALIGN_MIDDLE)

    DrawRect(5, 395, 190, 190, LGRAY)
    DrawTextRect(5, 395, 190, 190, sometext, ALIGN_LEFT | VALIGN_BOTTOM)
    DrawRect(200, 395, 190, 190, LGRAY)
    DrawTextRect(200, 395, 190, 190, sometext, ALIGN_FIT | VALIGN_BOTTOM)
    DrawRect(395, 395, 190, 190, LGRAY)
    DrawTextRect(395, 395, 190, 190, sometext, ALIGN_RIGHT | VALIGN_BOTTOM)

    DrawRect(5, 590, 190, 190, LGRAY)
    DrawTextRect(5, 590, 190, 190, sometext, ALIGN_LEFT | VALIGN_TOP | ROTATE)
    DrawRect(200, 590, 190, 190, LGRAY)
    DrawTextRect(200, 590, 190, 190, sometext, ALIGN_CENTER | VALIGN_MIDDLE | ROTATE)
    DrawRect(395, 590, 100, 100, LGRAY)
    DrawTextRect(395, 590, 100, 100, sometext, DOTS)

    FullUpdate()

def example_dither():
    ClearScreen()
    SetFont(arial12, BLACK)
    DrawTextRect( 20,  10, 256, 15, "No dithering", ALIGN_CENTER)
    DrawTextRect(300,  10, 256, 15, "Threshold (BW)", ALIGN_CENTER)
    DrawTextRect( 20, 300, 256, 15, "Pattern", ALIGN_CENTER)
    DrawTextRect(300, 300, 256, 15, "Diffusion", ALIGN_CENTER)
    for y in range(0, 256):
        for x in range(0, 256):
            c = (x + y) // 2
            DrawPixel(20+x, 40+y, c | (c<<8) | (c<<16))
            DrawPixel(300+x, 40+y, c | (c<<8) | (c<<16))
            DrawPixel(20+x, 320+y, c | (c<<8) | (c<<16))
            DrawPixel(300+x, 320+y, c | (c<<8) | (c<<16))
    DitherArea(300, 40, 256, 256, 2, DITHER_THRESHOLD)
    DitherArea(20, 320, 256, 256, 4, DITHER_PATTERN)
    DitherArea(300, 320, 256, 256, 4, DITHER_DIFFUSION)
    FullUpdate()

def dialog_handler(button):
    if button == 1:
        msg("Choosed: yes")
    else:
        msg("Choosed: no")

def menu3x3_handler(pos):
    msg("Menu: %d" % (pos))

def page_selected(page):
    global current_page
    current_page = page
    msg("Page: %d" % (page))

#timer_value = 0
#def timer_proc():
#    global timer_value
#    timer_value += 1
#    msg("Timer: %d" % (value))
#    SetHardTimer("MYTIMER", timer_proc, 1000)

def keyboard_entry(s):
    msg("Entered text: %s" % (s))
    global kbuffer
    kbuffer = s
    
def config_ok():
    SaveConfig(testcfg)

def print_bmk():
    for i in range(0, bmkcount):
        print("%d %d" % (bmklist[i], poslist[i]))
    print()

def bmk_added(page, pos):
    msg("Added bookmark: page %d, pos %d" % (page, pos))
    print_bmk()
    mainscreen_repaint()

def bmk_removed(page, pos):
    msg("Removed bookmark: page %d, pos %d" % (page, pos))
    print_bmk()
    mainscreen_repaint()

def bmk_selected(page, pos):
    msg("Selected bookmark: page %d, pos %d" % (page, pos))

def bmk_handler(action, page, pos):
    if action == BMK_ADDED: 
        bmk_added(page, pos) 
    elif action == BMK_REMOVED: 
        bmk_removed(page, pos) 
    elif action == BMK_SELECTED: 
        bmk_selected(page, pos) 

def dir_selected(path):
    print(file=sys.stderr)
    print(":%s" %(path), file=sys.stderr)
    global dirbuf
    dirbuf = path
    
def menu1_handler(index):
    global cindex
    cindex = index
    
    if index > 400 and index <= 499:
        LoadLanguage(langs[index-400])
        mainscreen_repaint()
    
    if index == 101:
        SetEventHandler(screen2_handler)
    elif index == 102:
        global orient
        orient += 1
        if orient > 2:
            orient = 0
        SetOrientation(orient)
        mainscreen_repaint()
    elif index == 103:
        example_textrect()
    elif index == 104:
        Message(ICON_INFORMATION, "Message", "This is a message.\n"
            "It will disappear after 5 seconds, or press any key", 5000)
    elif index == 105:
        Dialog(ICON_QUESTION, "Dialog", "This is a dialog.\n"
            "Do you like it?", "Yes", "No", dialog_handler)
#    elif index == 106:
#        global time_on
#        if timer_on:
#            ClearTimer(timer_proc)
#            timer_on = 0
#        else:
#            SetHardTimer("MYTIMER", timer_proc, 0)
#            timer_on = 1
    elif index == 107:
        OpenMenu3x3(m3x3, strings3x3, menu3x3_handler)
    elif index == 108:
        ShowHourglass()
        time.sleep(2)
        HideHourglass()
    elif index == 111:
        global kbuffer
        OpenKeyboard("Keyboard test", kbuffer, 35, 0, keyboard_entry)
    elif index == 112:
        OpenConfigEditor("Configuration", testcfg, testce2, config_ok, None)
    elif index == 113:
        OpenPageSelector(page_selected)
    elif index == 114:
        example_dither()
    elif index == 115:
        ClearScreen()
        for i in range(0, 256):
            FillArea(10+i*2, 10, i*2, 25, i | (i<<8) | (i<<16))
        FullUpdate()
        FineUpdate()
    elif index == 116:
        for i in range(0, bmkcount):
            poslist[i] = bmklist[i] + 1000000
        OpenBookmarks(current_page, current_page+1000000, bmklist, poslist, bmkcount, 15, bmk_handler)
    elif index == 117:
        OpenContents(contents, current_page, page_selected) #42
    elif index == 119:
        CreateNote("/test", "test", 0)
    elif index == 120:
        OpenNotesMenu("/test", "test", 0)
    elif index == 125:
        OpenDirectorySelector("Open directory", dirbuf, 1024, dir_selected)
    elif index == 121:
        CloseApp()

def main_handler(type, par1, par2):
    print("[%d %d %d]" % (type, par1, par2), file=sys.stderr)

    if type == EVT_INIT:
        # occurs once at startup, only in main handler
        #testcfg = OpenConfig("/mnt/ext1/test.cfg", testce)

        global arial8n 
        arial8n = OpenFont("DroidSans", 8, 0)
        global arial12 
        arial12 = OpenFont("DroidSans", 12, 1)
        global arialb12 
        arialb12 = OpenFont("DroidSans", 12, 1)
        global cour16 
        cour16 = OpenFont("cour", 16, 1)
        global cour24 
        cour24 = OpenFont("cour", 24, 1)
        global times20 
        times20 = OpenFont("times", 20, 1)

        global menu1_theme
        global themes
        themes = EnumThemes()
        for i, themename in enumerate(themes):
            menu1_theme.append([ITEM_ACTIVE, 300 + i, themename, None])

        global menu1_lang
        global langs
        langs = EnumLanguages()
        for i, langname in enumerate(langs):
            menu1_lang.append([ITEM_ACTIVE, 400 + i, langname, None])

    if type == EVT_SHOW:
        # occurs when this event handler becomes active
        mainscreen_repaint()

    if type == EVT_KEYPRESS:
        if par1 == KEY_OK:
            OpenMenu(menu1, cindex, 20, 20, menu1_handler)
        elif par1 == KEY_BACK:
            CloseApp()
        elif par1 == KEY_LEFT:
            msg("KEY_LEFT")
        elif par1 == KEY_RIGHT:
            msg("KEY_RIGHT")
        elif par1 == KEY_UP:
            msg("KEY_UP")
        elif par1 == KEY_DOWN:
            msg("KEY_DOWN")
        elif par1 == KEY_MUSIC:
            msg("KEY_MUSIC")
        elif par1 == KEY_MENU:
            msg("KEY_MENU")
        elif par1 == KEY_DELETE:
            msg("KEY_DELETE")

    if type == EVT_EXIT:
        # occurs only in main handler when exiting or when SIGINT received.
        # save configuration here, if needed
        pass
        
    return 0

ctab = 1
#int 
def screen2_handler(type, par1, par2):
    global ctab
    if type == EVT_SHOW:
        ClearScreen()
        SetFont(times20, BLACK)
        DrawTextRect(0, 100, 600, 300, "Screen 2\nPress OK to return", ALIGN_CENTER)
        FillArea(0, ScreenHeight()-24, ScreenWidth(), 24, DGRAY)
        DrawRect(0, ScreenHeight()-24, ScreenWidth(), 2, BLACK)
        DrawTabs(None, ctab, 25)
        for i in range(9, 21):
            f = OpenFont("LiberationSans", i, 0)
            SetFont(f, BLACK)
            buf = "%i qwertyuiopasdfghjklzxcvbnm" % (i)
            DrawString(2, 100+i*24, buf)
            CloseFont(f)
            f = OpenFont("LiberationSans-Bold", i, 0)
            SetFont(f, BLACK)
            DrawString(305, 100+i*24, buf)
            CloseFont(f)
        FullUpdate()
    if type == EVT_KEYPRESS:
        if par1 == KEY_LEFT:
            if ctab > 0:
                ctab -= 1
        elif par1 == KEY_RIGHT:
            if ctab < 24: 
                ctab += 1
        else:
            SetEventHandler(main_handler)
            return 0
        DrawTabs(None, ctab, 25)
        FullUpdate()
    return 0

        
if __name__ == "__main__":
    InkViewMain(main_handler)




#include "inkview.h"
#include "cpuzzle.h"
#include "common.h"
#include <algorithm>

typedef std::vector<std::string> TPuzzlesList;
typedef TPuzzlesList::iterator TPuzzlesListIt;

CPuzzle *puzzle = NULL;
ifont   *arial10n = NULL;
iconfig *config = NULL;

#define getPuzzle if (puzzle) puzzle
//const char *dataDir = SDCARDDIR"/crosspix/";
const char *dataDir = "/mnt/ext1/crosspix/";

imenu mainMenu[] = {
  { ITEM_HEADER,   0, "Crosspix", NULL },
  { ITEM_ACTIVE, 101, "New Game", NULL },
  { ITEM_ACTIVE, 102, "Restart puzzle", NULL },
  { ITEM_ACTIVE, 103, "Show solution", NULL },
  { ITEM_SEPARATOR, 0, NULL, NULL },
  { ITEM_ACTIVE, 201, "Change orientation", NULL },
  { ITEM_SEPARATOR, 0, NULL, NULL },
  { ITEM_ACTIVE, 301, "How to play", NULL },
  { ITEM_ACTIVE, 302, "Controls", NULL },
  { ITEM_SEPARATOR, 0, NULL, NULL },
  { ITEM_ACTIVE, 122, "Exit", NULL },
  { 0, 0, NULL, NULL }
};

iconfigedit mainConfig[] = {
  { CFG_TEXT, NULL, "Orientation", NULL, PROP_ORIENTATION, "0", NULL, NULL },
  { CFG_TEXT, NULL, "Last Puzzle", NULL, PROP_LASTPUZZLE, "Animals01.cry", NULL, NULL },
  { CFG_TEXT, NULL, "Puzzle state", NULL, PROP_PUZZLESTATE, "", NULL, NULL },
  { CFG_TEXT, NULL, "Cursor horiz position", NULL, PROP_CURSORXPOS, "1", NULL, NULL },
  { CFG_TEXT, NULL, "Cursor vert  position", NULL, PROP_CURSORYPOS, "1", NULL, NULL },
  { CFG_TEXT, NULL, "Zoom shift", NULL, PROP_ZOOM, "0", NULL, NULL },
  { 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};

bool isFileExists(const char *aFilename) 
{
  struct stat stFileInfo;
  return stat(aFilename, &stFileInfo) == 0;
}

// function declarations
void mainScreenRepaint();

void getPuzzlesList(const char *aPath, TPuzzlesList &aPuzzlesList)
{
    DIR *pdir = iv_opendir(aPath);
    if (!pdir)
    {
        printf ("ERROR! pdir could not be initialised correctly\n");
        return;
    }
    
    while (struct dirent *pent = iv_readdir(pdir))
    {
        if (!pent)
        {
            printf ("ERROR! pent could not be initialised correctly\n");
            break;
        }
        
        char *pDot = strrchr(pent->d_name, '.');
        if (!pDot)
            continue;
        if (strcmp(pDot, ".cry") == 0)
        {        
            aPuzzlesList.push_back(pent->d_name);
        }
    }    
    iv_closedir(pdir);
}

imenu *puzzlesListMenu = NULL;
TPuzzlesList puzzlesList;

void SelectPuzzleHandler(int aButton)
{
   if ((aButton >= 200) && (aButton - 200 < (int)puzzlesList.size()))
   {       
       char pathBuff[PATH_MAX];
       strcpy(pathBuff, dataDir);
       strcat(pathBuff, (char *)puzzlesList[aButton - 200].c_str());
       
       if (CPuzzle *tmpPuzzle = readPuzzle(pathBuff))
       {
           delete puzzle;
           puzzle = tmpPuzzle;
           
           mainScreenRepaint();
       }
       else
       {
           Message(ICON_ERROR, "Error", pathBuff, 3);       
       }
   }   
}

void NewPuzzleHandler(int aButton)
{
    if ((aButton == 1))
    {
        if (puzzlesListMenu)
        {
            delete[] puzzlesListMenu;
            puzzlesListMenu = NULL;
        }
        puzzlesList.clear();
        puzzlesList.reserve(0);
        
        getPuzzlesList(dataDir, puzzlesList);

        if (puzzlesList.empty())
            return;

        std::sort(puzzlesList.begin(), puzzlesList.end());

        puzzlesListMenu = new imenu[puzzlesList.size() + 1];
        for(int i = 0, c = puzzlesList.size(); i < c; ++i)
        {
            puzzlesListMenu[i].type = ITEM_ACTIVE;            
            puzzlesListMenu[i].text = (char *)puzzlesList[i].c_str();
            puzzlesListMenu[i].submenu = NULL; 
            puzzlesListMenu[i].index = 200 + i;
        }
        puzzlesListMenu[puzzlesList.size()].type = 0;
        OpenMenu(puzzlesListMenu, 0, 40, 40, SelectPuzzleHandler);
    }
}

void RestartPuzzleHandler(int aButton)
{
    if ((aButton == 1) && puzzle)
    {
        puzzle->ClearSolution();    
    }
}

void ShowSolutionHandler(int aButton)
{
    if ((aButton == 1) && puzzle)
    {
        puzzle->ShowSolution();    
    }
}

void RotateHandler(int direction)
{
    SetOrientation(direction);
    mainScreenRepaint();
} 

int cindex=0;
void mainMenuHandler(int index)
{
    cindex = index;

    switch (index)
    {
      case 101:
          Dialog(ICON_QUESTION, "Confirmation", "Start other puzzle?", "Yes", "No", NewPuzzleHandler);
          break;
      case 102:
          Dialog(ICON_QUESTION, "Confirmation", "You are really want restart current puzzle?", "Yes", "No", RestartPuzzleHandler);
          break;
      case 103:
          Dialog(ICON_QUESTION, "Confirmation", "Make some magic and show solution?", "Yes", "No", ShowSolutionHandler);
          break;
      case 122:
          CloseApp();
          break;
      case 201:
          OpenRotateBox(RotateHandler);
          break;
      case 301:
          Dialog(ICON_INFORMATION, "How to play", "In order to solve a puzzle, one needs to determine which cells are going to be boxes and which are going to be empty. Determining which cells are to be empty (called spaces) is as important as determining which are to be filled (called boxes). Later in the solving process, the spaces help to determine where a clue (continuing block of boxes and a number in the legend) may spread. Solvers usually use a dot or a cross to mark cells that are spaces for sure.\n"
                                                  "It is also important never to guess. Only cells that can be determined by logic should be filled. If guessing, a single error can spread over the entire field and completely ruin the solution. It usually comes to the surface only after a while, when it is very difficult to correct the puzzle. Usually, only advanced and experienced solvers are able to fix it completely and finish such ruined puzzles.\n"
                                                  "Simpler puzzles can usually be solved by a reasoning on a single row only (or a single column) at each given time, to determine as many boxes and spaces on that row as possible. Then trying another row (or column), until there are rows that contain undetermined cells.\n",
                                                  "OK", NULL);
          break;      
      case 302:
          Dialog(ICON_INFORMATION, "Controls", "Use joystik to move cursor.\n"
                                               "Press button 'Ok' to mark cell filled in first time, and mark free in second time.\n"
                                               "Use buttons '+' and '-' to zoom game grid.\n"
                                               "Press 'Trash' button to show this menu.\n",
                                               "", "", NULL);
          break;
    }
}

void mainScreenRepaint()
{
    if (!puzzle)
	    return;
	    
	ClearScreen();
	
	puzzle->CalcParams();
	puzzle->DrawGameGrid();
	puzzle->DrawCursor();
	
    FullUpdate();
}

void ReadConfigAndInit()
{
    config = OpenConfig(CONFIGPATH"/crosspix.cfg", mainConfig);
    
    int direction = ReadInt(config, PROP_ORIENTATION, ROTATE0);
    if ((direction >= ROTATE0) && (direction <= ROTATE180))    
        SetOrientation(direction);

    bool nameFromConfig = true;
            
    char *lastPuzzle = ReadString(config, "LastPuzzle", "Animals01.cry");
    char buff[PATH_MAX];
    strcpy(buff, dataDir);
    strcat(buff, lastPuzzle);
    
    if (!isFileExists(buff))
    {
        nameFromConfig = false;
        puzzlesList.clear();        
        getPuzzlesList(dataDir, puzzlesList);
        if (puzzlesList.empty())
        {
            Message(ICON_ERROR, "Init Error", "Can`t find game data.\nYou must place .cry files in folder /crosspix on SD card.", 5);
            return;        
        }
        strcpy(buff, dataDir);
        strcat(buff, puzzlesList[0].c_str());
    }

	puzzle = readPuzzle(buff);
	if (puzzle)
	{
	    if (nameFromConfig)
	        puzzle->ReadState();	
	}
	
}

int main_handler(int type, int par1, int par2)
{
	int res = 0;
	if (type == EVT_INIT) 
	{
		// occurs once at startup, only in main handler
	}
	else if (type == EVT_SHOW) 
	{
		// occurs when this event handler becomes active
		ReadConfigAndInit();
		mainScreenRepaint();
	}
	else if (type == EVT_KEYPRESS) 
	{
        switch (par1) 
		{
            case KEY_DELETE:
            case KEY_BACK:
            case KEY_MENU:
		        OpenMenu(mainMenu, cindex, 40, 40, mainMenuHandler);
		        res = 1;
                break;
            case KEY_LEFT:
            case KEY_RIGHT:
            case KEY_UP:
            case KEY_DOWN:
                getPuzzle->MoveCursor(par1);
                res = 1;
                break;
            case KEY_PLUS:
            case KEY_PREV:
                if (puzzle && puzzle->ZoomIn())
                    mainScreenRepaint();
                res = 1;
                break;
            case KEY_MINUS:
            case KEY_NEXT:
                if (puzzle && puzzle->ZoomOut())
                    mainScreenRepaint();
                res = 1;
                break;
            case KEY_OK:
                getPuzzle->ClickCell();
                res = 1;
                break;
        }
	}
	else if (type == EVT_EXIT) 
	{
		getPuzzle->WriteState();
		WriteInt(config, PROP_ORIENTATION, GetOrientation());
		SaveConfig(config);
		CloseConfig(config);
	}
	return res;
}

int main(int argc, char **argv)
{
	InkViewMain(main_handler);		
	return 0;
}

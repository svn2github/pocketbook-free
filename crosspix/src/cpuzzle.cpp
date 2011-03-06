
#include "cpuzzle.h"
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>

#include "inkview.h"
#include "common.h"

   
CPuzzle::CPuzzle(const std::string &aFileName):
             m_IsInited(false), m_Font(NULL), m_FileName(aFileName),
             m_gridCellSize(12), m_RowsCount(0), m_ColsCount(0), m_ZoomShift(0),
             m_MaxVertNumbers(0), m_MaxHorizNumbers(0), 
             m_gridStartX(1), m_gridStartY(0), m_CursorXpos(0), m_CursorYpos(0),
             m_UpdateRectX(MAX_INT), m_UpdateRectY(MAX_INT), m_UpdateRectXl(0), m_UpdateRectYl(0)
{
}

CPuzzle::~CPuzzle()
{
    if (m_Font)
        CloseFont(m_Font);
}

void CPuzzle::InitFromBuff(int aCols, int aRows, char *aBuff)
{
    m_Grid.reserve(aRows);
    m_Grid.resize(aRows);
    
    m_HorizNumbers.reserve(aRows);
    m_HorizNumbers.resize(aRows);
    
    m_VertNumbers.reserve(aCols);
    m_VertNumbers.resize(aCols);
    
    for(int row = 0; row < aRows; ++row)
    {
        m_Grid[row].reserve(aCols);
        m_Grid[row].resize(aCols);
        
        bool isGroup = false;
        int  groupCounter = 0;
        int  numbersCounter = 0;
        for(int col = 0; col < aCols; ++col)
        {            
            if (*aBuff++ == 0x01)
            {                
                m_Grid[row][col] = 1;
                if (!isGroup)
                    isGroup = true;
                ++groupCounter;                   
            }    
            else
            {
                m_Grid[row][col] = 0;
                if (isGroup)
                {
                    m_HorizNumbers[row].push_back(groupCounter);
                    ++numbersCounter;
                    isGroup = false;                    
                    groupCounter = 0;
                }
            }
        }
        if (isGroup)
        {
            m_HorizNumbers[row].push_back(groupCounter);
            ++numbersCounter;
        }
        if (m_HorizNumbers[row].empty())
            m_HorizNumbers[row].push_back(0);
        
        if (numbersCounter > m_MaxHorizNumbers)
            m_MaxHorizNumbers = numbersCounter;
    }
    
    for(int col = 0; col < aCols; ++col)
    {
        bool isGroup = false;
        int  groupCounter = 0;
        int  numbersCounter = 0;
        for(int row = 0; row < aRows; ++row)
        {
            if (m_Grid[row][col] == 1)
            {
                isGroup = true;
                ++groupCounter;            
            }
            else if (isGroup)
            {
                m_VertNumbers[col].push_back(groupCounter);
                ++numbersCounter;
                isGroup = false;     
                groupCounter = 0;
            }
        }
        if (isGroup)
        {
            m_VertNumbers[col].push_back(groupCounter);            
            ++numbersCounter;
        }
        
        if (m_VertNumbers[col].empty())
            m_VertNumbers[col].push_back(0);
        
        if (numbersCounter > m_MaxVertNumbers)
            m_MaxVertNumbers = numbersCounter;
    }
    m_RowsCount = aRows;
    m_ColsCount = aCols;
    
    m_CursorXpos = m_ColsCount >> 1;
    m_CursorYpos = m_RowsCount >> 1;

    m_IsInited = true;
}

#define minimalCellSize 8

void CPuzzle::CalcParams()
{
    if (!m_IsInited)
        return;
    
    int sw = ScreenWidth(), sh = ScreenHeight();
    m_gridCellSize = std::min((sw - 2) / (m_MaxHorizNumbers + m_ColsCount), \
                            (sh	- 2) / (m_MaxVertNumbers + m_RowsCount));
    m_gridCellSize += m_ZoomShift;
    if (m_gridCellSize < minimalCellSize)
        m_gridCellSize = minimalCellSize;
    printf("Zoom shift: %d; cell: %d\n", m_ZoomShift, m_gridCellSize);
                                
    int numbersWidth  = m_MaxHorizNumbers * m_gridCellSize;
    int numbersHeight = m_MaxVertNumbers  * m_gridCellSize;
    
    int cellsWidth  = m_ColsCount * m_gridCellSize;
    int cellsHeight = m_RowsCount * m_gridCellSize;  

    m_gridStartX = sw - cellsWidth - numbersWidth;
    if (m_gridStartX <= 0)
        m_gridStartX = 0;
    else
    {
        int xCenter = (sw - cellsWidth) / 2;
        if (m_gridStartX > xCenter)
            m_gridStartX = xCenter;
        else
            m_gridStartX /= 2;
    }
    
    m_gridStartY = numbersHeight;
    if (m_gridStartY + cellsHeight > sh)
        m_gridStartY = sh - cellsHeight;
    else
    {
        int yCenter = (sh - cellsHeight) / 2;
        if (m_gridStartY < yCenter)
            m_gridStartY = yCenter;
        else
            m_gridStartY += yCenter - (m_gridStartY/2);
    }
}

void CPuzzle::DrawGameGrid()
{
    if (!m_IsInited)
        return;
        
    const int gridPadding = 1;
    
    #define selColor(p, mp) (p % 5 == 0) || (p == mp)?BLACK:DGRAY
        
    // Draw grid
    int lineLength = m_ColsCount * m_gridCellSize;
    for (int r = 0; r <= m_RowsCount; ++r)
    {
        int y = m_gridStartY + (r * m_gridCellSize);
        DrawLine(m_gridStartX, y, m_gridStartX + lineLength, y, selColor(r, m_RowsCount));
    }
    
    lineLength = m_RowsCount * m_gridCellSize - 1;
    for (int c = 0; c <= m_ColsCount; ++c)
    {
        int x = m_gridStartX + (c * m_gridCellSize);
        DrawLine(x, m_gridStartY, x, m_gridStartY + lineLength, selColor(c, m_ColsCount));
    }
    
    if (m_Font)
        CloseFont(m_Font);
    int fontSize = m_gridCellSize + 1;
    int maxCharWidth = m_gridCellSize / 2;
    while(fontSize > 5)
    {
        m_Font = OpenFont("DroidSans", fontSize, 0);
        if (!m_Font)
		{
		    printf("Can`t open font DroidSans with size %d", fontSize);
		    return;        
		}
		    
        SetFont(m_Font, BLACK);
        bool badSize = false; 

        for(char c = '0', ec = '9'; c <= ec; ++c)
        {
            if (CharWidth(c) > maxCharWidth)
            {
                badSize = true;
                break;            
            }
        }
        if (!badSize)
            break;
        --fontSize;
    }
    printf("Font size: %d\n", fontSize);
    
    int gridRightSide = m_gridStartX + (m_ColsCount * m_gridCellSize) + 1;
    int numCellSize = m_gridCellSize - 1;
    char buff[8];
    
    // Draw horizontal numbers
    for (int row = 0; row < m_RowsCount; ++row)
    {
        int xPos = gridRightSide + gridPadding;
        int yPos = m_gridStartY + (row * m_gridCellSize);
        for (TNumbersListIt it = m_HorizNumbers[row].begin(), it_end = m_HorizNumbers[row].end();
             it != it_end;
             ++it)
        {
            sprintf(buff, "%d", *it);
            FillArea(xPos, yPos + 1, numCellSize, numCellSize, LGRAY);
            DrawTextRect(xPos, yPos + 1, m_gridCellSize, numCellSize, buff, ALIGN_CENTER | VALIGN_MIDDLE);
            xPos += m_gridCellSize;
        }
    }
    
    // Draw vertical numbers
    for (int col = 0; col < m_ColsCount; ++col)
    {
        int xPos = m_gridStartX + (col * m_gridCellSize);
        int yPos = m_gridStartY - m_gridCellSize;        
        for (TNumbersListRIt it = m_VertNumbers[col].rbegin(), it_end = m_VertNumbers[col].rend();
             it != it_end;
             ++it)
        {
            sprintf(buff, "%d", *it);
            FillArea(xPos + 1, yPos, numCellSize, numCellSize, LGRAY);
            DrawTextRect(xPos, yPos, m_gridCellSize + 1, numCellSize, buff, ALIGN_CENTER | VALIGN_MIDDLE);
            yPos -= m_gridCellSize;
        }
    }
    
    RefreshAllCells();
}

void CPuzzle::DrawCursor(int aColor)
{  
   int x = m_gridStartX + (m_gridCellSize * m_CursorXpos);
   int y = m_gridStartY + (m_gridCellSize * m_CursorYpos);
   
   DrawRect(x + 2, y + 2, m_gridCellSize - 3, m_gridCellSize - 3, aColor);
   //FillArea(x + 3, y + 3, m_gridCellSize - 5, m_gridCellSize - 5, LGRAY);
   
   AddCellToUpdate(m_CursorXpos, m_CursorYpos);
}
    
void CPuzzle::MoveCursor(int aKey)
{
    int curXnew = m_CursorXpos;
    int curYnew = m_CursorYpos;
    
    switch(aKey)
    {
        case KEY_LEFT:
        {
            if (m_CursorXpos == 0)
                curXnew = m_ColsCount - 1;
            else
                --curXnew;        
        }; break;
        
        case KEY_RIGHT:
        {
            if (m_CursorXpos == m_ColsCount - 1)
                curXnew = 0;
            else
                ++curXnew;        
        }; break;
        
        case KEY_UP:
        {
            if (m_CursorYpos == 0)
                curYnew = m_RowsCount - 1;
            else
                --curYnew;
        }; break;
        
        case KEY_DOWN:
        {
            if (m_CursorYpos == m_RowsCount - 1)
                curYnew = 0;
            else
                ++curYnew;
        }; break;
                
        default: return;
    }
    
    DrawCursor( (m_Grid[m_CursorYpos][m_CursorXpos] & 0x06)==0x02?BLACK:WHITE);
    AddCellToUpdate(m_CursorXpos, m_CursorYpos);
    m_CursorXpos = curXnew;    
    m_CursorYpos = curYnew;    
    DrawCursor( (m_Grid[m_CursorYpos][m_CursorXpos] & 0x06)==0x02?WHITE:BLACK);
    UpdateCells();
}

void CPuzzle::ClickCell()
{
    int val = 0x00;
    switch(m_Grid[m_CursorYpos][m_CursorXpos] & 0x06)
    {
        case 0x00: val = 0x02; break;
        case 0x02: val = 0x06; break;
        case 0x04: val = 0x04; break;
        default: m_Grid[m_CursorYpos][m_CursorXpos] &= 0x01; 
    }
    m_Grid[m_CursorYpos][m_CursorXpos] ^= val;
    
    RefreshCell(m_CursorXpos, m_CursorYpos);
    DrawCursor();
    
    UpdateCells();
}

bool CPuzzle::ZoomIn()
{
    if (m_ZoomShift < 10)
        m_ZoomShift += 1;
    return true;
}

bool CPuzzle::ZoomOut()
{
    if (m_gridCellSize > minimalCellSize)
        m_ZoomShift -= 1;
    else
        return false;
    return true;
}

void CPuzzle::AddCellToUpdate(int aX, int aY)
{
    int x = m_gridStartX + (m_gridCellSize * aX);
    int y = m_gridStartY + (m_gridCellSize * aY);

    if (x < m_UpdateRectX)
        m_UpdateRectX = x;
    if (y < m_UpdateRectY)
        m_UpdateRectY = y;
    if (x + m_gridCellSize > m_UpdateRectXl)
       m_UpdateRectXl = x + m_gridCellSize;
    if (y + m_gridCellSize > m_UpdateRectYl)
       m_UpdateRectYl = y + m_gridCellSize;
}

void CPuzzle::RefreshCell(int aX, int aY, bool aNeedUpdate)
{
    if ((aX >= m_ColsCount) || (aY >= m_RowsCount))
        return;
        
    int x = m_gridStartX + (m_gridCellSize * aX) + 1;
    int y = m_gridStartY + (m_gridCellSize * aY) + 1;
    int w = m_gridCellSize - 1;
    int h = w;
    
    int color = WHITE;
    switch(m_Grid[aY][aX] & 0x06)
    {
        case 0x02: color = BLACK; break;
        case 0x04:
        {
            FillArea(x, y, w, h, color);
            color = LGRAY;
            const int smallCellSize = 4;
            const int smallCellOffset = (m_gridCellSize / 2) - (smallCellSize / 2);
            
            x += smallCellOffset;
            y += smallCellOffset;
            w = smallCellSize - 1;
            h = smallCellSize - 1;
        }; break;    
    }
    FillArea(x, y, w, h, color);
    if (aNeedUpdate)
        AddCellToUpdate(aX, aY);
}

void CPuzzle::RefreshAllCells()
{
    for(int row = 0; row < m_RowsCount; ++row)
    {
        for(int col = 0; col < m_ColsCount; ++col)
        {
            RefreshCell(col, row, false);
        }    
    }
    PartialUpdateBW(m_gridStartX, m_gridStartY, m_gridCellSize * m_ColsCount, m_gridCellSize * m_RowsCount);
}

void CPuzzle::ShowSolution()
{
    for(int row = 0; row < m_RowsCount; ++row)
    {
        for(int col = 0; col < m_ColsCount; ++col)
        {
            m_Grid[row][col] &= 0x01;
            m_Grid[row][col] |= m_Grid[row][col] << 1;        
        }        
    }
    RefreshAllCells();
}

void CPuzzle::ClearSolution()
{
    for(int row = 0; row < m_RowsCount; ++row)
    {
        for(int col = 0; col < m_ColsCount; ++col)
        {
            m_Grid[row][col] &= 0x01;
        }
    }
    RefreshAllCells();
}

void CPuzzle::WriteState()
{
    if (!config || !m_IsInited)
        return;
        
    std::string gridState;
    for(int row = 0; row < m_RowsCount; ++row)
    {
        for(int col = 0; col < m_ColsCount; ++col)
        {
            switch(m_Grid[row][col] & 0x06)
            {
                case 0x02: gridState += '1'; break;
                case 0x04: gridState += '2'; break;
                default:   gridState += '0';
            }
        }
    }
    WriteString(config, PROP_PUZZLESTATE, (char *)gridState.c_str());    
    WriteString(config, PROP_LASTPUZZLE, (char *)m_FileName.c_str());
    
    WriteInt(config, PROP_CURSORXPOS, m_CursorXpos);
    WriteInt(config, PROP_CURSORYPOS, m_CursorYpos);
    
    char buff[10];
    sprintf(buff, "%d", m_ZoomShift);
    WriteString(config, PROP_ZOOM, buff);
}

void CPuzzle::ReadState()
{
    if (!config || !m_IsInited)
        return;

    m_ZoomShift = atoi(ReadString(config, PROP_ZOOM, "0"));
    if ((m_ZoomShift > 10) || (m_ZoomShift < -10))
        m_ZoomShift = 0;
        
    int newCX = ReadInt(config, PROP_CURSORXPOS, m_CursorXpos);
    int newCY = ReadInt(config, PROP_CURSORYPOS, m_CursorYpos);
    
    if (newCX < m_ColsCount)
        m_CursorXpos = newCX;
    if (newCY < m_RowsCount)
        m_CursorYpos = newCY;
        
    char *state = ReadString(config, PROP_PUZZLESTATE, "");
    state = strpbrk(state, "012");
    if (!state || ((int)strlen(state) < (m_ColsCount * m_RowsCount)))
    {
        printf("Wrong state line");
    }
    else
    {
        for(int row = 0; row < m_RowsCount; ++row)
        {
            for(int col = 0; col < m_ColsCount; ++col)
            {
                switch(*state)
                {
                    case '2': m_Grid[row][col] |= 0x04; break;
                    case '1': m_Grid[row][col] |= 0x02; break;
                    case '0': m_Grid[row][col] &= 0x01; break;
                }
                ++state;
            }
        }    
    }
}
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-

#define getLineAndCheckResult() \
    puzzleFile.getline(tmpStr, sizeof(tmpStr)); \
    if (!puzzleFile.good()) \
    {                       \
        std::cerr << "Wrong file format" << std::endl; \
        return NULL;        \
    }

CPuzzle *readPuzzle(const std::string &aPuzzlePath)
{   
    std::ifstream puzzleFile;
    puzzleFile.open(aPuzzlePath.c_str(), std::ifstream::in | std::ifstream::binary);
    
    if (!puzzleFile.good())
    {
        std::cerr << "Can`t open file: " << aPuzzlePath << std::endl;
        return NULL;
    }
        
    char tmpStr[256];
    getLineAndCheckResult(); // skip first line
    
    getLineAndCheckResult(); // Grid size
    if (strncasecmp(tmpStr, "GRIDSIZE=", sizeof("GRIDSIZE=") - 1) != 0)
    {
        std::cerr << "Wrong file format, can`t find grid size." << std::endl << tmpStr << std::endl;
        return NULL;    
    }
    
    char *gridValueStart = tmpStr + sizeof("GRIDSIZE=");
    char *x_char = strchr(gridValueStart, 'x');
    if (!x_char)
    {
        std::cerr << "Wrong file format, can`t parse grid size." << std::endl << tmpStr << std::endl;
        return NULL;    
    }
    *x_char++ = 0;
    
    int gridCols = atoi(gridValueStart), gridRows = atoi(x_char);
    // end. Grid size parsed.
    
    getLineAndCheckResult(); // skip difficulty
    getLineAndCheckResult(); // skip something...
    getLineAndCheckResult(); // skip name
    getLineAndCheckResult(); // skip flag

    #define gridBuffSize 1024 * 1024 
    char *buff = new char[gridBuffSize]; // 1MB buffer
    
    puzzleFile.get(buff, gridBuffSize);
    if (gridRows * gridCols > puzzleFile.gcount())
    {
        std::cerr << "Wrong file format, can`t read full grid data." << std::endl;
        delete[] buff;
        return NULL;    
    }
    puzzleFile.close();
        
    CPuzzle *newPuzzle = new CPuzzle(aPuzzlePath.substr(aPuzzlePath.rfind("/") + 1));
    newPuzzle->InitFromBuff(gridCols, gridRows, buff);
    delete[] buff;       

    return newPuzzle;
}


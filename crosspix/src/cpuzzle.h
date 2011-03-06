#ifndef CPUZZLE_H
#define CPUZZLE_H 

#include <string>
#include <vector>
#include <list>

#include "inkview.h"

class CPuzzle
{
private:
    typedef std::vector< std::vector<unsigned char> > TGrid;
    
    typedef std::list<int>                 TNumbersList;
    typedef TNumbersList::iterator         TNumbersListIt;
    typedef TNumbersList::reverse_iterator TNumbersListRIt;
    
    typedef std::vector<TNumbersList> TNumbersColumn;
    typedef TNumbersColumn::iterator  TNumbersColumnIt;
    
    bool m_IsInited;
    
    ifont *m_Font;
    
    std::string m_FileName;
    
    int m_gridCellSize;
    
    int m_RowsCount;
    int m_ColsCount;
    
    int m_ZoomShift;
    
    TGrid m_Grid;
    TNumbersColumn m_VertNumbers;
    TNumbersColumn m_HorizNumbers;
    
    int m_MaxVertNumbers;
    int m_MaxHorizNumbers;
    
    int m_gridStartX;
    int m_gridStartY;
    
    int m_CursorXpos;
    int m_CursorYpos;
    
    int m_UpdateRectX;
    int m_UpdateRectY;
    int m_UpdateRectXl;
    int m_UpdateRectYl;
        
    void RefreshCell(int aX, int aY, bool aNeedUpdate = true);
    void RefreshAllCells();
    
    void AddCellToUpdate(int aX, int aY);
    void UpdateCells()
    {
        PartialUpdateBW(m_UpdateRectX, m_UpdateRectY, m_UpdateRectXl - m_UpdateRectX, m_UpdateRectYl - m_UpdateRectY);    
    }
public:
    CPuzzle(const std::string &aFileName);
   ~CPuzzle();
   
    void InitFromBuff(int aCols, int aRows, char *aBuff);
    void CalcParams();
    void DrawGameGrid();
    void DrawCursor(int aColor = BLACK);
       
    void MoveCursor(int aKey);
    void ClickCell();
    
    bool ZoomIn();
    bool ZoomOut();
    
    void ShowSolution();
    
    void ClearSolution();
    
    void WriteState();
    void ReadState();
};

CPuzzle *readPuzzle(const std::string &aPuzzlePath);

#endif

import random
import inkview as iv

class NumericTable:
    def __init__(self, rows, cols):
        self.rows = rows
        self.cols = cols
        self.numbers = None
        self.generate()
        
    def generate(self):
        maxnumber = self.rows * self.cols
        self.numbers = numbers = [0] * maxnumber
        for i in range(maxnumber):
            while True:
                idx = random.randint(0, maxnumber-1)
                if numbers[idx] == 0:
                    numbers[idx] = i + 1
                    break
        
        
table = NumericTable(5, 5)
timesNN = None

def repaint():
    iv.ClearScreen()
    rows = table.rows
    cols = table.cols
    
    h = iv.ScreenHeight()
    w = iv.ScreenWidth()
    border = 5
    size = min(w, h) - border * 2
    table_x = border
    table_y = border
    cell_h = size // rows
    cell_w = size // cols
    size = min(cell_w * cols, cell_h * rows)

    y = table_y
    for i in range(rows+1):
        iv.DrawLine(table_x, y, table_x + size, y, iv.BLACK)
        y += cell_h
    
    x = table_x
    for i in range(cols+1):
        iv.DrawLine(x, table_y, x, table_y + size, iv.BLACK)
        x += cell_w
        
    iv.SetFont(timesNN, iv.BLACK)
    y = table_y
    for r in range(rows):
        x = table_x
        for c in range(cols):
            n = table.numbers[r*cols + c]
            iv.DrawTextRect(x, y, cell_w, cell_h, str(n), iv.ALIGN_CENTER | iv.VALIGN_MIDDLE)
            x += cell_w
        y += cell_h


@iv.MainHandler
def main_handler(type, par1, par2):
    global timesNN
    if type == iv.EVT_INIT:
        table.generate()
        timesNN = iv.OpenFont("times", 60, 1)
    if type == iv.EVT_SHOW:
        repaint()
        iv.FullUpdate()
    if type == iv.EVT_KEYPRESS:
        key = par1
        if key in (iv.KEY_LEFT, iv.KEY_RIGHT, iv.KEY_UP, iv.KEY_DOWN, iv.KEY_OK):
            table.generate()
            iv.Repaint()
        if key == iv.KEY_BACK:
            iv.CloseApp()
    if type == iv.EVT_EXIT:
        if timesNN:
            iv.CloseFont(timesNN)
            timesNN = None
    return 0

def main():
    iv.InkViewMain(main_handler)


if __name__ == "__main__":
    main()
    
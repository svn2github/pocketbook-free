from inkview import *

def main_handler(type, par1, par2):
    if type == EVT_SHOW:
        ClearScreen()
        FullUpdate()
        DrawRect(10, 18, 580, 104, 0)
        for i in range(16):
            FillArea(12+i*36, 20, 36, 100, i*0x111111)
        FullUpdate()
        FineUpdate()
        
    if type == EVT_KEYPRESS:
       CloseApp()
   
    return 0

if __name__ == "__main__":
    PyInkViewMain(main_handler)


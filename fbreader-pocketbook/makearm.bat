if not %POCKETBOOKSDK%.==. goto C1
echo Environment variable POCKETBOOKSDK is not set
pause
:C1

set PATH=%POCKETBOOKSDK%\arm-linux\bin;%POCKETBOOKSDK%\bin;%PATH%
del /s/q images\c
mkdir images\c
pbres -c images/c/images.c images/*.bmp
gcc -c -o images/c/images.o images/c/images.c
if errorlevel 1 goto EX

make %1 %2 %3 %4 %5
strip fbreader/fbreader.app

:EX

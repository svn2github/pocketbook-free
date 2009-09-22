if not %POCKETBOOKSDK%.==. goto C1
echo Environment variable POCKETBOOKSDK is not set
pause
:C1
set PATH=%POCKETBOOKSDK%\arm-linux\bin;%POCKETBOOKSDK%\bin;%PATH%

set INCLUDE=-I/arm-linux/include
set LIBS=-linkview -lfreetype -lz
set OUTPUT=Lines.app

rm -f %OUTPUT%

set IMAGES=
if not exist ..\images\*.bmp goto NOIMG
set IMAGES=temp.c
pbres -c %IMAGES% ../images/*.bmp
pbres -c runes.c ../images_rune/*.bmp
if errorlevel 1 goto L_ER
:NOIMG

gcc -Wall -O2 -fomit-frame-pointer -I/arm-linux/include -I../src -o %OUTPUT% ../src/*.c %IMAGES% -linkview -lfreetype -lz
gcc -Wall -O2 -fomit-frame-pointer -I/arm-linux/include -I../src -o LinesR.app ../src/*.c runes.c -linkview -lfreetype -lz
if errorlevel 1 goto L_ER
strip %OUTPUT%
string LinesR.app
exit 0

:L_ER
exit 1


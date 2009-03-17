if not %POCKETBOOKSDK%.==. goto C1
echo Environment variable POCKETBOOKSDK is not set
pause
:C1
set PATH=%POCKETBOOKSDK%\arm-linux\bin;%POCKETBOOKSDK%\bin;%PATH%

set INCLUDE=-I/arm-linux/include -I../djvulibre/djvulibre-arm
set LIBS=-L./lib/arm -ldjvulibre -ltiff -linkview -lpthread -ljpeg -lz -lstdc++
set OUTPUT=djviewer.app

rm -f %OUTPUT%

set IMAGES=
if not exist images\*.bmp goto NOIMG
set IMAGES=%TEMP%\images.temp.c
pbres -c %IMAGES% images/*.bmp
if errorlevel 1 goto L_ER
:NOIMG

del /s/q images\c
mkdir images\c
pbres -c images/c/images.c images/*.bmp
gcc -c -o images/c/images.o images/c/images.c
g++ -Wall -O2 -fomit-frame-pointer %INCLUDE% -o %OUTPUT% src/*.c* images/c/images.o %LIBS%
strip %OUTPUT%

exit 0

:L_ER
exit 1


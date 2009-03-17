if not %POCKETBOOKSDK%.==. goto C1
echo Environment variable POCKETBOOKSDK is not set
pause
:C1
set PATH=%POCKETBOOKSDK%\bin;%PATH%

set INCLUDE=-I../djvulibre/djvulibre-i386
set LIBS=-L./lib/i386 -ldjvulibre -ltiff -linkview -lpthread -ljpeg -lfreetype -lz -liconv -lgdi32
set OUTPUT=djviewer.exe

rm -f %OUTPUT%

del /s/q images\c
mkdir images\c
pbres -c images/c/images.c images/*.bmp
gcc -c -o images/c/images.o images/c/images.c
g++ -static -Wall -O2 -fomit-frame-pointer %INCLUDE% -o %OUTPUT% src/*.c* images/c/images.o %LIBS%
if errorlevel 1 goto L_ER

exit 0

:L_ER
exit 1


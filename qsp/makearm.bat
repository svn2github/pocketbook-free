if not %POCKETBOOKSDK%.==. goto C1
echo Environment variable POCKETBOOKSDK is not set
pause
:C1
set PATH=%POCKETBOOKSDK%\arm-linux\bin;%POCKETBOOKSDK%\bin;%PATH%

set INCLUDE=-I/arm-linux/include
set LIBS=-linkview -lfreetype -lz -lpthread
set OUTPUT=QSP.app

rm -f %OUTPUT%

set IMAGES=
if not exist images\*.bmp goto NOIMG
set IMAGES=%TEMP%\images.temp.c
pbres -c %IMAGES% images/*.bmp
if errorlevel 1 goto L_ER
:NOIMG

gcc -Wall -O2 -fomit-frame-pointer %INCLUDE% -c src/qsp/*.c src/qsp/onig/*.c src/qsp/onig/enc/*.c %IMAGES%
g++ -Wall -O2 -fomit-frame-pointer %INCLUDE% -c src/*.cpp
if errorlevel 1 goto L_ER

g++ -Wall -O2 -fomit-frame-pointer -o %OUTPUT% *.o %LIBS%

rm -f *.o

if errorlevel 1 goto L_ER

echo Built OK!
pause
exit 0

:L_ER
echo Built with ERRORS!
pause
exit 1


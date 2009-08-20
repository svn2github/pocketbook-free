if not %POCKETBOOKSDK%.==. goto C1
echo Environment variable POCKETBOOKSDK is not set
pause
:C1
set PATH=%POCKETBOOKSDK%\bin;%PATH%

set INCLUDE=
set LIBS=-linkview -lfreetype -ljpeg -lgdi32 -lz -lc
set OUTPUT=qsp.exe

rm -f %OUTPUT%

set IMAGES=
if not exist images\*.bmp goto NOIMG
set IMAGES=%TEMP%\images.temp.c
pbres -c %IMAGES% images/*.bmp
if errorlevel 1 goto L_ER
:NOIMG

rem gcc -Wall -O2 -fomit-frame-pointer %INCLUDE% -c src/qsp/*.c src/qsp/onig/*.c src/qsp/onig/enc/*.c %IMAGES%
g++ -Wall -O2 -fomit-frame-pointer %INCLUDE% -c src/*.cpp %IMAGES%
if errorlevel 1 goto L_ER

g++ -static -Wall -O2 -fomit-frame-pointer -o %OUTPUT% *.o %LIBS%

if errorlevel 1 goto L_ER

echo Built OK!
pause
exit 0

:L_ER
echo Built with ERRORS!
pause
exit 1



if not %POCKETBOOKSDK%.==. goto C1
echo Environment variable POCKETBOOKSDK is not set
pause
:C1
set PATH=%POCKETBOOKSDK%\arm-linux\bin;%POCKETBOOKSDK%\bin;%PATH%

set INCLUDE=-I/arm-linux/include -DNO_STL
set LIBS=-linkview -lfreetype -lz -lstdc++ -lcurl
set OUTPUT=pbcheckers-win.app

rm -f %OUTPUT%

set IMAGES=
if not exist images\*.bmp goto NOIMG
set IMAGES=%TEMP%\images.temp.c
pbres -c %IMAGES% images/*.bmp
if errorlevel 1 goto L_ER
:NOIMG

gcc -Wall -O2 -fomit-frame-pointer %INCLUDE% -I./src -o %OUTPUT% src/checkers.cc src/rcheckers.cc src/echeckers.cc src/pbcheckers.cc %IMAGES% %LIBS%
if errorlevel 1 goto L_ER

exit 0

:L_ER
exit 1


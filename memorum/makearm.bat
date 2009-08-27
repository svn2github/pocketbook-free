if not %POCKETBOOKSDK%.==. goto C1
echo Environment variable POCKETBOOKSDK is not set
pause
:C1
set PATH=%POCKETBOOKSDK%\arm-linux\bin;%POCKETBOOKSDK%\bin;%PATH%

set INCLUDE=-I/arm-linux/include
set LIBS=-linkview -lfreetype -lz
set OUTPUT=ARM/memorum.app

rm -f %OUTPUT%

set IMAGES=
if not exist images\*.bmp goto NOIMG
set IMAGES=%TEMP%\images.temp.c
pbres -c %IMAGES% images/*.bmp
if errorlevel 1 goto L_ER
:NOIMG

g++ -Wall -O2 -fomit-frame-pointer %INCLUDE% -I./src -o %OUTPUT% src/*.cpp %IMAGES% %LIBS%
if errorlevel 1 goto L_ER
strip %OUTPUT%

exit 0

:L_ER
exit 1


if not %POCKETBOOKSDK%.==. goto C1
echo Environment variable POCKETBOOKSDK is not set
pause
:C1
set PATH=%POCKETBOOKSDK%\bin;%PATH%

set INCLUDE=
set LIBS=-linkview -lfreetype -lcurl -ljpeg -lz -lgdi32
set OUTPUT=zwadrax.exe

rm -f %OUTPUT%

set IMAGES=
if not exist images\*.bmp goto NOIMG
set IMAGES=%TEMP%\images.temp.c
pbres -c %IMAGES% images/*.bmp
if errorlevel 1 goto L_ER
gcc -c -o %IMAGES%.o %IMAGES%
if errorlevel 1 goto L_ER
set IMAGES=%IMAGES%.o

:NOIMG

g++ -static -pedantic -Wall -fomit-frame-pointer %INCLUDE% -o %OUTPUT% src/*.cxx %IMAGES% %LIBS%
if errorlevel 1 goto L_ER

exit /b 0

:L_ER
pause
exit /b 1


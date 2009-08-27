if not %POCKETBOOKSDK%.==. goto C1
::echo Environment variable POCKETBOOKSDK is not set
::pause
set POCKETBOOKSDK=G:\pbsdk
:C1
set PATH=%POCKETBOOKSDK%\bin;%PATH%

set INCLUDE=
set LIBS=-linkview -lfreetype -ljpeg -lz -lgdi32 -lstdc++
set OUTPUT=metro.exe

rm -f %OUTPUT%

set IMAGES=
if not exist images\*.bmp goto NOIMG
set IMAGES=%TEMP%\images.temp.c
pbres -c %IMAGES% images/*.bmp
if errorlevel 1 goto L_ER
:NOIMG

gcc -static -Wall -O2 -fomit-frame-pointer %INCLUDE% -o %OUTPUT% src/*.cpp %IMAGES% %LIBS%
if errorlevel 1 goto L_ER
%OUTPUT%
:L_ER



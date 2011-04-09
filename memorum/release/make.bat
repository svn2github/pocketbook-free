set PROJECT=Release\memorum.app

set CXX=g++
set CXXFLAGS=-Wall -O2 -fomit-frame-pointer
set LIBS=-linkview -lfreetype -lz
set INCLUDE=-I/arm-linux/include -I./src
set SRC=src/*.cpp
set IMGS=

set POCKETBOOKSDK=D:\PBSDK
set PATH=%POCKETBOOKSDK%\arm-linux\bin;%POCKETBOOKSDK%\bin;%PATH%

if not exist %SRC% cd ..

rm -f %PROJECT%

if not exist images\*.bmp goto NOIMG
set IMGS=images/images.c
pbres -c %IMGS% images/*.bmp
if errorlevel 1 goto L_ER

:NOIMG
%CXX% %CXXFLAGS% %INCLUDE% -o %PROJECT% %SRC% %IMGS% %LIBS%
if errorlevel 1 goto L_ER
cd Release
sh makeinst.sh
exit 0

:L_ER
pause
exit 1


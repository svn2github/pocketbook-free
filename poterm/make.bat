set POCKETBOOKSDK=C:\PBSDK
if not %POCKETBOOKSDK%.==. goto C1
echo Environment variable POCKETBOOKSDK is not set
pause
:C1
set PATH=%POCKETBOOKSDK%\bin;%PATH%

set INCLUDE=-I%POCKETBOOKSDK%\usr\include
set LIBS=-L%POCKETBOOKSDK%\lib\w32api -linkview -lfreetype -ljpeg -lz -lgdi32
set OUTPUT=poterm.exe

rm -f %OUTPUT%

g++ -static -Wall -O2 -DHOST_X86 -DHOST_WINDOWS -fomit-frame-pointer %INCLUDE% -o %OUTPUT% poterm.cpp Term.cpp %LIBS%

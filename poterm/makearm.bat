set POCKETBOOKSDK=C:\PBSDK
if not %POCKETBOOKSDK%.==. goto C1
echo Environment variable POCKETBOOKSDK is not set
pause
:C1
set PATH=%POCKETBOOKSDK%\arm-linux\bin;%POCKETBOOKSDK%\bin;%PATH%

set INCLUDE=-I%POCKETBOOKSDK%\arm-linux\include
set LIBS=-L%POCKETBOOKSDK%\arm-linux\lib -linkview -lfreetype -lz
set OUTPUT=poterm.app

rm -f %OUTPUT%

g++ -Wall -O2 -fomit-frame-pointer -DHOST_ARM -DHOST_WINDOWS %INCLUDE% -o %OUTPUT% poterm.cpp Term.cpp %LIBS%

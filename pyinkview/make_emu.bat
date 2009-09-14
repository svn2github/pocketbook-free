@echo off

if not %POCKETBOOKSDK%.==. goto C1
echo Environment variable POCKETBOOKSDK is not set
pause
:C1
set PATH=%POCKETBOOKSDK%\bin;%PATH%

set INCLUDE=
set LIBS=-linkview -lfreetype -ljpeg -lz -lgdi32

set IMAGES=
if not exist demo\images\*.bmp goto NOIMG
set IMAGES=%TEMP%\images.temp.c
pbres -c %IMAGES% demo/images/*.bmp
if errorlevel 1 goto error
:NOIMG

rem 
C:\PBSDK\src\swigwin-1.3.40\swig.exe  -python -py3 inkview.i

gcc -shared -o _inkview.dll inkview_wrap.c -IC:\PBSDK\src\Python-3.1.1-pc -IC:\PBSDK\src\Python-3.1.1-pc\Include  %LIBS% %IMAGES% -I/usr/lib/python3.0 C:\PBSDK\src\Python-3.1.1-pc\libpython3.1.dll.a 
if errorlevel 1 goto error

exit 0

:error
exit 1

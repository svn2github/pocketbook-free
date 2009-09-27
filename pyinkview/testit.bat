@echo off

if not %POCKETBOOKSDK%.==. goto C1
echo Environment variable POCKETBOOKSDK is not set
pause
:C1
set PATH=%POCKETBOOKSDK%\bin;%PATH%

set PYTHONPATH=..:.

C:\PBSDK\src\Python-3.1.1-pc\python.exe  demo\inkdemo.py
rem C:\PBSDK\src\Python-3.1.1-pc\python.exe  demo\grays.py

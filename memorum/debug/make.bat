set PROJECT=Debug\Memorum.exe

set CXX=g++
set CXXFLAGS=-static -Wall -O0 -g
set LIBS=-linkview -lfreetype -ljpeg -lz -lgdi32
set INCLUDE=-I\usr\include
set SRC=src/*.cpp
set IMGS=

set POCKETBOOKSDK=D:\PBSDK
set PATH=%POCKETBOOKSDK%\bin;%PATH%

if not exist %SRC% cd ..

rm -f %PROJECT%

if not exist images\*.bmp goto NOIMG
set IMGS=images/images.c
pbres -c %IMGS% images/*.bmp
if errorlevel 1 goto L_ER

:NOIMG
%CXX% %CXXFLAGS% %INCLUDE% -o %PROJECT% %SRC% %IMGS% %LIBS%
if errorlevel 1 goto L_ER
GC -dir-src -no-cmt_decl- -no-cmt_add_fct_def_class- -no-cmt_add_file- -no-cmt_add_gc_tag- -cmt_dont_modify- -no-pp_include_unix- -space_if- -no-stmt_concat_switch- -code_len-115 -pp_style-1
exit 0

:L_ER
pause
exit 1


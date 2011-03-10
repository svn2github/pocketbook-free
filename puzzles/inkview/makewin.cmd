@echo off

set THEGAME=%1

set PATH=%POCKETBOOKSDK%\bin;%PATH%

set LIBS=-linkview -lfreetype -lcurl -ljpeg -lz -lgdi32

set GGG=../%THEGAME%.c ../malloc.c ../drawing.c ../midend.c ../misc.c ../random.c ../tree234.c ../dsf.c ../laydomino.c ../combi.c ../maxflow.c ../divvy.c ../latin.c ../version.c ../printing.c

gcc -static -Wall -O2 -fomit-frame-pointer -o %THEGAME%.exe ink.c %GGG% %LIBS% -D%THEGAME%_GAME

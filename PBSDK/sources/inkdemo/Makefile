LIBDIR = -L/lib -L/usr/lib -L../../lib
LIBS = -linkview -lfreetype -ljpeg -lz #-lgdi32
OUT = inkdemo.pc
INCLUDE=-I/arm-linux/include #-I./src
ARMLIBDIR = -L/lib -L/usr/lib -L../../arm-linux/lib
ARMLIBS = -linkview -lfreetype -lz
ARMOUT = inkdemo
CFILES = src/*.c
IMAGES =

CC = gcc
CFLAFS = -Wall -O2 -fomit-frame-pointer

#if not exist images\*.bmp goto NOIMG
#set IMAGES=%TEMP%\images.temp.c
#pbres -c %IMAGES% images/*.bmp
#if errorlevel 1 goto L_ER
#:NOIMG

all: pc arm

pc inkdemo.pc: $(CFILES)
	$(CC) -static $(CFLAGS) $(LIBDIR) -o $(OUT) $(CFILES) $(IMAGES) $(LIBS)

arm inkdemo: $(CFILES)
	$(CC) $(CFLAGS) $(INCLUDE) $(ARMLIBDIR) -o $(ARMOUT) $(CFILES) \
	$(IMAGES) $(ARMLIBS)

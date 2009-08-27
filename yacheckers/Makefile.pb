OS=$(shell uname | tr a-z A-Z | sed 's/_.*//')

ifeq (${OS},CYGWIN)

SDKDIR = %POCKETBOOKSDK%
#C:/PBSDK
LIBDIR=-L$(SDKDIR)/lib
LIBS = -linkview -lfreetype -ljpeg -lz -lgdi32 -lsupc++
INCLUDE = -I$(SDKDIR)/include -DNO_STL
OUT = pbcheckers
WINEOUT = $(OUT).exe.so
CC = gcc
CXX = g++
CFLAGS = -Wall -fomit-frame-pointer -O2
WINEFLAGS = -mwindows -m32

ARMCC = /arm-linux/bin/gcc
ARMCXX = /arm-linux/bin/g++
ARMLIBDIR = -L/arm-linux/lib
ARMINCLUDE = -I/arm-linux/include

else

SDKDIR = /usr/local/pocketbook
LIBDIR=-L$(SDKDIR)/lib
LIBS = -linkview -ljpeg `freetype-config --libs` -lsupc++
INCLUDE = -I$(SDKDIR)/include `freetype-config --cflags` -DNO_STL
OUT = pbcheckers
WINEOUT = $(OUT).exe.so
CC = winegcc
CXX = wineg++
CFLAGS = -Wall -fomit-frame-pointer -O2
WINEFLAGS = -mwindows -m32

ARMCC = $(SDKDIR)/bin/arm-linux-gcc
ARMCXX = $(SDKDIR)/bin/arm-linux-g++
ARMLIBDIR = -L$(SDKDIR)/arm-linux/lib $(LIBDIR)
ARMINCLUDE = -I$(SDKDIR)/arm-linux/include  $(INCLUDE)

ARMOUT = $(OUT).app

endif


CPPFILES = src/checkers.cc \
	src/rcheckers.cc\
	src/echeckers.cc\
	src/pbcheckers.cc
IMGC = images/bbb.c
IMGOBJ = $(IMGC:.c=.o)

IMGSRC =
IMAGES = $(wildcard ./images/*.bmp)

all: pc arm
pc: $(OUT)
arm: $(ARMOUT)

ifneq "$(IMAGES)" ""
IMGSRC = $(IMGC)
$(IMGC): $(IMAGES)
	$(SDKDIR)/bin/pbres -c $@ $(IMAGES)
endif


$(OUT): $(CPPFILES) $(IMGC)
	$(CC) -o $@ $^ $(INCLUDE) $(LIBDIR) $(LIBS) $(CFLAGS) $(WINEFLAGS)

$(ARMOUT): $(CPPFILES) $(IMGC)
	$(ARMCC) $(CFLAGS) $(ARMINCLUDE) -o $@ $^  $(ARMLIBDIR) $(LIBS)

clean:
	-rm -f $(ARMOUT) $(OUT) $(WINEOUT) $(IMGSRC)

.PHONY: all pc arm clean


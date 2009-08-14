SDKDIR = /usr/local/pocketbook
LIBDIR= -L/usr/lib -L/home/fax/pb/workspace/libical/lib/ -L$(SDKDIR)/lib
LIBS = -Wl,-Bstatic -lical -Wl,-Bdynamic -linkview -ljpeg -lpthread `freetype-config --libs` `libpng12-config --libs` 
#LIBS = -linkview -ljpeg  -static -lical -dynamic `freetype-config --libs` `libpng12-config --libs`
INCLUDE = -I$(SDKDIR)/include `freetype-config --cflags` \
	  `libpng-config --cflags` -I/usr/include 
OUT = calendar
WINEOUT = $(OUT).exe.so
CC = wineg++
CCC = winegcc
CFLAGS = -Wall -fomit-frame-pointer -O2 -Wno-write-strings -DDEBUG
WINEFLAGS = -m32 -mwindows
CFILES = $(wildcard src/*.cpp)
OFILES = $(CFILES:%.cpp=%.o)
IMGC = src/images.temp.c
IMGO = src/images.temp.o
IMGOARM = src/images.temp.arm.o

ARMCC=$(SDKDIR)/bin/arm-linux-g++
ARMCCC=$(SDKDIR)/bin/arm-linux-gcc
#ARMCC=/opt/cross/bin/arm-linux-gnueabi-g++

ARMLIBDIR = -L$(SDKDIR)/arm-linux/lib $(LIBDIR) -L$(SDKDIR)/lib
ARMINCLUDE = -I$(SDKDIR)/arm-linux/include -I$(SDKDIR)/arm-linux/include/c++/3.4/ -I$(SDKDIR)/arm-linux/include/c++/3.4/arm-linux-gnu/
ARMOUT = $(OUT).app
IMGSRC =
IMAGES = $(wildcard images/*.bmp)

pc: $(WINEOUT)
#all: pc arm
all: pc
arm: $(ARMOUT)

ifneq "$(IMAGES)" ""
IMGSRC = $(IMGC)
$(IMGC): $(IMAGES)
	$(SDKDIR)/bin/pbres -c $@ $^
endif

$(IMGO): $(IMGC)
	$(CCC) -c $(INCLUDE) $(LIBDIR) $(LIBS) $(CFLAGS) $(WINEFLAGS) $^ -o $@

%.o: %.cpp
	@ echo Compiling $@...
	@ $(CC) -o $@ -c  $^ $(INCLUDE) $(CFLAGS) $(WINEFLAGS)

$(WINEOUT): $(OFILES) $(IMGO)
	$(CC) -o $@ $^  $(WINEFLAGS) $(LIBDIR) $(LIBS) $(CFLAGS)

$(IMGOARM): $(IMGC)
	$(ARMCCC) -c $(ARMINCLUDE) $(ARMLIBDIR) $(LIBS) $(CFLAGS) $(INCLUDE) $^ -o $@

$(ARMOUT): $(CFILES) $(IMGOARM)
	$(ARMCC) -o $@ $^ $(CFLAGS) $(ARMINCLUDE) $(ARMLIBDIR) $(LIBS) $(INCLUDE)

clean:
	-rm -f $(ARMOUT) $(OUT) $(WINEOUT) $(IMGSRC) $(OFILES)

.PHONY: all pc arm clean

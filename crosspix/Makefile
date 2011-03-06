SDKDIR = /usr/local/pocketbook
LIBDIR=-L$(SDKDIR)/lib
LIBS = -lstdc++ -linkview -ljpeg `freetype-config --libs`
INCLUDE = -I$(SDKDIR)/include `freetype-config --cflags`
OUT = crosspix-wine.exe
WINEOUT = $(OUT).exe.so
CC = winegcc
CPP= wineg++
CPPFLAGS = -Wall -Wno-write-strings -fomit-frame-pointer -O2
WINEFLAGS = -mwindows -m32
CPPFILES = src/*.cpp
IMGCPP = src/images.temp.cpp
#ARMCC = arm-linux-gnu-gcc
ARMCC = $(SDKDIR)/bin/arm-linux-gcc
ARMLIBDIR = -L$(SDKDIR)/arm-linux/lib $(LIBDIR)
ARMINCLUDE = -I$(SDKDIR)/arm-linux/include $(INCLUDE)
ARMOUT = $(OUT).app
IMGSRC =
IMAGES = $(wildcard images/*.bmp)

all: pc arm
pc: $(OUT)
arm: $(ARMOUT)

ifneq "$(IMAGES)" ""
IMGSRC = $(IMGC)
$(IMGC): $(IMAGES)
	$(SDKDIR)/bin/pbres -c $@ $^
endif

$(OUT): $(CPPFILES) $(IMGSRC)
	$(CPP) -o $@ $^ $(INCLUDE) $(LIBDIR) $(LIBS) $(CPPFLAGS) $(WINEFLAGS)

$(ARMOUT): $(CPPFILES) $(IMGSRC)
	$(ARMCC) $(CPPFLAGS) $(ARMINCLUDE) $(ARMLIBDIR) $(LIBS) -o $@ $^

clean:
	-rm -f $(ARMOUT) $(OUT) $(WINEOUT) $(IMGSRC)

.PHONY: all pc arm clean

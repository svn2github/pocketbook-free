SDKDIR = /usr/local/pocketbook
LIBDIR=-L$(SDKDIR)/lib
LIBS = -linkview -ljpeg `freetype-config --libs` -lsupc++
INCLUDE = -I$(SDKDIR)/include `freetype-config --cflags`
OUT = Lines
WINEOUT = $(OUT).exe.so
CC = winegcc
CFLAGS = -Wall -fomit-frame-pointer -O2
WINEFLAGS = -mwindows -m32
CFILES = src/*.c
IMGC = images/images.temp.c
#ARMCC = arm-linux-gnu-gcc
ARMCC = $(SDKDIR)/bin/arm-linux-gcc
ARMLIBDIR = -L$(SDKDIR)/arm-linux/lib $(LIBDIR)
ARMINCLUDE = -I$(SDKDIR)/arm-linux/include $(INCLUDE)
ARMOUT = $(OUT).app
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

$(OUT): $(CFILES) $(IMGSRC)
	$(CC) -o $@ $^ $(INCLUDE) $(LIBDIR) $(LIBS) $(CFLAGS) $(WINEFLAGS)

$(ARMOUT): $(CFILES) $(IMGSRC)
	$(ARMCC) $(CFLAGS) $(ARMINCLUDE) $(ARMLIBDIR) $(LIBS) -o $@ $^

clean:
	-rm -f $(ARMOUT) $(OUT) $(WINEOUT) $(IMGSRC)

.PHONY: all pc arm clean


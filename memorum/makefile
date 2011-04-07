OS=$(shell uname | tr a-z A-Z | sed 's/_.*//')

ifeq (${OS},CYGWIN)

#old
# All Target
all: 
	pbres -c images/temp.c images/*.bmp
	gcc -static -Wall -O2 -fomit-frame-pointer -o memorum.exe src/*.c images/temp.c -linkview -lfreetype -ljpeg -lz -lgdi32

# Other Targets
clean:
	rm -f memorum.exe

else

SDKDIR = /usr/local/pocketbook
LIBDIR=-L$(SDKDIR)/lib
LIBS = -linkview -ljpeg `freetype-config --libs` -lstdc++
INCLUDE = -I$(SDKDIR)/include `freetype-config --cflags`
OUT = memorum
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


SRC = src/*.cpp

#IMGC = images/temp.c
#IMGOBJ = $(IMGC:.c=.o)

OBJ = $(addprefix obj/,$(notdir $(SRC:.cpp=.o)))

IMGSRC =
#IMAGES = $(wildcard ./images/*.bmp)

all: pc arm
pc: $(OUT)
arm: $(ARMOUT)

$(OUT): $(SRC) $(IMGC)
	$(CC) -o $@ $^ $(INCLUDE) $(LIBDIR) $(LIBS) $(CFLAGS) $(WINEFLAGS)

$(ARMOUT): $(SRC) $(IMGC)
	$(ARMCC) $(CFLAGS) $(ARMINCLUDE) -o $@ $^  $(ARMLIBDIR) $(LIBS)

clean:
	-rm -f $(ARMOUT) $(OUT) $(WINEOUT) $(IMGSRC)

.PHONY: all pc arm clean



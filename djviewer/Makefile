SDKDIR = ../../PBSDK-linux-libwine

# common data
CFLAGS = -Wall -fomit-frame-pointer -O2

# platform specific data
ifeq ($(MAKECMDGOALS), arm)
	CC = arm-linux-gnu-gcc
	CXX = arm-linux-gnu-g++
	LIBDIR = -L$(SDKDIR)/arm-linux/lib -Llib/arm
	LIBS = -linkview -ljpeg -ldjvulibre -lpthread -lfreetype -lz
	INCLUDE = -I$(SDKDIR)/include -Iinclude -I$(SDKDIR)/arm-linux/include -I$(SDKDIR)/arm-linux/include/freetype2
	OUT = djviewer.app
	OBJS = objarm
else
	CC = winegcc
	CXX = wineg++
	LIBDIR = -L$(SDKDIR)/lib -Llib/i386
	LIBS = -linkview -ljpeg -ldjvulibre -lpthread `freetype-config --libs` -mwindows
	INCLUDE = -I$(SDKDIR)/include -Iinclude `freetype-config --cflags`
	OUT = djviewer.exe
	OBJS = objwine
endif

TEMPFILES = djviewer.exe djviewer.exe.so djviewer.app objwine objarm

SOURCES = $(wildcard src/*.c) $(wildcard images/*.bmp)
OBJECTS = $(patsubst src/%.c, %.o, $(wildcard src/*.c)) $(patsubst src/%.cpp, %.o, $(wildcard src/*.cpp)) $(patsubst images/%.bmp, %.o, $(wildcard images/*.bmp))

VPATH = src images $(OBJS)

wine arm : $(OUT)

%.o : %.c
	mkdir -p $(OBJS)
	$(CC) -c $^ -o $(OBJS)/$@ $(INCLUDE) $(CFLAGS)

%.o : %.cpp
	mkdir -p $(OBJS)
	$(CXX) -c $^ -o $(OBJS)/$@ $(INCLUDE) $(CFLAGS)

%.c : %.bmp
	mkdir -p $(OBJS)
	$(SDKDIR)/bin/pbres -c $@ $^

$(OUT): $(OBJECTS)
	$(CXX) -o $@ $(addprefix $(OBJS)/, $(notdir $^)) $(LIBDIR) $(LIBS)

clean:
	rm -r -f $(TEMPFILES)
	
.PHONY: clean arm wine


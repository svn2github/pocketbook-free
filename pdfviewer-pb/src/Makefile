SDKDIR = ../../PBSDK-linux-libwine

# common data
CFLAGS = -Wall -fomit-frame-pointer -O2

# platform specific data
ifeq ($(MAKECMDGOALS), arm)
	#CC = arm-linux-gnu-gcc
	CC = $(SDKDIR)/bin/arm-linux-gcc
	#CXX = arm-linux-gnu-g++
	CXX = $(SDKDIR)/bin/arm-linux-g++
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

SOURCES = $(wildcard *.c)
OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c)) $(patsubst %.cpp, %.o, $(wildcard *.cpp)) $(patsubst %.cc, %.o, $(wildcard *.cc)) $(patsubst images/%.bmp, %.o, $(wildcard images/*.bmp))

VPATH = $(OBJS)

wine arm : $(OUT)

%.o : %.c
	mkdir -p $(OBJS)
	$(CC) -c $^ -o $(OBJS)/$@ $(INCLUDE) $(CFLAGS)

%.o : %.cpp
	mkdir -p $(OBJS)
	$(CXX) -c $^ -o $(OBJS)/$@ $(INCLUDE) $(CFLAGS)

$(OUT): $(OBJECTS)
	$(CXX) -o $@ $(addprefix $(OBJS)/, $(notdir $^)) $(LIBDIR) $(LIBS)

clean:
	rm -r -f $(TEMPFILES)
	
.PHONY: clean arm wine


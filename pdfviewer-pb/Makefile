SDKDIR = /usr/local/pocketbook

# common data
CFLAGS = -Wall -fomit-frame-pointer -O2

# platform specific data
ifeq ($(MAKECMDGOALS), arm)
	#CC = arm-linux-gnu-gcc
	CC = $(SDKDIR)/bin/arm-linux-gcc
	#CXX = arm-linux-gnu-g++
	CXX = $(SDKDIR)/bin/arm-linux-g++
	LIBDIR = -L$(SDKDIR)/arm-linux/lib -Llib/arm
	LIBS = -linkview -ljpeg -lpthread -lfreetype -lz -lpoppler -lfontconfig -lexpat `freetype-config --libs`
	INCLUDE = -I$(SDKDIR)/include -Iinclude -I$(SDKDIR)/arm-linux/include -I$(SDKDIR)/arm-linux/include/freetype2 -I$(SDKDIR)/arm-linux/include/fontconfig -Ipoppler-0.10-pb -Ipoppler-0.10-pb/poppler
	OUT = pdfviewer.app
	OBJS = objarm
else
	CC = winegcc
	CXX = wineg++
	LIBDIR = -L$(SDKDIR)/lib -Llib/i386
	LIBS = -linkview -ljpeg -lpthread -lfontconfig `freetype-config --libs` -mwindows -lpoppler
	INCLUDE = -I$(SDKDIR)/include -Iinclude `freetype-config --cflags` -Ipoppler-0.10-pb -Ipoppler-0.10-pb/poppler
	OUT = pdfviewer.exe
	OBJS = objwine
endif

TEMPFILES = pdfviewer.exe pdfviewer.exe.so pdfviewer.app objwine objarm

SOURCES = $(wildcard src/*.c) $(wildcard images/*.bmp)
OBJECTS = $(patsubst src/%.cc, %.o, $(wildcard src/*.cc)) $(patsubst src/%.c, %.o, $(wildcard src/*.c)) $(patsubst src/%.cpp, %.o, $(wildcard src/*.cpp)) $(patsubst images/%.bmp, %.o, $(wildcard images/*.bmp))

VPATH = src images $(OBJS)

wine arm : $(OUT)

%.o : %.c
	mkdir -p $(OBJS)
	$(CC) -c $^ -o $(OBJS)/$@ $(INCLUDE) $(CFLAGS)

%.o : %.cpp
	mkdir -p $(OBJS)
	$(CXX) -c $^ -o $(OBJS)/$@ $(INCLUDE) $(CFLAGS)

%.o : %.cc
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


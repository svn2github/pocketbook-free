OUT = minerpro

SDKDIR = /usr/local/pocketbook_eabi
PROJECT = $(OUT).app
OBJDIR=obj_arm_eabi
LIBS = -L$(SDKDIR)/lib -linkview -lMorphology -lfreetype -ljpeg -lz
INCLUDES = -I$(SDKDIR)/include
CC = arm-linux-gnueabi-gcc
CXX = arm-linux-gnueabi-g++
CFLAGS = -Os -D__ARM__ -Wall -fomit-frame-pointer
CXXFLAGS = $(CFLAGS)
LD = arm-linux-gnueabi-gcc
LDFLAGS = -Wl,-s

all: $(PROJECT)

clean:
	rm -rf $(PROJECT)
	rm -rf $(OBJDIR)

SOURCES_C = \
    src/miner.cpp\
    images/images.temp.c

CDEPS = -MT$@ -MF`echo $@ | sed -e 's,\.o$$,.d,'` -MD -MP

OBJS  = \
    $(OBJDIR)/miner.o\
    $(OBJDIR)/images.temp.o

$(OBJDIR):
	mkdir $(OBJDIR)

$(OBJDIR)/%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $(INCLUDES) $(CDEPS) $<

$(OBJDIR)/%.o: images/%.c
	$(CC) -c -o $@ $(CFLAGS) $(INCLUDES) $(CDEPS) $<

$(OBJDIR)/%.o: src/%.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) $(INCLUDES) $(CDEPS) $<

$(PROJECT) : $(OBJDIR) $(OBJS)
	$(LD) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

# Dependencies tracking:
-include $(OBJDIR)/*.d
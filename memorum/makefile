OUT=memorum

ifndef BUILD
BUILD=emu
#BUILD=arm_gnueabi
endif

include /usr/local/pocketbook/common.mk

CXXFLAGS+= -I.. -I/usr/local/pocketbook_eabi/include -I/usr/local/pocketbook_eabi/include/sigc++-2.0 \
-DHAS_NO_IV_GET_DEFAULT_FONT `freetype-config --cflags`

#LDFLAGS+= -L../pbtk/obj_$(BUILD)
#LIBS+=-lpbtk

ifeq (${BUILD},emu)
CXXFLAGS+=-g `pkg-config --cflags sigc++-2.0` -I./../
LIBS+=-g -lsigc-2.0
else 
ifeq (${BUILD},arm)
CXXFLAGS+=-D_OLD_DEV_
LIBS+=-L/usr/local/pocketbook/arm-linux/lib -lsigc-2.0
else
LIBS+=-L/usr/local/pocketbook_eabi/lib -lsigc-2.0
endif
endif

SOURCES=\
 src/calendar.cpp\
 src/cards.cpp\
 src/config.cpp\
 src/main.cpp


PIXMAPS=

PIXMAPS_C=$(PIXMAPS:.xpm=.c)
PIXMAPS_OBJS=$(addprefix $(OBJDIR)/,$(PIXMAPS_C:.c=.o))

all:$(PROJECT)

$(PROJECT): $(PIXMAPS_C) $(OBJDIR) $(OBJS) $(PIXMAPS_OBJS)
	$(CXX) -o $@ $(OBJS) $(PIXMAPS_OBJS) $(LDFLAGS) $(LIBS)

$(OBJDIR):
	mkdir -p $(OBJDIR)/src
	mkdir -p $(OBJDIR)/images

$(OBJDIR)/%.cxx.o: %.cxx
	$(CXX) -c -o $@ $(CXXFLAGS) $(INCLUDES) $(CDEPS) $<
$(OBJDIR)/%.cpp.o: %.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) $(INCLUDES) $(CDEPS) $<

$(OBJDIR)/images/%.o: images/%.c
	$(CC) -c -o $@ $(CFLAGS) $(INCLUDES) $(CDEPS) $<

$(PIXMAPS_C): $(PIXMAPS)

images/%.c: images/%.xpm
	./xpbres -c $@ $<

-include $(OBJDIR)/src/*.d

OUT = poterm

ifndef BUILD
#BUILD=arm_gnueabi
BUILD=emu
endif

include /usr/local/pocketbook/common.mk

CFLAGS+=-D_POCKETBOOK `freetype-config --cflags` -I../ 
CXXFLAGS+=-D_POCKETBOOK `freetype-config --cflags` -I../

LDFLAGS+= -L../pbtk/obj_$(BUILD)
LIBS+=-lpbtk
#-lWildMidi

ifeq ($(BUILD),arm)
CXXFLAGS+=-I/usr/local/pocketbook/arm-linux/include\
  -I/usr/local/pocketbook/arm-linux/include/sigc++-2.0
LDFLAGS+=-L/usr/local/pocketbook/arm-linux/lib -lsigc-2.0
endif

ifeq ($(BUILD),arm_gnueabi)
CXXFLAGS+=-I/usr/local/pocketbook_eabi/include -DHAS_NO_IV_GET_DEFAULT_FONT\
  -I/usr/local/pocketbook_eabi/include/sigc++-2.0
LDFLAGS+=-L/usr/local/pocketbook_eabi/lib -lsigc-2.0
endif

ifeq ($(BUILD),emu)
CXXFLAGS+=`pkg-config --cflags sigc++-2.0`
LDFLAGS+= -lsigc-2.0
endif

SOURCES_CPP = \
    poterm.cpp \
    Term.cpp
#test.cpp

SOURCES_CXX =

SOURCES_C = 
CDEPS = -MT$@ -MF`echo $@ | sed -e 's,\.o$$,.d,'` -MD -MP

OBJS  = $(addprefix $(OBJDIR)/,$(SOURCES_C:.c=.o)) \
        $(addprefix $(OBJDIR)/,$(SOURCES_CXX:.cxx=.o))\
        $(addprefix $(OBJDIR)/,$(SOURCES_CPP:.cpp=.o))


$(OBJDIR)/%.o: %.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) $(INCLUDES) $(CDEPS) $<

$(PROJECT) : $(OBJDIR) $(OBJS)
	$(LD) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

# Dependencies tracking:
-include $(OBJDIR)/*.d

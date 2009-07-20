#
# PoTerm Makefile
# ---------------
#
TRG = poterm

### Common
### ------
SDKDIR   = /usr/local/pocketbook
CFLAGS   = -pipe -Wall -W -Werror -fomit-frame-pointer
CPPFLAGS =  -pipe -Wall -W -Werror -fomit-frame-pointer -fno-exceptions

ifeq ($(HOST),x86)
# x86 specific stuff
# ------------------
SYS       = $(shell uname)
OBJ_DIR   = OBJ_$(SYS)
CC        = winegcc
CLINK     = winegcc
CPP       = wineg++
CPPLINK   = wineg++
CFLAGS   += -g
CPPFLAGS += -g
DEFS      = -DHOST_X86
LFLAGS    = -L$(SDKDIR)/lib -mwindows -m32
INC       = -I$(SDKDIR)/include `freetype-config --cflags`
LIBS      = -linkview -ljpeg `freetype-config --libs`

else
ifeq ($(HOST),arm)
# Arm specific stuff
#-------------------
SYS       = ARM
OBJ_DIR   = OBJ_$(SYS)
PREFIX    = $(SDKDIR)/bin/arm-linux-
CC        = $(PREFIX)gcc
CLINK     = $(PREFIX)gcc
CPP       = $(PREFIX)g++
CPPLINK   = $(PREFIX)g++
CFLAGS   += -O2
CPPFLAGS += -O2
DEFS      = -DHOST_ARM
LFLAGS    = -L$(SDKDIR)/arm-linux/lib
INC       = -I$(SDKDIR)/arm-linux/include
LIBS      = -linkview -ljpeg `freetype-config --libs`

else

all:
	$(error Please specify one of the followong platforms: x86 or arm)

x86:
	make HOST=x86 $(TRG)
arm:
	make HOST=arm $(TRG).app
clean:
	@rm -fr $(TRG) $(TRG).app core* $(TRG).exe* OBJ_* valgrind* *.gdb *.o *.d *.obj *~

endif
endif


SRCS  = poterm.cpp Term.cpp
OBJS  = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

### Load dependecies
### ----------------
DEPS = $(wildcard $(OBJ_DIR)/*.d)
ifneq ($(strip $(DEPS)),)
include $(DEPS)
endif

### Compilation and dependecies generation
### --------------------------------------
define COMPILE
$(CPP) -c -MD $(CPPFLAGS) $(INC) $(DEFS) -o $@ $<
endef
define C_COMPILE
$(CC) -c -MD $(CFLAGS) $(INC) $(DEFS) -o $@ $<
endef


$(OBJ_DIR)/%.o: %.c
		$(C_COMPILE)
$(OBJ_DIR)/%.o: %.cpp
		$(COMPILE)

$(OBJ_DIR):
		mkdir $(OBJ_DIR)

$(TRG):		$(OBJ_DIR) $(OBJS)
		$(CPPLINK) -o $@ $(LFLAGS) $(OBJS) $(LIBS)
$(TRG).app:	$(OBJ_DIR) $(OBJS)
		$(CPPLINK) -o $@ $(LFLAGS) $(OBJS) $(LIBS)

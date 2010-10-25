#**********************************************************************************************
#    Copyright (C) 2010 Stephan Olbrich reader42@gmx.de
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#**********************************************************************************************
#
# telnet_starter Makefile
# ---------------
#
TRG = telnet_starter

### Common
### ------
SDKDIR   = /usr/local/pocketbook/
#CFLAGS   = -pipe -Wall -W -Werror -fomit-frame-pointer
#CPPFLAGS =  -pipe -Wall -W -Werror -fomit-frame-pointer -fno-exceptions
CFLAGS   = -pipe -Wall -W -fomit-frame-pointer
CPPFLAGS =  -pipe -Wall -W -fomit-frame-pointer -fno-exceptions

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
INC       = -I$(SDKDIR)/include `freetype-config --cflags` -I/usr/include/c++/4.4/bits/
#LIBS      = -linkview -ljpeg `freetype-config --libs`
LIBS      = -linkview `freetype-config --libs` -lpng -ljpeg -lpthread -lcurl

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
LIBS      = -linkview -lpng `freetype-config --libs` -ljpeg

else

all:
	$(error Please specify one of the followong platforms: x86 or arm)

x86:
	make HOST=x86 $(TRG)
arm:
	make HOST=arm $(TRG).app
clean:
	@rm -fr $(TRG) $(TRG).app core* $(TRG).exe* OBJ_* valgrind* *.gdb *.o *.d *.obj *~
test:
	echo $(OBJS)

endif
endif

SRC_DIR = src

SRCS  = main.cpp MainView.cpp
SRCS_C = 
OBJS  = $(SRCS:%.cpp=$(OBJ_DIR)/%.o) $(SRCS_C:%.c=$(OBJ_DIR)/%.o)


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


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
		$(C_COMPILE)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
		$(COMPILE)

$(OBJ_DIR):
		mkdir $(OBJ_DIR)

$(TRG):		$(OBJ_DIR) $(OBJS)
		$(CPPLINK) -o $@ $(LFLAGS) $(OBJS) $(LIBS)
$(TRG).app:	$(OBJ_DIR) $(OBJS)
		$(CPPLINK) -o $@ $(LFLAGS) $(OBJS) $(LIBS)

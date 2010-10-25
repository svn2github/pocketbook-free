# Makefile for utelnetd
# 
# Configure this with the following environment variables:
#

# where to install
INSTDIR		:= /usr/local/pocketbook/mnt/ext1/games/

# GNU target string 
CROSS		:= arm-linux-
#CROSS		:= 

DEBUG := 1

ifeq ($(CROSS),arm-linux-)

# where to find login program
LOGIN		:= /mnt/ext1/games/bash

else

ifneq ("", "$(BSD)")
LOGIN		:= /usr/bin/login
else
LOGIN		:= /bin/bash
endif

endif


ifneq ("", "$(BSD)")
CORE		:= utelnetd.core
else
CORE		:= core
endif


# nothing to configure below this line... ---8<---8<---8<---

PROGS     = utelnetd

INSTMODE  = 0755
INSTOWNER = root
INSTGROUP = root

OBJS      = utelnetd.o

CC        = $(CROSS)gcc
INSTALL   = install


CFLAGS	 += -I. -pipe -DSHELLPATH=\"$(LOGIN)\" -Wall

ifeq ($(CROSS),arm-linux-)

ifneq ("","$(DEBUG)")
CFLAGS   += -DDEBUG -g -Os -pipe -Wall -W -fomit-frame-pointer
STRIP	  = \#
else
CFLAGS	 += -fomit-frame-pointer -pipe -Wall -W
STRIP	  = $(CROSS)strip 
endif

else

ifneq ("","$(DEBUG)")
CFLAGS   += -DDEBUG -g -Os
STRIP	  = \#
else
CFLAGS	 += -fomit-frame-pointer
STRIP	  = $(CROSS)strip 
endif

endif

ifeq ("1", "$(BSD)")
CFLAGS   += -DBSD
endif


all: $(PROGS)

$(PROGS): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@
	$(STRIP) --remove-section=.comment --remove-section=.note $@

.PHONY: install
install: $(PROGS)
	$(INSTALL) -d $(INSTDIR)
	$(INSTALL) -m $(INSTMODE) -o $(INSTOWNER) -g $(INSTGROUP) $(PROGS) $(INSTDIR)

.PHONY: clean
clean:
	rm -f $(PROGS) *.o $(CORE)

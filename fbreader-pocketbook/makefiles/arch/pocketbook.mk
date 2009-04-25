include $(ROOTDIR)/makefiles/arch/unix.mk

ifeq "$(INSTALLDIR)" ""
  INSTALLDIR=/usr
endif
IMAGEDIR = $(INSTALLDIR)/share/pixmaps
APPIMAGEDIR = $(INSTALLDIR)/share/pixmaps/%APPLICATION_NAME%

CFLAGS = -pipe -fno-exceptions -Wall -Wno-ctor-dtor-privacy -W \
	 -DLIBICONV_PLUG -DARM -DXMLCONFIGHOMEDIR=\"/home\"

AR = ar rsu

ifneq "$(EMULATOR)" "1"
  CC = arm-linux-gnu-gcc
  LD = arm-linux-gnu-g++
  LDFLAGS = -L/usr/local/pocketbook/arm-linux/lib
  EXTERNALINCLUDE = -I/usr/local/pocketbook/arm-linux/include
else
  CC = gcc
  LD = wineg++
  LDFLAGS += -mwindows -m32
  LDFLAGS = -L/usr/local/pocketbook/lib
  EXTERNALINCLUDE = -I/usr/local/pocketbook/include `freetype-config --cflags`
endif

ifeq "$(UI_TYPE)" "nanox"
  UILIBS = -L$(ROOTDIR)/v3/arm/lib -lfreetype -lpthread -ljpeg -lpng -lungif
  NXINCLUDE =
  ZLSHARED = no
endif

XML_LIB = -L$(ROOTDIR)/v3/arm/lib -lexpat
ARCHIVER_LIB = -L$(ROOTDIR)/v3/arm/lib -lz -lbz2

RM = rm -rvf
RM_QUIET = rm -rf

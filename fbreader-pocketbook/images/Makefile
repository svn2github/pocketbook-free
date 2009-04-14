ROOTDIR = $(CURDIR)/..
MAKEFILESDIR = $(ROOTDIR)/makefiles
include $(MAKEFILESDIR)/config.mk

SDKDIR = /usr/local/pocketbook
IMGC = c/images.c
IMGO = c/images.o
IMAGES = $(wildcard *.bmp)
CFLAGS = -c -O2 -fomit-frame-pointer $(INCLUDE) #-Wall
#EMUCFLAGS = -DEMULATOR -DMODEL="\"i386\"" -DFWVERSION="\"internal\""

$(IMGO): $(IMGC)
	$(CC) $(CFLAGS) -o $@ $^

$(IMGC): $(IMAGES)
	$(SDKDIR)/bin/pbres -c $@ $^

clean:
	rm -f $(IMGC) $(IMGO)

.PHONY: clean

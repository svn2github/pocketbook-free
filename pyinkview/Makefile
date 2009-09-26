# Makefile
PROJECT=_inkview.dll

PBSDKDIR=/cygdrive/c/PBSDK
PYTHONDIR=/cygdrive/C/PBSDK/src/Python-3.1.1-pc
SWIGDIR=/cygdrive/c/PBSDK/src/swigwin-1.3.40

INCLUDE=-I$(PBSDKDIR)/usr/include -I$(PYTHONDIR) -I$(PYTHONDIR)/Include -I./libffi
LIBS=-linkview -lfreetype -ljpeg -lz -lgdi32 $(PYTHONDIR)/libpython3.1.dll.a -L./libffi -lffi
#-L$(PBSDKDIR)/lib 

IMAGES=$(foreach dir, demo/images/, $(wildcard demo/images/*.bmp))
INTERFACES=$(foreach dir, ./, $(wildcard ./*.i))

SOURCES=inkview_wrap.c images.temp.c
OBJECTS=$(SOURCES:.c=.o)

CC=gcc
CCFLAGS=-Wall -g -O0

SWIG=$(SWIGDIR)/swig
PBRES=pbres

all: $(PROJECT)

$(PROJECT): $(OBJECTS)
	$(CC) $(CCFLAGS) -shared -o $(PROJECT) $(OBJECTS) $(LIBS)

%.o: %.c
	$(CC) $(CCFLAGS) $(INCLUDE) -c $< -o $@

inkview_wrap.c: $(INTERFACES)
	$(SWIG) -python -py3 inkview.i

inkview_wrap.o: inkview_wrap.c
	$(CC) $(INCLUDE) -c inkview_wrap.c -o $@

images.temp.c: $(IMAGES)
	$(PBRES) -c $@ $(IMAGES)

clean:
	rm -f *.exe
	rm -f *.o *.o
	rm -f *.dll *.so
	rm -f *.pyc
	rm -f images.temp.c
	rm -f inkview_wrap.c inkview.py
	rm -f *.stackdump

	
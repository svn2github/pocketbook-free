# PBSDK Makefile

TARGET=_inkview.dll

INCLUDE=-I/usr/include -I/usr/local/include/python3.1 -I/usr/local/lib/libffi-3.0.9/include
LIBS=-linkview -lfreetype -ljpeg -lz -lgdi32 -L/usr/local/lib -lpython3.1.dll -lffi

IMAGES=$(foreach dir, demo/images/, $(wildcard demo/images/*.bmp))
INTERFACES=$(foreach dir, ./, $(wildcard ./*.i))

SOURCES=inkview_wrap.c callback.c #images.temp.c 
OBJECTS=$(SOURCES:.c=.o)

CC=gcc
CCFLAGS=-Wall -g -O0

SWIG=/cygdrive/c/PBSDK/src/swigwin-1.3.40/swig
PBRES=pbres

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CCFLAGS) -shared -o $(TARGET) $(OBJECTS) $(LIBS)

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

	
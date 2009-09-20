# Makefile
PROJECT=shulte.exe

INCLUDE=-I/cygdrive/c/PBSDK/usr/include 
LIBS=-linkview -lfreetype -ljpeg -lz -lgdi32
#-L/cygdrive/c/PBSDK/lib 
SOURCES=$(foreach dir, src/, $(wildcard src/*.cc))
OBJECTS = $(patsubst src/%,obj/%,$(SOURCES:.cc=.o))

CXX=g++
CXXFLAGS=-static -Wall -g -O0

all: $(PROJECT)

$(PROJECT): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(PROJECT) $(OBJECTS)  $(LIBS)

obj/%.o: src/%.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f *.exe
	rm -f obj/*.o src/*.o
	
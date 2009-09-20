PROJECT=hulte.exe
INCLUDE=-I/cygdrive/c/PBSDK/usr/include 
LIBS=-linkview -lfreetype -ljpeg -lz -lgdi32
#-L/cygdrive/c/PBSDK/lib 

all: $(PROJECT)

$(PROJECT): src/testhulte.cc src/NumericTable.cc
	g++ -static -Wall -g -O0 $(INCLUDE) -o $(PROJECT) src/*.cc $(LIBS)

clean:
	rm -f *.exe
	
	
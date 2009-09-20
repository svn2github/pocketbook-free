# All Target
all: 
	pbres -c temp.c ../images/*.bmp
	gcc -static -Wall -O0 -g -o Lines.exe ../src/*.c temp.c -linkview -lfreetype -ljpeg -lz -lgdi32

# Other Targets
clean:
	rm -f Lines.exe
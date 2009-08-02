# All Target
all: 
	pbres -c images/temp.c images/*.bmp
	gcc -static -Wall -O2 -fomit-frame-pointer -o PSokoban.exe src/*.c images/temp.c -linkview -lfreetype -ljpeg -lz -lgdi32

# Other Targets
clean:
	rm -f PSokoban.exe
	
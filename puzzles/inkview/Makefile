SDKDIR = /usr/local/pocketbook_eabi

CC = arm-linux-gnueabi-gcc
CFLAGS = -Wall -O2 -fomit-frame-pointer -std=c99 -D_XOPEN_SOURCE=600
INCLUDES = -I$(SDKDIR)/include -DHOST_ARM_PRO
LIBS = -L$(SDKDIR)/lib -linkview -lMorphology -lfreetype -ljpeg -lz

OBJECTS = malloc.o drawing.o midend.o misc.o random.o printing.o tree234.o dsf.o laydomino.o combi.o maxflow.o divvy.o latin.o version.o

DOIT = $(CC) $(CFLAGS) $(INCLUDES) -c ink.c -o ink.o -D`echo $< |cut -d. -f1 |tr a-z A-Z`_GAME; \
       $(CC) -o $@ ink.o $^ $(LIBS)

all: blackbox.app bridges.app cube.app dominosa.app fifteen.app filling.app flip.app galaxies.app guess.app     \
		inertia.app lightup.app map.app mines.app net.app netslide.app pattern.app \
		pegs.app rect.app samegame.app sixteen.app slant.app solo.app tents.app\
		twiddle.app unequal.app keen.app magnets.app signpost.app singles.app towers.app 

clean:
	rm *.o *.app

%.app: %.o $(OBJECTS)
	$(DOIT)

%.o: ../%.c ../puzzles.h
	$(CC) $(CFLAGS) -c $< -o $@


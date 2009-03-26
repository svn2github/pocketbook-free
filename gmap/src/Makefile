SRC = main.c goog.c raster.c
OBJ = $(SRC:.c=.o)
OUT = gmap

LDLIBS = -linkview -lfreetype -lpng -lz
LDFLAGS = -s

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	-rm -f *.o $(OUT)

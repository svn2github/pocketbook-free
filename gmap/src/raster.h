#ifndef _raster_h_
#define _raster_h_

#include <inkview.h>

typedef struct raster_s {
	int w,h;
	char* data;
} raster;

typedef raster * praster;

praster load_png_grayscale(const char* fname); 
praster load_jpg_grayscale(const char* fname);
praster create_empty_raster(int width, int height, char fill);
void delete_raster(praster r);
void show_raster(praster r, int x0, int y0,
        int xmin, int ymin, int xmax, int ymax);

#endif

#ifndef _raster_h_
#define _raster_h_

#include <inkview.h>

struct raster {
	int w,h;
	char* data;
};

typedef struct raster * praster;

praster load_png_grayscale(const char* fname); 
praster load_jpg_grayscale(const char* fname);
void delete_raster(praster r);
void show_raster(praster r, int x0, int y0,
        int xmin, int ymin, int xmax, int ymax);

#endif

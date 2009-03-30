#ifndef _goog_h_
#define _goog_h_

#include <stdint.h>
#include <stdio.h>
#include <inkview.h>

#define TILE_SIZE 256
#define MAX_ZOOM 17
#define MIN_ZOOM -2

#define LAYER_MAP 0
#define LAYER_SATELLITE 1
#define LAYER_TERRAIN 2

void coord2xy(double lat, double lon, uint32_t *px, uint32_t *py);
void xy2coord(uint32_t x, uint32_t y, double *plat, double *plon);
// char buf[FILENAME_MAX]
char* tile_file_name(char* buf, unsigned tx, unsigned ty, int zoom, int layer);

uint32_t tile_pixel_size(int zoom);
uint32_t tile_count(int zoom);
double km_per_pixel(uint32_t x, uint32_t y, int zoom);
#endif

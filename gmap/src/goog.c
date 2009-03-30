#include <math.h>
#include <stdint.h>
#include "goog.h"

#include <stdio.h>

void coord2xy(double lat, double lon, uint32_t *px, uint32_t *py) {
	double xx, yy, e;
	xx=2*remainder(lon / 360.0, 1.0); // -1...1
	e=sin(lat*M_PI/180.0);
	if(e==1 || e==-1) {
		yy=-e;
	} else {
		yy=-log((1+e)/(1-e))/2/M_PI; // -1...1
	}
	xx=floor((xx+1)*2147483648.); // 0..2^32
	if(xx<0) xx=0;
	if(xx>=4294967296.) xx=4294967295.;
	yy=floor((yy+1)*2147483648.0); // 0..2^32
	if(yy<0) yy=0;
	if(yy>=4294967296.) yy=4294967295.;
	*px=(uint32_t)(long long)xx;
	*py=(uint32_t)(long long)yy;
}

void xy2coord(uint32_t x, uint32_t y, double *plat, double *plon) {
	double xx,yy,e,f;
	xx=((double)x)/2147483648. - 1.; // -1...1
	yy=((double)y)/2147483648. - 1.; // -1...1
	*plon=xx*180.;
	f=exp(-yy*2*M_PI); // (1+e)/(1-e)=f => 1+e=f-fe => (1+f)e=f-1
	e=(f-1)/(f+1);
	*plat=180.0/M_PI*asin(e);
}

#define EARTH_RADIUS 6371.

double km_per_pixel(uint32_t x, uint32_t y, int zoom) {
	double lat,lon;
	xy2coord(x,y,&lat,&lon);
	return 2*M_PI*EARTH_RADIUS/tile_count(zoom)/TILE_SIZE * cos(lat*M_PI/180.);
}

uint32_t tile_pixel_size(int zoom) {
	return 1UL<<(32-MAX_ZOOM+zoom-8);
}

uint32_t tile_count(int zoom) {
	return 1UL<<(MAX_ZOOM-zoom);
}

static const char layer_names[][20] = {
	"tiles_Map",
	"tiles_Satellite",
	"tiles_Terrain"
};

char* tile_file_name(char* buf, unsigned tx, unsigned ty, int zoom, int layer) {
        extern char map_dir[];
	int tc=tile_count(zoom);
	tx%=tc; if(tx<0) tx+=tc;
	ty%=tc; if(ty<0) ty+=tc;
	fprintf(stderr,"%d %d %d %d\n",tx,ty,zoom,layer);
	snprintf(buf, FILENAME_MAX, "%s/%s/%d/%d/%d/%d/%d.png",
	                map_dir,
			layer_names[layer], 
                        zoom, 
                        tx/1024, tx%1024, 
                        ty/1024, ty%1024);
	return buf;
}

#include <inkview.h>

#include "main.h"
#include "raster.h"
#include "goog.h"
#include "locations.h"

/***** STATE & CONFIG *****/
uint32_t centerx, centery;
int zoom, layer;
int orientation;
int last_location;
char map_dir[256] = MAP_DIR;

/***** TILE CACHE ****/
struct cached_tile {
	uint32_t tx,ty;
	int zoom,layer;
	praster r;
};

struct cached_tile* tile_cache = NULL;

unsigned hashvalue(uint32_t tx, uint32_t ty, int zoom, int layer) {
	return (tx+ty*31+zoom*415+layer*926) % TILE_CACHE_SIZE;
}

void create_tile_cache() {
	unsigned i;
	tile_cache=malloc(TILE_CACHE_SIZE*sizeof(struct cached_tile));
	if(!tile_cache) return;
	for(i=0; i<TILE_CACHE_SIZE; ++i) {
		tile_cache[i].zoom=MAX_ZOOM+42; // to avoid accidental hits
		tile_cache[i].r=NULL;
	}
}

void destroy_tile_cache() {
	unsigned i;
	if(!tile_cache) return;
	for(i=0; i<TILE_CACHE_SIZE; ++i) 
		if(tile_cache[i].r!=NULL) 
			delete_raster(tile_cache[i].r);
	free(tile_cache);
	tile_cache=NULL;
}

#define NOT_FOUND ((void*)(-1))
praster get_tile_from_cache(uint32_t tx, uint32_t ty, int zoom, int layer) {
	if(!tile_cache) return NOT_FOUND;
	unsigned h=hashvalue(tx, ty, zoom, layer);
	struct cached_tile* p=&tile_cache[h];
	if(p->tx==tx && p->ty==ty && p->zoom==zoom && p->layer==layer) return p->r;
	return NOT_FOUND;
}

void put_tile_to_cache(uint32_t tx, uint32_t ty, int zoom, int layer, praster r) {
	if(!tile_cache) return;
	unsigned h=hashvalue(tx, ty, zoom, layer);
	struct cached_tile* p=&tile_cache[h];
	if(p->r) delete_raster(p->r);
	p->tx=tx;
	p->ty=ty;
	p->zoom=zoom;
	p->layer=layer;
	p->r=r;
}

/***** DRAW *****/
char filename[FILENAME_MAX];
praster get_tile(uint32_t tx, uint32_t ty, int zoom, int layer) {
	praster r=get_tile_from_cache(tx,ty,zoom,layer);
	if(r!=NOT_FOUND) {
		fprintf(stderr,"Cache hit!\n");
		return r;
	}
	r=NULL;
	tile_file_name(filename, tx, ty, zoom, layer);
	fprintf(stderr,"Loading file %s\n", filename);	
	r=load_png_grayscale(filename);
	if(!r) r=load_jpg_grayscale(filename);
	put_tile_to_cache(tx,ty,zoom,layer,r);
	return r;
}

void show_tiles(int statusbar_heigth) {
        int xmax=ScreenWidth();
        int ymax=ScreenHeight()-statusbar_heigth;
	uint32_t pixelsz=tile_pixel_size(zoom);
	uint32_t x0=(centerx/pixelsz)-xmax/2,
		 y0=(centery/pixelsz)-ymax/2;
	uint32_t tx0=x0/TILE_SIZE, ty0=y0/TILE_SIZE;
	uint32_t tx,ty;
	int dx,dy;
	fprintf(stderr,"center=(%lu,%lu) zoom=%d\n", centerx, centery, zoom);
	fprintf(stderr, "x0,y0=(%lu,%lu) pixelsz=%lu\n", x0, y0, pixelsz);
	for(dx=tx0*TILE_SIZE-x0,tx=tx0; dx<xmax; dx+=TILE_SIZE,++tx)
	    	for(dy=ty0*TILE_SIZE-y0,ty=ty0; dy<ymax; dy+=TILE_SIZE,++ty) {
			praster r=get_tile(tx, ty, zoom, layer);
			if(r) {
				fprintf(stderr,"show_raster(r,%d,%d)\n",dx,dy);
				show_raster(r, dx,dy, 0,0, xmax,ymax);
			} else {
				fprintf(stderr,"not found\n");
			}
	}
}

/***** INTERFACE *****/

extern ibitmap m3x3;
static const char* s3x3[9] = {
	"Show world",    "@KA_zmin",    "@Info",
	"Map/Satellite", "@Menu",       "Locations",	
	"@KA_rtte",      "@KA_zout",    "Options"
};

void NOT_IMPLEMENTED(void) {
	Message(ICON_WARNING, "Sorry", "Sorry, this function is not implemented yet", 5000);
}

void OUT_OF_MEMORY(void) {
	Message(ICON_ERROR, "Sorry", T("@No_memory"), 5000);
	CloseApp();
}

void main_repaint();
void load_config();
void edit_config();
void save_config();

void show_info() {
	double lat,lon;
	char buf[200];
	xy2coord(centerx, centery, &lat, &lon);
	snprintf(buf, sizeof(buf), "Current location lat=%.5f lon=%.5f\nZoom=%d Layer=%d", 
		lat, lon, zoom, layer);
	Message(ICON_INFORMATION, PROGRAM_NAME " " VERSION, buf, 10000);
}

void rotate_handler(int o) {
	if(o==orientation) return; // nothing to do
	SetOrientation(o);
	orientation=o;
	main_repaint();
}

void zoom_world() {
	centerx=0x80000000U;
	centery=0x80000000U;
	zoom=MAX_ZOOM-1;
}

void toc_handler(long long pos) {
	const location *loc=get_location(pos);
	coord2xy(loc->lat, loc->lon, &centerx, &centery);
	zoom=loc->zoom;
	fprintf(stderr, "Jump to %s (%g, %g, %d)\n", loc->name, loc->lat, loc->lon, loc->zoom);
	last_location=pos;
	main_repaint();
}

void show_locations() {
	// implemented as TOC
	int i;
	int nloc=get_location_count();
	if(nloc==0) {
		Message(ICON_WARNING, "No locations loaded", "We don't know about locations", 5000);
		return;
	}
	tocentry* toc=calloc(nloc, sizeof(tocentry));
	if(!toc) {
		OUT_OF_MEMORY();
		return;
	}
	for(i=0; i<nloc; ++i) {
		const location *loc=get_location(i);
		toc[i].level=0;
		toc[i].page=loc->zoom;
		toc[i].text=strdup(loc->name);
		toc[i].position=i;
	}
	OpenContents(toc,nloc,last_location,toc_handler);
	for(i=0; i<nloc; ++i) free(toc[i].text);
	free(toc);
}

void m3x3_handler(int choice) {
	fprintf(stderr, "m3x3_handler(%d)\n", choice);
	switch(choice) {
	case 0: /* Show world */
		zoom_world();
		Repaint();
		break;
	case 1: /* Zoom in */
		if(zoom>MIN_ZOOM) --zoom;
		Repaint();
		break;
	case 2: /* Info */
		show_info();
		break;
	case 3: /* Map/Satellite/Terrain */
		layer=(layer+1)%3;
		Repaint();
		break;
	case 4: /* Menu */
		/* Do nothing */
		break;
	case 5: /* Locations */
		show_locations();
		Repaint();
		break;
	case 6: /* Rotate screen */
		OpenRotateBox(&rotate_handler);
		break;
	case 7: /* Zoom out */
		if(zoom<MAX_ZOOM) ++zoom;
		Repaint();
		break;
	case 8: /* Options */
		edit_config();
		main_repaint();
		break;

	}
}

const char* layer_name[] = { "MAP", "SAT", "TER" };

int show_statusbar() {
	// TODO: bitmap with map/satellite/terrain
	double lat, lon;
	char buf[200];
	xy2coord(centerx, centery, &lat, &lon);
	snprintf(buf, sizeof(buf), "lat=%.5f lon=%.5f", lat, lon);
	int percent=100*(zoom-MIN_ZOOM)/(MAX_ZOOM-MIN_ZOOM);
	return DrawPanel(NULL, (char*)layer_name[layer], buf, percent);
}

void main_repaint() {
	int h;
	fprintf(stderr,"EVT_SHOW\n");
	ClearScreen();
	h=show_statusbar();
	show_tiles(h);
	FullUpdate();
	FineUpdate();
}

int main_handler(int type, int par1, int par2) {
	if (type == EVT_INIT) {
		fprintf(stderr,"EVT_INIT\n");
		SetOrientation(orientation);
	}
	if (type == EVT_SHOW) {
		main_repaint();
	}
	if (type == EVT_KEYPRESS) {
		fprintf(stderr,"EVT_KEYPRESS(%d)\n", par1);
		switch(par1) {
		case KEY_LEFT:
			centerx-=CLICK_SHIFT*tile_pixel_size(zoom);
			Repaint();
			break;
		case KEY_RIGHT:
			centerx+=CLICK_SHIFT*tile_pixel_size(zoom);
			Repaint();
			break;
		case KEY_UP:
			centery-=CLICK_SHIFT*tile_pixel_size(zoom);
			Repaint();
			break;
		case KEY_DOWN:
			centery+=CLICK_SHIFT*tile_pixel_size(zoom);
			Repaint();
			break;
		case KEY_PLUS:
			if(zoom>MIN_ZOOM) --zoom;
			Repaint();
			break;
		case KEY_MINUS:
			if(zoom<MAX_ZOOM) ++zoom;
			Repaint();
			break;
		case KEY_OK:
			OpenMenu3x3(&m3x3, s3x3, m3x3_handler);
			break;
		case KEY_BACK:
			CloseApp();
			break;
		}
	}
	return 0;
}

/***** CONFIG *****/

iconfig * cfg=NULL;

static char* choice_startwith[] =
        { "Last position", "World map", NULL };
static char* choice_disk[] = 
	{ "SD card", "Internal memory", NULL };
static char* choice_orientation[] =
	{ "Portrait", "Left landscape", "Upside-down", "Right landscape", NULL };

static iconfigedit confedit[] = {
	{ "Start position", "cfg.pos", CFG_INDEX, "Last position", choice_startwith },
	{ "Maps located in", "cfg.disk", CFG_INDEX, "SD card", choice_disk },
	{ "Path within disk", "cfg.path", CFG_TEXT, "system/googlemaps", NULL },
	{ "Initial orientation", "cfg.orientation", CFG_INDEX, "Portrait", choice_orientation },
	{ NULL, NULL, 0, NULL, NULL}
};

void load_config_2() {
	orientation=ReadInt(cfg, "cfg.orientation", 0);

	snprintf(map_dir, sizeof(map_dir), "/mnt/%s/%s",
		ReadInt(cfg, "cfg.disk", 0) ? "ext1":"ext2",
		ReadString(cfg, "cfg.path", "system/googlemaps"));
	fprintf(stderr, "orientation=%d MAP_DIR=%s\n", orientation, map_dir);
}

void load_config() {
	char *p;
	double lat,lon,z,l;
	int k;
	if(!cfg) cfg=OpenConfig(CONFIG_FILE, confedit);
	fprintf(stderr, "OpenConfig -> %p\n", cfg);
	k=ReadInt(cfg, "cfg.pos", 1);
	if(k==0) { /* Last position */
		p=ReadString(cfg, "last.lat", "0.0");
		lat=atof(p);
		p=ReadString(cfg, "last.lon", "0.0");
		lon=atof(p);
		z=ReadInt(cfg, "last.zoom", MAX_ZOOM-1);
		l=ReadInt(cfg, "last.layer", 0);
        } else { /* World map */
		lat=0.0;
		lon=0.0;
		z=MAX_ZOOM-1;
		l=0;
	}
	coord2xy(lat, lon, &centerx, &centery);
	zoom=z;
	layer=l;
	fprintf(stderr, "x=%lu y-=%lu z=%d l-%d\n", centerx, centery, zoom, layer);
	last_location=0;
	load_config_2();
}

void save_config() {
	double lat,lon;
	char p[30];
	xy2coord(centerx, centery, &lat, &lon);
	snprintf(p,sizeof(p),"%.6f",lat);
	WriteString(cfg,"last.lat",p);
	snprintf(p,sizeof(p),"%.6f",lon);
	WriteString(cfg,"last.lon",p);
	WriteInt(cfg,"last.zoom",zoom);
	WriteInt(cfg,"last.layer",layer);
	SaveConfig(cfg);
}

void config_handler() {
	SaveConfig(cfg);
	load_config_2();
}

void edit_config() {
	if(!cfg) cfg=OpenConfig(CONFIG_FILE, confedit);
	OpenConfigEditor("Configuration	", cfg, confedit, config_handler, NULL);
}

int main(int argc, char **argv) {
	create_tile_cache();
	load_config();
	snprintf(filename, sizeof(filename), "%s/locations", map_dir);
	load_locations(filename);
	InkViewMain(main_handler);
	save_config();
	CloseConfig(cfg);
	destroy_tile_cache();
	return 0;
}

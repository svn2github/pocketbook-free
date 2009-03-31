#include <inkview.h>

#include "main.h"
#include "raster.h"
#include "goog.h"
#include "locations.h"

/***** STATE & CONFIG *****/
uint32_t centerx, centery;
int zoom, layer;
int orientation;
int last_location=0;
char map_dir[256] = MAP_DIR;
int use_last_pos=0;

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

static praster r_empty=NULL;
praster get_empty_raster() {
	if(r_empty) return r_empty;
	r_empty=create_empty_raster(TILE_SIZE, TILE_SIZE, 0x77);
	return r_empty;
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
				show_raster(get_empty_raster(), dx,dy, 0,0, xmax,ymax);
			}
	}
}

/***** INTERFACE *****/

const char* layer_short_name[] = { "MAP", "SAT", "TER", NULL };

extern ibitmap m3x3;
static const char* s3x3[9] = {       
	"Show world",    "Zoom in",     "@Info",
	"Map/Satellite", "@Menu",       "Locations",	
	"@KA_rtte",      "Zoom out",    "Options"
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
void load_position();
void save_position();

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
		// TODO: OpenMenu
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

int show_statusbar() {
	// TODO: bitmap with map/satellite/terrain
	double lat, lon;
	char buf[200];
	xy2coord(centerx, centery, &lat, &lon);
	snprintf(buf, sizeof(buf), "lat=%.5f lon=%.5f", lat, lon);
	int percent=100*(zoom-MIN_ZOOM)/(MAX_ZOOM-MIN_ZOOM);
	return DrawPanel(NULL, (char*)layer_short_name[layer], buf, percent);
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
	{ "Portrait", "Left landscape", "Right landscape", "Upside-down", NULL };

static iconfigedit confedit[] = {
	{ "Start position", "usepos", CFG_INDEX, "0", choice_startwith },
	{ "Maps located in", "disk", CFG_INDEX, "0", choice_disk },
	{ "Path within disk", "path", CFG_TEXT, "system/googlemaps", NULL },
	{ "Initial orientation", "orientation", CFG_INDEX, "0", choice_orientation },
	{ NULL, NULL, 0, NULL, NULL}
};

void load_config() {
	if(!cfg) cfg=OpenConfig(CONFIG_FILE, confedit);
	orientation = ReadInt(cfg, "orientation", 0);
	snprintf(map_dir, sizeof(map_dir), "/mnt/%s/%s",
		ReadInt(cfg, "disk", 0) ? "ext1":"ext2",
		ReadString(cfg, "path", "system/googlemaps"));
	use_last_pos = 0==ReadInt(cfg,"usepos",0);

	fprintf(stderr, "orientation=%d MAP_DIR=%s usepos=%d\n", 
		orientation, map_dir, use_last_pos);
}

void save_config() {
	SaveConfig(cfg);
}


void config_handler() {
	SaveConfig(cfg);
	SetEventHandler(main_handler);
}

void edit_config() {
	if(!cfg) cfg=OpenConfig(CONFIG_FILE, confedit);
	OpenConfigEditor("Configuration", cfg, confedit, config_handler, NULL);
}

void load_position() {
	double lat,lon;
	int z,l;
	FILE* f=fopen(LAST_POSITION_FILE,"r");
	if(!f) goto fail;
	if(fscanf(f, "%lf %lf %d %d", &lat, &lon, &z, &l)!=4) goto fail;
	coord2xy(lat, lon, &centerx, &centery);
	zoom=z;
	layer=l;
	fprintf(stderr, "x=%lu y-=%lu z=%d l=%d\n", centerx, centery, zoom, layer);
	fclose(f);
	return;
fail:
	if(f) fclose(f);
	zoom_world();
}

void save_position() {
	double lat,lon;
	FILE* f=fopen(LAST_POSITION_FILE,"w");
	if(!f) return;
	xy2coord(centerx, centery, &lat, &lon);
	fprintf(f,"%.5lf %.5lf %d %d\n", lat, lon, zoom, layer);
	fclose(f);
}

int main(int argc, char **argv) {
	create_tile_cache();
	load_config();
	if(use_last_pos) load_position();
	else zoom_world();
	snprintf(filename, sizeof(filename), "%s/locations", map_dir);
	load_locations(filename);
	InkViewMain(main_handler);
	save_position();
	save_config();
	CloseConfig(cfg);
	destroy_tile_cache();
	return 0;
}

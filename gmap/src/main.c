#include <inkview.h>
#include <math.h>

#include "main.h"
#include "raster.h"
#include "goog.h"
#include "locations.h"

/***** STATE & CONFIG *****/
uint32_t centerx, centery;
int zoom, layer;
int orientation;
int last_location=0;
char map_dir[256] = SDCARDDIR "/" MAP_DEFAULT_DIR;
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
				show_raster(r, dx,dy, 0,0, xmax,ymax);
			} else {
				fprintf(stderr,"not found\n");
				show_raster(get_empty_raster(), dx,dy, 0,0, xmax,ymax);
			}
	}
}

/***** INTERFACE *****/

const int layer_count=3;
const char* layer_name[] = { "Map", "Satellite", "Terrain", NULL };
const char* layer_short_name[] = { "MAP", "SAT", "TER", NULL };

extern ibitmap m3x3;
static const char* s3x3[9] = {       
	"Show world",    "Zoom in",     "@Info",
	"Map/Satellite", "@Menu",       "Locations",	
	"@KA_rtte",      "Zoom out",    "Options"
};

ifont *small_font = NULL;
ifont *large_font = NULL;

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

int shift_pixel(int direction, int hold) {
    int shift=0;
    int divider=4;
    switch(hold) {
        case 1:
            divider=2;
            break;
        case 2:
            divider=1;
            break;
    }
    switch(direction) {
        case KEY_LEFT:
        case KEY_RIGHT:
            shift=ScreenWidth()/divider;
            break;
        case KEY_DOWN:
            shift-=STATUSBAR_HEIGHT;
        case KEY_UP:
            shift=ScreenHeight()/divider;
            break;
    }
    return shift;
}

void zoom_world() {
	centerx=0x80000000U;
	centery=0x80000000U;
	zoom=MAX_ZOOM-1;
}

tocentry* toc=NULL;
int toc_count=0;

void delete_toc() {
	int i;
	if(toc==NULL) return;
	for(i=0; i<toc_count; ++i)
		if(toc[i].text) free(toc[i].text);
	free(toc);
	toc=NULL; toc_count=0;
}

void toc_handler(long long pos) {
	const location *loc=get_location(pos);
	coord2xy(loc->lat, loc->lon, &centerx, &centery);
	zoom=loc->zoom;
	fprintf(stderr, "Jump to %s (%g, %g, %d)\n", loc->name, loc->lat, loc->lon, loc->zoom);
	last_location=pos;
	delete_toc();
	main_repaint();
}

void show_locations() {
	// implemented as TOC
	int i;
	toc_count=get_location_count();
	if(toc_count==0) {
		Message(ICON_WARNING, "No locations loaded", "We don't know about locations", 5000);
		return;
	}
	toc=calloc(toc_count, sizeof(tocentry));
	if(!toc) {
		OUT_OF_MEMORY();
		return;
	}
	for(i=0; i<toc_count; ++i) {
		const location *loc=get_location(i);
		toc[i].level=0;
		toc[i].page=loc->zoom;
		toc[i].text=strdup(loc->name);
		if(!toc[i].text) OUT_OF_MEMORY();
		toc[i].position=i;
	}
	OpenContents(toc,toc_count,last_location,toc_handler);
}

imenu *layer_menu=NULL;

void select_layer_handler(int l) {
	if(layer!=l) {
		layer=l;
		main_repaint();
	}
	free(layer_menu);
	layer_menu=NULL;
}

void select_layer() {
	int i;
	layer_menu=calloc(layer_count+2,sizeof(imenu));
	layer_menu[0].type=ITEM_HEADER;
	layer_menu[0].text="Layers";
	layer_menu[0].index=-42;
	for(i=0; i<layer_count; ++i) {
		layer_menu[i+1].type=ITEM_ACTIVE;
		layer_menu[i+1].text=(char*)layer_name[i];
		layer_menu[i+1].index=i;
	}
	OpenMenu(layer_menu, layer, 20, 20, select_layer_handler);
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
		select_layer();
		break;
	case 4: /* Menu */
		/* Do nothing */
		break;
	case 5: /* Locations */
		show_locations();
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
		break;

	}
}

int show_statusbar() {
	// TODO: bitmap with map/satellite/terrain
	double lat, lon;
	char buf[200];
	int w=ScreenWidth(), yb=ScreenHeight(), yt=yb-STATUSBAR_HEIGHT;
	DrawLine(0, yt, w, yt, DGRAY);
	DrawLine(0, yt+1, w, yt+1, LGRAY);
	FillArea(0, yt+2, w, yb-yt-2, DGRAY);
	SetFont(large_font,WHITE);
	DrawString(3, yt+2, layer_short_name[layer]);
	xy2coord(centerx, centery, &lat, &lon);
	snprintf(buf, sizeof(buf), "lat=%.5f lon=%.5f zoom=%d", lat, lon, zoom);
	SetFont(small_font,WHITE);
	DrawString(60, yt+5, buf);
	{
		// compute ruler step
		double kmpp, rulerstep, r;
		int i,n;
		kmpp=km_per_pixel(centerx, centery, zoom);
		rulerstep=kmpp*100;
		r=pow(10,floor(log10(rulerstep)));
		if(rulerstep/r>5)
			rulerstep=5*r;
		else if(rulerstep/r>2)
			rulerstep=2*r;
		else
			rulerstep=r;
		// draw ruler
		n=floor(270/(rulerstep/kmpp));
		fprintf(stderr, "kmpp=%g rulerstep=%g n=%d\n", kmpp, rulerstep, n);

		for(i=0; i<n; ++i) {
			int xl=w-300+(int)(round(i*rulerstep/kmpp));
			int xr=w-300+(int)(round((i+1)*rulerstep/kmpp));
			int w;
			fprintf(stderr, "%d xl=%d xr=%d\n", i, xl, xr);
			FillArea(xl, yb-5, xr-xl, 4, i%2?WHITE:BLACK);
			snprintf(buf, sizeof(buf), "%g",
				(i+1)*rulerstep*(rulerstep<1 ? 1000:1));
			w=StringWidth(buf);
			DrawString(xr-w,yt+3,buf);
			if(i==n-1) DrawString(xr+1,yt+3,(rulerstep<1 ? "m":"km"));
		}
	}
	return yb-yt;
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
    static int latest_key=0;
    static int repeat=0;
	if (type == EVT_INIT) {
		fprintf(stderr,"EVT_INIT\n");
		SetOrientation(orientation);
		small_font=OpenFont("sans-serif", 12, 1);
		large_font=OpenFont("sans-serif", 16, 1);
	}
	if (type == EVT_SHOW) {
		main_repaint();
	}
    if(type == EVT_KEYPRESS) {
        repeat=0;
        latest_key=par1;
    }
    if(type == EVT_KEYREPEAT) {
        if(repeat<2) repeat++;
    }
	if ((type == EVT_KEYRELEASE && repeat==0) || type==EVT_KEYREPEAT) {
        int shift = shift_pixel(latest_key, repeat);
		//fprintf(stderr,"EVT_KEYPRESS(%d,%d)\n", par1,par2);
		switch(latest_key) {
		case KEY_LEFT:
			centerx-=shift*tile_pixel_size(zoom);
			Repaint();
			break;
		case KEY_RIGHT:
			centerx+=shift*tile_pixel_size(zoom);
			Repaint();
			break;
		case KEY_UP:
			centery-=shift*tile_pixel_size(zoom);
			Repaint();
			break;
		case KEY_DOWN:
			centery+=shift*tile_pixel_size(zoom);
			Repaint();
			break;
		case KEY_NEXT:
		case KEY_PLUS:
			if(zoom>MIN_ZOOM) zoom-=(repeat>0?2:1);
			Repaint();
			break;
		case KEY_PREV:
            if(repeat>0) {
                //Exit on PB360
                CloseApp();
                break;
            }
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
    if(type == EVT_KEYRELEASE) {
        repeat=0;
        latest_key=0;
    }
	return 0;
}

/***** CONFIG *****/

iconfig * cfg=NULL;

static char* choice_startwith[] =
        { "Last position", "World map", NULL };
static char* choice_disk[] = 
	{ "SD card", "Internal flash", NULL };
static char* choice_orientation[] =
	{ "Portrait", "Left landscape", "Right landscape", "Upside-down", NULL };

static iconfigedit confedit[] = {
	{ CFG_INDEX, NULL, "Start position", "hint.usepos", "usepos", "0", choice_startwith, NULL },
	{ CFG_INDEX, NULL, "Maps located on", "hint.disk", "disk", "0", choice_disk, NULL },
	{ CFG_TEXT, NULL, "Path within disk", "hint.path", "path", MAP_DEFAULT_DIR, NULL, NULL },
	{ CFG_INDEX, NULL, "Initial orientation", "hint.orientation", "orientation", "0", choice_orientation, NULL },
    { 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};

void load_config() {
	if(!cfg) cfg=OpenConfig(CONFIG_FILE, confedit);
	orientation = ReadInt(cfg, "orientation", 0);
	snprintf(map_dir, sizeof(map_dir), "%s/%s",
		ReadInt(cfg, "disk", 0) ? FLASHDIR:SDCARDDIR,
		ReadString(cfg, "path", MAP_DEFAULT_DIR));
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

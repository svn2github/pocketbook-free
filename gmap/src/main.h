#ifndef _main_h_
#define _main_h_

/* Who am I */
#define PROGRAM_NAME "Google map viewer"
#define VERSION "0.01"

/* how many tiles to store in memory */
#define TILE_CACHE_SIZE 2049

/* how many pixels to shift on pressing left/right/up/down */
#define CLICK_SHIFT 64
/* how many pixels to shift on holding left/right/up/down */
#define HOLD_SHIFT 256

/* Where are the maps */
#define MAP_DIR SDCARDDIR "/googlemaps"

#define CONFIG_FILE USERDATA "/gmap.conf"

/* Test location */
//   location="Paris, France"	lat="48.856667"	lng="2.350987"	zoom="5" 
#define TEST_LAT 48.856667
#define TEST_LON 2.350987
#define TEST_ZOOM 5
#define TEST_LAYER 0 

#endif

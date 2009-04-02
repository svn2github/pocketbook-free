#ifndef _main_h_
#define _main_h_

/* Who am I */
#define PROGRAM_NAME "Google map viewer"
#define VERSION "0.02-090330"

/* how many tiles to store in memory */
#define TILE_CACHE_SIZE 253

/* how many pixels to shift on pressing left/right/up/down */
#define CLICK_SHIFT 64
/* how many pixels to shift on holding left/right/up/down */
#define HOLD_SHIFT 256

/* Where are the maps */
#define MAP_DIR SDCARDDIR "/googlemaps"

#define CONFIG_FILE USERDATA "/gmap.conf"
#define LAST_POSITION_FILE USERDATA "/gmap.last"

#endif

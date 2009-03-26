#ifndef _locations_h_
#define	_locations_h_

#define LOCNAME_SIZE 64
typedef struct location_s {
    char name[LOCNAME_SIZE+1];
    double lat,lon;
    int zoom;
} location;

void load_locations(const char* filename);
void add_location(const location* loc);
int get_location_count();
const location* get_location(int index);
const location* get_location_by_name(const char* name);

#endif	/* _LOCATIONS_H */


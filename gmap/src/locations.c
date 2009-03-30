#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "locations.h"
#include "main.h"

location* locs=NULL;
int loc_size=0, loc_count=0;

/*
# This is the locations file used by GMapCatcher.
#
# This file contains a list of Locations/Position.
# Each entry should be kept on an individual line.
# The latitude, longitud and zoom should be TAB separated.
#
# Additionally, comments (such as these) may be inserted on
# lines sarting with a '#' symbol.
#
# For example:
#
#   location="Paris, France"	lat="48.856667"	lng="2.350987"	zoom="5"
#
location="Paris, France"	lat="48.856667"	lng="2.350987"	zoom="5"
location="Mountain View, CA"	lat="37.400465"	lng="-122.073003"	zoom="5"
*/

void load_locations(const char* filename) {
	char buf[500];
	char *p, *q;
	location cur;
	int known, what;
	FILE* f=fopen(filename,"r");
	if(!f) return;
	while(fgets(buf, sizeof(buf), f)) {
		memset(&cur, 0, sizeof(cur));
		known=0;
		p=buf;
		while(1) {
			what=-1;
			while(isspace(*p) && *p!=0) ++p;
			if(*p==0 || *p=='#') break; // end of line or comment
			if(!strncmp(p, "location=", 9)) {
				what=0;
				p+=9;
			}
			if(!strncmp(p, "lat=", 4)) {
				what=1;
				p+=4;
			}
			if(!strncmp(p, "lng=", 4)) {
				what=2;
				p+=4;
			}
			if(!strncmp(p, "zoom=", 5)) {
				what=3;
				p+=5;
			}
			if(what<0) {
			}
			while(isspace(*p) && *p!=0) ++p;
			if(*p!='"') break; // format error
			q=++p;
			while(*p!=0 && *p!='"') ++p;
			if(*p!='"') break; // format error
			*p=0; ++p;
			switch(what) {
			case 0:
				strncpy(cur.name,q,sizeof(cur.name)-1);
				break;
			case 1:
				cur.lat=atof(q);
				break;
			case 2:
				cur.lon=atof(q);
				break;
			case 3:
				cur.zoom=atoi(q);
				break;
			default:
				;
			}
			if(what>=0) known |= (1<<what);
		}
		if(known==0xF) {
			fprintf(stderr, "Location %s (lat=%g,lon=%g,zoom=%d)\n",
				cur.name, cur.lat, cur.lon, cur.zoom);
			add_location(&cur);
		}
	}
}

void add_location(const location* loc) {
	if(loc_size==loc_count) {
		// resize
		loc_size+=10+loc_count;
		locs=realloc(locs, sizeof(location)*loc_size); // allocate more space
	}
	memcpy(&locs[loc_count++],loc,sizeof(location));
}

int get_location_count() { return loc_count; }

const location* get_location(int index) {
	if(index>=0 && index<loc_count) return &locs[index];
	return NULL;
}

const location* get_location_by_name(const char* lname) {
	int i;
	for(i=0; i<loc_count; ++i)
		if(!strcmp(locs[i].name, lname)) return &locs[i];
	return NULL;
}

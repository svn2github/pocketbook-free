#ifndef _draw_h_
#define	_draw_h_

#include "inkview.h"
#include <algorithm>
#include <math.h>

#include "define.h"
#include "mymath.h"

//point
typedef struct point_s {
	int x;
	int y;
} point;

point operator- (point p1, point p2) {
	point a = {p1.x - p2.x, p1.y - p2.y};
	return a;
}

point operator+ (point p1, point p2) {
	point a = {p1.x + p2.x, p1.y + p2.y};
	return a;
}

point operator* (float u, point p1) {
	point a = {p1.x * (int)u, p1.y * (int)u};
	return a;
}

bool operator== (point p1, point p2) {
	if (p1.x == p2.x && p1.y == p2.y) {
		return true;
	}
	return false;
}

using namespace std;

//---------------------------------------------------------------------------
// Функция рисует окружность
void DrawCircle(int cx, int cy, int d, int color)
{
	int df = 150 / d;
	if(!df) df = 1;
	for(int i = 0; i < 1000; i += df)
	{
		DrawLine(cx + d * cosi(i) / 2000,
			 cy + d * sini(i) / 2000,
			 cx + d * cosi(i + df) / 2000,
			 cy + d * sini(i + df) / 2000,
			 color);
	}
}


void DrawColoredPixel(int x, int y, int color, int type) {
	int colors[2];
	if (color == BLACK) {
		colors[0] = BLACK;
		colors[1] = LGRAY;
	} else {
		colors[0] = DGRAY;
		colors[1] = WHITE;
	}
	type = type % 4;
	if (type == 0) {
		DrawPixel(x, y, colors[(x+y)%2]);
	}
	if (type == 1) {
		if ((x+y)%4 == 1) {
			DrawPixel(x, y, colors[0]);
		} else {
			DrawPixel(x, y, colors[1]);
		}
	}
	
	if (type == 2) {
		if (x%4 == y%4) {
			DrawPixel(x, y, colors[0]);
		} else {
			DrawPixel(x, y, colors[1]);
		}
	}
	
	if (type == 3) {
		if (y%3 == 0 || x%3 == 0) {
			DrawPixel(x, y, colors[0]);
		} else {
			DrawPixel(x, y, colors[1]);
		}
	}
}

void DrawStation(int x, int y, int d, int color, int type, bool solid) {
		int cx = x - d/2 - 1;
		int cy = y - d /2 - 1;
		int fx = x + d/2 + 1;
		int fy = y + d /2 + 1;
		for (cx = x - d/2 - 1; cx < fx; cx++) {
			for (cy = y - d /2 - 1; cy < fy; cy++) {
				if ((cx - x)*(cx - x) + (cy - y)*(cy - y) < d*d/4) {
					if (solid) {
						DrawColoredPixel(cx, cy, color, type);
					} else {
						DrawPixel(cx, cy, WHITE);
					}
				}
			}
		}
	
	DrawCircle(x, y, d, color);
}

	
//2 отрезка пересекаются в точке
bool intersection(point p1, point p2, point p3, point p4, point *out_intersection) {
	// Store the values for fast access and easy
	// equations-to-code conversion
	if (p1 == p4 || (p1 == p3)) {
		*out_intersection = p1;
		return true;
	}
	if (p2 == p3 || p2 == p4) {
		*out_intersection = p2;
		return true;
	}
	int A1 = p2.y - p1.y;
	int B1 = p1.x - p2.x;
	int C1 = (A1*p1.x) + (B1*p1.y);

	int A2 = p4.y - p3.y;
	int B2 = p3.x - p4.x;
	int C2 = A2*p3.x + B2*p3.y;

	int det = A1*B2 - A2*B1;
	if (det==0){
		return false;
	}else{
		// Return the point of intersection
		out_intersection->x = (B2*C1 - B1*C2)/det;
		out_intersection->y = (A1*C2 - A2*C1)/det;
		return true;
	}
}

void find_points_edge(point p1, point p2, int width, point *pp) {
	int dy1 = p1.y - p2.y;
	int dx1 = p1.x - p2.x;
	int pw =  (int)round((float)width / 2);
	//вертикальная линия
	if (dx1 == 0) {
		if (p1.y < p2.y) {
			pp[0].x = pp[1].x = p1.x + pw;
			pp[2].x = pp[3].x = p1.x - pw;
		} else {
			pp[0].x = pp[1].x = p1.x - pw;
			pp[2].x = pp[3].x = p1.x + pw;
		}
		pp[0].y = pp[2].y = p1.y;
		pp[1].y = pp[3].y = p2.y;
	//горизонтальная линия
	} else if (dy1 == 0) {
		if (p1.x < p2.x) {
			pp[0].y = pp[1].y = p1.y - pw;
			pp[2].y = pp[3].y = p1.y + pw;
		} else {
			pp[0].y = pp[1].y = p1.y + pw;
			pp[2].y = pp[3].y = p1.y - pw;
		}
		pp[0].x = pp[2].x = p1.x;
		pp[1].x = pp[3].x = p2.x;
	// наклонная
	} else {
		float tg = (float)dx1 / dy1;
		float dox = sqrt((float)width * width / (4 * (1 + tg*tg)));
		if (p2.y < p1.y) { dox = -dox;}
		int doy = (int) (dox * tg);
		pp[0].x = p1.x + (int)dox;
		pp[0].y = p1.y - doy;
		pp[1].x = p2.x + (int)dox;
		pp[1].y = p2.y - doy;
		
		pp[2].x = p1.x - (int)dox;
		pp[2].y = p1.y + doy;
		pp[3].x = p2.x - (int)dox;
		pp[3].y = p2.y + doy;
	}
}

void DrawLine(point p1, point p2, int color) {
	DrawLine(p1.x, p1.y, p2.x, p2.y, color);
}

void DrawChange(point p1, point p2, int width, int color) {
	int cx, cy;
	int xmin = 0, xmax = 0, ymin = 0, ymax = 0;
	
	point pp[4];
	find_points_edge(p1, p2, width, pp);

	xmin = min(pp[0].x, min(pp[1].x, min(pp[2].x, pp[3].x)));
	xmax = max(pp[0].x, max(pp[1].x, max(pp[2].x, pp[3].x)));
	ymin = min(pp[0].y, min(pp[1].y, min(pp[2].y, pp[3].y)));
	ymax = max(pp[0].y, max(pp[1].y, max(pp[2].y, pp[3].y)));
	
	int mx[4] = {pp[0].x, pp[1].x, pp[3].x, pp[2].x};
	int my[4] = {pp[0].y, pp[1].y, pp[3].y, pp[2].y};
	
	for (cx = xmin; cx < xmax; cx ++) {
		for (cy = ymin; cy < ymax; cy ++) {
			if (pnpoly(4,mx, my, cx, cy)) {
				DrawPixel(cx, cy, WHITE);
			}
		}
	}

	for (int j = 0; j < 4 - 1; j++) {
		DrawLine(mx[j], my[j], mx[j+1], my[j+1], color);
	}
}

void DrawEdge(point p1, point p2, int width, int color, int type, point * p3, int adcount, bool solid) {
	int cx, cy;
	int xmin = 0, xmax = 0, ymin = 0, ymax = 0;
	
	//Случай если просто линия
	if (adcount == 0) {
		point pp[4];
		find_points_edge(p1, p2, width, pp);

		xmin = min(pp[0].x, min(pp[1].x, min(pp[2].x, pp[3].x)));
		xmax = max(pp[0].x, max(pp[1].x, max(pp[2].x, pp[3].x)));
		ymin = min(pp[0].y, min(pp[1].y, min(pp[2].y, pp[3].y)));
		ymax = max(pp[0].y, max(pp[1].y, max(pp[2].y, pp[3].y)));
		
		int mx[4] = {pp[0].x, pp[1].x, pp[3].x, pp[2].x};
		int my[4] = {pp[0].y, pp[1].y, pp[3].y, pp[2].y};
		if (solid) {
			for (cx = xmin; cx < xmax; cx ++) {
				for (cy = ymin; cy < ymax; cy ++) {
					if (pnpoly(4,mx, my, cx, cy)) {
						DrawColoredPixel(cx, cy, color, type);
					}
				}
			}
		}
		if (!solid) {
			for (int j = 0; j < 4+2*adcount - 1; j++) {
				DrawLine(mx[j], my[j], mx[j+1], my[j+1], color);
			}
		}
	//если есть дополнительные точки
	} else {
		int i = 0;
		point prevp = p1, curp = p3[0];
		point pr[4], cr[4];
		find_points_edge(prevp, curp, width, pr);

		prevp = curp;
		point tt1, tt2;
		
		int mx[4 + 2*ADDITIONAL_POINTS];
		int my[4 + 2*ADDITIONAL_POINTS];
		for (i = 0; i < adcount; i++) {
			if (i + 1 == adcount) {
				curp = p2;
			} else {
				curp = p3[i+1];
			}
			
			find_points_edge(prevp, curp, width, cr);
			if (intersection(pr[0], pr[1], cr[0], cr[1], &tt1) && intersection(cr[2], cr[3], pr[2], pr[3], &tt2)) {
				if (i == 0) {
					mx[0] = pr[0].x;
					mx[3 + 2*adcount] = pr[2].x;
					my[0] = pr[0].y;
					my[3 + 2*adcount] = pr[2].y;
					xmin = min(pr[0].x, pr[2].x);
					xmax = max(pr[0].x, pr[2].x);
					ymin = min(pr[0].y, pr[2].y);
					ymax = max(pr[0].y, pr[2].y);
					
				}
				
				mx[i+1] = tt1.x;
				mx[2 + 2*adcount - i] = tt2.x;
				my[i+1] = tt1.y;
				my[2 + 2*adcount - i] = tt2.y;
				
				xmin = min(xmin, min(tt1.x, tt2.x));
				xmax = max(xmax, max(tt1.x, tt2.x));
				ymin = min(ymin, min(tt1.y, tt2.y));
				ymax = max(ymax, max(tt1.y, tt2.y));
				
				if (i + 1 == adcount) {
					mx[i+2] = cr[1].x;
					mx[1 + 2*adcount - i] = cr[3].x;
					my[i+2] = cr[1].y;
					my[1 + 2*adcount - i] = cr[3].y;
					xmin = min(xmin, min(cr[1].x, cr[3].x));
					xmax = max(xmax, max(cr[1].x, cr[3].x));
					ymin = min(ymin, min(cr[1].y, cr[3].y));
					ymax = max(ymax, max(cr[1].y, cr[3].y));
				}
			}
			memcpy(pr, cr, sizeof(cr));
			prevp = curp;
		}
		
		
		if (solid) {
			for (cx = xmin; cx < xmax; cx ++) {
				for (cy = ymin; cy < ymax; cy ++) {
					if (pnpoly(4+2*adcount, mx, my, cx, cy)) {
						DrawColoredPixel(cx, cy, color, type);
					}
				}
			}
		}
		if (!solid) {
			for (int j = 0; j < 4+2*adcount - 1; j++) {
				DrawLine(mx[j], my[j], mx[j+1], my[j+1], color);
			}
		}
	}
}


#endif	/* draw_h */

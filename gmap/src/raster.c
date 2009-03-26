#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <inkview.h>

#include "raster.h"

void delete_raster(praster r) {
	if(r) {
		free(r->data);
		free(r);
	}
}

praster load_png_grayscale(const char* fname) {
	int i;
	FILE* f=NULL;
	char header[8];
	png_structp png_ptr=NULL;
	png_infop info_ptr=NULL;
	png_bytepp rows=NULL;
	char* bytes=NULL;

	struct raster* r=NULL;
	unsigned long width, height;
	int bit_depth, color_type, row_bytes;
	
	f=fopen(fname,"rb");
	if(!f) return NULL;

	if(fread(header,1,8,f)!=8) goto out;
	/* check if this is a png file */
	if(!png_check_sig(header,8)) goto out;

	png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr) goto out;
	info_ptr=png_create_info_struct(png_ptr);
	if(!info_ptr) goto out;

	/* catch errors */
	if(setjmp(png_jmpbuf(png_ptr))) {
		if(r) {
			delete_raster(r);
			r=NULL;
		}
		goto out;
	}

	/* read file */
	png_init_io(png_ptr, f);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);
   	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
	if(width>10000 || height>10000) goto out;

       	/* Set up some transforms. */
   	if (color_type & PNG_COLOR_MASK_ALPHA) {
      		png_set_strip_alpha(png_ptr);
   	}
   	if (bit_depth > 8) {
      		png_set_strip_16(png_ptr);
   	}
        if (color_type == PNG_COLOR_TYPE_PALETTE) {
      		png_set_palette_to_rgb(png_ptr);
   	}
    	if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA || color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_rgb_to_gray_fixed(png_ptr, 1, -1, -1);
	}

   	/* Update the png info struct.*/
   	png_read_update_info(png_ptr, info_ptr);

	/* allocate raster and rows */
	row_bytes=png_get_rowbytes(png_ptr,info_ptr);
	rows=malloc(height*sizeof(png_bytep));
	if(!rows) goto out;
	bytes=malloc(height*row_bytes);
	if(!bytes) goto out;
	for(i=0; i<height; ++i) rows[i]=bytes + i*width;
	png_read_image(png_ptr, rows);

	/* save result */
	r=malloc(sizeof(struct raster));
	if(!r) goto out;
	r->w=width;
	r->h=height;
	r->data=malloc(width*height);
	if(!r->data) {
		free(r);
		r=NULL;
		goto out;
	}
	memcpy(r->data,bytes,width*height);
		
out:
	if(bytes) free(bytes);
	if(rows) free(rows);
	if(png_ptr) png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
	if(f) fclose(f);
	return r;
}

struct my_error_mgr {
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr * my_error_ptr;

void my_error_exit (j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr) cinfo->err;
  (*cinfo->err->output_message) (cinfo);
  longjmp(myerr->setjmp_buffer, 1);
}

#define JPEG_STRIP_SIZE 16
praster load_jpg_grayscale(const char* fname) {
	FILE *f = NULL;
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;

	JSAMPARRAY bytes = NULL;
	struct raster *r = NULL;
	int row_stride;
	int width, height;
	int i, i0, nread;

	f=fopen(fname,"rb");
	if(!f) return NULL;

	cinfo.err=jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit=my_error_exit;
	if(setjmp(jerr.setjmp_buffer)) {
		if(r) {
			delete_raster(r);
			r=NULL;
		}
		goto out;
	}
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, f);

	jpeg_read_header(&cinfo, TRUE);
	cinfo.out_color_space=JCS_GRAYSCALE;
	width=cinfo.image_width;
	height=cinfo.image_height;

	r=malloc(sizeof(struct raster));
	if(!r) goto out;
	r->w=width;
	r->h=height;
	r->data=malloc(width*height);
	if(!r->data) {
		free(r);
		r=NULL;
		goto out;
	}

	jpeg_start_decompress(&cinfo);
	
	row_stride = cinfo.output_width * cinfo.output_components;
        bytes = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, JPEG_STRIP_SIZE);
	while (cinfo.output_scanline < cinfo.output_height) {
		i0=cinfo.output_scanline;
		nread=jpeg_read_scanlines(&cinfo, bytes, JPEG_STRIP_SIZE);
		for(i=0; i<nread; ++i)
			memcpy(r->data+width*(i0+i), bytes[i], width);
	}
	jpeg_finish_decompress(&cinfo);
out:
	jpeg_destroy_decompress(&cinfo);
	return r;
}

void show_raster(praster r, int x0, int y0,
        int xmin, int ymin, int xmax, int ymax) {
	char* rpos=r->data;
	int rw=r->w, rh=r->h, rs=r->w;
	if(x0<xmin) {
		rw-=xmin-x0;
		if(rw<=0) return;
		rpos+=xmin-x0;
		x0=xmin;
	}
	if(y0<0) {
		rh-=ymin-y0;
		if(rh<=0) return;
		rpos+=(ymin-y0)*rs;
		y0=ymin;
	}
	if(x0+rw>xmax) {
		rw=xmax-x0;
		if(rw<=0) return;
	}
	if(y0+rh>ymax) {
		rh=ymax-y0;
		if(rh<=0) return;
	}
	Stretch(rpos, IMAGE_GRAY8, rw, rh, rs, x0, y0, rw, rh, 0);
	DitherArea(x0, y0, rw, rh, 16, DITHER_PATTERN);
}

//========================================================================
//
// SplashBitmap.cc
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2007 Ilmari Heikkinen <ilmari.heikkinen@gmail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdio.h>
#include "goo/gmem.h"
#include "SplashErrorCodes.h"
#include "SplashBitmap.h"

//------------------------------------------------------------------------
// SplashBitmap
//------------------------------------------------------------------------

SplashBitmap::SplashBitmap(int widthA, int heightA, int rowPad,
			   SplashColorMode modeA, GBool alphaA,
			   GBool topDown) {
  width = widthA;
  height = heightA;
  mode = modeA;
  switch (mode) {
  case splashModeMono1:
    rowSize = (width + 7) >> 3;
    break;
  case splashModeMono4:
    rowSize = (width + 1) >> 1;
    break;
  case splashModeMono8:
    rowSize = width;
    break;
  case splashModeRGB8:
  case splashModeBGR8:
    rowSize = width * 3;
    break;
  case splashModeXBGR8:
    rowSize = width * 4;
    break;
#if SPLASH_CMYK
  case splashModeCMYK8:
    rowSize = width * 4;
    break;
#endif
  }
  rowSize += rowPad - 1;
  rowSize -= rowSize % rowPad;
  data = (SplashColorPtr)gmalloc(rowSize * height);
  if (!topDown) {
    data += (height - 1) * rowSize;
    rowSize = -rowSize;
  }
  if (alphaA) {
    alpha = (Guchar *)gmalloc(width * height);
  } else {
    alpha = NULL;
  }
}


SplashBitmap::~SplashBitmap() {
  if (rowSize < 0) {
    gfree(data + (height - 1) * rowSize);
  } else {
    gfree(data);
  }
  gfree(alpha);
}


SplashError SplashBitmap::writePNMFile(char *fileName) {
  FILE *f;
  SplashError e;

  if (!(f = fopen(fileName, "wb"))) {
    return splashErrOpenFile;
  }

  e = this->writePNMFile(f);
  
  fclose(f);
  return e;
}


SplashError SplashBitmap::writePNMFile(FILE *f) {
  SplashColorPtr row, p;
  Guchar c;
  int x, y;

  switch (mode) {

  case splashModeMono1:
    fprintf(f, "P4\n%d %d\n", width, height);
    row = data;
    for (y = 0; y < height; ++y) {
      p = row;
      for (x = 0; x < width; x += 8) {
	fputc(*p ^ 0xff, f);
	++p;
      }
      row += rowSize;
    }
    break;

  case splashModeMono4:
    fprintf(f, "P5\n%d %d\n", width, height);
    row = data;
    for (y = 0; y < height; ++y) {
      p = row;
      for (x = 0; x < width; ++x) {
	c = ((x & 1) ? *p : (*p >> 4)) & 0xf;
	fputc(c | (c<<4), f);
	if (x & 1) ++p;
      }
      row += rowSize;
    }
    break;

  case splashModeMono8:
    fprintf(f, "P5\n%d %d\n255\n", width, height);
    row = data;
    for (y = 0; y < height; ++y) {
      p = row;
      for (x = 0; x < width; ++x) {
	fputc(*p, f);
	++p;
      }
      row += rowSize;
    }
    break;

  case splashModeRGB8:
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    row = data;
    for (y = 0; y < height; ++y) {
      p = row;
      for (x = 0; x < width; ++x) {
	fputc(splashRGB8R(p), f);
	fputc(splashRGB8G(p), f);
	fputc(splashRGB8B(p), f);
	p += 3;
      }
      row += rowSize;
    }
    break;

  case splashModeXBGR8:
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    row = data;
    for (y = 0; y < height; ++y) {
      p = row;
      for (x = 0; x < width; ++x) {
	fputc(splashBGR8R(p), f);
	fputc(splashBGR8G(p), f);
	fputc(splashBGR8B(p), f);
	p += 4;
      }
      row += rowSize;
    }
    break;


  case splashModeBGR8:
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    row = data;
    for (y = 0; y < height; ++y) {
      p = row;
      for (x = 0; x < width; ++x) {
	fputc(splashBGR8R(p), f);
	fputc(splashBGR8G(p), f);
	fputc(splashBGR8B(p), f);
	p += 3;
      }
      row += rowSize;
    }
    break;

#if SPLASH_CMYK
  case splashModeCMYK8:
    // PNM doesn't support CMYK
    break;
#endif
  }
  return splashOk;
}


void SplashBitmap::getPixel(int x, int y, SplashColorPtr pixel) {
  SplashColorPtr p;
  Guchar c;

  if (y < 0 || y >= height || x < 0 || x >= width) {
    return;
  }
  switch (mode) {
  case splashModeMono1:
    p = &data[y * rowSize + (x >> 3)];
    pixel[0] = (p[0] & (0x80 >> (x & 7))) ? 0xff : 0x00;
    break;
  case splashModeMono4:
    p = &data[y * rowSize + (x >> 1)];
    c = ((x & 1) ? *p : (*p >> 4)) & 0xf;
    pixel[0] = c | (c<<4);
    break;
  case splashModeMono8:
    p = &data[y * rowSize + x];
    pixel[0] = p[0];
    break;
  case splashModeRGB8:
    p = &data[y * rowSize + 3 * x];
    pixel[0] = p[0];
    pixel[1] = p[1];
    pixel[2] = p[2];
    break;
  case splashModeXBGR8:
    p = &data[y * rowSize + 4 * x];
    pixel[0] = p[2];
    pixel[1] = p[1];
    pixel[2] = p[0];
    pixel[3] = p[3];
    break;
  case splashModeBGR8:
    p = &data[y * rowSize + 3 * x];
    pixel[0] = p[2];
    pixel[1] = p[1];
    pixel[2] = p[0];
    break;
#if SPLASH_CMYK
  case splashModeCMYK8:
    p = &data[y * rowSize + 4 * x];
    pixel[0] = p[0];
    pixel[1] = p[1];
    pixel[2] = p[2];
    pixel[3] = p[3];
    break;
#endif
  }
}

Guchar SplashBitmap::getAlpha(int x, int y) {
  return alpha[y * width + x];
}

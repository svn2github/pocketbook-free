/*
 * Copyright (C) 2008 Alexander Egorov <lunohod@gmx.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <algorithm>

#include <iostream>
#include <vector>
#include <map>
#include <dirent.h>

#include <ZLUnicodeUtil.h>
#include <ZLImage.h>
#include "../image/ZLNXImageManager.h"

#include "ZLNXPaintContext.h"
#include <inkview.h>
#include <inkinternal.h>

using namespace std;

struct xxx_link {
	int x1, y1, x2, y2;
	int kind;
	std::string id;
	bool next;
};
extern int imgposx, imgposy;
extern std::vector<xxx_link> xxx_page_links;
extern iv_wlist *xxx_wordlist;
extern int xxx_wlistlen, xxx_wlistsize;
extern char *screenbuf;
extern int use_antialiasing;
extern int lock_drawing;
#define ROUND_26_6_TO_INT(valuetoround) (((valuetoround) + 63) >> 6)

static FT_Matrix imatrix = { 65536, 15000, 0, 65536 };

#define setPixelPointer(x, y, c, s) { c=screenbuf+((x)>>2)+((myWidth*(y))>>2); s=((x)&3)<<1; }


ZLNXPaintContext::ZLNXPaintContext() {
//	myWidth = 600;
//	myHeight = 800;
	myWidth = 800;
	myHeight = 600;

	myStringHeight = -1;
	mySpaceWidth = -1;
	myDescent = 0;

	fCurFamily = "";
	fCurSize = 0;
	fCurItalic = false;
	fCurBold = false;

	FT_Error error;

	error = FT_Init_FreeType( &library );

	//fPath.push_back("/mnt/fbreader/fonts/");
	//fPath.push_back("/mnt/FBREADER/FONTS/");
	////fPath.push_back("/mnt/CRENGINE/FONTS/");
	////fPath.push_back("/mnt/crengine/fonts/");
	//fPath.push_back("/root/fbreader/fonts/");
	//fPath.push_back("/root/crengine/fonts/");
	//fPath.push_back("/root/fonts/truetype/");
	fPath.push_back(USERFONTDIR);
	fPath.push_back(SYSTEMFONTDIR);

	cacheFonts();
}

ZLNXPaintContext::~ZLNXPaintContext() {

	for(std::map<std::string, std::map<int, Font> >::iterator x = fontCache.begin();
			x != fontCache.end();
			x++) {

		for(std::map<int, Font>::iterator y = x->second.begin();
				y != x->second.end();
				y++) {

			std::map<int, std::map<unsigned long, FT_BitmapGlyph> >::iterator piter = y->second.glyphCacheAll.begin();
			while(piter != y->second.glyphCacheAll.end()) {
				std::map<unsigned long, FT_BitmapGlyph>::iterator piter2 = piter->second.begin();
				while(piter2 != piter->second.end()) {
					FT_Bitmap_Done(library, (FT_Bitmap*)&(piter2->second->bitmap));			
					piter2++;
				}
				piter++;
			}

			if(y->second.myFace != NULL)
				FT_Done_Face(y->second.myFace);
		}
	}

	if(library)
		FT_Done_FreeType(library);
}


void ZLNXPaintContext::fillFamiliesList(std::vector<std::string> &families) const {
}

void ZLNXPaintContext::cacheFonts() const {
	DIR *dir_p;
	struct dirent *dp;
	int idx;
	bool bold, italic;
	int bi_hash;

	FT_Error error;
	FT_Face lFace;

	for(std::vector<std::string>::iterator it = fPath.begin();
			it != fPath.end();
			it++) {

		dir_p = NULL;
		dir_p = opendir(it->c_str());	
		if(dir_p == NULL)
			continue;

		while((dp = readdir(dir_p)) != NULL) {
			idx = strlen(dp->d_name);

			if(idx <= 4)
				continue;

			idx -= 4;
			if(strncasecmp(dp->d_name + idx, ".TTF", 4) &&
				strncasecmp(dp->d_name + idx, ".ttf", 4))
				continue;


			std::string fFullName = it->c_str();
			fFullName += "/";
			fFullName += dp->d_name;					

			for(int i = 0 ;; i++) {
				error = FT_New_Face(library, fFullName.c_str(), i, &lFace);
				if(error)
					break;

				if(FT_IS_SCALABLE(lFace)) {
					bold = lFace->style_flags & FT_STYLE_FLAG_BOLD;				
					italic = lFace->style_flags & FT_STYLE_FLAG_ITALIC;				
					bi_hash = (bold?2:0) + (italic?1:0);

					Font *fc = &(fontCache[lFace->family_name])[bi_hash];

					if(fc->fileName.length() == 0) {
						fc->familyName = lFace->family_name;
						fc->fileName = fFullName;
						fc->index = i;
						fc->isBold = bold;
						fc->isItalic = italic;
					}
				}

				FT_Done_Face(lFace);
			}
		}

/*		
		cout << "---------------------" << endl;

		for(std::map<std::string, std::map<int, Font> >::iterator x = fontCache.begin();
				x != fontCache.end();
				x++) {
			cout << "family: " << x->first << endl;

			for(std::map<int, Font>::iterator y = x->second.begin();
					y != x->second.end();
					y++) {
				cout << "	hash: " << y->first << endl;
				cout << "	file: " << y->second.fileName << endl;
				cout << "	index: " << y->second.index << endl;
				cout << "	b:	"	<< y->second.isBold << endl;
				cout << "	i:	"	<< y->second.isItalic << endl;
				cout << endl;
			}
		}
*/		

		closedir(dir_p);
	}
}

const std::string ZLNXPaintContext::realFontFamilyName(std::string &fontFamily) const {
	return fontFamily;
}

void ZLNXPaintContext::setFont(const std::string &family, int size, bool bold, bool italic) {

	//fprintf(stderr, "setFont: %s, %d, %d, %d\n", family.c_str(), size, bold?1:0, italic?1:0);
	FT_Error error;

	if((family == fCurFamily) && (bold == fCurBold) && (italic == fCurItalic) && (size == fCurSize)) {
		return;
	}

	fCurFamily = family;
	fCurBold = bold;
	fCurItalic = italic;

	std::string defFont("Liberation Sans");

	int bi_hash = (bold?2:0) + (italic?1:0);
	std::map<std::string, std::map<int, Font> >::iterator it = fontCache.find(family);
	std::map<std::string, std::map<int, Font> >::iterator it2 = fontCache.find(defFont);
	if (it == fontCache.end()) it = it2;

	Font *fc = &((it->second)[bi_hash]);

	/*
	   cout << "	hash: " << bi_hash << endl;
	   cout << "	family: " << fc->familyName << endl;
	   cout << "	file: " << fc->fileName << endl;
	   cout << "	index: " << fc->index << endl;
	   cout << "	b:	"	<< fc->isBold << endl;
	   cout << "	i:	"	<< fc->isItalic << endl;
	   cout << endl;
	   */

	if(fc->fileName.size() == 0)
		fc = &((it->second)[0]);

	if(fc->myFace == NULL) {
		error = FT_New_Face(library, fc->fileName.c_str(), fc->index, &fc->myFace);
		if(error) {
			return;
		}
	}

	if (fc->myFaceSub == NULL) {
		Font *fc2 = &((it2->second)[bi_hash]);
		if(fc2->myFace == NULL) {
			error = FT_New_Face(library, fc2->fileName.c_str(), fc2->index, &fc2->myFace);
			if(error) {
				return;
			}
		}
		fc->myFaceSub = fc2->myFace;
	}

	embold = (bold && ! fc->isBold);
	italize = (italic && ! fc->isItalic);

	face = &(fc->myFace);
	facesub = &(fc->myFaceSub);
	if(size >= 6)
		fCurSize = size;
	else
		fCurSize = 6; 

	//FT_Set_Char_Size( *face, fCurSize * 64, 0, 160, 0 );
	FT_Set_Pixel_Sizes( *face, fCurSize, 0);
	FT_Set_Pixel_Sizes( *facesub, fCurSize, 0);

	charWidthCache = &(fc->charWidthCacheAll[fCurSize]);
	glyphCache = &(fc->glyphCacheAll[fCurSize]);
	glyphIdxCache = &(fc->glyphIdxCacheAll[fCurSize]);
	kerningCache = &(fc->kerningCacheAll[fCurSize]);

	myStringHeight = (fCurSize * 160 / 72) / 2;
	myDescent = (abs((*face)->size->metrics.descender) + 63 ) >> 6;
	mySpaceWidth = -1;

}

void ZLNXPaintContext::setColor(ZLColor color, LineStyle style) {

	tColor = ((color.Red * 77 + color.Green * 151 + color.Blue * 28) >> 8);
	//fprintf(stderr, "[C:%02x%02x%02x]", color.Red, color.Green, color.Blue);
	//printf("setColor\n");
}

void ZLNXPaintContext::setFillColor(ZLColor color, FillStyle style) {
	//printf("setFillColor\n");
	//fprintf(stderr, "[F:%02x%02x%02x]", color.Red, color.Green, color.Blue);

	fColor = ((color.Red * 77 + color.Green * 151 + color.Blue * 28) >> 8);
	fColor &= 3;
	fColor <<= 6;
}

static int char_index(FT_Face f, int c) {

	int idx = FT_Get_Char_Index(f, c);
	if (idx == 0 && f->charmap->encoding_id == 0) {
		if (c >= 0x410 && c <= 0x4ff) {
			idx = c - 0x350;
		} else {
			switch (c) {
				case 0x401: idx = 0xa8; break;
				case 0x404: idx = 0xaa; break;
				case 0x407: idx = 0xaf; break;
				case 0x406: idx = 0xb2; break;
				case 0x456: idx = 0xb3; break;
				case 0x451: idx = 0xb8; break;
				case 0x454: idx = 0xba; break;
				case 0x457: idx = 0xbf; break;
				default: idx = c; break;
			}
		}
		idx = FT_Get_Char_Index(f, idx);
		if (idx == 0) {
			switch (c) {
				case 0x401: idx = 0xc5; break;
				case 0x404: idx = 0xc5; break;
				case 0x407: idx = 0x49; break;
				case 0x406: idx = 0x49; break;
				case 0x456: idx = 0x69; break;
				case 0x451: idx = 0xe5; break;
				case 0x454: idx = 0xe5; break;
				case 0x457: idx = 0x69; break;
				default: idx = c; break;
			}
			idx = FT_Get_Char_Index(f, idx);
		}

	}
	return idx;

}

int ZLNXPaintContext::stringWidth(const char *str, int len) const {
	int w = 0;
	int ch_w;
	char *p = (char *)str;
	unsigned long         codepoint;
	unsigned char         in_code;
	int                   expect = 0;
	FT_Face uface;
	FT_UInt glyph_idx = 0;
	FT_UInt previous;
	FT_Bool use_kerning;
	FT_Vector delta; 
	int kerning = 0;

	use_kerning = (*face)->face_flags & FT_FACE_FLAG_KERNING;

	while ( *p && len-- > 0)
	{
		in_code = *p++ ;

		if ( in_code >= 0xC0 )
		{
			if ( in_code < 0xE0 )           /*  U+0080 - U+07FF   */
			{
				expect = 1;
				codepoint = in_code & 0x1F;
			}
			else if ( in_code < 0xF0 )      /*  U+0800 - U+FFFF   */
			{
				expect = 2;
				codepoint = in_code & 0x0F;
			}
			else if ( in_code < 0xF8 )      /* U+10000 - U+10FFFF */
			{
				expect = 3;
				codepoint = in_code & 0x07;
			}
			continue;
		}
		else if ( in_code >= 0x80 )
		{
			--expect;

			if ( expect >= 0 )
			{
				codepoint <<= 6;
				codepoint  += in_code & 0x3F;
			}
			if ( expect >  0 )
				continue;

			expect = 0;
		}
		else                              /* ASCII, U+0000 - U+007F */
			codepoint = in_code;

		if (codepoint == 0xad) continue;

		uface = *face;
		if(glyphIdxCache->find(codepoint) != glyphIdxCache->end()) {
			glyph_idx = (*glyphIdxCache)[codepoint];
			if (glyph_idx & 0x80000000) uface = *facesub;
		} else {
			glyph_idx = char_index(uface, codepoint);
			if (glyph_idx == 0) {
				uface = *facesub;
				glyph_idx = char_index(uface, codepoint) | 0x80000000;
			}
			(*glyphIdxCache)[codepoint] = glyph_idx;
		}
		glyph_idx &= ~0x80000000;

		if ( use_kerning && previous && glyph_idx ) { 
			if((kerningCache->find(glyph_idx) != kerningCache->end()) &&
				((*kerningCache)[glyph_idx].find(previous) != (*kerningCache)[glyph_idx].end())) {
				
				kerning = ((*kerningCache)[glyph_idx])[previous];
			} else {

				FT_Get_Kerning( uface, previous, glyph_idx, FT_KERNING_DEFAULT, &delta ); 
				kerning = delta.x >> 6;

				int *k = &((*kerningCache)[glyph_idx])[previous];
				*k = kerning;
			}
		} else {
			kerning = 0;
		}

		if(charWidthCache->find(codepoint) != charWidthCache->end()) {
			w += (*charWidthCache)[codepoint] + kerning;
		} else {
			if(!FT_Load_Glyph(uface, glyph_idx,  FT_LOAD_DEFAULT)) {
				ch_w = ROUND_26_6_TO_INT(uface->glyph->advance.x); // or face->glyph->metrics->horiAdvance >> 6
				w += ch_w + kerning;
				charWidthCache->insert(std::make_pair(codepoint, ch_w));
			} 
			//	else
			//		printf("glyph %d not found\n", glyph_idx);
		}
		previous = glyph_idx;
	}

	return w;
}

int ZLNXPaintContext::spaceWidth() const {
	if (mySpaceWidth == -1) {
		mySpaceWidth = stringWidth(" ", 1);
	}
	return mySpaceWidth;
}

int ZLNXPaintContext::stringHeight() const {
	if (myStringHeight == -1) {
		//FIXME
		//		myStringHeight = (*face)->size->metrics.height >> 6;
		//		printf("myStringHeight: %d\n", myStringHeight);
	}
	return myStringHeight;
}

int ZLNXPaintContext::descent() const {
	return myDescent;
}

// Denis //

bool HebrewTestWord(char *str, int size){

	int unic1=0;
	int i=0;
	while (i<size-2){
		if ((((str[i] & 0xE0) ^ 0xC0) == 0)&&(((str[i+1] & 0xC0) ^ 0x80) == 0)){
				unic1=(str[i] & 0x1F)<<6;
				unic1=unic1 | (str[i+1] & 0x3F);
		}
		if ((((str[i] & 0xF0) ^ 0xE0) == 0)&&(((str[i+1] & 0xC0) ^ 0x80) == 0)&&(((str[i+2] & 0xC0) ^ 0x80) == 0)){
			unic1=(str[i] & 0x0F)<<12;
			unic1=unic1 | ((str[i+1] & 0x3F)<<6);
			unic1=unic1 | (str[i+2] & 0x3F);
		}
		if (((unic1>=0x05D0)&&(unic1<=0x05EA)) || 
			((unic1>=0x05F0)&&(unic1<=0x05F4)) || 
			((unic1>=0x05B0)&&(unic1<=0x05C3)) ||
			((unic1>=0xFB1D)&&(unic1<=0xFB4F))){
			return true;
		} else {
			unic1=0;
			i++;
		}
	}
	return false;
}

void HebrewInvert(char *Data2, unsigned int size){
	
	unsigned int i=0;
	int unic1=0;
	int unic2=0;
	unsigned int k=0;
	unsigned int bk;

	while (((Data2[k]=='(') || 
		(Data2[k]=='[') || 
		(Data2[k]=='<') ||
		(Data2[k]=='{') ||
		(Data2[k]=='\"'))
		 && (k<size-1)){
		k++;
	}
	
	if ( ((Data2[k]>='0') && (Data2[k]<='9')) || 
		((Data2[k]>='A') && (Data2[k]<='Z')) || 
		((Data2[k]>='a') && (Data2[k]<='z')) || 
		(size<=2) ||
		(k>size-2)
		){
		return;
	}

	k=0;		
//	unsigned int size=size;
	while (
		((Data2[0]=='(') ||
		(Data2[0]=='[') ||
		(Data2[0]=='<') ||
		(Data2[0]=='{'))
			&& (k<size)
		){
		if ((Data2[0]=='(') && (k<size)){
			memcpy(&Data2[0], &Data2[1], size-1);
			Data2[size-1]=')';
			k++;
			size--;
		}
		if ((Data2[0]=='[') && (k<size)){
			memcpy(&Data2[0], &Data2[1], size-1);
			Data2[size-1]=']';
			k++;
			size--;
		}
		if ((Data2[0]=='<') && (k<size)){
			memcpy(&Data2[0], &Data2[1], size-1);
			Data2[size-1]='>';
			k++;
			size--;
		}
		if ((Data2[0]=='{') && (k<size)){
			memcpy(&Data2[0], &Data2[1], size-1);
			Data2[size-1]='}';
			k++;
			size--;
		}
	}
	
	bk=0;
	while (
		((Data2[size+bk-1]==')') ||
		(Data2[size+bk-1]==']') ||
		(Data2[size+bk-1]=='>') ||
		(Data2[size+bk-1]=='}') ||
		(Data2[size+bk-1]==':') ||
		(Data2[size+bk-1]==';') ||
		(Data2[size+bk-1]=='.') ||
		(Data2[size+bk-1]==',') ||
		(Data2[size+bk-1]=='!') ||
		(Data2[size+bk-1]=='?'))
			&& (bk<size)
		){
		if ((Data2[size+bk-1]==')') && (bk<size)){
			for (i=size+bk-1; i>bk; i--)
				Data2[i]=Data2[i-1];
			Data2[bk]='(';
			bk++;
			size--;
		}
		if ((Data2[size+bk-1]==']') && (bk<size)){
			for (i=size+bk-1; i>bk; i--)
				Data2[i]=Data2[i-1];
			Data2[bk]='[';
			bk++;
			size--;
		}
		if ((Data2[size+bk-1]=='>') && (bk<size)){
			for (i=size+bk-1; i>bk; i--)
				Data2[i]=Data2[i-1];
			Data2[bk]='<';
			bk++;
			size--;
		}
		if ((Data2[size+bk-1]=='}') && (bk<size)){
			for (i=size+bk-1; i>bk; i--)
				Data2[i]=Data2[i-1];
			Data2[bk]='{';
			bk++;
			size--;
		}
		if ((Data2[size+bk-1]==':') && (bk<size)){
			for (i=size+bk-1; i>bk; i--)
				Data2[i]=Data2[i-1];
			Data2[bk]=':';
			bk++;
			size--;
		}
		if ((Data2[size+bk-1]==';') && (bk<size)){
			for (i=size+bk-1; i>bk; i--)
				Data2[i]=Data2[i-1];
			Data2[bk]=';';
			bk++;
			size--;
		}
		if ((Data2[size+bk-1]=='.') && (bk<size)){
			for (i=size+bk-1; i>bk; i--)
				Data2[i]=Data2[i-1];
			Data2[bk]='.';
			bk++;
			size--;
		}
		if ((Data2[size+bk-1]==',') && (bk<size)){
			for (i=size+bk-1; i>bk; i--)
				Data2[i]=Data2[i-1];
			Data2[bk]=',';
			bk++;
			size--;
		}
		if ((Data2[size+bk-1]=='!') && (bk<size)){
			for (i=size+bk-1; i>bk; i--)
				Data2[i]=Data2[i-1];
			Data2[bk]='!';
			bk++;
			size--;
		}
		if ((Data2[size+bk-1]=='?') && (bk<size)){
			for (i=size+bk-1; i>bk; i--)
				Data2[i]=Data2[i-1];
			Data2[bk]='?';
			bk++;
			size--;
		}
	}

	i=bk;
	if (size>1){
		if ((((Data2[i] & 0xE0) ^ 0xC0) == 0)&&(((Data2[i+1] & 0xC0) ^ 0x80) == 0)){
			unic1=(Data2[i] & 0x1F)<<6;
			unic1=unic1 | (Data2[i+1] & 0x3F);
		}
		if ((((Data2[i] & 0xF0) ^ 0xE0) == 0)&&(((Data2[i+1] & 0xC0) ^ 0x80) == 0)&&(((Data2[i+2] & 0xC0) ^ 0x80) == 0)){
			unic1=(Data2[i] & 0x1F)<<12;
			unic1=unic1 | ((Data2[i+1] & 0x3F)<<6);
			unic1=unic1 | (Data2[i+2] & 0x3F);
		}
	}

	if (((unic1>=0x05D0)&&(unic1<=0x05EA)) || 
		((unic1>=0x05F0)&&(unic1<=0x05F4)) || 
		((unic1>=0x05B0)&&(unic1<=0x05C3)) || 
		((unic1>=0xFB1D)&&(unic1<=0xFB4F)) ||
   		((unic1>=0xA1) && (unic1<=0xBF)) ||
   		((unic1>=0x2010) && (unic1<=0x2046)) ||
		(Data2[i]>0)){
 		int unic1size, unic2size;
		while ((size>2)&&(size<1000)){
			char buf1[5], buf2[5];
			
			unic1=0;
			unic1size=0;
			unic2size=0;
			
		   	if (Data2[i]>0){
		   		unic1size=1;
		   	}
			if ((((Data2[i] & 0xE0) ^ 0xC0) == 0)&&(((Data2[i+1] & 0xC0) ^ 0x80) == 0)){
				unic1=(Data2[i] & 0x1F)<<6;
				unic1=unic1 | (Data2[i+1] & 0x3F);
				if (((unic1>=0x05D0)&&(unic1<=0x05EA)) || 
					((unic1>=0x05F0)&&(unic1<=0x05F4)) || 
					((unic1>=0x05B0)&&(unic1<=0x05C3)) ||
			   		((unic1>=0xA1) && (unic1<=0xBF))){
					unic1size=2;
				}
			}
			if ((((Data2[i] & 0xF0) ^ 0xE0) == 0)&&(((Data2[i+1] & 0xC0) ^ 0x80) == 0)&&(((Data2[i+2] & 0xC0) ^ 0x80) == 0)){
				unic1=(Data2[i] & 0x0F)<<12;
				unic1=unic1 | ((Data2[i+1] & 0x3F)<<6);
				unic1=unic1 | (Data2[i+2] & 0x3F);
			   	if (((unic1>=0xFB1D)&&(unic1<=0xFB4F)) ||
			   		((unic1>=0x2010) && (unic1<=0x2046))
			   		){
					unic1size=3;
				}
			}
			unic2=0;
		   	if (Data2[i+size-1]>0){
		   		unic2size=1;
		   	}
			if ((((Data2[i+size-2] & 0xE0) ^ 0xC0) == 0)&&(((Data2[i+size-1] & 0xC0) ^ 0x80) == 0)){
				unic2=(Data2[i+size-2] & 0x1F)<<6;
				unic2=unic2 | (Data2[i+size-1] & 0x3F);
				if (((unic2>=0x05D0)&&(unic2<=0x05EA)) || 
					((unic2>=0x05F0)&&(unic2<=0x05F4)) || 
					((unic2>=0x05B0)&&(unic2<=0x05C3)) ||
			   		((unic2>=0xA1) && (unic2<=0xBF))){
					unic2size=2;
				}
			}
			if ((((Data2[i+size-3] & 0xF0) ^ 0xE0) == 0)&&
				(((Data2[i+size-2] & 0xC0) ^ 0x80) == 0)&&
				(((Data2[i+size-1] & 0xC0) ^ 0x80) == 0)){
				unic2=(Data2[i+size-3] & 0x0F)<<12;
				unic2=unic2 | ((Data2[i+size-2] & 0x3F)<<6);
				unic2=unic2 | (Data2[i+size-1] & 0x3F);
			   	if (((unic2>=0xFB1D)&&(unic2<=0xFB4F)) ||
			   		((unic2>=0x2010) && (unic2<=0x2046))
			   		){
					unic2size=3;
				}
			}
			if ((unic1size==0)||(unic2size==0)){
				fprintf(stderr, "Denis - ZLTextView.cpp - HebrewInvert - Not Support Symbols - Invert was break\n\t unic1=%d unic2=%d\n", unic1, unic2);
				return;
			}
			if (unic1size>unic2size){
				memcpy(&buf1[0], &Data2[i], unic1size);
				memcpy(&buf2[0], &Data2[i+size-unic2size], unic2size);
				memcpy(&Data2[i+unic2size], &Data2[i+unic2size+(unic1size-unic2size)], size-unic2size-unic1size);
				memcpy(&Data2[i], &buf2[0], unic2size);
				memcpy(&Data2[i+size-unic1size], &buf1[0], unic1size);
				i+=unic2size;
				size-=(unic1size+unic2size);
			} else {
				if (unic1size<unic2size){
					memcpy(&buf1[0], &Data2[i], unic1size);
					memcpy(&buf2[0], &Data2[i+size-unic2size], unic2size);
					unsigned int l;
					for (l=0; l<size-unic1size-unic2size; l++)
						Data2[i+size-unic1size-l-1]=Data2[i+size-unic1size-l-(unic2size-unic1size)-1];
					memcpy(&Data2[i], &buf2[0], unic2size);
					memcpy(&Data2[i+size-unic1size], &buf1[0], unic1size);
					i+=unic2size;
					size-=(unic1size+unic2size);
				} else {
					memcpy(&buf1[0], &Data2[i], unic1size);
					memcpy(&buf2[0], &Data2[i+size-unic2size], unic2size);
					memcpy(&Data2[i], &buf2[0], unic2size);
					memcpy(&Data2[i+size-unic1size], &buf1[0], unic1size);					
					i+=unic2size;
					size-=(unic1size+unic2size);
				}
			}

		}
	} else {
		if (size>3){
			fprintf(stderr, "Denis - ZLTextView.cpp - HebrewInvert - Not Support Symbols - Invert was break\n\t unic1=%d\n", unic1);
		}
	}
	return;
}

// Denis //

void ZLNXPaintContext::drawString(int x, int y, const char *str, int len) {

	if (lock_drawing) return;
	//fprintf(stderr ,"[DrawString:%s]", str);

	FT_BitmapGlyph glyph;
	FT_BitmapGlyph *pglyph;
	FT_Vector     spen;                    /* untransformed origin  */
	FT_Vector     pen;                    /* untransformed origin  */

	FT_Face uface;
	FT_UInt glyph_idx = 0;
	FT_UInt previous;
	FT_Bool use_kerning;
	FT_Vector delta; 
	int slen;
	
	use_kerning = (*face)->face_flags & FT_FACE_FLAG_KERNING;

	char *p = (char *)str;

	unsigned long         codepoint, codepoint2;
	unsigned char         in_code;
	int                   expect;
	int kerning;

//	bool mark = false;
	int nch = 0;

	spen.x = pen.x = x;
	spen.y = pen.y = y;
	slen = len;

	while ( *p && len--)
	{
		in_code = *p++ ;

		if ( in_code >= 0xC0 )
		{
			if ( in_code < 0xE0 )           /*  U+0080 - U+07FF   */
			{
				expect = 1;
				codepoint = in_code & 0x1F;
			}
			else if ( in_code < 0xF0 )      /*  U+0800 - U+FFFF   */
			{
				expect = 2;
				codepoint = in_code & 0x0F;
			}
			else if ( in_code < 0xF8 )      /* U+10000 - U+10FFFF */
			{
				expect = 3;
				codepoint = in_code & 0x07;
			}
			continue;
		}
		else if ( in_code >= 0x80 )
		{
			--expect;

			if ( expect >= 0 )
			{
				codepoint <<= 6;
				codepoint  += in_code & 0x3F;
			}
			if ( expect >  0 )
				continue;

			expect = 0;
		}
		else                              /* ASCII, U+0000 - U+007F */
			codepoint = in_code;

		if (codepoint == 0xad) continue;
		nch++;

		uface = *face;
		if(glyphIdxCache->find(codepoint) != glyphIdxCache->end()) {
			glyph_idx = (*glyphIdxCache)[codepoint];
			if (glyph_idx & 0x80000000) uface = *facesub;
		} else {
			glyph_idx = char_index(uface, codepoint);
			if (glyph_idx == 0) {
				uface = *facesub;
				glyph_idx = char_index(uface, codepoint) | 0x80000000;
			}
			(*glyphIdxCache)[codepoint] = glyph_idx;
		}
		glyph_idx &= ~0x80000000;

		if ( use_kerning && previous && glyph_idx ) { 
			if((kerningCache->find(glyph_idx) != kerningCache->end()) &&
				((*kerningCache)[glyph_idx].find(previous) != (*kerningCache)[glyph_idx].end())) {
				
				kerning = ((*kerningCache)[glyph_idx])[previous];
			} else {

				FT_Get_Kerning(uface, previous, glyph_idx, FT_KERNING_DEFAULT, &delta ); 
				kerning = delta.x >> 6;

				int *k = &((*kerningCache)[glyph_idx])[previous];
				*k = kerning;
			}
			pen.x += kerning;		
		}

		codepoint2 = (codepoint << 2);
		if (embold)  codepoint2 |= 1;
		if (italize) codepoint2 |= 2;

		if(glyphCache->find(codepoint2) != glyphCache->end()) { 
			pglyph = &(*glyphCache)[codepoint2];
		} else {
			if(FT_Load_Glyph(uface, glyph_idx,  FT_LOAD_DEFAULT)){
				continue;
			}
			if (embold) {
				if ((uface)->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
				    FT_Outline_Embolden(&uface->glyph->outline, 63);
			}
			if (italize) {
				if ((uface)->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
				    FT_Outline_Transform(&uface->glyph->outline, &imatrix);
			}
			if (use_antialiasing == 2) {
				if ((uface)->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
				    FT_Outline_Embolden(&uface->glyph->outline, 15);
			}
			FT_Render_Glyph(uface->glyph,
				use_antialiasing ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO);

			FT_Get_Glyph(uface->glyph, (FT_Glyph*)&glyph);

			glyph->root.advance.x = uface->glyph->advance.x;	  			

			(*glyphCache)[codepoint2] = glyph;
			pglyph = &glyph;
		}

		drawGlyph( &(*pglyph)->bitmap,
				pen.x + (*pglyph)->left,
				pen.y - (*pglyph)->top);


/*		if(!mark) {
			drawLine(pen.x + (*pglyph)->left, y+1, pen.x + ((*pglyph)->root.advance.x >> 6), y+1);
			mark = true;
		}
*/

		/* increment pen position */
		pen.x += (*pglyph)->root.advance.x >> 6;
		previous = glyph_idx;
	}

	if (tColor >= 120) {
		invert(spen.x, spen.y-myStringHeight+myStringHeight/4, pen.x-spen.x, myStringHeight);
/*		char a[100];
		strncpy(a, str, slen);
		fprintf(stderr, "Denis - ZLNXPaintContext.cpp - drawString - tColor str=%s", a);*/
	}

	if (xxx_wlistlen+3 >= xxx_wlistsize) {
		xxx_wlistsize = xxx_wlistsize + (xxx_wlistsize >> 1) + 64;
		xxx_wordlist = (iv_wlist *) realloc(xxx_wordlist, xxx_wlistsize * sizeof(iv_wlist));
	}
	if (nch > 1) {
		iv_wlist *wp = &(xxx_wordlist[xxx_wlistlen++]);
		wp->x1 = imgposx+spen.x;
		wp->x2 = imgposx+pen.x;
		wp->y1 = imgposy+spen.y-myStringHeight+myStringHeight/4;
		wp->y2 = imgposy+spen.y+myStringHeight/4;
		wp->word = (char *) malloc(slen+1);
		memcpy(wp->word, str, slen);
// Denis //
// Вставка для разворота слова при передаче его в словарь //
		if (HebrewTestWord(wp->word, slen)){
			HebrewInvert(wp->word, slen);
		}
// Denis //
		wp->word[slen] = 0;
		xxx_wordlist[xxx_wlistlen].word = NULL;
	}

}

void ZLNXPaintContext::drawImage(int x, int y, const ZLImageData &image) {

	if (lock_drawing) return;
	fprintf(stderr, "[IM:%i,%i,%i,%i]", x, y, image.width(), image.height());

	char *c;
	char *c_src;
	int s, s_src;
	ZLNXImageData &nximage = (ZLNXImageData &)image;
	char *src = nximage.getImageData();
	int iW = nximage.width();
	int iH = nximage.height();
	int scanline = nximage.scanline();
	int jW, jH, xx, yy, slxx, slyy, step;

	if (x < 0) x = 0;
	jW = iW;
	jH = iH;
	if (iW > myWidth - x) {
		jW = myWidth - x;
		jH = (iH * (myWidth - x)) / iW;
	}
	if (jH > myHeight) {
		jW = (jW * myHeight) / jH;
		jH = myHeight;
	}
	if (y - iH < 0) y = iH;

	step = (iW * 1024) / jW;

	for(yy = 0; yy < jH; yy++) {
		slyy = ((yy * step) >> 10) * scanline;
		for(xx = 0; xx < jW; xx++) {
			 slxx = (xx * step) >> 10;
  			 c_src = src + (slxx >> 2) + slyy;
			 s_src = (slxx & 3) << 1;
			 setPixelPointer(x + xx, (y - iH) + yy, c, s);
			 *c &= ~(0xc0 >> s);
			 *c |= (((*c_src << s_src) & 0xc0) >> s);
		}
	}




}

void ZLNXPaintContext::drawLine(int x0, int y0, int x1, int y1) {
	if (lock_drawing) return;
	drawLine(x0, y0, x1, y1, false);
}

void ZLNXPaintContext::drawLine(int x0, int y0, int x1, int y1, bool fill) {

	if (lock_drawing) return;

	int i, j;
	int k, s;
	char *c = screenbuf;
	bool done = false;

	if (x0 < 0 || y0 < 0 || x1 < 0 || y1 < 0) return;
	if (x0 >= myWidth || y0 >= myHeight || x1 >= myWidth || y1 >= myHeight) return;
	
	if(x1 != x0) {
		k = (y1 - y0) / (x1 - x0);
		j = y0;
		i = x0;

		do {
			if(i == x1)
				done = true;

			setPixelPointer(i, j, c, s);

			*c &= ~(0xc0 >> s);

			if(fill)
				*c |= (fColor >> s);


			j += k;

			if(x1 > x0)
				i++;
			else 
				i--;

		} while(!done);

	} else {
		i = x0;
		j = y0;
//		s = (i & 3) << 1;

		do {
			if(j == y1)
				done = true;

			setPixelPointer(i, j, c, s);
			*c &= ~(0xc0 >> s);

			if(fill)
				*c |= (fColor >> s);

			if(y1 > y0)
				j++;
			else if(y1 < y0)
				j--;

		} while(!done);
	}
}

void ZLNXPaintContext::fillRectangle(int x0, int y0, int x1, int y1) {
	//printf("fillRectangle\n");

	if (lock_drawing) return;

	int j;

	j = y0;
	do {
		drawLine(x0, j, x1, j, true);

		if(y1 > y0)
			j++;
		else if(y1 < y0)
			j--;
	} while(( y1 > y0) && ( j <= y1 )  ||
			(j <= y0));
}

void ZLNXPaintContext::drawFilledCircle(int x, int y, int r) {
	//printf("drawFilledCircle\n");
}

void ZLNXPaintContext::clear(ZLColor color) {
	int i;
	if (lock_drawing) return;
	memset(screenbuf, 0xff, 1024*1024/4);
	xxx_page_links.clear();
	for (i=0; i<xxx_wlistlen; i++) free(xxx_wordlist[i].word);
	free(xxx_wordlist);
	xxx_wordlist = NULL;
	xxx_wlistsize = xxx_wlistlen = 0;
}

int ZLNXPaintContext::width() const {
	return myWidth;
}

int ZLNXPaintContext::height() const {
	return myHeight;
}

void ZLNXPaintContext::drawGlyph( FT_Bitmap*  bitmap, FT_Int x, FT_Int y)
{
	if (lock_drawing) return;
	FT_Int  i, j, p, q;
	FT_Int  x_max = x + bitmap->width;
	FT_Int  y_max = y + bitmap->rows;
	char *c = screenbuf;
	int mode = bitmap->pixel_mode;
	unsigned char *data = bitmap->buffer;
	unsigned char val;
	int s;

	for ( i = x, p = 0; i < x_max; i++, p++ ) {
		if (i < 0 || i >= myWidth) continue;
		for ( j = y, q = 0; j < y_max; j++, q++ ) {
			if (j < 0 || j >= myHeight ) continue;

			setPixelPointer(i, j, c, s);
			if (mode == FT_PIXEL_MODE_MONO) {
				val = data[q * bitmap->pitch + (p >> 3)];
				val = (val & (0x80 >> (p & 7))) ? 255 : 0;
			} else {
				val = data[q * bitmap->pitch + p];
			}
			if(val >= 192) {
				*c &= ~(0xc0 >> s);
			} else if (val >= 128) {
				*c &= ~(0x80 >> s);
			} else if (val >= 64) {
				*c &= ~(0x40 >> s);
			}

		}
	}
}

void ZLNXPaintContext::invert(int x, int y, int w, int h)
{
	char *c;
	int s, xx, yy;

	for (yy=y; yy<y+h; yy++) {
		for (xx=x; xx<x+w; xx++) {
			setPixelPointer(xx, yy, c, s);
			*c ^= (0xc0 >> s);
		}
	}
}


/*
void ZLNXPaintContext::setPixelPointer(int x, int y, char **c, int *s)
{
	switch (rotation()) {
		default:
		{
			*c = screenbuf + x / 4 + myWidth * y / 4;
			*s =  (x & 3) << 1;
			break;
		}
		case DEGREES90:
		{
			*c = buf + x * myHeight / 4 + (myHeight - y - 1) / 4;
			*s = (3 - y & 3) * 2;
			break;
		}
		case DEGREES180:
		{
//			*c = buf + (myWidth - x + 3) / 4 + (3 + myWidth * (myHeight- y)) / 4;
			*c = buf + x / 4 + myWidth * y / 4;
			*s =  (x & 3) << 1;
//			*s =  (3 - x & 3) << 1;

//			*c = buf +  (myWidth - x) * myHeight / 4 + y / 4;
//			*s = (y & 3) << 1;
			break;
		}
		case DEGREES270:
		{
			*c = buf +  (myWidth - x) * myHeight / 4 + y / 4;
			*s = (y & 3) << 1;
			break;
		}
	}
}
*/

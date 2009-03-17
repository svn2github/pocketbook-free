/*
 * Copyright (C) 2004-2008 Geometer Plus <contact@geometerplus.com>
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

#ifndef __HUFFDECOMPRESSOR_H__
#define __HUFFDECOMPRESSOR_H__

#include <string>
#include <shared_ptr.h>

class ZLInputStream;

class BitReader {

public:
	BitReader(unsigned char *data, int len);
	~BitReader();
	unsigned long peek(int n);
	bool eat(int n);
	size_t left();

private:
	unsigned char *adata;
	int pos;
	int nbits;

};


class HuffDecompressor {

public:
	HuffDecompressor(shared_ptr<ZLInputStream> &base, PdbHeader &header, size_t start, size_t count);
	~HuffDecompressor();
	size_t decompress(ZLInputStream &stream, char *buffer, size_t compressedSize, size_t maxUncompressedSize);

private:
	void do_unpack(BitReader *bits, int depth);
	unsigned char **huffs;
	int huffcount;
	unsigned long *dict1;
	unsigned long *dict2;
	unsigned char **dicts;
	bool ready;
	unsigned char *rbuffer;
	int rpos;
	int rmax;
	unsigned long entry_bits;

};

#endif /* __DOCDECOMPRESSOR_H__ */

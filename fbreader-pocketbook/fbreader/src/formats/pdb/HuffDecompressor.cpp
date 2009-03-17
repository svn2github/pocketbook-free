#include <string.h>

#include <ZLInputStream.h>

#include "PdbReader.h"
#include "HuffDecompressor.h"

BitReader::BitReader(unsigned char *data, int len) {

	adata = new unsigned char[len+8];
	memcpy(adata, data, len);
	memset(adata+len, 0, 8);
	pos = 0;
	nbits = len * 8;

}

BitReader::~BitReader() {
	delete adata;
}

unsigned long BitReader::peek(int n) {

        unsigned long long r = 0;
	unsigned long g = 0;
        while (g < n) {
		r = (r << 8) | adata[(pos+g)>>3];
		g = g + 8 - ((pos+g) & 7);
	}
        r = (r >> (g - n));
	r &= ((1LL << n) - 1LL);
	return (unsigned long)r;

}

bool BitReader::eat(int n) {

	pos += n;
	return (pos <= nbits);

}

size_t BitReader::left() {

	return nbits - pos;

}

static unsigned long unpack_ulong_msb(unsigned char *d, int off) {
	return (d[off] << 24) | (d[off+1] << 16) | (d[off+2] << 8) | d[off+3];
}

static unsigned long unpack_ulong_lsb(unsigned char *d, int off) {
	return (d[off+3] << 24) | (d[off+2] << 16) | (d[off+1] << 8) | d[off];
}

HuffDecompressor::HuffDecompressor(shared_ptr<ZLInputStream> &base, PdbHeader &header, size_t start, size_t count) {

	size_t curoff, nextoff, off1, off2;
	int i;

	huffs = new unsigned char * [count];
	huffcount = count;
	ready = false;

	for (i=0; i<count; i++) {
		curoff = header.Offsets[start+i];
		nextoff =
			(start+i+1 < header.Offsets.size()) ?
			header.Offsets[start+i+1] :
			base->sizeOfOpened();

		huffs[i] = new unsigned char[8+nextoff-curoff];
		base->seek(curoff, true);
		base->read((char *)huffs[i], nextoff-curoff);
		memset(huffs[i]+(nextoff-curoff), 0, 8);
	}

	if (strncmp((const char *)huffs[0], "HUFF", 4) != 0) {
		fprintf(stderr, "Invalid HUFF header\n");
		return;
	}
	if (strncmp((const char *)huffs[1], "CDIC", 4) != 0) {
		fprintf(stderr, "Invalid CDIC header\n");
		return;
	}

	entry_bits = unpack_ulong_msb(huffs[1], 12);
	off1 = unpack_ulong_msb(huffs[0], 16);
	off2 = unpack_ulong_msb(huffs[0], 20);
	dict1 = new unsigned long[256];
	for (i=0; i<256; i++) dict1[i] = unpack_ulong_lsb(huffs[0], off1+i*4);
	dict2 = new unsigned long[64];
	for (i=0; i<64; i++) dict2[i] = unpack_ulong_lsb(huffs[0], off2+i*4);
	dicts = huffs+1;
	ready = true;

}

HuffDecompressor::~HuffDecompressor() {
	int i;
	for (i=0; i<huffcount; i++) delete huffs[i];
	delete huffs;
	if (! ready) return;
	delete dict1;
	delete dict2;
}

void HuffDecompressor::do_unpack(BitReader *bits, int depth) {

	unsigned long dw, v, codelen, code, r, dicno, off1, off2, blen;
	unsigned char *dic, *slice;
	BitReader *br;

	//fprintf(stderr, "\nU(%i,%i) ", bits->left(), depth);

	if (depth > 32) {
		fprintf(stderr, "Corrupt file\n");
		ready = 0;
		return;
	}

	while (bits->left() > 0) {
		dw = bits->peek(32);
		v = dict1[dw >> 24];
 		codelen = v & 0x1f;
		if (codelen == 0) {
			fprintf(stderr, "codelen=0\n");
			ready = 0;
			return;
		}
		code = (codelen == 0) ? 0 : (dw >> (32 - codelen));
		r = (v >> 8);
		if ((v & 0x80) == 0) {
			while (code < dict2[(codelen-1)*2]) {
				codelen += 1;
				code = (codelen == 0) ? 0 : (dw >> (32 - codelen));
			}
			r = dict2[(codelen-1)*2+1];
		}
		r -= code;
		if (codelen == 0) {
			fprintf(stderr, "codelen=0\n");
			ready = 0;
			return;
		}
		if (! bits->eat(codelen)) return;
		dicno = (entry_bits == 32) ? 0 : (r >> entry_bits);
		off1 = 16 + (r - (dicno << entry_bits)) * 2;
		dic = dicts[dicno];
		off2 = 16 + dic[off1] * 256 + dic[off1+1];
		blen = dic[off2] * 256 + dic[off2+1];
		slice = dic+(off2+2);
		if (blen & 0x8000) {
			blen &= 0x7fff;
			if (rpos+blen > rmax-1) blen = rmax-1-rpos;
			if (blen != 0) memcpy(rbuffer+rpos, slice, blen);
			rbuffer[rpos+blen] = 0;
			//if (blen != 0) fprintf(stderr, "[%s]", rbuffer+rpos);
			rpos += blen;
		} else {
			do_unpack(br = new BitReader(slice, blen & 0x7fff), depth + 1);
			delete br;
			if (! ready) return;
		}

	}

}


size_t HuffDecompressor::decompress(ZLInputStream &stream, char *buffer, size_t csize, size_t maxsize) {

	BitReader *br;
	unsigned char *inbuffer = new unsigned char[csize+8];
	stream.read((char *)inbuffer, csize);
	memset(inbuffer+csize, 0, 8);
	rbuffer = (unsigned char *)buffer;
	rpos = 0;
	rmax = maxsize;
	do_unpack(br = new BitReader(inbuffer, csize), 0);
	delete br;
	delete inbuffer;
	return rpos;

}


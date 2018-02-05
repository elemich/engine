#ifndef TGA_READER
#define TGA_READER

#include <cstdio>

bool LoadUncompressedTGA(FILE * fTGA,const char* filename,void*& buf,int &bufsize,int &width,int &height,int &bpp);	// Load an Uncompressed file
bool LoadCompressedTGA(FILE * fTGA,const char* filename,void*& buf,int &bufsize,int &width,int &height,int &bpp);		// Load a Compressed file

bool LoadTGA(const char* filename,void*& buf,int &bufsize,int &width,int &height,int &bpp);

#endif
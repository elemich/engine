#ifndef TGA_READER
#define TGA_READER

bool LoadTGA(const char* filename,unsigned char*& buf,int &bufsize,int &width,int &height,int &bpp);

#endif
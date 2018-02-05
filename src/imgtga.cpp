/********************************************************************************
/Name:		TGA.cpp																*
/Header:	tga.h																*
/Purpose:	Load Compressed and Uncompressed TGA files							*
/Functions:	LoadTGA(Texture * texture, char * filename)							*
/			LoadCompressedTGA(Texture * texture, char * filename, FILE * fTGA)	*
/			LoadUncompressedTGA(Texture * texture, char * filename, FILE * fTGA)*	
/*******************************************************************************/
#include "imgtga.h"

//#include <fstream>
#include <cstring>

/********************************************************************************
/name :		LoadTGA(Texture * texture, char * filename)							*
/function:  Open and test the file to make sure it is a valid TGA file			*	
/parems:	texture, pointer to a Texture structure								*
/			filename, string pointing to file to open							*
/********************************************************************************/

typedef struct
{
	unsigned char Header[12];									// TGA File Header
} TGAHeader;


typedef struct
{
	unsigned char header[6];								// First 6 Useful Bytes From The Header
	unsigned int		bytesPerPixel;							// Holds Number Of Bytes Per Pixel Used In The TGA File
	unsigned int		imageSize;								// Used To Store The Image Size When Setting Aside Ram
	unsigned int		temp;									// Temporary Variable
	unsigned int		type;	
	unsigned int		Height;									//Height of Image
	unsigned int		Width;									//Width ofImage
	unsigned int		Bpp;									// Bits Per Pixel
} TGA;


TGAHeader tgaheader;									// TGA header
TGA tga;												// TGA image data

unsigned char uTGAcompare[12] = {0,0,2, 0,0,0,0,0,0,0,0,0};	// Uncompressed TGA Header
unsigned char cTGAcompare[12] = {0,0,10,0,0,0,0,0,0,0,0,0};	// Compressed TGA Header

bool LoadTGA(const char* filename,void*& buf,int &bufsize,int &width,int &height,int &bpp)				// Load a TGA file
{
	FILE * fTGA;												// File pointer to texture file
	fTGA = fopen(filename, "rb");								// Open file for reading

	if(fTGA == NULL)											// If it didn't open....
	{
		printf("%s\n", "Could not open texture file");	// Display an error message
		return false;														// Exit function
	}

	if(fread(&tgaheader, sizeof(TGAHeader), 1, fTGA) == 0)					// Attempt to read 12 byte header from file
	{
		printf("%s\n", "Could not read file header");		// If it fails, display an error message 
		if(fTGA != NULL)													// Check to seeiffile is still open
		{
			fclose(fTGA);													// If it is, close it
		}
		
		//__debugbreak();
		
		
		return false;														// Exit function
	}

	if(memcmp(uTGAcompare, &tgaheader, sizeof(tgaheader)) == 0)				// See if header matches the predefined header of 
	{																		// an Uncompressed TGA image
		LoadUncompressedTGA(fTGA,filename,buf,bufsize,width,height,bpp);						// If so, jump to Uncompressed TGA loading code
	}
	else if(memcmp(cTGAcompare, &tgaheader, sizeof(tgaheader)) == 0)		// See if header matches the predefined header of
	{																		// an RLE compressed TGA image
		LoadCompressedTGA(fTGA,filename,buf,bufsize,width,height,bpp);							// If so, jump to Compressed TGA loading code
	}
	else																	// If header matches neither type
	{
		printf("%s\n", "TGA file be type 2 or type 10 ");	// Display an error
		fclose(fTGA);
		
		//__debugbreak();
		
		
		return false;																// Exit function
	}
	return true;															// All went well, continue on
}

bool LoadUncompressedTGA(FILE * fTGA,const char* filename,void*& buf,int &bufsize,int &width,int &height,int &bpp)	// Load an uncompressed TGA (note, much of this code is based on NeHe's 
{																			// TGA Loading code nehe.gamedev.net)
	if(fread(tga.header, sizeof(tga.header), 1, fTGA) == 0)					// Read TGA header
	{										
		printf("%s\n", "Could not read info header");		// Display error
		if(fTGA != NULL)													// if file is still open
		{
			fclose(fTGA);													// Close it
		}
		
		//__debugbreak();
		
		
		return false;														// Return failular
	}	

	width  = tga.header[1] * 256 + tga.header[0];					// Determine The TGA Width	(highbyte*256+lowbyte)
	height = tga.header[3] * 256 + tga.header[2];					// Determine The TGA Height	(highbyte*256+lowbyte)
	bpp	= tga.header[4];										// Determine the bits per pixel
	tga.Width		= width;										// Copy width into local structure						
	tga.Height		= height;										// Copy height into local structure
	tga.Bpp			= bpp;											// Copy BPP into local structure

	if((width <= 0) || (height <= 0) || ((bpp != 24) && (bpp !=32)))	// Make sure all information is valid
	{
		printf("%s\n", "Invalid texture information");	// Display Error
		if(fTGA != NULL)													// Check if file is still open
		{
			fclose(fTGA);													// If so, close it
		}

		//__debugbreak();

		return false;														// Return failed
	}

	/*if(bpp == 24)													// If the BPP of the image is 24...
		bpp	= GL_RGB;											// Set Image type to GL_RGB
	else																	// Else if its 32 BPP
		texture->type	= GL_RGBA;*/											// Set image type to GL_RGBA

	tga.bytesPerPixel	= (tga.Bpp / 8);									// Compute the number of BYTES per pixel
	tga.imageSize		= (tga.bytesPerPixel * tga.Width * tga.Height);		// Compute the total amout ofmemory needed to store data
	buf = new unsigned char[tga.imageSize];					// Allocate that much memory

	if(buf == NULL)											// If no space was allocated
	{
		printf("%s\n", "Could not allocate memory for image");	// Display Error
		fclose(fTGA);														// Close the file
		
		//__debugbreak();
		
		return false;														// Return failed
	}

	if(fread(buf, 1, tga.imageSize, fTGA) != tga.imageSize)	// Attempt to read image data
	{
		printf("%s\n", "Could not read image data");		// Display Error
		if(buf != NULL)										// If imagedata has data in it
		{
			delete [] buf;
			buf=0;
		}
		fclose(fTGA);		
		
		//__debugbreak();
		
		// Close file
		return false;														// Return failed
	}

	// Byte Swapping Optimized By Steve Thomas
	for(unsigned int cswap = 0; cswap < (int)tga.imageSize; cswap += tga.bytesPerPixel)
	{
		((unsigned char *)buf)[cswap] ^= ((unsigned char *)buf)[cswap+2] ^=
			((unsigned char *)buf)[cswap] ^= ((unsigned char *)buf)[cswap+2];
	}

	fclose(fTGA);															// Close file
	return true;															// Return success
}

bool LoadCompressedTGA(FILE * fTGA,const char* filename,void*& buf,int &bufsize,int &width,int &height,int &bpp)		// Load COMPRESSED TGAs
{ 
	if(fread(tga.header, sizeof(tga.header), 1, fTGA) == 0)					// Attempt to read header
	{
		printf("%s\n", "Could not read info header");		// Display Error
		if(fTGA != NULL)													// If file is open
		{
			fclose(fTGA);													// Close it
		}

		//__debugbreak();

		return false;														// Return failed
	}

	width  = tga.header[1] * 256 + tga.header[0];					// Determine The TGA Width	(highbyte*256+lowbyte)
	height = tga.header[3] * 256 + tga.header[2];					// Determine The TGA Height	(highbyte*256+lowbyte)
	bpp	= tga.header[4];										// Determine Bits Per Pixel
	tga.Width		= width;										// Copy width to local structure
	tga.Height		= height;										// Copy width to local structure
	tga.Bpp			= bpp;											// Copy width to local structure

	if((width <= 0) || (height <= 0) || ((bpp != 24) && (bpp !=32)))	//Make sure all texture info is ok
	{
		printf("%s\n", "Invalid texture information");	// If it isnt...Display error
		if(fTGA != NULL)													// Check if file is open
		{
			fclose(fTGA);													// Ifit is, close it
		}

		//__debugbreak();

		return false;														// Return failed
	}

	/*if(texture->bpp == 24)													// If the BPP of the image is 24...
		texture->type	= GL_RGB;											// Set Image type to GL_RGB
	else																	// Else if its 32 BPP
		texture->type	= GL_RGBA;	*/										// Set image type to GL_RGBA

	tga.bytesPerPixel	= (tga.Bpp / 8);									// Compute BYTES per pixel
	tga.imageSize		= (tga.bytesPerPixel * tga.Width * tga.Height);		// Compute amout of memory needed to store image
	buf	= new unsigned char[tga.imageSize];					// Allocate that much memory

	if(buf == NULL)											// If it wasnt allocated correctly..
	{
		printf("%s\n", "Could not allocate memory for image");	// Display Error
		fclose(fTGA);
		

		//__debugbreak();

		return false;														// Return failed
	}

	unsigned int pixelcount	= tga.Height * tga.Width;							// Nuber of pixels in the image
	unsigned int currentpixel	= 0;												// Current pixel being read
	unsigned int currentbyte	= 0;												// Current byte 
	unsigned char * colorbuffer = new unsigned char[tga.bytesPerPixel];			// Storage for 1 pixel

	do
	{
		unsigned char chunkheader = 0;											// Storage for "chunk" header

		if(fread(&chunkheader, sizeof(unsigned char), 1, fTGA) == 0)				// Read in the 1 byte header
		{
			printf("%s\n", "Could not read RLE header");	// Display Error
			if(fTGA != NULL)												// If file is open
			{
				fclose(fTGA);												// Close file
			}
			if(buf != NULL)									// If there is stored image data
			{
				delete [] buf;
				buf=0;									// Delete image data
			}

			//__debugbreak();

			return false;													// Return failed
		}

		if(chunkheader < 128)												// If the ehader is < 128, it means the that is the number of RAW color packets minus 1
		{																	// that follow the header
			chunkheader++;													// add 1 to get number of following color values
			for(short counter = 0; counter < chunkheader; counter++)		// Read RAW color values
			{
				if(fread(colorbuffer, 1, tga.bytesPerPixel, fTGA) != tga.bytesPerPixel) // Try to read 1 pixel
				{
					printf("%s\n", "Could not read image data");		// IF we cant, display an error

					if(fTGA != NULL)													// See if file is open
					{
						fclose(fTGA);													// If so, close file
					}

					if(colorbuffer != NULL)												// See if colorbuffer has data in it
					{
						delete [] colorbuffer;
						colorbuffer=0;										// If so, delete it
					}

					if(buf != NULL)										// See if there is stored Image data
					{
						delete [] buf;
						buf=0;											// If so, delete it too
					}

					//__debugbreak();

					return false;														// Return failed
				}
				// write to memory
				((unsigned char *)buf)[currentbyte		] = colorbuffer[2];				    // Flip R and B vcolor values around in the process 
				((unsigned char *)buf)[currentbyte + 1	] = colorbuffer[1];
				((unsigned char *)buf)[currentbyte + 2	] = colorbuffer[0];

				if(tga.bytesPerPixel == 4)												// if its a 32 bpp image
				{
					((unsigned char *)buf)[currentbyte + 3] = colorbuffer[3];				// copy the 4th byte
				}

				currentbyte += tga.bytesPerPixel;										// Increase thecurrent byte by the number of bytes per pixel
				currentpixel++;															// Increase current pixel by 1

				if(currentpixel > pixelcount)											// Make sure we havent read too many pixels
				{
					printf("%s\n", "Too many pixels read");			// if there is too many... Display an error!

					if(fTGA != NULL)													// If there is a file open
					{
						fclose(fTGA);													// Close file
					}	

					if(colorbuffer != NULL)												// If there is data in colorbuffer
					{
						delete [] colorbuffer;
						colorbuffer=0;													// Delete it
					}

					if(buf != NULL)										// If there is Image data
					{
						delete [] buf;
						buf=0;										// delete it
					}

					//__debugbreak();

					return false;														// Return failed
				}
			}
		}
		else																			// chunkheader > 128 RLE data, next color reapeated chunkheader - 127 times
		{
			chunkheader -= 127;															// Subteact 127 to get rid of the ID bit
			if(fread(colorbuffer, 1, tga.bytesPerPixel, fTGA) != tga.bytesPerPixel)		// Attempt to read following color values
			{	
				printf("%s\n", "Could not read from file");			// If attempt fails.. Display error (again)

				if(fTGA != NULL)														// If thereis a file open
				{
					fclose(fTGA);														// Close it
				}

				if(colorbuffer != NULL)													// If there is data in the colorbuffer
				{
					delete [] colorbuffer;
					colorbuffer=0;													// delete it
				}

				if(buf != NULL)											// If thereis image data
				{
					delete [] buf;
					buf=0;												// delete it
				}

				//__debugbreak();

				return false;															// return failed
			}

			for(short counter = 0; counter < chunkheader; counter++)					// copy the color into the image data as many times as dictated 
			{																			// by the header
				((unsigned char *)buf)[currentbyte		] = colorbuffer[2];					// switch R and B bytes areound while copying
				((unsigned char *)buf)[currentbyte + 1	] = colorbuffer[1];
				((unsigned char *)buf)[currentbyte + 2	] = colorbuffer[0];

				if(tga.bytesPerPixel == 4)												// If TGA images is 32 bpp
				{
					((unsigned char *)buf)[currentbyte + 3] = colorbuffer[3];				// Copy 4th byte
				}

				currentbyte += tga.bytesPerPixel;										// Increase current byte by the number of bytes per pixel
				currentpixel++;															// Increase pixel count by 1

				if(currentpixel > pixelcount)											// Make sure we havent written too many pixels
				{
					printf("%s\n", "Too many pixels read");			// if there is too many... Display an error!

					if(fTGA != NULL)													// If there is a file open
					{
						fclose(fTGA);													// Close file
					}	

					if(colorbuffer != NULL)												// If there is data in colorbuffer
					{
						delete [] colorbuffer;
						colorbuffer=0;												// Delete it
					}

					if(buf != NULL)										// If there is Image data
					{
						delete [] buf;
						buf=0;										// delete it
					}

					//__debugbreak();

					return false;														// Return failed
				}
			}
		}
	}

	while(currentpixel < pixelcount);													// Loop while there are still pixels left
	fclose(fTGA);																		// Close the file
	return true;																		// return success
}

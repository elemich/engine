#include "entities.h"

#include <stdio.h>

//#include "include\renderer.h"
#include "picopng.h"
#include "jpgdecoder.h"
#include "tga_reader.h"

extern "C"
{
#pragma message ("@mic remove hidden __dso_handle")
	void *__dso_handle=NULL;
};

struct IMAGE
{
	void*	m_buf;
	int		m_bufsize;
	int		m_width;
	int		m_height;
	int		m_bpp;

	IMAGE():m_buf(0),m_bufsize(0),m_width(0),m_height(0),m_bpp(0){}
	~IMAGE(){if(m_buf)delete [] ((unsigned char*)m_buf);m_buf=0;}
};

IMAGE*& getdataref(void*& d)
{
	return (IMAGE*&)d;
}

IMAGE* getdata(void* d)
{
	return (IMAGE*)d;
}


////TEXTURE

Texture::Texture():
	Resource(RESOURCE_TEXTURE),
	texture_type(TEXTURE_GENERIC)
{
	/*TextureManager::Instance()->Push(this);
	TextureManager::Instance()->textures[texture_type].Push(this);*/
}

Texture::Texture(ETexture e):
	Resource(RESOURCE_TEXTURE),
	texture_type(e)
{
	/*TextureManager::Instance()->Push(this);
	TextureManager::Instance()->textures[texture_type].Push(this);*/
}



TextureFile::TextureFile():
	Texture(TEXTURE_FILE)
{
	this->__data=new IMAGE;
}

TextureFile::TextureFile(ETexture e)
{
	this->__data=new IMAGE;
}

TextureFile::~TextureFile()
{
	if(this->__data)
		delete ((IMAGE*)this->__data);
	this->__data=NULL;
}

int TextureFile::load(char* fn)
{
	FILE *f=fopen(fn,"rb");

	if(f)
	{
		short int bmp_signature=0x4d42;
		int jpg_signature1=0xe0ffd8ff;
		int jpg_signature2=0xdbffd8ff;
		int jpg_signature3=0xe1ffd8ff;
		int png_signature1=0x474e5089;
		int png_signature2=0x0a1a0a0d;

		int sign1;
		int sign2;
		
		fread(&sign1,4,1,f);
		fread(&sign2,4,1,f);

		if(bmp_signature==(short int)sign1)
		{
			fclose(f);
			return loadBMP(fn);
		}
		else if(jpg_signature1==sign1 || jpg_signature3==sign1 || jpg_signature2==sign2  || jpg_signature3==sign2 )
		{
			fclose(f);
			return loadJPG(fn);
		}
		else if(png_signature1==sign1 && png_signature2==sign2)
		{
			fclose(f);
			return loadPNG(fn);
		}
		else if(655360==sign1 && 0==sign2)
		{
			fclose(f);
			return loadTGA(fn);
		}

		fclose(f);
	}

	return 1;
}

int TextureFile::loadJPG(char* fn)
{
	IMAGE* image=getdataref(this->__data);

	int ncomp;
	image->m_buf=(void*)jpgd::decompress_jpeg_image_from_file(fn,&image->m_width,&image->m_height,&ncomp,4);


	if(image->m_buf)
		return 0;

	return 1;
}

int TextureFile::loadPNG(char* fn)
{
	std::vector<unsigned char> image;
	unsigned long w, h;
	int error=-1;

	{
		std::string filename=fn;
		std::vector<unsigned char> buffer;
		loadFile(buffer, filename);

		error = decodePNG(image, w, h, buffer.empty() ? 0 : &buffer[0], (unsigned long)buffer.size());
	}
	

	if(!error)
	{
		IMAGE* i=getdataref(this->__data);
		i->m_bufsize=image.size();
		i->m_buf=new unsigned char[i->m_bufsize];
		memcpy(i->m_buf,&image[0],i->m_bufsize);
		i->m_width=w;
		i->m_height=h;

		return 0;
	}
	return 1;

}

int TextureFile::loadTGA(char* fn)
{
	IMAGE* i=getdataref(this->__data);

	return LoadTGA(fn,i->m_buf,i->m_bufsize,i->m_width,i->m_height,i->m_bpp);
	return 1;
}

int TextureFile::loadBMP(char* filename)
{
	FILE* f=fopen(filename,"rb");

	if(f)
	{
		IMAGE* image=getdataref(this->__data);

		//@mic only for bitmap
		fseek(f,0x12,SEEK_SET);
		fread(&image->m_width,2,1,f);
		fseek(f,0x16,SEEK_SET);
		fread(&image->m_height,2,1,f);
		fseek(f,0x1c,SEEK_SET);
		fread(&image->m_bpp,2,1,f);

		rewind(f);


		fseek(f,0,SEEK_END);
		image->m_bufsize=ftell(f)-54;
		rewind(f);
		fseek(f,54,SEEK_SET);

		image->m_buf=new int[image->m_bufsize];

		fread(image->m_buf,1,image->m_bufsize,f);

		fclose(f);

		return 0;
	}

	return 1;
}

void* TextureFile::GetBuffer()
{
	return getdataref(this->__data)->m_buf;
}
int TextureFile::GetSize()
{
	return getdataref(this->__data)->m_bufsize;
}
int TextureFile::GetWidth()
{
	return getdataref(this->__data)->m_width;
}
int TextureFile::GetHeight()
{
	return getdataref(this->__data)->m_height;
}
int TextureFile::GetBpp()
{
	return getdataref(this->__data)->m_bpp;
}

void TextureFile::draw(RendererInterface* renderer)
{
	renderer->draw(this);
}



TextureLayered::TextureLayered():
	Texture(TEXTURE_LAYERED)
{

}

TextureProcedural::TextureProcedural():
	Texture(TEXTURE_LAYERED)
{

}



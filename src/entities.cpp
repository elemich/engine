#include "entities.h"

#include "imgpng.h"
#include "imgjpg.h"
#include "imgtga.h"

#include <algorithm>



///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

File::File(String iString):filename(iString),
handle(0){}

void File::SetFilename(String iFilename){this->filename=iFilename;}
bool File::Open(char* mode)
{
	if(!this->filename.Count())
		return false;

	this->handle=fopen(filename,mode);

	if(!handle)
		return false;

	return true;
}

bool File::IsOpen()
{
	return this->handle ? true : false;
}

void File::Close()
{
	this->handle ? fclose(this->handle) : 0;
}

bool File::Exist()
{
	if(!this->filename.Count())
		return false;

	return File::Exist(this->filename);
}

int File::Size()
{
	int result;

	if(this->Open())
	{
		int curPos=ftell(this->handle);
		fseek(this->handle,0,SEEK_END);
		result=ftell(this->handle);
		fseek(this->handle,0,curPos);
	}
	else
	{
		if(!this->filename.Count())
			return -1;

		result=File::Size(this->filename);
	}

	return result;
}

bool File::Create()
{
	return File::Create(this->filename);
}

bool File::Delete()
{
	if(this->filename.Count())
		return !::remove(this->filename);
	return false;
}

int File::CountOccurrences(char iChar)
{
	int occurrences=0;

	bool wasOpen=this->IsOpen();

	if(!this->IsOpen())
		this->Open();

	if(this->IsOpen())
	{
		int oldPos=ftell(this->handle);

		fseek(this->handle,0,SEEK_END);

		int ___size=ftell(this->handle);

		int ___i=0;
		while(___i < ___size)
		{
			fseek(this->handle,___i,SEEK_SET);

			char c=fgetc(this->handle);

			if(iChar==c)
				occurrences++;

			___i++;
		}

		fseek(this->handle,oldPos,SEEK_SET);
	}

	if(!wasOpen)
		this->Close();

	return occurrences;
}

//statics function


bool File::Create(const char* iFilename)
{
	FILE* tFile=fopen(iFilename,"w");
	if(tFile)
		fclose(tFile);
	else return false;

	return true;
}
bool File::Exist(const char* iFilename)
{
	FILE* tFile=fopen(iFilename,"r");

	if(tFile)
	{
		fclose(tFile);
		return true;
	}
	
	return false;
}
bool File::Delete(const char* iFilename)
{
	return !::remove(iFilename);
}

int File::Size(const char* iFilename)
{
	if(!iFilename)
		return -1;

	int RetVal;

	FILE* tFile=fopen(iFilename,"r");

	if(tFile)
	{
		fseek(tFile,0,SEEK_END);
		RetVal=ftell(tFile);
		fclose(tFile);
	}
	else
		RetVal=-2;
	
	return RetVal;
}

void* File::Read(int iSize,int iNum)
{
	if(this->handle)
	{
		void* ____data=new char[iNum];
		fread(____data,iSize,iNum,this->handle);
		return ____data;
	}

	return 0;
}

bool File::Write(void* iData,int iSize,int iNum)
{
	if(this->handle)
	{
		fwrite(iData,iSize,iNum,this->handle);
		return true;
	}

	return false;
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

ShaderInterface::ShaderInterface()
{}

ShaderInterface* ShaderInterface::Find(const char* name,bool exact)
{
	for(int i=0;i<(int)pool.size();i++)
	{
		ShaderInterface* element=pool[i];

		const char* programName=element->GetName();

		if(element && programName)
			if(exact ? 0==strcmp(programName,name) :  0!=strstr(programName,name))
				return pool[i];
	}

	return 0;
}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


Keyframe::Keyframe():time(0.0f),value(0){}

Influence::Influence():
cpIdx(0),
	nCpIdx(0),
	weight(0.0f)
{}

Cluster::Cluster():
bone(0),
	influences(NULL),
	ninfluences(0)
{}

KeyCurve::KeyCurve():
channel(INVALID_CHANNEL),
	start(-1),
	end(-1)
{}



AnimClip::AnimClip():
start(-1),
	end(-1)
{}

Animation::Animation():
entity(0),
	clipIdx(0),
	start(-1),
	end(-1)
{
}

void copychannel(EChannel channel,float& val,float* poff,float* roff,float* soff)
{
	if(!val)
		return;

	switch(channel)
	{
	case TRANSLATEX:poff[0]	= val; break;
	case TRANSLATEY:poff[1]	= val; break;
	case TRANSLATEZ:poff[2]	= val; break;
	case ROTATEX:roff[0] = val; break;
	case ROTATEY:roff[1] = val; break;
	case ROTATEZ:roff[2] = val; break;
	case SCALEX:soff[0]	= val; break;
	case SCALEY:soff[1]	= val; break;
	case SCALEZ:soff[2]	= val; break;
	}
}

float cubic_interpolation(float v0, float v1, float v2, float v3, float x)
{
	float P = (v3 - v2) - (v0 - v1);
	float Q = (v0 - v1) - P;
	float R = v2 - v0;
	float S = v1;

	float x2 = x * x;
	float x3 = x2 * x;


	return P * x3 + Q * x2 + R * x + S;
}


void AnimationController::add(Animation* anim)
{
	if(this->animations.empty())
	{
		this->start=anim->start;
		this->end=anim->end;
	}

	this->animations.push_back(anim);

	if(!this->animations.empty())
	{
		this->start=anim->start>this->start ? anim->start : this->start;
		this->end=anim->end<this->end ? anim->end : this->end;
	}
}

void AnimationController::Stop()
{
	this->play=false;
	this->lastFrameTime=0;
}

void AnimationController::Play()
{
	this->play=true;
}

void AnimationController::SetFrame(float iFrame)
{
	for(size_t i=0;i<this->animations.size();i++)
	{
		Animation* anim=this->animations[i];

		int keyIdx=0;

		if(anim->clipIdx<(int)anim->clips.size())
		{
			AnimClip* curvegroup=anim->clips[anim->clipIdx];

			if(curvegroup)
			{
				int numcurves=(int)curvegroup->curves.size();

				vec3 poff,roff,soff(1,1,1);
				float val=0;

				for(int curveIdx=0;curveIdx<numcurves;curveIdx++)
				{
					KeyCurve &curve=*curvegroup->curves[curveIdx];

					int			numCurveKeys=(int)curve.frames.size();
					int			lastKeyIdx=numCurveKeys-1;

					if(numCurveKeys==1)
					{
						val=curve.frames[0]->value;
						copychannel(curve.channel,val,poff,roff,soff);
					}
					else
					{
						for (keyIdx = 0; keyIdx < numCurveKeys; keyIdx++)
						{
							if(keyIdx!=lastKeyIdx)
							{
								if(!(iFrame>=curve.frames[keyIdx]->time && iFrame<=curve.frames[keyIdx+1]->time))
									continue;

								Keyframe	*aa=curve.frames[(keyIdx>0 ? keyIdx-1 : keyIdx)];
								Keyframe	*bb=curve.frames[keyIdx];
								Keyframe	*cc=curve.frames[keyIdx+1];
								Keyframe	*dd=curve.frames[(keyIdx < lastKeyIdx-1 ? keyIdx+2 : keyIdx+1)];

								float		t=(iFrame - bb->time) / (cc->time - bb->time);

								val=cubic_interpolation(aa->value,bb->value,cc->value,dd->value,t);

								copychannel(curve.channel,val,poff,roff,soff);

								break;
							}
							else
							{
								val=curve.frames[lastKeyIdx]->value;
								copychannel(curve.channel,val,poff,roff,soff);
							}
						}
					}
				}

				mat4 sm,rm,tm;

				if(poff.iszero())
					tm.translate(anim->entity->local.position());
				else
					tm.translate(poff);

				sm.scale(soff);

				rm.rotate(-roff[2],0,0,1);
				rm.rotate(-roff[1],0,1,0);
				rm.rotate(-roff[0],1,0,0);

				anim->entity->local=rm*sm*tm;
			}

		}
	}
}

void AnimationController::update()
{
	if(this->play)
	{
		this->SetFrame(this->cursor);

			float curDelta=this->lastFrameTime ? (Timer::instance->GetTime()-this->lastFrameTime)/1000.0f : 0;
			this->cursor+=curDelta*this->speed;
			this->lastFrameTime=Timer::instance->GetTime();

			if(this->cursor>this->end)
				this->cursor=this->cursor-this->end;
	}
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

Bone::Bone()
	:
root(0)
{
	int x;
	int y;
	int z;

	while(true){
		x=rand() % 2;
		y=rand() % 2;
		z=rand() % 2;

		srand(rand() % 10000);

		if(x>0 || y>0 || z>0)
			break;
	}

	color.make((float)x,(float)y,(float)z);
}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


Light::Light():
light_cast(0),
	light_volumetric(0),
	light_groundprojection(0),
	light_nearattenuation(0),
	light_farattenuation(0),
	light_shadows(0),
	light_intensity(0),
	light_innerangle(0),
	light_outerangle(0),
	light_fog(0),
	light_decaystart(0),
	light_nearstart(0),
	light_nearend(0),
	light_farstart(0),
	light_farend(0)
{}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


Material::Material()
{
	VectorMathNamespace::make(emissive,3,0.2f,0.2f,0.2f);
	VectorMathNamespace::make(diffuse,3,0.2f,0.2f,0.2f);
	VectorMathNamespace::make(normalmap,3,0.2f,0.2f,0.2f);
	VectorMathNamespace::make(bump,3,0.2f,0.2f,0.2f);
	VectorMathNamespace::make(transparent,3,0.2f,0.2f,0.2f);
	VectorMathNamespace::make(displacement,3,0.2f,0.2f,0.2f);
	VectorMathNamespace::make(ambient,3,0.2f,0.2f,0.2f);
	VectorMathNamespace::make(specular,3,0,0,0);
	VectorMathNamespace::make(reflection,3,0,0,0);

	fbump=0;
	femissive=0;
	fambient=0;
	fdiffuse=0;
	ftransparent=0;
	fdisplacement=0;
	fspecular=0;
	freflection=0;
	fshininess=0;
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

Mesh::Mesh():
controlpoints(NULL),
	ncontrolpoints(0),
	vertexindices(0),
	nvertexindices(0),
	texcoord(0),
	ntexcoord(0),
	normals(NULL),
	nnormals(0),
	npolygons(0),
	colors(NULL),
	ncolors(0),
	isCCW(true)
{
}


float** Mesh::GetControlPoints()
{
	return (float**)this->controlpoints;
}

int Mesh::GetNumControlPoints()
{
	return this->ncontrolpoints;
}

float** Mesh::GetTriangles()
{
	return (float**)this->vertexindices;
}

int Mesh::GetNumTriangles()
{
	return this->nvertexindices;
}

float** Mesh::GetUV()
{
	return (float**)this->texcoord;
}

int Mesh::GetNumUV()
{
	return this->ntexcoord;
}


float** Mesh::GetNormals()
{
	return (float**)this->normals;
}

int Mesh::GetNumNormals()
{
	return this->nnormals;
}

std::vector<Material*>& Mesh::GetMaterials()
{
	return this->materials;
}

void Mesh::draw(Renderer3DInterfaceBase* renderer3d)
{
	renderer3d->draw(this);
}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////



Skin::Skin():
textures(NULL),
	ntextures(0),
	clusters(NULL),
	nclusters(0),
	vertexcache(NULL)
{
}


void Skin::update()
{
	if(this->vertexcache)
		delete [] this->vertexcache;

	this->vertexcache=new float[this->ncontrolpoints*3];
	float (*wcache)[3]=new float[this->ncontrolpoints][3];

	memset(wcache,0,this->ncontrolpoints*sizeof(float)*3);

	for(int tncluster=0;tncluster<this->nclusters;tncluster++)
	{
		Cluster *clu=&clusters[tncluster];

		float *src=NULL;
		float *dst=NULL;

		if(!clu || (clu && !clu->bone))
			continue;

		mat4 palette=clu->offset * clu->bone->world;

		mat4 final;
		final.zero();

		final+=palette;

		mat4 skinmtx1=final;
		mat4 skinmtx2;

		for(int nw=0;nw<clu->ninfluences;nw++)
		{
			Influence &inf=clu->influences[nw];

			src=this->controlpoints[inf.cpIdx[0]];
			dst=&this->vertexcache[inf.cpIdx[0]*3];	

			if(inf.weight!=1.0f)
			{
				float v[3];

				MatrixMathNamespace::transform(v,skinmtx1,src);

				wcache[inf.cpIdx[0]][0]+=v[0]*inf.weight;
				wcache[inf.cpIdx[0]][1]+=v[1]*inf.weight;
				wcache[inf.cpIdx[0]][2]+=v[2]*inf.weight;

				memcpy(dst,wcache[inf.cpIdx[0]],3*sizeof(float));
			}
			else
			{
				MatrixMathNamespace::transform(dst,skinmtx1,src);
			}

			//dst=this->entity->local.transform(dst);

			for(int i=1;i<inf.nCpIdx;i++)
			{
				memcpy(&this->vertexcache[inf.cpIdx[i]*3],dst,3*sizeof(float));

			}
		}
	}

	delete [] wcache;
}

void Skin::draw(Renderer3DInterfaceBase* renderer3d)
{
	renderer3d->draw(this);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
Script::Script():runtime(0),handle(0)
{

}
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////



extern "C"
{
    //#pragma message (LOCATION " remove hidden __dso_handle")
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

Texture::Texture()
{
	/*TextureManager::Instance()->Push(this);
	TextureManager::Instance()->textures[texture_type].Push(this);*/
}




TextureFile::TextureFile()
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




TextureLayered::TextureLayered()
{

}

TextureProcedural::TextureProcedural()
{

}



///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////




EntityScript::EntityScript():entity(0){}



Entity::Entity():parent(0){}
Entity::~Entity(){}


void Entity::SetParent(Entity* iParent)
{
	Entity* oldParent=this->parent;
	this->parent=iParent;

	if(oldParent)
		oldParent->childs.erase(std::find(oldParent->childs.begin(),oldParent->childs.end(),this));

	if(this->parent)
		this->parent->childs.push_back(this);
}

void Entity::update()
{
	for(std::vector<EntityComponent*>::iterator it=this->components.begin();it!=this->components.end();it++)
		(*it)->update();

	this->parent ? this->world=(this->local * this->parent->world) : this->world;

	for(std::list<Entity*>::iterator it=this->childs.begin();it!=this->childs.end();it++)
		(*it)->update();
}

void Entity::draw(Renderer3DInterfaceBase* renderer)
{
	renderer->draw(this->local.position(),5,vec3(1,1,1));

	for(std::vector<EntityComponent*>::iterator it=this->components.begin();it!=this->components.end();it++)
		(*it)->draw(renderer);

	for(std::list<Entity*>::iterator it=this->childs.begin();it!=this->childs.end();it++)
		(*it)->draw(renderer);
}



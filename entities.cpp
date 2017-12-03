#include "entities.h"

#include "interfaces.h"

#include <stdio.h>
#include <string>

#include <stdio.h>

#include "picopng.h"
#include "jpgdecoder.h"
#include "tga_reader.h"
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


Keyframe::Keyframe():time(0.0f),value(0){}

Influence::Influence():
influence_controlpointindex(0),
	influence_ncontrolpointindex(0),
	influence_weight(0.0f)
{}

Cluster::Cluster():
cluster_bone(0),
	cluster_influences(NULL),
	cluster_ninfluences(0)
{}

KeyCurve::KeyCurve():
keycurve_channel(INVALID_CHANNEL),
	keycurve_start(-1),
	keycurve_end(-1)
{}

void Gizmo::draw(Renderer3dInterface* renderer)
{
	renderer->draw(this);
}


AnimClip::AnimClip():
curvegroup_start(-1),
	curvegroup_end(-1)
{}

Animation::Animation():
entity(0),
	clipIdx(0),
	start(-1),
	end(-1)
{}

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

void AnimationController::update()
{
	if(this->play)
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
					int numcurves=(int)curvegroup->curvegroup_keycurves.size();

					vec3 poff,roff,soff(1,1,1);
					float val=0;

					for(int curveIdx=0;curveIdx<numcurves;curveIdx++)
					{
						KeyCurve &curve=*curvegroup->curvegroup_keycurves[curveIdx];

						int			numCurveKeys=(int)curve.keycurve_keyframes.size();
						int			lastKeyIdx=numCurveKeys-1;

						if(numCurveKeys==1)
						{
							val=curve.keycurve_keyframes[0]->value;
							copychannel(curve.keycurve_channel,val,poff,roff,soff);
						}
						else
						{
							for (keyIdx = 0; keyIdx < numCurveKeys; keyIdx++)
							{
								if(keyIdx!=lastKeyIdx)
								{
									if(!(this->cursor>=curve.keycurve_keyframes[keyIdx]->time && this->cursor<=curve.keycurve_keyframes[keyIdx+1]->time))
										continue;

									Keyframe	*aa=curve.keycurve_keyframes[(keyIdx>0 ? keyIdx-1 : keyIdx)];
									Keyframe	*bb=curve.keycurve_keyframes[keyIdx];
									Keyframe	*cc=curve.keycurve_keyframes[keyIdx+1];
									Keyframe	*dd=curve.keycurve_keyframes[(keyIdx < lastKeyIdx-1 ? keyIdx+2 : keyIdx+1)];

									float		t=(this->cursor - bb->time) / (cc->time - bb->time);

									val=cubic_interpolation(aa->value,bb->value,cc->value,dd->value,t);

									copychannel(curve.keycurve_channel,val,poff,roff,soff);

									break;
								}
								else
								{
									val=curve.keycurve_keyframes[lastKeyIdx]->value;
									copychannel(curve.keycurve_channel,val,poff,roff,soff);
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

			anim->entity->nAnimated++;
		}


		if(this->cursor>this->end)
		{
			this->cursor=0;

			if(!this->looped)
				this->play=false;
		}
		else
		{
			float curDelta=(Timer::instance->GetTime()-this->lastFrameTime)/1000.0f;
			this->cursor+=curDelta*this->speed;
			this->lastFrameTime=Timer::instance->GetTime();
		}
	}
}

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



void Bone::draw(Renderer3dInterface* renderer)
{
	renderer->draw(this);
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

ELight	Light::LightType()
{
	return light_type;
}

EDecay	Light::DecayType()
{
	return light_decaytype;
}

bool    Light::Cast()
{
	return light_cast;
}
bool    Light::Volumetric()
{
	return light_volumetric;
}
bool    Light::GroundProjection()
{
	return light_groundprojection;
}
bool    Light::NearAttenuation()
{
	return light_nearattenuation;
}
bool    Light::FarAttenuation()
{
	return light_farattenuation;
}
bool    Light::Shadows()
{
	return light_shadows;
}
float*	Light::Color()
{
	return light_color;
}
float*	Light::ShadowColor()
{
	return light_shadowcolor;
}
float	Light::Intensity()
{
	return light_intensity;
}
float	Light::InnerAngle()
{
	return light_innerangle;
}
float	Light::OuterAngle()
{
	return light_outerangle;
}
float  Light::Fog()
{
	return light_fog;
}
float	Light::DecayStart()
{
	return light_decaystart;
}
float	Light::NearStart()
{
	return light_nearstart;
}
float	Light::NearEnd()
{
	return light_nearend;
}
float	Light::FarStart()
{
	return light_farstart;
}
float	Light::FarEnd()
{
	return light_farend;
}




void Light::draw(Renderer3dInterface* renderer)
{

}

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
mesh_controlpoints(NULL),
	mesh_ncontrolpoints(0),
	mesh_vertexindices(0),
	mesh_nvertexindices(0),
	mesh_texcoord(0),
	mesh_ntexcoord(0),
	mesh_normals(NULL),
	mesh_nnormals(0),
	mesh_npolygons(0),
	mesh_colors(NULL),
	mesh_ncolors(0),
	mesh_isCCW(true)
{
}


float** Mesh::GetControlPoints()
{
	return (float**)this->mesh_controlpoints;
}

int Mesh::GetNumControlPoints()
{
	return this->mesh_ncontrolpoints;
}

float** Mesh::GetTriangles()
{
	return (float**)this->mesh_vertexindices;
}

int Mesh::GetNumTriangles()
{
	return this->mesh_nvertexindices;
}

float** Mesh::GetUV()
{
	return (float**)this->mesh_texcoord;
}

int Mesh::GetNumUV()
{
	return this->mesh_ntexcoord;
}


float** Mesh::GetNormals()
{
	return (float**)this->mesh_normals;
}

int Mesh::GetNumNormals()
{
	return this->mesh_nnormals;
}

std::vector<Material*>& Mesh::GetMaterials()
{
	return this->mesh_materials;
}


void Mesh::draw(Renderer3dInterface* renderer)
{
	renderer->draw(this);
}



///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////



Skin::Skin():
skin_textures(NULL),
	skin_ntextures(0),
	skin_clusters(NULL),
	skin_nclusters(0),
	skin_vertexcache(NULL)
{
}



void Skin::draw(Renderer3dInterface* renderer)
{
	renderer->draw(this);
}

void Skin::update()
{
	if(this->skin_vertexcache)
		delete [] this->skin_vertexcache;

	this->skin_vertexcache=new float[this->mesh_ncontrolpoints*3];
	float (*wcache)[3]=new float[this->mesh_ncontrolpoints][3];

	memset(wcache,0,this->mesh_ncontrolpoints*sizeof(float)*3);

	for(int tncluster=0;tncluster<this->skin_nclusters;tncluster++)
	{
		Cluster *clu=&skin_clusters[tncluster];

		float *src=NULL;
		float *dst=NULL;

		if(!clu || (clu && !clu->cluster_bone))
			continue;

		mat4 palette=clu->cluster_offset * clu->cluster_bone->world;

		mat4 final;
		final.zero();

		final+=palette;

		mat4 skinmtx1=final;
		mat4 skinmtx2;

		for(int nw=0;nw<clu->cluster_ninfluences;nw++)
		{
			Influence &inf=clu->cluster_influences[nw];

			src=this->mesh_controlpoints[inf.influence_controlpointindex[0]];
			dst=&this->skin_vertexcache[inf.influence_controlpointindex[0]*3];	

			if(inf.influence_weight!=1.0f)
			{
				float v[3];

				MatrixMathNamespace::transform(v,skinmtx1,src);

				wcache[inf.influence_controlpointindex[0]][0]+=v[0]*inf.influence_weight;
				wcache[inf.influence_controlpointindex[0]][1]+=v[1]*inf.influence_weight;
				wcache[inf.influence_controlpointindex[0]][2]+=v[2]*inf.influence_weight;

				memcpy(dst,wcache[inf.influence_controlpointindex[0]],3*sizeof(float));
			}
			else
			{
				MatrixMathNamespace::transform(dst,skinmtx1,src);
			}

			//dst=this->entity->local.transform(dst);

			for(int i=1;i<inf.influence_ncontrolpointindex;i++)
			{
				memcpy(&this->skin_vertexcache[inf.influence_controlpointindex[i]*3],dst,3*sizeof(float));

			}
		}
	}

	delete [] wcache;
}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////



extern "C"
{
#pragma message (LOCATION " remove hidden __dso_handle")
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

void TextureFile::draw(Renderer3dInterface* renderer)
{
	renderer->draw(this);
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

void _setEntitieslevel(Entity* e)
{
	if(!e)
		return;

	e->level=e->parent ? e->parent->level+1 : e->level;
	e->expanded=!e->level ? true : false;

	for_each(e->childs.begin(),e->childs.end(),_setEntitieslevel);
}


EntityScript::EntityScript(Entity* iEntity):entity(iEntity){}



Entity::Entity():
	selected(false),
	expanded(false),
	level(0),
	properties(0),
	script(0),
	module(0)
{
	nDrawed=0;
	nAnimated=0;
	nUpdated=0;
}

	
void Entity::SetParent(Entity* iParent)
{

	Entity* oldParent=this->parent;
	this->parent=iParent;

	if(oldParent)
		oldParent->childs.erase(std::find(oldParent->childs.begin(),oldParent->childs.end(),this));

	if(this->parent)
		this->parent->childs.push_back(this);

	_setEntitieslevel(this);
}

Entity* Entity::Create(Entity* iParent)
{
	Entity* e=new Entity;
	e->SetParent(iParent);
	return e;
}

Entity::~Entity()
{
}

void Entity::update()
{
	for(std::vector<EntityComponent*>::iterator it=this->components.begin();it!=this->components.end();it++)
		(*it)->update();

	if(this->script)
		this->script->update();

	this->parent ? this->world=(this->local * this->parent->world) : this->world;

	for(std::list<Entity*>::iterator it=this->childs.begin();it!=this->childs.end();it++)
		(*it)->update();
}


void Entity::draw(Renderer3dInterface* renderer)
{	
	MatrixStack::Push(MatrixStack::MODEL,this->world);

	renderer->draw(this->local.position(),5,vec3(1,1,1));

	for(std::vector<EntityComponent*>::iterator it=this->components.begin();it!=this->components.end();it++)
		(*it)->draw(renderer);

	for(std::list<Entity*>::iterator it=this->childs.begin();it!=this->childs.end();it++)
		(*it)->draw(renderer);

	MatrixStack::Pop(MatrixStack::MODEL);
}

#include "entities.h"

#include <algorithm>

#include <cstdlib>



///////////////////////////////////////////////
///////////////////////////////////////////////
////////////Resource load functions////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

Entity* loadEntityRecursively(Entity* iEntityParent,FILE* iFile,std::vector<EntityComponent*>& iComponents)
{
	int nameCount;
	int componentsSize;
	int childsSize;
	unsigned char componentCode;

	Entity* tEntity=new Entity;

	tEntity->SetParent(iEntityParent);

	wprintf(L"-------creating new entity-------\n");

	fread(&childsSize,sizeof(int),1,iFile);//4

	wprintf(L"childsize %d\n",childsSize);

	fread(tEntity->local,sizeof(float),16,iFile);//64
	fread(tEntity->world,sizeof(float),16,iFile);//64

	wprintf(L"local,world matrices\n");

	fread(&nameCount,sizeof(int),1,iFile);//4

	wprintf(L"name count %d\n",nameCount);

	StringUtils::ReadWstring(iFile,tEntity->name);

	wprintf(L"name: %s\n",tEntity->name.c_str());

	fread(tEntity->bbox.a,sizeof(float),3,iFile);//12
	fread(tEntity->bbox.b,sizeof(float),3,iFile);//12

	wprintf(L"bounding box\n");

	fread(&componentsSize,sizeof(int),1,iFile);//4

	wprintf(L"componentsSize %d\n",componentsSize);

	for(int i=0;i<componentsSize;i++)
	{
		fread(&componentCode,sizeof(unsigned char),1,iFile);//1

		wprintf(L"componentsCode %d\n",componentCode);

		if(componentCode==Serializer::Script)
		{
			Script* tScript=tEntity->CreateComponent<Script>();

			fread(&nameCount,sizeof(int),1,iFile);//4

			StringUtils::ReadWstring(iFile,tScript->file);

			wprintf(L"componentName %s\n",tScript->file.c_str());

			iComponents.push_back(tScript);
		}
	}

	if(childsSize)
	{
		for(int i=0;i<childsSize;i++)
			loadEntityRecursively(tEntity,iFile,iComponents);
	}
	else
		wprintf(L"---loadEntityRecursively end---\n");

	return tEntity;
}




#ifndef EDITORBUILD

FILE*        resourceData=0;
unsigned int resourceDataSize=0;
unsigned int resourceTableSize=0;
unsigned int resourceTableStart=0;
unsigned int resourceTableEnd=0;
unsigned int resourceDataStart=0;
unsigned int resourceDataEnd=0;


Scene* LoadScene(FilePath iSceneResource,FILE* iFile)
{
	Scene* tScene=new Scene;
	std::vector<EntityComponent*> tComponents;

	//assign the file name to the scene name
	tScene->name=iSceneResource.Name();

	wprintf(L"loading scene %s\n",tScene->name.c_str());

	//barely load entities and components
	if(!(tScene->entityRoot=loadEntityRecursively(tScene->entityRoot,resourceData,tComponents)))
		wprintf(L"error loading scene %s\n",tScene->name.c_str());
	else
		wprintf(L"approaching to load %d component(s)\n",tComponents.size());

	//eventually load needed resources
	int tComponentIdx=0;
	for(std::vector<EntityComponent*>::iterator tLoadedComponent=tComponents.begin();tLoadedComponent<tComponents.end();tLoadedComponent++,tComponentIdx++)
	{
		EntityComponent* tComponent=*tLoadedComponent;

		if(tComponent->is<Script>())
		{
			Script* tScript=dynamic_cast<Script*>(tComponent);

			wprintf(L"component %d is a script\n",tComponentIdx);

			if(tScript)
			{
				String tLibFile=L"lib" + tScript->file.Name() + L".so";

				wprintf(L"loading script %s\n",tLibFile.c_str());

				void *libhandle=0;
				
				#ifndef _MSC_VER
					libhandle=dlopen(StringUtils::ToChar(tLibFile).c_str(), 1 /*RTLD_LAZY*/);
				#endif

				if(libhandle)
				{
					wprintf(L"approaching to load create script class\n");

					EntityScript* (*tCreateFunction)()=0;
					
					#ifndef _MSC_VER
						tCreateFunction=(EntityScript* (*)())dlsym(libhandle,"Create");
					#endif

					if(tCreateFunction)
					{
						wprintf(L"create the %s class\n",tLibFile.c_str());

						tScript->runtime=tCreateFunction();

						if(tScript->runtime)
						{
							tScript->runtime->entity=tScript->entity;
							wprintf(L"init the script class\n");
							tScript->runtime->init();
						}
						else
							wprintf(L"error creating %s class\n",tLibFile.c_str());
					}
					else
						wprintf(L"error loading create script class\n");
				}
				else
					wprintf(L"error loading script %s\n",tLibFile.c_str());
			}
			else
				wprintf(L"error getting script pointer from EntityComponent\n");
		}
	}

	return tScene;
}

void* Resource::Load(FilePath iResourceName)
{
    fseek(resourceData,resourceTableStart,SEEK_SET);

    wchar_t         tFilePath[500];
    unsigned int    tFileStart=0;
    unsigned int    tFileSize=0;

    bool            tFileFound=false;

    while(ftell(resourceData)<resourceTableEnd)
    {
        fwscanf(resourceData,L"%s",tFilePath);
        fwscanf(resourceData,L"%u",&tFileStart);
        fwscanf(resourceData,L"%u",&tFileSize);

        if(iResourceName==String(tFilePath))
        {
            tFileFound=true;
            break;
        }
    }

    if(tFileFound)
    {
		fseek(resourceData,resourceDataStart+tFileStart,SEEK_SET);

		if(iResourceName.Extension()==String(L"engineScene"))
		{
			return LoadScene(iResourceName,resourceData);
		}
		else
		{
			char* rBuffer=new char[tFileSize];
			fread(rBuffer,tFileSize,1,resourceData);

			return rBuffer;
		}
    }

    return 0;
}

#endif





///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////Renderer3DBase/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

Renderer3DBase::Renderer3DBase():
	shader_unlit(0),
	shader_unlit_color(0),
	shader_unlit_texture(0),
	shader_font(0),
	shader_shaded_texture(0),
	picking(false)
{}

Renderer3DBase::~Renderer3DBase()
{
	
}

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////MatrixStack//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


#define MATRIXSTACK_ARRAY_SIZES 64


float matrixstack[MatrixStack::MATRIXMODE_MAX][MATRIXSTACK_ARRAY_SIZES][16];
int	  levels[MatrixStack::MATRIXMODE_MAX];
int	  mode;

mat4 MatrixStack::model;
mat4 MatrixStack::projection;
mat4 MatrixStack::view;

void MatrixStack::Reset()

{
	mode=MatrixStack::MODEL;

	for(int i=0;i<MatrixStack::MATRIXMODE_MAX;i++)
		for(int j=0;j<MATRIXSTACK_ARRAY_SIZES;j++)
			Matrix::identity(matrixstack[i][j]);

	levels[0]=levels[1]=0;
}



float* MatrixStack::Get(MatrixStack::matrixmode m,int lev)
{
	return matrixstack[m][(lev<0 ? levels[m] : lev)];
}

float* MatrixStack::Get()
{
	return Get((MatrixStack::matrixmode)mode);
}


void MatrixStack::SetProjectionMatrix(float* pm)
{
	memcpy(matrixstack[MatrixStack::PROJECTION][levels[MatrixStack::PROJECTION]],pm,sizeof(float)*16);
}
void MatrixStack::SetModelMatrix(float* mm)
{
	memcpy(matrixstack[MatrixStack::MODEL][levels[MatrixStack::MODEL]],mm,sizeof(float)*16);
}
void MatrixStack::SetViewMatrix(float* mm)
{
	memcpy(matrixstack[MatrixStack::VIEW][levels[MatrixStack::VIEW]],mm,sizeof(float)*16);
}

mat4 MatrixStack::GetProjectionMatrix()
{
	return matrixstack[PROJECTION][levels[PROJECTION]];
}
mat4 MatrixStack::GetModelMatrix()
{
	return matrixstack[MODEL][levels[MODEL]];
}

mat4 MatrixStack::GetViewMatrix()
{
	return matrixstack[VIEW][levels[VIEW]];
}


void MatrixStack::Push()
{
	Push((MatrixStack::matrixmode)mode);
}

void MatrixStack::Pop()
{
	Pop((MatrixStack::matrixmode)mode);
}



void MatrixStack::Identity()
{
	Identity((MatrixStack::matrixmode)mode);
}

void MatrixStack::Identity(MatrixStack::matrixmode m)
{
	Matrix::identity(Get(m));
}

void MatrixStack::Load(float* m)
{
	memcpy(matrixstack[mode][levels[mode]],m,sizeof(float)*16);
}

void MatrixStack::Load(MatrixStack::matrixmode md,float* m)
{
	memcpy(matrixstack[md][levels[md]],m,sizeof(float)*16);
}

void MatrixStack::Multiply(float* m)
{
	/*MatrixMathNamespace::multiply(m,matrixstack[mode][levels[mode]]);
	SetMatrix((MatrixStack::matrixmode)mode,m);*/
	Matrix::multiply(matrixstack[mode][levels[mode]],m);
}

void MatrixStack::Multiply(MatrixStack::matrixmode m,float* mtx)
{
	Matrix::multiply(Get(m),mtx);
}

void MatrixStack::Push(MatrixStack::matrixmode m)
{
	if(levels[m]<(MATRIXSTACK_ARRAY_SIZES-1))
	{
		levels[m]++;
		memcpy(matrixstack[m][levels[m]],matrixstack[m][levels[m]-1],sizeof(float)*16);
	}
}

void MatrixStack::Push(MatrixStack::matrixmode m,float* mtx)
{
	if(levels[m]<(MATRIXSTACK_ARRAY_SIZES-1))
	{
		levels[m]++;
		memcpy(matrixstack[m][levels[m]],mtx,sizeof(float)*16);
	}
}



void MatrixStack::Pop(MatrixStack::matrixmode m)
{
	if(levels[m]>0)
	{
		levels[m]--;
	}
}



void MatrixStack::Rotate(float a,float x,float y,float z)
{
	Matrix::rotate(Get(),a,x,y,z);
}

void MatrixStack::Translate(float x,float y,float z)
{
	float f[3]={x,y,z};
	Matrix::translate(Get(),f);
}

void MatrixStack::Scale(float x,float y,float z)
{
	Matrix::scale(Get(),Get(),x,y,z);
}

MatrixStack::matrixmode MatrixStack::GetMode()
{
	return (MatrixStack::matrixmode)mode;
}

void MatrixStack::SetMode(MatrixStack::matrixmode m)
{
	mode=m;
}

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////////Shader////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

std::vector<Shader*> ____shaders;

DLLBUILD std::vector<Shader*>& GetShadersPool()
{
	return ____shaders;
}

Shader::Shader(Renderer3DBase* iRenderer):
	renderer(iRenderer),
	programId(-1),
	vertexShaderId(-1),
	fragmentShaderId(-1)
{
	GetShadersPool().push_back(this);
}

Shader::~Shader()
{
	GetShadersPool().erase(std::find(GetShadersPool().begin(),GetShadersPool().end(),this));
}

Shader* Shader::Find(const char* iNameToFind,bool iExact)
{
	for(size_t i=0;i<GetShadersPool().size();i++)
	{
		Shader* element=GetShadersPool()[i];

		if(element)
		{
			std::string tShaderName=StringUtils::ToChar(element->name);

				if(iExact ? tShaderName==iNameToFind :  tShaderName.find(iNameToFind)!=std::string::npos)
					return element;
		}
		
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

KeyCurve::~KeyCurve()
{
}


AnimClip::AnimClip():
	start(-1),
	end(-1)
{}

AnimClip::~AnimClip(){}

Animation::Animation():
	entity(0),
	index(0),
	start(-1),
	end(-1)
{}

Animation::~Animation()
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
        case INVALID_CHANNEL:
            wprintf(L"copychannel: INVALID_CHANNEL selected\n");
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

AnimationController::AnimationController():
	speed(1),
	cursor(0),
	play(false),
	looped(true),
	start(0),
	end(0),
	frameTime(0),
	resolutionFps(60)
{}

AnimationController::~AnimationController(){}


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
	this->frameTime=0;
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

		if(anim->index<(int)anim->clips.size())
		{
			AnimClip* curvegroup=anim->clips[anim->index];

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

		int		tCurrentTime=Timer::GetInstance()->GetCurrent();

		float	tCurrentDelta=	this->frameTime ? (tCurrentTime-this->frameTime)/1000.0f : 0;

		this->cursor+=tCurrentDelta*this->speed;
		this->frameTime=tCurrentTime;

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
		x=std::rand() % 2;
		y=std::rand() % 2;
		z=std::rand() % 2;

		std::srand(std::rand() % 10000);

		if(x>0 || y>0 || z>0)
			break;
	}

	color.make((float)x,(float)y,(float)z);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////////Line/////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

Line::Line(){}

Line::~Line()
{
}

void Line::draw(Renderer3DBase* renderer3d)
{
	for(std::list<vec3>::iterator i=this->points.begin(),j=i++;i!=this->points.end() && j!=this->points.end();i++,j++)
		renderer3d->draw(*i,*j,vec3(1,1,1));
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


Material::Material():
	emissive(0.2f,0.2f,0.2f),
	diffuse(0.2f,0.2f,0.2f),
	normalmap(0.2f,0.2f,0.2f),
	bump(0.2f,0.2f,0.2f),
	transparent(0.2f,0.2f,0.2f),
	displacement(0.2f,0.2f,0.2f),
	ambient(0.2f,0.2f,0.2f),
	specular(0,0,0),
	reflection(0,0,0),
	fbump(0),
	femissive(0),
	fambient(0),
	fdiffuse(0),
	ftransparent(0),
	fdisplacement(0),
	fspecular(0),
	freflection(0),
	fshininess(0)
{}

Material::~Material(){}

void Material::update(){};

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
{}

Mesh::~Mesh(){}

void Mesh::draw(Renderer3DBase* renderer3d)
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

				Matrix::transform(v,skinmtx1,src);

				wcache[inf.cpIdx[0]][0]+=v[0]*inf.weight;
				wcache[inf.cpIdx[0]][1]+=v[1]*inf.weight;
				wcache[inf.cpIdx[0]][2]+=v[2]*inf.weight;

				memcpy(dst,wcache[inf.cpIdx[0]],3*sizeof(float));
			}
			else
			{
				Matrix::transform(dst,skinmtx1,src);
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

void Skin::draw(Renderer3DBase* renderer3d)
{
	renderer3d->draw(this);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
Script::Script():runtime(0){}


void Script::update()
{
	if(this->runtime)
        this->runtime->update();
}
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////




////TEXTURE

Texture::Texture():m_buf(0),m_bufsize(0),m_width(0),m_height(0),m_bpp(0){}
Texture::~Texture()
{
	SAFEDELETEARRAY(this->m_buf);

}




TextureFile::TextureFile()
{

}


TextureFile::~TextureFile()
{
}

void* TextureFile::GetBuffer()
{
	return this->m_buf;
}
int TextureFile::GetSize()
{
	return this->m_bufsize;
}
int TextureFile::GetWidth()
{
	return this->m_width;
}
int TextureFile::GetHeight()
{
	return this->m_height;
}
int TextureFile::GetBpp()
{
	return this->m_bpp;
}




TextureLayered::TextureLayered(){}

TextureLayered::~TextureLayered(){}

TextureProcedural::TextureProcedural(){}



///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////EntityScript///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

EntityScript::EntityScript():entity(0){}

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////Entity/////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

Entity::Entity():
	parent(0)
{}

Entity::~Entity()
{
	this->SetParent(0);

	for(std::vector<EntityComponent*>::iterator tCom=this->components.begin();tCom!=this->components.end();tCom++)
		SAFEDELETE(*tCom);

	for(std::list<Entity*>::iterator tEn=this->childs.begin();tEn!=this->childs.end();tEn++)
		SAFEDELETE(*tEn);
}

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

void Entity::draw(Renderer3DBase* renderer)
{
	renderer->draw(this->local.position(),5,vec3(1,1,1));

	for(std::vector<EntityComponent*>::iterator it=this->components.begin();it!=this->components.end();it++)
		(*it)->draw(renderer);

	for(std::list<Entity*>::iterator it=this->childs.begin();it!=this->childs.end();it++)
		(*it)->draw(renderer);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////////Scene/////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

Scene::Scene():entityRoot(0){}
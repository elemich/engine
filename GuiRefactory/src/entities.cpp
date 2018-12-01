#include "entities.h"

#include <algorithm>

#include <cstdlib>


///////////////////////////////
///////////////////////////////
////////////globals////////////
///////////////////////////////
///////////////////////////////

std::list<AnimationController*> globalAnimationControllers;

GLOBALGETTERFUNC(GlobalAnimationControllersInstance,globalAnimationControllers,std::list<AnimationController*>&);


///////////////////////////////////////////////
///////////////////////////////////////////////
////////////Resource load functions////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

namespace SerializerHelpers
{
	std::list<Skin*>							globalSkinsToBind;
	std::list< std::list<unsigned int> >		globalSkinsClusterBoneIdToBind;

	std::list<AnimationController*>				globalAnimationControllersToBind;
	std::list< std::list<unsigned int> >		globalAnimationControllersAnimationsIndicesToBind;

	void SetEntityId(Entity* iEntity,unsigned int& iId);
	Entity* GetRootEntity(Entity* iEntity);
	Entity* GetEntityById(Entity* iEntity,unsigned int iId);

	Entity* loadSceneEntityRecursively(Entity* iEntity,FILE* iFile);

	void Load(Mesh*,FILE*);
	void Load(Skin*,FILE*);
	void Load(Animation*,FILE*);
	void Load(AnimationController*,FILE*);
	void Load(Skin*,FILE*);
	void Load(Line*,FILE*);
	void Load(Script*,FILE*);

	extern void LoadScriptModule(Script*);
	extern void UnloadScriptModule(Script*);

	void BindSkinLinks(Entity* iEntityParent);
	void BindAnimationLinks(Entity* iEntityParent);
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
	//assign the file name to the scene name
	tScene->name=iSceneResource.Name();

	printf("loading scene %s\n",StringUtils::ToChar(tScene->name).c_str());

	tScene->entities=SerializerHelpers::loadSceneEntityRecursively(tScene->entities,resourceData);

	//barely load entities and components
	if(!tScene->entities)
		printf("error loading scene %s\n",StringUtils::ToChar(tScene->name).c_str());

	return tScene;
}

void* Resource::Load(FilePath iResourceName)
{
    fseek(resourceData,resourceTableStart,SEEK_SET);

    char	        tFilePath[500];
    unsigned int    tFileStart=0;
    unsigned int    tFileSize=0;

    bool            tFileFound=false;

    while(ftell(resourceData)<resourceTableEnd)
    {
        fscanf(resourceData,"%s %u %u",tFilePath,&tFileStart,&tFileSize);

        if(iResourceName==StringUtils::ToWide(tFilePath))
        {
            tFileFound=true;
            break;
        }
    }

    if(tFileFound)
    {
		printf("Resource::Load: %s found",StringUtils::ToChar(iResourceName).c_str());

		fseek(resourceData,resourceDataStart+tFileStart,SEEK_SET);

		String tFileExtension=iResourceName.Extension();
		String tSceneExtension(L"engineScene");

		printf("tFileExtension: %s , tSceneExtension: %s",StringUtils::ToChar(tFileExtension).c_str(),StringUtils::ToChar(tSceneExtension).c_str());

		if(tFileExtension==tSceneExtension)
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
	else
		printf("Resource::Load: %s not found",StringUtils::ToChar(iResourceName).c_str());

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
/////////////////Keyframe///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


Keyframe::Keyframe():time(0.0f),value(0){}

Influence::Influence():
controlpointindex(0),
	ncontrolpointindex(0),
	weight(0.0f)
{}

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////Cluster////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

Cluster::Cluster():
bone(0),
	influences(NULL),
	ninfluences(0)
{}

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////KeyCurve////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

KeyCurve::KeyCurve():
	channel(INVALID_CHANNEL),
	start(-1),
	end(-1)
{}

KeyCurve::~KeyCurve()
{
}

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////AnimClip///////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

AnimClip::AnimClip():
	start(-1),
	end(-1)
{}

AnimClip::~AnimClip(){}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////Animation////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////



Animation::Animation():
	index(0),
	start(-1),
	end(-1)
{}

Animation::~Animation()
{
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////AnimationController////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

void copychannel(unsigned int channel,float& val,float* poff,float* roff,float* soff)
{
	if(!val)
		return;

	switch(channel)
	{
        case KeyCurve::Channel::TRANSLATEX:poff[0]	= val; break;
        case KeyCurve::Channel::TRANSLATEY:poff[1]	= val; break;
        case KeyCurve::Channel::TRANSLATEZ:poff[2]	= val; break;
        case KeyCurve::Channel::ROTATEX:roff[0] = val; break;
        case KeyCurve::Channel::ROTATEY:roff[1] = val; break;
        case KeyCurve::Channel::ROTATEZ:roff[2] = val; break;
        case KeyCurve::Channel::SCALEX:soff[0]	= val; break;
        case KeyCurve::Channel::SCALEY:soff[1]	= val; break;
        case KeyCurve::Channel::SCALEZ:soff[2]	= val; break;
		case KeyCurve::Channel::INVALID_CHANNEL:
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

const std::list<AnimationController*> AnimationController::GetPool()
{
	return GlobalAnimationControllersInstance();
}

AnimationController::AnimationController():
	speed(1),
	cursor(0),
	play(false),
	looped(true),
	start(0),
	end(0),
	frameTime(0),
	framesPerSecond(60)
{
	GlobalAnimationControllersInstance().push_back(this);
}

AnimationController::~AnimationController()
{
	GlobalAnimationControllersInstance().remove(this);
}


void AnimationController::AddAnimation(Animation* iAnimation)
{
	if(this->animations.empty())
	{
		this->start=iAnimation->start;
		this->end=iAnimation->end;
	}

	this->animations.push_back(iAnimation);

	if(!this->animations.empty())
	{
		this->start=iAnimation->start>this->start ? iAnimation->start : this->start;
		this->end=iAnimation->end<this->end ? iAnimation->end : this->end;
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
	for(std::list<vec3>::iterator i=this->points.begin(),j;i!=this->points.end();i++)
	{
		j=i;
		std::advance(j,1);

		if(j!=this->points.end())
			renderer3d->draw(*i,*j,vec3(1,1,1));
	}
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


Light::Light():
	cast(0),
	volumetric(0),
	groundprojection(0),
	nearattenuation(0),
	farattenuation(0),
	shadows(0),
	intensity(0),
	innerangle(0),
	outerangle(0),
	fog(0),
	decaystart(0),
	nearstart(0),
	nearend(0),
	farstart(0),
	farend(0)
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
	ncontrolpoints(0),
	nvertexindices(0),
	ntexcoord(0),
	nnormals(0),
	npolygons(0),

	controlpoints(0),
	vertexindices(0),
	texcoord(0),
	normals(0),
	
	
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
	textures(0),
	ntextures(0),
	clusters(0),
	nclusters(0),
	vertexcache(0)
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

			src=this->controlpoints[inf.controlpointindex[0]];
			dst=&this->vertexcache[inf.controlpointindex[0]*3];

			if(inf.weight!=1.0f)
			{
				float v[3];

				Matrix::transform(v,skinmtx1,src);

				wcache[inf.controlpointindex[0]][0]+=v[0]*inf.weight;
				wcache[inf.controlpointindex[0]][1]+=v[1]*inf.weight;
				wcache[inf.controlpointindex[0]][2]+=v[2]*inf.weight;

				memcpy(dst,wcache[inf.controlpointindex[0]],3*sizeof(float));
			}
			else
			{
				Matrix::transform(dst,skinmtx1,src);
			}

			//dst=this->entity->local.transform(dst);

			for(int i=1;i<inf.ncontrolpointindex;i++)
			{
				memcpy(&this->vertexcache[inf.controlpointindex[i]*3],dst,3*sizeof(float));

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
Script::Script():runtime(0),handle(0){}

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

Camera::Camera():fov(90),ratio(16/9.0f),Near(1),Far(1000.0f),perspective(true){}
Camera::Camera(vec3 pos,vec3 target,vec3 up,float iFov,float iRatio,float iNear,float iFar,bool iPerspective):fov(iFov),ratio(iRatio),Near(iNear),Far(iFar),perspective(iPerspective)
{
	this->projection= !perspective ? this->projection : this->projection.perspective(90,16/9,1,1000);
	this->view.move(pos);
	this->view.lookat(target,up);
}

void Camera::update()
{

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
////////////////EntityComponent////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

EntityComponent::EntityComponent():entity(0){}
EntityComponent::~EntityComponent(){}

void EntityComponent::update(){}
void EntityComponent::draw(Renderer3DBase*){}
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
	parent(0),
	id(-1),
	saved(false)
{}

Entity::~Entity()
{
}

void Entity::SetParent(Entity* iParent)
{

}

void Entity::update()
{
	for(std::list<EntityComponent*>::iterator it=this->components.begin();it!=this->components.end();it++)
		(*it)->update();

	this->parent ? this->world=(this->local * this->parent->world) : this->world;

	for(std::list<Entity*>::iterator it=this->childs.begin();it!=this->childs.end();it++)
		(*it)->update();
}

void Entity::draw(Renderer3DBase* renderer)
{
	renderer->draw(this->local.position(),5,vec3(1,1,1));

	for(std::list<EntityComponent*>::iterator it=this->components.begin();it!=this->components.end();it++)
		(*it)->draw(renderer);

	for(std::list<Entity*>::iterator it=this->childs.begin();it!=this->childs.end();it++)
		(*it)->draw(renderer);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////////Scene/////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

Scene::Scene():entities(0){}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////SerializerHelpers///////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

void SerializerHelpers::SetEntityId(Entity* iEntity,unsigned int& iId)
{
	iEntity->id=iId;

	wprintf(L"entity %s, id %i\n",iEntity->name.c_str(),iId);

	for(std::list<Entity*>::iterator entityIter=iEntity->childs.begin();entityIter!=iEntity->childs.end();entityIter++)
		SetEntityId(*entityIter,++iId);
}

Entity* SerializerHelpers::GetRootEntity(Entity* iEntity)
{
	return !iEntity->parent ? iEntity : GetRootEntity(iEntity->parent);
}

Entity* SerializerHelpers::GetEntityById(Entity* iEntity,unsigned int iId)
{
	if(iEntity->id==iId)
		return iEntity;

	for(std::list<Entity*>::iterator entityIter=iEntity->childs.begin();entityIter!=iEntity->childs.end();entityIter++)
	{
		Entity* tEntity=GetEntityById(*entityIter,iId);

		if(tEntity)
			return tEntity;
	}

	return 0;
}

void SerializerHelpers::Load(Mesh* tMesh,FILE* iFile)
{
	fread(&tMesh->ncontrolpoints,sizeof(unsigned int),1,iFile);
	fread(&tMesh->nvertexindices,sizeof(unsigned int),1,iFile);
	fread(&tMesh->ntexcoord,sizeof(unsigned int),1,iFile);
	fread(&tMesh->nnormals,sizeof(unsigned int),1,iFile);
	fread(&tMesh->npolygons,sizeof(unsigned int),1,iFile);
	fread(&tMesh->isCCW,sizeof(bool),1,iFile);

	if(tMesh->ncontrolpoints)
	{
		tMesh->controlpoints=new float[tMesh->ncontrolpoints][3];
		fread(tMesh->controlpoints,sizeof(float)*3*tMesh->ncontrolpoints,1,iFile);
	}

	if(tMesh->nvertexindices)
	{
		tMesh->vertexindices=new unsigned int[tMesh->nvertexindices];
		fread(tMesh->vertexindices,sizeof(unsigned int)*tMesh->nvertexindices,1,iFile);
	}

	if(tMesh->ntexcoord)
	{
		tMesh->texcoord=new float[tMesh->ntexcoord][2];
		fread(tMesh->texcoord,sizeof(float)*2*tMesh->ntexcoord,1,iFile);
	}
	
	if(tMesh->nnormals)
	{
		tMesh->normals=new float[tMesh->nnormals][3];
		fread(tMesh->normals,sizeof(float)*3*tMesh->nnormals,1,iFile);
	}
}


void SerializerHelpers::Load(Skin* tSkin,FILE* iFile)
{
	std::list<unsigned int>	globalSkinsToBindVector;

	SerializerHelpers::globalSkinsToBind.push_back(dynamic_cast<Skin*>(tSkin));

	fread(&tSkin->nclusters,sizeof(unsigned int),1,iFile);
	fread(&tSkin->ntextures,sizeof(unsigned int),1,iFile);

	tSkin->clusters=new Cluster[tSkin->nclusters];

	for(int clusterIdx=0;clusterIdx<tSkin->nclusters;clusterIdx++)
	{
		Cluster* tCluster=&tSkin->clusters[clusterIdx];

		unsigned int tBoneId;
		fread(&tBoneId,sizeof(unsigned int),1,iFile);

		globalSkinsToBindVector.push_back(tBoneId);

		fread(&tCluster->ninfluences,sizeof(unsigned int),1,iFile);
		fread(tCluster->offset,sizeof(float),16,iFile);

		tCluster->influences=new Influence[tCluster->ninfluences];

		for(int influenceIdx=0;influenceIdx<tCluster->ninfluences;influenceIdx++)
		{
			Influence& tInfluence=tCluster->influences[influenceIdx];

			fread(&tInfluence.weight,sizeof(float),1,iFile);
			fread(&tInfluence.ncontrolpointindex,sizeof(unsigned int),1,iFile);

			if(tInfluence.ncontrolpointindex)
				tInfluence.controlpointindex=new unsigned int[tInfluence.ncontrolpointindex];

			fread(tInfluence.controlpointindex,sizeof(unsigned int)*tInfluence.ncontrolpointindex,1,iFile);
		}
	}

	SerializerHelpers::globalSkinsClusterBoneIdToBind.push_back(globalSkinsToBindVector);

	int tMeshCodeCheck;
	fread(&tMeshCodeCheck,sizeof(int),1,iFile);

	if(tMeshCodeCheck!=Serializer::Mesh)
		DEBUG_BREAK();

	SerializerHelpers::Load((Mesh*)tSkin,iFile);
}


void SerializerHelpers::Load(Animation* tAnimation,FILE* iFile)
{
	fread(&tAnimation->start,sizeof(float),1,iFile);//1
	fread(&tAnimation->end,sizeof(float),1,iFile);//1
	fread(&tAnimation->index,sizeof(int),1,iFile);//1

	unsigned int tAnimClipsSize;

	fread(&tAnimClipsSize,sizeof(unsigned int),1,iFile);//1

	for(int iterAnimClip=0;iterAnimClip<tAnimClipsSize;iterAnimClip++)
	{
		AnimClip* tAnimClip=new AnimClip;

		tAnimation->clips.push_back(tAnimClip);

		fread(&tAnimClip->start,sizeof(float),1,iFile);//1
		fread(&tAnimClip->end,sizeof(float),1,iFile);//1

		unsigned int tKeyCurveSize;

		fread(&tKeyCurveSize,sizeof(unsigned int),1,iFile);//1

		for(int iterKeyCurve=0;iterKeyCurve<tKeyCurveSize;iterKeyCurve++)
		{
			KeyCurve* tKeyCurve=new KeyCurve;

			tAnimClip->curves.push_back(tKeyCurve);

			fread(&tKeyCurve->channel,sizeof(unsigned int),1,iFile);//1
			fread(&tKeyCurve->start,sizeof(float),1,iFile);//1
			fread(&tKeyCurve->end,sizeof(float),1,iFile);//1

			unsigned int tKeyframeSize;

			fread(&tKeyframeSize,sizeof(unsigned int),1,iFile);//1

			for(int iterKeyFrame=0;iterKeyFrame<tKeyframeSize;iterKeyFrame++)
			{
				Keyframe* tKeyframe=new Keyframe;

				tKeyCurve->frames.push_back(tKeyframe);

				fread(&tKeyframe->time,sizeof(float),1,iFile);//1
				fread(&tKeyframe->value,sizeof(float),1,iFile);//1
			}
		}
	}
}

void SerializerHelpers::Load(AnimationController* tAnimationController,FILE* iFile)
{
	unsigned int tAnimationIdsSize;
	fread(&tAnimationIdsSize,sizeof(unsigned int),1,iFile);

	if(tAnimationIdsSize)
	{
		unsigned int *tAnimationIdsArray=new unsigned int[tAnimationIdsSize];
		fread(tAnimationIdsArray,sizeof(unsigned int)*tAnimationIdsSize,1,iFile);

		globalAnimationControllersToBind.push_back(tAnimationController);
		std::list<unsigned int> tAnimationsIdList;

		for(int i=0;i<tAnimationIdsSize;i++)
			tAnimationsIdList.push_back(tAnimationIdsArray[i]);

		globalAnimationControllersAnimationsIndicesToBind.push_back(tAnimationsIdList);

		SAFEDELETEARRAY(tAnimationIdsArray);
	}

	fread(&tAnimationController->speed,sizeof(float),1,iFile);
	fread(&tAnimationController->cursor,sizeof(float),1,iFile);
	fread(&tAnimationController->play,sizeof(bool),1,iFile);
	fread(&tAnimationController->looped,sizeof(bool),1,iFile);
	fread(&tAnimationController->start,sizeof(float),1,iFile);
	fread(&tAnimationController->end,sizeof(float),1,iFile);
	fread(&tAnimationController->framesPerSecond,sizeof(unsigned int),1,iFile);
	fread(&tAnimationController->frameTime,sizeof(unsigned int),1,iFile);
}


void SerializerHelpers::Load(Line* tLine,FILE* iFile)
{
	unsigned int	tPointSize;
	vec3			tPoint;

	fread(&tPointSize,sizeof(unsigned int),1,iFile);

	for(int i=0;i<tPointSize;i++)
	{
		fread(&tPoint,sizeof(float)*3,1,iFile);
		tLine->points.push_back(tPoint);
	}
}

void SerializerHelpers::Load(Script* tScript,FILE* iFile)
{
	StringUtils::ReadWstring(iFile,tScript->file);

#ifndef EDITORBUILD
	LoadScriptModule(tScript);
#endif
}

void SerializerHelpers::BindSkinLinks(Entity* iEntity)
{
	if(!iEntity->parent && globalSkinsToBind.size())
	{
		std::list<Skin*>::iterator						skinIter=globalSkinsToBind.begin();
		std::list< std::list<unsigned int> >::iterator	indexListIter=globalSkinsClusterBoneIdToBind.begin();

		for(;skinIter!=globalSkinsToBind.end() && indexListIter!=globalSkinsClusterBoneIdToBind.end();skinIter++,indexListIter++)
		{
			Skin*							tSkin=*skinIter;
			std::list<unsigned int>&		tIndices=*indexListIter;

			std::list<unsigned int>::iterator indexIter=tIndices.begin();

			for(int clusterIdx=0;clusterIdx<tSkin->nclusters && indexIter!=tIndices.end();clusterIdx++,indexIter++)
			{
				tSkin->clusters[clusterIdx].bone=GetEntityById(iEntity,*indexIter);

				if(!tSkin->clusters[clusterIdx].bone)
					DEBUG_BREAK();
			}
		}

		globalSkinsToBind.clear();
		globalSkinsClusterBoneIdToBind.clear();
	}
}

void SerializerHelpers::BindAnimationLinks(Entity* iEntity)
{
	if(!iEntity->parent && globalAnimationControllersToBind.size())
	{
		std::list<AnimationController*>::iterator						animIter=globalAnimationControllersToBind.begin();
		std::list< std::list<unsigned int> >::iterator					indexListIter=globalAnimationControllersAnimationsIndicesToBind.begin();

		for(;animIter!=globalAnimationControllersToBind.end() && indexListIter!=globalAnimationControllersAnimationsIndicesToBind.end();animIter++,indexListIter++)
		{
			AnimationController*			tAnimController=*animIter;
			std::list<unsigned int>&		tIndices=*indexListIter;

			std::list<unsigned int>::iterator indexIter=tIndices.begin();

			for(;indexIter!=tIndices.end();indexIter++)
			{
				Entity* tEntityContainingAnimation=GetEntityById(iEntity,*indexIter);

				if(!tEntityContainingAnimation)
					DEBUG_BREAK();

				Animation* tAnim=tEntityContainingAnimation->findComponent<Animation>();

				if(!tAnim)
					DEBUG_BREAK();

				tAnimController->AddAnimation(tAnim);
			}
		}

		globalAnimationControllersToBind.clear();
		globalAnimationControllersAnimationsIndicesToBind.clear();
	}
}

Entity* SerializerHelpers::loadSceneEntityRecursively(Entity* iEntityParent,FILE* iFile)
{
	unsigned int componentsSize;
	unsigned int childsSize;
	int componentCode;

	Entity* tEntity=new Entity;

	tEntity->SetParent(iEntityParent);

	fread(&childsSize,sizeof(unsigned int),1,iFile);
	fread(&tEntity->id,sizeof(unsigned int),1,iFile);

	fread(tEntity->local,sizeof(float),16,iFile);
	fread(tEntity->world,sizeof(float),16,iFile);

	StringUtils::ReadWstring(iFile,tEntity->name);

	printf("Entity: %s, id: %d\n",StringUtils::ToChar(tEntity->name).c_str(),tEntity->id);

	fread(tEntity->bbox.a,sizeof(float),3,iFile);
	fread(tEntity->bbox.b,sizeof(float),3,iFile);

	fread(&componentsSize,sizeof(unsigned int),1,iFile);

	for(int tComponentIndex=0;tComponentIndex<componentsSize;tComponentIndex++)
	{
		fread(&componentCode,sizeof(int),1,iFile);

		switch(componentCode)
		{
			case Serializer::Script: SerializerHelpers::Load(tEntity->CreateComponent<Script>(),iFile); break;
			case Serializer::Line: SerializerHelpers::Load(tEntity->CreateComponent<Line>(),iFile); break;
			case Serializer::Animation: SerializerHelpers::Load(tEntity->CreateComponent<Animation>(),iFile); break;
			case Serializer::AnimationController: SerializerHelpers::Load(tEntity->CreateComponent<AnimationController>(),iFile); break; 
			case Serializer::Mesh: SerializerHelpers::Load(tEntity->CreateComponent<Mesh>(),iFile); break;
			case Serializer::Skin: SerializerHelpers::Load(tEntity->CreateComponent<Skin>(),iFile); break;
			/*default:
				DEBUG_BREAK();*/
		}
	}

	if(childsSize)
	{
		for(int i=0;i<childsSize;i++)
			loadSceneEntityRecursively(tEntity,iFile);
	}

	BindSkinLinks(tEntity);
	BindAnimationLinks(tEntity);

	return tEntity;
}


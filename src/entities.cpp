#include "entities.h"

#include <algorithm>

#include <cstdlib>

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////ResourceNode/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

ResourceNode::ResourceNode():
	parent(0),
	level(0),
	isDir(0),
	selectedLeft(0),
	selectedRight(0)
{}

ResourceNode::~ResourceNode()
{
	this->fileName="";
	this->selectedLeft=false;
	this->selectedRight=false;
	this->level=0;
	this->isDir=false;
}

ResourceNodeDir::ResourceNodeDir():
	expanded(0),
	SAFESTLIMPL(std::list<ResourceNodeDir*>,dirs),
	SAFESTLIMPL(std::list<ResourceNode*>,files)
{}

ResourceNodeDir::~ResourceNodeDir()
{
    for(std::list<ResourceNode*>::iterator nCh=this->files.begin();nCh!=this->files.end();nCh++)
		SAFEDELETE(*nCh);

    for(std::list<ResourceNodeDir*>::iterator nCh=this->dirs.begin();nCh!=this->dirs.end();nCh++)
    {
        ResourceNodeDir* tResNodeDir=*nCh;

        tResNodeDir->~ResourceNodeDir();
    }




	if(this->parent)
	{
		ResourceNodeDir* tParent=(ResourceNodeDir*)this->parent;

		if(this->isDir)
			tParent->dirs.erase(std::find(tParent->dirs.begin(),tParent->dirs.end(),this));
		else
			tParent->files.erase(std::find(tParent->files.begin(),tParent->files.end(),this));
	}

	SAFESTLDEST(dirs);
	SAFESTLDEST(files);
}


Scene* gLoadScene()
{

}

#ifdef EDITORBUILD

ResourceNodeDir rootProjectDirectory;

String gFindResource(String& iCurrentDirectory,String& iProjectDir,ResourceNodeDir* iResDir,String& iResourceName)
{
	//store current dir

	if(iResDir->parent)
	{
		iCurrentDirectory+=iResDir->fileName.Buffer();
		iCurrentDirectory+="\\";
	}

	//if node contains files, process them, later process other dir nodes

	for(std::list<ResourceNode*>::iterator tResFile=iResDir->files.begin();tResFile!=iResDir->files.end();tResFile++)
	{
		String	tVirtualFileName(iCurrentDirectory + (*tResFile)->fileName);

		if(tVirtualFileName==iResourceName)
		{
			return iProjectDir + iCurrentDirectory + (*tResFile)->fileName;
		}
	}

	for(std::list<ResourceNodeDir*>::iterator tResNodeDir=iResDir->dirs.begin();tResNodeDir!=iResDir->dirs.end();tResNodeDir++)
	{
		String t=gFindResource(iCurrentDirectory,iProjectDir,*tResNodeDir,iResourceName);

		if(t!="")
			return t;
	}

	return "";
}

void* Resource::Load(String iResourceName)
{
    String tRootTrailingSlashes("\\");

	FilePath tResource=gFindResource(tRootTrailingSlashes,rootProjectDirectory.fileName,&rootProjectDirectory,iResourceName);

	if(tResource.File().Count())
    {
        String tFileExtension=tResource.Extension();

        if(tFileExtension=="engineScene")
        {

        }
    }

    return 0;
}

#else

FILE*        resourceData=0;
unsigned int resourceDataSize=0;
unsigned int resourceTableSize=0;
unsigned int resourceTableStart=0;
unsigned int resourceTableEnd=0;
unsigned int resourceDataStart=0;
unsigned int resourceDataEnd=0;

void* Resource::Load(String iResourceName)
{
    fseek(resourceData,resourceTableStart,SEEK_SET);

    char            tFilePath[500];
    unsigned int    tFileStart=0;
    unsigned int    tFileSize=0;

    bool            tFileFound=false;

    while(ftell(resourceData)<resourceTableEnd)
    {
        fscanf(resourceData,"%s",tFilePath);
        fscanf(resourceData,"%u",&tFileStart);
        fscanf(resourceData,"%u",&tFileSize);

        printf("found file %s\n",tFilePath);
        printf("found file start %u\n",tFileStart);
        printf("found file size %u\n",tFileSize);

        if(iResourceName==String(tFilePath))
        {
            tFileFound=true;
            break;
        }
    }

    if(tFileFound)
    {
        fseek(resourceData,resourceDataStart+tFileStart,SEEK_SET);

        char* rBuffer=new char[tFileSize];
        fread(rBuffer,tFileSize,1,resourceData);

        return rBuffer;
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
	unlit(0),
	unlit_color(0),
	unlit_texture(0),
	font(0),
	shaded_texture(0),
	picking(false),
	SAFESTLIMPL(std::vector<Shader*>,shaders)
{}

Renderer3DBase::~Renderer3DBase()
{
	SAFESTLDEST(shaders);
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
//////////////////////File/////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

File::File(String iString):path(iString),
	data(0){}

bool File::IsOpen()
{
	return this->data ? true : false;
}

//////////////////////File/////////////////////

bool File::Open(const char* mode)
{
	if(!this->path.Count() || this->IsOpen())
		return false;

	this->data=fopen(path,mode);

	if(!data)
		return false;

	int tell=ftell(this->data);

	if(tell!=0)
		DEBUG_BREAK();

	return true;
}

void File::Close()
{
	this->data ? fclose(this->data),this->data=0 : 0;
}

bool File::Exist()
{
	if(!this->path.Count())
		return false;

	return File::Exist(this->path);
}

int File::Size()
{
	int result=0;

	bool tWasOpen=this->IsOpen();

	bool tIsOpen=!tWasOpen ? this->Open() : true;

	if(tIsOpen)
	{
		int curPos=ftell(this->data);
		fseek(this->data,0,SEEK_END);
		result=ftell(this->data);
		fseek(this->data,0,curPos);
	}

	if(!tWasOpen)
		this->Close();

	return result;
}

bool File::Create()
{
	return File::Create(this->path);
}

bool File::Delete()
{
	if(this->path.Count())
		return !::remove(this->path);
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
		int oldPos=ftell(this->data);

		fseek(this->data,0,SEEK_END);

		int ___size=ftell(this->data);

		int ___i=0;
		while(___i < ___size)
		{
			fseek(this->data,___i,SEEK_SET);

			char c=fgetc(this->data);

			if(iChar==c)
				occurrences++;

			___i++;
		}

		fseek(this->data,oldPos,SEEK_SET);
	}

	if(!wasOpen)
		this->Close();

	return occurrences;
}

String File::All()
{
	String tS;

	bool wasOpen=this->IsOpen();

	if(!wasOpen)
		this->Open();

	if(this->IsOpen())
	{
		int tSize=this->Size();

		if(tSize>0)
		{
			char* tT=new char[tSize+1];
			fread(tT,tSize,1,this->data);
			tT[tSize]='\0';
			tS=tT;
			SAFEDELETEARRAY(tT);
		}
	}


	if(!wasOpen)
		this->Close();

	if(tS.Count()==0)
		tS="";

	return tS;
}

int File::Read(void* outData,int iSize)
{
	if(this->data)
	{
		return fread(outData,iSize,1,this->data);
	}

	return 0;
}

bool File::ReadW(wchar_t* outData,int iSize)
{
	if(this->data)
	{
		wchar_t* outValue=fgetws(outData,iSize*2,this->data);
		outData[iSize]=L'\0';
		return outValue ? true : false;
	}

	return 0;
}

int File::Write(void* iData,int iSize,int iNum)
{
	if(this->data)
		return fwrite(iData,iSize,iNum,this->data);

	return 0;
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

Shader::Shader(){}

Shader* Shader::Find(const char* name,bool exact)
{
	for(size_t i=0;i<GetPool().size();i++)
	{
		Shader* element=GetPool()[i];

		const char* programName=element->GetName();

		if(element && programName)
			if(exact ? 0==strcmp(programName,name) :  0!=strstr(programName,name))
				return GetPool()[i];
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
	SAFESTLIMPL(std::vector<Keyframe*>,frames),
	channel(INVALID_CHANNEL),
	start(-1),
	end(-1)
{}

KeyCurve::~KeyCurve()
{
	SAFESTLDEST(frames);
}


AnimClip::AnimClip():
	SAFESTLIMPL(std::vector<KeyCurve*>,curves),
	start(-1),
	end(-1)
{}

AnimClip::~AnimClip()
{
	SAFESTLDEST(curves);
}

Animation::Animation():
	entity(0),
	index(0),
	start(-1),
	end(-1),
	SAFESTLIMPL(std::vector<AnimClip*>,clips)
{}

Animation::~Animation()
{
	SAFESTLDEST(clips);
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
            printf("copychannel: INVALID_CHANNEL selected\n");
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
	SAFESTLIMPL(std::vector<Animation*>,animations),
	speed(1),
	cursor(0),
	play(false),
	looped(true),
	start(0),
	end(0),
	lastFrameTime(0),
	resolutionFps(60)
{}

AnimationController::~AnimationController()
{
	SAFESTLDEST(animations);
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
	SAFESTLIMPL(std::vector<Texture*>,textures),
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

Material::~Material()
{
	SAFESTLDEST(textures);
}

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
	isCCW(true),
	SAFESTLIMPL(std::vector<Material*>,materials)
{}

Mesh::~Mesh()
{
	SAFESTLDEST(materials);
}

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




TextureLayered::TextureLayered():
	SAFESTLIMPL(std::vector<Texture*>,textures)
{}

TextureLayered::~TextureLayered()
{
	SAFESTLDEST(textures);
}

TextureProcedural::TextureProcedural(){}



///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////




EntityScript::EntityScript():entity(0){}

Entity::Entity():
	parent(0),
	SAFESTLIMPL(std::vector<EntityComponent*>,components),
	SAFESTLIMPL(std::list<Entity*>,childs)
{}

Entity::~Entity()
{
	this->SetParent(0);

	for(std::vector<EntityComponent*>::iterator tCom=this->components.begin();tCom!=this->components.end();tCom++)
		SAFEDELETE(*tCom);

	for(std::list<Entity*>::iterator tEn=this->childs.begin();tEn!=this->childs.end();tEn++)
		SAFEDELETE(*tEn);

	SAFESTLDEST(components);
	SAFESTLDEST(childs);
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

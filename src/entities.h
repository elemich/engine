#ifndef __ENTITY_HEADER__
#define __ENTITY_HEADER__

#include "primitives.h"

struct DLLBUILD Entity;
struct DLLBUILD Cluster;
struct DLLBUILD Bone;
struct DLLBUILD EntityScript;

struct DLLBUILD Renderer3DBase;
struct DLLBUILD Shader;


#define PROCESS_ENTITIES_RECURSIVELY 1


struct DLLBUILD MatrixStack
{
	enum matrixmode
	{
		PROJECTION=0,
		MODEL,
		VIEW,
		MATRIXMODE_MAX
	};

	static void Reset();


	static void Push();
	static void Pop();
	static void Identity();
	static float* Get();
	static void Load(float* m);
	static void Multiply(float* m);

	static void Pop(MatrixStack::matrixmode);
	static void Push(MatrixStack::matrixmode);
	static void Push(MatrixStack::matrixmode,float*);
	static void Identity(MatrixStack::matrixmode);
	static float* Get(MatrixStack::matrixmode,int lev=-1);
	static void Load(MatrixStack::matrixmode,float*);
	static void Multiply(MatrixStack::matrixmode,float*);

	static void Rotate(float a,float x,float y,float z);
	static void Translate(float x,float y,float z);
	static void Scale(float x,float y,float z);

	static mat4 GetProjectionMatrix();
	static mat4 GetModelMatrix();
	static mat4 GetViewMatrix();

	static void SetProjectionMatrix(float*);
	static void SetModelMatrix(float*);
	static void SetViewMatrix(float*);

	static  MatrixStack::matrixmode GetMode();
	static  void SetMode(MatrixStack::matrixmode m);

	static mat4 model;
	static mat4 projection;
	static mat4 view;
};



struct DLLBUILD Shader : TPoolVector<Shader>
{
	static Shader* Find(const char*,bool exact=true);

	virtual int GetProgram()=0;
	virtual void SetProgram(int)=0;

	virtual void SetName(const char*)=0;
	virtual const char* GetName()=0;

	virtual int GetUniform(int slot,char* var)=0;
	virtual int GetAttrib(int slot,char* var)=0;

	virtual void Use()=0;

	virtual const char* GetPixelShader()=0;
	virtual const char* GetFragmentShader()=0;

	virtual int GetAttribute(const char*)=0;
	virtual int GetUniform(const char*)=0;

	virtual int init()=0;

	virtual int GetPositionSlot()=0;
	virtual int GetColorSlot()=0;
	virtual int GetProjectionSlot()=0;
	virtual int GetModelviewSlot()=0;
	virtual int GetTexcoordSlot()=0;
	virtual int GetTextureSlot()=0;
	virtual int GetLightposSlot()=0;
	virtual int GetMouseSlot()=0;
	virtual int GetLightdiffSlot()=0;
	virtual int GetLightambSlot()=0;
	virtual int GetNormalSlot()=0;
	virtual int GetHoveringSlot()=0;

	virtual void SetSelectionColor(bool pick,void* ptr,vec2 mposNrm)=0;

	virtual bool SetMatrix4f(int slot,float* mtx)=0;

	virtual unsigned int& GetBufferObject()=0;

	virtual void SetProjectionMatrix(float*)=0;
	virtual void SetModelviewMatrix(float*)=0;
	virtual void SetMatrices(float* view,float* mdl)=0;

	Shader();
};



enum EEntity
{
	ENTITY_GENERIC=0,
	ENTITY_IMAGE,
	ENTITY_MESH,
	ENTITY_SKIN,
	ENTITY_BONE,
	ENTITY_LIGHT,
	ENTITY_CAMERA,
	ENTITY_GUI,
	ENTITY_MAX
};

enum EInterpolation
{
	INTERP_CONSTANT=0,
	INTERP_ADDITIVE,
	INTERP_MAX
};

enum EChannel
{
	SCALEX,
	SCALEY,
	SCALEZ,
	TRANSLATEX,
	TRANSLATEY,
	TRANSLATEZ,
	ROTATEX,
	ROTATEY,
	ROTATEZ,
	INVALID_CHANNEL
};

enum EAnimationState
{
	ANIMSTATE_STOP,
	ANIMSTATE_PLAY,
	ANIMSTATE_PAUSE,
	ANIMSTATE_MAX
};

enum EResourceId
{
	RESOURCE_TEXTURE,
	RESOURCE_MATERIAL
};

enum ELight//as fbx enum
{
	LIGHT_POINT,
	LIGHT_DIRECTIONAL,
	LIGHT_SPOT,
	LIGHT_AREA,
	LIGHT_VOLUME,
	LIGHT_MAX
};

enum EDecay//as fbx
{
	DECAY_NONE,
	DECAY_LINEAR,
	DECAY_QUADRATIC,
	DECAY_CUBIC,
	DECAY_MAX
};

enum EMaterial
{
	MATERIAL_DIFFUSE,
	MATERIAL_AMBIENT,
	MATERIAL_SPECULAR,
	MATERIAL_SHININESS,
	MATERIAL_BUMP,
	MATERIAL_NORMALMAP,
	MATERIAL_TRANSPARENT,
	MATERIAL_REFLECTION,
	MATERIAL_EMISSIVE,
	MATERIAL_DISPLACEMENT,
	MATERIAL_MAX
};

enum ETexture
{
	TEXTURE_GENERIC,
	TEXTURE_FILE,
	TEXTURE_LAYERED,
	TEXTURE_PROCEDURAL,
	TEXTURE_MAX
};

static const char *EEntityNames[ENTITY_MAX]=
{
	"ENTITY_GENERIC",
	"ENTITY_IMAGE",
	"ENTITY_MESH",
	"ENTITY_SKIN",
	"ENTITY_BONE",
	"ENTITY_LIGHT",
	"ENTITY_CAMERA",
	"ENTITY_GUI"
};


struct DLLBUILD ResourceNode
{
	ResourceNode* parent;

	FilePath fileName;

	bool selectedLeft;
	bool selectedRight;
	int level;
	bool isDir;

	ResourceNode();
	~ResourceNode();
};

struct DLLBUILD ResourceNodeDir : ResourceNode
{
	bool expanded;

	SAFESTLDECL(std::list<ResourceNodeDir*>,dirs);
	SAFESTLDECL(std::list<ResourceNode*>,files);

	ResourceNodeDir();
	~ResourceNodeDir();
};

struct DLLBUILD Resource
{
	static ResourceNodeDir* rootProjectDirectory;

	static String Find(String iResourceName);

	template<typename T> static T* Load(String iResourceName)
	{
		String tFileName=Find(iResourceName);

		if(tFileName!="")
		{

		}

		return 0;
	}
};



struct DLLBUILD File
{
	FilePath path;
	void* data;

	File(String iString=0);

	bool Open(const char* mode="r");
	void Close();
	bool IsOpen();
	bool Exist();
	int Size();
	bool Create();
	int CountOccurrences(char);
	int Read(void* outData,int iSize);
	bool ReadW(wchar_t* outData,int iSize);
	int Write(void* iData,int iSize,int iNum);
	bool Delete();
	String All();

	static bool Create(const char* iFilename);
	static bool Exist(const char* iFilename);
	static bool Delete(const char* iFilename);
	static int Size(const char* iFilename);
};

struct DLLBUILD Texture
{
	Texture();

	virtual int load(char*)=0;
	virtual int loadBMP(char*)=0;
	virtual int loadPNG(char*)=0;
	virtual int loadJPG(char*)=0;
	virtual int loadTGA(char*)=0;
	virtual void* GetBuffer()=0;
	virtual int GetSize()=0;
	virtual int GetWidth()=0;
	virtual int GetHeight()=0;
	virtual int GetBpp()=0;
};


struct DLLBUILD Material
{
	SAFESTLDECL(std::vector<Texture*>,textures);

	EMaterial m_type;

	vec3 emissive;
	float femissive;
	vec3 ambient;
	float fambient;
	vec3 diffuse;
	float fdiffuse;
	vec3 normalmap;
	vec3 bump;
	float fbump;
	vec3 transparent;
	float ftransparent;
	vec3 displacement;
	float fdisplacement;

	vec3 specular;
	float fspecular;
	float fshininess;
	vec3 reflection;
	float freflection;

	Material();
	~Material();

	void update();
};


struct DLLBUILD Influence
{
	int		*cpIdx;
	int		nCpIdx;
	float	weight;

	Influence();
};

struct DLLBUILD Bone;
struct DLLBUILD Cluster
{
	Entity* bone;

	Influence	*influences;
	int			ninfluences;

	mat4		offset;

	Cluster();
};

struct DLLBUILD Keyframe
{
	Keyframe();
	float		 time;
	float value;
};

struct DLLBUILD KeyCurve
{
	KeyCurve();
	~KeyCurve();


	SAFESTLDECL(std::vector<Keyframe*>,frames);

	EChannel		channel;
	float			start;
	float			end;
};


struct DLLBUILD AnimClip
{
	AnimClip();
	~AnimClip();

	SAFESTLDECL(std::vector<KeyCurve*>,curves);

	float start;
	float end;
};

struct DLLBUILD EntityBase
{
	virtual void update(){}
	virtual void draw(Renderer3DBase*){}
};

struct DLLBUILD EntityComponent : EntityBase
{
	Entity* entity;

	EntityComponent():entity(0){}

	template<class C> C* is(){return dynamic_cast<C*>(this);}

	virtual void update(){}
	virtual void draw(Renderer3DBase*){}
};

struct DLLBUILD Entity : EntityBase
{
	Entity*					parent;

	SAFESTLDECL(std::vector<EntityComponent*>,components);
	SAFESTLDECL(std::list<Entity*>,childs);

	mat4					local;
	mat4					world;

	String					name;

	AABB					bbox;

	Entity();
	~Entity();

	virtual void SetParent(Entity* iParent);
	virtual void update();
	virtual void draw(Renderer3DBase*);
	
	template<class C> C* CreateComponent()
	{
		C* newComp=new C;
		newComp->entity=this;
		this->components.push_back(newComp);
		return newComp;
	}

	template<class C> C* findComponent()
	{
		for(int i=0;i<(int)this->components.size();i++)
		{
			if(dynamic_cast<C*>(this->components[i]))
				return (C*)this->components[i];
		}

		return 0;
	}

	template<class C> std::vector<C*> findComponents()
	{
		std::vector<C*> vecC;

		for(int i=0;i<(int)this->components.size();i++)
		{
			if(dynamic_cast<C*>(this->components[i]))
				vecC.push_back((C*)this->components[i]);
		}

		return vecC;
	}
};

struct DLLBUILD Root : EntityComponent
{
};

struct DLLBUILD Skeleton : EntityComponent
{
};

struct DLLBUILD Animation : EntityComponent
{
	Animation();
	~Animation();

	Entity* entity;

	SAFESTLDECL(std::vector<AnimClip*>,clips);

	float	start;
	float	end;

	int index;
};

struct DLLBUILD Gizmo : EntityComponent
{
};

struct DLLBUILD AnimationController : EntityComponent
{
	SAFESTLDECL(std::vector<Animation*>,animations);

	float speed;
	float cursor;
	bool play;
	bool looped;

	float start;
	float end;

	int resolutionFps;

	int lastFrameTime;

	AnimationController();
	~AnimationController();

	void add(Animation* anim);

	void Stop();
	void Play();

	virtual void update();

	void SetFrame(float iFrame);
};

struct DLLBUILD Bone : EntityComponent
{
	Bone();

	Bone* root;

	vec3    color;

};

struct DLLBUILD Light : EntityComponent
{
	Light();

	ELight	light_type;
	EDecay	light_decaytype;
	bool    light_cast;
	bool    light_volumetric;
	bool    light_groundprojection;
	bool    light_nearattenuation;
	bool    light_farattenuation;
	bool    light_shadows;
	vec3	light_color;
	vec3	light_shadowcolor;
	float	light_intensity;
	float	light_innerangle;
	float	light_outerangle;
	float   light_fog;
	float	light_decaystart;
	float	light_nearstart;
	float	light_nearend;
	float	light_farstart;
	float	light_farend;
};

struct DLLBUILD Mesh : EntityComponent
{
	Mesh();
	~Mesh();

	int save(char*);
	int load(char*);

	float (*controlpoints)[3];
	int	  ncontrolpoints;

	unsigned int *vertexindices;
	int	  nvertexindices;

	float (*texcoord)[2];
	int	  ntexcoord;

	float (*normals)[3];
	int	  nnormals;

	float (*colors)[3];
	int	  ncolors;

	int	  npolygons;

	bool  isCCW;

	SAFESTLDECL(std::vector<Material*>,materials);

	void draw(Renderer3DBase*);
};

struct DLLBUILD Script : EntityComponent , TPoolVector<Script>
{
	File script;
	EntityScript* runtime;
	FilePath module;

	Script();

	void update();
};

struct DLLBUILD EntityScript
{
	Entity* entity;

	EntityScript();

	virtual void init(){};
	virtual void deinit(){};
	virtual void update(){};
};

struct DLLBUILD Skin : Mesh
{
	Skin();

	virtual void		update();
	
	
	Texture	    *textures;
	int			ntextures;

	Cluster		*clusters;
	int			nclusters;


	float*	    vertexcache;

	void draw(Renderer3DBase*);
};

struct DLLBUILD Camera : EntityComponent
{
	float fov;
	float ratio;
	float Near;
	float Far;

	mat4 matrix;

	vec3 target;
};

struct DLLBUILD TextureFile : Texture
{
	TextureFile();
	~TextureFile();

	int load(char*);
	int loadBMP(char*);//0 error
	int loadPNG(char*);
	int loadJPG(char*);//0 error
	int loadTGA(char*);//0 error
	void* GetBuffer();
	int GetSize();
	int GetWidth();
	int GetHeight();
	int GetBpp();

	String filename;
	void* data;
};



struct DLLBUILD TextureLayered
{
	SAFESTLDECL(std::vector<Texture*>,textures);

	TextureLayered();
	~TextureLayered();
};

struct DLLBUILD TextureProcedural : Texture 
{
	TextureProcedural();

	int load(char*){return 0;}
	int loadBMP(char*){return 0;}
	int loadPNG(char*){return 0;}
	int loadJPG(char*){return 0;}
	int loadTGA(char*){return 0;}
	void* GetBuffer(){return 0;}
	int GetSize(){return 0;}
	int GetWidth(){return 0;}
	int GetHeight(){return 0;}
	int GetBpp(){return 0;}
};

struct DLLBUILD Renderer3DBase
{
	Renderer3DBase();
	~Renderer3DBase();

	virtual void draw(vec3,float psize=1.0f,vec3 color=vec3(1,1,1))=0;
	virtual void draw(vec2)=0;
	virtual void draw(vec3,vec3,vec3 color=vec3(1,1,1))=0;
	virtual void draw(vec4)=0;
	virtual void draw(AABB,vec3 color=vec3(1,1,1))=0;
	virtual void draw(mat4 mtx,float size,vec3 color=vec3(1,1,1))=0;
	//virtual void draw(Font*,char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4)=0;
	virtual void draw(char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4)=0;
	virtual void draw(Bone*)=0;
	virtual void draw(Mesh*)=0;
	virtual void draw(Skin*)=0;
	virtual void draw(Texture*)=0;
	virtual void draw(Light*)=0;
	virtual void drawUnlitTextured(Mesh*)=0;
	virtual void draw(Mesh*,std::vector<unsigned int>& textureIndices,int texture_slot,int texcoord_slot)=0;
	virtual void draw(Camera*)=0;
	virtual void draw(Gizmo*)=0;
	virtual void draw(Script*)=0;

	bool picking;

	Shader* unlit;
	Shader* unlit_color;
	Shader* unlit_texture;
	Shader* font;
	Shader* shaded_texture;

	SAFESTLDECL(std::vector<Shader*>,shaders);
};

struct DLLBUILD Scene
{
	Entity* rootEntity;
};

#endif //__ENTITY_HEADER__
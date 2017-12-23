#ifndef __ENTITY_HEADER__
#define __ENTITY_HEADER__

#include "primitives.h"

struct Renderer3DInterfaceBase;
struct ShaderInterface;
struct EntityScript;


#define PROCESS_ENTITIES_RECURSIVELY 1



struct ShaderInterface : TPoolVector<ShaderInterface>
{
	static ShaderInterface* Find(const char*,bool exact=true);

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

	ShaderInterface();
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


struct Resource : THierarchyList<Entity>
{
	Resource* GetResource(){return this;}
	Resource* GetTexture(){return 0;}
	Resource* GetMaterial(){return 0;}
};

struct File : Resource
{
	String fullPath;
};

struct Dir : Resource
{
	String fullPath;
};

struct Texture
{
	Texture();

	Texture* GetTexture(){return this;}
	Texture* GetTextureFile(){return 0;}
	Texture* GetTextureLayered(){return 0;}

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


struct Material : Resource
{
	Material();

	Material* GetMaterial(){return this;}

	void update(){}

	std::vector<Texture*> textures;



	EMaterial m_type;

	float emissive[3];
	float femissive;
	float ambient[3];
	float fambient;
	float diffuse[3];
	float fdiffuse;
	float normalmap[3];
	float bump[3];
	float fbump;
	float transparent[3];
	float ftransparent;
	float displacement[3];
	float fdisplacement;

	float specular[3];
	float fspecular;
	float fshininess;
	float reflection[3];
	float freflection;
};


struct Influence
{
	int		*cpIdx;
	int		nCpIdx;
	float	weight;

	Influence();
};

struct Bone;
struct Cluster
{
	Entity* bone;

	Influence	*influences;
	int			ninfluences;

	mat4		offset;

	Cluster();
};


struct Keyframe
{
	Keyframe();
	float		 time;
	float value;
};





struct KeyCurve
{
	

	KeyCurve();

	std::vector<Keyframe*>	frames;
	EChannel		channel;
	float			start;
	float			end;
};


struct AnimClip
{
	AnimClip();

	std::vector<KeyCurve*> curves;

	float start;
	float end;
};

struct EntityComponent
{
	Entity* entity;

	EntityComponent():entity(0){}

	template<class C> C* is(){return dynamic_cast<C*>(this);}

	virtual void update(){}
	virtual void draw(Renderer3DInterfaceBase*){}

	virtual EntityComponent* GetComponent()=0;
};



struct Entity : THierarchyList<Entity>
{
	mat4					local;
	mat4					world;

	String					name;

	AABB					bbox;

	Entity();
	~Entity();

	std::vector<EntityComponent*> components;
	
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
				vecC.push_back(this->components[i]);
		}

		return vecC;
	}
};

struct Root : EntityComponent
{
	virtual Root* GetComponent(){return this;}
};

struct Skeleton : EntityComponent
{
	virtual Skeleton* GetComponent(){return this;}
};

struct Animation : EntityComponent
{
	virtual Animation* GetComponent(){return this;}

	Animation();

	Entity* entity;

	std::vector<AnimClip*> clips;

	float	start;
	float	end;

	int clipIdx;
};

struct Gizmo : EntityComponent
{
	virtual Gizmo* GetComponent(){return this;}
};

struct AnimationController : EntityComponent
{
	virtual AnimationController* GetComponent(){return this;}

	std::vector<Animation*> animations;

	float speed;
	float cursor;
	bool play;
	bool looped;

	float start;
	float end;

	int resolutionFps;

	int lastFrameTime;

	AnimationController():speed(1),cursor(0),play(false),looped(true),start(0),end(0),lastFrameTime(0),resolutionFps(60){}

	void add(Animation* anim);

	void update();
};

struct Bone : EntityComponent
{
	virtual Bone* GetComponent(){return this;}

	Bone();

	Bone* root;

	vec3    color;

};

struct Light : EntityComponent
{
	virtual Light* GetComponent(){return this;}

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

struct Mesh : EntityComponent
{
	virtual Mesh* GetComponent(){return this;}

	Mesh();

	int save(char*);
	int load(char*);

	float** GetControlPoints();
	int GetNumControlPoints();

	float** GetTriangles();
	int GetNumTriangles();

	float** GetUV();
	int GetNumUV();
	int** GetUVIndices();

	float** GetNormals();
	int GetNumNormals();

	std::vector<Material*>& GetMaterials();

	float (*mesh_controlpoints)[3];
	int	  mesh_ncontrolpoints;

	unsigned int *mesh_vertexindices;
	int	  mesh_nvertexindices;

	float (*mesh_texcoord)[2];
	int	  mesh_ntexcoord;

	float (*mesh_normals)[3];
	int	  mesh_nnormals;

	float (*mesh_colors)[3];
	int	  mesh_ncolors;

	int	  mesh_npolygons;

	bool  mesh_isCCW;

	std::vector<Material*> mesh_materials;

	void draw(Renderer3DInterfaceBase*);
};

struct Script : EntityComponent
{
	virtual Script* GetComponent(){return this;}

	FilePath name;
	EntityScript* runtime;
	String data;

	void* handle;

	Script();
};

struct EntityScript
{
	virtual EntityScript* GetComponent(){return this;}

	Entity* entity;

	EntityScript();

	virtual void init(){};
	virtual void deinit(){};
	virtual void update(){};
};

struct Skin : Mesh
{
	virtual Skin* GetComponent(){return this;}

	Skin();

	void		update();
	
	
	Texture	    *skin_textures;
	int			skin_ntextures;

	Cluster		*skin_clusters;
	int			skin_nclusters;


	float*	    skin_vertexcache;

	void draw(Renderer3DInterfaceBase*);
};

struct Camera : EntityComponent
{
	virtual Camera* GetComponent(){return this;}

	float fov;
	float ratio;
	float Near;
	float Far;

	mat4 matrix;

	vec3 target;
};

struct TextureFile : Texture
{
	TextureFile();
	~TextureFile();

	TextureFile* GetTextureFile(){return this;}

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

	void* __data;

};



struct TextureLayered
{
	std::vector<Texture*> textures;

	TextureLayered();

	TextureLayered* GetTextureLayered(){return this;}
};

struct TextureProcedural : Texture 
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

struct Renderer3DInterfaceBase
{
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
};

#endif //__ENTITY_HEADER__
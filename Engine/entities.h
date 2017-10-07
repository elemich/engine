#ifndef __ENTITY_HEADER__
#define __ENTITY_HEADER__

#include "interfaces.h"

#define PROCESS_ENTITIES_RECURSIVELY 1

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


struct Resource
{
	static std::vector<Entity*> resourcePool;

	Resource* GetResource(){return this;}
	Resource* GetTexture(){return 0;}
	Resource* GetMaterial(){return 0;}
};

struct Texture
{
	Texture();

	Texture* GetTexture(){return this;}
	Texture* GetTextureFile(){return 0;}
	Texture* GetTextureLayered(){return 0;}

	virtual void draw(RendererInterface*)=0;

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
	void draw(RendererInterface*){}

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
	int		*influence_controlpointindex;
	int		influence_ncontrolpointindex;
	float	influence_weight;

	Influence();
};

struct Bone;
struct Cluster
{
	Entity* cluster_bone;

	Influence	*cluster_influences;
	int			cluster_ninfluences;

	mat4		cluster_offset;

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

	std::vector<Keyframe*>	keycurve_keyframes;
	EChannel		keycurve_channel;
	float			keycurve_start;
	float			keycurve_end;
};


struct AnimClip
{
	AnimClip();

	std::vector<KeyCurve*> curvegroup_keycurves;

	float curvegroup_start;
	float curvegroup_end;
};

struct EntityComponent
{
	Entity* entity;

	EntityComponent():entity(0){}

	template<class C> C* is(){return dynamic_cast<C*>(this);}

	virtual void update(){}
	virtual void draw(RendererInterface* ri){}

};

struct Entity 
{
	static std::list<Entity*> pool;
	static Entity* Find(const char*,bool exact=true);

	virtual void beginDraw();
	virtual void draw(RendererInterface*);
	virtual void endDraw();
	virtual void update();

	Entity*					entity_parent;
	std::list<Entity*>		entity_childs;		

	mat4					entity_transform;
	mat4					entity_world;

	String					entity_name;

	AABB					entity_bbox;

	int						nDrawed;
	int						nUpdated;
	int						nAnimated;

	bool operator==(const Entity& e){return this==&e;}

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
};

struct Animation : EntityComponent
{
	Animation();

	Entity* entity;

	std::vector<AnimClip*> clips;

	int		nprocessed;

	float	start;
	float	end;

	int clipIdx;
};


struct AnimationController : EntityComponent
{
	std::vector<Animation*> animations;

	float speed;
	float cursor;
	bool play;
	bool looped;

	float start;
	float end;

	int lastTime;

	AnimationController():speed(1),cursor(0),play(false),looped(true),start(0),end(0),lastTime(0){}

	void add(Animation* anim);

	void update();
	void draw(RendererInterface*){}
};

struct Bone : EntityComponent
{
	Bone();

	Bone* bone_root;

	vec3    bone_color;

	void draw(RendererInterface*);
	void update();
};

struct Light : EntityComponent
{
	Light();

	ELight	LightType();
	EDecay	DecayType();

	bool    Cast();
	bool    Volumetric();
	bool    GroundProjection();
	bool    NearAttenuation();
	bool    FarAttenuation();
	bool    Shadows();
	float*	Color();
	float*	ShadowColor();
	float	Intensity();
	float	InnerAngle();
	float	OuterAngle();
	float   Fog();
	float	DecayStart();
	float	NearStart();
	float	NearEnd();
	float	FarStart();
	float	FarEnd();

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

	void update();
	void draw(RendererInterface*);
};

struct Mesh : EntityComponent
{
	Mesh();

	virtual void draw(RendererInterface*);
	virtual void update();

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
};

struct Skin : Mesh
{
	Skin();

	void		draw(RendererInterface*);
	void		update();
	
	
	Texture	    *skin_textures;
	int			skin_ntextures;

	Cluster		*skin_clusters;
	int			skin_nclusters;


	float*	    skin_vertexcache;
};

struct Camera : EntityComponent
{
	float fov;
	float ratio;
	/*float near;
	float far;*/

	mat4 matrix;
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

	void draw(RendererInterface*);
};



struct TextureLayered
{
	std::vector<Texture*> textures;

	TextureLayered();

	TextureLayered* GetTextureLayered(){return this;}

	void draw(RendererInterface*){}
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

	void draw(RendererInterface*){}
};



#endif //__ENTITY_HEADER__
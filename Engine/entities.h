#ifndef __ENTITY_HEADER__
#define __ENTITY_HEADER__

#include "interfaces.h"

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
	Bone* cluster_bone;

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

	TDAutoArray<Keyframe*>	keycurve_keyframes;
	EChannel		keycurve_channel;
	float			keycurve_start;
	float			keycurve_end;
};

namespace Timer
{
	int GetTime();
}


struct CurveGroup
{
	CurveGroup();

	TDAutoArray<KeyCurve*> curvegroup_keycurves;

	float curvegroup_start;
	float curvegroup_end;
};



struct Animation
{
	Animation();

	virtual int animate(float)=0;

	TDAutoArray<CurveGroup*> animation_curvegroups;

	int		animation_nprocessed;
	float	animation_time;
	bool	animation_direction;

	float	animation_start;
	float	animation_end;

	EAnimationState animation_state;

	int animation_animselected;
	mat4 animation_transform;

	vec3 animation_pos;
	vec3 animation_scl;
	vec3 animation_rot;
};

struct Entity : Animation
{
	static std::list<Entity*> pool;
	static Entity* Find(const char*,bool exact=true);

	virtual Entity* GetEntity(){return this;}
	virtual Bone* GetBone(){return 0;}
	virtual Mesh* GetMesh(){return 0;}
	virtual Skin* GetSkin(){return 0;}
	virtual Light* GetLight(){return 0;}

	virtual void beginDraw();
	virtual void draw(RendererInterface*);
	virtual void endDraw();
	virtual void update();
	int		 animate(float);

	Entity*					entity_parent;
	std::list<Entity*>	entity_childs;		

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


};

struct Bone : Entity
{
	Bone();

	Entity* bone_root;

	vec3    bone_color;

	Bone* GetBone(){return this;}

	void draw(RendererInterface*);
	void update();
	int animate(float);
};

struct Light : Entity
{


	Light();

	Light* GetLight(){return this;}
	
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

struct Mesh : Entity
{
	Mesh();

	Mesh* GetMesh(){return this;}

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

	Skin* GetSkin(){return this;}


	void		draw(RendererInterface*);
	void		update();
	
	
	Texture	    *skin_textures;
	int			skin_ntextures;

	Cluster		*skin_clusters;
	int			skin_nclusters;


	float*	    skin_vertexcache;
	
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
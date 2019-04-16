#ifndef __ENTITY_HEADER__
#define __ENTITY_HEADER__

#include "primitives.h"

struct DLLBUILD EntityBase;
struct DLLBUILD Entity;
struct DLLBUILD EntityComponent;
struct DLLBUILD Cluster;
struct DLLBUILD Bone;
struct DLLBUILD EntityScript;
struct DLLBUILD AnimationController;

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



struct DLLBUILD Shader
{
	Renderer3DBase* renderer; 

	Shader(Renderer3DBase*);
	virtual ~Shader();

	static Shader* Find(const char*,bool exact=true);

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
	virtual int GetPointSize()=0;

	virtual void SetSelectionColor(bool pick,void* ptr,vec2 iMpos,vec2 iRectSize)=0;


	virtual bool SetMatrix4f(int slot,float* mtx)=0;

	virtual unsigned int& GetBufferObject()=0;

	virtual void SetProjectionMatrix(float*)=0;
	virtual void SetModelviewMatrix(float*)=0;
	virtual void SetMatrices(float* view,float* mdl)=0;

	

	String				name;
	int					programId;
	int					vertexShaderId;
	int					fragmentShaderId;
	unsigned int		vbo;
	unsigned int		vao;
	unsigned int		ibo;
};

struct DLLBUILD Serializer
{
	static const int Root=0;
	static const int Skeleton=1;
	static const int Animation=2;
	static const int Gizmo=3;
	static const int AnimationController=4;
	static const int Bone=5;
	static const int Light=6;
	static const int Mesh=7;
	static const int Script=8;
	static const int Camera=9;
	static const int Project=10;
	static const int Scene=11;
	static const int Entity=12;
	static const int Material=13;
	static const int Texture=14;
	static const int Line=14;
	static const int Skin=15;
	static const int Unknown=0xffffffff;
};

struct DLLBUILD Resource
{
	static void* Load(FilePath iResourceName);
};

struct DLLBUILD Texture
{
	unsigned char*	m_buf;
	int		m_bufsize;
	int		m_width;
	int		m_height;
	int		m_bpp;

	Texture();
	~Texture();

	virtual int load(char*)=0;
	virtual void* GetBuffer()=0;
	virtual int GetSize()=0;
	virtual int GetWidth()=0;
	virtual int GetHeight()=0;
	virtual int GetBpp()=0;
};


struct DLLBUILD Material
{
	enum Type
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


	std::vector<Texture*> textures;

	Type m_type;

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
	unsigned int		ncontrolpointindex;
	float	weight;

	unsigned int		*controlpointindex;

	Influence();
};

struct DLLBUILD Cluster
{
	Entity* bone;

	Influence		*influences;
	unsigned int	ninfluences;

	mat4		offset;

	Cluster();
};

struct DLLBUILD Keyframe
{
	Keyframe();
	float time;
	float value;
};

struct DLLBUILD KeyCurve
{
	enum Channel
	{
		SCALEX=0,
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

	KeyCurve();
	~KeyCurve();

	std::vector<Keyframe*> frames;

	unsigned int	channel;
	float			start;
	float			end;
};


struct DLLBUILD AnimClip
{
	AnimClip();
	~AnimClip();

	std::vector<KeyCurve*> curves;

	float start;
	float end;
};

struct DLLBUILD EntityBase
{
	virtual void init(){}
	virtual void update(){}
	virtual void render(Renderer3DBase*){}
	virtual void deinit(){}
};

struct DLLBUILD EntityComponent : EntityBase
{
private:
	friend Entity;
	Entity*			entity;
public:
	EntityComponent();
	virtual ~EntityComponent();

	Entity* Entity();

	template<class C> C* is(){return dynamic_cast<C*>(this);}

	virtual void update();
	virtual void render(Renderer3DBase*);
};
	
struct DLLBUILD Entity : EntityBase
{
protected:
	Entity*					parent;

	std::list<EntityComponent*> components;
	std::list<Entity*> childs;

	bool					saved;
public:

	unsigned int			id;

	mat4					local;
	mat4					world;

	String					name;

	AABB					bbox;


	Entity();
	virtual ~Entity();

	virtual Entity*						Append(Entity* iEntity);
	virtual Entity*						Remove(Entity* iEntity);
	const std::list<Entity*>&			Childs();
	const std::list<EntityComponent*>&	Components();
	virtual Entity*						Parent();
	virtual void						update();
	virtual void						render(Renderer3DBase*);

	template<class C> C* CreateComponent()
	{
		C* tCreatedComponent=new C;
		tCreatedComponent->entity=this;
		this->components.push_back(tCreatedComponent);
		return tCreatedComponent;
	}

	template<class C> C* FindComponent()
	{
		for(std::list<EntityComponent*>::iterator it=this->components.begin();it!=this->components.end();it++)
		{
			if(dynamic_cast<C*>(*it))
				return (C*)*it;
		}

		return 0;
	}

	template<class C> std::vector<C*> FindComponents()
	{
		std::vector<C*> vecC;

		for(std::list<EntityComponent*>::iterator it=this->components.begin();it!=this->components.end();it++)
		{
			if(dynamic_cast<C*>(*it))
				vecC.push_back((C*)*it);
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
	std::vector<AnimClip*> clips;

	float	start;
	float	end;

	int index;

	Animation();
	~Animation();
};

struct DLLBUILD Gizmo : EntityComponent
{
};

struct DLLBUILD AnimationController : EntityComponent
{
	std::vector<Animation*> animations;

	float speed;
	float cursor;

	bool playing;
	bool looped;

	float start;
	float end;

	unsigned int framesPerSecond;
	unsigned int frameTime;

	AnimationController();
	virtual ~AnimationController();

	void AddAnimation(Animation*);

	void SetId(unsigned int);

	void Stop();
	void Play();

	virtual void update();

	void SetFrame(float iCursor);
};

struct DLLBUILD Bone : EntityComponent
{
	Bone();

	Bone* root;

	vec3    color;

};

struct DLLBUILD Line : EntityComponent
{
	Line();
	~Line();

	std::list<vec3> points;

	void render(Renderer3DBase*);
};

struct DLLBUILD Light : EntityComponent
{
	struct Type
	{
		static const int POINT=0;
		static const int DIRECTIONAL=1;
		static const int SPOT=2;
		static const int AREA=3;
		static const int VOLUME=4;
		static const int UNKNOWN=5;
	};

	struct Decay
	{
		static const int NONE=0;
		static const int LINEAR=1;
		static const int QUADRATIC=2;
		static const int CUBIC=3;
		static const int UNKNOWN=4;
	};

	Light();

	unsigned int	type;
	unsigned int	decaytype;

	bool    cast;
	bool    volumetric;
	bool    groundprojection;
	bool    nearattenuation;
	bool    farattenuation;
	bool    shadows;
	vec3	color;
	vec3	shadowcolor;
	float	intensity;
	float	innerangle;
	float	outerangle;
	float   fog;
	float	decaystart;
	float	nearstart;
	float	nearend;
	float	farstart;
	float	farend;
};

struct DLLBUILD Mesh : EntityComponent
{
	Mesh();
	~Mesh();

	unsigned int	  ncontrolpoints;
	unsigned int	  nvertexindices;
	unsigned int	  ntexcoord;
	unsigned int	  nnormals;
	unsigned int	  npolygons;

	float (*controlpoints)[3];

	unsigned int *vertexindices;

	float (*texcoord)[2];
	float (*normals)[3];
	
	bool  isCCW;

	std::vector<Material*> materials;

	void render(Renderer3DBase*);
};

struct DLLBUILD Script : EntityComponent
{
	FilePath		sourcepath;
	EntityScript*	runtime;
	void*			handle;
	
	Script();

	virtual void update();
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
	unsigned int	nclusters;
	unsigned int	ntextures;

	Texture			*textures;
	Cluster			*clusters;

	float			*vertexcache;

	Skin();

	virtual void	update();
	virtual void	render(Renderer3DBase*);
};

struct DLLBUILD Camera : EntityComponent
{
	float fov;
	float ratio;
	float Near;
	float Far;

	bool  perspective;

	Camera();
	Camera(vec3 pos,vec3 target,vec3 up,float iFov,float iRatio,float iNear,float iFar,bool iPerspective);

	mat4 projection;
	mat4 view;
	mat4 model;

	vec3 target;

	virtual void update();
};

struct DLLBUILD TextureFile : Texture
{
	TextureFile();
	~TextureFile();

	void* GetBuffer();
	int GetSize();
	int GetWidth();
	int GetHeight();
	int GetBpp();

	int load(char*){return 0;}

	FilePath filename;
};



struct DLLBUILD TextureLayered
{
	std::vector<Texture*> textures;

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
	virtual ~Renderer3DBase();

	virtual void UpdateEntities(Entity*);
	virtual void RenderEntities(Entity*);

	virtual void Initialize(){};
	virtual void Deinitialize(){};

	virtual int		CreateShader(const char* name,int shader_type, const char* shader_src)=0;
	virtual Shader* CreateProgram(const char* name,const char* vertexsh,const char* fragmentsh)=0;
	virtual Shader* CreateShaderProgram(const char* name,const char* pix,const char* frag)=0;

	virtual void DrawPoint(vec3,float psize=1.0f,vec3 color=vec3(1,1,1))=0;
	virtual void draw(vec2)=0;
	virtual void DrawLine(vec3,vec3,vec3 color=vec3(1,1,1))=0;
	virtual void draw(vec4)=0;
	virtual void draw(AABB,vec3 color=vec3(1,1,1))=0;
	virtual void draw(mat4 mtx,float size,vec3 color=vec3(1,1,1))=0;
	//virtual void draw(Font*,char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4)=0;
	virtual void DrawText(char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4)=0;
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

	Shader* shader_unlit;
	Shader* shader_unlit_color;
	Shader* shader_unlit_texture;
	Shader* shader_font;
	Shader* shader_shaded_texture;

	std::vector<Shader*> shaders;
};

struct DLLBUILD Scene
{
protected:
	Entity* rootSceneEntity;
	String	sceneName;
public:

	const String& 	GetSceneName();
	void			SetSceneName(const String&);

	virtual Entity*			GetSceneRootEntity();
	virtual void			SetSceneRootEntity(Entity*);

	Scene();
};

#endif //__ENTITY_HEADER__

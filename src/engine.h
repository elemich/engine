#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <list>
#include <functional>
#include <algorithm>
#include <cctype>
#include <cstring> //for g++ str* functions

//forward declaration

struct ThreadData;

//data structs

struct StringData
{
	char* data;
	size_t size;
};
struct FilePathData : StringData{};
struct vec2Data{float v[2];};
struct vec3Data{float v[3];};
struct vec4Data{float v[4];};
struct mat2Data{float v[4];};
struct mat3Data{float v[9];};
struct mat4Data{float v[16];};

struct TimerData
{
	unsigned int currentFrameTime;
	unsigned int currentFrameDeltaTime;
	unsigned int lastFrameTime;
	unsigned int renderFps;
};

struct TaskData
{
	std::function<void()> func;
	bool remove;
	bool executing;
	bool pause;
	void* owner;
};

struct ThreadData
{
	int id;
	bool pause;
	std::list<void*> tasks;
	void* executing;
	unsigned int sleep;
};

extern "C" __declspec(dllexport) void* 	MallocData(size_t iBytes);
extern "C" __declspec(dllexport) void 	FreeData(void* iData);

extern "C" __declspec(dllexport) void 	DeleteData(void*);
extern "C" __declspec(dllexport) void 	DeleteArrayData(void*);

extern "C" __declspec(dllexport) StringData* 	CreateStringData();
extern "C" __declspec(dllexport) FilePathData* 	CreateFilePathData();
extern "C" __declspec(dllexport) vec2Data* 		CreateVec2Data();
extern "C" __declspec(dllexport) vec3Data* 		CreateVec3Data();
extern "C" __declspec(dllexport) vec4Data* 		CreateVec4Data();
extern "C" __declspec(dllexport) mat2Data* 		CreateMat2Data();
extern "C" __declspec(dllexport) mat3Data* 		CreateMat3Data();
extern "C" __declspec(dllexport) mat4Data* 		CreateMat4Data();
extern "C" __declspec(dllexport) TimerData* 	CreateTimerData();
extern "C" __declspec(dllexport) TaskData* 		CreateTaskData();
extern "C" __declspec(dllexport) ThreadData* 	CreateThreadData();

//forward declarations

struct EntityData;
struct ClusterData;
struct BoneData;
struct EntityScriptData;

struct Renderer3DBaseData;
struct ShaderData;

//data structs 

struct ResourceData
{
	ResourceData* parent;
	std::vector<ResourceData*> childs;
};

struct FileData : ResourceData
{
	FilePathData path;
	void* data;
};

struct DirData : ResourceData
{
	StringData fullPath;
};

struct TextureData
{
};


struct MaterialData : ResourceData
{
	std::vector<TextureData*> textures;
	unsigned int m_type;
};


struct InfluenceData
{
	int		*cpIdx;
	int		nCpIdx;
	float	weight;
};


struct ClusterData
{
	EntityData* bone;

	InfluenceData	*influences;
	int			ninfluences;

	mat4Data		offset;
};


struct KeyframeData
{
	float		 time;
	float value;
};

struct KeyCurveData
{
	std::vector<KeyframeData*>	frames;
	unsigned int		channel;
	float			start;
	float			end;
};


struct AnimClipData
{
	std::vector<KeyCurveData*> curves;

	float start;
	float end;
};

struct EntityBaseData
{
	
};

struct EntityComponentData : EntityBaseData
{
	EntityData* entity;
};

struct EntityData : EntityBaseData
{
	void*					parent;
	std::list<void*>		childs;
	std::vector<void*> components;
};

struct RootData : EntityComponentData
{
};

struct SkeletonData : EntityComponentData
{
};

struct AnimationData : EntityComponentData
{
	EntityData* entity;

	std::vector<AnimClipData*> clips;

	float	start;
	float	end;

	int clipIdx;
};

struct GizmoData : EntityComponentData
{
};

struct AnimationControllerData : EntityComponentData
{
	std::vector<AnimationData*> animations;

	float speed;
	float cursor;
	bool play;
	bool looped;

	float start;
	float end;

	int resolutionFps;

	int lastFrameTime;
};

struct BoneData : EntityComponentData
{
	BoneData* root;
	vec3Data    color;
};

struct LightData : EntityComponentData
{
	unsigned int	light_type;
	unsigned int	light_decaytype;
	bool    light_cast;
	bool    light_volumetric;
	bool    light_groundprojection;
	bool    light_nearattenuation;
	bool    light_farattenuation;
	bool    light_shadows;
	vec3Data	light_color;
	vec3Data	light_shadowcolor;
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

struct MeshData : EntityComponentData
{
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

	std::vector<MaterialData*> materials;
};

struct ScriptData : EntityComponentData
{
	FileData file;
	EntityScriptData* runtime;
	FilePathData modulePath;
	unsigned int moduleBase;
};

struct EntityScriptData
{
	EntityData* entity;
};

struct SkinData : MeshData
{
	TextureData	    *textures;
	int			ntextures;

	ClusterData		*clusters;
	int			nclusters;


	float*	    vertexcache;
};

struct CameraData : EntityComponentData
{
	float fov;
	float ratio;
	float Near;
	float Far;

	mat4Data matrix;

	vec3Data target;
};

struct TextureFileData : TextureData
{
	StringData filename;
	void* __data;
};

struct TextureLayeredData
{
	std::vector<TextureData*> textures;
};

struct TextureProceduralData : TextureData 
{
};

struct Renderer3DBaseData
{
	bool picking;

	ShaderData* unlit;
	ShaderData* unlit_color;
	ShaderData* unlit_texture;
	ShaderData* font;
	ShaderData* shaded_texture;

	std::vector<ShaderData*> shaders;
};

struct SceneData
{
	EntityData* rootEntity;
};

///////////////////////////////////////

extern "C" __declspec(dllexport) ResourceData* CreateResourceData();
extern "C" __declspec(dllexport) FileData* CreateFileData();
extern "C" __declspec(dllexport) DirData* CreateDirData();
extern "C" __declspec(dllexport) TextureData* CreateTextureData();
extern "C" __declspec(dllexport) MaterialData* CreateMaterialData();
extern "C" __declspec(dllexport) InfluenceData* CreateInfluenceData();
extern "C" __declspec(dllexport) ClusterData* CreateClusterData();
extern "C" __declspec(dllexport) KeyframeData* CreateKeyframeData();
extern "C" __declspec(dllexport) KeyCurveData* CreateKeyCurveData();
extern "C" __declspec(dllexport) AnimClipData* CreateAnimClipData();
extern "C" __declspec(dllexport) EntityBaseData* CreateEntityBaseData();
extern "C" __declspec(dllexport) EntityComponentData* CreateEntityComponentData();
extern "C" __declspec(dllexport) EntityData* CreateEntityData();
extern "C" __declspec(dllexport) EntityData* CreateEntityData();
extern "C" __declspec(dllexport) RootData* CreateRootData();
extern "C" __declspec(dllexport) SkeletonData* CreateSkeletonData();
extern "C" __declspec(dllexport) AnimationData* CreateAnimationData();
extern "C" __declspec(dllexport) GizmoData* CreateGizmoData();
extern "C" __declspec(dllexport) AnimationControllerData* CreateAnimationControllerData();
extern "C" __declspec(dllexport) BoneData* CreateBoneData();
extern "C" __declspec(dllexport) LightData* CreateLightData();
extern "C" __declspec(dllexport) MeshData* CreateMeshData();
extern "C" __declspec(dllexport) ScriptData* CreateScriptData();
extern "C" __declspec(dllexport) EntityScriptData* CreateEntityScriptData();
extern "C" __declspec(dllexport) SkinData* CreateSkinData();
extern "C" __declspec(dllexport) CameraData* CreateCameraData();
extern "C" __declspec(dllexport) TextureFileData* CreateTextureFileData();
extern "C" __declspec(dllexport) TextureLayeredData* CreateTextureLayeredData();
extern "C" __declspec(dllexport) TextureProceduralData* CreateTextureProceduralData();
extern "C" __declspec(dllexport) Renderer3DBaseData* CreateRenderer3DBaseData();
extern "C" __declspec(dllexport) SceneData* CreateSceneData();

#endif //ENGINE_H




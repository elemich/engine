#include "engine.h"

#include <cstdio>
#include <cstdlib>
#include <typeinfo>
#include <cmath>

extern "C" void* MallocData(size_t iBytes)
{
	return (void*)malloc(iBytes);
}

extern "C" void FreeData(void* iData)
{
	free(iData);
	iData=0;
}

extern "C" void DeleteData(void* genericData)
{
	delete genericData;
	genericData=0;
}

extern "C" void DeleteArrayData(void* genericArrayData)
{
	delete [] genericArrayData;
	genericArrayData=0;
}

extern "C" StringData* CreateStringData()
{
	StringData* stringData=new StringData;
	stringData->data=0;
	stringData->size=0;
	return stringData;
}

extern "C" FilePathData* CreateFilePathData()
{
	FilePathData* filepathdata=new FilePathData;
	return filepathdata;
}

extern "C" vec2Data* CreateVec2Data()
{
	vec2Data* v2data=new vec2Data;
	v2data->v[0]=0;
	v2data->v[1]=0;
	return v2data;
}

extern "C" vec3Data* CreateVec3Data()
{
	vec3Data* vec3data=new vec3Data;
	vec3data->v[0]=0;
	vec3data->v[1]=0;
	vec3data->v[2]=0;
	return vec3data;
}

extern "C" vec4Data* CreateVec4Data()
{
	vec4Data* vec4data=new vec4Data;
	vec4data->v[0]=0;
	vec4data->v[1]=0;
	vec4data->v[2]=0;
	vec4data->v[3]=0;
	return vec4data;
}

extern "C" mat2Data* CreateMat2Data()
{
	mat2Data* mat2data=new mat2Data;
	mat2data->v[0]=1;
	mat2data->v[1]=0;
	mat2data->v[2]=0;
	mat2data->v[3]=1;
	return mat2data;
}

extern "C" mat3Data* CreateMat3Data()
{
	mat3Data* mat3data=new mat3Data;
	mat3data->v[0]=1;
	mat3data->v[1]=0;
	mat3data->v[2]=0;
	mat3data->v[3]=0;
	mat3data->v[4]=1;
	mat3data->v[5]=0;
	mat3data->v[6]=0;
	mat3data->v[7]=0;
	mat3data->v[8]=1;
	
	return mat3data;
}

extern "C" mat4Data* CreateMat4Data()
{
	mat4Data* mat4data=new mat4Data;
	mat4data->v[0]=1;
	mat4data->v[1]=0;
	mat4data->v[2]=0;
	mat4data->v[3]=0;
	mat4data->v[4]=0;
	mat4data->v[5]=1;
	mat4data->v[6]=0;
	mat4data->v[7]=0;
	mat4data->v[8]=0;
	mat4data->v[9]=0;
	mat4data->v[10]=1;
	mat4data->v[11]=0;
	mat4data->v[12]=0;
	mat4data->v[13]=0;
	mat4data->v[14]=0;
	mat4data->v[15]=1;
	
	return mat4data;
}

extern "C" TimerData* CreateTimerData()
{
	TimerData* timerdata=new TimerData;
	
	return timerdata;
}

extern "C" TaskData* CreateTaskData()
{
	TaskData* taskdata=new TaskData;
	
	taskdata->remove=0;
	taskdata->executing=0;
	taskdata->pause=0;
	taskdata->owner=0;
	
	return taskdata;
}

extern "C" ThreadData* CreateThreadData()
{
	ThreadData* threaddata=new ThreadData;
	
	threaddata->id=0;
	threaddata->pause=0;
	threaddata->executing=0;
	threaddata->sleep=1;
	
	return threaddata;
}

/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////

#include "imgpng.h"
#include "imgjpg.h"
#include "imgtga.h"

#include <algorithm>

extern "C" ResourceData* CreateResourceData()
{
	ResourceData* resourcedata=new ResourceData;
	resourcedata->parent=0;
	return resourcedata;
}

extern "C" FileData* CreateFileData()
{
	FileData* filedata=new FileData;
	filedata->data=0;
	return filedata;
}

extern "C" DirData* CreateDirData()
{
	DirData* dirdata=new DirData;
	return dirdata;
}

extern "C" TextureData* CreateTextureData()
{
	TextureData* texturedata=new TextureData;
	return texturedata;
}

extern "C" MaterialData* CreateMaterialData()
{
	MaterialData* materialdata=new MaterialData;
	materialdata->m_type=0;
	return materialdata;
}

extern "C" InfluenceData* CreateInfluenceData()
{
	InfluenceData* influencedata=new InfluenceData;
	influencedata->cpIdx=0;
	influencedata->nCpIdx=0;
	influencedata->weight=0;
	return influencedata;
}

extern "C" ClusterData* CreateClusterData()
{
	ClusterData* clusterdata=new ClusterData;
	clusterdata->bone=0;
	clusterdata->influences=0;
	clusterdata->ninfluences=0;
	return clusterdata;
}

extern "C" KeyframeData* CreateKeyframeData()
{
	KeyframeData* keyframedata=new KeyframeData;
	keyframedata->time=0;
	keyframedata->value=0;
	return keyframedata;
}

extern "C" KeyCurveData* CreateKeyCurveData()
{
	KeyCurveData* keycurvedata=new KeyCurveData;
	keycurvedata->channel=0;
	keycurvedata->start=0;
	keycurvedata->end=0;
	return keycurvedata;
}

extern "C" AnimClipData* CreateAnimClipData()
{
	AnimClipData* animclipdata=new AnimClipData;
	animclipdata->start=0;
	animclipdata->end=0;
	return animclipdata;
}

extern "C" EntityBaseData* CreateEntityBaseData()
{
	EntityBaseData* t=new EntityBaseData;
	return t;
}

extern "C" EntityComponentData* CreateEntityComponentData()
{
	EntityComponentData* t=new EntityComponentData;
	t->entity=0;
	return t;
}

extern "C" EntityData* CreateEntityData()
{
	EntityData* t=new EntityData;
	t->parent=0;
	return t;
}


extern "C" RootData* CreateRootData()
{
	RootData* t=new RootData;
	return t;
}

extern "C" SkeletonData* CreateSkeletonData()
{
	SkeletonData* t=new SkeletonData;
	return t;
}

extern "C" AnimationData* CreateAnimationData()
{
	AnimationData* t=new AnimationData;
	return t;
}

extern "C" GizmoData* CreateGizmoData()
{
	GizmoData* t=new GizmoData;
	return t;
}

extern "C" AnimationControllerData* CreateAnimationControllerData()
{
	AnimationControllerData* t=new AnimationControllerData;
	return t;
}

extern "C" BoneData* CreateBoneData()
{
	BoneData* t=new BoneData;
	return t;
}

extern "C" LightData* CreateLightData()
{
	LightData* t=new LightData;
	return t;
}

extern "C" MeshData* CreateMeshData()
{
	MeshData* t=new MeshData;
	return t;
}

extern "C" ScriptData* CreateScriptData()
{
	ScriptData* t=new ScriptData;
	return t;
}

extern "C" EntityScriptData* CreateEntityScriptData()
{
	EntityScriptData* t=new EntityScriptData;
	return t;
}

extern "C" SkinData* CreateSkinData()
{
	SkinData* t=new SkinData;
	return t;
}

extern "C" CameraData* CreateCameraData()
{
	CameraData* t=new CameraData;
	return t;
}

extern "C" TextureFileData* CreateTextureFileData()
{
	TextureFileData* t=new TextureFileData;
	return t;
}

extern "C" TextureLayeredData* CreateTextureLayeredData()
{
	TextureLayeredData* t=new TextureLayeredData;
	return t;
}

extern "C" TextureProceduralData* CreateTextureProceduralData()
{
	TextureProceduralData* t=new TextureProceduralData;
	return t;
}

extern "C" Renderer3DBaseData* CreateRenderer3DBaseData()
{
	Renderer3DBaseData* t=new Renderer3DBaseData;
	return t;
}

extern "C" SceneData* CreateSceneData()
{
	SceneData* t=new SceneData;
	return t;
}

bool DllMain(void*,unsigned int iReason,void*)
{
	if(iReason==1)//DLL_PROCESS_ATTACH
	{
		printf("engine dll loaded\n");
		return true;
	}
}




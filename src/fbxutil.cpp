#include "fbxutil.h"

#include "win32.h"

Fbx* allocatedPlugin=0;

Fbx::Fbx()
{
	this->name=L"FbxPlugin";
}

void Fbx::Load()
{
	if(!this->loaded)
	{
		this->MenuFbx=this->Menu(L"Plugins\\Fbx",true);
		this->MenuActionImport=this->Menu(L"Plugins\\Fbx\\Import",false);
		this->MenuActionExport=this->Menu(L"Plugins\\Fbx\\Export",false);

		this->loaded=true;
	}
}

void Fbx::Unload()
{
	if(this->loaded)
	{


	}
}

void Fbx::OnMenuPressed(int iIdx)
{
	if(iIdx==MenuActionImport)
	{
		String tFbxFile=Ide::GetInstance()->subsystem->FileChooser(L"Fbx Files (*.fbx)",L"*.fbx");

		if(tFbxFile.size())
		{
			EditorEntity* importedEntities=this->Import(StringUtils::ToChar(tFbxFile).c_str());
			Ide::GetInstance()->mainAppWindow->mainContainer->BroadcastToTabs(&Tab::OnEntitiesChange,importedEntities);
		}
	}
	else if(iIdx==MenuActionExport)
	{
		wprintf(L"not yet implemented\n");
	}
}

PluginSystem::Plugin* GetPlugin(PluginSystem* tPluginSystem)
{
	allocatedPlugin=new Fbx;
	return allocatedPlugin;
}
void DestroyPlugin()
{
	if(allocatedPlugin)
	{
		delete allocatedPlugin;
		allocatedPlugin=0;
	}
}

#include <map>

#include <fbxsdk.h>
#include <fbxsdk\fileio\fbxiosettings.h>

#include <windows.h>



FbxManager* globalFbxManager=0;
const char* globalSceneFilename=0;

std::map<FbxNode*,EditorEntity*>				globalMapFromNodeToEntity;
std::map<FbxNode*,AnimationController*> globalMapFromNodeToAnimationController;
std::map<FbxSurfaceMaterial*,Material*>	globalMapFromFbxMaterialToMaterial;
std::map<FbxTexture*,Texture*>			globalMapFromFbxTextureToTexture;

EditorEntity* globalRootEntity=0;

#define GENERATE_MISSING_KEYS 0
#define GENERATE_INDEXED_GEOMETRY 1
#define DEBUG_PRINTF 0

//#pragma message (LOCATION " remember to implement generating missing fbx animation keys")

void FillSkin(FbxNode* fbxNode,Skin* skin);
void FillMesh(FbxNode* fbxNode,Mesh* mesh);
void FillMaterial(FbxNode*,Material*);
void FillLight(FbxNode* fbxNode,Light* light);

void StoreKeyframes(float& animationStart,float& animationEnd,AnimClip* animation,EChannel channel,FbxAnimCurve* fbxAnimCurve);
void ExtractAnimations(FbxNode*,EditorEntity*);
void ExtractTexturesandMaterials(FbxScene*);
void GetSceneDetails(FbxScene*);

bool AnimationPresent(FbxAnimLayer * pAnimLayer, FbxNode* pNode);
bool AnimationTake(FbxAnimLayer* pAnimLayer,FbxNode* pNode,Animation* animation,String &animnae);

EditorEntity* acquireNodeStructure(FbxNode* fbxNode,EditorEntity* parent);
EditorEntity* acquireNodeData(FbxNode* fbxNode,EditorEntity* parent);

EditorEntity* processNodeRecursive(EditorEntity* (*func)(FbxNode*,EditorEntity*),FbxNode* fbxNode,EditorEntity* parent);

struct cmat
{
	vec3 a,b,c;

	cmat(vec3 x,vec3 y,vec3 z)
	{
		a=x,b=y,c=z;
	}
};

void FillMaterials(FbxNode* fbxNode);

int nBonesTotal=0;

mat4 GetMatrix(FbxAMatrix &fbxAMatrix)
{
	mat4 retmat(fbxAMatrix);

	return retmat;
}

mat4 GetMat4(FbxAMatrix fbxAMatrix)
{
	FbxVector4 t=fbxAMatrix.GetT();
	FbxVector4 r=fbxAMatrix.GetR();
	FbxVector4 s=fbxAMatrix.GetS();

	mat4 m;

	m.rotate(vec3((float)r[0],(float)r[1],(float)r[2]));
	m.scale(vec3((float)s[0],(float)s[1],(float)s[2]));
	m.translate((float)t[0],(float)t[1],(float)t[2]);

	return m;
}

cmat GetCmat(FbxAMatrix fbxAMatrix)
{
	FbxVector4 t=fbxAMatrix.GetT();
	FbxVector4 r=fbxAMatrix.GetR();
	FbxVector4 s=fbxAMatrix.GetS();

	cmat m(vec3((float)t[0],(float)t[1],(float)t[2]),vec3((float)r[0],(float)r[1],(float)r[2]),vec3((float)s[0],(float)s[1],(float)s[2]));

	return m;
}

void GetVec3(float* b,FbxDouble3 a)
{
	b[0]=(float)a[0],b[1]=(float)a[1],b[2]=(float)a[2];
}

void GetVec4(float* b,FbxDouble4 a)
{
	b[0]=(float)a[0],b[1]=(float)a[1],b[2]=(float)a[2],b[3]=(float)a[3];
}

FbxAMatrix GetPoseMatrix(FbxPose* pPose, int pNodeIndex)
{
	FbxAMatrix lPoseMatrix;
	FbxMatrix lMatrix = pPose->GetMatrix(pNodeIndex);

	memcpy((double*)lPoseMatrix, (double*)lMatrix, sizeof(lMatrix.mData));

	return lPoseMatrix;
}

void GetComp(FbxAMatrix& am,FbxVector4& t,FbxVector4& r,FbxVector4& s)
{
	t=am.GetT();
	r=am.GetR();
	s=am.GetS();
}

// Get the geometry offset to a node. It is never inherited by the children.
FbxAMatrix GetGeometry(FbxNode* pNode)
{
	const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);

	return FbxAMatrix(lT, lR, lS);
}


EditorEntity* acquireNodeStructure(FbxNode* iFbxNode,EditorEntity* iEntityParent)
{
	EditorEntity* tEntity=new EditorEntity;

	if(!iEntityParent)//create the root and add a child to it
	{
		globalRootEntity=tEntity;

		tEntity->name=FilePath(StringUtils::ToWide(globalSceneFilename)).Name();
	}
	
	globalMapFromNodeToEntity.insert(std::pair<FbxNode*,EditorEntity*>(iFbxNode,tEntity));

	tEntity->OnPropertiesCreate();
		
	if(iEntityParent && !tEntity->name.size())
		tEntity->name=StringUtils::ToWide(iFbxNode->GetName());

	tEntity->local=GetMatrix(iFbxNode->EvaluateLocalTransform(FBXSDK_TIME_ZERO));

	ExtractAnimations(iFbxNode,tEntity);
	tEntity->SetParent(iEntityParent);

	if(iFbxNode->GetScene()->GetRootNode()==iFbxNode)
	{
		FbxAxisSystem as=iFbxNode->GetScene()->GetGlobalSettings().GetAxisSystem();

		int sign;

		vec3 axes;

		switch(as.GetUpVector(sign))
		{
		case FbxAxisSystem::eXAxis:
			tEntity->local.rotate((float)sign * 90,0,0,1);
			printf("upVector is X\n");
			break;
		case FbxAxisSystem::eYAxis:
			tEntity->local.rotate(-90.0f,1,0,0);
			printf("upVector is Y\n");
			break;
		case FbxAxisSystem::eZAxis:
				
			/*axes=entity->local.axis(1,0,0);
			entity->local.rotate((float)sign * 90,axes);*/
			printf("upVector is Z\n");
			break;
		}
	}

	tEntity->world = tEntity->parent ? (tEntity->local * tEntity->parent->world) : tEntity->local;

	return tEntity;
}

EditorEntity* acquireNodeData(FbxNode* fbxNode,EditorEntity* parent)
{
	EditorEntity* entity=globalMapFromNodeToEntity[fbxNode];

	if(!entity)
		return 0;

	if(fbxNode->GetSkeleton())
	{
		EditorBone* bone=entity->CreateComponent<EditorBone>();

		if(bone)
		{
			if(bone->entity->parent)
			{
				if(bone->entity->parent->findComponent<EditorBone>())
					bone->root=bone->entity->parent->findComponent<EditorBone>()->root;
				else
					bone->root=bone;
			}
		}

		nBonesTotal++;
	}
	else if(fbxNode->GetGeometry())
	{
		int deformerCount=fbxNode->GetGeometry()->GetDeformerCount();

		Skin* skin=0;
		Mesh* mesh=0;
		EditorLine* line=0;

		if(deformerCount)//skin
		{
			skin=entity->CreateComponent<EditorSkin>();
			FillSkin(fbxNode,skin);
		}
		else if(fbxNode->GetMesh())
		{
			mesh=entity->CreateComponent<EditorMesh>();
			FillMesh(fbxNode,mesh);
		}
		else if(fbxNode->GetLine())
		{
			FbxLine* fbxLine=fbxNode->GetLine();

			if(fbxLine)
			{
				line=entity->CreateComponent<EditorLine>();

				for(int i=0;i<fbxLine->GetIndexArraySize();i++)
				{
					FbxVector4 tFbxPoint=fbxLine->GetControlPointAt(fbxLine->GetPointIndexAt(i));
					line->AddPoint(vec3(tFbxPoint[0],tFbxPoint[1],tFbxPoint[2]));
				}
			}
		}

		for(int i=0;i<fbxNode->GetMaterialCount();i++)
		{
			Material* material=(Material*)globalMapFromFbxMaterialToMaterial.at(fbxNode->GetMaterial(i));

			if(!material)
				DEBUG_BREAK();

			mesh ? mesh->materials.push_back(material) : skin->materials.push_back(material);
		}

		fbxNode->GetGeometry()->ComputeBBox();

		FbxDouble3 bbMin=fbxNode->GetGeometry()->BBoxMin;
		FbxDouble3 bbMax=fbxNode->GetGeometry()->BBoxMax;
		
		entity->bbox.a.make((float)bbMin[0],(float)bbMin[1],(float)bbMin[2]);
		entity->bbox.b.make((float)bbMax[0],(float)bbMax[1],(float)bbMax[2]);
	}
	else if(fbxNode->GetLight())
	{
		entity->CreateComponent<EditorLight>();
	}

	return entity;
}


EditorEntity* processNodeRecursive(EditorEntity* (*func)(FbxNode*,EditorEntity*),FbxNode* fbxNode,EditorEntity* parent)
{
	EditorEntity* entity=0;

	if(!fbxNode)
		return entity;

	entity=func(fbxNode,parent);

	int nChilds=fbxNode->GetChildCount();

	for(int i=0;i<nChilds;i++)
		processNodeRecursive(func,fbxNode->GetChild(i),entity);

	return entity;
}


void ParseAnimationCurve(Animation* a)
{
	float& amin=a->start;
	float& amax=a->end;

	for(int i=0;i<(int)a->clips.size();i++)
	{
		float& start=a->clips[i]->start;
		float& end=a->clips[i]->end;

		AnimClip* g=a->clips[i];

		for(int j=0;j<(int)g->curves.size();j++)
		{
				KeyCurve* c=g->curves[j];
				if(c->start!=start )
					DEBUG_BREAK();
				if(c->end!=end)
				{
					Keyframe* key=new Keyframe;
					key->time=end;
					key->value=c->frames[(int)c->frames.size()-1]->value;
					c->frames.push_back(key);
				}
		}
	}
}

void ExtractAnimations(FbxNode* fbxNode,EditorEntity* entity)
{
	for(int animStackIdx=0;animStackIdx<fbxNode->GetScene()->GetSrcObjectCount<FbxAnimStack>();animStackIdx++)
	{
		AnimClip* curvegroup=new AnimClip;

		float	animStart=0,
				animEnd=0;
		
		for(int animLayerIdx=0;animLayerIdx<fbxNode->GetScene()->GetSrcObject<FbxAnimStack>(animStackIdx)->GetSrcObjectCount<FbxAnimLayer>();animLayerIdx++)
		{
			FbxAnimLayer* layer=fbxNode->GetScene()->GetSrcObject<FbxAnimStack>(animStackIdx)->GetSrcObject<FbxAnimLayer>(animLayerIdx);

			StoreKeyframes(animStart,animEnd,curvegroup,TRANSLATEX,fbxNode->LclTranslation.GetCurve(layer,FBXSDK_CURVENODE_COMPONENT_X));
			StoreKeyframes(animStart,animEnd,curvegroup,TRANSLATEY,fbxNode->LclTranslation.GetCurve(layer,FBXSDK_CURVENODE_COMPONENT_Y));
			StoreKeyframes(animStart,animEnd,curvegroup,TRANSLATEZ,fbxNode->LclTranslation.GetCurve(layer,FBXSDK_CURVENODE_COMPONENT_Z));
			StoreKeyframes(animStart,animEnd,curvegroup,ROTATEX,fbxNode->LclRotation.GetCurve(layer,FBXSDK_CURVENODE_COMPONENT_X));
			StoreKeyframes(animStart,animEnd,curvegroup,ROTATEY,fbxNode->LclRotation.GetCurve(layer,FBXSDK_CURVENODE_COMPONENT_Y));
			StoreKeyframes(animStart,animEnd,curvegroup,ROTATEZ,fbxNode->LclRotation.GetCurve(layer,FBXSDK_CURVENODE_COMPONENT_Z));
			StoreKeyframes(animStart,animEnd,curvegroup,SCALEX,fbxNode->LclScaling.GetCurve(layer,FBXSDK_CURVENODE_COMPONENT_X));
			StoreKeyframes(animStart,animEnd,curvegroup,SCALEY,fbxNode->LclScaling.GetCurve(layer,FBXSDK_CURVENODE_COMPONENT_Y));
			StoreKeyframes(animStart,animEnd,curvegroup,SCALEZ,fbxNode->LclScaling.GetCurve(layer,FBXSDK_CURVENODE_COMPONENT_Z));

			
			//DONTCANCEL AnimationTake(layer,fbxNode,animation,animname);
		}

		if(!(int)curvegroup->curves.size())
			delete curvegroup;
		else
		{
			Animation* animation=entity->CreateComponent<EditorAnimation>();
			animation->clips.push_back(curvegroup);
			animation->start=animStart;
			animation->end=animEnd;
			ParseAnimationCurve(animation);

			AnimationController *animController=globalRootEntity->findComponent<AnimationController>();

			if(!animController)
			{
				animController=globalRootEntity->CreateComponent<EditorAnimationController>();
				unsigned int tAnimationControllerFreeId=EditorAnimationController::GetFreeId();
				animController->SetId(tAnimationControllerFreeId);
			}
			if(!animController)
				DEBUG_BREAK();

			if(animController)
				animController->AddAnimation(animation);				
		}
	}
}

		





EditorEntity* Fbx::Import(const char* fname)
{
	globalRootEntity=0;

	printf("Importing file %s\n",fname);

	FbxIOSettings * fbxIOSettings=0;
	FbxImporter* fbxImporter=0;
	FbxScene* fbxScene=0;

	globalSceneFilename=fname;

	globalFbxManager=FbxManager::Create();

	if(globalFbxManager)
		fbxIOSettings = FbxIOSettings::Create(globalFbxManager, IOSROOT );

	if(fbxIOSettings)
		globalFbxManager->SetIOSettings(fbxIOSettings);

	fbxImporter = FbxImporter::Create(globalFbxManager, "");

	if(fbxImporter)
		if(!fbxImporter->Initialize(fname, -1, globalFbxManager->GetIOSettings())) 
		{
			printf("Call to FbxImporter::Initialize() with %s failed.\n",fname);
			printf("Error returned: %s\n", fbxImporter->GetStatus().GetErrorString());
			DEBUG_BREAK();//delete all
			return 0;
		}

	fbxScene = FbxScene::Create(globalFbxManager,"myScene");

	if(fbxScene)
	{
		printf("importing fbx scene...");
		if(fbxImporter->Import(fbxScene))
		{
			printf("ok\n");

			GetSceneDetails(fbxScene);

			ExtractTexturesandMaterials(fbxScene);

			printf("acquiring fbx nodes structure...\n");
			processNodeRecursive(acquireNodeStructure,fbxScene->GetRootNode(),0);//acquire nodes structure
			processNodeRecursive(acquireNodeData,fbxScene->GetRootNode(),0);//acquire nodes structure
			/*printf("filling acquired structures...\n");
			processNodeRecursive2(fbxScene->GetRootNode(),0);//fill our structure*/
			printf("...ready\n");
		}
		else
			printf("error\n");
	}

	

	if(fbxScene)fbxScene->Destroy(true);
	if(fbxScene)fbxImporter->Destroy(true);
	if(fbxScene)fbxIOSettings->Destroy(true);
	if(fbxScene)globalFbxManager->Destroy();

	globalMapFromNodeToEntity.clear();
	globalMapFromFbxMaterialToMaterial.clear();
	globalMapFromFbxTextureToTexture.clear();

	return globalRootEntity;
}



void StoreKeyframes(float& animation_start,float& animation_end,AnimClip* curvegroup,EChannel channel,FbxAnimCurve* fbxAnimCurve)
{
	if(fbxAnimCurve && fbxAnimCurve->KeyGetCount())
	{	
		KeyCurve *curve=new KeyCurve;

		curve->channel=channel;

		FbxTimeSpan timeSpan;

		fbxAnimCurve->GetTimeInterval(timeSpan);

		curve->start=(float)timeSpan.GetStart().GetSecondDouble();
		curve->end=(float)timeSpan.GetStop().GetSecondDouble();

		curvegroup->start= (curvegroup->start == -1) ? curve->start :  (curve->start < curvegroup->start ? curve->start : curvegroup->start);
		curvegroup->end = (curvegroup->end== -1) ? curve->end : (curve->end > curvegroup->end ? curve->end : curvegroup->end);

		animation_start= (animation_start == -1) ? curvegroup->start :  (curvegroup->start < animation_start ? curvegroup->start : animation_start);
		animation_end = (animation_end== -1) ? curvegroup->end : (curvegroup->end > animation_end ? curvegroup->end : animation_end);

		for(int i=0;i<fbxAnimCurve->KeyGetCount();i++)
		{
			FbxAnimCurveKey fbxKey=fbxAnimCurve->KeyGet(i);
			Keyframe		*keyframe=new Keyframe;
			
			keyframe->time=(float)fbxKey.GetTime().GetSecondDouble();
			keyframe->value=fbxKey.GetValue();

			curve->frames.push_back(keyframe);
		}

		curvegroup->curves.push_back(curve);
	}
}

void FillLight(FbxNode* fbxNode,Light* light)
{
	FbxLight* fbxLight=fbxNode->GetLight();

	if(fbxLight)
	{
		printf("FillLight...\n");

		Light* light=new Light;
			
		light->light_type=(ELight)(int)fbxLight->LightType.Get();
		light->light_decaytype=(EDecay)(int)fbxLight->DecayStart.Get();
			   
		light->light_cast,fbxLight->CastLight.Get();
		light->light_volumetric,fbxLight->DrawVolumetricLight.Get();
		light->light_groundprojection,fbxLight->DrawGroundProjection.Get();
		light->light_shadows,fbxLight->CastShadows.Get();
		light->light_farattenuation,fbxLight->EnableFarAttenuation.Get();
		light->light_nearattenuation,fbxLight->EnableNearAttenuation.Get();
			
		GetVec3(light->light_color,fbxLight->Color.Get());
		GetVec3(light->light_shadowcolor,fbxLight->ShadowColor.Get());
		light->light_intensity,fbxLight->Intensity.Get();
		light->light_innerangle,fbxLight->InnerAngle.Get();
		light->light_outerangle,fbxLight->OuterAngle.Get();
		light->light_fog,fbxLight->Fog.Get();
		light->light_decaystart,fbxLight->DecayStart.Get();
		light->light_nearstart,fbxLight->NearAttenuationStart.Get();
		light->light_nearend,fbxLight->NearAttenuationEnd.Get();
		light->light_farstart,fbxLight->FarAttenuationStart.Get();
		light->light_farend,fbxLight->FarAttenuationEnd.Get();
	}
}


void FillMesh(FbxNode* fbxNode,Mesh* mesh)
{
	if(!fbxNode || !mesh)
		return;

	FbxGeometry* fbxGeometry=fbxNode->GetGeometry();
	FbxMesh* fbxMesh=fbxNode->GetMesh();

	mesh->isCCW=fbxMesh->CheckIfVertexNormalsCCW();

	printf("checking for non-tri polygons...");

	bool non_tri_check=true;

	

	while(non_tri_check)
	{
		fbxGeometry=fbxNode->GetGeometry();
		fbxMesh=fbxNode->GetMesh();

		int pCount=fbxMesh->GetPolygonCount();

		for(int i=0;i<pCount;i++)
		{
			int polygonSize=fbxMesh->GetPolygonSize(i);

			if(polygonSize>3)
			{
				
				printf("error(polygon %d has %d vertices), retriangulate...",i,polygonSize);

				FbxGeometryConverter triangulator(globalFbxManager);
				triangulator.TriangulateInPlace(fbxNode);

				printf("retriangulate ok...recheck...");

				break;
			}

			if(i==pCount-1)
				non_tri_check=false;
		}
	}

	printf("done\n");

	int nLayers=fbxMesh->GetUVLayerCount();

	mesh->npolygons=fbxMesh->GetPolygonCount();
	mesh->ncontrolpoints=fbxMesh->GetControlPointsCount();
	mesh->ntexcoord=fbxMesh->GetTextureUVCount();

#if GENERATE_INDEXED_GEOMETRY
	mesh->ntexcoord=mesh->npolygons*3;
	mesh->nnormals=mesh->npolygons*3;
	mesh->ncontrolpoints=mesh->npolygons*3;
#else
	mesh->nvertexindices=fbxMesh->GetPolygonVertexCount();
	mesh->vertexindices=new unsigned int[mesh->nvertexindices];

	if(!mesh->nvertexindices)
		DEBUG_BREAK();
#endif
	
	mesh->controlpoints=new float[mesh->ncontrolpoints][3];
	mesh->normals=new float[mesh->nnormals][3];
	mesh->texcoord=new float[mesh->ntexcoord][2];

#if DEBUG_PRINTF
	for(int i=0;i<mesh->nvertexindices;i++)
		printf("triIdx[%d]: %d\n",i,mesh->vertexindices[i]);
#endif 

	FbxLayerElementArrayTemplate<FbxVector2> *textureUVs;
	fbxMesh->GetTextureUV(&textureUVs);
	FbxLayerElementUV::EMappingMode mappingMode;

	FbxLayer *fbxLayer=fbxMesh->GetLayer(0);

	if(fbxLayer)
	{
		FbxLayerElementUV* fbxLayerElementUV=fbxLayer->GetUVs();

		if(fbxLayerElementUV)
			mappingMode=fbxLayerElementUV->GetMappingMode();
	}

#if GENERATE_INDEXED_GEOMETRY

	//copy vertices and texcoord

	FbxArray<FbxLayerElement::EType> uvLayers=fbxMesh->GetAllChannelUV(0);

	for(int i=0;i<mesh->npolygons;i++)
	{
		for(int j=0;j<3;j++)
		{
			int dstIdx=i*3+j;

			//mesh->mesh_vertexindices[dstIdx]=fbxMesh->GetPolygonVertex(i,j);

			FbxDouble4 cpoints=fbxMesh->GetControlPointAt(fbxMesh->GetPolygonVertex(i,j));
			Vector::make(mesh->controlpoints[dstIdx],3,(float)cpoints[0],(float)cpoints[1],(float)cpoints[2]);
			
			FbxVector2 uv=textureUVs->GetAt(fbxMesh->GetTextureUVIndex(i,j));
			Vector::make(mesh->texcoord[dstIdx],2,(float)uv[0],1-(float)uv[1]);
		}	
	}

#else
	//copy triangle indices
	memcpy(mesh->vertexindices,fbxMesh->GetPolygonVertices(),sizeof(int)*mesh->nvertexindices);

	//copy indexed vertices
	{
		FbxDouble4* cpoints=fbxMesh->GetControlPoints();

		for(int i=0;i<mesh->ncontrolpoints;i++)
			vector::make(mesh->controlpoints[i],3,(float)cpoints[i][0],(float)cpoints[i][1],(float)cpoints[i][2]);
	}

	for(int i=0;i<mesh->nvertexindices;i++)
	{
		for(int j=0;j<3;j++)
		{
			FbxVector2 &v=textureUVs->GetAt(fbxMesh->GetTextureUVIndex(i,j));
			vector::make(mesh->texcoord[mesh->vertexindices[i]],2,(float)v[0],(float)v[1]);
		}
	}

#endif

	//copy per-vertex normals

	for(int i=0;i<mesh->nvertexindices;i++)
	{
		for(int j=0;j<3;j++)
		{
			FbxVector4 v;
			fbxMesh->GetPolygonVertexNormal(mesh->vertexindices[i],j,v);
			Vector::make(mesh->normals[i],3,(float)v[0],(float)v[1],(float)v[2]);
		}
	}

	
}

void FillSkin(FbxNode* fbxNode,Skin* skin)
{
	if(!fbxNode)
		return;

	FbxMesh* fbxMesh=fbxNode->GetMesh();
	FbxSkin* fbxSkin=(FbxSkin*)fbxNode->GetMesh()->GetDeformer(0);

	if(!fbxMesh)
		return;

	FillMesh(fbxNode,skin);

	if(fbxNode->GetMesh()->GetDeformerCount()>1)
		DEBUG_BREAK();

	skin->nclusters=fbxSkin->GetClusterCount();
	skin->clusters=new Cluster[skin->nclusters];

	for(int i=0;i<skin->nclusters;i++)
	{
		FbxCluster* fbxCluster=fbxSkin->GetCluster(i);

		Cluster& cluster=(Cluster&)skin->clusters[i];


		cluster.bone=globalMapFromNodeToEntity[fbxCluster->GetLink()];

		if(!cluster.bone)
			continue;

		cluster.ninfluences=fbxCluster->GetControlPointIndicesCount();

		FbxAMatrix transform_matrix;
		fbxCluster->GetTransformMatrix(transform_matrix);
		transform_matrix *= GetGeometry(fbxNode);

		FbxAMatrix transform_link_matrix;
		fbxCluster->GetTransformLinkMatrix(transform_link_matrix);

		FbxAMatrix inverse_bind_pose = transform_link_matrix.Inverse() * transform_matrix;

		cluster.offset=GetMatrix(inverse_bind_pose);

		if(!cluster.ninfluences)
			continue;

		cluster.influences=new Influence[cluster.ninfluences];

		double	*weights=fbxCluster->GetControlPointWeights();
		int		*cpidx=fbxCluster->GetControlPointIndices();	

		int* meshIndices=fbxMesh->GetPolygonVertices();

		std::vector< std::vector<int> > finalIndices(cluster.ninfluences);

		for(int i=0;i<cluster.ninfluences;i++)
		{
			for(int j=0;j<fbxMesh->GetPolygonVertexCount();j++)
			{
				if(cpidx[i]==meshIndices[j])
					finalIndices[i].push_back(j);
			}
		}

		for(int influenceIdx=0;influenceIdx<cluster.ninfluences;influenceIdx++)
		{
			Influence& influence=cluster.influences[influenceIdx];

			influence.ncontrolpointindex=(int)finalIndices[influenceIdx].size();

			if(!influence.ncontrolpointindex)
				DEBUG_BREAK();

			influence.controlpointindex=new unsigned int[influence.ncontrolpointindex];

			for(int i=0;i<influence.ncontrolpointindex;i++)
				influence.controlpointindex[i]=finalIndices[influenceIdx][i];

			influence.weight=(float)weights[influenceIdx];
		}
	}
}



void ExtractTexturesandMaterials(FbxScene* lScene)
{
	for(int i=0;i<lScene->GetSrcObjectCount<FbxFileTexture>();i++)
	{
		FbxFileTexture* fbxfiletexture=lScene->GetSrcObject<FbxFileTexture>(i);

		const char* name=fbxfiletexture->GetName();

		TextureFile* texture=new TextureFile;
		texture->filename=StringUtils::ToWide(fbxfiletexture->GetFileName());

		if(texture->load((char*)fbxfiletexture->GetFileName()))
		{
			printf("extracting texture %s %d,%d:%d\n",name,texture->GetWidth(),texture->GetHeight(),texture->GetBpp());
		}
		else
			printf("failed open %s\n",texture->filename.c_str());

		globalMapFromFbxTextureToTexture.insert(std::pair<FbxTexture*,Texture*>(fbxfiletexture,texture));
	}

	for(int i=0;i<lScene->GetSrcObjectCount<FbxLayeredTexture>();i++)
	{
		FbxLayeredTexture* fbxlayeredtexture=lScene->GetSrcObject<FbxLayeredTexture>(i);
		TextureLayered* layeredtexture=new TextureLayered();

		for(int j=0;j<fbxlayeredtexture->GetSrcObjectCount<FbxFileTexture>();j++)
		{
			layeredtexture->textures.push_back((Texture*)globalMapFromFbxTextureToTexture.at(fbxlayeredtexture->GetSrcObject<FbxFileTexture>(j)));
			globalMapFromFbxTextureToTexture.insert(std::pair<FbxTexture*,Texture*>(fbxlayeredtexture,layeredtexture->textures[j]));
		}
	}

	for(int i=0;i<lScene->GetSrcObjectCount<FbxProceduralTexture>();i++)
	{
		TextureProcedural* proceduraltexture=new TextureProcedural;

		globalMapFromFbxTextureToTexture.insert(std::pair<FbxTexture*,Texture*>(lScene->GetSrcObject<FbxProceduralTexture>(i),proceduraltexture));
	}

	////////////

	for(int i=0;i<lScene->GetSrcObjectCount<FbxSurfaceMaterial>();i++)
	{
		FbxSurfaceMaterial* fbxsurfacematerial=lScene->GetSrcObject<FbxSurfaceMaterial>(i);
		FbxSurfaceLambert* sl=(FbxSurfaceLambert*)fbxsurfacematerial;

		if(sl)
		{
			Material* material=new Material;
			
			material->femissive=(float)sl->EmissiveFactor.Get();
			GetVec3(material->emissive,sl->Emissive.Get());
			material->fambient=(float)sl->AmbientFactor.Get();
			GetVec3(material->ambient,sl->Ambient.Get());
			material->fdiffuse=(float)sl->DiffuseFactor.Get();
			GetVec3(material->diffuse,sl->Diffuse.Get());
			GetVec3(material->normalmap,sl->NormalMap.Get());
			GetVec3(material->bump,sl->Bump.Get());
			material->fbump=(float)sl->BumpFactor.Get();
			GetVec3(material->transparent,sl->TransparentColor.Get());
			material->ftransparent=(float)sl->TransparencyFactor.Get();
			GetVec3(material->displacement,sl->DisplacementColor.Get());
			material->fdisplacement=(float)sl->DisplacementFactor.Get();

			if(sl->Is<FbxSurfacePhong>())
			{
				FbxSurfacePhong* sp=(FbxSurfacePhong*)sl;

				material->fspecular=(float)sp->SpecularFactor.Get();
				GetVec3(material->specular,sp->Specular.Get());
				material->fshininess=(float)sp->Shininess.Get();
				GetVec3(material->reflection,sp->Reflection.Get());
				material->freflection=(float)sp->ReflectionFactor.Get();
			}

			globalMapFromFbxMaterialToMaterial.insert(std::pair<FbxSurfaceMaterial*,Material*>(sl,material));

			int foundtextures=0;
			for(int j=0;j<FbxLayerElement::sTypeTextureCount;j++)
			{
				FbxProperty property=fbxsurfacematerial->FindProperty(FbxLayerElement::sTextureChannelNames[j]);

				if (!property.IsValid() && !property.GetSrcObjectCount<FbxTexture>())
					continue;


				for (int k = 0; k < property.GetSrcObjectCount<FbxTexture>(); k++)
				{
					Texture* texture=(Texture*)globalMapFromFbxTextureToTexture.at(property.GetSrcObject<FbxTexture>(k));

					if(texture)
						material->textures.push_back(texture);
				}
			}
		}
	}
}

void GetSceneDetails(FbxScene* lScene)
{
	printf("\n");

	int nmeshes=lScene->GetSrcObjectCount<FbxMesh>();
	int nlights=lScene->GetSrcObjectCount<FbxLight>();
	int ncameras=lScene->GetSrcObjectCount<FbxCamera>();
	int ntextures=lScene->GetSrcObjectCount<FbxTexture>();
	int nfiletextures=lScene->GetSrcObjectCount<FbxFileTexture>();
	int nlayeredtextures=lScene->GetSrcObjectCount<FbxLayeredTexture>();
	int nproceduraltextures=lScene->GetSrcObjectCount<FbxProceduralTexture>();
	int nskeleton=lScene->GetSrcObjectCount<FbxSkeleton>();
	int nskins=lScene->GetSrcObjectCount<FbxSkin>();
	int ngeometries=lScene->GetSrcObjectCount<FbxGeometry>();
	int nsurfacematerials=lScene->GetSrcObjectCount<FbxSurfaceMaterial>();
	int fbxsurfacelambert=lScene->GetSrcObjectCount<FbxSurfaceLambert>();
	int fbxsurfacephong=lScene->GetSrcObjectCount<FbxSurfacePhong>();
	int numanimations=lScene->GetSrcObjectCount<FbxAnimStack>();
	int numanimationlayers=lScene->GetSrcObjectCount<FbxAnimLayer>();
	int numanimcurves=lScene->GetSrcObjectCount<FbxAnimCurve>();
	int numposes=lScene->GetSrcObjectCount<FbxPose>();
	int numcharacters=lScene->GetSrcObjectCount<FbxCharacter>();

	printf("nmeshes: %d\n",nmeshes);
	printf("nlights: %d\n",nlights);
	printf("ncameras: %d\n",ncameras);
	printf("ntextures: %d\n",ntextures);
	printf("nfiletextures: %d\n",nfiletextures);
	printf("nlayeredtextures: %d\n",nlayeredtextures);
	printf("nproceduraltextures: %d\n",nproceduraltextures);
	printf("nskeleton: %d\n",nskeleton);
	printf("nskins: %d\n",nskins);
	printf("ngeometries: %d\n",ngeometries);
	printf("nsurfacematerials: %d\n",nsurfacematerials);
	printf("fbxsurfacelambert: %d\n",fbxsurfacelambert);
	printf("fbxsurfacephong: %d\n",fbxsurfacephong);
	printf("numanimations: %d\n",numanimations);
	printf("numanimationlayers: %d\n",numanimationlayers);
	printf("numanimcurves: %d\n",numanimcurves);
	printf("numposes: %d\n",numposes);
	printf("numcharacters: %d\n",numcharacters);

	printf("\n");
}

/*
BOOL WINAPI DllMain(HINSTANCE,DWORD fdwReason,LPVOID)
{
	switch(fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			wprintf(L"fbximporter loaded\n");
			return TRUE;
		break;

		default:
			wprintf(L"fbximporter DllMain error\n");
	}

	return 1;
}*/

/*
int main()
{

}*/


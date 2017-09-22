#include "fbxutil.h"

#pragma message (LOCATION " mesh needs other polygons type ?")		

#include <map>

FbxManager* fbxManager=0;
const char* sceneFilename=0;

std::map<FbxNode*,Entity*>				mapFromNodeToEntity;
std::map<FbxSurfaceMaterial*,Material*>	mapFromFbxMaterialToMaterial;
std::map<FbxTexture*,Texture*>			mapFromFbxTextureToTexture;

#define GENERATE_MISSING_KEYS 0
#define GENERATE_INDEXED_GEOMETRY 1
#define DEBUG_PRINTF 0

#pragma message (LOCATION " remember to implement generating missing fbx animation keys")

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

void GetVec3(float* b,FbxDouble3 &a)
{
	b[0]=(float)a[0],b[1]=(float)a[1],b[2]=(float)a[2];
}

void GetVec4(float* b,FbxDouble4 &a)
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


Entity* acquireNodeStructure(FbxNode* fbxNode,Entity* parent)
{
	Entity* entity=0;

	Bone* bone=0;

	if(!parent)//create the root and add a child to it
	{
		Entity* rootNode=0;
		EntityComponent* component=0;

		if(!Entity::pool.size())
		{
			rootNode=new Entity;
			rootNode->entity_name="RootNode";
		}
		else
			rootNode=Entity::pool.front();

		entity=new Entity;
		
		parent=rootNode;

		const char* begin=strrchr(sceneFilename,'\\');
		const char* end=strrchr(begin,'.');

		const char* bPtr=begin++;

		int i=0;
		while(++bPtr!=end)i++;

		entity->entity_name=std::string(begin,i).c_str();

		mapFromNodeToEntity.insert(std::pair<FbxNode*,Entity*>(fbxNode,entity));
	
	}
	else
	{
		entity=new Entity;
		mapFromNodeToEntity.insert(std::pair<FbxNode*,Entity*>(fbxNode,entity));
	}
	

	if(entity)
	{
		
		if(!entity->entity_name.Count())
			entity->entity_name=fbxNode->GetName();

		entity->entity_transform=GetMatrix(fbxNode->EvaluateLocalTransform(FBXSDK_TIME_ZERO));

		ExtractAnimations(fbxNode,entity);
		entity->entity_parent=parent;

		if(!entity->entity_parent)
		{
			FbxAxisSystem as=fbxNode->GetScene()->GetGlobalSettings().GetAxisSystem();

			int sign;

			vec3 axes;

			switch(as.GetUpVector(sign))
			{
			case FbxAxisSystem::eXAxis:
				entity->entity_transform.rotate((float)sign * 90,0,0,1);
				printf("upVector is X\n");
				break;
			case FbxAxisSystem::eYAxis:
				printf("upVector is Y\n");
				break;
			case FbxAxisSystem::eZAxis:
				axes=entity->entity_transform.axis(1,0,0);
				entity->entity_transform.rotate((float)sign * 90,axes);
				printf("upVector is Z\n");
				break;
			}
		}

		entity->entity_world = entity->entity_parent ? (entity->entity_transform * entity->entity_parent->entity_world) : entity->entity_transform;
	}

	if(parent && entity)
		parent->entity_childs.push_back(entity);

	return entity;
}

Entity* acquireNodeData(FbxNode* fbxNode,Entity* parent)
{
	Entity* entity=mapFromNodeToEntity[fbxNode];

	if(!entity)
		return 0;

	if(fbxNode->GetSkeleton())
	{
		Bone* bone=entity->CreateComponent<Bone>();

		if(bone)
		{
			if(bone->entity->entity_parent)
			{
				if(bone->entity->entity_parent->findComponent(&EntityComponent::GetBone))
					bone->bone_root=((Bone*)bone->entity->entity_parent->findComponent(&EntityComponent::GetBone))->bone_root;
				else
					bone->bone_root=bone;
			}
		}

		nBonesTotal++;
	}
	else if(fbxNode->GetGeometry())
	{
		int deformerCount=fbxNode->GetGeometry()->GetDeformerCount();

		Skin* skin=0;
		Mesh* mesh=0;

		if(deformerCount)//skin
		{
			skin=entity->CreateComponent<Skin>();
			FillSkin(fbxNode,skin);
		}
		else//mesh
		{
			mesh=entity->CreateComponent<Mesh>();
			FillMesh(fbxNode,mesh);
		}

		for(int i=0;i<fbxNode->GetMaterialCount();i++)
		{
			Material* material=(Material*)mapFromFbxMaterialToMaterial.at(fbxNode->GetMaterial(i));

			if(!material)
				__debugbreak();

			mesh ? mesh->mesh_materials.push_back(material) : skin->mesh_materials.push_back(material);
		}

		fbxNode->GetGeometry()->ComputeBBox();

		FbxDouble3 bbMin=fbxNode->GetGeometry()->BBoxMin;
		FbxDouble3 bbMax=fbxNode->GetGeometry()->BBoxMax;
		
		entity->entity_bbox.a.make((float)bbMin[0],(float)bbMin[1],(float)bbMin[2]);
		entity->entity_bbox.b.make((float)bbMax[0],(float)bbMax[1],(float)bbMax[2]);
	}
	else if(fbxNode->GetLight())
	{
		entity->CreateComponent<Light>();
	}

	return entity;
}


Entity* processNodeRecursive(Entity* (*func)(FbxNode*,Entity*),FbxNode* fbxNode,Entity* parent)
{
	Entity* entity=0;

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
	float& amin=a->animation_start;
	float& amax=a->animation_end;

	for(int i=0;i<(int)a->animation_curvegroups.size();i++)
	{
		float& start=a->animation_curvegroups[i]->curvegroup_start;
		float& end=a->animation_curvegroups[i]->curvegroup_end;

		CurveGroup* g=a->animation_curvegroups[i];

		for(int j=0;j<(int)g->curvegroup_keycurves.size();j++)
		{
				KeyCurve* c=g->curvegroup_keycurves[j];
				if(c->keycurve_start!=start )
					__debugbreak();
				if(c->keycurve_end!=end)
				{
					Keyframe* key=new Keyframe;
					key->time=end;
					key->value=c->keycurve_keyframes[(int)c->keycurve_keyframes.size()-1]->value;
					c->keycurve_keyframes.push_back(key);
				}
		}
	}
}

void ExtractAnimations(FbxNode* fbxNode,Entity* entity)
{
	for(int animStackIdx=0;animStackIdx<fbxNode->GetScene()->GetSrcObjectCount<FbxAnimStack>();animStackIdx++)
	{
		CurveGroup* curvegroup=new CurveGroup;

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

		if(!(int)curvegroup->curvegroup_keycurves.size())
			delete curvegroup;
		else
		{
			Animation* animation=new Animation;
			animation->entity=entity;
			entity->components.push_back(animation);
			animation->animation_curvegroups.push_back(curvegroup);
			animation->animation_start=animStart;
			animation->animation_end=animEnd;
			ParseAnimationCurve(animation);
		}
	}
}

		





void InitFbxSceneLoad(char* fname)
{
	printf("Importing file %s\n",fname);

	FbxIOSettings * fbxIOSettings=0;
	FbxImporter* fbxImporter=0;
	FbxScene* fbxScene=0;

	sceneFilename=fname;

	fbxManager=FbxManager::Create();

	if(fbxManager)
		fbxIOSettings = FbxIOSettings::Create(fbxManager, IOSROOT );

	if(fbxIOSettings)
		fbxManager->SetIOSettings(fbxIOSettings);

	fbxImporter = FbxImporter::Create(fbxManager, "");

	if(fbxImporter)
		if(!fbxImporter->Initialize(fname, -1, fbxManager->GetIOSettings())) 
		{
			printf("Call to FbxImporter::Initialize() with %s failed.\n",fname);
			printf("Error returned: %s\n", fbxImporter->GetStatus().GetErrorString());
			return;
		}

	fbxScene = FbxScene::Create(fbxManager,"myScene");

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
	if(fbxScene)fbxManager->Destroy();

	mapFromNodeToEntity.clear();
	mapFromFbxMaterialToMaterial.clear();
	mapFromFbxTextureToTexture.clear();
}



void StoreKeyframes(float& animation_start,float& animation_end,CurveGroup* curvegroup,EChannel channel,FbxAnimCurve* fbxAnimCurve)
{
	if(fbxAnimCurve && fbxAnimCurve->KeyGetCount())
	{	
		KeyCurve *curve=new KeyCurve;

		curve->keycurve_channel=channel;

		FbxTimeSpan timeSpan;

		fbxAnimCurve->GetTimeInterval(timeSpan);

		curve->keycurve_start=(float)timeSpan.GetStart().GetSecondDouble();
		curve->keycurve_end=(float)timeSpan.GetStop().GetSecondDouble();

		curvegroup->curvegroup_start= (curvegroup->curvegroup_start == -1) ? curve->keycurve_start :  (curve->keycurve_start < curvegroup->curvegroup_start ? curve->keycurve_start : curvegroup->curvegroup_start);
		curvegroup->curvegroup_end = (curvegroup->curvegroup_end== -1) ? curve->keycurve_end : (curve->keycurve_end > curvegroup->curvegroup_end ? curve->keycurve_end : curvegroup->curvegroup_end);

		animation_start= (animation_start == -1) ? curvegroup->curvegroup_start :  (curvegroup->curvegroup_start < animation_start ? curvegroup->curvegroup_start : animation_start);
		animation_end = (animation_end== -1) ? curvegroup->curvegroup_end : (curvegroup->curvegroup_end > animation_end ? curvegroup->curvegroup_end : animation_end);

		for(int i=0;i<fbxAnimCurve->KeyGetCount();i++)
		{
			FbxAnimCurveKey fbxKey=fbxAnimCurve->KeyGet(i);
			Keyframe		*keyframe=new Keyframe;
			
			keyframe->time=(float)fbxKey.GetTime().GetSecondDouble();
			keyframe->value=fbxKey.GetValue();

			curve->keycurve_keyframes.push_back(keyframe);
		}

		curvegroup->curvegroup_keycurves.push_back(curve);
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

	mesh->mesh_isCCW=fbxMesh->CheckIfVertexNormalsCCW();

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

				FbxGeometryConverter triangulator(fbxManager);
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

	mesh->mesh_npolygons=fbxMesh->GetPolygonCount();
	mesh->mesh_ncontrolpoints=fbxMesh->GetControlPointsCount();
	mesh->mesh_nvertexindices=fbxMesh->GetPolygonVertexCount();
	mesh->mesh_ntexcoord=fbxMesh->GetTextureUVCount();

	if(!mesh->mesh_nvertexindices)
		__debugbreak();

#if GENERATE_INDEXED_GEOMETRY
	mesh->mesh_ntexcoord=mesh->mesh_npolygons*3;
	mesh->mesh_nnormals=mesh->mesh_npolygons*3;
	mesh->mesh_ncontrolpoints=mesh->mesh_npolygons*3;
#endif
	
	mesh->mesh_controlpoints=new float[mesh->mesh_ncontrolpoints][3];
	mesh->mesh_vertexindices=new unsigned int[mesh->mesh_nvertexindices];
	mesh->mesh_normals=new float[mesh->mesh_nnormals][3];
	mesh->mesh_texcoord=new float[mesh->mesh_ntexcoord][2];

#if DEBUG_PRINTF
	for(int i=0;i<mesh->mesh_nvertexindices;i++)
		printf("triIdx[%d]: %d\n",i,mesh->mesh_vertexindices[i]);
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

	for(int i=0;i<mesh->mesh_npolygons;i++)
	{
		for(int j=0;j<3;j++)
		{
			int dstIdx=i*3+j;

			//mesh->mesh_vertexindices[dstIdx]=fbxMesh->GetPolygonVertex(i,j);

			FbxDouble4 cpoints=fbxMesh->GetControlPointAt(fbxMesh->GetPolygonVertex(i,j));
			VectorMathNamespace::make(mesh->mesh_controlpoints[dstIdx],3,(float)cpoints[0],(float)cpoints[1],(float)cpoints[2]);
			
			FbxVector2 uv=textureUVs->GetAt(fbxMesh->GetTextureUVIndex(i,j));
			VectorMathNamespace::make(mesh->mesh_texcoord[dstIdx],2,(float)uv[0],1-(float)uv[1]);
		}	
	}

#else
	//copy triangle indices
	memcpy(mesh->mesh_vertexindices,fbxMesh->GetPolygonVertices(),sizeof(int)*mesh->mesh_nvertexindices);

	//copy indexed vertices
	{
		FbxDouble4* cpoints=fbxMesh->GetControlPoints();

		for(int i=0;i<mesh->mesh_ncontrolpoints;i++)
			vector::make(mesh->mesh_controlpoints[i],3,(float)cpoints[i][0],(float)cpoints[i][1],(float)cpoints[i][2]);
	}

	for(int i=0;i<mesh->mesh_nvertexindices;i++)
	{
		for(int j=0;j<3;j++)
		{
			FbxVector2 &v=textureUVs->GetAt(fbxMesh->GetTextureUVIndex(i,j));
			vector::make(mesh->mesh_texcoord[mesh->mesh_vertexindices[i]],2,(float)v[0],(float)v[1]);
		}
	}

#endif

	//copy per-vertex normals

	for(int i=0;i<mesh->mesh_nvertexindices;i++)
	{
		for(int j=0;j<3;j++)
		{
			FbxVector4 v;
			fbxMesh->GetPolygonVertexNormal(mesh->mesh_vertexindices[i],j,v);
			VectorMathNamespace::make(mesh->mesh_normals[i],3,(float)v[0],(float)v[1],(float)v[2]);
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
		__debugbreak();

	skin->skin_nclusters=fbxSkin->GetClusterCount();
	skin->skin_clusters=new Cluster[skin->skin_nclusters];

	for(int i=0;i<skin->skin_nclusters;i++)
	{
		FbxCluster* fbxCluster=fbxSkin->GetCluster(i);

		Cluster& cluster=(Cluster&)skin->skin_clusters[i];


		cluster.cluster_bone=mapFromNodeToEntity[fbxCluster->GetLink()];

		if(!cluster.cluster_bone)
			continue;

		cluster.cluster_ninfluences=fbxCluster->GetControlPointIndicesCount();

		FbxAMatrix transform_matrix;
		fbxCluster->GetTransformMatrix(transform_matrix);
		transform_matrix *= GetGeometry(fbxNode);

		FbxAMatrix transform_link_matrix;
		fbxCluster->GetTransformLinkMatrix(transform_link_matrix);

		FbxAMatrix inverse_bind_pose = transform_link_matrix.Inverse() * transform_matrix;

		cluster.cluster_offset=GetMatrix(inverse_bind_pose);

		if(!cluster.cluster_ninfluences)
			continue;

		cluster.cluster_influences=new Influence[cluster.cluster_ninfluences];

		double	*weights=fbxCluster->GetControlPointWeights();
		int		*cpidx=fbxCluster->GetControlPointIndices();	

		int* meshIndices=fbxMesh->GetPolygonVertices();

		std::vector< std::vector<int> > finalIndices(cluster.cluster_ninfluences);

		for(int i=0;i<cluster.cluster_ninfluences;i++)
		{
			for(int j=0;j<fbxMesh->GetPolygonVertexCount();j++)
			{
				if(cpidx[i]==meshIndices[j])
					finalIndices[i].push_back(j);
			}
		}

		for(int influenceIdx=0;influenceIdx<cluster.cluster_ninfluences;influenceIdx++)
		{
			Influence& influence=cluster.cluster_influences[influenceIdx];

			influence.influence_ncontrolpointindex=(int)finalIndices[influenceIdx].size();

			if(!influence.influence_ncontrolpointindex)
				__debugbreak();

			influence.influence_controlpointindex=new int[influence.influence_ncontrolpointindex];

			for(int i=0;i<influence.influence_ncontrolpointindex;i++)
				influence.influence_controlpointindex[i]=finalIndices[influenceIdx][i];

			influence.influence_weight=(float)weights[influenceIdx];
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
		texture->filename=fbxfiletexture->GetFileName();

		if(texture->load(texture->filename))
		{
			printf("extracting texture %s %d,%d:%d\n",name,texture->GetWidth(),texture->GetHeight(),texture->GetBpp());
		}
		else
			printf("failed open %s\n",texture->filename);

		mapFromFbxTextureToTexture.insert(std::pair<FbxTexture*,Texture*>(fbxfiletexture,texture));
	}

	for(int i=0;i<lScene->GetSrcObjectCount<FbxLayeredTexture>();i++)
	{
		FbxLayeredTexture* fbxlayeredtexture=lScene->GetSrcObject<FbxLayeredTexture>(i);
		TextureLayered* layeredtexture=new TextureLayered();

		for(int j=0;j<fbxlayeredtexture->GetSrcObjectCount<FbxFileTexture>();j++)
		{
			layeredtexture->textures.push_back((Texture*)mapFromFbxTextureToTexture.at(fbxlayeredtexture->GetSrcObject<FbxFileTexture>(j)));
			mapFromFbxTextureToTexture.insert(std::pair<FbxTexture*,Texture*>(fbxlayeredtexture,layeredtexture->textures[j]));
		}
	}

	for(int i=0;i<lScene->GetSrcObjectCount<FbxProceduralTexture>();i++)
	{
		TextureProcedural* proceduraltexture=new TextureProcedural;

		mapFromFbxTextureToTexture.insert(std::pair<FbxTexture*,Texture*>(lScene->GetSrcObject<FbxProceduralTexture>(i),proceduraltexture));
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

			mapFromFbxMaterialToMaterial.insert(std::pair<FbxSurfaceMaterial*,Material*>(sl,material));

			int foundtextures=0;
			for(int j=0;j<FbxLayerElement::sTypeTextureCount;j++)
			{
				FbxProperty property=fbxsurfacematerial->FindProperty(FbxLayerElement::sTextureChannelNames[j]);

				if (!property.IsValid() && !property.GetSrcObjectCount<FbxTexture>())
					continue;


				for (int k = 0; k < property.GetSrcObjectCount<FbxTexture>(); k++)
				{
					Texture* texture=(Texture*)mapFromFbxTextureToTexture.at(property.GetSrcObject<FbxTexture>(k));

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


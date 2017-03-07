#include "fbxutil.h"

#pragma message (LOCATION " mesh needs other polygons type ?")		

#include <map>

FbxManager* lSdkManager=0;

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


Entity* processMapFbxToEntityFunc(FbxNode* fbxNode,Entity* parent)
{
	Entity* entity=0;

	if(!parent)
	{
		entity=new Entity();
		mapFromNodeToEntity.insert(std::pair<FbxNode*,Entity*>(fbxNode,entity));
	}
	else if(fbxNode->GetSkeleton())
	{
		entity=new Bone();
		//entity=bone;
		mapFromNodeToEntity.insert(std::pair<FbxNode*,Entity*>(fbxNode,entity));
		nBonesTotal++;
	}
	else if(fbxNode->GetGeometry())
	{
		int deformerCount=fbxNode->GetGeometry()->GetDeformerCount();

		if(deformerCount)//skin
		{
			Skin* skin=new Skin();
			entity=skin;
			mapFromNodeToEntity.insert(std::pair<FbxNode*,Entity*>(fbxNode,entity));
		}
		else//mesh
		{
			Mesh* mesh=new Mesh;
			entity=mesh;
			mapFromNodeToEntity.insert(std::pair<FbxNode*,Entity*>(fbxNode,entity));
		}

		fbxNode->GetGeometry()->ComputeBBox();

		FbxDouble3 bbMin=fbxNode->GetGeometry()->BBoxMin;
		FbxDouble3 bbMax=fbxNode->GetGeometry()->BBoxMax;
		
		entity->entity_bbox.a.make((float)bbMin[0],(float)bbMin[1],(float)bbMin[2]);
		entity->entity_bbox.b.make((float)bbMax[0],(float)bbMax[1],(float)bbMax[2]);
	}
	else if(fbxNode->GetLight())
	{
		Light* light=new Light();
		entity=light;
		mapFromNodeToEntity.insert(std::pair<FbxNode*,Entity*>(fbxNode,entity));
	}
	else
	{
		entity=new Entity();
		mapFromNodeToEntity.insert(std::pair<FbxNode*,Entity*>(fbxNode,entity));
	}

	if(entity)
	{
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

		//set root bone if bone
		if(entity->GetBone())
		{
			if(entity->entity_parent)
			{
				Bone* bone=entity->GetBone();

				if(bone->entity_parent->GetBone())
					bone->bone_root=bone->entity_parent->GetBone();
				else
					bone->bone_root=bone;
			}
		}
	}

	if(parent && entity)
		parent->entity_childs.push_back(entity);

	return entity;
}

Entity* processExtractSceneFunc(FbxNode* fbxNode,Entity* parent)
{
	Entity* entity=0;

	if(!parent)
		entity=mapFromNodeToEntity.at(fbxNode);
	else if(fbxNode->GetSkeleton())
	{
		Bone* bone=(Bone*)mapFromNodeToEntity.at(fbxNode);
		entity=(Bone*)bone;
	}
	else if(fbxNode->GetGeometry())
	{
		int deformerCount=fbxNode->GetGeometry()->GetDeformerCount();

		Skin* skin=0;
		Mesh* mesh=0;

		if(deformerCount)//skin
		{
			skin=(Skin*)mapFromNodeToEntity.at(fbxNode);
			FillSkin(fbxNode,skin);
			entity=skin;
		}
		else//mesh
		{
			mesh=(Mesh*)mapFromNodeToEntity.at(fbxNode);
			FillMesh(fbxNode,mesh);
			entity=mesh;
		}

		for(int i=0;i<fbxNode->GetMaterialCount();i++)
		{
			Material* material=(Material*)mapFromFbxMaterialToMaterial.at(fbxNode->GetMaterial(i));

			if(!material)
				__debugbreak();

			mesh ? mesh->mesh_materials.push_back(material) : skin->mesh_materials.push_back(material);
		}
	}
	else if(fbxNode->GetLight())
	{
		Light* light=(Light*)mapFromNodeToEntity.at(fbxNode);
		FillLight(fbxNode,light);
		entity=(Light*)light;
	}
	else
	{
		entity=mapFromNodeToEntity.at(fbxNode);
	}

	return entity;
}



Entity* processNodeRecursive1(FbxNode* fbxNode,Entity* parent)

{
	Entity* entity=0;

	if(!fbxNode)
		return entity;

	entity=processMapFbxToEntityFunc(fbxNode,parent);

	int nChilds=fbxNode->GetChildCount();

	for(int i=0;i<nChilds;i++)
		Entity* child=processNodeRecursive1(fbxNode->GetChild(i),entity);

	return entity;
}

Entity* processNodeRecursive2(FbxNode* fbxNode,Entity* parent)

{
	Entity* entity=0;

	if(!fbxNode)
		return entity;

	entity=processExtractSceneFunc(fbxNode,parent);

	int nChilds=fbxNode->GetChildCount();

	for(int i=0;i<nChilds;i++)
		Entity* child=processNodeRecursive2(fbxNode->GetChild(i),entity);

	return entity;
}

void ParseAnimationCurve(Animation* a)
{
	float& amin=a->animation_start;
	float& amax=a->animation_end;

	for(int i=0;i<a->animation_curvegroups.Count();i++)
	{
		float& start=a->animation_curvegroups[i]->curvegroup_start;
		float& end=a->animation_curvegroups[i]->curvegroup_end;

		CurveGroup* g=a->animation_curvegroups[i];

		for(int j=0;j<g->curvegroup_keycurves.Count();j++)
		{
				KeyCurve* c=g->curvegroup_keycurves[j];
				if(c->keycurve_start!=start )
					__debugbreak();
				if(c->keycurve_end!=end)
				{
					Keyframe* key=new Keyframe;
					key->time=end;
					key->value=c->keycurve_keyframes[c->keycurve_keyframes.Count()-1]->value;
					c->keycurve_keyframes.Append(key);
				}
		}
	}
}

void ExtractAnimations(FbxNode* fbxNode,Entity* entity)
{
	for(int animStackIdx=0;animStackIdx<fbxNode->GetScene()->GetSrcObjectCount<FbxAnimStack>();animStackIdx++)
	{
		CurveGroup* curvegroup=new CurveGroup;
		
		for(int animLayerIdx=0;animLayerIdx<fbxNode->GetScene()->GetSrcObject<FbxAnimStack>(animStackIdx)->GetSrcObjectCount<FbxAnimLayer>();animLayerIdx++)
		{
			FbxAnimLayer* layer=fbxNode->GetScene()->GetSrcObject<FbxAnimStack>(animStackIdx)->GetSrcObject<FbxAnimLayer>(animLayerIdx);

			StoreKeyframes(entity,curvegroup,TRANSLATEX,fbxNode->LclTranslation.GetCurve(layer,FBXSDK_CURVENODE_COMPONENT_X));
			StoreKeyframes(entity,curvegroup,TRANSLATEY,fbxNode->LclTranslation.GetCurve(layer,FBXSDK_CURVENODE_COMPONENT_Y));
			StoreKeyframes(entity,curvegroup,TRANSLATEZ,fbxNode->LclTranslation.GetCurve(layer,FBXSDK_CURVENODE_COMPONENT_Z));
			StoreKeyframes(entity,curvegroup,ROTATEX,fbxNode->LclRotation.GetCurve(layer,FBXSDK_CURVENODE_COMPONENT_X));
			StoreKeyframes(entity,curvegroup,ROTATEY,fbxNode->LclRotation.GetCurve(layer,FBXSDK_CURVENODE_COMPONENT_Y));
			StoreKeyframes(entity,curvegroup,ROTATEZ,fbxNode->LclRotation.GetCurve(layer,FBXSDK_CURVENODE_COMPONENT_Z));
			StoreKeyframes(entity,curvegroup,SCALEX,fbxNode->LclScaling.GetCurve(layer,FBXSDK_CURVENODE_COMPONENT_X));
			StoreKeyframes(entity,curvegroup,SCALEY,fbxNode->LclScaling.GetCurve(layer,FBXSDK_CURVENODE_COMPONENT_Y));
			StoreKeyframes(entity,curvegroup,SCALEZ,fbxNode->LclScaling.GetCurve(layer,FBXSDK_CURVENODE_COMPONENT_Z));

			
			//DONTCANCEL AnimationTake(layer,fbxNode,animation,animname);
		}

		if(!curvegroup->curvegroup_keycurves.Count())
			delete curvegroup;
		else
		{
			entity->animation_curvegroups.Append(curvegroup);
			ParseAnimationCurve(entity);
		}
	}
}

		





FbxScene* InitFbxSceneLoad(char* fname)
{
	printf("Importing file %s\n",fname);

	lSdkManager=FbxManager::Create();

	FbxIOSettings * ios = FbxIOSettings::Create(lSdkManager, IOSROOT );
	lSdkManager->SetIOSettings(ios);

	FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

	bool lImportStatus = lImporter->Initialize(fname, -1, lSdkManager->GetIOSettings());

	if(!lImportStatus) {
		printf("Call to FbxImporter::Initialize() with %s failed.\n",fname);
		printf("Error returned: %s\n", lImporter->GetStatus().GetErrorString());
		return NULL;
	}

	FbxScene* lScene = FbxScene::Create(lSdkManager,"myScene");

	printf("importing fbx scene...");
	lImporter->Import(lScene);
	printf("ok\n");

	GetSceneDetails(lScene);

	return lScene;
}



void StoreKeyframes(Animation* animation,CurveGroup* curvegroup,EChannel channel,FbxAnimCurve* fbxAnimCurve)
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

		animation->animation_start= (animation->animation_start == -1) ? curvegroup->curvegroup_start :  (curvegroup->curvegroup_start < animation->animation_start ? curvegroup->curvegroup_start : animation->animation_start);
		animation->animation_end = (animation->animation_end== -1) ? curvegroup->curvegroup_end : (curvegroup->curvegroup_end > animation->animation_end ? curvegroup->curvegroup_end : animation->animation_end);

		for(int i=0;i<fbxAnimCurve->KeyGetCount();i++)
		{
			FbxAnimCurveKey fbxKey=fbxAnimCurve->KeyGet(i);
			Keyframe		*keyframe=new Keyframe;
			
			keyframe->time=(float)fbxKey.GetTime().GetSecondDouble();
			keyframe->value=fbxKey.GetValue();

			curve->keycurve_keyframes.Append(keyframe);
		}

		curvegroup->curvegroup_keycurves.Append(curve);
		if(curve->keycurve_keyframes.Count()>1000)
			__debugbreak();
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

				FbxGeometryConverter triangulator(lSdkManager);
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

		cluster.cluster_bone=(Bone*)mapFromNodeToEntity[fbxCluster->GetLink()];

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

		TDAutoArray< TDAutoArray<int> > finalIndices(cluster.cluster_ninfluences);

		for(int i=0;i<cluster.cluster_ninfluences;i++)
		{
			for(int j=0;j<fbxMesh->GetPolygonVertexCount();j++)
			{
				if(cpidx[i]==meshIndices[j])
					finalIndices[i].Append(j);
			}
		}

		for(int influenceIdx=0;influenceIdx<cluster.cluster_ninfluences;influenceIdx++)
		{
			Influence& influence=cluster.cluster_influences[influenceIdx];

			influence.influence_ncontrolpointindex=finalIndices[influenceIdx].Count();

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

		if(!texture->load(texture->filename))
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

	
	ExtractTexturesandMaterials(lScene);

	printf("acquiring fbx nodes structure...\n");
	processNodeRecursive1(lScene->GetRootNode(),0);//acquire nodes structure
	printf("filling acquired structures...\n");
	processNodeRecursive2(lScene->GetRootNode(),0);//fill our structure

	mapFromNodeToEntity.clear();
	mapFromFbxMaterialToMaterial.clear();
	mapFromFbxTextureToTexture.clear();

	printf("...ready\n");
}


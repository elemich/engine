#ifndef __FBXUTIL__HEADER__
#define __FBXUTIL__HEADER__

#include "entities.h"

#include <fbxsdk.h>
#include <fbxsdk\fileio\fbxiosettings.h>

#include <map>

struct cmat
{
	vec3 a,b,c;

	cmat(vec3 x,vec3 y,vec3 z)
	{
		a=x,b=y,c=z;
	}
};

void InitFbxSceneLoad(char* fname);

void FillSkin(FbxNode* fbxNode,Skin* skin);
void FillMesh(FbxNode* fbxNode,Mesh* mesh);
void FillMaterial(FbxNode*,Material*);
void FillLight(FbxNode* fbxNode,Light* light);

void StoreKeyframes(Animation*,CurveGroup* animation,EChannel channel,FbxAnimCurve* fbxAnimCurve);
void ExtractAnimations(FbxNode*,Entity*);
void ExtractTexturesandMaterials(FbxScene*);
void GetSceneDetails(FbxScene*);
FbxNode* ExtractSkeleton(FbxNode* fbxNode,Skeleton* skeleton,BoneSkeleton* boneParent,std::map<FbxSkeleton*,BoneSkeleton*>& boneMap,std::vector<FbxGeometry*>& geometries);

bool AnimationPresent(FbxAnimLayer * pAnimLayer, FbxNode* pNode);
bool AnimationTake(FbxAnimLayer* pAnimLayer,FbxNode* pNode,Animation* animation,String &animnae);

bool processMapFbxToEntityFunc(FbxNode* fbxNode);//return true if must process child (in case
void processExtractSceneFunc(FbxNode* fbxNode);



#endif __FBXUTIL__HEADER__
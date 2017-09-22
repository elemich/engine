#ifndef __FBXUTIL__HEADER__
#define __FBXUTIL__HEADER__

#include "entities.h"

#include <fbxsdk.h>
#include <fbxsdk\fileio\fbxiosettings.h>



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

void StoreKeyframes(float& animationStart,float& animationEnd,CurveGroup* animation,EChannel channel,FbxAnimCurve* fbxAnimCurve);
void ExtractAnimations(FbxNode*,Entity*);
void ExtractTexturesandMaterials(FbxScene*);
void GetSceneDetails(FbxScene*);

bool AnimationPresent(FbxAnimLayer * pAnimLayer, FbxNode* pNode);
bool AnimationTake(FbxAnimLayer* pAnimLayer,FbxNode* pNode,Animation* animation,String &animnae);

Entity* acquireNodeStructure(FbxNode* fbxNode,Entity* parent);
Entity* acquireNodeData(FbxNode* fbxNode,Entity* parent);

Entity* processNodeRecursive(Entity* (*func)(FbxNode*,Entity*),FbxNode* fbxNode,Entity* parent);



#endif __FBXUTIL__HEADER__
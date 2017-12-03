#include "fbxutil.h"

#include <set>

std::set<FbxTime> timeSet;

bool AnimationPresent(FbxAnimLayer * pAnimLayer, FbxNode* pNode)
{
	///////////// TRANS
	{

    FbxAnimCurve *pCurveTX = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);

		if (pCurveTX)
		{
			int lKeyCountTX = pCurveTX->KeyGetCount();
			if (lKeyCountTX > 0)
				return true;
		}
	}
	{
    FbxAnimCurve *pCurveTY = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if (pCurveTY)
		{
			int lKeyCountTY = pCurveTY->KeyGetCount();
			if (lKeyCountTY > 0)
				return true;
		}
	}
	{
    FbxAnimCurve *pCurveTZ = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if (pCurveTZ)
		{
			int lKeyCountTZ = pCurveTZ->KeyGetCount();
			if (lKeyCountTZ > 0)
				return true;
		}     
	}
	/////////// SCALE
	{
    FbxAnimCurve*  pCurveSX = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
		if (pCurveSX)
		{
			int lKeyCountSX = pCurveSX->KeyGetCount();
			if (lKeyCountSX > 0)
				return true;
		}
	}
	{
    FbxAnimCurve*  pCurveSY = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if (pCurveSY)
		{
			int lKeyCountSY = pCurveSY->KeyGetCount();
			if (lKeyCountSY > 0)
				return true;
		}
	}
	{
    FbxAnimCurve*  pCurveSZ = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if (pCurveSZ)
		{
			int lKeyCountSZ = pCurveSZ->KeyGetCount();
			if (lKeyCountSZ > 0)
				return true;   
		}
	}
	/////////// ROTATE
	{
    FbxAnimCurve* pCurveRX = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
		if (pCurveRX)
		{
			int lKeyCountRX = pCurveRX->KeyGetCount();

			if (lKeyCountRX > 0)
				return true;
		}
	}
	{
    FbxAnimCurve* pCurveRY = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if (pCurveRY)
		{
			int lKeyCountRY = pCurveRY->KeyGetCount();

			if (lKeyCountRY > 0)
				return true;
		}
	}
	{
    FbxAnimCurve* pCurveRZ = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if (pCurveRZ)
		{
			int lKeyCountRZ = pCurveRZ->KeyGetCount();
			if (lKeyCountRZ > 0)
				return true;
		}   
	}
	return false;
}

struct  lpFloat3TimeKey
{      
	float time;
	vec3 value;
}; 


bool AnimationTake(FbxAnimLayer* pAnimLayer,FbxNode* pNode,Animation* animation,String &animnae)
{
	//animationKeySet.clear();
	//const char * pTakeName = pAnimLayer->GetName();
	const char * pNodeName = pNode->GetName();


  bool bAnimationPresent = AnimationPresent(pAnimLayer, pNode);

  if(!bAnimationPresent)
	  return false;


	//scene()->SetCurrentTake(pAnimLayer->GetName());

	FbxTakeInfo* lCurrentTakeInfo = pNode->GetScene()->GetTakeInfo(animnae.Buf());
	FbxTime tStart, tStop;

	if (lCurrentTakeInfo)
	{
		tStart = lCurrentTakeInfo->mLocalTimeSpan.GetStart();
		tStop = lCurrentTakeInfo->mLocalTimeSpan.GetStop();
	}
	else
	{
		// Take the time line value
		//FbxTimeSpan lTimeLineTimeSpan;
		//pNode->GetScene()->GlobalTimeSettings().GetTimelineDefautTimeSpan(lTimeLineTimeSpan);

		//tStart = lTimeLineTimeSpan.GetStart();
		//tStop  = lTimeLineTimeSpan.GetStop();

    // filled in by animation
		tStart.SetSecondDouble(FLT_MAX);
		tStop.SetSecondDouble(-FLT_MAX);
	} 

  FbxTime::EMode pTimeMode = pNode->GetScene()->GetGlobalSettings().GetTimeMode();
  double frameRate = FbxTime::GetFrameRate(pTimeMode);

  std::set<FbxTime> timeSet;

  FbxAnimCurve* pCurveRX = 0;
  FbxAnimCurve* pCurveRY = 0;
  FbxAnimCurve* pCurveRZ = 0;

  FbxAnimCurve* pCurveSX = 0;
  FbxAnimCurve* pCurveSY = 0;
  FbxAnimCurve* pCurveSZ = 0;

  FbxAnimCurve* pCurveTX = 0;
  FbxAnimCurve* pCurveTY = 0;
  FbxAnimCurve* pCurveTZ = 0;


  ///////
  ////// TRANS
  {
    pCurveTX = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
    if (pCurveTX)
    {
      int lKeyCountTX = pCurveTX->KeyGetCount();

      for(int lCount = 0; lCount < lKeyCountTX; lCount++)
      {
        FbxTime   lKeyTime = pCurveTX->KeyGetTime(lCount);    
        if (lKeyTime < tStart)
          tStart = lKeyTime;

        if (lKeyTime > tStop)
          tStop = lKeyTime;

        timeSet.insert(lKeyTime);
      }
    }
  }
  {
    pCurveTY = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
    if (pCurveTY)
    {
      int lKeyCountTY = pCurveTY->KeyGetCount();

      for(int lCount = 0; lCount < lKeyCountTY; lCount++)
      {
        FbxTime   lKeyTime = pCurveTY->KeyGetTime(lCount);    
        if (lKeyTime < tStart)
          tStart = lKeyTime;

        if (lKeyTime > tStop)
          tStop = lKeyTime;
        timeSet.insert(lKeyTime);


      }
    }
  }
  {
    pCurveTZ = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
    if (pCurveTZ)
    {
      int lKeyCountTZ = pCurveTZ->KeyGetCount();

      for(int lCount = 0; lCount < lKeyCountTZ; lCount++)
      {
        FbxTime   lKeyTime = pCurveTZ->KeyGetTime(lCount);   
        if (lKeyTime < tStart)
          tStart = lKeyTime;

        if (lKeyTime > tStop)
          tStop = lKeyTime;

        timeSet.insert(lKeyTime);
      }
    }     
  }
  /////////// SCALE
  {
    pCurveSX = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
    if (pCurveSX)
    {
      int lKeyCountSX = pCurveSX->KeyGetCount();

      for(int lCount = 0; lCount < lKeyCountSX; lCount++)
      {
        FbxTime   lKeyTime = pCurveSX->KeyGetTime(lCount);    
        if (lKeyTime < tStart)
          tStart = lKeyTime;

        if (lKeyTime > tStop)
          tStop = lKeyTime;

        timeSet.insert(lKeyTime);
      }
    }
  }
  {
    pCurveSY = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
    if (pCurveSY)
    {
      int lKeyCountSY = pCurveSY->KeyGetCount();

      for(int lCount = 0; lCount < lKeyCountSY; lCount++)
      {
        FbxTime   lKeyTime = pCurveSY->KeyGetTime(lCount);    
        if (lKeyTime < tStart)
          tStart = lKeyTime;

        if (lKeyTime > tStop)
          tStop = lKeyTime;

        timeSet.insert(lKeyTime);        
      }
    }
  }
  {
    pCurveSZ = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
    if (pCurveSZ)
    {
      int lKeyCountSZ = pCurveSZ->KeyGetCount();

      for(int lCount = 0; lCount < lKeyCountSZ; lCount++)
      {
        FbxTime   lKeyTime = pCurveSZ->KeyGetTime(lCount);    
        if (lKeyTime < tStart)
          tStart = lKeyTime;

        if (lKeyTime > tStop)
          tStop = lKeyTime;

        timeSet.insert(lKeyTime);        
      }     
    }
  }
  /////////// ROTATE
  {
    pCurveRX = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);

    if (pCurveRX)
    {
      int lKeyCountRX = pCurveRX->KeyGetCount();

      for(int lCount = 0; lCount < lKeyCountRX; lCount++)
      {
        FbxTime   lKeyTime = pCurveRX->KeyGetTime(lCount);    
        float lKeyValue = static_cast<float>(pCurveRX->KeyGetValue(lCount));

        if (lKeyTime < tStart)
          tStart = lKeyTime;

        if (lKeyTime > tStop)
          tStop = lKeyTime;

        timeSet.insert(lKeyTime);


      }
    }
  }
  {
    pCurveRY = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
    if (pCurveRY)
    {
      int lKeyCountRY = pCurveRY->KeyGetCount();

      for(int lCount = 0; lCount < lKeyCountRY; lCount++)
      {
        FbxTime   lKeyTime = pCurveRY->KeyGetTime(lCount);    
        float lKeyValue = static_cast<float>(pCurveRY->KeyGetValue(lCount));

        if (lKeyTime < tStart)
          tStart = lKeyTime;

        if (lKeyTime > tStop)
          tStop = lKeyTime;

        timeSet.insert(lKeyTime);        

      }
    }
  }
  {
    pCurveRZ = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
    if (pCurveRZ)
    {
      int lKeyCountRZ = pCurveRZ->KeyGetCount();

      for(int lCount = 0; lCount < lKeyCountRZ; lCount++)
      {
        FbxTime lKeyTime = pCurveRZ->KeyGetTime(lCount);    
        float lKeyValue = static_cast<float>(pCurveRZ->KeyGetValue(lCount));

        if (lKeyTime < tStart)
          tStart = lKeyTime;

        if (lKeyTime > tStop)
          tStop = lKeyTime;
        timeSet.insert(lKeyTime);   

      }  
    }   
  }



  double timeStartSecondsTimesRate = tStart.GetSecondDouble() * frameRate;
  double timeStopSecondsTimesRate = tStop.GetSecondDouble() * frameRate;  

 /* m_pAnimController->m_frameRate = frameRate;
  if (timeStartSecondsTimesRate < m_pAnimController->m_timeStartSecondsTimesRate)
    m_pAnimController->m_timeStartSecondsTimesRate = timeStartSecondsTimesRate;

  if (timeStopSecondsTimesRate > m_pAnimController->m_timeStopSecondsTimesRate)
    m_pAnimController->m_timeStopSecondsTimesRate = timeStopSecondsTimesRate;*/

  /////////////////////////
  timeSet.insert(tStart);
  timeSet.insert(tStop);


  std::set<FbxTime>::iterator it;

  const char * name = pNode->GetName();


  FbxVector4 lastRot;

	FbxVector4 localT, localR, localS;

  
  localT  = pNode->LclTranslation.Get();
  localR  = pNode->LclRotation.Get();
  localS  = pNode->LclScaling.Get();

  std::vector<lpFloat3TimeKey*> aTransKey;
  std::vector<lpFloat3TimeKey*> aScaleKey;
  std::vector<lpFloat3TimeKey*> aRotKey;


#define FOR_EACH(list, it)  \
	for (it = (list).begin(); it != (list).end(); ++it)

	FOR_EACH(timeSet, it)
	{
		const FbxTime &pTime = *it;

  
    FbxAMatrix localMatrix = pNode->GetScene()->GetEvaluator()->GetNodeLocalTransform(pNode, pTime);
    FbxVector4 _scale = localMatrix.GetS();
	vec3 scale((float)_scale[0],(float)_scale[1],(float)_scale[2]);


		lpFloat3TimeKey *scaleKey=new lpFloat3TimeKey;

		float floatTime=(float)(pTime.GetSecondDouble() * frameRate);

		scaleKey->time = floatTime;   
		scaleKey->value.make(scale[0],scale[1],scale[2]);

		aScaleKey.push_back(scaleKey);

    FbxVector4 _trans = localMatrix.GetT();
    lpFloat3TimeKey *transKey=new lpFloat3TimeKey;

	vec3 trans((float)_trans[0],(float)_trans[1],(float)_trans[2]);

		transKey->time = floatTime;   

		if (_isnan(trans[0]) || !_finite(trans[0]))
			trans[0] = (float)localT[0];
		if (_isnan(trans[1]) || !_finite(trans[1]))
		  trans[1] = (float)localT[1];
		if (_isnan(trans[2]) || !_finite(trans[2]))
		  trans[2] = (float)localT[2];

		transKey->value.make(trans[0],trans[1],trans[2]);

		aTransKey.push_back(transKey);
    
		FbxVector4 _localRot = localMatrix.GetR();

		vec3 localRot((float)_localRot[0],(float)_localRot[1],(float)_localRot[2]);

		lpFloat3TimeKey *rotKey=new lpFloat3TimeKey;
		rotKey->time = floatTime;   
		rotKey->value.make(localRot[0],localRot[1],localRot[2]);

		aRotKey.push_back(rotKey);
	}
#if GENERATE_MISSING_KEYS

	KeyCurve *curve=new KeyCurve[9];

	curve[0].keycurve_channel=KeyCurve::TRANSLATEX;
	curve[1].keycurve_channel=KeyCurve::TRANSLATEY;
	curve[2].keycurve_channel=KeyCurve::TRANSLATEZ;
	curve[3].keycurve_channel=KeyCurve::ROTATEX;
	curve[4].keycurve_channel=KeyCurve::ROTATEY;
	curve[5].keycurve_channel=KeyCurve::ROTATEZ;
	curve[6].keycurve_channel=KeyCurve::SCALEX;
	curve[7].keycurve_channel=KeyCurve::SCALEY;
	curve[8].keycurve_channel=KeyCurve::SCALEZ;

	curve[0].keycurve_nkeys=aTransKey.Count();
	curve[1].keycurve_nkeys=aTransKey.Count();
	curve[2].keycurve_nkeys=aTransKey.Count();
	curve[3].keycurve_nkeys=aRotKey.Count();
	curve[4].keycurve_nkeys=aRotKey.Count();
	curve[5].keycurve_nkeys=aRotKey.Count();
	curve[6].keycurve_nkeys=aScaleKey.Count();
	curve[7].keycurve_nkeys=aScaleKey.Count();
	curve[8].keycurve_nkeys=aScaleKey.Count();

	curve[0].keycurve_keys=new Keyframe[aTransKey.Count()];
	curve[1].keycurve_keys=new Keyframe[aTransKey.Count()];
	curve[2].keycurve_keys=new Keyframe[aTransKey.Count()];
	curve[3].keycurve_keys=new Keyframe[aRotKey.Count()];
	curve[4].keycurve_keys=new Keyframe[aRotKey.Count()];
	curve[5].keycurve_keys=new Keyframe[aRotKey.Count()];
	curve[6].keycurve_keys=new Keyframe[aScaleKey.Count()];
	curve[7].keycurve_keys=new Keyframe[aScaleKey.Count()];
	curve[8].keycurve_keys=new Keyframe[aScaleKey.Count()];

	for(int i=0;i<aTransKey.Count();i++)//trans
	{
		curve[0].keycurve_keys[i].keyframe_ftime=aTransKey[i].time;
		curve[0].keycurve_keys[i].value=aTransKey[i].value[0];

		curve[1].keycurve_keys[i].keyframe_ftime=aTransKey[i].time;
		curve[1].keycurve_keys[i].value=aTransKey[i].value[1];

		curve[2].keycurve_keys[i].keyframe_ftime=aTransKey[i].time;
		curve[2].keycurve_keys[i].value=aTransKey[i].value[2];
	}

	animation->Append(&curve[0]);
	animation->Append(&curve[1]);
	animation->Append(&curve[2]);

	for(int i=0;i<aRotKey.Count();i++)//rot
	{
		curve[3].keycurve_keys[i].keyframe_ftime=aRotKey[i].time;
		curve[3].keycurve_keys[i].value=aRotKey[i].value[0];

		curve[4].keycurve_keys[i].keyframe_ftime=aRotKey[i].time;
		curve[4].keycurve_keys[i].value=aRotKey[i].value[1];

		curve[5].keycurve_keys[i].keyframe_ftime=aRotKey[i].time;
		curve[5].keycurve_keys[i].value=aRotKey[i].value[2];
	}

	animation->GetCurves().Append(&curve[3]);
	animation->GetCurves().Append(&curve[4]);
	animation->GetCurves().Append(&curve[5]);

	for(int i=0;i<aScaleKey.Count();i++)//scl
	{
		curve[6].keycurve_keys[i].keyframe_ftime=aScaleKey[i].time;
		curve[6].keycurve_keys[i].value=aScaleKey[i].value[0];

		curve[7].keycurve_keys[i].keyframe_ftime=aScaleKey[i].time;
		curve[7].keycurve_keys[i].value=aScaleKey[i].value[1];

		curve[8].keycurve_keys[i].keyframe_ftime=aScaleKey[i].time;
		curve[8].keycurve_keys[i].value=aScaleKey[i].value[2];
	}

	animation->GetCurves().Append(&curve[6]);
	animation->GetCurves().Append(&curve[7]);
	animation->GetCurves().Append(&curve[8]);

#pragma message (LOCATION " remember to destroy lpFloat3TimeKey array content")
	
#endif
	return true;
}

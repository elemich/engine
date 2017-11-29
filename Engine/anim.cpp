#include "entities.h"

Keyframe::Keyframe():time(0.0f),value(0){}

Influence::Influence():
	influence_controlpointindex(0),
	influence_ncontrolpointindex(0),
	influence_weight(0.0f)
	{}

Cluster::Cluster():
	cluster_bone(0),
	cluster_influences(NULL),
	cluster_ninfluences(0)
	{}

KeyCurve::KeyCurve():
	keycurve_channel(INVALID_CHANNEL),
	keycurve_start(-1),
	keycurve_end(-1)
{}

void Gizmo::draw(RendererInterface* renderer)
{
	renderer->draw(this);
}


AnimClip::AnimClip():
	curvegroup_start(-1),
	curvegroup_end(-1)
{}

Animation::Animation():
	entity(0),
	clipIdx(0),
	start(-1),
	end(-1)
{}

void copychannel(EChannel channel,float& val,float* poff,float* roff,float* soff)
{
	if(!val)
		return;

	switch(channel)
	{
	case TRANSLATEX:poff[0]	= val; break;
	case TRANSLATEY:poff[1]	= val; break;
	case TRANSLATEZ:poff[2]	= val; break;
	case ROTATEX:roff[0] = val; break;
	case ROTATEY:roff[1] = val; break;
	case ROTATEZ:roff[2] = val; break;
	case SCALEX:soff[0]	= val; break;
	case SCALEY:soff[1]	= val; break;
	case SCALEZ:soff[2]	= val; break;
	}
}

float cubic_interpolation(float v0, float v1, float v2, float v3, float x)
{
	float P = (v3 - v2) - (v0 - v1);
	float Q = (v0 - v1) - P;
	float R = v2 - v0;
	float S = v1;

	float x2 = x * x;
	float x3 = x2 * x;


	return P * x3 + Q * x2 + R * x + S;
}


void AnimationController::add(Animation* anim)
{
	if(this->animations.empty())
	{
		this->start=anim->start;
		this->end=anim->end;
	}

	this->animations.push_back(anim);

	if(!this->animations.empty())
	{
		this->start=anim->start>this->start ? anim->start : this->start;
		this->end=anim->end<this->end ? anim->end : this->end;
	}
}

void AnimationController::update()
{
	if(this->play)
	{
		for(size_t i=0;i<this->animations.size();i++)
		{
			Animation* anim=this->animations[i];

			int keyIdx=0;

			if(anim->clipIdx<(int)anim->clips.size())
			{
				AnimClip* curvegroup=anim->clips[anim->clipIdx];

				if(curvegroup)
				{
					int numcurves=(int)curvegroup->curvegroup_keycurves.size();

					vec3 poff,roff,soff(1,1,1);
					float val=0;

					for(int curveIdx=0;curveIdx<numcurves;curveIdx++)
					{
						KeyCurve &curve=*curvegroup->curvegroup_keycurves[curveIdx];

						int			numCurveKeys=(int)curve.keycurve_keyframes.size();
						int			lastKeyIdx=numCurveKeys-1;

						if(numCurveKeys==1)
						{
							val=curve.keycurve_keyframes[0]->value;
							copychannel(curve.keycurve_channel,val,poff,roff,soff);
						}
						else
						{
							for (keyIdx = 0; keyIdx < numCurveKeys; keyIdx++)
							{
								if(keyIdx!=lastKeyIdx)
								{
									if(!(this->cursor>=curve.keycurve_keyframes[keyIdx]->time && this->cursor<=curve.keycurve_keyframes[keyIdx+1]->time))
										continue;

									Keyframe	*aa=curve.keycurve_keyframes[(keyIdx>0 ? keyIdx-1 : keyIdx)];
									Keyframe	*bb=curve.keycurve_keyframes[keyIdx];
									Keyframe	*cc=curve.keycurve_keyframes[keyIdx+1];
									Keyframe	*dd=curve.keycurve_keyframes[(keyIdx < lastKeyIdx-1 ? keyIdx+2 : keyIdx+1)];

									float		t=(this->cursor - bb->time) / (cc->time - bb->time);

									val=cubic_interpolation(aa->value,bb->value,cc->value,dd->value,t);

									copychannel(curve.keycurve_channel,val,poff,roff,soff);

									break;
								}
								else
								{
									val=curve.keycurve_keyframes[lastKeyIdx]->value;
									copychannel(curve.keycurve_channel,val,poff,roff,soff);
								}
							}
						}
					}

					mat4 sm,rm,tm;

					if(poff.iszero())
						tm.translate(anim->entity->local.position());
					else
						tm.translate(poff);

					sm.scale(soff);

					rm.rotate(-roff[2],0,0,1);
					rm.rotate(-roff[1],0,1,0);
					rm.rotate(-roff[0],1,0,0);

					anim->entity->local=rm*sm*tm;
				}

			}

			anim->entity->nAnimated++;
		}


		if(this->cursor>this->end)
		{
			this->cursor=0;

			if(!this->looped)
				this->play=false;
		}
		else
		{
			float curDelta=(Timer::instance->GetTime()-this->lastFrameTime)/1000.0f;
			this->cursor+=curDelta*this->speed;
			this->lastFrameTime=Timer::instance->GetTime();
		}
	}
}


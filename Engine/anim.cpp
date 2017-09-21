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


CurveGroup::CurveGroup():
	curvegroup_start(-1),
	curvegroup_end(-1)
{}

Animation::Animation():
	animation_animselected(0),
	animation_nprocessed(0),
	animation_time(0),
	animation_direction(0),
	animation_start(-1),
	animation_end(-1),
	animation_scl(1,1,1),
	ftime(0)
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

void Animation::update()
{
	int keyIdx=0;
	this->animation_nprocessed=0;

	this->animation_transform=this->entity->entity_transform;

	//return 1;

	if(this->animation_animselected<(int)this->animation_curvegroups.size())
	{
		CurveGroup* curvegroup=this->animation_curvegroups[this->animation_animselected];

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
							if(!(ftime>=curve.keycurve_keyframes[keyIdx]->time && ftime<=curve.keycurve_keyframes[keyIdx+1]->time))
								continue;

							Keyframe	*a=curve.keycurve_keyframes[(keyIdx>0 ? keyIdx-1 : keyIdx)];
							Keyframe	*b=curve.keycurve_keyframes[keyIdx];
							Keyframe	*c=curve.keycurve_keyframes[keyIdx+1];
							Keyframe	*d=curve.keycurve_keyframes[(keyIdx < lastKeyIdx-1 ? keyIdx+2 : keyIdx+1)];

							float		t=(ftime - b->time) / (c->time - b->time);

							val=cubic_interpolation(a->value,b->value,c->value,d->value,t);

							this->animation_nprocessed++;

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
				tm.translate(this->entity->entity_transform.position());
			else
				tm.translate(poff);

			sm.scale(soff);

			rm.rotate(-roff[2],0,0,1);
			rm.rotate(-roff[1],0,1,0);
			rm.rotate(-roff[0],1,0,0);

			this->animation_transform=rm*sm*tm;
		}

	}

	this->entity->entity_world = this->entity->entity_parent ? (this->animation_transform * this->entity->entity_parent->entity_world) : this->animation_transform;

	this->entity->nAnimated++;

	ftime+=0.02f;

	if(ftime>this->animation_end)
		ftime=this->animation_start;
}
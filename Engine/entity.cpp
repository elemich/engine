#include "entities.h"

#include "datatypes.h"

#include <stdio.h>

#define PROCESS_ENTITIES_RECURSIVELY 0


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

Entity::Entity(EEntity e)
:
	entity_type(e),
	entity_parent(0)
{
	Entity::entities.Push(this);
	Entity::typed_entities[this->entity_type].Push(this);

	nDrawed=0;
	nAnimated=0;
	nUpdated=0;
}

Entity::~Entity()
{
	Entity::typed_entities[this->entity_type].Erase(this);
	Entity::entities.Erase(this);
}

void Entity::update()
{
	this->entity_world = this->entity_parent ? (this->animation_transform * this->entity_parent->entity_world) : this->animation_transform;
	this->nUpdated++;
}

void Entity::beginDraw()
{
	MatrixStack::Push(MatrixStack::MODELVIEW);
	MatrixStack::Multiply(MatrixStack::MODELVIEW,this->entity_world);
}

void Entity::endDraw()
{
	MatrixStack::Pop(MatrixStack::MODELVIEW);
}

void Entity::draw(RendererInterface* renderer)
{
	nDrawed++;
}

void copychannel(EChannel channel,float& val,float* poff,float* roff,float* soff)
{
	if(!val)
		return;

	switch(channel)
	{
		case TRANSLATEX:poff[0]	= val; break;
		case TRANSLATEY:poff[1]	= val; break;
		case TRANSLATEZ:poff[2]	= val; break;
		case ROTATEX:roff[0]		= val; break;
		case ROTATEY:roff[1]		= val; break;
		case ROTATEZ:roff[2]		= val; break;
		case SCALEX:soff[0]		= val; break;
		case SCALEY:soff[1]		= val; break;
		case SCALEZ:soff[2]		= val; break;
	}
}

int Entity::animate(float ftime)
{
	int keyIdx=0;
	this->animation_nprocessed=0;

	this->animation_transform=this->entity_transform;

	//return 1;

	if(this->animation_animselected<this->animation_curvegroups.Count())
	{
		CurveGroup* curvegroup=this->animation_curvegroups[this->animation_animselected];

		if(curvegroup)
		{
			int numcurves=curvegroup->curvegroup_keycurves.Count();

			vec3 poff,roff,soff(1,1,1);
			float val=0;

			for(int curveIdx=0;curveIdx<numcurves;curveIdx++)
			{
				KeyCurve &curve=*curvegroup->curvegroup_keycurves[curveIdx];

				int			numCurveKeys=curve.keycurve_keyframes.Count();
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
				tm.translate(this->entity_transform.position());
			else
				tm.translate(poff);

			sm.scale(soff);

			rm.rotate(-roff[2],0,0,1);
			rm.rotate(-roff[1],0,1,0);
			rm.rotate(-roff[0],1,0,0);

			this->animation_transform=rm*sm*tm;
		}

	}

	this->nAnimated++;

	return this->animation_nprocessed;
}






////////////////////////////////////////77



typedef TDLAutoList<Entity*> LIST;
typedef TDLAutoNode<Entity*> NODE;

TDLAutoList<Entity*> Entity::entities;
TDLAutoList<Entity*> Entity::typed_entities[ENTITY_MAX];






Entity* Entity::Find(const char* name,bool exact)
{
	for(NODE *node=entities.Head();node;node=node->next)
	{
		Entity* e=node->data;
		if(e && (exact ? e->entity_name==name : e->entity_name.Contains(name)))
			return e;
	}
	return 0;
}

/*
void Entity::debugProcessed()
{
	int nError=0;
	for(NODE *node=entities.Head();node;node=node->next)
	{
		if(node->data)
		{
			int& nDrawed=node->data->nDrawed;
			int& nUpdated=node->data->nUpdated;
			int& nAnimated=node->data->nAnimated;
			String &name=node->data->entity_name;
			if(nDrawed>1 || nUpdated>1 || nAnimated>1){
				nError++;
				__debugbreak();
			}
			nDrawed=nUpdated=nAnimated=0;
		}
	}
}*/

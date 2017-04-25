#include "entities.h"

//#include "include\renderer.h"

void Skeleton::draw(RendererInterface* renderer)
{
	renderer->draw(this);
}

void Skeleton::update()
{

}

void Skeleton::animate()
{




	/*int keyIdx=0;
	this->animation_nprocessed=0;

	this->animation_transform=this->entity_transform;


	/ *for(int i=0;i<(int)bones.size();i++)
	{
		if(bones[i]->animation)
			bones[i]->animation->a
	}* /

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

	return this->animation_nprocessed;*/
}

Bone::Bone()
	:
	bone_root(0)
{
	int x;
	int y;
	int z;

	while(true){
		x=rand() % 2;
		y=rand() % 2;
		z=rand() % 2;

		srand(rand() % 10000);

		if(x>0 || y>0 || z>0)
			break;
	}

	bone_color.make((float)x,(float)y,(float)z);
}

void Bone::update()
{
	Entity::update();
}	

void Bone::draw(RendererInterface* renderer)
{
	Entity::draw(renderer);
	renderer->draw(this);
}

int Bone::animate(float ftime)
{
	if(this == this->bone_root)
		this->animation_time = this->animation_nprocessed ? this->animation_time+0.02f : 0.0f;
	else
		this->animation_time = this->bone_root->animation_time;

	Entity::animate(this->animation_time);

#pragma message (LOCATION " this->animation_time+=0.03f must be moved away from Bone::animate")

	if(this->bone_root != this)
		this->bone_root->animation_nprocessed+=this->animation_nprocessed;

	return this->animation_nprocessed;
}





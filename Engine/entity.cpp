#include "entities.h"

#include "datatypes.h"

#include <stdio.h>



std::list<Entity*> Entity::pool;

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

Entity::Entity():entity_parent(0)
{
	if(!pool.size())
	pool.push_back(this);

	nDrawed=0;
	nAnimated=0;
	nUpdated=0;
}

Entity::~Entity()
{
	pool.remove(this);
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
	for(int i=0;i<(int)components.size();i++)
		components[i]->draw(renderer);
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
		case ROTATEX:roff[0] = val; break;
		case ROTATEY:roff[1] = val; break;
		case ROTATEZ:roff[2] = val; break;
		case SCALEX:soff[0]	= val; break;
		case SCALEY:soff[1]	= val; break;
		case SCALEZ:soff[2]	= val; break;
	}
}

int Entity::animate(float ftime)
{
	for(int i=0;i<(int)components.size();i++)
		components[i]->animate(ftime);

	return 0;
}

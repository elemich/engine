#include "entities.h"

#include "datatypes.h"

#include <stdio.h>


void _setEntitieslevel(Entity* e)
{
	if(!e)
		return;

	e->level=e->parent ? e->parent->level+1 : e->level;
	e->expanded=!e->level ? true : false;

	for_each(e->childs.begin(),e->childs.end(),_setEntitieslevel);
}


Entity::Entity():
	selected(false),
	expanded(false),
	level(0),
	properties(0)
{
	nDrawed=0;
	nAnimated=0;
	nUpdated=0;
}

	
void Entity::SetParent(Entity* iParent)
{

	Entity* oldParent=this->parent;
	this->parent=iParent;

	if(oldParent)
		oldParent->childs.erase(std::find(oldParent->childs.begin(),oldParent->childs.end(),this));

	if(this->parent)
		this->parent->childs.push_back(this);

	_setEntitieslevel(this);
}

Entity* Entity::Create(Entity* iParent)
{
	Entity* e=new Entity;
	e->SetParent(iParent);
	return e;
}

Entity::~Entity()
{
}

void Entity::update()
{
	/*if(this->nDrawed>=1 && this->nUpdated>=1)
	{
		this->nAnimated=0;
		this->nUpdated=0;
		this->nDrawed=0;
	}

	
	if(this->nUpdated>1)
	{
		return;
	}*/

	this->world = this->parent ? (this->transform * this->parent->world) : this->transform;

	for(std::vector<EntityComponent*>::iterator it=this->components.begin();it!=this->components.end();it++)
		(*it)->update();

	for(std::list<Entity*>::iterator it=this->childs.begin();it!=this->childs.end();it++)
		(*it)->update();

	//this->nUpdated++;
}

void Entity::beginDraw()
{
	MatrixStack::Push(MatrixStack::MODELVIEW);
	MatrixStack::Multiply(MatrixStack::MODELVIEW,this->world);
}

void Entity::endDraw()
{
	MatrixStack::Pop(MatrixStack::MODELVIEW);
}

void Entity::draw(RendererInterface* renderer)
{	
	/*if(this->nDrawed>1)
	{
		return;
	}*/

	for(std::vector<EntityComponent*>::iterator it=this->components.begin();it!=this->components.end();it++)
		(*it)->draw(renderer);

	for(std::list<Entity*>::iterator it=this->childs.begin();it!=this->childs.end();it++)
		(*it)->draw(renderer);

	//renderer->draw(this->bbox);

	//this->nDrawed++;
}


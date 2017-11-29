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


EntityScript::EntityScript(Entity* iEntity):entity(iEntity){}



Entity::Entity():
	selected(false),
	expanded(false),
	level(0),
	properties(0),
	script(0),
	module(0)
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
	for(std::vector<EntityComponent*>::iterator it=this->components.begin();it!=this->components.end();it++)
		(*it)->update();

	if(this->script)
		this->script->update();

	this->parent ? this->world=(this->local * this->parent->world) : this->world;

	for(std::list<Entity*>::iterator it=this->childs.begin();it!=this->childs.end();it++)
		(*it)->update();
}


void Entity::draw(RendererInterface* renderer)
{	
	MatrixStack::Push(MatrixStack::MODEL,this->world);

	for(std::vector<EntityComponent*>::iterator it=this->components.begin();it!=this->components.end();it++)
		(*it)->draw(renderer);

	for(std::list<Entity*>::iterator it=this->childs.begin();it!=this->childs.end();it++)
		(*it)->draw(renderer);

	MatrixStack::Pop(MatrixStack::MODEL);
}


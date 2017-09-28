#include "entities.h"

#include "datatypes.h"

#include <stdio.h>



std::list<Entity*> Entity::pool;



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
	if(this->nDrawed>=1 && this->nUpdated>=1)
	{
		this->nAnimated=0;
		this->nUpdated=0;
		this->nDrawed=0;
	}

	
	if(this->nUpdated>1)
	{
		return;
	}

	for(std::vector<EntityComponent*>::iterator it=this->components.begin();it!=this->components.end();it++){
			(*it)->update();
	}

	this->entity_world = this->entity_parent ? (this->entity_transform * this->entity_parent->entity_world) : this->entity_transform;

	this->nUpdated++;

	for(std::list<Entity*>::iterator it=this->entity_childs.begin();it!=this->entity_childs.end();it++)
		(*it)->update();

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
	if(this->nDrawed>1)
	{
		return;
	}

	this->beginDraw();

	for(std::vector<EntityComponent*>::iterator it=this->components.begin();it!=this->components.end();it++)
		(*it)->draw(renderer);

	for(std::list<Entity*>::iterator it=this->entity_childs.begin();it!=this->entity_childs.end();it++)
		(*it)->draw(renderer);

	this->endDraw();

	this->nDrawed++;
}


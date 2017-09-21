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




std::vector<EntityComponent*> Entity::findComponents(EntityComponent* (EntityComponent::*isa)()) 
{
	std::vector<EntityComponent*> retComp;

	for(int i=0;i<(int)this->components.size();i++)
	{
		if((this->components[i]->*isa)())
			retComp.push_back((this->components[i]->*isa)());
	}

	return retComp;
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
	this->entity_world = this->entity_parent ? (this->entity_transform * this->entity_parent->entity_world) : this->entity_transform;


	for(std::vector<EntityComponent*>::iterator it=this->components.begin();it!=this->components.end();it++){
		if((*it)->GetAnim())
			(*it)->update();
	}

	for(std::vector<EntityComponent*>::iterator it=this->components.begin();it!=this->components.end();it++){
		if((*it)->GetBone())
			(*it)->update();
	}

	for(std::vector<EntityComponent*>::iterator it=this->components.begin();it!=this->components.end();it++){
		if((*it)->GetMesh())
			(*it)->update();
	}

	for(std::vector<EntityComponent*>::iterator it=this->components.begin();it!=this->components.end();it++){
		if((*it)->GetSkin())
			(*it)->update();
	}

	

	for(std::list<Entity*>::iterator it=this->entity_childs.begin();it!=this->entity_childs.end();it++)
		(*it)->update();

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
	if(this->nDrawed>1)
	{
		return;
	}
	//beginDraw();

	for(std::vector<EntityComponent*>::iterator it=this->components.begin();it!=this->components.end();it++)
		(*it)->draw(renderer);

	for(std::list<Entity*>::iterator it=this->entity_childs.begin();it!=this->entity_childs.end();it++)
		(*it)->draw(renderer);

	//endDraw();

	this->nDrawed++;
}


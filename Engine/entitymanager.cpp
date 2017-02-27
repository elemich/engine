#include "datatypes.h"

int Entity::animate(float){return 0;}
void Entity::update(){}
void Entity::draw(){}

/*
typedef TDLList<Entity*> LIST;
typedef TDLNode<Entity*> NODE;

TDLList<Entity*> EntityManager::entities;
TDLList<Entity*> EntityManager::typed_entities[ENTITY_MAX];


int processNode(NODE* node,int type,int time)
{
	int retValue=0;

#if PROCESS_ENTITIES_RECURSIVELY
	if(node)
#else
	for(;node;node=node->next)
#endif
	{
		switch(type)
		{
		case 0:
			retValue+=node->data->animate(time);
			break;
		case 1:
			node->data->update();
			break;
		case 2:
			node->data->draw();
			break;
		}

#if PROCESS_ENTITIES_RECURSIVELY
		retValue+=processNode(node->data->entity_childs.Head(),type,time);
#endif
	}

	return retValue;
}


void EntityManager::draw()
{
	processNode(Head(),2,0);
}

void EntityManager::update()
{
	processNode(Head(),1,0);
}

int EntityManager::animate(float _time)
{
	return processNode(Head(),0,_time);
}


Entity* EntityManager::Find(const char* name,bool exact)
{
	for(Node<Entity*> *node=Head();node;node=node->next)
	{
		Entity* e=node->data;
		if(e && (exact ? e->entity_name==name : e->entity_name.Contains(name)))
			return e;
	}
	return 0;
}

void EntityManager::debugProcessed()
{
	int nError=0;
	for(Node<Entity*> *node=Head();node;node=node->next)
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

#include "interfaces.h"

#include "entities.h"

std::vector<RendererInterface*> RendererInterface::renderers;



/*
typedef TDLAutoNode<Entity*> ENTITYNODE;


int processNode(ENTITYNODE* node,int type,float time)
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
			for(int i=0;i<RendererInterface::renderers;i++)
				RendererInterface::renderers[i].draw(node->data);
			break;
		}

#if PROCESS_ENTITIES_RECURSIVELY
		retValue+=processNode(node->data->entity_childs.Head(),type,time);
#endif
	}

	return retValue;
}
*/



static void draw()//draw on all instanced renderers
{
	/*for(int i=0;i<RendererInterface::renderers;i++)
		processNode*/
}
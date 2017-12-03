
#include "entities.h"

struct Pippo : EntityScript
{
	void init()
	{
		this->entity->local.identity();
	}
	
	void update()
	{
		this->entity->local.translate(0.1f,0.0f,0.0f);
	}
	
	void deinit()
	{
		this->entity->local.identity();
	}
};


extern "C" __declspec(dllexport) EntityScript* Create(){return new Pippo;}
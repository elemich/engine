#include "entities.h"

//#include "include\renderer.h"

Bone::Bone()
	:
	bone_root(NULL)
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





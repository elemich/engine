#include "entities.h"

//#include "include\renderer.h"

Bone::Bone()
	:
	root(0)
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

	color.make((float)x,(float)y,(float)z);
}



void Bone::draw(RendererInterface* renderer)
{
	renderer->draw(this);
}





#include "entities.h"

Material::Material():
	Resource(RESOURCE_MATERIAL)
{
	VectorMathNamespace::make(emissive,3,0.2f,0.2f,0.2f);
	VectorMathNamespace::make(diffuse,3,0.2f,0.2f,0.2f);
	VectorMathNamespace::make(normalmap,3,0.2f,0.2f,0.2f);
	VectorMathNamespace::make(bump,3,0.2f,0.2f,0.2f);
	VectorMathNamespace::make(transparent,3,0.2f,0.2f,0.2f);
	VectorMathNamespace::make(displacement,3,0.2f,0.2f,0.2f);
	VectorMathNamespace::make(ambient,3,0.2f,0.2f,0.2f);
	VectorMathNamespace::make(specular,3,0,0,0);
	VectorMathNamespace::make(reflection,3,0,0,0);

	fbump=0;
	femissive=0;
	fambient=0;
	fdiffuse=0;
	ftransparent=0;
	fdisplacement=0;
	fspecular=0;
	freflection=0;
	fshininess=0;
}

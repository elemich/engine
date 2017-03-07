#include "entities.h"

Light::Light():
	light_cast(0),
	light_volumetric(0),
	light_groundprojection(0),
	light_nearattenuation(0),
	light_farattenuation(0),
	light_shadows(0),
	light_intensity(0),
	light_innerangle(0),
	light_outerangle(0),
	light_fog(0),
	light_decaystart(0),
	light_nearstart(0),
	light_nearend(0),
	light_farstart(0),
	light_farend(0)
{}

ELight	Light::LightType()
{
	return light_type;
}

EDecay	Light::DecayType()
{
return light_decaytype;
}

bool    Light::Cast()
{
return light_cast;
}
bool    Light::Volumetric()
{
return light_volumetric;
}
bool    Light::GroundProjection()
{
return light_groundprojection;
}
bool    Light::NearAttenuation()
{
return light_nearattenuation;
}
bool    Light::FarAttenuation()
{
return light_farattenuation;
}
bool    Light::Shadows()
{
return light_shadows;
}
float*	Light::Color()
{
return light_color;
}
float*	Light::ShadowColor()
{
return light_shadowcolor;
}
float	Light::Intensity()
{
return light_intensity;
}
float	Light::InnerAngle()
{
return light_innerangle;
}
float	Light::OuterAngle()
{
return light_outerangle;
}
float  Light::Fog()
{
return light_fog;
}
float	Light::DecayStart()
{
return light_decaystart;
}
float	Light::NearStart()
{
return light_nearstart;
}
float	Light::NearEnd()
{
return light_nearend;
}
float	Light::FarStart()
{
return light_farstart;
}
float	Light::FarEnd()
{
return light_farend;
}




void Light::update()
{

}
void Light::draw(RendererInterface* renderer)
{

}
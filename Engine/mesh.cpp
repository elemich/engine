#include "entities.h"

//#include "include\renderer.h"

Mesh::Mesh():
	mesh_controlpoints(NULL),
	mesh_ncontrolpoints(0),
	mesh_vertexindices(0),
	mesh_nvertexindices(0),
	mesh_texcoord(0),
	mesh_ntexcoord(0),
	mesh_normals(NULL),
	mesh_nnormals(0),
	mesh_npolygons(0),
	mesh_colors(NULL),
	mesh_ncolors(0),
	mesh_isCCW(true)
{
}


float** Mesh::GetControlPoints()
{
	return (float**)this->mesh_controlpoints;
}

int Mesh::GetNumControlPoints()
{
	return this->mesh_ncontrolpoints;
}

float** Mesh::GetTriangles()
{
	return (float**)this->mesh_vertexindices;
}

int Mesh::GetNumTriangles()
{
	return this->mesh_nvertexindices;
}

float** Mesh::GetUV()
{
	return (float**)this->mesh_texcoord;
}

int Mesh::GetNumUV()
{
	return this->mesh_ntexcoord;
}


float** Mesh::GetNormals()
{
	return (float**)this->mesh_normals;
}

int Mesh::GetNumNormals()
{
	return this->mesh_nnormals;
}

std::vector<Material*>& Mesh::GetMaterials()
{
	return this->mesh_materials;
}

void Mesh::update()
{
}

void Mesh::draw(RendererInterface* renderer)
{
	//


	AABB &bbox=this->entity->entity_bbox;
	//AABB aabb(this->entity_world.transform(bbox.a),this->entity_world.transform(bbox.b));
	//renderer->draw(aabb,vec3(1,0,0));	

	renderer->draw(bbox,vec3(1,1,0));

	renderer->draw(this);

	//this->entity->endDraw();
}



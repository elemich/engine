#include "entities.h"

//#include "include\renderer.h"

#include <string>

Skin::Skin():
	skin_textures(NULL),
	skin_ntextures(0),
	skin_clusters(NULL),
	skin_nclusters(0),
	skin_vertexcache(NULL)
{
}



void Skin::draw(RendererInterface* renderer)
{
	renderer->draw(this);
}

void Skin::update()
{
	//this->entity->update();

	/*this->skin_vertexcache=0;
	return;*/

	if(this->skin_vertexcache)
		delete [] this->skin_vertexcache;

	this->skin_vertexcache=new float[this->mesh_ncontrolpoints*3];
	float (*wcache)[3]=new float[this->mesh_ncontrolpoints][3];

	memset(wcache,0,this->mesh_ncontrolpoints*sizeof(float)*3);

	for(int tncluster=0;tncluster<this->skin_nclusters;tncluster++)
	{
		Cluster *clu=&skin_clusters[tncluster];

		float *src=NULL;
		float *dst=NULL;

		if(!clu || (clu && !clu->cluster_bone))
			continue;

		mat4 palette=clu->cluster_offset * clu->cluster_bone->entity_world;

		mat4 final;
		final.zero();

		final+=palette;

		mat4 skinmtx1=final;
		mat4 skinmtx2;

		for(int nw=0;nw<clu->cluster_ninfluences;nw++)
		{
			Influence &inf=clu->cluster_influences[nw];

			src=this->mesh_controlpoints[inf.influence_controlpointindex[0]];
			dst=&this->skin_vertexcache[inf.influence_controlpointindex[0]*3];	

			if(inf.influence_weight!=1.0f)
			{
				float v[3];

				MatrixMathNamespace::transform(v,skinmtx1,src);

				wcache[inf.influence_controlpointindex[0]][0]+=v[0]*inf.influence_weight;
				wcache[inf.influence_controlpointindex[0]][1]+=v[1]*inf.influence_weight;
				wcache[inf.influence_controlpointindex[0]][2]+=v[2]*inf.influence_weight;

				memcpy(dst,wcache[inf.influence_controlpointindex[0]],3*sizeof(float));
			}
			else
			{
				MatrixMathNamespace::transform(dst,skinmtx1,src);
			}

			for(int i=1;i<inf.influence_ncontrolpointindex;i++)
			{
				memcpy(&this->skin_vertexcache[inf.influence_controlpointindex[i]*3],dst,3*sizeof(float));
			}
		}
	}

	delete [] wcache;
}


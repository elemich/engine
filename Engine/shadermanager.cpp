#include "datatypes.h"

#include "shader_data.h"

typedef TDLList<ShaderInterface*> LIST;
typedef TDLNode<ShaderInterface*> NODE;

TDLList<ShaderInterface*> ShaderManager::shaders;
ShaderInterface*		  ShaderManager::current=0;

ShaderInterface* ShaderManager::Find(const char* name,bool exact)
{
	NODE *shaderNode=shaders.First();

	while(shaderNode)
	{
		ShaderInterface* element=shaderNode->data;

		String programName=element->GetName();

		if(element && programName.Buf())
			if(exact ? programName==name :  programName==name)
				return shaderNode->data;

		shaderNode=shaderNode->next;
	}

	return NULL;
}


void ShaderManager::SetMatrices(float* proj,float* mdlv)
{
	for(NODE *ns=shaders.First();ns;ns=ns->next)
	{
		ShaderInterface* shader=ns->data;

		if(ShaderManager::GetCurrent()!=shader)
			shader->Use();

		shader->SetProjectionMatrix(proj);
		shader->SetModelviewMatrix(mdlv);
	}
}

ShaderInterface* ShaderManager::GetCurrent()
{
	return current;
}
void ShaderManager::SetCurrent(ShaderInterface* shader)
{
	current=shader;
}






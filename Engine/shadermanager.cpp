#include "datatypes.h"


ShadersPool ShaderInterface::shadersPool;



std::vector<ShaderInterface*> ShadersPool::pool;
ShaderInterface*			  currentShader=0;

ShaderInterface* ShadersPool::Find(const char* name,bool exact)
{
	for(int i=0;i<(int)pool.size();i++)
	{
		ShaderInterface* element=pool[i];

		const char* programName=element->GetName();

		if(element && programName)
			if(exact ? 0==strcmp(programName,name) :  0!=strstr(programName,name))
				return pool[i];
	}

	return 0;
}


void ShadersPool::SetMatrices(float* proj,float* mdlv)
{
	for(int i=0;i<(int)pool.size();i++)
	{
		ShaderInterface* shader=pool[i];

		if(GetCurrent()!=shader)
			shader->Use();

		shader->SetProjectionMatrix(proj);
		shader->SetModelviewMatrix(mdlv);
	}
}

ShaderInterface* ShadersPool::GetCurrent()
{
	return currentShader;
}
void ShadersPool::SetCurrent(ShaderInterface* shader)
{
	currentShader=shader;
}



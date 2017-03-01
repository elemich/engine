#include "datatypes.h"

typedef TDLAutoList<ShaderInterface*> LIST;
typedef TDLAutoNode<ShaderInterface*> NODE;

TDLAutoList<ShaderInterface*> ShaderInterface::shaders;
ShaderInterface*			  _current=0;
const ShaderInterface*&			 ShaderInterface::current=(const ShaderInterface*&)_current;

ShaderInterface* ShaderInterface::Find(const char* name,bool exact)
{
	NODE *shaderNode=shaders.Head();

	while(shaderNode)
	{
		ShaderInterface* element=shaderNode->data;

		String programName=element->GetName();

		if(element && programName.Buf())
			if(exact ? programName==name :  programName==name)
				return shaderNode->data;

		shaderNode=shaderNode->Next();
	}

	return 0;
}


void ShaderInterface::SetMatrices(float* proj,float* mdlv)
{
	for(NODE *ns=shaders.Head();ns;ns=ns->Next())
	{
		ShaderInterface* shader=ns->data;

		if(ShaderInterface::GetCurrent()!=shader)
			shader->Use();

		shader->SetProjectionMatrix(proj);
		shader->SetModelviewMatrix(mdlv);
	}
}

ShaderInterface* ShaderInterface::GetCurrent()
{
	return _current;
}
void ShaderInterface::SetCurrent(ShaderInterface* shader)
{
	_current=shader;
}





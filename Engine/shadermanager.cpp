#include "datatypes.h"


ShaderInterface::ShaderInterface()
{}



ShaderInterface* ShaderInterface::Find(const char* name,bool exact)
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






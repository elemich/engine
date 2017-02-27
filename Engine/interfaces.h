#ifndef INTERFACES_H
#define INTERFACES_H

#include "datatypes.h"
#include "conditionals.h"









struct ObjectInterface
{

};

struct StringInterface : ObjectInterface
{

};

struct NumberInterface : ObjectInterface
{

};



struct AppInterface
{
	virtual int Init()=0;
	virtual void AppLoop()=0;
	virtual void CreateMainWindow()=0;
};

struct RendererInterface
{
	virtual char* Name()=0;
	virtual void Render()=0;
};

struct LoggerInterface
{

};

struct FolderBrowserInterface
{

};

struct SceneEntitiesInterface
{

};

struct PropertyInterface
{

};







#endif //INTERFACES_H



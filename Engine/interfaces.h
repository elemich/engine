#ifndef INTERFACES_H
#define INTERFACES_H

#include <cstdio>

#include "t_datatypes.h"
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







#endif //INTERFACES_H



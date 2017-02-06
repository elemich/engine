#ifndef INTERFACES_H
#define INTERFACES_H

#include <cstdio>

#include "t_datatypes.h"
#include "conditionals.h"


template<class T> struct SmartPointer
{
private:
	T* _pointer;
public:

	SmartPointer(){_pointer=new T();}
	~SmartPointer(){if(_pointer)delete _pointer;_pointer=0;}

	T& operator*(){return *_pointer;}
	T* operator->(){return _pointer;}

	void operator=(T* tp){_pointer=tp;}
	void operator=(T& tp){_pointer=&tp;}
};






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
	virtual void Init()=0;
	virtual void Render()=0;
};







#endif //INTERFACES_H



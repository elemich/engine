#ifndef T_DATATYPES_H
#define T_DATATYPES_H

#define _CRT_SECURE_NO_WARNINGS

#include <vector>
#include <list>
#include <cstdio>

#define SAFEDELETE(_ptr) \
	if(_ptr!=0){\
	delete _ptr;\
	_ptr=0;}\
	

#define SAFEDELETEARRAY(_ptr) \
	if(_ptr!=0){\
	delete [] _ptr;\
	_ptr=0;}\

#define SAFERELEASE(_ptr) \
	if(_ptr!=0)\
	{\
	_ptr->Release();\
	_ptr=0;\
	}\
	

template<class T,int size> struct TNumberedVectorInterface
{
	T v[size];
};

template <class T> struct TStaticClass
{
	static T* instance;

	TStaticClass()
	{
		if(!instance)
			instance=(T*)this;
	}
};

template <class T> T* TStaticClass<T>::instance=0;


template <class T> struct TClassPool
{
	static std::vector<T*> pool;

	TClassPool()
	{
		pool.push_back((T*)this);
	}

	~TClassPool()
	{
		pool.erase(std::find(pool.begin(),pool.end(),(T*)this));
	}
};

template <class T> std::vector<T*> TClassPool<T>::pool;


template<class T> struct SmartPointer
{
private:
	T* _pointer;
public:

	SmartPointer(){_pointer=new T();}
	~SmartPointer(){SAFEDELETE(_pointer);}

	T& operator*(){return *_pointer;}
	T* operator->(){return _pointer;}

	void operator=(T* tp){_pointer=tp;}
	void operator=(T& tp){_pointer=&tp;}
};


#endif //T_DATATYPES_H







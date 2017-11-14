#ifndef T_DATATYPES_H
#define T_DATATYPES_H

#define _CRT_SECURE_NO_WARNINGS

#include <vector>
#include <list>
#include <cstdio>
#include <typeinfo>
#include <algorithm>
#include <functional>

#define SAFEDELETE(_ptr) \
	if(0!=_ptr){\
	delete _ptr;\
	_ptr=0;}\
	

#define SAFEDELETEARRAY(_ptr) \
	if(0!=_ptr){\
	delete [] _ptr;\
	_ptr=0;}\

#define SAFERELEASE(_ptr) \
	if(0!=_ptr)\
	{\
	_ptr->Release();\
	_ptr=0;\
	}\
	

struct THierarchy
{
	
};

template <typename T> struct THierarchyVector : THierarchy
{
	T* parent;
	std::vector<T*> childs;

	THierarchyVector():parent(0){}
};

template <typename T> struct THierarchyList : THierarchy
{
	T* parent;
	std::list<T*> childs;

	THierarchyList():parent(0){}
};

template<class T,int size> struct TNumberedVectorInterface
{
	T v[size];
};

template <class T> struct TStaticInstance
{
	static T* instance;

	TStaticInstance()
	{
		if(!instance)
			instance=(T*)this;
	}
};

template <class T> T* TStaticInstance<T>::instance=0;


template <typename T> struct TPoolVector : TStaticInstance< TPoolVector<T> >
{
	static std::vector<T*> pool;

	TPoolVector()
	{
		pool.push_back((T*)this);
	}

	~TPoolVector()
	{
		pool.erase(std::find(pool.begin(),pool.end(),(T*)this));
	}

	static void BroadcastToPool(void (T::*func)())
	{
		for_each(TPoolVector<T>::pool.begin(),TPoolVector<T>::pool.end(),std::mem_fun(func));
	}
	static void BroadcastToPool(void (T::*func)(void*),void* data=0)
	{
		for_each(TPoolVector<T>::pool.begin(),TPoolVector<T>::pool.end(),std::bind(func,std::placeholders::_1,data));
	}
};

template <class T> std::vector<T*> TPoolVector<T>::pool;

template<typename T> std::vector<T*>& GetPool(){return TPoolVector<T>::pool;}


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







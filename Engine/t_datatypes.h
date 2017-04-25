#ifndef T_DATATYPES_H
#define T_DATATYPES_H

#define _CRT_SECURE_NO_WARNINGS

#include <vector>
#include <list>
#include <cstdio>

#define SAFEDELETE(_ptr) \
	if(_ptr!=0){\
	printf("SAFEDELETE(%p)\n",_ptr);\
	delete _ptr;\
	_ptr=0;}\
	

#define SAFEDELETEARRAY(_ptr) \
	if(_ptr!=0){\
	printf("SAFEDELETEARRAY(%p)\n",_ptr);\
	delete [] _ptr;\
	_ptr=0;}\
	
	


template<class T> struct FourLinkNode
{
	typedef FourLinkNode NODE;

	enum
	{
		LEFT,
		TOP,
		RIGHT,
		BOTTOM
	};

	T data;

	NODE* links[4];

	NODE*& left;
	NODE*& top;
	NODE*& right;
	NODE*& bottom;

	FourLinkNode(T h,NODE* l,NODE* t,NODE* r,NODE* b):data(h),left(links[LEFT]),top(links[TOP]),right(links[RIGHT]),bottom(links[BOTTOM]){links[LEFT]=l,links[TOP]=t,links[RIGHT]=r,links[BOTTOM]=b;printf("creating node %p with data %p\n",this,data);}

	void LinkWith(NODE* n,int direction)
	{
		if(links[direction])
			n->LinkWith(links[direction],direction);
		links[direction]=n;
		if(!n)return;
		n->links[direction<2 ? direction+2 : direction-2]=this;
	}

	void LinkWithAll(NODE* l, NODE* t,NODE* r,NODE* b)
	{
		LinkWith(l,LEFT);
		LinkWith(l,TOP);
		LinkWith(r,RIGHT);
		LinkWith(b,BOTTOM);
	}

	int Count()
	{
		int cnt=0;
		for(int direction=0;direction<4;direction++)
			cnt+=(links[direction] ? links[direction]->Count() : 0);

		return cnt;
	}

	NODE* Find(T toFind,NODE* parentNode=0)
	{
		printf("searching %p in node %p\n",toFind,this);
		if(!toFind)
			return 0;
		if(toFind==data)
		{
			printf("found %p in node %p\n",data,this);
			return this;
		}

		NODE* founded=0;
		for(int direction=0;direction<4;direction++)
		{
			NODE* pLink=links[direction];

			if(pLink && pLink!=parentNode)
			{
				if(founded=pLink->Find(toFind,this))
					return founded;
			}
		}

		return 0;
	}


};

template<class A,class B> struct TPair
{
	A first;
	B second;
};

template<class T,int size> struct TNumberedVectorInterface
{
	T v[size];
};

template <class T,int i> struct ClassID
{
	static const int classid=i;
	static std::list<T*> classpool;

	ClassID()
	{
		classpool.push_back((T*)this);
	}

	~ClassID()
	{
		classpool.remove(std::find(classpool.begin(),classpool.end(),(T*)this));
	}
};

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







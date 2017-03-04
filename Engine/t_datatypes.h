#ifndef T_DATATYPES_H
#define T_DATATYPES_H

#define _CRT_SECURE_NO_WARNINGS

#include <vector>
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
	
	



template<class T> struct TVector
{
private:
	T* _data;
	int _size;
public:

	const int &size;


	TVector():size(_size){_data=0;_size=0;}
	TVector(int sz):size(_size){_size=sz;_data=new T[_size];}
	TVector(const TVector& v):size(_size)
	{
		if(v.size==0)
			return;

		_data=new T[v.size];

		T* dst=_data;
		T* src=v._data;

		if(v.size)while(*dst++=*src++);

		_size=v.size;

	}
	virtual operator const T*(){return _data;}

	T& operator[](int idx){return _data[idx];}
	
	TVector operator+(TVector& v)
	{
		TVector vNew;

		T* v1=(char*)_data;
		T* v2=v;

		if(v1!=0 || v2!=0)
			vNew._data=new T[this->size + s.size];

		T* sDest=tNew._data;

		if(t1!=0)
		while(*tDest++=*t1++);
		if(t2!=0)
		while(*tDest++=*t2++);


		return vNew;
	}


	TVector& operator+=(TVector& v)
	{
		T* vNew=0;

		T* t1=_data;
		T* t2=v;

		if(t1!=0 || t2!=0)
			vNew=new T[this->size + s.size];

		T* sDest=vNew;

		if(t1!=0)
		while(*sDest++=*t1++);
		if(t2!=0)
		while(*sDest++=*t2++);

		SAFEDELETEARRAY(_data);

		_data=vNew;

		return *this;
	}

	TVector& operator+=(T& t)
	{
		T* vNew=0;
	
		T* pOld=_data;

		vNew=new T[this->size + 1];

		T* sDest=vNew;

		if(pOld!=0)
		{int sz=_size;while((sz--) && (*sDest++=*pOld++));}

		*sDest=t;
		_size++;

		SAFEDELETEARRAY(_data);

		_data=vNew;

		return *this;
	}

	TVector& operator=(TVector& v)
	{
		if(this==&v)
			return *this;

		T* tNew=new T[s.size];

		T* t1=tNew;
		T* t2=v;

		while(*s1++=*s2++);

		SAFEDELETEARRAY(_data);

		_data=tNew;

		return *this;
	}

};


template<class T,int I> struct TFixedVector : TVector<T>
{
	TFixedVector():TVector<T>(I){}
};




template<class C> struct TDAutoArray
{
protected:

	#define	ARRAY_SPACE 10

	typedef TDAutoArray AUTOARRAY;

	C*  data;
	int count;
	int spacedim;
	int space;



public:

	AUTOARRAY():data(new C[ARRAY_SPACE]),count(0),spacedim(ARRAY_SPACE),space(ARRAY_SPACE){}
	//----------------------------------------------------------
	AUTOARRAY(int spc):data(new C[spc]),count(0),spacedim(spc),space(spc){}
	//----------------------------------------------------------
	AUTOARRAY(const AUTOARRAY& array)
		:
	count(0),
		spacedim(ARRAY_SPACE),
		space(ARRAY_SPACE)
	{
		count=array.Count();
		space=array.Space();
		spacedim=array.Spacedim();
		data=new C[count+array.Space()];

		int i=0;
		while(i<count)
		{
			data[i]=array[i];
			i++;
		}

	}
	//----------------------------------------------------------
	 AUTOARRAY&  AUTOARRAY::operator=(const AUTOARRAY& array)
	{
		AUTOARRAY acopy(array);
		this->~AUTOARRAY();
		count=acopy.Count();
		space=array.Space();
		data=new C[count+space];
		int i=0;
		while(i<count)
		{
			data[i]=acopy[i];
			i++;
		}
		return *this;
	}
	//----------------------------------------------------------
	 int AUTOARRAY::Count()const{return count;}
	//----------------------------------------------------------
	 int AUTOARRAY::Space()const{return space;}
	//----------------------------------------------------------
	 int AUTOARRAY::Spacedim()const{return spacedim;}
	//----------------------------------------------------------
	 C& AUTOARRAY::operator[](int index)const{return data[index];}
	//----------------------------------------------------------
	 void AUTOARRAY::Do(void (*func)(C& fparam))
	{
		int i=0;
		while(i<count){func(data[i]);i++;}
	}
	//----------------------------------------------------------
	 void AUTOARRAY::Erase(int index)
	{
		if(index>=count)return;

		C* newdata=new C[count+spacedim-1];
		int i=0;

		while(i<index)
		{
			newdata[i]=data[i];
			i++;
		}
		while(i<count)
		{
			newdata[i]=data[i+1];
			i++;
		}
		count-=1;
		space+=1;

		delete [] data;

		data=newdata;
	}
	//----------------------------------------------------------
	 int AUTOARRAY::Find(C c)
	{
		int i=0;
		while(i<count){
			if(data[i]==c)return i;
			i++;
		}
		return -1;
	}
	//----------------------------------------------------------
	 AUTOARRAY::~AUTOARRAY()
	{
		if(data)delete [] data;
		data=0;
		count=space=spacedim=0;
	}
	//----------------------------------------------------------
	 void AUTOARRAY::Empty()
	{
		if(data)memset(data,0,4*count);
		count=0;
		space=spacedim;
	}
	//----------------------------------------------------------
	 int AUTOARRAY::Append(C c)
	{
		if(!space)
		{
			C* newdata=new C[count+spacedim+1];
			int i=0;

			while(i<count)
			{
				newdata[i]=data[i];
				i++;
			}
			newdata[count]=c;
			space=spacedim;

			delete [] data;

			data=newdata;
		}
		else
		{
			data[count]=c;
			space-=1;
		}

		count+=1;

		return count-1;
	}
	//----------------------------------------------------------
	 AUTOARRAY AUTOARRAY::operator+(AUTOARRAY& array)
	{
		int totaldim=count+array.Count();

		AUTOARRAY ret(totaldim+spacedim);

		int i=0;

		while(i<count)
		{
			ret.Append(data[i]);
			i++;
		}
		while(i<totaldim)
		{
			ret.Append(array[i-count]);
			i++;
		}

		return ret;
	}

};




template<typename T> struct TDLNode
{
	T data;

	TDLNode<T>* prev;
	TDLNode<T>* next;
};

template<class T> struct TDLList
{
	typedef  TDLNode<T> NODE;

private:
	NODE* _head;
	NODE* _last;

	NODE* _current;

	int _size;

	NODE* __node()
	{
		NODE* n=new NODE;
		n->data=0;
		n->prev=0;
		n->next=0;
		return n;
	}

	void __addNode(NODE* n,bool dir/*0=prev,1=next*/)
	{
		if(!n)
			return;

		if(!_head)
			_last=_head=n;
		else if(dir==true)
		{
			_last->next=n;
			n->prev=_last;
			_last=n;
		}
		else
		{
			_head->prev=n;
			n->next=_head;
			_head=n;
		}

		

		_size++;
	}

public:

	const int &size;

	TDLList():_head(0),_last(0),_current(_head),_size(0),size(_size){}
	
	void operator+=(T t)
	{
		NODE* newNode=__node();

		newNode->data=t;

		__addNode(newNode,true);

	}

	void operator-=(T t)
	{
		NODE* newNode=__node();

		newNode->data=t;

		__addNode(newNode,false);

	}

	NODE* FindNode(NODE* n)
	{
		NODE* t=_head;

		while(t)
		{
			if(t==n)
				return t;
			t=t->next;
		};

		return 0;
	}

	NODE* FindElementNode(T& el)
	{
		NODE* t=_head;

		while(t)
		{
			if(t->data==el)
				return t;
			t=t->next;
		};

		return 0;
	}

	void IReset(bool dir/*0=prev,1=next*/)
	{
		_current=(dir==false ? _last : _head);
	}

	NODE* operator++(int)
	{
		return (_current=_current->next);
	}
	NODE* operator--(int)
	{
		return (_current=current->_prev);
	}

	NODE* Cur(){return _current;}
	NODE* First(){return _head;}
	NODE* Last(){return _last;}
	operator T*(){return (_current ? &_current->data : 0);}
};

template <class T> struct TDLAutoNode
{
	TDLAutoNode* prev;
	TDLAutoNode* next;

	T data;


	TDLAutoNode(){prev=next=0;}

	~TDLAutoNode(){/*prev=next=0;*/}

	TDLAutoNode(T& t){
		prev=next=0;
		data=t;
	}

	void Link(TDLAutoNode* p,TDLAutoNode* n){
		prev=p;
		next=n;
	}

	void Unlink(){
		if(prev)
			prev->next=next;
		if(next)
			next->prev=prev;
	}

	TDLAutoNode* Next()const{return next;}

	TDLAutoNode* Prev()const{return prev;}

	T& Data(){return data;}

	operator T&(){return data;}
};

template<class T> struct TDLAutoList
{

private:
	TDLAutoNode<T>* head;
	TDLAutoNode<T>* last;
	TDLAutoNode<T>* current;
	int		 count;
public:


	//----------------------------------------------------------
	 TDLAutoList()
		:
	head(0),
		last(0),
		count(0)
	{
	}
	//----------------------------------------------------------
	 TDLAutoList(T c)
		:
	head(0),
		last(0),
		count(0)
	{
		Push(c);
	}
	//----------------------------------------------------------
	 TDLAutoList(int num, T t, ...)
		:
	head(0),
		last(0),
		count(0)
	{
		va_list vl;
		va_start(vl,t);
		Push(t);
		int i=1;
		while(i<num)
		{
			Push(va_arg(vl,T));
			i++;
		}
		va_end(vl);
	}
	//----------------------------------------------------------
	 TDLAutoList(T* t, ...)
		:
	head(0),
		last(0),
		count(0)
	{
		va_list vl;
		va_start(vl,t);
		Push(*t);
		T* tmp=va_arg(vl,T*);
		while(tmp)
		{
			Push(*tmp);
			tmp=va_arg(vl,T*);
		}
		va_end(vl);
	}
	//----------------------------------------------------------
	 TDLAutoList(const TDLAutoList& l)
		:
	head(0),
		last(0),
		count(0)
	{
		delete this;

		TDLAutoNode* t=l.Head();

		while(t)
		{
			Push(t);
			t=t->Next();
		}
	}
	//----------------------------------------------------------
	 ~TDLAutoList()
	{
		while(head)
		{
			TDLAutoNode<T>* t=head->Next();
			delete head;
			head=t;

		}
		head=last=0;
		count=0;


	}
	//----------------------------------------------------------
	 TDLAutoList& operator=(const TDLAutoList& l)
	{

		this->~TDLAutoList();
		TDLAutoNode* t=l.Head();

		while(t)
		{
			Push(t->Data());
			t=t->Next();
		}

		return *this;
	}
	//----------------------------------------------------------
	 void Do(void (*func)(T& c))
	{
		TDLAutoNode* t=head;
		while(t)
		{
			func(*t);
			t=t->Next();
		}
	}
	//----------------------------------------------------------
	 void Do(void (*func)(T& c,void*),void* data)
	{
		TDLAutoNode* t=head;
		while(t)
		{
			func(*t,data);
			t=t->Next();
		}
	}
	//----------------------------------------------------------
	 void Do(void (*func)(T& c,void*,void*),void* data1,void* data2)
	{
		TDLAutoNode* t=head;
		while(t)
		{
			func(*t,data1,data2);
			t=t->Next();
		}
	}
	//----------------------------------------------------------
	TDLAutoList  operator+(const TDLAutoList& l)
	{
		 ret(*this);
		TDLAutoNode* t=l.Head();

		while(t)
		{
			ret.Push(t);
			t=t->Next();
		}

		return ret;
	}
	//----------------------------------------------------------
	 void Push(T t)
	{
		TDLAutoNode<T>* node=new TDLAutoNode<T>(t);

		node->Link(last,0);

		if(!head)
		{
			head=node;
			last=head;
		}
		else
		{
			last->Link(last->Prev(),node);
			last=node;
		}

		count++;


	}
	//----------------------------------------------------------
	 void Push(int num, T t, ...)
	{
		va_list vl;
		va_start(vl,t);
		Push(t);
		int i=1;
		while(i<num)
		{
			Push(va_arg(vl,T));
			i++;
		}
		va_end(vl);
	}
	//----------------------------------------------------------
	 void Erase(T t)
	{
		if(!t)return;

		TDLAutoNode<T>* node=Find(t);

		if(node)
		{
			node->Unlink();
			delete node;

			count--;
		}
	}
	//----------------------------------------------------------
	 void Delete(T t)
	{
		if(!t)return;

		TDLAutoNode* node=Find(t);

		if(node)
		{
			node->Unlink();
			delete node->Data();
			delete node;

			count--;
		}
	}
	//----------------------------------------------------------
	 TDLAutoNode<T>* Find(T t)const
	{
		TDLAutoNode<T>* node=head;

		while(node)
		{
			if(t==*node)return node;
			node=node->Next();
		}

		return 0;
	}
	//----------------------------------------------------------
	 void Print()
	{
		TDLAutoNode<T>* node=head;

		while(node)
		{
			printf("%p\n",node);
			node=node->Next();
		}

	}
	//----------------------------------------------------------
	 int Count()const{return count;}
	//----------------------------------------------------------
	 TDLAutoNode<T>* Head()const{return head;}
	//----------------------------------------------------------
	 TDLAutoNode<T>* Last()const{return last;}
};





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

template <class T,int size> struct TNumberedVectorInterface
{
	T v[size];
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







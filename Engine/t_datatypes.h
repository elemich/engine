#ifndef T_DATATYPES_H
#define T_DATATYPES_H


#define SAFEDELETE(_ptr) \
	if(_ptr!=0)\
	delete _ptr;\
	_ptr=0;\
	printf("deleted\n");


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

		SAFEDELETE(_data);

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

		SAFEDELETE(_data);

		_data=tNew;

		return *this;
	}

};


template<class T,int I> struct TFixedVector : TVector<T>
{
	TFixedVector():TVector<T>(I){}
};

template<typename T> struct TDLNode
{
	T _data;

	TDLNode<T>* _prev;
	TDLNode<T>* _next;
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
		n->_data=0;
		n->_prev=0;
		n->_next=0;
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
			_last->_next=n;
			n->_prev=_last;
			_last=n;
		}
		else
		{
			_head->_prev=n;
			n->_next=_head;
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

		newNode->_data=t;

		__addNode(newNode,true);

	}

	void operator-=(T t)
	{
		NODE* newNode=__node();

		newNode->_data=t;

		__addNode(newNode,false);

	}

	NODE* FindNode(NODE* n)
	{
		NODE* t=_head;

		while(t)
		{
			if(t==n)
				return t;
			t=t->_next;
		};

		return 0;
	}

	NODE* FindElementNode(T& el)
	{
		NODE* t=_head;

		while(t)
		{
			if(t->_data==el)
				return t;
			t=t->_next;
		};

		return 0;
	}

	void IReset(bool dir/*0=prev,1=next*/)
	{
		_current=(dir==false ? _last : _head);
	}

	NODE* operator++(int)
	{
		return (_current=_current->_next);
	}
	NODE* operator--(int)
	{
		return (_current=current->_prev);
	}

	NODE* Cur(){return _current;}
};


template<class A,class B> struct TPair
{
	A first;
	B second;
};


#endif //T_DATATYPES_H







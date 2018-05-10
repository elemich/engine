#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#define PRINT(x) #x
#define PRINTF(x) PRINT(x)

#define LOCATION " @mic (" __FILE__ " : " PRINTF(__LINE__) ")"

#define SAFEDELETE(_ptr) \
	if(0!=_ptr){\
	delete _ptr;\
	_ptr=0;}\

#define SAFEDELETEARRAY(_ptr) \
	if(0!=_ptr){\
	delete [] _ptr;\
	_ptr=0;}\

#ifdef DEBUG
	#ifdef _MSC_VER
		#define DEBUG_BREAK \
		#pragma message "BREAK ON __FILE__ " : " PRINTF(__LINE__) " \
		__debugbreak
	#else
		#define DEBUG_BREAK __builtin_trap
	#endif
#else
	#define DEBUG_BREAK __builtin_trap
#endif

#define SAFESTLDECL(iStlType,iStlName) \
	private:				\
	iStlType* _##iStlName;	\
	public:					\
	iStlType& iStlName

#define SAFESTLIMPL(iStlType,iStlName) \
	_##iStlName(new iStlType), \
	iStlName(*_##iStlName)

#define SAFESTLDEST(iStlName) \
	SAFEDELETE(this->_##iStlName)

#include <vector>
#include <list>
#include <functional>
#include <algorithm>
#include <cctype>
#include <cstring> //for g++ str* functions
#include <cstdio>

#ifdef __ANDROID__
	#include <dlfcn.h>
	#include <android/log.h>
	#define  LOG_TAG    "Engine"
	#define  printf(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#endif

#ifdef _MSC_VER
	#ifdef CREATEDLL
		#define DLLBUILD __declspec(dllexport)
	#else
		#define DLLBUILD __declspec(dllimport)
	#endif

	//#define LOADLIBRARY(iLibName) LoadLibrary(iLibName)
#else
	#define DLLBUILD
	//#define LOADLIBRARY(iLibName) dlopen(iLibName,RTLD_LAZY)
#endif

struct DLLBUILD  mat2;
struct DLLBUILD  mat3;
struct DLLBUILD  mat4;
struct DLLBUILD  ThreadPool;
struct DLLBUILD  Thread;

template <typename T> struct DLLBUILD THierarchy
{
	T* parent;
	THierarchy():parent(0){}
};

template <typename T> struct DLLBUILD  THierarchyVector : THierarchy<T>
{
	SAFESTLDECL(std::vector<T*>,childs);

	THierarchyVector():SAFESTLIMPL(std::vector<T*>,childs){}
	~THierarchyVector(){SAFESTLDEST(childs);}
};

template <typename T> struct DLLBUILD  THierarchyList : THierarchy<T>
{
	SAFESTLDECL(std::list<T*>,childs);

	THierarchyList():SAFESTLIMPL(std::list<T*>,childs){}
	~THierarchyList(){SAFESTLDEST(childs);}
};

template<class T,int size> struct DLLBUILD  TNumberedVectorInterface
{
	T v[size];
};

template <typename T> struct DLLBUILD  TStaticInstance
{
	static T* instance;

	TStaticInstance()
	{
		if(!instance)
			instance=(T*)this;
	}
};

template <typename T> T* TStaticInstance<T>::instance=0;

template <typename T> struct DLLBUILD TPoolVector
{
private:
	static std::vector<T*>* _pool;
public:

	TPoolVector()
	{
		this->_pool->push_back((T*)this);
	}

	~TPoolVector()
	{
		this->_pool->erase(std::find(this->_pool->begin(),this->_pool->end(),(T*)this));
	}

	static void BroadcastToPool(void (T::*func)(void*),void* data=0)
	{
		for(typename std::vector< T*>::iterator tPoolElement=TPoolVector<T>::GetPool().begin();tPoolElement!=TPoolVector<T>::GetPool().end();tPoolElement++)
			((*tPoolElement)->*func)(data);
	}

	static std::vector<T*>& GetPool()
	{
		return *TPoolVector<T>::_pool;
	}
};

template <typename T> std::vector<T*>* TPoolVector<T>::_pool=new std::vector<T*>;


struct DLLBUILD String
{
protected:
	char* data;
	size_t size;
public:

	String();
	String(const char* s);
	String(const String& s);
	String(const wchar_t* s);
	String(int number);
	String(float scalar);
	String(char* from,int t);

	~String();

	DLLBUILD friend String operator+(const String a,const String& b);

	//String& operator=(const char* s);
	String& operator=(const String& s);
	bool operator==(const String& s);
	bool operator==(const char* s);
	char operator[](int i);
	String& operator+=(const String&);



	static String Random(int iCount);
	operator char*()const;
	//operator float()const;
	const int Count()const;
	const char* Buffer()const;
	bool Contains(const char*);
	bool Alloc(int iBytes);
	bool Copy(const char* iChar);
	//operator bool();
};

struct DLLBUILD  FilePath : String
{
	FilePath():String(){}
	FilePath(const char* s):String(s){}
	FilePath(const String& s):String(s){}
	FilePath(const wchar_t* s):String(s){}
	FilePath(int number):String(number){}
	FilePath(float scalar):String(scalar){}
	FilePath(char* from,int t):String(from,t){}

	String File();
	String Name();
	String Fullpath();
	String Path();
	String Extension();
	String PointedExtension();

	String PathUp(int iLevels);
};


namespace Vector
{
	#define PI 				3.141592653589793238462643383279502
	#define PI_OVER_180	 	0.017453292519943295769236907684886

	DLLBUILD float dot(const float *a ,const float *b ,int dim);
	DLLBUILD float* cross(float* c,const float* a,const float* b);
	DLLBUILD float* sum(float *c,const float *a ,const float *b,int dim );
	DLLBUILD float* subtract(float *c,const float *a ,const float *b,int dim );
	DLLBUILD float* negate(float *b ,const float *a ,int dim);
	DLLBUILD float* make(float *v ,int dim,float x=0,float y=0,float z=0,float t=0);
	DLLBUILD float* copy(float* b,const float* a,int dim);
	DLLBUILD float length(const float *v,int dim );
	DLLBUILD float* normalize(float *b ,const float *a,int dim );
	DLLBUILD float* scale(float *b ,const float *a ,int dim,float x=1,float y=1,float z=1,float t=1);
	DLLBUILD float* scale(float *b ,const float *a ,float,int dim);
	DLLBUILD float* scale(float *c,const float *a ,const float *b,int dim );
	DLLBUILD float angleg(const float *a,const float *b,int dim);
	DLLBUILD float angler(const float *a,const float *b,int dim);
	DLLBUILD float* minimum(float *c,const float *a,const float *b,int dim);
	DLLBUILD float* maximum(float *c,const float *a,const float *b,int dim);
	DLLBUILD bool equal(const float *a,const float *b,int dim);
	//String stringize(const float *a,int dim);
	DLLBUILD void print(float* v);
};

struct DLLBUILD  vec2 : TNumberedVectorInterface<float,2>
{
	float &x,&y;

	vec2();
	vec2(const vec2& a);
	vec2(float fv[2]);
	vec2(float x,float y);
	vec2 operator=(vec2& a);
	vec2 operator+(vec2& a);
	vec2 operator-(vec2& a);
	bool operator==(vec2& a);
	bool operator!=(vec2& a);
	float& operator[](int i);
	void scale(float f);
	void scale(vec2 a);

	vec2 minimum(vec2& a);
	vec2 maximum(vec2& a);
	static float dot(vec2& a,vec2& b);
	float dot(vec2& a);
	float length();
	void normalize();
	void make(float a,float b);
	void negate();
	String stringize();
	operator float* ();
	operator void* ();
	operator char* ();
};


struct DLLBUILD  vec3 : TNumberedVectorInterface<float,3>
{
	float &x,&y,&z;

	vec3();
	vec3(const vec3& a);
	vec3(float fv[3]);
	vec3(float x,float y,float z);

	float& operator[](int i);

	bool operator==(vec3 a);
	bool operator==(vec3& a)const;
	bool operator!=(vec3& a);

	vec3& operator=(vec3 a);

	vec3 operator+(vec3& a);
	vec3& operator+=(vec3& a);
	vec3 operator-(vec3& a);
	vec3 operator-();
	vec3& operator-=(vec3& a);

	vec3 operator*(float f);
	vec3& operator*=(float f);

	vec3& scale(float f);
	vec3& scale(vec3 a);
	vec3 minimum(vec3 a);
	vec3 maximum(vec3 a);
	float length();
	static float dot(vec3& a,vec3& b);
	static vec3 cross(vec3& a,vec3& b);
	vec3& normalize();
	vec3& make(float a,float b,float c);
	//vec3& make(double a,double b,double c){VectorMathNamespace::make(v,3,(float)a,(float)b,(float)c);return *this;}
	vec3& negate();

	String stringize();

	operator float* ();
	operator void* ();
	operator char* ();

	bool iszero();
};

struct DLLBUILD  vec4 : TNumberedVectorInterface<float,4>
{
	float &x,&y,&z,&w;

	vec4();
	vec4(const vec3& a);
	vec4(const vec4& a);
	vec4(float fv[4]);
	vec4(float x,float y,float z,float t);
	vec4(float x,float y,float z);
	vec4& operator=(const vec4& a);
	vec4& operator=(const vec3& a);
	vec4 operator+(const vec4& a);
	vec4& operator+=(const vec4& a);
	vec4 operator-(const vec4& a);
	vec4& operator-=(vec4& a);
	void scale(float f);
	void scale(vec4 a);
	bool operator==(const vec4& a);
	bool operator!=(const vec4& a);
	float& operator[](int i);
	vec4 minimum(const vec4& a);
	vec4 maximum(const vec4& a);
	float length();
	static float dot(const vec4& a,const vec4& b);
	void normalize();
	void make(float a,float b,float c,float d);
	void negate();
	String stringize();
	operator float* ();
	operator void* ();
	operator char* ();
	operator long unsigned int();
	operator vec3 ();
	operator vec2 ();
};



struct DLLBUILD  AABB : TNumberedVectorInterface<vec3,2>
{
	vec3 &a,&b;

	AABB():a(v[0]),b(v[1]){}
	AABB(vec3 aa,vec3 bb):a(v[0]),b(v[1]){a=aa;b=bb;}

	bool Contains(vec3 iv);

	void Grow(AABB ab);
	void Shrink(AABB ab);
};

namespace Matrix
{
	DLLBUILD float* identity(float* m);

	DLLBUILD float* sum(float* c,float* a,float* b);

	DLLBUILD float* subtract(float* c,float* a,float* b);

	DLLBUILD float* copy(float* b,const float* a);

	DLLBUILD float* make(float* mm,float a,float b,float c,float d,float e,float f,float g,float h,float i,float l,float m,float n,float o,float p,float q,float r);

	DLLBUILD float* negate(float* b,float* a);
	DLLBUILD float* traspose(float* b,float* a);
	DLLBUILD float* translate(float*,vec3 iString);

	DLLBUILD float* scale(float* b,float* a,float x,float y,float z);
	DLLBUILD float* scale(float* b,float* a,vec3 v);
	DLLBUILD float* scale(float* b,float* a,float s);

	DLLBUILD float* set(float* m,vec3 s,vec3 r,vec3 t);

	DLLBUILD float* multiply(float* c,float* a,float* b);
	DLLBUILD void multiply(float* a,float* b);

	DLLBUILD void print(float*);

	DLLBUILD void perspective(float*,float fov,float ratio,float near,float far);
	DLLBUILD void ortho(float*,float left,float right ,float bottom,float up,float near,float far);
	DLLBUILD void lookat(float*,float posx,float posy,float posz,float centerx,float centery,float centerz,float upx,float upy,float upz);
	DLLBUILD void lookat(float*,float*,float,float,float,float,float,float);


	DLLBUILD void rotate(float* mout,float* m,float a,float x,float y,float z);
	DLLBUILD void rotate(float* m,float a,float x,float y,float z);
	DLLBUILD void rotate(float* b,float* a,vec3);
	DLLBUILD float* rotateq(float*,const float*,const float*);

	DLLBUILD float* invert(float* b,float* a);
	DLLBUILD float det(const float* m);

	DLLBUILD float* transform(float* out,float* m,float* iString);
	DLLBUILD float* transform(float* out,float* m,float,float,float);

	DLLBUILD float* orientation(float* c,float* m,float* a);
	DLLBUILD float* orientation(float* c,float* m,float x,float y,float z);
	DLLBUILD void orientations(float* m,float*,float*,float*);
};



struct DLLBUILD  mat2 : TNumberedVectorInterface<float,4>
{
	float &m11,&m12,&m21,&m22;

	mat2();
};



struct DLLBUILD  mat3 : TNumberedVectorInterface<float,9>
{

	float &m11,&m12,&m13,&m21,&m22,&m23,&m31,&m32,&m33;

	mat3();
	mat3(float*);
	mat3(const mat3&);
	mat3(mat4);

	mat3& operator=(mat3);
	mat3& operator=(mat4);

	mat3& identity();

	float* operator[](int i);
	operator float* (){return v;}
};

struct DLLBUILD  mat4 : TNumberedVectorInterface<float,16>
{
	//float &m11,&m12,&m13,&m14,&m21,&m22,&m23,&m24,&m31,&m32,&m33,&m34,&m41,&m42,&m43,&m44;

	mat4();
	mat4(const mat4&);
	mat4(mat3);
	mat4(const float*);
	mat4(const double*);

	mat4& operator=(const mat4&);
	mat4& operator=(mat3);
	mat4 operator*(mat4);
	mat4 operator*(float);
	vec4 operator*(vec4);
	mat4& operator*=(mat4);
	mat4& operator*=(float);
	mat4& operator*=(vec3);

	mat4 operator+(mat4&);
	mat4& operator+=(mat4&);
	mat4 operator-(mat4&);
	mat4& operator-=(mat4&);
	mat4 operator-();
	bool 	operator==(mat4&);
	float* 	operator[](int);

	mat4& identity();
	mat4& identity33();
	vec3 transform(vec3 iString);
	void transformself(vec3& inout);
	vec3 transform(float,float,float);
	mat4& traspose();
	mat4& rotate(float a,float x,float y,float z);
	mat4& rotate(float a,vec3);
	mat4& rotate(vec3);
	mat4& copy(float* );
	mat4& copy33(mat4&);
	mat4& move(vec3);
	mat4& translate(float x,float y,float z);
	mat4& translate(vec3 v);
	mat4& scale(float);
	mat4& scale(vec3);
	mat4& invert();
	mat4 inverse();
	mat4& srt(vec3 s,vec3 r,vec3 t);
	mat4& trs(vec3 t,vec3 r,vec3 s);
	vec3 position();
	void axes(vec3& a,vec3& b,vec3& c);
	vec3 axis(vec3 iString);
	vec3 axis(float,float,float);
	void print();
	mat4& perspective(float fov,float ratio,float near,float far);
	//mat4& lookat(vec3 target,vec3 pos,vec3 up);
	mat4& lookat(vec3 target,vec3 up=vec3(0,0,1));
	operator float* (){return v;}
	mat4& ortho(float left, float right,float bottom, float top,float near, float far);
	void zero();
};

struct DLLBUILD  Timer : TStaticInstance<Timer>
{
	unsigned int currentFrameTime;
	unsigned int currentFrameDeltaTime;
	unsigned int lastFrameTime;
	unsigned int renderFps;

	Timer();

	virtual unsigned int GetTime()=0;
	virtual void update();
};

struct DLLBUILD  Task
{
	std::function<void()> func;
	bool remove;
	bool executing;
	bool pause;
	Thread* owner;

	void Block(bool);
};

struct DLLBUILD  Thread  : TPoolVector<Thread>
{
	int id;
	bool pause;
	std::list<Task*> tasks;
	Task* executing;
	unsigned int sleep;

	Thread();
	~Thread();

	Task* NewTask(std::function<void()>,bool remove=true,bool iBlock=false);

	void Block(bool);
};


vec4 plane(vec3 a,vec3 b,vec3 c);
bool isPointOnPlane(vec4 plane,vec3 point);
bool isPointOnLine(vec3 a,vec3 b,vec3 point);
void eqSolve(float* result,int nrow,int ncol,float** eqsys);

void printEqSys(int nrow,int ncol,float** eqsys);


#endif //PRIMITIVES_H




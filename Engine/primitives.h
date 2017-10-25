#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "t_datatypes.h"

#define PRINT(x) #x
#define PRINTF(x) PRINT(x)

#define LOCATION " @mic (" __FILE__ " : " PRINTF(__LINE__) ")"



struct StringWide
{
	wchar_t *data;

	StringWide():data(0){};
	~StringWide(){SAFEDELETEARRAY(data);}

	operator const wchar_t*(){return data;}
};

struct String
{
private:
	char* data;
public:

	String();
	String(const char* s);
	String(int,const char* s,...);
	String(const String& s);
	String(const wchar_t* s);
	String(int number);
	String(float scalar);

	~String();

	String& operator=(const char* s);
	String& operator=(const String& s);
	bool operator==(const char* s);
	char operator[](int i);
	friend String operator+(const String a,const String& b);
	String& operator+=(const String&);

	operator char*()const;
	operator float()const;
	int Count()const;
	const char* Buf()const;
	bool Contains(const char*);
	wchar_t* Wstring(int& oSize);

};


namespace VectorMathNamespace
{
	#define PI 				3.141592653589793238462643383279502
	#define PI_OVER_180	 	0.017453292519943295769236907684886

	float dot(const float *a ,const float *b ,int dim);
	float* cross(float* c,const float* a,const float* b);
	float* sum(float *c,const float *a ,const float *b,int dim );
	float* subtract(float *c,const float *a ,const float *b,int dim );
	float* negate(float *b ,const float *a ,int dim);
	float* make(float *v ,int dim,float x=0,float y=0,float z=0,float t=0);
	float* copy(float* b,const float* a,int dim);
	float length(const float *v,int dim );
	float* normalize(float *b ,const float *a,int dim );
	float* scale(float *b ,const float *a ,int dim,float x=1,float y=1,float z=1,float t=1);
	float* scale(float *b ,const float *a ,float,int dim);
	float* scale(float *c,const float *a ,const float *b,int dim );
	float angleg(const float *a,const float *b,int dim);
	float angler(const float *a,const float *b,int dim);
	float* minimum(float *c,const float *a,const float *b,int dim);
	float* maximum(float *c,const float *a,const float *b,int dim);
	bool equal(const float *a,const float *b,int dim);
	//String stringize(const float *a,int dim);
	void print(float* v);
};

struct vec2 : TNumberedVectorInterface<float,2>
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


struct vec3 : TNumberedVectorInterface<float,3>
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

struct vec4 : TNumberedVectorInterface<float,4>
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



struct AABB : TNumberedVectorInterface<vec3,2>
{
	vec3 &a,&b;

	AABB():a(v[0]),b(v[1]){}
	AABB(vec3 aa,vec3 bb):a(v[0]),b(v[1]){a=aa;b=bb;}

	bool Contains(vec3 iv);

	void Grow(AABB ab);
	void Shrink(AABB ab);
};

struct mat2;
struct mat3;
struct mat4;

namespace MatrixMathNamespace
{
	float* identity(float* m);

	float* sum(float* c,float* a,float* b);

	float* subtract(float* c,float* a,float* b);

	float* copy(float* b,const float* a);

	float* make(float* mm,float a,float b,float c,float d,float e,float f,float g,float h,float i,float l,float m,float n,float o,float p,float q,float r);

	float* negate(float* b,float* a);
	float* traspose(float* b,float* a);
	float* translate(float*,vec3 in);

	float* scale(float* b,float* a,float x,float y,float z);
	float* scale(float* b,float* a,vec3 v);
	float* scale(float* b,float* a,float s);

	float* set(float* m,vec3 s,vec3 r,vec3 t);

	float* multiply(float* c,float* a,float* b);
	void multiply(float* a,float* b);

	void print(float*);

	void perspective(float*,float fov,float ratio,float near,float far);
	void ortho(float*,float left,float right ,float bottom,float up,float near,float far);
	void lookat(float*,float posx,float posy,float posz,float centerx,float centery,float centerz,float upx,float upy,float upz);
	void lookat(float*,float*,float,float,float,float,float,float);


	void rotate(float* mout,float* m,float a,float x,float y,float z);
	void rotate(float* m,float a,float x,float y,float z);
	void rotate(float* b,float* a,vec3);
	float* rotateq(float*,const float*,const float*);

	float* invert(float* b,float* a);
	float det(const float* m);

	float* transform(float* out,float* m,float* in);
	float* transform(float* out,float* m,float,float,float);

	float* orientation(float* c,float* m,float* a);
	float* orientation(float* c,float* m,float x,float y,float z);
	void orientations(float* m,float*,float*,float*);
};



struct mat2 : TNumberedVectorInterface<float,4>
{
	float &m11,&m12,&m21,&m22;
};



struct mat3 : TNumberedVectorInterface<float,9>
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

struct mat4 : TNumberedVectorInterface<float,16>
{
	float &m11,&m12,&m13,&m14,&m21,&m22,&m23,&m24,&m31,&m32,&m33,&m34,&m41,&m42,&m43,&m44;

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
	vec3 transform(vec3 in);
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
	vec3 axis(vec3 in);
	vec3 axis(float,float,float);
	void print();
	mat4& perspective(float fov,float ratio,float near,float far);
	//mat4& lookat(vec3 target,vec3 pos,vec3 up);
	mat4& lookat(vec3 target,vec3 up=vec3(0,0,1));
	operator float* (){return v;}
	mat4& ortho(float left, float right,float bottom, float top,float near, float far);
	void zero();
};

struct Timer : TStaticInstance<Timer>
{
	unsigned int currentFrameTime;
	unsigned int currentFrameDeltaTime;
	unsigned int lastFrameTime;
	unsigned int renderFps;

	Timer():renderFps(60){}

	virtual unsigned int GetTime()=0;

	void update(){};
};


vec4 plane(vec3 a,vec3 b,vec3 c);
bool isPointOnPlane(vec4 plane,vec3 point);
bool isPointOnLine(vec3 a,vec3 b,vec3 point);
void eqSolve(float* result,int nrow,int ncol,float** eqsys);

void printEqSys(int nrow,int ncol,float** eqsys);


struct Cluster;
struct Influence;
struct Keyframe;

struct Entity;
struct EntityComponent;
struct Animation;
struct Root;
struct Skeleton;
struct BoneSkeleton;
struct Bone;
struct Mesh;
struct Skin;
struct Texture;
struct Light;
struct Camera;

struct RenderSurface;
struct GuiViewport;
struct GuiRect;

#endif //PRIMITIVES_H




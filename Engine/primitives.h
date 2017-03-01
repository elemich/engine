#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "t_datatypes.h"

struct String
{
private:
	char* data;
public:

	String();
	String(const char* s);
	String(int,const char* s,...);
	String(const String& s);
	String(int number);
	String(float scalar);

	void operator=(const char* s);
	bool operator==(const char* s);
	char operator[](int i);
	String operator+(const char* b);

	operator char*()const;
	operator float()const;
	int Count()const;
	const char* Buf()const;
	bool Contains(const char*);

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
	float* minimum(float *c,float *a,float *b,int dim);
	float* maximum(float *c,float *a,float *b,int dim);
	bool equal(const float *a,const float *b,int dim);
	//String stringize(const float *a,int dim);
	void print(float* v);
};





struct vec2 : TNumberedVectorInterface<float,2>
{
	float &x,&y;

	vec2():x(v[0]),y(v[1]){VectorMathNamespace::make(v,2,0,0);}
	vec2(const vec2& a):x(v[0]),y(v[1]){VectorMathNamespace::copy(v,a.v,2);}
	vec2(float fv[2]):x(v[0]),y(v[1]){VectorMathNamespace::copy(v,fv,2);}
	vec2(float x,float y):x(v[0]),y(v[1]){VectorMathNamespace::make(v,2,x,y);}
	vec2 operator=(vec2& a){VectorMathNamespace::copy(v,a.v,2);return *this;}
	vec2 operator+(vec2& a){vec2 r;VectorMathNamespace::sum(r,a.v,v,2);return r;}
	vec2 operator-(vec2& a){vec2 r;VectorMathNamespace::subtract(r,a.v,v,2);return r;}
	bool operator==(vec2& a){return VectorMathNamespace::equal(a.v,v,2);}
	bool operator!=(vec2& a){return !VectorMathNamespace::equal(a.v,v,2);}
	float& operator[](int i){return v[i];}
	void scale(float f){VectorMathNamespace::scale(v,v,f,2);}
	void scale(vec2 a){VectorMathNamespace::scale(v,a.v,v,2);}

	vec2 minimum(vec2& a){vec2 r; VectorMathNamespace::minimum(r,a.v,v,2);return r;}
	vec2 maximum(vec2& a){vec2 r; VectorMathNamespace::maximum(r,a.v,v,2);return r;}
	static float dot(vec2& a,vec2& b){return VectorMathNamespace::dot(a.v,b.v,2);}
	float dot(vec2& a){return VectorMathNamespace::dot(a.v,v,2);}
	float length(){return VectorMathNamespace::length(v,2);}
	void normalize(){VectorMathNamespace::normalize(v,v,2);}
	void make(float a,float b){VectorMathNamespace::make(v,2,a,b);}
	void negate(){VectorMathNamespace::negate(v,v,2);}
	String stringize(){char str[100];/*sprintf(str,"%g,%g",v[0],v[1]);*/String s(str);return s;}
	operator float* (){return v;}
	operator void* (){return v;}
	operator char* (){return stringize();}
};


struct vec3 : TNumberedVectorInterface<float,3>
{
	float &x,&y,&z;

	vec3():x(v[0]),y(v[1]),z(v[2]){VectorMathNamespace::make(v,3,0,0,0);}
	vec3(const vec3& a):x(v[0]),y(v[1]),z(v[2]){VectorMathNamespace::copy(v,a.v,3);}
	vec3(float fv[3]):x(v[0]),y(v[1]),z(v[2]){VectorMathNamespace::copy(v,fv,3);}
	vec3(float x,float y,float z):x(v[0]),y(v[1]),z(v[2]){VectorMathNamespace::make(v,3,x,y,z);}

	float& operator[](int i){return v[i];}

	bool operator==(vec3 a){return VectorMathNamespace::equal(v,a.v,3);}
	bool operator==(vec3& a)const{return VectorMathNamespace::equal(a.v,v,3);}
	bool operator!=(vec3& a){return !VectorMathNamespace::equal(a.v,v,3);}

	vec3& operator=(vec3 a){VectorMathNamespace::copy(v,a.v,3);return *this;}
	
	vec3 operator+(vec3& a){vec3 r;VectorMathNamespace::sum(r,v,a.v,3);return r;}
	vec3& operator+=(vec3& a){VectorMathNamespace::sum(v,v,a.v,3);return *this;}
	vec3 operator-(vec3& a){vec3 r;VectorMathNamespace::subtract(r,v,a.v,3);return r;}
	vec3 operator-(){vec3 r;VectorMathNamespace::negate(r,v,3);return r;}
	vec3& operator-=(vec3& a){VectorMathNamespace::subtract(v,v,a.v,3);return *this;}
	
	vec3 operator*(float f){vec3 g;VectorMathNamespace::scale(g,v,f,3);return g;}
	vec3& operator*=(float f){VectorMathNamespace::scale(v,v,f,3);return *this;}

	vec3& scale(float f){VectorMathNamespace::scale(v,v,f,3);return *this;}
	vec3& scale(vec3 a){VectorMathNamespace::scale(v,a.v,v,3);return *this;}
	vec3 minimum(vec3 a){vec3 r; VectorMathNamespace::minimum(r,a.v,v,3);return r;}
	vec3 maximum(vec3 a){vec3 r; VectorMathNamespace::maximum(r,a.v,v,3);return r;}
	float length(){return VectorMathNamespace::length(v,3);}
	static float dot(vec3& a,vec3& b){return VectorMathNamespace::dot(a.v,b.v,3);}
	static vec3 cross(vec3& a,vec3& b){vec3 c;VectorMathNamespace::cross(c,a.v,b.v);return c;}
	vec3& normalize(){VectorMathNamespace::normalize(v,v,3);return *this;}
	vec3& make(float a,float b,float c){VectorMathNamespace::make(v,3,a,b,c);return *this;}
	//vec3& make(double a,double b,double c){VectorMathNamespace::make(v,3,(float)a,(float)b,(float)c);return *this;}
	vec3& negate(){VectorMathNamespace::negate(v,v,3);return *this;}

	String stringize(){char str[100];/*sprintf(str,"%g,%g,%g",v[0],v[1],v[2]);*/String s(str);return s;}
	
	operator float* (){return v;}
	operator void* (){return v;}
	operator char* (){return stringize();}

	bool iszero(){return (v[0]==0 && v[1]==0 && v[2]==0);}
};

struct vec4 : TNumberedVectorInterface<float,4>
{
	float &x,&y,&z,&w;

	vec4():x(v[0]),y(v[1]),z(v[2]),w(v[3]){VectorMathNamespace::make(v,4,0,0,0,0);}
	vec4(const vec4& a):x(v[0]),y(v[1]),z(v[2]),w(v[3]){VectorMathNamespace::copy(v,a.v,4);}
	vec4(vec3& a):x(v[0]),y(v[1]),z(v[2]),w(v[3]){VectorMathNamespace::make(v,4,a[0],a[1],a[2],1.0f);}
	vec4(float fv[4]):x(v[0]),y(v[1]),z(v[2]),w(v[3]){VectorMathNamespace::copy(v,fv,4);}
	vec4(float x,float y,float z,float t):x(v[0]),y(v[1]),z(v[2]),w(v[3]){VectorMathNamespace::make(v,4,x,y,z,t);}
	vec4(float x,float y,float z):x(v[0]),y(v[1]),z(v[2]),w(v[3]){VectorMathNamespace::make(v,4,x,y,z,1.0f);}
	vec4& operator=(vec4& a){VectorMathNamespace::copy(v,a.v,4);return *this;}
	vec4& operator=(vec3& a){VectorMathNamespace::copy(v,a.v,3);v[3]=1.0f;return *this;}
	vec4 operator+(vec4& a){vec4 r;VectorMathNamespace::sum(r,a.v,v,4);return r;}
	vec4 operator-(vec4& a){vec4 r;VectorMathNamespace::subtract(r,a.v,v,4);return r;}
	void scale(float f){VectorMathNamespace::scale(v,v,f,4);}
	void scale(vec4 a){VectorMathNamespace::scale(v,a.v,v,4);}
	bool operator==(vec4& a){return VectorMathNamespace::equal(a.v,v,4);}
	bool operator!=(vec4& a){return !VectorMathNamespace::equal(a.v,v,4);}
	float& operator[](int i){return v[i];}
	vec4 minimum(vec4& a){vec4 r; VectorMathNamespace::minimum(r,a.v,v,4);return r;}
	vec4 maximum(vec4& a){vec4 r; VectorMathNamespace::maximum(r,a.v,v,4);return r;}
	float length(){return VectorMathNamespace::length(v,4);}
	static float dot(vec4& a,vec4& b){return VectorMathNamespace::dot(a.v,b.v,4);}
	void normalize(){VectorMathNamespace::normalize(v,v,4);}
	void make(float a,float b,float c,float d){VectorMathNamespace::make(v,4,a,b,c,d);}
	void negate(){VectorMathNamespace::negate(v,v,4);}
	String stringize(){char str[100];/*sprintf(str,"%g,%g,%g,%g",v[0],v[1],v[2],v[3]);*/String s(str);return s;}
	operator float* (){return v;}
	operator void* (){return v;}
	operator char* (){return stringize();}
	operator long unsigned int()
	{
		long unsigned int i;
		i|=(((i>>0)|(unsigned char)((v[0]<0 ? 0 : (v[0]>1 ? 1 : v[0]))*255))<<0);
		i|=(((i>>8)|(unsigned char)((v[1]<0 ? 0 : (v[1]>1 ? 1 : v[1]))*255))<<8);
		i|=(((i>>16)|(unsigned char)((v[2]<0 ? 0 : (v[2]>1 ? 1 : v[2]))*255))<<16);
		i|=(((i>>24)|(unsigned char)((v[3]<0 ? 0 : (v[3]>1 ? 1 : v[3]))*255))<<24);
		return i;
	}
	operator vec3& (){return *this;}
	operator vec2& (){return *this;}
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

	void perspective(float*,float left,float right ,float bottom,float up,float near,float far);
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
	mat4 operator*(vec3);
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
	mat4& perspective(float left,float right ,float bottom,float up,float near,float far);
	//mat4& lookat(vec3 target,vec3 pos,vec3 up);
	mat4& lookat(vec3 target);
	operator float* (){return v;}
	mat4& ortho(float left, float right,float bottom, float top,float near, float far);
	void zero();
};

struct Entity;
struct Bone;
struct Mesh;
struct Skin;
struct Texture;
struct Light;

#endif //PRIMITIVES_H




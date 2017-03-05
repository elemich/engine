#include "primitives.h"

String::String():data(NULL){}
String::String(const char* s){if(!s){data=NULL;return;}data=new char[strlen(s)+1];strcpy(data,s);}
String::String(int size,const char* s,...)
{
	__debugbreak();
	/*va_list vl;
	va_start(vl,s);
	char t[size];
	vsnprintf(t,size,s,vl);
	va_end(vl);
	data=new char[strlen(t)+1];
	strcpy(data,t);*/
}
String::String(const String& s)
{
    if(s.data)
    {
        data=new char[strlen(s.Buf())+1];
        strcpy(data,s.Buf());
        return;
    }
    else
    {
        data=NULL;
        return;
    }
}
String::String(int number)
{
    char n[100];
    sprintf(n,"%i",number);
    data=new char[strlen(n)+1];
    strcpy(data,n);
}
String::String(float scalar)
{
    char n[100];
    sprintf(n,"%g",scalar);
    data=new char[strlen(n)+1];
    strcpy(data,n);
}

String::~String()
{
	SAFEDELETEARRAY(data);
}

void String::operator=(const char* s)
{
    if(s)
    {
        delete [] data;
        data=NULL;
    }
    data=new char[strlen(s)+1];
    strcpy(data,s);
}

bool String::operator==(const char* s)
{
    if(strcmp(data,s)==0)return true;
        return false;
}

char String::operator[](int i)
{
    if(i<(int)strlen(data))
    {
        return data[i];
    }
    else
        return '\0';}
String String::operator+(const char* b)
{
    char* ab=new char[Count()+strlen(b)+1];
    strcpy(ab,Buf());
    strcpy(&ab[Count()],b);
    String s(ab);
    return s;
}

String::operator float()const
{
    return (float)atof(data);
}

String::operator char*()const{return data;}
int String::Count()const{return strlen(data);}
const char* String::Buf()const{return data;}

bool String::Contains(const char* in)
{
	if(!in || !data)
		return 0;

	return strstr(data,in) ? true : false;
}



//---------------------------------


float* VectorMathNamespace::cross(float* c,const float* a,const float* b)
{
    c[0]=a[1]*b[2]-a[2]*b[1];
    c[1]=a[2]*b[0]-a[0]*b[2];
    c[2]=a[0]*b[1]-a[1]*b[0];

    return c;
}


float* VectorMathNamespace::sum(float *c,const float *a ,const float *b ,int dim)
{
    for(int i=0;i<dim;i++)c[i]=a[i]+b[i];
    return c;
}

float* VectorMathNamespace::subtract(float *c,const float *a ,const float *b,int dim )
{
    for(int i=0;i<dim;i++)c[i]=a[i]-b[i];
    return c;
}

float* VectorMathNamespace::negate(float *b ,const float *a ,int dim)
{
    for(int i=0;i<dim;i++)b[i]=-a[i];
    return b;
}

float* VectorMathNamespace::make(float *v ,int dim,float x,float y,float z,float t)
{
    float tt[4]={x,y,z,t};
    for(int i=0;i<dim;i++)
    v[i]=tt[i];
    return v;
}


float VectorMathNamespace::angleg(const float *a,const float *b,int dim)
{
    return acos(dot(a,b,dim)/length(a,dim) * length(b,dim)) * (float)(180.0/PI);
}


float VectorMathNamespace::angler(const float *a,const float *b,int dim)
{
    return acos(dot(a,b,dim)/(length(a,dim) * length(b,dim)));
}


float* VectorMathNamespace::copy(float* b,const float* a,int dim)
{
	for(int i=0;i<dim;i++)
		b[i]=a[i];
    return b;
}



float VectorMathNamespace::dot(const float *a ,const float *b,int dim )
{
    float ret=0.0f;
    for(int i=0;i<dim;i++)
    {
        ret+=a[i]*b[i];
    }
    return ret;
}

void VectorMathNamespace::print(float* v){/*printf("%g,%g,%g\n",v[0],v[1],v[2]);*/}


float VectorMathNamespace::length(const float *v,int dim )
{
    float arg=0.0f;
    for(int i=0;i<dim;i++)
        arg+=v[i]*v[i];
    float result=sqrt(arg);
    return result;
}


float* VectorMathNamespace::normalize(float *b ,const float *a,int dim )
{
    float d=length(a,dim);

    for(int i=0;i<dim;i++)
        b[i]=a[i]/d;

    return b;
}


float* VectorMathNamespace::scale(float *b ,const float *a ,int dim,float x,float y,float z,float t)
{
    float v[4]={x,y,z,t};

    for(int i=0;i<dim;i++)
        b[i]=a[i]*v[i];

    return b;
}


float* VectorMathNamespace::scale(float *c,const float *a ,const float *b,int dim )
{
    for(int i=0;i<dim;i++)
        c[i]=a[i]*b[i];
    return c;
}


float* VectorMathNamespace::scale(float *b ,const float *a ,float s,int dim)
{
    for(int i=0;i<dim;i++)
            b[i]=a[i]*s;
    return b;
}



float* VectorMathNamespace::minimum(float *c,float *a,float *b,int dim)
{
    for(int i=0;i<dim;i++)
    c[i]=(a[i]<b[i] ? a[i] : b[i]);

    return c;
}


float* VectorMathNamespace::maximum(float *c,float *a,float *b,int dim)
{
    for(int i=0;i<dim;i++)
    c[i]=(a[i]>b[i] ? a[i] : b[i]);

    return c;
}


bool VectorMathNamespace::equal(const float *a,const float *b,int dim)
{
    for(int i=0;i<dim;i++)
        if(a[1]!=b[i])return false;
    return true;
}





vec2::vec2():x(v[0]),y(v[1]){VectorMathNamespace::make(v,2,0,0);}
vec2::vec2(const vec2& a):x(v[0]),y(v[1]){VectorMathNamespace::copy(v,a.v,2);}
vec2::vec2(float fv[2]):x(v[0]),y(v[1]){VectorMathNamespace::copy(v,fv,2);}
vec2::vec2(float x,float y):x(v[0]),y(v[1]){VectorMathNamespace::make(v,2,x,y);}
vec2 vec2::operator=(vec2& a){VectorMathNamespace::copy(v,a.v,2);return *this;}
vec2 vec2::operator+(vec2& a){vec2 r;VectorMathNamespace::sum(r,a.v,v,2);return r;}
vec2 vec2::operator-(vec2& a){vec2 r;VectorMathNamespace::subtract(r,a.v,v,2);return r;}
bool vec2::operator==(vec2& a){return VectorMathNamespace::equal(a.v,v,2);}
bool vec2::operator!=(vec2& a){return !VectorMathNamespace::equal(a.v,v,2);}
float& vec2::operator[](int i){return v[i];}
void vec2::scale(float f){VectorMathNamespace::scale(v,v,f,2);}
void vec2::scale(vec2 a){VectorMathNamespace::scale(v,a.v,v,2);}
vec2 vec2::minimum(vec2& a){vec2 r; VectorMathNamespace::minimum(r,a.v,v,2);return r;}
vec2 vec2::maximum(vec2& a){vec2 r; VectorMathNamespace::maximum(r,a.v,v,2);return r;}
float vec2::dot(vec2& a,vec2& b){return VectorMathNamespace::dot(a.v,b.v,2);}
float vec2::dot(vec2& a){return VectorMathNamespace::dot(a.v,v,2);}
float vec2::length(){return VectorMathNamespace::length(v,2);}
void vec2::normalize(){VectorMathNamespace::normalize(v,v,2);}
void vec2::make(float a,float b){VectorMathNamespace::make(v,2,a,b);}
void vec2::negate(){VectorMathNamespace::negate(v,v,2);}
String vec2::stringize(){char str[100];/*sprintf(str,"%g,%g",v[0],v[1]);*/String s(str);return s;}
vec2::operator float* (){return v;}
vec2::operator void* (){return v;}
vec2::operator char* (){return stringize();}


vec3::vec3():x(v[0]),y(v[1]),z(v[2]){VectorMathNamespace::make(v,3,0,0,0);}
vec3::vec3(const vec3& a):x(v[0]),y(v[1]),z(v[2]){VectorMathNamespace::copy(v,a.v,3);}
vec3::vec3(float fv[3]):x(v[0]),y(v[1]),z(v[2]){VectorMathNamespace::copy(v,fv,3);}
vec3::vec3(float x,float y,float z):x(v[0]),y(v[1]),z(v[2]){VectorMathNamespace::make(v,3,x,y,z);}
float& vec3::operator[](int i){return v[i];}
bool vec3::operator==(vec3 a){return VectorMathNamespace::equal(v,a.v,3);}
bool vec3::operator==(vec3& a)const{return VectorMathNamespace::equal(a.v,v,3);}
bool vec3::operator!=(vec3& a){return !VectorMathNamespace::equal(a.v,v,3);}
vec3& vec3::operator=(vec3 a){VectorMathNamespace::copy(v,a.v,3);return *this;}
vec3 vec3::operator+(vec3& a){vec3 r;VectorMathNamespace::sum(r,v,a.v,3);return r;}
vec3& vec3::operator+=(vec3& a){VectorMathNamespace::sum(v,v,a.v,3);return *this;}
vec3 vec3::operator-(vec3& a){vec3 r;VectorMathNamespace::subtract(r,v,a.v,3);return r;}
vec3 vec3::operator-(){vec3 r;VectorMathNamespace::negate(r,v,3);return r;}
vec3& vec3::operator-=(vec3& a){VectorMathNamespace::subtract(v,v,a.v,3);return *this;}
vec3 vec3::operator*(float f){vec3 g;VectorMathNamespace::scale(g,v,f,3);return g;}
vec3& vec3::operator*=(float f){VectorMathNamespace::scale(v,v,f,3);return *this;}
vec3& vec3::scale(float f){VectorMathNamespace::scale(v,v,f,3);return *this;}
vec3& vec3::scale(vec3 a){VectorMathNamespace::scale(v,a.v,v,3);return *this;}
vec3 vec3::minimum(vec3 a){vec3 r; VectorMathNamespace::minimum(r,a.v,v,3);return r;}
vec3 vec3::maximum(vec3 a){vec3 r; VectorMathNamespace::maximum(r,a.v,v,3);return r;}
float vec3::length(){return VectorMathNamespace::length(v,3);}
float vec3::dot(vec3& a,vec3& b){return VectorMathNamespace::dot(a.v,b.v,3);}
vec3 vec3::cross(vec3& a,vec3& b){vec3 c;VectorMathNamespace::cross(c,a.v,b.v);return c;}
vec3& vec3::normalize(){VectorMathNamespace::normalize(v,v,3);return *this;}
vec3& vec3::make(float a,float b,float c){VectorMathNamespace::make(v,3,a,b,c);return *this;}
//vec3& make(double a,double b,double c){VectorMathNamespace::make(v,3,(float)a,(float)b,(float)c);return *this;}
vec3& vec3::negate(){VectorMathNamespace::negate(v,v,3);return *this;}
String vec3::stringize(){char str[100];/*sprintf(str,"%g,%g,%g",v[0],v[1],v[2]);*/String s(str);return s;}
vec3::operator float* (){return v;}
vec3::operator void* (){return v;}
vec3::operator char* (){return stringize();}
bool vec3::iszero(){return (v[0]==0 && v[1]==0 && v[2]==0);}

vec4::vec4():x(v[0]),y(v[1]),z(v[2]),w(v[3]){VectorMathNamespace::make(v,4,0,0,0,0);}
vec4::vec4(const vec4& a):x(v[0]),y(v[1]),z(v[2]),w(v[3]){VectorMathNamespace::copy(v,a.v,4);}
vec4::vec4(vec3& a):x(v[0]),y(v[1]),z(v[2]),w(v[3]){VectorMathNamespace::make(v,4,a[0],a[1],a[2],1.0f);}
vec4::vec4(float fv[4]):x(v[0]),y(v[1]),z(v[2]),w(v[3]){VectorMathNamespace::copy(v,fv,4);}
vec4::vec4(float x,float y,float z,float t):x(v[0]),y(v[1]),z(v[2]),w(v[3]){VectorMathNamespace::make(v,4,x,y,z,t);}
vec4::vec4(float x,float y,float z):x(v[0]),y(v[1]),z(v[2]),w(v[3]){VectorMathNamespace::make(v,4,x,y,z,1.0f);}
vec4& vec4::operator=(vec4& a){VectorMathNamespace::copy(v,a.v,4);return *this;}
vec4& vec4::operator=(vec3& a){VectorMathNamespace::copy(v,a.v,3);v[3]=1.0f;return *this;}
vec4 vec4::operator+(vec4& a){vec4 r;VectorMathNamespace::sum(r,a.v,v,4);return r;}
vec4 vec4::operator-(vec4& a){vec4 r;VectorMathNamespace::subtract(r,a.v,v,4);return r;}
void vec4::scale(float f){VectorMathNamespace::scale(v,v,f,4);}
void vec4::scale(vec4 a){VectorMathNamespace::scale(v,a.v,v,4);}
bool vec4::operator==(vec4& a){return VectorMathNamespace::equal(a.v,v,4);}
bool vec4::operator!=(vec4& a){return !VectorMathNamespace::equal(a.v,v,4);}
float& vec4::operator[](int i){return v[i];}
vec4 vec4::minimum(vec4& a){vec4 r; VectorMathNamespace::minimum(r,a.v,v,4);return r;}
vec4 vec4::maximum(vec4& a){vec4 r; VectorMathNamespace::maximum(r,a.v,v,4);return r;}
float vec4::length(){return VectorMathNamespace::length(v,4);}
float vec4::dot(vec4& a,vec4& b){return VectorMathNamespace::dot(a.v,b.v,4);}
void vec4::normalize(){VectorMathNamespace::normalize(v,v,4);}
void vec4::make(float a,float b,float c,float d){VectorMathNamespace::make(v,4,a,b,c,d);}
void vec4::negate(){VectorMathNamespace::negate(v,v,4);}
String vec4::stringize(){char str[100];/*sprintf(str,"%g,%g,%g,%g",v[0],v[1],v[2],v[3]);*/String s(str);return s;}
vec4::operator float* (){return v;}
vec4::operator void* (){return v;}
vec4::operator char* (){return stringize();}
vec4::operator long unsigned int()
{
	long unsigned int i;
	i|=(((i>>0)|(unsigned char)((v[0]<0 ? 0 : (v[0]>1 ? 1 : v[0]))*255))<<0);
	i|=(((i>>8)|(unsigned char)((v[1]<0 ? 0 : (v[1]>1 ? 1 : v[1]))*255))<<8);
	i|=(((i>>16)|(unsigned char)((v[2]<0 ? 0 : (v[2]>1 ? 1 : v[2]))*255))<<16);
	i|=(((i>>24)|(unsigned char)((v[3]<0 ? 0 : (v[3]>1 ? 1 : v[3]))*255))<<24);
	return i;
}
vec4::operator vec3 (){return vec3(x,y,z);}
vec4::operator vec2 (){return vec2(x,y);}








float* MatrixMathNamespace::identity(float* m)
{
    m[0]=1;
    m[1]=0;
    m[2]=0;
    m[3]=0;
    m[4]=0;
    m[5]=1;
    m[6]=0;
    m[7]=0;
    m[8]=0;
    m[9]=0;
    m[10]=1;
    m[11]=0;
    m[12]=0;
    m[13]=0;
    m[14]=0;
    m[15]=1;
    return m;
}

float* MatrixMathNamespace::make(float* mm,float a,float b,float c,float d,float e,float f,float g,float h,float i,float l,float m,float n,float o,float p,float q,float r)
{
    mm[0]=a;
    mm[1]=b;
    mm[2]=c;
    mm[3]=d;
    mm[4]=e;
    mm[5]=f;
    mm[6]=g;
    mm[7]=h;
    mm[8]=i;
    mm[9]=l;
    mm[10]=m;
    mm[11]=n;
    mm[12]=o;
    mm[13]=p;
    mm[14]=q;
    mm[15]=r;

    return mm;
}

float* MatrixMathNamespace::sum(float* c,float* a,float* b)
{
    c[0]=a[0]+b[0];		c[1]=a[1]+b[1];		c[2]=a[2]+b[2];		c[3]=a[3]+b[3];
    c[4]=a[4]+b[4];		c[5]=a[5]+b[5];		c[6]=a[6]+b[6];		c[7]=a[7]+b[7];
    c[8]=a[8]+b[8];		c[9]=a[9]+b[9];		c[10]=a[10]+b[10];	c[11]=a[11]+b[11];
    c[12]=a[12]+b[12];	c[13]=a[13]+b[13];	c[14]=a[14]+b[14];	c[15]=a[15]+b[15];

    return c;
}

float* MatrixMathNamespace::subtract(float* c,float* a,float* b)
{
    c[0]=a[0]-b[0];		c[1]=a[1]-b[1];		c[2]=a[2]-b[2];		c[3]=a[3]-b[3];
    c[4]=a[4]-b[4];		c[5]=a[5]-b[5];		c[6]=a[6]-b[6];		c[7]=a[7]-b[7];
    c[8]=a[8]-b[8];		c[9]=a[9]-b[9];		c[10]=a[10]-b[10];	c[11]=a[11]-b[11];
    c[12]=a[12]-b[12];	c[13]=a[13]-b[13];	c[14]=a[14]-b[14];	c[15]=a[15]-b[15];

	return c;
}

float* MatrixMathNamespace::copy(float* b,const float* a)
{
	// if(a==b)return b;

    b[0]=a[0];		b[1]=a[1];		b[2]=a[2];		b[3]=a[3];
    b[4]=a[4];		b[5]=a[5];		b[6]=a[6];		b[7]=a[7];
    b[8]=a[8];		b[9]=a[9];		b[10]=a[10];	b[11]=a[11];
    b[12]=a[12];	b[13]=a[13];	b[14]=a[14];	b[15]=a[15];
	return b;
}

float* MatrixMathNamespace::negate(float* b,float* a)
{
    b[0]=-a[0];
    b[1]=-a[1];
    b[2]=-a[2];
    return b;
}

float* MatrixMathNamespace::multiply(float* c,float* a,float* b)
{
	float *aa,*bb,av[16],bv[16];

	if(a!=c && b!=c)
        {aa=a;bb=b;}
	else if(a==c && b==c)
        {memcpy(av,a,16*sizeof(float));aa=av;memcpy(bv,b,16*sizeof(float));bb=bv;}
	else if(a==c)
        {memcpy(av,a,16*sizeof(float));aa=av;bb=b;}
	else if(b==c)
        {memcpy(bv,b,16*sizeof(float));aa=a;bb=bv;}

	for(int i = 0; i < 4; i++)
	{
		c[i*4] = bb[i*4] * aa[0] + bb[i*4+1] * aa[4] + bb[i*4+2] * aa[8]    + bb[i*4+3] * aa[12];
		c[i*4+1] = bb[i*4] * aa[1] + bb[i*4+1] * aa[5] + bb[i*4+2] * aa[9]  + bb[i*4+3] * aa[13];
		c[i*4+2] = bb[i*4] * aa[2] + bb[i*4+1] * aa[6] + bb[i*4+2] * aa[10] + bb[i*4+3] * aa[14];
		c[i*4+3] = bb[i*4] * aa[3] + bb[i*4+1] * aa[7] + bb[i*4+2] * aa[11] + bb[i*4+3] * aa[15];

//			c[i*4] = bb[i*4] * aa[0] + bb[i*4+1] * aa[1] + bb[i*4+2] * aa[2]    + bb[i*4+3] * aa[3];
//			c[i*4+1] = bb[i*4] * aa[4] + bb[i*4+1] * aa[5] + bb[i*4+2] * aa[6]  + bb[i*4+3] * aa[7];
//			c[i*4+2] = bb[i*4] * aa[8] + bb[i*4+1] * aa[9] + bb[i*4+2] * aa[10] + bb[i*4+3] * aa[11];
//			c[i*4+3] = bb[i*4] * aa[12] + bb[i*4+1] * aa[13] + bb[i*4+2] * aa[14] + bb[i*4+3] * aa[15];
	}
    return c;
}

void MatrixMathNamespace::multiply(float* a,float* b)
{
	float aa[16];
	memcpy(aa,a,sizeof(float[16]));

	for(int i = 0; i < 4; i++)
	{
		a[i*4] = b[i*4] * aa[0] + b[i*4+1] * aa[4] + b[i*4+2] * aa[8]    + b[i*4+3] * aa[12];
		a[i*4+1] = b[i*4] * aa[1] + b[i*4+1] * aa[5] + b[i*4+2] * aa[9]  + b[i*4+3] * aa[13];
		a[i*4+2] = b[i*4] * aa[2] + b[i*4+1] * aa[6] + b[i*4+2] * aa[10] + b[i*4+3] * aa[14];
		a[i*4+3] = b[i*4] * aa[3] + b[i*4+1] * aa[7] + b[i*4+2] * aa[11] + b[i*4+3] * aa[15];

		//			c[i*4] = b[i*4] * a[0] + b[i*4+1] * a[1] + b[i*4+2] * a[2]    + b[i*4+3] * a[3];
		//			c[i*4+1] = b[i*4] * a[4] + b[i*4+1] * a[5] + b[i*4+2] * a[6]  + b[i*4+3] * a[7];
		//			c[i*4+2] = b[i*4] * a[8] + b[i*4+1] * a[9] + b[i*4+2] * a[10] + b[i*4+3] * a[11];
		//			c[i*4+3] = b[i*4] * a[12] + b[i*4+1] * a[13] + b[i*4+2] * a[14] + b[i*4+3] * a[15];
	}
}


void MatrixMathNamespace::print(float* m)
{
    printf("%g,%g,%g,%g\n",m[0],m[4],m[8],m[12]);
    printf("%g,%g,%g,%g\n",m[1],m[5],m[9],m[13]);
    printf("%g,%g,%g,%g\n",m[2],m[6],m[10],m[14]);
    printf("%g,%g,%g,%g\n",m[3],m[7],m[11],m[15]);

	printf("\n");
}

float* MatrixMathNamespace::traspose(float* b,float* a)
{
    float *tt,t[16];

    if(a==b){memcpy(t,a,16*sizeof(float));tt=t;}
	else tt=a;

    for(int i=0;i<4;i++)
    for(int j=0;j<4;j++)
    b[i*4+j]=tt[j*4+i];

    return b;
}
float* MatrixMathNamespace::translate(float* m,vec3 v)
{
    m[12] += m[0] * v[0] + m[4] * v[1] + m[8] * v[2];
    m[13] += m[1] * v[0] + m[5] * v[1] + m[9] * v[2];
    m[14] += m[2] * v[0] + m[6] * v[1] + m[10] * v[2];
    m[15] += m[3] * v[0] + m[7] * v[1] + m[11] * v[2];

    return m;
}

float* MatrixMathNamespace::scale(float* b,float* a,float x,float y,float z)
{
    b[0]=a[0]*x;
    b[5]=a[5]*y;
    b[10]=a[10]*z;

    return b;
}

float* MatrixMathNamespace::scale(float* b,float* a,vec3 v)
{
    b[0]=a[0]*v[0];
    b[5]=a[5]*v[1];
    b[10]=a[10]*v[2];

    return b;
}

float* MatrixMathNamespace::scale(float* b,float* a,float s)
{
    b[0]=a[0]*s;
    b[5]=a[5]*s;
    b[10]=a[10]*s;

    return b;
}

float* MatrixMathNamespace::set(float* m,vec3 s,vec3 r,vec3 t)
{
    float   ms[16],
            mr[16],
            mt[16];

    MatrixMathNamespace::identity(ms);
    MatrixMathNamespace::identity(mr);
    MatrixMathNamespace::identity(mt);

    MatrixMathNamespace::scale(ms,ms,s);
    MatrixMathNamespace::rotate(mr,mr,r);
    MatrixMathNamespace::translate(mt,t);

    //multiply(,)

    return m;
}


void MatrixMathNamespace::ortho(float* m,float l,float r,float b,float t,float n,float f)
{
    m[0]=(2*n)/(r-l);
    m[1]=0;
    m[2]=0;
    m[3]=0;

    m[4]=0;
    m[5]=(2*n)/(t-b);
    m[6]=0;
    m[7]=0;

    m[8]=(r+l)/(r-l);
    m[9]=(t+b)/(t-b);
    m[10]=-(f+n)/(f-n);
    m[11]=-1;

    m[12]=0;
    m[13]=0;
    m[14]=-(2*f*n)/(f-n);
    m[15]=0;
}

void MatrixMathNamespace::perspective(float* m,float left,float right,float bottom,float top,float near,float far)
{
	float a = 2.0f / (right - left);
	float b = 2.0f / (top - bottom);
	float c = -2.0f / (far - near);

	float tx = - (right + left)/(right - left);
	float ty = - (top + bottom)/(top - bottom);
	float tz = - (far + near)/(far - near);

	m[0]=a;
	m[1]=0;
	m[2]=0;
	m[3]=0;

	m[4]=0;
	m[5]=b;
	m[6]=0;
	m[7]=0;

	m[8]=0;
	m[9]=0;
	m[10]=c;
	m[11]=0;

	m[12]=tx;
	m[13]=ty;
	m[14]=tx;
	m[15]=1;

	/*float om[16] = {
		a, 0, 0, 0,
		0, b, 0, 0,
		0, 0, c, 0,
		tx, ty, tz, 1
	};*/
}

void MatrixMathNamespace::lookat(float* m,float px,float py,float pz,float cx,float cy,float cz,float ux,float uy,float uz)
{
    float   p[3]={px,py,pz},
            c[3]={cx,cy,cz},
            u[3]={ux,uy,uz},
            f[3],
            s[3];

    VectorMathNamespace::subtract(f,c,p,3);
    VectorMathNamespace::normalize(f,f,3);
    VectorMathNamespace::cross(s,f,u);
    VectorMathNamespace::normalize(s,s,3);
    VectorMathNamespace::cross(u,s,f);

    MatrixMathNamespace::make(m,s[0],u[0],-f[0],0,s[1],u[1],-f[1],0,s[2],u[2],-f[2],0,0,0,0,1);


    m[12] += m[0] * -px + m[4] * -py + m[8] * -pz;
    m[13] += m[1] * -px + m[5] * -py + m[9] * -pz;
    m[14] += m[2] * -px + m[6] * -py + m[10] * -pz;
    m[15] += m[3] * -px + m[7] * -py + m[11] * -pz;

}

void MatrixMathNamespace::lookat(float* m,float* at,float px,float py,float pz,float ux,float uy,float uz)
{
    float atinv[16];
    MatrixMathNamespace::invert(atinv,at);

    float   p[3]={px,py,pz},
            c[3]={-atinv[12],-atinv[13],-atinv[14]},
            u[3]={ux,uy,uz},
            f[3],
            s[3];

    VectorMathNamespace::subtract(f,c,p,3);
    VectorMathNamespace::normalize(f,f,3);
    VectorMathNamespace::cross(s,f,u);
    VectorMathNamespace::normalize(s,s,3);
    VectorMathNamespace::cross(u,s,f);

    float l[16]=
    {
        s[0],u[0],-f[0],0,
        s[1],u[1],-f[1],0,
        s[2],u[2],-f[2],0,
        0,0,0,1
    };

    MatrixMathNamespace::multiply(m,m,atinv);
    MatrixMathNamespace::multiply(m,m,l);


    m[12] += m[0] * -px + m[4] * -py + m[8] * -pz;
    m[13] += m[1] * -px + m[5] * -py + m[9] * -pz;
    m[14] += m[2] * -px + m[6] * -py + m[10] * -pz;
    m[15] += m[3] * -px + m[7] * -py + m[11] * -pz;
}

void MatrixMathNamespace::rotate(float* mr,float* m,float angle,float u,float v,float w)
{
    float ang=(float)(angle*PI_OVER_180);
	float m2[16] = {0};

	float nrm=sqrt(u*u+v*v+w*w);

	float   x=u/nrm,
            y=v/nrm,
            z=w/nrm;

	float c=cos(ang);
	float s=sin(ang);

	float xx=x*x,
            yy=y*y,
		    zz=z*z;


	m2[0] = xx+(1.0f-xx)*c;
	m2[1] = (1.0f-c)*x*y-s*z;
	m2[2] = (1.0f-c)*x*z+s*y;
	m2[3] = 0.0f;

	m2[4] = (1.0f-c)*y*x+s*z;
	m2[5] = yy+(1.0f-yy)*c;
	m2[6] = (1.0f-c)*y*z-s*x;
	m2[7] = 0.0f;

	m2[8] = (1.0f-c)*z*x-s*y;
	m2[9] = (1.0f-c)*z*y+s*x;
	m2[10] = zz+(1.0f-zz)*c;
	m2[11] = 0.0f;

	m2[12] = 0;
	m2[13] = 0;
	m2[14] = 0;
	m2[15] = 1.0f;

    MatrixMathNamespace::multiply(mr,m,m2);
}

void MatrixMathNamespace::rotate(float* mr,float* m,vec3 v)
{
    float rx[16],ry[16],rz[16];

    MatrixMathNamespace::identity(rx);
    MatrixMathNamespace::identity(ry);
    MatrixMathNamespace::identity(rz);

    if(v[2]!=0)
        MatrixMathNamespace::rotate(rz,rz,v[2],0,0,1);
    if(v[1]!=0)
        MatrixMathNamespace::rotate(ry,ry,v[1],0,1,0);
    if(v[0]!=0)
        MatrixMathNamespace::rotate(rx,rx,v[0],1,0,0);

    float zy[16],zyx[16];

    multiply(zy,ry,rz);
    multiply(zyx,rx,zy);
    multiply(mr,m,zyx);
}

void MatrixMathNamespace::rotate(float* m,float a,float x,float y,float z)
{
	rotate(m,m,a,x,y,z);
}


float MatrixMathNamespace::det(const float* m)
{
    return  m[12]*m[9]*m[6]*m[3]-
            m[8]*m[13]*m[6]*m[3]-
            m[12]*m[5]*m[10]*m[3]+
            m[4]*m[13]*m[10]*m[3]+
            m[8]*m[5]*m[14]*m[3]-
            m[4]*m[9]*m[14]*m[3]-
            m[12]*m[9]*m[2]*m[7]+
            m[8]*m[13]*m[2]*m[7]+
            m[12]*m[1]*m[10]*m[7]-
            m[0]*m[13]*m[10]*m[7]-
            m[8]*m[1]*m[14]*m[7]+
            m[0]*m[9]*m[14]*m[7]+
            m[12]*m[5]*m[2]*m[11]-
            m[4]*m[13]*m[2]*m[11]-
            m[12]*m[1]*m[6]*m[11]+
            m[0]*m[13]*m[6]*m[11]+
            m[4]*m[1]*m[14]*m[11]-
            m[0]*m[5]*m[14]*m[11]-
            m[8]*m[5]*m[2]*m[15]+
            m[4]*m[9]*m[2]*m[15]+
            m[8]*m[1]*m[6]*m[15]-
            m[0]*m[9]*m[6]*m[15]-
            m[4]*m[1]*m[10]*m[15]+
            m[0]*m[5]*m[10]*m[15];
}

float* MatrixMathNamespace::invert(float* i,float* m)
{
    float *mc,mcc[16];

    if(m!=i)mc=m;
    else {memcpy(mcc,m,sizeof(float)*16);mc=mcc;}

    float x=det(m);
    if(!x) return NULL;

    i[0]= (-mc[13]*mc[10]*mc[7] +mc[9]*mc[14]*mc[7] +mc[13]*mc[6]*mc[11]
    -mc[5]*mc[14]*mc[11] -mc[9]*mc[6]*mc[15] +mc[5]*mc[10]*mc[15])/x;
    i[4]= ( mc[12]*mc[10]*mc[7] -mc[8]*mc[14]*mc[7] -mc[12]*mc[6]*mc[11]
    +mc[4]*mc[14]*mc[11] +mc[8]*mc[6]*mc[15] -mc[4]*mc[10]*mc[15])/x;
    i[8]= (-mc[12]*mc[9]* mc[7] +mc[8]*mc[13]*mc[7] +mc[12]*mc[5]*mc[11]
    -mc[4]*mc[13]*mc[11] -mc[8]*mc[5]*mc[15] +mc[4]*mc[9]* mc[15])/x;
    i[12]=( mc[12]*mc[9]* mc[6] -mc[8]*mc[13]*mc[6] -mc[12]*mc[5]*mc[10]
    +mc[4]*mc[13]*mc[10] +mc[8]*mc[5]*mc[14] -mc[4]*mc[9]* mc[14])/x;
    i[1]= ( mc[13]*mc[10]*mc[3] -mc[9]*mc[14]*mc[3] -mc[13]*mc[2]*mc[11]
    +mc[1]*mc[14]*mc[11] +mc[9]*mc[2]*mc[15] -mc[1]*mc[10]*mc[15])/x;
    i[5]= (-mc[12]*mc[10]*mc[3] +mc[8]*mc[14]*mc[3] +mc[12]*mc[2]*mc[11]
    -mc[0]*mc[14]*mc[11] -mc[8]*mc[2]*mc[15] +mc[0]*mc[10]*mc[15])/x;
    i[9]= ( mc[12]*mc[9]* mc[3] -mc[8]*mc[13]*mc[3] -mc[12]*mc[1]*mc[11]
    +mc[0]*mc[13]*mc[11] +mc[8]*mc[1]*mc[15] -mc[0]*mc[9]* mc[15])/x;
    i[13]=(-mc[12]*mc[9]* mc[2] +mc[8]*mc[13]*mc[2] +mc[12]*mc[1]*mc[10]
    -mc[0]*mc[13]*mc[10] -mc[8]*mc[1]*mc[14] +mc[0]*mc[9]* mc[14])/x;
    i[2]= (-mc[13]*mc[6]* mc[3] +mc[5]*mc[14]*mc[3] +mc[13]*mc[2]*mc[7]
    -mc[1]*mc[14]*mc[7] -mc[5]*mc[2]*mc[15] +mc[1]*mc[6]* mc[15])/x;
    i[6]= ( mc[12]*mc[6]* mc[3] -mc[4]*mc[14]*mc[3] -mc[12]*mc[2]*mc[7]
    +mc[0]*mc[14]*mc[7] +mc[4]*mc[2]*mc[15] -mc[0]*mc[6]* mc[15])/x;
    i[10]=(-mc[12]*mc[5]* mc[3] +mc[4]*mc[13]*mc[3] +mc[12]*mc[1]*mc[7]
    -mc[0]*mc[13]*mc[7] -mc[4]*mc[1]*mc[15] +mc[0]*mc[5]* mc[15])/x;
    i[14]=( mc[12]*mc[5]* mc[2] -mc[4]*mc[13]*mc[2] -mc[12]*mc[1]*mc[6]
    +mc[0]*mc[13]*mc[6] +mc[4]*mc[1]*mc[14] -mc[0]*mc[5]* mc[14])/x;
    i[3]= ( mc[9]* mc[6]* mc[3] -mc[5]*mc[10]*mc[3] -mc[9]* mc[2]*mc[7]
    +mc[1]*mc[10]*mc[7] +mc[5]*mc[2]*mc[11] -mc[1]*mc[6]* mc[11])/x;
    i[7]= (-mc[8]* mc[6]* mc[3] +mc[4]*mc[10]*mc[3] +mc[8]* mc[2]*mc[7]
    -mc[0]*mc[10]*mc[7] -mc[4]*mc[2]*mc[11] +mc[0]*mc[6]* mc[11])/x;
    i[11]=( mc[8]* mc[5]* mc[3] -mc[4]*mc[9]* mc[3] -mc[8]* mc[1]*mc[7]
    +mc[0]*mc[9]* mc[7] +mc[4]*mc[1]*mc[11] -mc[0]*mc[5]* mc[11])/x;
    i[15]=(-mc[8]* mc[5]* mc[2] +mc[4]*mc[9]* mc[2] +mc[8]* mc[1]*mc[6]
    -mc[0]*mc[9]* mc[6] -mc[4]*mc[1]*mc[10] +mc[0]*mc[5]* mc[10])/x;

    return i;
}


float* MatrixMathNamespace::transform(float* c,float* m,float* a)
{
    float *t,tt[3];

    if(a!=c)t=a;
    else 
	{memcpy(tt,a,3*sizeof(float));t=tt;}

    c[0] = m[0] * t[0] + m[4] * t[1] + m[8] * t[2] + m[12];
    c[1] = m[1] * t[0] + m[5] * t[1] + m[9] * t[2] + m[13];
    c[2] = m[2] * t[0] + m[6] * t[1] + m[10] * t[2] + m[14];

return c;
}

float* MatrixMathNamespace::transform(float* c,float* m,float x,float y,float z)
{
    float v[3]={x,y,z};
    return transform(c,m,v);;
}

/** \brief Simply transform a vector by matrix and return it after normalizing.Vectors can be the same.
    *
    * \c output vector
    * \m input matrix
    * \a input vector
    * \return c (output vector)
    *
    */


float* MatrixMathNamespace::orientation(float* c,float* m,float* a)
{
    float *t,tt[3];

    if(a!=c)t=a;
    else {memcpy(tt,a,3*sizeof(float));t=tt;}

    c[0] = m[0] * t[0] + m[4] * t[1] + m[8] * t[2] ;
    c[1] = m[1] * t[0] + m[5] * t[1] + m[9] * t[2] ;
    c[2] = m[2] * t[0] + m[6] * t[1] + m[10] * t[2];

    VectorMathNamespace::normalize(c,c,3);

    return c;
}

float* MatrixMathNamespace::orientation(float* c,float* m,float x,float y,float z)
{
    float v[3]={x,y,z};
    return MatrixMathNamespace::orientation(c,m,v);
}


void MatrixMathNamespace::orientations(float* m,float* a,float* b,float*c)
{
    orientation(a,m,1,0,0);
    orientation(b,m,0,1,0);
    orientation(c,m,0,0,1);
}



#define init_mat2_references m11(v[0]),m12(v[1]),m21(v[2]),m22(v[3])
#define init_mat3_references m11(v[0]),m12(v[1]),m13(v[2]),m21(v[3]),m22(v[4]),m23(v[5]),m31(v[6]),m32(v[7]),m33(v[8])
#define init_mat4_references m11(v[0]),m12(v[1]),m13(v[2]),m14(v[3]),m21(v[4]),m22(v[5]),m23(v[6]),m24(v[7]),m31(v[8]),m32(v[9]),m33(v[10]),m34(v[11]),m41(v[12]),m42(v[13]),m43(v[14]),m44(v[15])


mat3::mat3():init_mat3_references
{
	v[0]=1;v[1]=0,v[2]=0;
	v[3]=0;v[4]=1,v[5]=0;
	v[6]=0;v[7]=0,v[8]=1;
}

mat3::mat3(float* a):init_mat3_references{memcpy(v,a,sizeof(float)*9);}
mat3::mat3(const mat3& a):init_mat3_references{memcpy(v,a.v,sizeof(float)*9);}
mat3::mat3(mat4 a):init_mat3_references{v[0]=a.v[0];v[1]=a.v[1],v[2]=a.v[2];v[3]=a.v[4];v[4]=a.v[5],v[5]=a.v[6];v[6]=a.v[8];v[7]=a.v[9],v[8]=a.v[10];}

mat3& mat3::operator=(mat3 a){memcpy(v,a,sizeof(float)*9);return *this;}
mat3& mat3::operator=(mat4 a){v[0]=a.v[0];v[1]=a.v[1],v[2]=a.v[2];v[3]=a.v[4];v[4]=a.v[5],v[5]=a.v[6];v[6]=a.v[8];v[7]=a.v[9],v[8]=a.v[10];return *this;}

mat3& mat3::identity()
{
	v[0]=1;v[1]=0,v[2]=0;
	v[3]=0;v[4]=1,v[5]=0;
	v[6]=0;v[7]=0,v[8]=1;

	return *this;
}


float* mat3::operator[](int i){return &v[i*3];}




mat4::mat4():init_mat4_references{MatrixMathNamespace::identity(v);};
mat4::mat4(const mat4& mm):init_mat4_references{MatrixMathNamespace::copy(v,mm.v);}
mat4::mat4(const float* mm):init_mat4_references{MatrixMathNamespace::copy(v,mm);}
mat4::mat4(const double* mm):init_mat4_references{for(int i=0;i<16;i++)v[i]=(float)mm[i];}
mat4::mat4(mat3 a):init_mat4_references{v[0]=a.v[0],v[1]=a.v[1],v[2]=a.v[2],v[4]=a.v[3],v[5]=a.v[4],v[6]=a.v[5],v[8]=a.v[7],v[9]=a.v[8],v[10]=a.v[9];v[3]=v[7]=v[11]=v[12]=v[13]=v[14]=0;v[15]=1.0f;}
mat4& mat4::operator=(mat3 a){v[0]=a.v[0],v[1]=a.v[1],v[2]=a.v[2],v[4]=a.v[3],v[5]=a.v[4],v[6]=a.v[5],v[8]=a.v[7],v[9]=a.v[8],v[10]=a.v[9];v[3]=v[7]=v[11]=v[12]=v[13]=v[14]=0;v[15]=1.0f;return *this;}
mat4& mat4::operator=(const mat4& mm){MatrixMathNamespace::copy(v,mm.v);return *this;}
bool mat4::operator==(mat4& mm){for(int i=0;i<16;i++){if(v[i]!=mm.v[i])return false;}return true;}

mat4 mat4::operator*(mat4 mm){mat4 t;MatrixMathNamespace::multiply(t.v,mm.v,v);return t;}
mat4 mat4::operator*(float f){mat4 t;MatrixMathNamespace::scale(t,v,f);return t;}
mat4 mat4::operator*(vec3 v){mat4 t;MatrixMathNamespace::scale(t,v,v);return t;}
mat4& mat4::operator*=(mat4 mm){MatrixMathNamespace::multiply(v,mm.v,v);return *this;}
mat4& mat4::operator*=(vec3 v3){MatrixMathNamespace::scale(v,v,v3);return *this;}
mat4& mat4::operator*=(float f){MatrixMathNamespace::scale(v,v,f);return *this;}
mat4 mat4::operator+(mat4& mm){mat4 t;MatrixMathNamespace::sum(t.v,v,mm.v);return t;}
mat4& mat4::operator+=(mat4& mm){MatrixMathNamespace::sum(v,v,mm.v);return *this;}
mat4 mat4::operator-(mat4& mm){mat4 t;MatrixMathNamespace::subtract(t.v,v,mm.v);return t;}
mat4& mat4::operator-=(mat4& mm){MatrixMathNamespace::subtract(v,v,mm.v);return *this;}
mat4 mat4::operator-(){mat4 t;MatrixMathNamespace::negate(t,v);return t;}
float* 	mat4::operator[](int i){return &v[i*4];}

mat4& mat4::identity(){MatrixMathNamespace::identity(v);return *this;}
mat4& mat4::identity33(){static int idx[9]={0,1,2,4,5,6,8,9,10};for(int i=0;i<10;i++)v[idx[i]]=(idx[i]%5 ? 0.0f : 1.0f);return *this;}
vec3 mat4::transform(vec3 in){vec3 r;MatrixMathNamespace::transform(r,v,in);return r;}
void mat4::transformself(vec3& inout){MatrixMathNamespace::transform(inout,v,inout);}
vec3 mat4::transform(float x,float y,float z){vec3 r;MatrixMathNamespace::transform(r,v,x,y,z);return r;}

mat4& mat4::traspose(){MatrixMathNamespace::traspose(v,v);return *this;}

mat4& mat4::rotate(float a,float x,float y,float z){MatrixMathNamespace::rotate(v,v,a,x,y,z);return *this;}
mat4& mat4::rotate(float a,vec3 v3){MatrixMathNamespace::rotate(v,v,a,v3[0],v3[1],v3[2]);return *this;}
mat4& mat4::rotate(vec3 v3){MatrixMathNamespace::rotate(v,v,v3);return *this;}

mat4& mat4::scale(float scalar){MatrixMathNamespace::scale(v,v,scalar);return *this;}
mat4& mat4::scale(vec3 scalevector){MatrixMathNamespace::scale(v,v,scalevector);return *this;}

mat4& mat4::srt(vec3 s,vec3 r,vec3 t){mat4 ms;ms.scale(s);mat4 mr;MatrixMathNamespace::rotate(mr,mr,r);mat4 mt;mt.translate(t);*this=ms*mr*mt;return *this;}
mat4& mat4::trs(vec3 t,vec3 r,vec3 s){mat4 ms;ms.scale(s);mat4 mr;MatrixMathNamespace::rotate(mr,mr,r);mat4 mt;mt.translate(t);*this=mt*mr*ms;return *this;}


mat4& mat4::copy(float* mm){MatrixMathNamespace::copy(mm,v);return *this;}
mat4& mat4::copy33(mat4& mm){static int idx[9]={0,1,2,4,5,6,8,9,10};for(int i=0;i<10;i++)v[idx[i]]=mm.v[idx[i]];return *this;}
mat4& mat4::move(vec3 v3){v[12] = v3[0];v[13] = v3[1];v[14] = v3[2];return *this;}

mat4& mat4::translate(float x,float y,float z){vec3 t(x,y,z);MatrixMathNamespace::translate(v,t);return *this;}
mat4& mat4::translate(vec3 v3){MatrixMathNamespace::translate(v,v3);return *this;}

mat4& mat4::invert(){MatrixMathNamespace::invert(v,v);return *this;}
mat4 mat4::inverse(){float ret[16];MatrixMathNamespace::invert(ret,v);return mat4(ret);}

void mat4::print(){MatrixMathNamespace::print(v);}

mat4& mat4::perspective(float left,float right ,float bottom,float up,float near,float far){MatrixMathNamespace::perspective(v,left,right,bottom,up,near,far);return *this;}
//mat4& mat4::lookat(vec3 target,vec3 p,vec3 y){matrix::lookat(v,p[0],p[1],p[2],target[0],target[1],target[2],y[0],y[1],y[2]);return *this;}
mat4& mat4::lookat(vec3 target){vec3 y=axis(vec3(0,1,0)),p=position();VectorMathNamespace::print(p);VectorMathNamespace::print(y);MatrixMathNamespace::lookat(v,p[0],p[1],p[2],target[0],target[1],target[2],y[0],y[1],y[2]);return *this;}

vec3 mat4::position(){vec3 t;MatrixMathNamespace::transform(t,v,0,0,0);return t;}

void mat4::axes(vec3& a,vec3& b,vec3& c){MatrixMathNamespace::orientations(v,a,b,c);}
vec3 mat4::axis(vec3 in){MatrixMathNamespace::orientation(in,v,in);return in;}
vec3 mat4::axis(float x,float y,float z){vec3 out(x,y,z);MatrixMathNamespace::orientation(out,v,out);return out;}

mat4& mat4::ortho(float left, float right,float bottom, float top,float near, float far) 
{ 
	float a = 2.0f / (right - left);
	float b = 2.0f / (top - bottom);
	float c = -2.0f / (far - near);

	float tx = - (right + left)/(right - left);
	float ty = - (top + bottom)/(top - bottom);
	float tz = - (far + near)/(far - near);

	float om[16] = {
		a, 0, 0, 0,
		0, b, 0, 0,
		0, 0, c, 0,
		tx, ty, tz, 1
	};

	MatrixMathNamespace::copy(this->v,om);
	return *this;
}

void mat4::zero(){memset(this->v,0,sizeof(float[16]));}




bool AABB::Contains(vec3 iv)
{
	return a.x<=iv.x && iv.x<=b.x && a.y<=iv.y && iv.y<=b.y && a.z<=iv.z && iv.z<=b.z;
	//return a.x<=iv.x<=b.x && a.y<=iv.y<=b.y && a.z<=iv.z<=b.z; //compiler launch C4804
}

void AABB::Grow(AABB ab)
{
	a.x=a.x>ab.a.x?ab.a.x:a.x; a.y=a.y>ab.a.y?ab.a.y:a.y; a.z=a.z>ab.a.z?ab.a.z:a.z;
	b.x=b.x<ab.b.x?ab.b.x:b.x; b.y=b.y<ab.b.y?ab.b.y:b.y; b.z=b.z<ab.b.z?ab.b.z:b.z;
}
void AABB::Shrink(AABB ab)
{
	a.x=a.x<ab.a.x?ab.a.x:a.x; a.y=a.y<ab.a.y?ab.a.y:a.y; a.z=a.z<ab.a.z?ab.a.z:a.z;
	b.x=b.x>ab.b.x?ab.b.x:b.x; b.y=b.y>ab.b.y?ab.b.y:b.y; b.z=b.z>ab.b.z?ab.b.z:b.z;
}
#include "primitives.h"

#include <cstdlib>
#include <typeinfo>
#include <cmath>


///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////////FilePath//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


FilePath::FilePath(){}
FilePath::FilePath(const String& iString):String(iString){}
FilePath::FilePath(const wchar_t* iString):String(iString){}


String FilePath::File()
{
	size_t tIdx=this->rfind('\\');
	return (tIdx!=std::string::npos && ++tIdx<this->size()) ? this->substr(tIdx) : String();
}
String FilePath::Name()
{
	size_t tFirst=this->rfind('\\');
	size_t tLast=this->rfind('.');

	if(tFirst<tLast && tFirst!=std::string::npos && tLast!=std::string::npos && ++tFirst<this->size())
		return String(*this,tFirst,tLast);

	return String();
}

String FilePath::Path()
{
	size_t tLast=this->rfind('\\');

	return tLast!=std::string::npos ? String(*this,0,tLast) : String();
}

String FilePath::Extension()
{
	size_t tLast=this->rfind('.');
	return (tLast!=std::string::npos && ++tLast<this->size()) ? this->substr(tLast) : String();
}

String FilePath::PointedExtension()
{
	size_t tLast=this->rfind('.');
	return (tLast!=std::string::npos) ? this->substr(tLast) : String();
}

String FilePath::PathUp(int iLevels)
{
	int tLevel=0;
	size_t tIdx=-1;

	while(true)
	{
		tIdx=this->rfind('\\',tIdx-1);

		if(std::string::npos!=tIdx && ++tLevel==iLevels)
			return String(*this,0,tIdx);
	}

	return String();
}

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////////File/////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

File::File():
data(0)
{}

File::File(String iString):
path(iString),
	data(0)
{}

File::~File()
{
	if(this->data!=0)
		this->Close();
}

bool File::IsOpen()
{
	return this->data ? true : false;
}

File::operator FILE* ()
{
	return this->data;
}

bool File::Open(const wchar_t* mode)
{
	if(!this->path.size() || this->IsOpen())
		return false;

	this->data=fopen(StringUtils::ToChar(this->path).c_str(),StringUtils::ToChar(mode).c_str());

	if(!data)
		return false;

	int tell=ftell(this->data);

	if(tell!=0)
		DEBUG_BREAK();

	return true;
}

void File::Close()
{
	this->data ? fclose(this->data),this->data=0 : 0;
}

bool File::Exist()
{
	return !this->path.empty() ? File::Exist(this->path.c_str()) : false;
}

int File::Size()
{
	int result=0;

	bool tWasOpen=this->IsOpen();

	bool tIsOpen=!tWasOpen ? this->Open() : true;

	if(tIsOpen)
	{
		int curPos=ftell(this->data);
		fseek(this->data,0,SEEK_END);
		result=ftell(this->data);
		fseek(this->data,0,curPos);
	}

	if(!tWasOpen)
		this->Close();

	return result;
}

bool File::Create()
{
	return File::Create(this->path.c_str());
}

bool File::Delete()
{
	return !this->path.empty() ? !::remove(StringUtils::ToChar(this->path).c_str()) : false;
}

int File::CountOccurrences(char iChar)
{
	int occurrences=0;

	bool wasOpen=this->IsOpen();

	if(!this->IsOpen())
		this->Open();

	if(this->IsOpen())
	{
		int oldPos=ftell(this->data);

		fseek(this->data,0,SEEK_END);

		int ___size=ftell(this->data);

		int ___i=0;
		while(___i < ___size)
		{
			fseek(this->data,___i,SEEK_SET);

			char c=fgetc(this->data);

			if(iChar==c)
				occurrences++;

			___i++;
		}

		fseek(this->data,oldPos,SEEK_SET);
	}

	if(!wasOpen)
		this->Close();

	return occurrences;
}


int File::Read(void* outData,int iSize)
{
	if(this->data)
	{
		return fread(outData,iSize,1,this->data);
	}

	return 0;
}


int File::Write(void* iData,int iSize,int iNum)
{
	if(this->data)
		return fwrite(iData,iSize,iNum,this->data);

	return 0;
}

//statics function


bool File::Create(String iFilename)
{
	FILE* tFile=fopen(StringUtils::ToChar(iFilename).c_str(),"w");
	if(tFile)
		fclose(tFile);
	else return false;

	return true;
}
bool File::Exist(String iFilename)
{
	FILE* tFile=fopen(StringUtils::ToChar(iFilename).c_str(),"r");

	if(tFile)
	{
		fclose(tFile);
		return true;
	}

	return false;
}
bool File::Delete(String iFilename)
{
	return !iFilename.empty() ? !::remove(StringUtils::ToChar(iFilename).c_str()) : false;
}

int File::Size(String iFilename)
{
	if(iFilename.empty())
		return -1;

	int RetVal;

	FILE* tFile=fopen(StringUtils::ToChar(iFilename).c_str(),"r");

	if(tFile)
	{
		fseek(tFile,0,SEEK_END);
		RetVal=ftell(tFile);
		fclose(tFile);
	}
	else
		RetVal=-2;

	return RetVal;
}



///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////////StringUtils///////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

void StringUtils::WriteWstring(FILE* iFile,String& iWstring)
{
	int nameCount=iWstring.size();

	fwrite(&nameCount,sizeof(int),1,iFile);
	fwrite(iWstring.c_str(),sizeof(wchar_t),nameCount,iFile);
}

void StringUtils::ReadWstring(FILE* iFile,String& iWstring)
{
	int nameCount=0;

	fread(&nameCount,sizeof(int),1,iFile);

	{
		wchar_t* tNameBuf=new wchar_t[nameCount+1];
		fread(tNameBuf,sizeof(wchar_t),nameCount,iFile);
		tNameBuf[nameCount]='\0';

		iWstring=tNameBuf;

		SAFEDELETEARRAY(tNameBuf);
	}
}

std::string StringUtils::ToChar(const String& iString)
{
	unsigned int tStrlen=iString.size()+1;
	char* tNameBuf=new char[tStrlen];
	wcstombs(tNameBuf,iString.c_str(),tStrlen);

	std::string tReturnString(tNameBuf);

	SAFEDELETEARRAY(tNameBuf);

	return tReturnString;
}

std::string StringUtils::ToChar(const wchar_t* iString)
{
	if(iString)
	{
		size_t tStrlen=wcslen(iString)+1;
		char* tNameBuf=new char[tStrlen];
		wcstombs(tNameBuf,iString,tStrlen);

		std::string tReturnString(tNameBuf);

		SAFEDELETEARRAY(tNameBuf);

		return tReturnString;
	}

	return std::string();
}

String StringUtils::ToWide(const char* iChars)
{
	unsigned int tStrlen=strlen(iChars)+1;
	wchar_t* tNameBuf=new wchar_t[tStrlen];
	mbstowcs(tNameBuf,iChars,tStrlen);

	String tReturnString(tNameBuf);

	SAFEDELETEARRAY(tNameBuf);

	return tReturnString;
}

String StringUtils::ToWide(const std::string& iChars)
{
	unsigned int tStrlen=iChars.size()+1;
	wchar_t* tNameBuf=new wchar_t[tStrlen];
	mbstowcs(tNameBuf,iChars.c_str(),tStrlen);

	String tReturnString(tNameBuf);

	SAFEDELETEARRAY(tNameBuf);

	return tReturnString;
}

String StringUtils::Int(int& iInt)
{
	String tString;

	wchar_t tCharInt[100];
	swprintf(tCharInt,100,L"%d",iInt);

	tString=tCharInt;

	return tString;
}

String StringUtils::Float(float& iFloat,int iBefore,int iAfter)
{
	String tString;

	wchar_t tCharInt[100];
	swprintf(tCharInt,100,L"%*.*f",iBefore,iAfter,iFloat);

	tString=tCharInt;

	return tString;
}

bool StringUtils::WriteWideFile(String iFilename,String iContent,String iMode)
{
	File tFileToWrite=iFilename;

	if(tFileToWrite.Open(iMode.c_str()))
	{
		fwrite(iContent.c_str(),sizeof(wchar_t),iContent.size(),tFileToWrite);

		tFileToWrite.Close();

		return true;
	}

	return false;
}


String ReadWideFile(String iFilename,String iMode)
{
	String tContent;
	File tFile=iFilename;

	if(tFile.Open(iMode.c_str()))
	{
		int tSize=tFile.Size();

		if(tSize>0)
		{
			wchar_t* tWideArray=new wchar_t[tSize+1];
			fread(tWideArray,sizeof(wchar_t),tSize,tFile);
			tWideArray[tSize]='\0';
			tContent=tWideArray;
			SAFEDELETEARRAY(tWideArray);
		}

		tFile.Close();
	}

	return tContent;
}

bool StringUtils::WriteCharFile(String iFilename,String iContent,String iMode)
{
	File tFileToWrite=iFilename;

	std::string iCharContent=StringUtils::ToChar(iContent);

	if(tFileToWrite.Open(iMode.c_str()))
	{
		fwrite(iCharContent.c_str(),sizeof(char),iCharContent.size(),tFileToWrite);

		tFileToWrite.Close();

		return true;
	}

	return false;
}

String StringUtils::ReadCharFile(String iFilename,String iMode)
{
	String tContent;

	File tFile=iFilename;

	if(tFile.Open(iMode.c_str()))
	{
		int tSize=tFile.Size();

		if(tSize>0)
		{
			char* tCharArray=new char[tSize+1];
			fread(tCharArray,sizeof(char),tSize,tFile.data);
			tCharArray[tSize]='\0';
			tContent=ToWide(tCharArray);
			SAFEDELETEARRAY(tCharArray);
		}

		tFile.Close();
	}

	return tContent;
}

//---------------------------------


float* Vector::cross(float* c,const float* a,const float* b)
{
    c[0]=a[1]*b[2]-a[2]*b[1];
    c[1]=a[2]*b[0]-a[0]*b[2];
    c[2]=a[0]*b[1]-a[1]*b[0];

    return c;
}


float* Vector::sum(float *c,const float *a ,const float *b ,int dim)
{
    for(int i=0;i<dim;i++)c[i]=a[i]+b[i];
    return c;
}

float* Vector::subtract(float *c,const float *a ,const float *b,int dim )
{
    for(int i=0;i<dim;i++)c[i]=a[i]-b[i];
    return c;
}

float* Vector::negate(float *b ,const float *a ,int dim)
{
    for(int i=0;i<dim;i++)b[i]=-a[i];
    return b;
}

float* Vector::make(float *v ,int dim,float x,float y,float z,float t)
{
    float tt[4]={x,y,z,t};
    for(int i=0;i<dim;i++)
    v[i]=tt[i];
    return v;
}


float Vector::angleg(const float *a,const float *b,int dim)
{
    return acos(dot(a,b,dim)/length(a,dim) * length(b,dim)) * (float)(180.0/PI);
}


float Vector::angler(const float *a,const float *b,int dim)
{
    return acos(dot(a,b,dim)/(length(a,dim) * length(b,dim)));
}


float* Vector::copy(float* b,const float* a,int dim)
{
	for(int i=0;i<dim;i++)
		b[i]=a[i];
    return b;
}



float Vector::dot(const float *a ,const float *b,int dim )
{
    float ret=0.0f;
    for(int i=0;i<dim;i++)
    {
        ret+=a[i]*b[i];
    }
    return ret;
}

void Vector::print(float* v){/*wprintf(L"%g,%g,%g\n",v[0],v[1],v[2]);*/}


float Vector::length(const float *v,int dim )
{
    float arg=0.0f;
    for(int i=0;i<dim;i++)
        arg+=v[i]*v[i];
    float result=sqrt(arg);
    return result;
}


float* Vector::normalize(float *b ,const float *a,int dim )
{
    float d=length(a,dim);

    for(int i=0;i<dim;i++)
        b[i]=a[i]/d;

    return b;
}


float* Vector::scale(float *b ,const float *a ,int dim,float x,float y,float z,float t)
{
    float v[4]={x,y,z,t};

    for(int i=0;i<dim;i++)
        b[i]=a[i]*v[i];

    return b;
}


float* Vector::scale(float *c,const float *a ,const float *b,int dim )
{
    for(int i=0;i<dim;i++)
        c[i]=a[i]*b[i];
    return c;
}


float* Vector::scale(float *b ,const float *a ,float s,int dim)
{
    for(int i=0;i<dim;i++)
            b[i]=a[i]*s;
    return b;
}



float* Vector::minimum(float *c,const float *a,const float *b,int dim)
{
    for(int i=0;i<dim;i++)
    c[i]=(a[i]<b[i] ? a[i] : b[i]);

    return c;
}


float* Vector::maximum(float *c,const float *a,const float *b,int dim)
{
    for(int i=0;i<dim;i++)
    c[i]=(a[i]>b[i] ? a[i] : b[i]);

    return c;
}


bool Vector::equal(const float *a,const float *b,int dim)
{
    for(int i=0;i<dim;i++)
        if(a[i]!=b[i])return false;
    return true;
}


edges::edges():left(*this->v[0]),top(*this->v[1]),right(*this->v[2]),bottom(*this->v[3]){};


vec2::vec2():x(v[0]),y(v[1]){Vector::make(v,2,0,0);}
vec2::vec2(const vec2& a):x(v[0]),y(v[1]){Vector::copy(v,a.v,2);}
vec2::vec2(float fv[2]):x(v[0]),y(v[1]){Vector::copy(v,fv,2);}
vec2::vec2(float x,float y):x(v[0]),y(v[1]){Vector::make(v,2,x,y);}
vec2 vec2::operator=(const vec2& a){Vector::copy(v,a.v,2);return *this;}
vec2 vec2::operator+(vec2& a){vec2 r;Vector::sum(r,a.v,v,2);return r;}
vec2 vec2::operator-(vec2& a){vec2 r;Vector::subtract(r,a.v,v,2);return r;}
bool vec2::operator==(vec2& a){return Vector::equal(a.v,v,2);}
bool vec2::operator!=(vec2& a){return !Vector::equal(a.v,v,2);}
float& vec2::operator[](int i){return v[i];}
void vec2::scale(float f){Vector::scale(v,v,f,2);}
void vec2::scale(vec2 a){Vector::scale(v,a.v,v,2);}
vec2 vec2::minimum(vec2& a){vec2 r; Vector::minimum(r,a.v,v,2);return r;}
vec2 vec2::maximum(vec2& a){vec2 r; Vector::maximum(r,a.v,v,2);return r;}
float vec2::dot(vec2& a,vec2& b){return Vector::dot(a.v,b.v,2);}
float vec2::dot(vec2& a){return Vector::dot(a.v,v,2);}
float vec2::length(){return Vector::length(v,2);}
void vec2::normalize(){Vector::normalize(v,v,2);}
void vec2::make(float a,float b){Vector::make(v,2,a,b);}
void vec2::negate(){Vector::negate(v,v,2);}
vec2::operator float* (){return v;}
vec2::operator void* (){return v;}


vec3::vec3():x(v[0]),y(v[1]),z(v[2]){Vector::make(v,3,0,0,0);}
vec3::vec3(const vec3& a):x(v[0]),y(v[1]),z(v[2]){Vector::copy(v,a.v,3);}
vec3::vec3(float fv[3]):x(v[0]),y(v[1]),z(v[2]){Vector::copy(v,fv,3);}
vec3::vec3(float x,float y,float z):x(v[0]),y(v[1]),z(v[2]){Vector::make(v,3,x,y,z);}
float& vec3::operator[](int i){return v[i];}
bool vec3::operator==(vec3 a){return Vector::equal(v,a.v,3);}
bool vec3::operator==(vec3& a)const{return Vector::equal(a.v,v,3);}
bool vec3::operator!=(vec3& a){return !Vector::equal(a.v,v,3);}
vec3& vec3::operator=(vec3 a){Vector::copy(v,a.v,3);return *this;}
vec3 vec3::operator+(vec3& a){vec3 r;Vector::sum(r,v,a.v,3);return r;}
vec3& vec3::operator+=(vec3& a){Vector::sum(v,v,a.v,3);return *this;}
vec3 vec3::operator-(vec3& a){vec3 r;Vector::subtract(r,v,a.v,3);return r;}
vec3 vec3::operator-(){vec3 r;Vector::negate(r,v,3);return r;}
vec3& vec3::operator-=(vec3& a){Vector::subtract(v,v,a.v,3);return *this;}
vec3 vec3::operator*(float f){vec3 g;Vector::scale(g,v,f,3);return g;}
vec3& vec3::operator*=(float f){Vector::scale(v,v,f,3);return *this;}
vec3& vec3::scale(float f){Vector::scale(v,v,f,3);return *this;}
vec3& vec3::scale(vec3 a){Vector::scale(v,a.v,v,3);return *this;}
vec3 vec3::minimum(vec3 a){vec3 r; Vector::minimum(r,a.v,v,3);return r;}
vec3 vec3::maximum(vec3 a){vec3 r; Vector::maximum(r,a.v,v,3);return r;}
float vec3::length(){return Vector::length(v,3);}
float vec3::dot(vec3& a,vec3& b){return Vector::dot(a.v,b.v,3);}
vec3 vec3::cross(vec3& a,vec3& b){vec3 c;Vector::cross(c,a.v,b.v);return c;}
vec3& vec3::normalize(){Vector::normalize(v,v,3);return *this;}
vec3& vec3::make(float a,float b,float c){Vector::make(v,3,a,b,c);return *this;}
//vec3& make(double a,double b,double c){VectorMathNamespace::make(v,3,(float)a,(float)b,(float)c);return *this;}
vec3& vec3::negate(){Vector::negate(v,v,3);return *this;}

vec3::operator float* (){return v;}
vec3::operator void* (){return v;}
bool vec3::iszero(){return (v[0]==0 && v[1]==0 && v[2]==0);}

vec4::vec4():x(v[0]),y(v[1]),z(v[2]),w(v[3]){Vector::make(v,4,0,0,0,0);}
vec4::vec4(const vec3& a):x(v[0]),y(v[1]),z(v[2]),w(v[3]){Vector::copy(v,a.v,3);w=0;}
vec4::vec4(const vec4& a):x(v[0]),y(v[1]),z(v[2]),w(v[3]){Vector::copy(v,a.v,4);}
vec4::vec4(float fv[4]):x(v[0]),y(v[1]),z(v[2]),w(v[3]){Vector::copy(v,fv,4);}
vec4::vec4(float x,float y,float z,float t):x(v[0]),y(v[1]),z(v[2]),w(v[3]){Vector::make(v,4,x,y,z,t);}
vec4::vec4(float x,float y,float z):x(v[0]),y(v[1]),z(v[2]),w(v[3]){Vector::make(v,4,x,y,z,1.0f);}
vec4& vec4::operator=(const vec4& a){Vector::copy(v,a.v,4);return *this;}
vec4& vec4::operator=(const vec3& a){Vector::copy(v,a.v,3);v[3]=1.0f;return *this;}
vec4 vec4::operator+(const vec4& a){vec4 r;Vector::sum(r,a.v,v,4);return r;}
vec4& vec4::operator+=(const vec4& a){Vector::sum(v,v,a.v,4);return *this;}
vec4 vec4::operator-(const vec4& a){vec4 r;Vector::subtract(r,a.v,v,4);return r;}
vec4& vec4::operator-=(vec4& a){Vector::subtract(v,v,a.v,4);return *this;}
void vec4::scale(float f){Vector::scale(v,v,f,4);}
void vec4::scale(vec4 a){Vector::scale(v,a.v,v,4);}
bool vec4::operator==(const vec4& a){return Vector::equal(a.v,v,4);}
bool vec4::operator!=(const vec4& a){return !Vector::equal(a.v,v,4);}
float& vec4::operator[](int i){return v[i];}
vec4 vec4::minimum(const vec4& a){vec4 r; Vector::minimum(r,a.v,v,4);return r;}
vec4 vec4::maximum(const vec4& a){vec4 r; Vector::maximum(r,a.v,v,4);return r;}
float vec4::length(){return Vector::length(v,4);}
float vec4::dot(const vec4& a,const vec4& b){return Vector::dot(a.v,b.v,4);}
void vec4::normalize(){Vector::normalize(v,v,4);}
void vec4::make(float a,float b,float c,float d){Vector::make(v,4,a,b,c,d);}
void vec4::negate(){Vector::negate(v,v,4);}
vec4::operator float* (){return v;}
vec4::operator void* (){return v;}
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








float* Matrix::identity(float* m)
{
    m[0]=1.0f;
    m[1]=0.0f;
    m[2]=0.0f;
    m[3]=0.0f;
    m[4]=0.0f;
    m[5]=1.0f;
    m[6]=0.0f;
    m[7]=0.0f;
    m[8]=0.0f;
    m[9]=0.0f;
    m[10]=1.0f;
    m[11]=0.0f;
    m[12]=0.0f;
    m[13]=0.0f;
    m[14]=0.0f;
    m[15]=1.0f;
    return m;
}

float* Matrix::make(float* mm,float a,float b,float c,float d,float e,float f,float g,float h,float i,float l,float m,float n,float o,float p,float q,float r)
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

float* Matrix::sum(float* c,float* a,float* b)
{
    c[0]=a[0]+b[0];		c[1]=a[1]+b[1];		c[2]=a[2]+b[2];		c[3]=a[3]+b[3];
    c[4]=a[4]+b[4];		c[5]=a[5]+b[5];		c[6]=a[6]+b[6];		c[7]=a[7]+b[7];
    c[8]=a[8]+b[8];		c[9]=a[9]+b[9];		c[10]=a[10]+b[10];	c[11]=a[11]+b[11];
    c[12]=a[12]+b[12];	c[13]=a[13]+b[13];	c[14]=a[14]+b[14];	c[15]=a[15]+b[15];

    return c;
}

float* Matrix::subtract(float* c,float* a,float* b)
{
    c[0]=a[0]-b[0];		c[1]=a[1]-b[1];		c[2]=a[2]-b[2];		c[3]=a[3]-b[3];
    c[4]=a[4]-b[4];		c[5]=a[5]-b[5];		c[6]=a[6]-b[6];		c[7]=a[7]-b[7];
    c[8]=a[8]-b[8];		c[9]=a[9]-b[9];		c[10]=a[10]-b[10];	c[11]=a[11]-b[11];
    c[12]=a[12]-b[12];	c[13]=a[13]-b[13];	c[14]=a[14]-b[14];	c[15]=a[15]-b[15];

	return c;
}

float* Matrix::copy(float* b,const float* a)
{
	// if(a==b)return b;

    b[0]=a[0];		b[1]=a[1];		b[2]=a[2];		b[3]=a[3];
    b[4]=a[4];		b[5]=a[5];		b[6]=a[6];		b[7]=a[7];
    b[8]=a[8];		b[9]=a[9];		b[10]=a[10];	b[11]=a[11];
    b[12]=a[12];	b[13]=a[13];	b[14]=a[14];	b[15]=a[15];
	return b;
}

float* Matrix::negate(float* b,float* a)
{
    b[0]=-a[0];
    b[1]=-a[1];
    b[2]=-a[2];
    return b;
}

float* Matrix::multiply(float* c,float* a,float* b)
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

void Matrix::multiply(float* a,float* b)
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


void Matrix::print(float* m)
{
    wprintf(L"%g,%g,%g,%g\n",m[0],m[4],m[8],m[12]);
    wprintf(L"%g,%g,%g,%g\n",m[1],m[5],m[9],m[13]);
    wprintf(L"%g,%g,%g,%g\n",m[2],m[6],m[10],m[14]);
    wprintf(L"%g,%g,%g,%g\n",m[3],m[7],m[11],m[15]);

	wprintf(L"\n");
}

float* Matrix::traspose(float* b,float* a)
{
    float *tt,t[16];

    if(a==b){memcpy(t,a,16*sizeof(float));tt=t;}
	else tt=a;

    for(int i=0;i<4;i++)
    for(int j=0;j<4;j++)
    b[i*4+j]=tt[j*4+i];

    return b;
}
float* Matrix::translate(float* m,vec3 v)
{
    m[12] += m[0] * v[0] + m[4] * v[1] + m[8] * v[2];
    m[13] += m[1] * v[0] + m[5] * v[1] + m[9] * v[2];
    m[14] += m[2] * v[0] + m[6] * v[1] + m[10] * v[2];
    m[15] += m[3] * v[0] + m[7] * v[1] + m[11] * v[2];

    return m;
}

float* Matrix::scale(float* b,float* a,float x,float y,float z)
{
    b[0]=a[0]*x;
    b[5]=a[5]*y;
    b[10]=a[10]*z;

    return b;
}

float* Matrix::scale(float* b,float* a,vec3 v)
{
    b[0]=a[0]*v[0];
    b[5]=a[5]*v[1];
    b[10]=a[10]*v[2];

    return b;
}

float* Matrix::scale(float* b,float* a,float s)
{
    b[0]=a[0]*s;
    b[5]=a[5]*s;
    b[10]=a[10]*s;

    return b;
}

float* Matrix::set(float* m,vec3 s,vec3 r,vec3 t)
{
    float   ms[16],
            mr[16],
            mt[16];

    Matrix::identity(ms);
    Matrix::identity(mr);
    Matrix::identity(mt);

    Matrix::scale(ms,ms,s);
    Matrix::rotate(mr,mr,r);
    Matrix::translate(mt,t);

    //multiply(,)

    return m;
}


void Matrix::ortho(float* m,float l,float r,float b,float t,float n,float f)
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

void Matrix::perspective(float* m,float fov,float ratio,float near,float far)
{
		float D2R = PI / 180.0;
		float yScale = 1.0 / tan(D2R * fov / 2);
		float xScale = yScale / ratio;
		float nearmfar = near - far;
		float mo[] = {
			xScale, 0, 0, 0,
			0, yScale, 0, 0,
			0, 0, (far + near) / nearmfar, -1,
			0, 0, 2*far*near / nearmfar, 0
		};
		memcpy(m, mo, sizeof(float)*16);
}

void Matrix::lookat(float* m,float px,float py,float pz,float cx,float cy,float cz,float ux,float uy,float uz)
{
    float   p[3]={px,py,pz},
            c[3]={cx,cy,cz},
            u[3]={-ux,-uy,-uz},
            f[3],
            s[3];

    Vector::subtract(f,c,p,3);
    Vector::normalize(f,f,3);
    Vector::cross(s,f,u);
    Vector::normalize(s,s,3);
    Vector::cross(u,s,f);

    Matrix::make(m,s[0],u[0],-f[0],0,s[1],u[1],-f[1],0,s[2],u[2],-f[2],0,0,0,0,1);


    m[12] += m[0] * -px + m[4] * -py + m[8] * -pz;
    m[13] += m[1] * -px + m[5] * -py + m[9] * -pz;
    m[14] += m[2] * -px + m[6] * -py + m[10] * -pz;
    m[15] += m[3] * -px + m[7] * -py + m[11] * -pz;

}

void Matrix::lookat(float* m,float* at,float px,float py,float pz,float ux,float uy,float uz)
{
    float atinv[16];
    Matrix::invert(atinv,at);

    float   p[3]={px,py,pz},
            c[3]={-atinv[12],-atinv[13],-atinv[14]},
            u[3]={ux,uy,uz},
            f[3],
            s[3];

    Vector::subtract(f,c,p,3);
    Vector::normalize(f,f,3);
    Vector::cross(s,f,u);
    Vector::normalize(s,s,3);
    Vector::cross(u,s,f);

    float l[16]=
    {
        s[0],u[0],-f[0],0,
        s[1],u[1],-f[1],0,
        s[2],u[2],-f[2],0,
        0,0,0,1
    };

    Matrix::multiply(m,m,atinv);
    Matrix::multiply(m,m,l);


    m[12] += m[0] * -px + m[4] * -py + m[8] * -pz;
    m[13] += m[1] * -px + m[5] * -py + m[9] * -pz;
    m[14] += m[2] * -px + m[6] * -py + m[10] * -pz;
    m[15] += m[3] * -px + m[7] * -py + m[11] * -pz;
}

void Matrix::rotate(float* mr,float* m,float angle,float u,float v,float w)
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

    Matrix::multiply(mr,m,m2);
}

void Matrix::rotate(float* mr,float* m,vec3 v)
{
    float rx[16],ry[16],rz[16];

    Matrix::identity(rx);
    Matrix::identity(ry);
    Matrix::identity(rz);

    if(v[2]!=0)
        Matrix::rotate(rz,rz,v[2],0,0,1);
    if(v[1]!=0)
        Matrix::rotate(ry,ry,v[1],0,1,0);
    if(v[0]!=0)
        Matrix::rotate(rx,rx,v[0],1,0,0);

    float zy[16],zyx[16];

    multiply(zy,ry,rz);
    multiply(zyx,rx,zy);
    multiply(mr,m,zyx);
}

void Matrix::rotate(float* m,float a,float x,float y,float z)
{
	rotate(m,m,a,x,y,z);
}


float Matrix::det(const float* m)
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

float* Matrix::invert(float* i,float* m)
{
    float *mc,mcc[16];

    if(m!=i)mc=m;
    else {memcpy(mcc,m,sizeof(float)*16);mc=mcc;}

    float x=det(m);
    if(!x) return 0;

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


float* Matrix::transform(float* c,float* m,float* a)
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

float* Matrix::transform(float* c,float* m,float x,float y,float z)
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


float* Matrix::orientation(float* c,float* m,float* a)
{
    float *t,tt[3];

    if(a!=c)t=a;
    else {memcpy(tt,a,3*sizeof(float));t=tt;}

    c[0] = m[0] * t[0] + m[4] * t[1] + m[8] * t[2] ;
    c[1] = m[1] * t[0] + m[5] * t[1] + m[9] * t[2] ;
    c[2] = m[2] * t[0] + m[6] * t[1] + m[10] * t[2];

    Vector::normalize(c,c,3);

    return c;
}

float* Matrix::orientation(float* c,float* m,float x,float y,float z)
{
    float v[3]={x,y,z};
    return Matrix::orientation(c,m,v);
}


void Matrix::orientations(float* m,float* a,float* b,float*c)
{
    orientation(a,m,1,0,0);
    orientation(b,m,0,1,0);
    orientation(c,m,0,0,1);
}



#define init_mat2_references m11(v[0]),m12(v[1]),m21(v[2]),m22(v[3])
#define init_mat3_references m11(v[0]),m12(v[1]),m13(v[2]),m21(v[3]),m22(v[4]),m23(v[5]),m31(v[6]),m32(v[7]),m33(v[8])
#define init_mat4_references m11(v[0]),m12(v[1]),m13(v[2]),m14(v[3]),m21(v[4]),m22(v[5]),m23(v[6]),m24(v[7]),m31(v[8]),m32(v[9]),m33(v[10]),m34(v[11]),m41(v[12]),m42(v[13]),m43(v[14]),m44(v[15])


mat2::mat2():init_mat2_references
{
	v[0]=1;v[1]=0;
	v[2]=0;v[3]=1;
}


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




mat4::mat4()/*:init_mat4_references*/{Matrix::identity(v);};
mat4::mat4(const mat4& mm)/*:init_mat4_references*/{Matrix::copy(v,mm.v);}
mat4::mat4(const float* mm)/*:init_mat4_references*/{Matrix::copy(v,mm);}
mat4::mat4(const double* mm)/*:init_mat4_references*/{for(int i=0;i<16;i++)v[i]=(float)mm[i];}
mat4::mat4(mat3 a)/*:init_mat4_references*/{v[0]=a.v[0],v[1]=a.v[1],v[2]=a.v[2],v[4]=a.v[3],v[5]=a.v[4],v[6]=a.v[5],v[8]=a.v[6],v[9]=a.v[7],v[10]=a.v[8];v[3]=v[7]=v[11]=v[12]=v[13]=v[14]=0;v[15]=1.0f;}
mat4& mat4::operator=(mat3 a){v[0]=a.v[0],v[1]=a.v[1],v[2]=a.v[2],v[4]=a.v[3],v[5]=a.v[4],v[6]=a.v[5],v[8]=a.v[6],v[9]=a.v[7],v[10]=a.v[8];v[3]=v[7]=v[11]=v[12]=v[13]=v[14]=0;v[15]=1.0f;return *this;}
mat4& mat4::operator=(const mat4& mm){Matrix::copy(v,mm.v);return *this;}
bool mat4::operator==(mat4& mm){for(int i=0;i<16;i++){if(v[i]!=mm.v[i])return false;}return true;}

mat4 mat4::operator*(mat4 mm){mat4 t;Matrix::multiply(t.v,mm.v,v);return t;}
mat4 mat4::operator*(float f){mat4 t;Matrix::scale(t,v,f);return t;}
vec4 mat4::operator*(vec4 v)
{
	vec4 result;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			float f=this->v[i*4+j];
			result[i] = v[i] + f * v[j];
		}
	}
	return result;
}
mat4& mat4::operator*=(mat4 mm){Matrix::multiply(v,mm.v,v);return *this;}
mat4& mat4::operator*=(vec3 v3){Matrix::scale(v,v,v3);return *this;}
mat4& mat4::operator*=(float f){Matrix::scale(v,v,f);return *this;}
mat4 mat4::operator+(mat4& mm){mat4 t;Matrix::sum(t.v,v,mm.v);return t;}
mat4& mat4::operator+=(mat4& mm){Matrix::sum(v,v,mm.v);return *this;}
mat4 mat4::operator-(mat4& mm){mat4 t;Matrix::subtract(t.v,v,mm.v);return t;}
mat4& mat4::operator-=(mat4& mm){Matrix::subtract(v,v,mm.v);return *this;}
mat4 mat4::operator-(){mat4 t;Matrix::negate(t,v);return t;}
float* 	mat4::operator[](int i){return &v[i*4];}

mat4& mat4::identity(){Matrix::identity(v);return *this;}
mat4& mat4::identity33(){static int idx[9]={0,1,2,4,5,6,8,9,10};for(int i=0;i<10;i++)v[idx[i]]=(idx[i]%5 ? 0.0f : 1.0f);return *this;}
vec3 mat4::transform(vec3 iString){vec3 r;Matrix::transform(r,v,iString);return r;}
void mat4::transformself(vec3& inout){Matrix::transform(inout,v,inout);}
vec3 mat4::transform(float x,float y,float z){vec3 r;Matrix::transform(r,v,x,y,z);return r;}

mat4& mat4::traspose(){Matrix::traspose(v,v);return *this;}

mat4& mat4::rotate(float a,float x,float y,float z){Matrix::rotate(v,v,a,x,y,z);return *this;}
mat4& mat4::rotate(float a,vec3 v3){Matrix::rotate(v,v,a,v3[0],v3[1],v3[2]);return *this;}
mat4& mat4::rotate(vec3 v3){Matrix::rotate(v,v,v3);return *this;}

mat4& mat4::scale(float scalar){Matrix::scale(v,v,scalar);return *this;}
mat4& mat4::scale(vec3 scalevector){Matrix::scale(v,v,scalevector);return *this;}

mat4& mat4::srt(vec3 s,vec3 r,vec3 t){mat4 ms;ms.scale(s);mat4 mr;Matrix::rotate(mr,mr,r);mat4 mt;mt.translate(t);*this=ms*mr*mt;return *this;}
mat4& mat4::trs(vec3 t,vec3 r,vec3 s){mat4 ms;ms.scale(s);mat4 mr;Matrix::rotate(mr,mr,r);mat4 mt;mt.translate(t);*this=mt*mr*ms;return *this;}


mat4& mat4::copy(float* mm){Matrix::copy(mm,v);return *this;}
mat4& mat4::copy33(mat4& mm){static int idx[9]={0,1,2,4,5,6,8,9,10};for(int i=0;i<10;i++)v[idx[i]]=mm.v[idx[i]];return *this;}
mat4& mat4::move(vec3 v3){v[12] = v3[0];v[13] = v3[1];v[14] = v3[2];return *this;}

mat4& mat4::translate(float x,float y,float z){vec3 t(x,y,z);Matrix::translate(v,t);return *this;}
mat4& mat4::translate(vec3 v3){Matrix::translate(v,v3);return *this;}

mat4& mat4::invert(){Matrix::invert(v,v);return *this;}
mat4 mat4::inverse(){float ret[16];Matrix::invert(ret,v);return mat4(ret);}

void mat4::print(){Matrix::print(v);}

mat4& mat4::perspective(float fov,float ratio,float near,float far){Matrix::perspective(v,fov,ratio,near,far);return *this;}
//mat4& mat4::lookat(vec3 target,vec3 p,vec3 y){matrix::lookat(v,p[0],p[1],p[2],target[0],target[1],target[2],y[0],y[1],y[2]);return *this;}
mat4& mat4::lookat(vec3 target,vec3 up)
{
	vec3 p=position();
	Matrix::lookat(v,p[0],p[1],p[2],target[0],target[1],target[2],up[0],up[1],up[2]);
	return *this;
}

vec3 mat4::position(){vec3 t;Matrix::transform(t,v,0,0,0);return t;}

void mat4::axes(vec3& a,vec3& b,vec3& c){Matrix::orientations(v,a,b,c);}
vec3 mat4::axis(vec3 iString){Matrix::orientation(iString,v,iString);return iString;}
vec3 mat4::axis(float x,float y,float z){vec3 out(x,y,z);Matrix::orientation(out,v,out);return out;}

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

	Matrix::copy(this->v,om);
	return *this;
}

void mat4::zero(){memset(this->v,0,sizeof(float[16]));}

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////////Timer//////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

Timer* _________timer=0;

Timer::Timer():renderFps(60)
{
	_________timer=this->Instance();
}

void Timer::update(){};

DLLBUILD Timer* GetTimer(){return _________timer;}

Timer* Timer::GetInstance(){return GetTimer();}

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////////AABB//////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


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

bool isPointOnPlane(vec4 plane,vec3 point)
{
	return -plane.w==plane.x*point.x+plane.y*point.y+plane.z*point.z;
}

bool isPointOnLine(vec3 a,vec3 b,vec3 point)
{
	vec3	aa(a.x,b.x,0),
			bb(a.y,b.y,0),
			cc(a.z,b.z,1);

	float* line[3]=
	{
		aa,bb,cc
	};


	vec3 result;

	eqSolve(result,3,3,line);

	return true;
}

vec4 plane(vec3 a,vec3 b,vec3 c)
{
	vec3 ab=b-a;
	vec3 ac=c-a;

	vec4 abxac=vec3::cross(ab,ac);

	abxac.w=-(abxac.x*a.x+abxac.y*a.y+abxac.z*a.z);

	return abxac;
}


void printEqSys(int nrow,int ncol,float** eqsys)
{
	for(int i=0;i<nrow;i++)
		for(int j=0;j<ncol;j++)
		{
			wprintf(L"%3.2f ",eqsys[i][j]);
			if(j==ncol-1)
				wprintf(L"\n");
			if(i==nrow-1 && j==ncol-1)
				wprintf(L"\n");
		}
}

void eqSolve(float* result,int nrow,int ncol,float** _eqsys)
{
	printEqSys(nrow,ncol,_eqsys);

	float** eqsys=new float*[nrow];

	for(int i=0;i<nrow;i++)
	{
		eqsys[i]=new float[ncol];
		memcpy(eqsys[i],&_eqsys[i][0],ncol*sizeof(float));
	}

	printEqSys(nrow,ncol,eqsys);

	for(int i=0;i<nrow;i++)
	{
		if(eqsys[i][i]!=0)
			Vector::scale(&eqsys[i][0],&eqsys[i][0],1.0f/eqsys[i][i],ncol);

		for(int j=0;j<ncol-1;j++)
		{
			if(i==j)
			{
				for(int k=0;k<nrow;k++)
				{
					if(k!=i && eqsys[k][j] && eqsys[i][j])
					{
						float *p=new float[ncol];
						memcpy(p,&eqsys[i][0],ncol*sizeof(float));

						Vector::scale(p,p,eqsys[k][j],ncol);
						Vector::subtract(&eqsys[k][0],&eqsys[k][0],p,ncol);
						delete [] p;
					}
				}
			}
		}
	}

	printEqSys(nrow,ncol,eqsys);

	result[ncol-1]=0;

	for(int i=0;i<nrow;i++)
		result[i]=eqsys[i][ncol-1];

	for(int i=0;i<nrow;i++)
		delete [] eqsys[i];

	delete [] eqsys;
}


Thread::Thread():exit(false){}
Thread::~Thread()
{
	for(std::list<Task*>::iterator it=this->tasks.begin();it!=this->tasks.end();it++)
		SAFEDELETE(*it);

	this->tasks.clear();
}

Task* Thread::NewTask(std::function<void()> iFunction,bool iRemove,bool iBlock)
{
	Task* task=new Task;

	task->func=iFunction;
	task->remove=iRemove;
	task->executing=false;
	task->pause=iBlock;
	task->owner=this;

	tasks.push_back(task);

	return task;
}

void Thread::DestroyTask(Task* iTask)
{
	this->tasks.erase(std::find(this->tasks.begin(),this->tasks.end(),iTask));

	SAFEDELETE(iTask);
}

void Thread::Block(bool iBlock)
{
	this->pause=iBlock;

	if(iBlock)
	{
		while(this->executing)
			this->pause=iBlock;
	}
}

void Task::Block(bool iBlock)
{
	this->pause=iBlock;

	if(iBlock)
	{
		while(this->executing)
			this->pause=iBlock;
	}
}



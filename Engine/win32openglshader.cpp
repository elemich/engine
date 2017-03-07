#include "win32.h"

#pragma message (LOCATION " @mic: this should go to common opengl part of the project cause there is no os-related call within")

extern PFNWGLCHOOSEPIXELFORMATEXTPROC wglChoosePixelFormatARB;
extern PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;

#ifndef GL_GLEXT_PROTOTYPES

extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
extern PFNGLDETACHSHADERPROC glDetachShader;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLENABLEVERTEXARRAYATTRIBPROC glEnableVertexArrayAttrib;
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
extern PFNGLACTIVETEXTUREPROC glActiveTexture;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLUNIFORM3FPROC glUniform3f;
extern PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
extern PFNGLUNIFORM3FVPROC glUniform3fv;
extern PFNGLUNIFORM4FVPROC glUniform4fv;
extern PFNGLTEXBUFFERPROC glTexBuffer;
extern PFNGLTEXTUREBUFFERPROC glTextureBuffer;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData;


#endif


void glCheckError()
{
	GLenum err=glGetError();
	if(err!=GL_NO_ERROR)
	{
		printf("glErr %d",err);

		HGLRC currentContext=wglGetCurrentContext();
		HDC currentContextDC=wglGetCurrentDC();

		__debugbreak();
	}
}



int simple_shader(int shader_type, const char* shader_src)
{
	GLint compile_success = 0;
	GLchar message[1024];
	int len=0;
	int shader_id;

	shader_id = glCreateShader(shader_type);glCheckError();

	if(!shader_id)
	{
		printf("glCreateShader error for %s,%s\n",shader_type,shader_src);glCheckError();
		__debugbreak();
		return 0;
	}

	glShaderSource(shader_id, 1, &shader_src,NULL);glCheckError();
	glCompileShader(shader_id);glCheckError();
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_success);glCheckError();

	if (GL_FALSE==compile_success)
	{
		printf( "\nglCompileShader error for %s\n",shader_src);
		glGetShaderInfoLog(shader_id, sizeof(message), &len, message);
		__debugbreak();
	}

	glGetShaderInfoLog(shader_id, sizeof(message), &len, message);

	if(len && len<sizeof(message))
	{
		char shaderName[100];

		switch(shader_type)
		{
		case GL_VERTEX_SHADER:
			sprintf_s(shaderName,"%s","GL_VERTEX_SHADER");
			break;
		case GL_FRAGMENT_SHADER:
			sprintf_s(shaderName,"%s","GL_FRAGMENT_SHADER");
			break;
		}

		char* s=strrchr(message,'\n');
		if(s)
			*s=0;
		printf("%s:%s",shaderName,message);
	}

	return shader_id;
}


int create_program(const char* vertexsh,const char* fragmentsh)
{
	GLint link_success=0;
	GLint program=0;
	GLint vertex_shader=0;
	GLint fragment_shader=0;
	GLchar message[1024]={0};
	GLint len=0;

	program = glCreateProgram();glCheckError();
	
	if(!program)
	{
		printf("glCreateProgram error for %s,%s\n",vertexsh,fragmentsh);
		__debugbreak();
		return 0;
	}

	vertex_shader=simple_shader(GL_VERTEX_SHADER, vertexsh);
	fragment_shader=simple_shader(GL_FRAGMENT_SHADER, fragmentsh);

	glAttachShader(program, vertex_shader);glCheckError();
	glAttachShader(program, fragment_shader);glCheckError();
	glLinkProgram(program);glCheckError();
	glGetProgramiv(program, GL_LINK_STATUS, &link_success);glCheckError();

	if (GL_FALSE==link_success)
	{
		printf("glLinkProgram error for %s\n",message);
		__debugbreak();
	}

	glGetProgramInfoLog(program,sizeof(message),&len,message);glCheckError();

	if(len && len<sizeof(message))
		printf("%s",message);
	

	return program;
}


//--------------------OpenGLShader-------------------------


ShaderInterface* OpenGLShader::Create(const char* name,const char* pix,const char* frag)
{
	if(shadersPool.Find(name))
		return 0;

	int program=create_program(pix,frag);

	if(program)
	{
		ShaderInterface* shader=new OpenGLShader();
		shader->SetName(name);
		shader->SetProgram(program);

		glUseProgram(program);glCheckError();

		shader->init();
		shader->Use();
		
		shadersPool.pool.push_back(shader);

		printf("adding %s to shaders list\n",name);

		return shader;
	}
	else

		printf("error creating shader %s\n",name);

	return 0;
}



unsigned int& OpenGLShader::GetBufferObject()
{
	return vbo;
}

int OpenGLShader::GetProgram(){return program;}
void OpenGLShader::SetProgram(int p){program=p;}

int OpenGLShader::GetUniform(int slot,char* var)
{
	return glGetUniformLocation(slot,var);glCheckError();
}
int OpenGLShader::GetAttrib(int slot,char* var)
{
	return glGetAttribLocation(slot,var);glCheckError();
}

void OpenGLShader::SetProjectionMatrix(float* pm)
{
	if(!pm)
		return;

	this->SetMatrix4f(this->GetProjectionSlot(),pm);

}
void OpenGLShader::SetModelviewMatrix(float* mm)
{
	if(!mm)
		return;

	this->SetMatrix4f(this->GetModelviewSlot(),mm);
}

void OpenGLShader::SetByMatrixStack()
{
	this->SetProjectionMatrix(MatrixStack::GetProjectionMatrix());
	this->SetModelviewMatrix(MatrixStack::GetModelviewMatrix());
}

void OpenGLShader::Use()
{
	ShaderInterface* current_shader=shadersPool.GetCurrent();

	if(!program || current_shader==this)
		return;
	
	glUseProgram(program);glCheckError();

	this->SetByMatrixStack();
	
	shadersPool.SetCurrent(this);
}

const char* OpenGLShader::GetPixelShader(){return 0;}
const char* OpenGLShader::GetFragmentShader(){return 0;}

int OpenGLShader::init()
{
	mat4 m;

	int proj=GetProjectionSlot();
	int mdlv=GetModelviewSlot();

	bool bOk = this->SetMatrix4f(proj,m) && this->SetMatrix4f(mdlv,m);

	return bOk;
}

int OpenGLShader::GetAttribute(const char* attrib)
{
	int location=glGetAttribLocation(program,attrib);glCheckError();
	return location;
}

int OpenGLShader::GetUniform(const char* uniform)
{
	int location=glGetUniformLocation(program,uniform);glCheckError();
	return location;
}

int OpenGLShader::GetPositionSlot()
{
	return GetAttribute("position");
}
int OpenGLShader::GetColorSlot()
{
	return GetAttribute("color");
}
int OpenGLShader::GetProjectionSlot()
{
	return GetUniform("projection");
}
int OpenGLShader::GetModelviewSlot()
{
	return GetUniform("modelview");
}
int OpenGLShader::GetTexcoordSlot()
{
	return GetAttribute("texcoord");
}
int OpenGLShader::GetTextureSlot()
{
	return GetUniform("texture");
}
int OpenGLShader::GetLightposSlot()
{
	return GetUniform("lightpos");
}
int OpenGLShader::GetLightdiffSlot()
{
	return GetUniform("lightdiff");
}
int OpenGLShader::GetLightambSlot()
{
	return GetUniform("lightamb");
}
int OpenGLShader::GetNormalSlot()
{
	return GetAttribute("normal");
}

bool OpenGLShader::SetMatrix4f(int slot,float* mtx)
{
	if(slot<0 || !mtx)
		return false;

	glUniformMatrix4fv(slot,1,0,mtx);glCheckError();
	return true;
}

void OpenGLShader::SetName(const char* n)
{
	name=n;
}
const char* OpenGLShader::GetName()
{
	return name;
}



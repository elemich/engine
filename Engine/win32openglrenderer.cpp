#include "win32.h"

extern App* ___app;


void glCheckError()
{
	GLenum err=glGetError();
	if(err!=GL_NO_ERROR)
	{
		printf("glErr %d",err);
	}
}


int simple_shader(int shader_type, const char* shader_src)
{
	int compile_success = 0;
	char message[1024];
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

	if(len)
		printf("%s\n%s\n",message,len>sizeof(message) ? "more..." : "");

	return shader_id;
}


int create_program(const char* vertexsh,const char* fragmentsh)
{
	int link_success;
	int program;
	int vertex_shader;
	int fragment_shader;
	GLchar message[1024];
	int len;

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

	if(len)
		printf("%s\n%s\n",message,len>sizeof(message) ? "more..." : "");

	return program;
}


//--------------------OpenGLRenderer-------------------------


ShaderInterface* OpenGLShader::Create(const char* name,const char* pix,const char* frag)
{
	int program=create_program(pix,frag);

	if(program)
	{
		ShaderInterface* shader=new OpenGLShader();
		shader->SetName(name);
		shader->SetProgram(program);

		shader->Use();
		shader->init();

		ShaderManager::shaders+=shader;

		printf("adding %s to shaders list\n",name);

		return shader;
	}
	else

		printf("error creating shader %s\n",name);

	return NULL;
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
	ShaderInterface* current_shader=ShaderManager::GetCurrent();

	if(!program || current_shader==this)
		return;
	
	glUseProgram(program);glCheckError();

	this->SetByMatrixStack();
	
	ShaderManager::SetCurrent(this);
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
	//return -1;
}

int OpenGLShader::GetUniform(const char* uniform)
{
	int location=glGetUniformLocation(program,uniform);glCheckError();
	/*if(location<0)
	__debugbreak();*/
	return location;
	//return -1;
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




//--------------------OpenGLRenderer-------------------------

LRESULT CALLBACK OpenGLProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	OpenGLFixedRenderer* renderer=(OpenGLFixedRenderer*)GetWindowLong(hwnd,GWL_USERDATA);

	LRESULT result=0;

	if(renderer)
		renderer->Render();

	switch(msg)
	{
	case WM_ERASEBKGND:
		return (LRESULT)1;
	default:
		result=DefWindowProc(hwnd,msg,wparam,lparam);
	}

	return result;
}


OpenGLFixedRenderer::OpenGLFixedRenderer()
{
	hglrc=0;
}

char* OpenGLFixedRenderer::Name()
{
	return 0;
}

void OpenGLFixedRenderer::Create(HWND container)
{
	hwnd=CreateWindow(WC_OPENGLWINDOW,"OpenGLFixedRenderer",WS_CHILD,CW_USEDEFAULT,CW_USEDEFAULT,100,100,container,0,0,0);

	hdc=GetDC(hwnd);

	if(!hdc)
		MessageBox(0,"Getting Device Context","GetDC",MB_OK|MB_ICONEXCLAMATION);

	PIXELFORMATDESCRIPTOR _pfd=         
	{
		sizeof(PIXELFORMATDESCRIPTOR), 
		1,                             
		PFD_DRAW_TO_WINDOW |           
		PFD_SUPPORT_OPENGL |           
		PFD_SUPPORT_GDI ,//PFD_DOUBLEBUFFER,              
		PFD_TYPE_RGBA,                 
		32,                            
		0, 0, 0, 0, 0, 0,              
		0,                             
		0,                             
		0,                             
		0, 0, 0, 0,                    
		32,                            
		0,                             
		0,                             
		PFD_MAIN_PLANE,                
		0,                             
		0, 0, 0                        
	};  

	pfd = _pfd;

	pixelFormat = ChoosePixelFormat(hdc,&pfd);

	if(pixelFormat==0)
		MessageBox(0,"pixel format error","ChoosePixelFormat",MB_OK|MB_ICONEXCLAMATION);

	if(!SetPixelFormat(hdc,pixelFormat,&pfd))
		MessageBox(0,"pixel format error","DescribePixelFormat",MB_OK|MB_ICONEXCLAMATION);

	hglrc = wglCreateContext(hdc);

	if(!hglrc)
		MessageBox(0,"creating context error","wglCreateContext",MB_OK|MB_ICONEXCLAMATION);

	wglMakeCurrent(hdc,hglrc);

	ReleaseDC(hwnd,hdc);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		printf("Error: %s\n", glewGetErrorString(err));
		__debugbreak();
	}

	printf("Status: Using GL %s\n", glGetString(GL_VERSION));
	printf("Status: GLSL ver %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	SetWindowLongPtr(hwnd,GWL_USERDATA,(LONG)this);
}

void OpenGLFixedRenderer::Render()
{
	if(!hglrc)
		return;

	hdc=GetDC(hwnd);

	wglMakeCurrent(hdc,hglrc);



	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	glClearColor(0,0,0,1);

	RECT rc;
	GetClientRect(hwnd,&rc);
	glViewport(rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top);


	SwapBuffers(hdc);
	ReleaseDC(hwnd,hdc);
}



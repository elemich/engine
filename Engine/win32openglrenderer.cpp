#include "win32.h"

#include "shader_data.cpp"

#pragma message (LOCATION " processNode should go to common part (maybe entities.h) of the project cause there is no os-related call within")
#pragma message (LOCATION " multiple opengl context needs glew32mx.lib")
#pragma message (LOCATION " TODO: move OpenGL bitmap render in the RendererViewportInterface cause is for the gui only")

/*
GLuint OpenGLRenderer::vertexArrayObject=0;
GLuint OpenGLRenderer::vertexBufferObject=0;
GLuint OpenGLRenderer::textureBufferObject=0;
GLuint OpenGLRenderer::indicesBufferObject=0;
GLuint OpenGLRenderer::frameBuffer=0;
GLuint OpenGLRenderer::textureColorbuffer=0;
GLuint OpenGLRenderer::textureRenderbuffer=0;
GLuint OpenGLRenderer::pixelBuffer=0;
GLuint OpenGLRenderer::renderBufferColor=0;
GLuint OpenGLRenderer::renderBufferDepth=0;*/

PFNWGLCHOOSEPIXELFORMATEXTPROC wglChoosePixelFormatARB = 0;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 0;

#ifndef GL_GLEXT_PROTOTYPES
extern PFNGLATTACHSHADERPROC glAttachShader=0;
extern PFNGLBINDBUFFERPROC glBindBuffer=0;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray=0;
extern PFNGLBUFFERDATAPROC glBufferData=0;
extern PFNGLCOMPILESHADERPROC glCompileShader=0;
extern PFNGLCREATEPROGRAMPROC glCreateProgram=0;
extern PFNGLCREATESHADERPROC glCreateShader=0;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers=0;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram=0;
extern PFNGLDELETESHADERPROC glDeleteShader=0;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays=0;
extern PFNGLDETACHSHADERPROC glDetachShader=0;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray=0;
extern PFNGLENABLEVERTEXARRAYATTRIBPROC glEnableVertexArrayAttrib=0;
extern PFNGLGENBUFFERSPROC glGenBuffers=0;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays=0;
extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation=0;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog=0;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv=0;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog=0;
extern PFNGLGETSHADERIVPROC glGetShaderiv=0;
extern PFNGLLINKPROGRAMPROC glLinkProgram=0;
extern PFNGLSHADERSOURCEPROC glShaderSource=0;
extern PFNGLUSEPROGRAMPROC glUseProgram=0;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer=0;
extern PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation=0;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation=0;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv=0;
extern PFNGLACTIVETEXTUREPROC glActiveTexture=0;
extern PFNGLUNIFORM1IPROC glUniform1i=0;
extern PFNGLUNIFORM1FPROC glUniform1f=0;
extern PFNGLGENERATEMIPMAPPROC glGenerateMipmap=0;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray=0;
extern PFNGLUNIFORM3FVPROC glUniform3fv=0;
extern PFNGLUNIFORM4FVPROC glUniform4fv=0;
extern PFNGLTEXBUFFERPROC glTexBuffer=0;
extern PFNGLTEXTUREBUFFERPROC glTextureBuffer=0;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData=0;
extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers=0;
extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers=0;
extern PFNGLREADNPIXELSPROC glReadnPixels=0;
extern PFNGLUNIFORM2FPROC glUniform2f=0;
extern PFNGLUNIFORM2FVPROC glUniform2fv=0;
extern PFNGLUNIFORM3FPROC glUniform3f=0;
extern PFNGLUNIFORM4FPROC glUniform4f=0;
extern PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB=0;
#endif


bool GLEW_INITED=false;

#define USE_COMMON_PIXELFORMAT_SYSTEM 1


vec3 rayStart;
vec3 rayEnd;


OpenGLRenderer::OpenGLRenderer(TabContainer* _tabContainer):
tabContainer(_tabContainer)
{
	hglrc=0;
}

OpenGLRenderer::~OpenGLRenderer()
{
}

char* OpenGLRenderer::Name()
{
	return 0;
}

void OpenGLRenderer::Create(HWND hwnd)
{
	hdc=GetDC(hwnd);

	RECT r;
	GetClientRect(hwnd,&r);

	int width=(float)(int)(r.right-r.left);
	int height=(float)(int)(r.bottom-r.top);

	DWORD error=0;


	int pixelFormat;

	for(int i=0;i<1;i++)
	{
		if(!hdc)
			MessageBox(0,"Getting Device Context","GetDC",MB_OK|MB_ICONEXCLAMATION);

		PIXELFORMATDESCRIPTOR pfd={0};
		pfd.nVersion=1;
		pfd.nSize=sizeof(PIXELFORMATDESCRIPTOR);
		pfd.dwFlags=PFD_DOUBLEBUFFER|PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL;
		pfd.iPixelType=PFD_TYPE_RGBA;
		pfd.cColorBits=32;
		pfd.cDepthBits=32;
		pfd.cStencilBits=32;

		pixelFormat = ChoosePixelFormat(hdc,&pfd);

		error=GetLastError();

		if(error!=NO_ERROR && error!=ERROR_OLD_WIN_VERSION)
			__debugbreak();

		if(pixelFormat==0)
			__debugbreak();

		if(!SetPixelFormat(hdc,pixelFormat,&pfd))
			__debugbreak();

		if(!(hglrc = wglCreateContext(hdc)))
			__debugbreak();

		if(!wglMakeCurrent(hdc,hglrc))
			__debugbreak();

		if(!wglChoosePixelFormatARB)wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATEXTPROC) wglGetProcAddress("wglChoosePixelFormatARB");
		if(!wglCreateContextAttribsARB)wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");

		#ifndef GL_GLEXT_PROTOTYPES

		if(!glAttachShader)glAttachShader = (PFNGLATTACHSHADERPROC) wglGetProcAddress("glAttachShader");
		if(!glBindBuffer)glBindBuffer = (PFNGLBINDBUFFERPROC) wglGetProcAddress("glBindBuffer");
		if(!glBindVertexArray) glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) wglGetProcAddress("glBindVertexArray");
		if(!glBufferData) glBufferData = (PFNGLBUFFERDATAPROC) wglGetProcAddress("glBufferData");
		if(!glCompileShader) glCompileShader = (PFNGLCOMPILESHADERPROC) wglGetProcAddress("glCompileShader");
		if(!glCreateProgram) glCreateProgram = (PFNGLCREATEPROGRAMPROC) wglGetProcAddress("glCreateProgram");
		if(!glCreateShader) glCreateShader = (PFNGLCREATESHADERPROC) wglGetProcAddress("glCreateShader");
		if(!glDeleteBuffers) glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) wglGetProcAddress("glDeleteBuffers");
		if(!glDeleteProgram) glDeleteProgram = (PFNGLDELETEPROGRAMPROC) wglGetProcAddress("glDeleteProgram");
		if(!glDeleteShader) glDeleteShader = (PFNGLDELETESHADERPROC) wglGetProcAddress("glDeleteShader");
		if(!glDeleteVertexArrays) glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC) wglGetProcAddress("glDeleteVertexArrays");
		if(!glDetachShader) glDetachShader = (PFNGLDETACHSHADERPROC) wglGetProcAddress("glDetachShader");
		if(!glEnableVertexAttribArray) glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) wglGetProcAddress("glEnableVertexAttribArray");
		if(!glEnableVertexArrayAttrib) glEnableVertexArrayAttrib = (PFNGLENABLEVERTEXARRAYATTRIBPROC) wglGetProcAddress("glEnableVertexArrayAttrib");
		if(!glGenBuffers) glGenBuffers = (PFNGLGENBUFFERSPROC) wglGetProcAddress("glGenBuffers");
		if(!glGenVertexArrays) glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) wglGetProcAddress("glGenVertexArrays");
		if(!glGetAttribLocation) glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) wglGetProcAddress("glGetAttribLocation");
		if(!glGetProgramInfoLog) glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) wglGetProcAddress("glGetProgramInfoLog");
		if(!glGetProgramiv) glGetProgramiv = (PFNGLGETPROGRAMIVPROC) wglGetProcAddress("glGetProgramiv");
		if(!glGetShaderInfoLog) glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) wglGetProcAddress("glGetShaderInfoLog");
		if(!glGetShaderiv) glGetShaderiv = (PFNGLGETSHADERIVPROC) wglGetProcAddress("glGetShaderiv");
		if(!glLinkProgram) glLinkProgram = (PFNGLLINKPROGRAMPROC) wglGetProcAddress("glLinkProgram");
		if(!glShaderSource) glShaderSource = (PFNGLSHADERSOURCEPROC) wglGetProcAddress("glShaderSource");
		if(!glUseProgram) glUseProgram = (PFNGLUSEPROGRAMPROC) wglGetProcAddress("glUseProgram");
		if(!glVertexAttribPointer) glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) wglGetProcAddress("glVertexAttribPointer");
		if(!glBindAttribLocation) glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) wglGetProcAddress("glBindAttribLocation");
		if(!glGetUniformLocation) glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) wglGetProcAddress("glGetUniformLocation");
		if(!glUniformMatrix4fv) glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) wglGetProcAddress("glUniformMatrix4fv");
		if(!glActiveTexture) glActiveTexture = (PFNGLACTIVETEXTUREPROC) wglGetProcAddress("glActiveTexture");
		if(!glUniform1i) glUniform1i = (PFNGLUNIFORM1IPROC) wglGetProcAddress("glUniform1i");
		if(!glUniform1f) glUniform1f = (PFNGLUNIFORM1FPROC) wglGetProcAddress("glUniform1f");
		if(!glUniform3f) glUniform3f = (PFNGLUNIFORM3FPROC) wglGetProcAddress("glUniform3f");
		if(!glGenerateMipmap) glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) wglGetProcAddress("glGenerateMipmap");
		if(!glDisableVertexAttribArray) glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) wglGetProcAddress("glDisableVertexAttribArray");
		if(!glUniform3fv) glUniform3fv = (PFNGLUNIFORM3FVPROC) wglGetProcAddress("glUniform3fv");
		if(!glUniform4fv) glUniform4fv = (PFNGLUNIFORM4FVPROC) wglGetProcAddress("glUniform4fv");
		if(!glTexBuffer) glTexBuffer = (PFNGLTEXBUFFERPROC) wglGetProcAddress("glTexBuffer");
		if(!glTextureBuffer) glTextureBuffer = (PFNGLTEXTUREBUFFERPROC) wglGetProcAddress("glTextureBuffer");
		if(!glBufferSubData) glBufferSubData = (PFNGLBUFFERSUBDATAPROC) wglGetProcAddress("glBufferSubData");
		if(!glGenFramebuffers)glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) wglGetProcAddress("glGenFramebuffers");
		if(!glGenRenderbuffers)glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) wglGetProcAddress("glGenRenderbuffers");
		/*if(!glBufferSubData)glViewportIndexedf = (PFNGLVIEWPORTINDEXEDFPROC) wglGetProcAddress("glViewportIndexedf");
		if(!glAddSwapHintRectWIN)glAddSwapHintRectWIN = (PFNGLADDSWAPHINTRECTWINPROC) wglGetProcAddress("glAddSwapHintRectWIN");
		if(!glBindFramebuffer)glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) wglGetProcAddress("glBindFramebuffer");
		if(!glNamedRenderbufferStorage)glNamedRenderbufferStorage = (PFNGLNAMEDRENDERBUFFERSTORAGEPROC) wglGetProcAddress("glNamedRenderbufferStorage");
		if(!glRenderbufferStorage)glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) wglGetProcAddress("glRenderbufferStorage");
		if(!glBindRenderbuffer)glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) wglGetProcAddress("glBindRenderbuffer");
		if(!glFramebufferRenderbuffer)glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) wglGetProcAddress("glFramebufferRenderbuffer");
		if(!glBlitFramebuffer)glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC) wglGetProcAddress("glBlitFramebuffer");
		if(!glCheckFramebufferStatus)glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) wglGetProcAddress("glCheckFramebufferStatus");
		if(!glDrawBuffers)glDrawBuffers = (PFNGLDRAWBUFFERSPROC) wglGetProcAddress("glDrawBuffers");
		if(!glBlitNamedFramebuffer) glBlitNamedFramebuffer = (PFNGLBLITNAMEDFRAMEBUFFERPROC) wglGetProcAddress("glBlitNamedFramebuffer");
		if(!glFramebufferTexture)glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC)wglGetProcAddress("glFramebufferTexture");
		if(!glFramebufferTexture2D)glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) wglGetProcAddress("glFramebufferTexture2D");*/
		if(!glReadnPixels)glReadnPixels = (PFNGLREADNPIXELSPROC) wglGetProcAddress("glReadnPixels");
		if(!glUniform2f)glUniform2f = (PFNGLUNIFORM2FPROC) wglGetProcAddress("glUniform2f");
		if(!glUniform2fv)glUniform2fv = (PFNGLUNIFORM2FVPROC) wglGetProcAddress("glUniform2fv");
		if(!glUniform3f)glUniform3f = (PFNGLUNIFORM3FPROC) wglGetProcAddress("glUniform3f");
		if(!glUniform4f)glUniform4f = (PFNGLUNIFORM4FPROC) wglGetProcAddress("glUniform4f");
		if(!wglGetPixelFormatAttribivARB)wglGetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC) wglGetProcAddress("wglGetPixelFormatAttribivARB");
#endif

		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hglrc);
	}
	
	const int pixelFormatAttribList[] = 
	{
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_DEPTH_BITS_ARB, 32,
		WGL_STENCIL_BITS_ARB, 32,
		//WGL_SWAP_COPY_ARB,GL_TRUE,        //End
		0
	};

	UINT numFormats;

	if(!wglChoosePixelFormatARB(hdc, pixelFormatAttribList, NULL, 1, &pixelFormat, &numFormats))
		MessageBox(0,"wglChoosePixelFormatARB fails","Engine",MB_OK|MB_ICONEXCLAMATION);
	

	const int versionAttribList[] = 
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB,1,
		WGL_CONTEXT_MINOR_VERSION_ARB,0, 
		0,        //End
	};

	if(!(hglrc = wglCreateContextAttribsARB(hdc, 0, versionAttribList)))
		MessageBox(0,"wglCreateContextAttribsARB fails","Engine",MB_OK|MB_ICONEXCLAMATION);


	if(hglrc)
		printf("TABCONTAINER: %p, HGLRC: %p, HDC: %p\n",this->tabContainer,hglrc,hdc);
	
	if(!wglMakeCurrent(hdc,hglrc))
		__debugbreak();

	//if(!vertexArrayObject)
	{
		{
			glGenFramebuffers(1,&frameBuffer);glCheckError();
			
			glGenTextures(1,&textureColorbuffer);glCheckError();
			glGenTextures(1,&textureRenderbuffer);glCheckError();

			glGenRenderbuffers(1,&renderBufferColor);glCheckError();
			glGenRenderbuffers(1,&renderBufferDepth);glCheckError();
		}
		
		glGenVertexArrays(1, &vertexArrayObject);glCheckError();
		glBindVertexArray(vertexArrayObject);glCheckError();

		glGenBuffers(1,&vertexBufferObject);glCheckError();
		glGenBuffers(1,&textureBufferObject);glCheckError();
		/*
		//glGenBuffers(1,&indicesBufferObject);

		glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);
		//glBindBuffer(GL_ARRAY_BUFFER,indicesBufferObject);

		glBufferData(GL_ARRAY_BUFFER,100000,0,GL_DYNAMIC_DRAW);*/


		glGenBuffers(1, &pixelBuffer);
	}

	printf("Status: Using GL %s\n", glGetString(GL_VERSION));
	printf("Status: GLSL ver %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));

	this->unlit=OpenGLShader::Create("unlit",unlit_vert,unlit_frag);
	this->unlit_color=OpenGLShader::Create("unlit_color",unlit_color_vert,unlit_color_frag);
	this->unlit_texture=OpenGLShader::Create("unlit_texture",unlit_texture_vs,unlit_texture_fs);
	this->font=OpenGLShader::Create("font",font_pixsh,font_frgsh);
	this->shaded_texture=OpenGLShader::Create("shaded_texture",texture_vertex_shaded_vert,texture_vertex_shaded_frag);

	this->shaders.push_back(this->unlit);
	this->shaders.push_back(this->unlit_color);
	this->shaders.push_back(this->unlit_texture);
	this->shaders.push_back(this->font);
	this->shaders.push_back(this->shaded_texture);
}


void OpenGLRenderer::ChangeContext()
{
	if(!hglrc || !hdc)
	{
		__debugbreak();
		return;
	}

	if(hglrc != wglGetCurrentContext() || hdc!=wglGetCurrentDC())
	{
		if(!wglMakeCurrent(hdc,hglrc))
			__debugbreak();
	}
}




void OpenGLRenderer::draw(Light*)
{

}

void OpenGLRenderer::draw(vec2)
{

}

void OpenGLRenderer::draw(Gizmo* gizmo)
{
	this->draw(vec3(0,0,0),vec3(10,0,0),vec3(1,0,0));
	this->draw(vec3(0,0,0),vec3(0,10,0),vec3(0,1,0));
	this->draw(vec3(0,0,0),vec3(0,0,10),vec3(0,0,1));
}

void OpenGLRenderer::draw(Piped* piped)
{
	
}


void OpenGLRenderer::draw(vec3 point,float psize,vec3 col)
{
	ShaderInterface* shader=this->unlit_color;

	if(!shader)
		return;

	shader->Use();

	////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());

	shader->SetSelectionColor(this->picking,0,vec2(this->tabContainer->mousex/this->tabContainer->width,this->tabContainer->mousey/this->tabContainer->height));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glPointSize(psize);

	int ps=shader->GetPositionSlot();
	int uniform_color=shader->GetUniform("color");

	if(uniform_color>=0)
	{glUniform3fv(uniform_color,1,col);glCheckError();}

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER,3*sizeof(float),point.v,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(ps);glCheckError();
	glVertexAttribPointer(ps, 3, GL_FLOAT, GL_FALSE, 0,0);glCheckError();
		
	glDrawArrays(GL_POINTS,0,1);glCheckError();

	glDisableVertexAttribArray(ps);glCheckError();

	glPointSize(1.0f);
	glDisable(GL_PROGRAM_POINT_SIZE);
	glDisable(GL_DEPTH_TEST);
}

void OpenGLRenderer::draw(vec4 rect)
{
	ShaderInterface* shader=this->unlit_color;

	int position_slot=-1;
	int modelview_slot=-1;

	if(shader)
	{
		shader->Use();
		
		////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());
		int mdl=shader->GetModelviewSlot();
		int view=shader->GetProjectionSlot();
		int ptrclr=shader->GetUniform("ptrclr");

		if(ptrclr>=0)
			glUniform4f(ptrclr,0,0,0,0);

	}
	else
		return;

	if(position_slot<0)
		return;

	float dx=rect[2]-rect[0];
	float dy=rect[3]-rect[1];

	float data[]=
	{
		rect[0],	rect[1],		0,
		rect[0],	rect[1]+dy,		0,
		rect[0]+dx,	rect[1]+dy,		0,
		rect[0]+dx,	rect[1],		0,
		rect[0],	rect[1],		0
	};

	glVertexAttribPointer(position_slot, 3, GL_FLOAT, GL_FALSE, 0,data);
	glEnableVertexAttribArray(position_slot);
	glDrawArrays(GL_LINE_STRIP, 0, 5);
}


void OpenGLRenderer::draw(mat4 mtx,float size,vec3 color)
{
	ShaderInterface* shader=this->unlit_color;

	if(!shader)
		return;

	shader->Use();
	////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());

	int mdl=shader->GetModelviewSlot();
	int view=shader->GetProjectionSlot();
	int ptrclr=shader->GetUniform("ptrclr");

	if(ptrclr>=0)
		glUniform4f(ptrclr,0,0,0,0);

	/*MatrixStack::Push();
	MatrixStack::Multiply(mtx);*/

	vec3 axes[6];

	vec3 zero=mtx.transform(0,0,0);

	axes[0]=axes[2]=axes[4]=zero;

	mtx.axes(axes[1],axes[3],axes[5]);

	glEnable(GL_DEPTH_TEST);

	int pos=shader->GetPositionSlot();
	int col=shader->GetUniform("color");

	glUniform3fv(col,1,color);

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER,18*sizeof(float),axes,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(pos);glCheckError();

	glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();

	glDrawArrays(GL_LINES,0,6);glCheckError();
	//glDrawElements(GL_TRIANGLES,3,GL_UNSIGNED_INT,)

	glDisableVertexAttribArray(pos);


	glDisable(GL_DEPTH_TEST);

	

	//MatrixStack::Pop();
}

void OpenGLRenderer::draw(AABB aabb,vec3 color)
{
	ShaderInterface* shader=this->unlit_color;

	if(!shader)
		return;

	shader->Use();

	////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());

	shader->SetSelectionColor(false,0,vec2(this->tabContainer->mousex/this->tabContainer->width,this->tabContainer->mousey/this->tabContainer->height));

	vec3 &a=aabb.a;
	vec3 &b=aabb.b;

	float dx=b.x-a.x;
	float dy=b.y-a.y;
	float dz=b.z-a.z;

	if(dx<0.00000001f && dy<0.00000001f && dz<0.00000001f)
		return;

	float parallelepiped[72]=
	{
		//lower quad
		a.x,a.y,a.z,		a.x+dx,a.y,a.z,
		a.x+dx,a.y,a.z,		a.x+dx,a.y+dy,a.z, 
		a.x+dx,a.y+dy,a.z,	a.x,a.y+dy,a.z,
		a.x,a.y+dy,a.z,     a.x,a.y,a.z,

		//upper quad
		a.x,a.y,a.z+dz,			a.x+dx,a.y,a.z+dz,
		a.x+dx,a.y,a.z+dz,		a.x+dx,a.y+dy,a.z+dz, 
		a.x+dx,a.y+dy,a.z+dz,	a.x,a.y+dy,a.z+dz,
		a.x,a.y+dy,a.z+dz,		a.x,a.y,a.z+dz,

		//staffs
		a.x,a.y,a.z,		a.x,a.y,a.z+dz,
		a.x+dx,a.y,a.z,		a.x+dx,a.y,a.z+dz,
		a.x+dx,a.y+dy,a.z,	a.x+dx,a.y+dy,a.z+dz,
		a.x,a.y+dy,a.z,   	a.x,a.y+dy,a.z+dz
	};

	glEnable(GL_DEPTH_TEST);

	int pos=shader->GetPositionSlot();
	int col=shader->GetUniform("color");

	glUniform3fv(col,1,color);

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER,72*sizeof(float),parallelepiped,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(pos);glCheckError();

	glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();

	glDrawArrays(GL_LINES,0,24);glCheckError();
	//glDrawElements(GL_TRIANGLES,3,GL_UNSIGNED_INT,)

	glDisableVertexAttribArray(pos);

	glBindBuffer(GL_ARRAY_BUFFER,0);

	glDisable(GL_DEPTH_TEST);
}

void OpenGLRenderer::draw(vec3 a,vec3 b,vec3 color)
{
	ShaderInterface* shader=this->unlit_color;

	if(!shader)
		return;

	float line[]=
	{
		a[0],a[1],a[2],
		b[0],b[1],b[2], 
	};
	
	shader->Use();

	shader->SetMatrices(MatrixStack::GetViewMatrix()*MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelMatrix());

	shader->SetSelectionColor(false,0,vec2(this->tabContainer->mousex/this->tabContainer->width,this->tabContainer->mousey/this->tabContainer->height));
	
	glEnable(GL_DEPTH_TEST);

	int pos=shader->GetPositionSlot();
	int col=shader->GetUniform("color");
	
	glUniform3fv(col,1,color);

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER,6*sizeof(float),line,GL_DYNAMIC_DRAW);
	
	glEnableVertexAttribArray(pos);glCheckError();

	glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();

	glDrawArrays(GL_LINES,0,2);glCheckError();
	//glDrawElements(GL_TRIANGLES,3,GL_UNSIGNED_INT,)

	glDisableVertexAttribArray(pos);


	glDisable(GL_DEPTH_TEST);

}

void OpenGLRenderer::draw(char* text,float x,float y,float width,float height,float sizex,float sizey,float* color4)
{
	ShaderInterface* shader=0;//line_color_shader

	if(!shader || !text)
		return;

	shader->Use();

	////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());

	/*Font* font=FontManager::Instance()->Head()->Data();

	this->draw(font,text,x,y,width,height,sizex,sizey,color4);*/
}


/*
void OpenGLFixedRenderer::draw(Window* w)
{
	line_shader->Use();

	int position_slot=line_shader->GetPositionSlot();

	vec2& pos=w->window_pos;
	vec2& dim=w->window_dim;

	float data[]=
	{
		pos[0]-dim[0]/2,	pos[1]+dim[1]/2,		0,//low-left
		pos[0]+dim[0]/2,	pos[1]+dim[1]/2,		0,//low-right
		pos[0]+dim[0]/2,	pos[1]-dim[1]/2,		0,//up-right
		pos[0]-dim[0]/2,	pos[1]-dim[1]/2,		0,//up-left
		pos[0]-dim[0]/2,	pos[1]+dim[1]/2,		0,//low-left
	};

	glVertexAttribPointer(position_slot, 3, GL_FLOAT, GL_FALSE, 0,data);
	glEnableVertexAttribArray(position_slot);
	glDrawArrays(GL_LINE_STRIP, 0, 5);
}*/

/*
void OpenGLFixedRenderer::draw(WindowManager* windowManager)
{
	this->line_shader->Use();

	mat4 mat;
	mat.ortho(-0.5f,0.5f,-0.5f,0.5f,0,10);
	this->line_shader->SetMatrix4f(this->line_shader->GetProjectionSlot(),mat);


	glDisable(GL_CULL_FACE);

	float width=(float)KernelServer::Instance()->GetApp()->GetWidth();
	float height=(float)KernelServer::Instance()->GetApp()->GetHeight();

	for(Node<Window*> *node=windowManager->Head();node;node=node->Next())
		this->draw(node->Data());

	glEnable(GL_CULL_FACE);
}*/

/*
void OpenGLFixedRenderer::draw(EntityManager& entityManager)
{
	glDisable(GL_CULL_FACE);

	//entityManager.draw();

	glEnable(GL_CULL_FACE);
}
*/



/*
void OpenGLFixedRenderer::draw(Font* font,char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4) 
{
	//https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_01

	if(!font || !phrase)
		return;

	ShaderInterface* shader=font_shader;

	if(!shader)
		return;

	shader->Use();

	GLuint tid;

	glEnable(GL_BLEND);glCheckError();
	glBlendFunc(GL_CONSTANT_COLOR, GL_ONE_MINUS_SRC_COLOR);

	if(color4)
		glBlendColor(color4[0],color4[1],color4[2],color4[3]);
	else
		glBlendColor(1,1,1,1);

	glActiveTexture(GL_TEXTURE0);glCheckError();
	glGenTextures(1,(GLuint*)&tid);glCheckError();
	glBindTexture(GL_TEXTURE_2D,tid);glCheckError();

	float sx=2.0f/width;
	float sy=2.0f/height;

	char* p=phrase;

	while(*p)
	{
		int err=font->loadchar(*p,sizex,sizey,width,height);

		if(!err)
		{
			unsigned char*	_buf=(unsigned char*)font->GetBuffer();
			int				_width=font->GetWidth();
			int				_rows=font->GetRows();
			float			_left=(float)font->GetLeft();
			float			_top=(float)font->GetTop();
			int				_advx=font->GetAdvanceX();
			int				_advy=font->GetAdvanceY();

			float			x2 = x + _left * sx;
			float			y2 = -y - _top * sy;
			float			w = _width * sx;
			float			h = _rows * sy;

			float box[] = {x2,-y2,0,1,x2 + w,-y2,0,1,x2,-y2-h,0,1,x2+w,-y2-h,0,1};
			float uv[] = {0,0,1,0,0,1,1,1};

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);glCheckError();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);glCheckError();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);glCheckError();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);glCheckError();

			glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,_width,_rows,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,_buf);glCheckError();

			glUniform1i(shader->GetTextureSlot(), 0);glCheckError();

			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			glEnableVertexAttribArray(shader->GetPositionSlot());glCheckError();
			glEnableVertexAttribArray(shader->GetTexcoordSlot());glCheckError();
			glVertexAttribPointer(shader->GetPositionSlot(), 4, GL_FLOAT, GL_FALSE,0,box);glCheckError();
			glVertexAttribPointer(shader->GetTexcoordSlot(), 2, GL_FLOAT, GL_FALSE,0,uv);glCheckError();
			glDrawArrays(GL_TRIANGLE_STRIP,0,4);glCheckError();
			glDisableVertexAttribArray(shader->GetPositionSlot());glCheckError();
			glDisableVertexAttribArray(shader->GetTexcoordSlot());glCheckError();

			x += (_advx >> 6) * sx;
			y += (_advy >> 6) * sy;
		}

		p++;
	}

	glDeleteTextures(1,(GLuint*)&tid);glCheckError();

	glDisable(GL_BLEND);glCheckError();
}*/


void OpenGLRenderer::draw(Texture* _t)
{
	return;
	ShaderInterface* shader=0;//unlit_texture

	Texture* texture=(Texture*)_t;
	//TextureFile* texture=(TextureFile*)_t;
	

	if(!shader || !texture->GetBuffer())
		return;

	shader->Use();

	const GLfloat rect_data[] = 
	{
		-1.0f, -1.0f,
		1.0f, -1.0f,
		-1.0f,  1.0f,
		1.0f,  1.0f
	};

	const GLfloat uv_data[] = 
	{
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f,  1.0f,
		0.0f,  0.0f,
	};

	int width=texture->GetWidth();
	int height=texture->GetHeight();
	void* buf=texture->GetBuffer();
	int  size=texture->GetSize();

	if(width<100)
		return;

	GLuint tid;
	glGenTextures(1,&tid);glCheckError();

	glActiveTexture(GL_TEXTURE0);glCheckError();
	glBindTexture(GL_TEXTURE_2D,tid);glCheckError();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glCheckError();

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,width,height, 0, GL_RGBA, GL_UNSIGNED_BYTE,buf);glCheckError();

	mat4 mat;

	int projection=shader->GetProjectionSlot();
	int modelview=shader->GetModelviewSlot();
	int textureslot=shader->GetTextureSlot();
	int position=shader->GetPositionSlot();
	int texcoord=shader->GetTexcoordSlot();

	mat.ortho(-3,3,-3,3,0,1000);
	glUniformMatrix4fv(projection,1,0,mat);glCheckError();
	mat.identity();
	glUniformMatrix4fv(modelview,1,0,mat);glCheckError();

	glUniform1i(textureslot, 0);glCheckError();
	glEnableVertexAttribArray(position);glCheckError();
	glEnableVertexAttribArray(texcoord);glCheckError();
	glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE,0,rect_data);glCheckError();
	glVertexAttribPointer(texcoord, 2, GL_FLOAT, GL_FALSE,0,uv_data);glCheckError();
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);glCheckError();

	glDisableVertexAttribArray(position);glCheckError();
	glDisableVertexAttribArray(texcoord);glCheckError();

	glDeleteTextures(1,&tid);glCheckError();
}

void OpenGLRenderer::draw(Mesh* mesh,std::vector<GLuint>& textureIndices,int texture_slot,int texcoord_slot)
{
	for(int i=0;i<(int)mesh->mesh_materials.size();i++)
	{
		for(int j=0;j<(int)mesh->mesh_materials[i]->textures.size() && !textureIndices.size();j++)
		{
			Texture* texture=mesh->mesh_materials[i]->textures[j];
			
			int		texture_width=texture->GetWidth();
			int		texture_height=texture->GetHeight();
			void	*texture_buffer=texture->GetBuffer();

			if(texture_buffer)
			{
				GLuint tid;
				glGenTextures(1,&tid);glCheckError();
				textureIndices.push_back(tid);

				glActiveTexture(GL_TEXTURE0);glCheckError();
				glBindTexture(GL_TEXTURE_2D,tid);glCheckError();

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);glCheckError();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);glCheckError();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);glCheckError();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glCheckError();

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,texture_width,texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE,texture_buffer);glCheckError();

				glBindBuffer(GL_ARRAY_BUFFER,textureBufferObject);
				glBufferData(GL_ARRAY_BUFFER,mesh->mesh_ntexcoord*2*sizeof(float),mesh->mesh_texcoord,GL_DYNAMIC_DRAW);

				glUniform1i(texture_slot, 0);glCheckError();
				glEnableVertexAttribArray(texcoord_slot);glCheckError();
				glVertexAttribPointer(texcoord_slot,2,GL_FLOAT,GL_FALSE,0,0);glCheckError();

				glBindBuffer(GL_ARRAY_BUFFER,0);
			}
		}
	}
}

void OpenGLRenderer::draw(Mesh* mesh)
{
	drawUnlitTextured(mesh);
}

void OpenGLRenderer::drawUnlitTextured(Mesh* mesh)
{
	ShaderInterface* shader = mesh->mesh_materials.size() ? this->unlit_texture : this->unlit_color;

	if(!shader || !mesh)
		return;

	vec3 lightpos(0,200,-100);

	if(shader==this->shaded_texture)
		this->draw(lightpos,5);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glFrontFace(mesh->mesh_isCCW ? GL_CCW : GL_CW);
	//glFrontFace(GL_CCW);

	shader->Use();

	shader->SetMatrices(MatrixStack::GetViewMatrix()*MatrixStack::GetProjectionMatrix(),mesh->entity->world);


	shader->SetSelectionColor(this->picking,mesh->entity,vec2(this->tabContainer->mousex/this->tabContainer->width,this->tabContainer->mousey/this->tabContainer->height));
	
	int position_slot = shader->GetPositionSlot();
	int texcoord_slot = shader->GetTexcoordSlot();
	int texture_slot = shader->GetTextureSlot();
	int normal_slot = shader->GetNormalSlot();

	std::vector<unsigned int> textureIndices;

	if(mesh->mesh_materials.size())
	{
		draw(mesh,textureIndices,texture_slot,texcoord_slot);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER,mesh->mesh_ncontrolpoints*3*sizeof(float),mesh->mesh_controlpoints,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(position_slot);glCheckError();
	if(normal_slot>=0)glEnableVertexAttribArray(normal_slot);glCheckError();
	glVertexAttribPointer(position_slot,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();
	if(normal_slot>=0)glVertexAttribPointer(normal_slot,3,GL_FLOAT,GL_FALSE,0,mesh->mesh_normals);glCheckError();

	glBindBuffer(GL_ARRAY_BUFFER,0);

	glDrawArrays(GL_TRIANGLES,0,mesh->mesh_npolygons*3);glCheckError();
	//glDrawElements(GL_TRIANGLES,mesh->mesh_nvertexindices*3,GL_UNSIGNED_INT,mesh->mesh_vertexindices);glCheckError();


	glDisableVertexAttribArray(position_slot);glCheckError();
	if(normal_slot>=0)glDisableVertexAttribArray(normal_slot);glCheckError();

	for(int i=textureIndices.size();i>0;i--)
	{
		glDeleteTextures(1,&textureIndices[i-1]);glCheckError();
		glDisableVertexAttribArray(texcoord_slot);glCheckError();
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}


void OpenGLRenderer::draw(Skin *skin)
{
	ShaderInterface* shader = skin->mesh_materials.size() ? this->unlit_texture : this->unlit_color;

	if(!skin || !skin->skin_vertexcache || !shader)
	{
		this->drawUnlitTextured(skin);
		return;
	}

	vec3 lightpos(0,200,-100);

	if(shader==this->shaded_texture)
		this->draw(lightpos,5);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glFrontFace(skin->mesh_isCCW ? GL_CCW : GL_CW);
	//glFrontFace(GL_CW);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	shader->Use();

	shader->SetMatrices(MatrixStack::GetViewMatrix()*MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelMatrix());


	shader->SetSelectionColor(this->picking,skin->entity,vec2(this->tabContainer->mousex/this->tabContainer->width,this->tabContainer->mousey/this->tabContainer->height));

	int position_slot = shader->GetPositionSlot();
	int texcoord_slot = shader->GetTexcoordSlot();
	int texture_slot = shader->GetTextureSlot();
	int lightpos_slot = shader->GetLightposSlot();
	int lightdiff_slot = shader->GetLightdiffSlot();
	int lightamb_slot = shader->GetLightambSlot();
	int normal_slot = shader->GetNormalSlot();
	int color_slot = shader->GetColorSlot();
	
	std::vector<GLuint> textureIndices;

	draw(skin,textureIndices,texture_slot,texcoord_slot);

	int uniformTextured=shader->GetUniform("textured");

	glUniform1f(uniformTextured,(GLfloat)textureIndices.size());glCheckError();

	if(lightdiff_slot>=0)glEnableVertexAttribArray(lightdiff_slot);glCheckError();
	if(lightamb_slot>=0)glEnableVertexAttribArray(lightamb_slot);glCheckError();
	if(lightpos_slot>=0)glEnableVertexAttribArray(lightpos_slot);glCheckError();

	float v[3]={255,255,255};

	if(lightdiff_slot>=0)glUniform3f(lightdiff_slot,v[0],v[1],v[2]);glCheckError();
	if(lightamb_slot>=0)glUniform3f(lightamb_slot,v[0]+128,v[1]+128,v[2]+255);glCheckError();
	if(lightpos_slot>=0)glUniform3fv(lightpos_slot,1,lightpos);glCheckError();
	
	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER,skin->mesh_ncontrolpoints*3*sizeof(float),skin->skin_vertexcache,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(position_slot);glCheckError();
	if(normal_slot>=0)glEnableVertexAttribArray(normal_slot);glCheckError();
	glVertexAttribPointer(position_slot,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();
	if(normal_slot>=0)glVertexAttribPointer(normal_slot,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();

	glDrawArrays(GL_TRIANGLES,0,skin->mesh_npolygons*3);glCheckError();
	//glDrawElements(GL_TRIANGLES,skin->mesh_ntriangleindices*3,GL_UNSIGNED_INT,skin->mesh_triangleindices);glCheckError();

	if(lightdiff_slot>=0)glDisableVertexAttribArray(position_slot);glCheckError();
	if(lightdiff_slot>=0)glDisableVertexAttribArray(normal_slot);glCheckError();

	if(lightdiff_slot>=0)glDisableVertexAttribArray(lightdiff_slot);glCheckError();
	if(lightamb_slot>=0)glDisableVertexAttribArray(lightamb_slot);glCheckError();
	if(lightpos_slot>=0)glDisableVertexAttribArray(lightpos_slot);glCheckError();
	
	for(int i=textureIndices.size();i>0;i--)
	{
		glDeleteTextures(1,&textureIndices[i-1]);glCheckError();
		glDisableVertexAttribArray(texcoord_slot);glCheckError();
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}

void OpenGLRenderer::draw(Camera*)
{

}

void OpenGLRenderer::draw(Bone* bone)
{
	ShaderInterface* shader=this->unlit_color;

	if(!shader)
		return;

	vec3 &a=bone->entity->parent->world.position();
	vec3 &b=bone->entity->world.position();

	float line[]=
	{
		a[0],a[1],a[2],
		b[0],b[1],b[2], 
	};

	shader->Use();

	shader->SetMatrices((MatrixStack::GetViewMatrix()*MatrixStack::GetProjectionMatrix()).inverse().traspose(),bone->root->entity->world);

	shader->SetSelectionColor(this->picking,bone->entity,vec2(this->tabContainer->mousex/this->tabContainer->width,this->tabContainer->mousey/this->tabContainer->height));

	glEnable(GL_DEPTH_TEST);

	int pos=shader->GetPositionSlot();
	int col=shader->GetUniform("color");

	glUniform3fv(col,1,bone->color);

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER,6*sizeof(float),line,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(pos);glCheckError();

	glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();

	glDrawArrays(GL_LINES,0,2);glCheckError();
	//glDrawElements(GL_TRIANGLES,3,GL_UNSIGNED_INT,)

	glDisableVertexAttribArray(pos);

	glDisable(GL_DEPTH_TEST);
}

float signof(float num){return (num>0 ? 1.0f : (num<0 ? -1.0f : 0.0f));}




void OpenGLRenderer::Render(GuiViewport* viewport,bool force)
{
	viewport->OnPaint(tabContainer);
}


void OpenGLRenderer::Render()
{
	for(std::list<GuiViewport*>::iterator vport=this->viewports.begin();vport!=this->viewports.end();vport++)
		this->Render(*vport);
	
}


/*float ratio=this->width/this->height;
	float calcRatio=ratio*tan(45*PI/180.0f);

	draw(vec3(0,1,-1),15);
	draw(vec3(calcRatio*this->RendererViewportInterface_farPlane,0,-this->RendererViewportInterface_farPlane+1),15);*/



/*
class Ray 
{ 
public: 
	Ray(const vec3 &orig, const vec3 &dir) : orig(orig), dir(dir) 
	{ 
		invdir = 1 / dir; 
		sign[0] = (invdir.x < 0); 
		sign[1] = (invdir.y < 0); 
		sign[2] = (invdir.z < 0); 
	} 
	vec3 orig, dir;       // ray orig and dir 
	vec3 invdir; 
	int sign[3]; 
}; 

bool intersect(AABB &bounds,const Ray &r) const 
{ 
	float tmin, tmax, tymin, tymax, tzmin, tzmax; 

	tmin = (bounds[r.sign[0]].x - r.orig.x) * r.invdir.x; 
	tmax = (bounds[1-r.sign[0]].x - r.orig.x) * r.invdir.x; 
	tymin = (bounds[r.sign[1]].y - r.orig.y) * r.invdir.y; 
	tymax = (bounds[1-r.sign[1]].y - r.orig.y) * r.invdir.y; 

	if ((tmin > tymax) || (tymin > tmax)) 
		return false; 
	if (tymin > tmin) 
		tmin = tymin; 
	if (tymax < tmax) 
		tmax = tymax; 

	tzmin = (bounds[r.sign[2]].z - r.orig.z) * r.invdir.z; 
	tzmax = (bounds[1-r.sign[2]].z - r.orig.z) * r.invdir.z; 

	if ((tmin > tzmax) || (tzmin > tmax)) 
		return false; 
	if (tzmin > tmin) 
		tmin = tzmin; 
	if (tzmax < tmax) 
		tmax = tzmax; 

	return true; 
}
*/




/*
void OpenGLRenderer::OnGuiLMouseDown()
{
	/ *float ratio=this->width/this->height;
	float calcRatioX=ratio*tan(45*PI/180.0f);
	float calcRatioY=-1;

	/ *vec3 ndc(2.0f * tabContainer->mousex / this->width -1.0f,-2.0f * (tabContainer->mousey-TabContainer::CONTAINER_HEIGHT) / this->height + 1.0f,0);
	mat4 viewInv=MatrixStack::view.inverse();
	rayStart=viewInv.transform(ndc.x*calcRatioX,ndc.y*calcRatioY,-1);
	rayEnd=viewInv.transform(ndc.x*calcRatioX*this->RendererViewportInterface_farPlane,ndc.y*calcRatioY*this->RendererViewportInterface_farPlane,-this->RendererViewportInterface_farPlane+1);* /
* /

}*/
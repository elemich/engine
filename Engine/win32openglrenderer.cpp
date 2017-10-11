#include "win32.h"

#pragma message (LOCATION " processNode should go to common part (maybe entities.h) of the project cause there is no os-related call within")
#pragma message (LOCATION " multiple opengl context needs glew32mx.lib")
#pragma message (LOCATION " TODO: move OpenGL bitmap render in the RendererViewportInterface cause is for the gui only")

GLuint OpenGLRenderer::vertexArrayObject=0;
GLuint OpenGLRenderer::vertexBufferObject=0;
GLuint OpenGLRenderer::textureBufferObject=0;
GLuint OpenGLRenderer::indicesBufferObject=0;
GLuint OpenGLRenderer::frameBuffer=0;
GLuint OpenGLRenderer::textureColorbuffer=0;
GLuint OpenGLRenderer::textureRenderbuffer=0;
GLuint OpenGLRenderer::pixelBuffer=0;
GLuint OpenGLRenderer::renderBufferColor=0;
GLuint OpenGLRenderer::renderBufferDepth=0;

PFNWGLCHOOSEPIXELFORMATEXTPROC wglChoosePixelFormatARB = 0;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 0;

#ifndef GL_GLEXT_PROTOTYPES

PFNGLATTACHSHADERPROC glAttachShader = 0;
PFNGLBINDBUFFERPROC glBindBuffer = 0;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = 0;
PFNGLBUFFERDATAPROC glBufferData = 0;
PFNGLCOMPILESHADERPROC glCompileShader = 0;
PFNGLCREATEPROGRAMPROC glCreateProgram = 0;
PFNGLCREATESHADERPROC glCreateShader = 0;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = 0;
PFNGLDELETEPROGRAMPROC glDeleteProgram = 0;
PFNGLDELETESHADERPROC glDeleteShader = 0;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = 0;
PFNGLDETACHSHADERPROC glDetachShader = 0;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = 0;
PFNGLENABLEVERTEXARRAYATTRIBPROC glEnableVertexArrayAttrib = 0;
PFNGLGENBUFFERSPROC glGenBuffers = 0;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = 0;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation = 0;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = 0;
PFNGLGETPROGRAMIVPROC glGetProgramiv = 0;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = 0;
PFNGLGETSHADERIVPROC glGetShaderiv = 0;
PFNGLLINKPROGRAMPROC glLinkProgram = 0;
PFNGLSHADERSOURCEPROC glShaderSource = 0;
PFNGLUSEPROGRAMPROC glUseProgram = 0;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = 0;
PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation = 0;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = 0;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = 0;
PFNGLACTIVETEXTUREPROC glActiveTexture = 0;
PFNGLUNIFORM1IPROC glUniform1i = 0;
PFNGLUNIFORM1FPROC glUniform1f = 0;
PFNGLUNIFORM3FPROC glUniform3f = 0;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap = 0;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = 0;
PFNGLUNIFORM3FVPROC glUniform3fv = 0;
PFNGLUNIFORM4FVPROC glUniform4fv = 0;
PFNGLTEXBUFFERPROC glTexBuffer = 0;
PFNGLTEXTUREBUFFERPROC glTextureBuffer = 0;
PFNGLBUFFERSUBDATAPROC glBufferSubData = 0;
PFNGLVIEWPORTINDEXEDFPROC glViewportIndexedf = 0;
PFNGLADDSWAPHINTRECTWINPROC glAddSwapHintRectWIN = 0;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = 0;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = 0;
PFNGLNAMEDRENDERBUFFERSTORAGEPROC glNamedRenderbufferStorage = 0;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage = 0;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers = 0;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer = 0;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer = 0;
PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer = 0;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = 0;
PFNGLDRAWBUFFERSPROC glDrawBuffers = 0;
PFNGLBLITNAMEDFRAMEBUFFERPROC glBlitNamedFramebuffer = 0;
PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture = 0;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = 0;

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

		int pixelFormat = ChoosePixelFormat(hdc,&pfd);

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
		if(!glBufferSubData)glViewportIndexedf = (PFNGLVIEWPORTINDEXEDFPROC) wglGetProcAddress("glViewportIndexedf");
		if(!glAddSwapHintRectWIN)glAddSwapHintRectWIN = (PFNGLADDSWAPHINTRECTWINPROC) wglGetProcAddress("glAddSwapHintRectWIN");
		if(!glGenFramebuffers)glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) wglGetProcAddress("glGenFramebuffers");
		if(!glBindFramebuffer)glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) wglGetProcAddress("glBindFramebuffer");
		if(!glNamedRenderbufferStorage)glNamedRenderbufferStorage = (PFNGLNAMEDRENDERBUFFERSTORAGEPROC) wglGetProcAddress("glNamedRenderbufferStorage");
		if(!glRenderbufferStorage)glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) wglGetProcAddress("glRenderbufferStorage");
		if(!glGenRenderbuffers)glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) wglGetProcAddress("glGenRenderbuffers");
		if(!glBindRenderbuffer)glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) wglGetProcAddress("glBindRenderbuffer");
		if(!glFramebufferRenderbuffer)glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) wglGetProcAddress("glFramebufferRenderbuffer");
		if(!glBlitFramebuffer)glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC) wglGetProcAddress("glBlitFramebuffer");
		if(!glCheckFramebufferStatus)glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) wglGetProcAddress("glCheckFramebufferStatus");
		if(!glDrawBuffers)glDrawBuffers = (PFNGLDRAWBUFFERSPROC) wglGetProcAddress("glDrawBuffers");
		if(!glBlitNamedFramebuffer) glBlitNamedFramebuffer = (PFNGLBLITNAMEDFRAMEBUFFERPROC) wglGetProcAddress("glBlitNamedFramebuffer");
		if(!glFramebufferTexture)glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC)wglGetProcAddress("glFramebufferTexture");
		if(!glFramebufferTexture2D)glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) wglGetProcAddress("glFramebufferTexture2D");
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

	int pixelFormat;
	UINT numFormats;

	if(!wglChoosePixelFormatARB(hdc, pixelFormatAttribList, NULL, 1, &pixelFormat, &numFormats))
		__debugbreak();

	

	const int versionAttribList[] = 
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB,4,
		WGL_CONTEXT_MINOR_VERSION_ARB,0, 
		0,        //End
	};

	if(!(hglrc = wglCreateContextAttribsARB(hdc, 0, versionAttribList)))
		__debugbreak();

	if(hglrc)
		printf("HGLRC: %p, HDC: %p\n",hglrc,hdc);
	
	if(!wglMakeCurrent(hdc,hglrc))
		__debugbreak();

	//wglMakeCurrent(TClassPool<OpenGLRenderer>::pool[0]->hdc, TClassPool<OpenGLRenderer>::pool[0]->hglrc);

	if(!vertexArrayObject)
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
	//printf("Status: Using extension %s\n", glGetString(GL_EXTENSIONS));

	

	//SetWindowLongPtr(hwnd,GWL_USERDATA,(LONG_PTR)this);

	OpenGLShader::Create("unlit",unlit_vert,unlit_frag);
	OpenGLShader::Create("unlit_color",unlit_color_vert,unlit_color_frag);
	OpenGLShader::Create("unlit_texture",unlit_texture_vs,unlit_texture_fs);
	OpenGLShader::Create("font",font_pixsh,font_frgsh);
	OpenGLShader::Create("shaded_texture",texture_vertex_shaded_vert,texture_vertex_shaded_frag);

	
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

void OpenGLRenderer::draw(vec3 point,float psize,vec3 col)
{
	ShaderInterface* shader=ShaderInterface::shadersPool.Find("unlit_color");

	if(!shader)
		return;

	shader->Use();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glPointSize(psize);

	int ps=shader->GetPositionSlot();glCheckError();
	int uniform_color=shader->GetUniform("color");glCheckError();

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
	ShaderInterface* shader=ShaderInterface::shadersPool.Find("unlit_color");

	int position_slot=-1;
	int modelview_slot=-1;

	if(shader)
	{
		shader->Use();
		position_slot=shader->GetPositionSlot();
		modelview_slot=shader->GetModelviewSlot();
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
	ShaderInterface* shader=ShaderInterface::shadersPool.Find("unlit_color");

	if(!shader)
		return;

	shader->Use();

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
	ShaderInterface* shader=ShaderInterface::shadersPool.Find("unlit_color");

	if(!shader)
		return;

	shader->Use();

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
	ShaderInterface* shader=ShaderInterface::shadersPool.Find("unlit_color");

	if(!shader)
		return;

	float line[]=
	{
		a[0],a[1],a[2],
		b[0],b[1],b[2], 
	};
	
	shader->Use();

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
	ShaderInterface* shader=ShaderInterface::shadersPool.Find("unlit_texture");

	if(!shader || !mesh)
		return;

	vec3 lightpos(0,200,-100);
	this->draw(lightpos,5);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glFrontFace(mesh->mesh_isCCW ? GL_CCW : GL_CW);
	//glFrontFace(GL_CCW);

	shader->Use();

	int position_slot = shader->GetPositionSlot();
	int model_slot = shader->GetModelviewSlot();
	int projection_slot = shader->GetProjectionSlot();
	int texcoord_slot = shader->GetTexcoordSlot();
	int texture_slot = shader->GetTextureSlot();
	int normal_slot = shader->GetNormalSlot();

	std::vector<unsigned int> textureIndices;

	draw(mesh,textureIndices,texture_slot,texcoord_slot);

	if(!textureIndices.size())
	{
		ShaderInterface* shader=ShaderInterface::shadersPool.Find("unlit_color");

		if(shader)
			shader->Use();

		position_slot = shader->GetPositionSlot();
		model_slot = shader->GetModelviewSlot();
		projection_slot = shader->GetProjectionSlot();
		texcoord_slot = shader->GetTexcoordSlot();
		texture_slot = shader->GetTextureSlot();
		normal_slot = shader->GetNormalSlot();

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	
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
	/*this->draw((Mesh*)skin);
	return;*/

	ShaderInterface* shader=ShaderInterface::shadersPool.Find("unlit_texture");

	if(!skin || !skin->skin_vertexcache || !shader)
	{
		this->drawUnlitTextured(skin);
		return;
	}

	vec3 lightpos(0,200,-100);
	this->draw(lightpos,10.0f);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glFrontFace(skin->mesh_isCCW ? GL_CCW : GL_CW);
	//glFrontFace(GL_CW);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	shader->Use();

	int position_slot = shader->GetPositionSlot();
	int model_slot = shader->GetModelviewSlot();
	int projection_slot = shader->GetProjectionSlot();
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

	if(!textureIndices.size())
	{
		ShaderInterface* shader=ShaderInterface::shadersPool.Find("unlit_color");

		if(shader)
			shader->Use();


		position_slot = shader->GetPositionSlot();
		model_slot = shader->GetModelviewSlot();
		projection_slot = shader->GetProjectionSlot();
		texcoord_slot = shader->GetTexcoordSlot();
		texture_slot = shader->GetTextureSlot();
		lightpos_slot = shader->GetLightposSlot();
		lightdiff_slot = shader->GetLightdiffSlot();
		lightamb_slot = shader->GetLightambSlot();
		normal_slot = shader->GetNormalSlot();
		color_slot = shader->GetColorSlot();
	}

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
	if(!bone)
		return;

	Bone* ec=bone->entity->entity_parent->findComponent<Bone>();
	
	if(ec)
	{
		Bone* bone=(Bone*)ec;

		vec3 b2p=bone->entity->entity_parent->entity_world.position();
		vec3 b1p=bone->entity->entity_world.position();
		this->draw(b1p,b2p,bone->bone_color);
	}
}

float signof(float num){return (num>0 ? 1.0f : (num<0 ? -1.0f : 0.0f));}



void OpenGLRenderer::Render(GuiViewport* viewport)
{
	if(!this->hglrc || !viewport || !viewport->surface)
		return;

	Entity::pool.empty() ? Entity::pool.empty() : Entity::pool.front()->update();

	vec4 &rectangle=viewport->rect;

	glViewport((int)0,(int)0,(int)rectangle.z,(int)rectangle.w);glCheckError();
	glScissor((int)0,(int)0,(int)rectangle.z,(int)rectangle.w);glCheckError();

	glEnable(GL_DEPTH_TEST);

	glClearColor(0.43f,0.43f,0.43f,0.0f);glCheckError();
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glCheckError();

	MatrixStack::Push(MatrixStack::PROJECTION,viewport->projection);
	MatrixStack::Multiply(MatrixStack::PROJECTION,viewport->view);
	MatrixStack::Push(MatrixStack::MODELVIEW,viewport->model);

	draw(vec3(0,0,0),vec3(1000,0,0),vec3(1,0,0));
	draw(vec3(0,0,0),vec3(0,1000,0),vec3(0,1,0));
	draw(vec3(0,0,0),vec3(0,0,1000),vec3(0,0,1));

	Entity::pool.empty() ? Entity::pool.empty() : Entity::pool.front()->draw(this);

	MatrixStack::Pop(MatrixStack::MODELVIEW);
	MatrixStack::Pop(MatrixStack::PROJECTION);

	glReadBuffer(GL_BACK);glCheckError();
	glReadPixels((int)0,(int)0,(int)rectangle.z,(int)rectangle.w,GL_BGRA,GL_UNSIGNED_BYTE,viewport->surface->renderBuffer);glCheckError();//@mic should implement pbo for performance

	viewport->surface->renderBitmap->CopyFromMemory(&D2D1::RectU(0,0,(int)rectangle.z,(int)rectangle.w),viewport->surface->renderBuffer,(int)(rectangle.z*4));

	viewport->OnPaint(viewport->surface->tab);
}

void OpenGLRenderer::Render(vec4 rectangle,mat4 _projection,mat4 _view,mat4 _model)
{
	if(!this->hglrc)
		return;

	ID2D1Bitmap* renderBitmap;

	D2D1_BITMAP_PROPERTIES bp=D2D1::BitmapProperties();
	bp.pixelFormat=tabContainer->renderTarget->GetPixelFormat();

	tabContainer->renderTarget->CreateBitmap(D2D1::SizeU((int)rectangle.z,(int)rectangle.w),bp,&renderBitmap);

	if(!renderBitmap)
		__debugbreak();

	unsigned char* renderBuffer=new unsigned char[(int)(rectangle.z*rectangle.w*4)];

	glViewport((int)0,(int)0,(int)rectangle.z,(int)rectangle.w);glCheckError();
	glScissor((int)0,(int)0,(int)rectangle.z,(int)rectangle.w);glCheckError();

	glEnable(GL_DEPTH_TEST);
	
	glClearColor(0.43f,0.43f,0.43f,0.0f);glCheckError();
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glCheckError();

	MatrixStack::Push(MatrixStack::PROJECTION,_projection);
	MatrixStack::Multiply(MatrixStack::PROJECTION,_view);
	MatrixStack::Push(MatrixStack::MODELVIEW,_model);

	draw(vec3(0,0,0),vec3(1000,0,0),vec3(1,0,0));
	draw(vec3(0,0,0),vec3(0,1000,0),vec3(0,1,0));
	draw(vec3(0,0,0),vec3(0,0,1000),vec3(0,0,1));

	Entity::pool.empty() ? Entity::pool.empty() : Entity::pool.front()->draw(this);

	MatrixStack::Pop(MatrixStack::MODELVIEW);
	MatrixStack::Pop(MatrixStack::PROJECTION);

	glReadBuffer(GL_BACK);glCheckError();
	glReadPixels((int)0,(int)0,(int)rectangle.z,(int)rectangle.w,GL_BGRA,GL_UNSIGNED_BYTE,renderBuffer);glCheckError();//@mic should implement pbo for performance

	renderBitmap->CopyFromMemory(&D2D1::RectU(0,0,(int)rectangle.z,(int)rectangle.w),renderBuffer,(int)(rectangle.z*4));

	if(!tabContainer->isRender)
		tabContainer->renderTarget->BeginDraw();

	tabContainer->renderTarget->DrawBitmap(renderBitmap,D2D1::RectF(0.0f,(float)TabContainer::CONTAINER_HEIGHT,rectangle.z,rectangle.w+30));

	//tabContainer->renderTarget->DrawRectangle(D2D1::RectF(1.0f,(float)TabContainer::CONTAINER_HEIGHT + 0.5f,(float)width-1.0f,(float)tabContainer->height-1.0f),tabContainer->SetColor(D2D1::ColorF::Yellow));

	//Direct2DGuiBase::DrawText(tabContainer->renderTarget,tabContainer->SetColor(TabContainer::COLOR_TEXT),this->mousePositionString.Buf(),0,tabContainer->height-20,tabContainer->width,tabContainer->height);

	if(!tabContainer->isRender)
		tabContainer->renderTarget->EndDraw();

	SAFERELEASE(renderBitmap);
	SAFEDELETEARRAY(renderBuffer);

}



void OpenGLRenderer::RenderViewports()
{
	for(std::list<GuiViewport*>::iterator it=this->viewports.begin();it!=this->viewports.end();it++)
	{
		if(it==this->viewports.begin())
			this->ChangeContext();

		this->Render(*it);
	}
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
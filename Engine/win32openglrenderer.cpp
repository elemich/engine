#include "win32.h"

#pragma message (LOCATION " processNode should go to common part (maybe entities.h) of the project cause there is no os-related call within")
#pragma message (LOCATION " multiple opengl context needs glew32mx.lib")

GLuint OpenGLRenderer::vertexArrayObject=0;
GLuint OpenGLRenderer::vertexBufferObject=0;
GLuint OpenGLRenderer::textureBufferObject=0;
GLuint OpenGLRenderer::indicesBufferObject=0;

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

#endif

bool GLEW_INITED=false;

#define USE_COMMON_PIXELFORMAT_SYSTEM 1

int processNode(RendererInterface* renderer,std::list<Entity*>& entitiesPool,std::list<Entity*>::iterator& poolNode,int type,float time)
{
	int retValue=0;

#if PROCESS_ENTITIES_RECURSIVELY
	if(poolNode!=entitiesPool.end())
#else
	for(;poolNode!=entitiesPool.end();poolNode++)
#endif
	{
		switch(type)
		{
		case 0:
			retValue+=(*poolNode)->animate(time);
		break;
		case 1:
			(*poolNode)->update();
		break;
		case 2:
			(*poolNode)->draw(renderer);
		break;
		}

#if PROCESS_ENTITIES_RECURSIVELY
		retValue+=processNode(renderer,entitiesPool,(*poolNode)->entity_childs.begin(),type,time);
#endif
	}

	return retValue;
}



LRESULT CALLBACK OpenGLProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	OpenGLRenderer* renderer=(OpenGLRenderer*)GetWindowLongPtr(hwnd,GWL_USERDATA);

	LRESULT result=0;

	switch(msg)
	{
		case WM_MOUSEMOVE:
		{
			result=DefWindowProc(hwnd,msg,wparam,lparam);

			POINTS p=MAKEPOINTS(lparam);

			bool leftButtonIsDown=(wparam & MK_LBUTTON)!=0;
			bool controlButtonIsDown=(wparam & MK_CONTROL)!=0;

			if(renderer)
				renderer->OnMouseMotion(p.x,p.y,leftButtonIsDown,controlButtonIsDown);

			SetFocus(hwnd);//for sending mousewheel to this window
		}
		break;
		case WM_MOUSEWHEEL:
		{
			result=DefWindowProc(hwnd,msg,wparam,lparam);

			short int delta = GET_WHEEL_DELTA_WPARAM(wparam);

			if(renderer && wparam && lparam)
				renderer->OnMouseWheel(delta);
		
		}
		break;
		case WM_RBUTTONDOWN:
			if(renderer)
				renderer->OnMouseRightDown();
		break;
		case WM_SIZE:
		{
			result=DefWindowProc(hwnd,msg,wparam,lparam);

			if(renderer)
				renderer->OnViewportSize(LOWORD(lparam),HIWORD(lparam));
		}
		break;
		/*case WM_LBUTTONDOWN:
			{
				result=DefWindowProc(hwnd,msg,wparam,lparam);

				if(renderer)
					renderer->OnViewportSize(LOWORD(lparam),HIWORD(lparam));
			}
			break;*/
		case WM_ERASEBKGND:
			return (LRESULT)1;
		break;
		
		default:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
	}

	return result;
}


OpenGLRenderer::OpenGLRenderer()
{
	RendererViewportInterface_viewScale=1.0f;
	RendererViewportInterface_farPlane=3000.0f;

	hglrc=0;
}

char* OpenGLRenderer::Name()
{
	return 0;
}

void OpenGLRenderer::Create(HWND container)
{
	hwnd=CreateWindow(WC_OPENGLWINDOW,"OpenGLFixedRenderer",WS_CHILD,CW_USEDEFAULT,CW_USEDEFAULT,100,100,container,0,0,0);
	hdc=GetDC(hwnd);

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
			MessageBox(0,"pixel format error","ChoosePixelFormat",MB_OK|MB_ICONEXCLAMATION);

		if(!SetPixelFormat(hdc,pixelFormat,&pfd))
			MessageBox(0,"pixel format error","SetPixelFormat",MB_OK|MB_ICONEXCLAMATION);

		if(!(hglrc = wglCreateContext(hdc)))
			MessageBox(0,"create context error","wglCreateContext",MB_OK|MB_ICONEXCLAMATION);

		if(!wglMakeCurrent(hdc,hglrc))
			MessageBox(0,"making current context error","wglMakeCurrent",MB_OK|MB_ICONEXCLAMATION);

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
		0,        //End
	};

	int pixelFormat;
	UINT numFormats;

	if(!wglChoosePixelFormatARB(hdc, pixelFormatAttribList, NULL, 1, &pixelFormat, &numFormats))
		MessageBox(0,"pixel format error","wglChoosePixelFormatARB",MB_OK|MB_ICONEXCLAMATION);

	const int versionAttribList[] = 
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB,4,
		WGL_CONTEXT_MINOR_VERSION_ARB,0,
		0,        //End
	};

	if(!(hglrc = wglCreateContextAttribsARB(hdc, 0, versionAttribList)))
		MessageBox(0,"createcontext error","wglCreateContextAttribsARB",MB_OK|MB_ICONEXCLAMATION);
	
	if(!wglMakeCurrent(hdc,hglrc))
		MessageBox(0,"make context current error","wglMakeCurrent",MB_OK|MB_ICONEXCLAMATION);

	if(!vertexArrayObject)
	{
		glGenVertexArrays(1, &vertexArrayObject);
		glBindVertexArray(vertexArrayObject);

		glGenBuffers(1,&vertexBufferObject);
		glGenBuffers(1,&textureBufferObject);
		/*
		//glGenBuffers(1,&indicesBufferObject);

		glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);
		//glBindBuffer(GL_ARRAY_BUFFER,indicesBufferObject);

		glBufferData(GL_ARRAY_BUFFER,100000,0,GL_DYNAMIC_DRAW);*/

		
	}

	printf("Status: Using GL %s\n", glGetString(GL_VERSION));
	printf("Status: GLSL ver %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
	//printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	SetWindowLongPtr(hwnd,GWL_USERDATA,(LONG_PTR)this);

	renderers.push_back(this);
}

OpenGLRenderer* OpenGLRenderer::CreateSharedContext(HWND container)
{
	OpenGLRenderer* sharedRenderer=new OpenGLRenderer();

	sharedRenderer->hwnd=CreateWindow(WC_OPENGLWINDOW,"OpenGLFixedRenderer",WS_CHILD,CW_USEDEFAULT,CW_USEDEFAULT,100,100,container,0,0,0);

	sharedRenderer->hdc=GetDC(sharedRenderer->hwnd);

	if(!sharedRenderer->hdc)
		MessageBox(0,"Getting Device Context","GetDC",MB_OK|MB_ICONEXCLAMATION);

	sharedRenderer->hglrc=hglrc;
	
	sharedRenderer->ChangeContext();

	printf("Status: Using GL %s\n", glGetString(GL_VERSION));
	printf("Status: GLSL ver %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
	//printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	SetWindowLongPtr(sharedRenderer->hwnd,GWL_USERDATA,(LONG_PTR)this);

	shared_renderers.push_back(sharedRenderer);

	return sharedRenderer;
}




void OpenGLRenderer::ChangeContext()
{
	if(!hglrc || !hdc)
	{
		__debugbreak();
		return;
	}

	if(!wglMakeCurrent(hdc,hglrc))
		__debugbreak();
}

void OpenGLRenderer::Render()
{
	if(!hglrc)
		return;

	//ChangeContext();
	
	
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);glCheckError();
	glClearColor(0,0,0,0);glCheckError();

	draw(vec3(0,0,0),15);
	draw(vec3(0,0,0),vec3(1000,0,0));
	//draw(AABB(vec3(100,0,100),vec3(200,150,150)));

	processNode(this,Entity::pool,Entity::pool.begin(),0,0);
	processNode(this,Entity::pool,Entity::pool.begin(),1,0);
	processNode(this,Entity::pool,Entity::pool.begin(),2,0);

	if(!SwapBuffers(hdc))
		MessageBox(0,"error","SwapBuffers",MB_OK|MB_ICONEXCLAMATION);

	for(int i=0;i<(int)shared_renderers.size();i++)
		shared_renderers[i]->Render();
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
	//glFrontFace(GL_CW);

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

	glUniform1f(shader->GetUniform("textured"),(GLfloat)textureIndices.size());glCheckError();

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


void OpenGLRenderer::draw(Bone* bone)
{
	if(!bone)
		return;

	if(bone->entity_parent->GetBone())
	{
		vec3 b2p=bone->entity_parent->entity_world.position();
		vec3 b1p=bone->entity_world.position();
		this->draw(b1p,b2p,bone->bone_color);
	}
}

float signof(float num){return (num>0 ? 1.0f : (num<0 ? -1.0f : 0.0f));}

void OpenGLRenderer::OnMouseWheel(float factor)
{
	RendererViewportInterface_viewScale+=-signof(factor)*(RendererViewportInterface_viewScale*0.1f);

	RECT rc;
	GetClientRect(this->hwnd,&rc);

	float halfW=((rc.right-rc.left)/2.0f)*RendererViewportInterface_viewScale;
	float halfH=((rc.bottom-rc.top)/2.0f)*RendererViewportInterface_viewScale;
	mat4& p=MatrixStack::projection.perspective(-halfW,halfW,-halfH,halfH,1,RendererViewportInterface_farPlane);
	MatrixStack::SetProjectionMatrix(p);

	//printf("wiewScale %3.2f\n",RendererViewportInterface_viewScale);
}

void OpenGLRenderer::OnMouseRightDown()
{
	mat4& m=MatrixStack::modelview.identity();
	MatrixStack::SetModelviewMatrix(m);
}

void OpenGLRenderer::OnViewportSize(int width,int height)
{
	glViewport(0,0,width,height);glCheckError();
	float halfW=(width/2.0f)*RendererViewportInterface_viewScale;
	float halfH=(height/2.0f)*RendererViewportInterface_viewScale;
	mat4& p=MatrixStack::projection.perspective(-halfW,halfW,-halfH,halfH,1,RendererViewportInterface_farPlane);
	MatrixStack::SetProjectionMatrix(p);
}

void OpenGLRenderer::OnMouseMotion(int x,int y,bool leftButtonDown,bool altIsDown)
{
	vec2 &pos=InputManager::mouseInput.mouse_pos;
	vec2 &oldpos=InputManager::mouseInput.mouse_posold;

	oldpos=pos;
	pos.x=(float)x;
	pos.y=(float)y;

	if(leftButtonDown && GetFocus()==this->hwnd)
	{
		float dX=(pos.x-oldpos.x);
		float dY=(pos.y-oldpos.y);

		mat4& modelview=MatrixStack::modelview;

		if(altIsDown)
		{
			mat4 mX;

			vec3 pos=modelview.position();

			modelview.move(vec3(0,0,0));

			if(dY)
				mX.rotate(-dY,1,0,0);

			modelview.rotate(-dX,0,1,0);

			modelview=modelview*mX;

			modelview.move(pos);
		}
		else
		{
			mat4 invMdlv(modelview.inverse());

			vec3 y=invMdlv.axis(0,1,0);
			vec3 x=invMdlv.axis(1,0,0);

			modelview.translate(x*dX);
			modelview.translate(y*-dY);
		}

		MatrixStack::SetModelviewMatrix(modelview);
	}
}


void OpenGLRenderer::OnMouseDown(int,int)
{

}





#ifndef WIN32_H
#define WIN32_H

#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

//#pragma warning(disable:4996) //


#include <windows.h>
#include <windowsx.h>

#include <winuser.h>
#include <wingdi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <objbase.h>
#include <CommCtrl.h>

#include <array>
#include <map>

#include "gl/gl.h"
//#define GL_GLEXT_PROTOTYPES
#include <glext.h>
#include <wgl.h>
#include <wglext.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

#include <d3d11.h>

#include "interfaces.h"

#define SPLITTER_DEBUG true
#define BROWSER_DEBUG true
#define TMP_DEBUG true

//#define RENDERER DirectXRenderer()
#define RENDERER OpenGLRenderer

#define WC_MAINAPPWINDOW "MainAppWindow 0.1"
#define WC_CONTAINERWINDOW "ContainerWindow"
#define WC_TABCONTAINERWINDOWCLASS "TabContainerWindowClass"

#define TAB_MENU_COMMAND_REMOVE	1
#define TAB_MENU_COMMAND_OPENGLWINDOW 2
#define TAB_MENU_COMMAND_PROJECTFOLDER 3
#define TAB_MENU_COMMAND_LOGGER 4
#define TAB_MENU_COMMAND_SCENEENTITIES 5
#define TAB_MENU_COMMAND_PROJECTFOLDER2 6
#define TAB_MENU_COMMAND_SHAREDOPENGLWINDOW 7
#define TAB_MENU_COMMAND_ENTITYPROPERTIES 8


#define MAINMENU_ENTITIES_IMPORTENTITY 1

#ifndef WINVER
#define WINVER 0x0601
#pragma message (LOCATION "  WINVER: " PRINTF(WINVER))
#endif




void glCheckError();



LRESULT CALLBACK MainWindowProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);

struct Direct2DGuiBase
{
	static ID2D1Factory *factory;
	static IWICImagingFactory *imager;
	static IDWriteFactory *writer;
	static IDWriteTextFormat *texter;

	static void Init(wchar_t* fontName,float fontSize);
	static void Release();

	static ID2D1HwndRenderTarget* InitHWNDRenderer(HWND hwnd);
	static void CreateRawBitmap(const wchar_t* fname,unsigned char*& buffer,float& width,float& height);

	static void DrawText(ID2D1RenderTarget*renderer,ID2D1Brush* brush,const char* text,float x,float y, float w,float h,float ax=-1,float ay=-1);

};










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
extern PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
extern PFNGLUNIFORM3FVPROC glUniform3fv;
extern PFNGLUNIFORM4FVPROC glUniform4fv;
extern PFNGLTEXBUFFERPROC glTexBuffer;
extern PFNGLTEXTUREBUFFERPROC glTextureBuffer;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData;
extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
extern PFNGLREADNPIXELSPROC glReadnPixels;
extern PFNGLUNIFORM2FPROC glUniform2f;
extern PFNGLUNIFORM2FVPROC glUniform2fv;
extern PFNGLUNIFORM3FPROC glUniform3f;
extern PFNGLUNIFORM4FPROC glUniform4f;
extern PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB;
#endif

struct OpenGLRenderer : Renderer3dInterface
{
	GLuint vertexArrayObject;
	GLuint vertexBufferObject;
	GLuint textureBufferObject;
	GLuint indicesBufferObject;
	GLuint frameBuffer;
	GLuint textureColorbuffer;
	GLuint textureRenderbuffer;
	GLuint pixelBuffer;
	GLuint renderBufferColor;
	GLuint renderBufferDepth;

	HGLRC hglrc;
	HDC   hdc;

#if USE_MULTIPLE_OPENGL_CONTEXTS
	GLEWContext* glewContext;
#endif

	TabContainer* tabContainer;

	OpenGLRenderer(TabContainer*);
	~OpenGLRenderer();

	virtual void Create(HWND container);

	char* Name();
	void ChangeContext();
	
	void draw(vec3,float psize=1.0f,vec3 color=vec3(1,1,1));
	void draw(vec2);
	void draw(vec3,vec3,vec3 color=vec3(1,1,1));
	void draw(vec4);
	void draw(AABB,vec3 color=vec3(1,1,1));
	void draw(mat4 mtx,float size,vec3 color=vec3(1,1,1));
	//void draw(Font*,char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4);
	void draw(char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4);
	
	void draw(Light*);
	void draw(Mesh*);
	void draw(Skin*);
	void draw(Bone*);
	void draw(Texture*);
	void drawUnlitTextured(Mesh*);
	void draw(Mesh*,std::vector<unsigned int>& textureIndices,int texture_slot,int texcoord_slot);
	void draw(Camera*);
	void draw(Gizmo*);
	void draw(Piped*);
	virtual void draw(Sphere*){}
	virtual void draw(Cylinder*){}
	virtual void draw(Tetrahedron*){}

	virtual void Render(GuiViewport*,bool force=false);
	virtual void Render();
};



struct DirectXRenderer : WindowData ,  Renderer3dInterface
{
	HINSTANCE               hInst;
	D3D_DRIVER_TYPE         driverType;
	D3D_FEATURE_LEVEL       featureLevel;
	ID3D11Device*           pd3dDevice;
	ID3D11DeviceContext*    pImmediateContext;
	IDXGISwapChain*         pSwapChain;
	ID3D11RenderTargetView* pRenderTargetView;

	DirectXRenderer();
	~DirectXRenderer();

	virtual char* Name();
	virtual void  Create(HWND container);
	virtual void Render();

	virtual void draw(vec3,float psize=1.0f,vec3 color=vec3(1,1,1)){}
	virtual void draw(vec2){}
	virtual void draw(vec3,vec3,vec3 color=vec3(1,1,1)){}
	virtual void draw(vec4){}
	//virtual void draw(Font*,char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4){}
	virtual void draw(char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4){}

	virtual void draw(Light*){}
	virtual void draw(Mesh*){}
	virtual void draw(Skin*){}
	virtual void draw(Bone*){}
	virtual void draw(Texture*){}
	virtual void drawUnlitTextured(Mesh*){}
	virtual void draw(Mesh*,std::vector<unsigned int>& textureIndices,int texture_slot,int texcoord_slot){}
};





#endif //WIN32_H
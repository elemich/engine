#ifndef WIN32_H
#define WIN32_H

struct TabContainerWin32;
struct EditorWindowContainerWin32;
struct Renderer2DInterfaceWin32;

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
#include "fbxutil.h"

#define SPLITTER_DEBUG true
#define BROWSER_DEBUG true
#define TMP_DEBUG true

//#define RENDERER DirectXRenderer()
#define RENDERER OpenGLRenderer

#define WC_MAINAPPWINDOW "MainAppWindow 0.1"
#define WC_CONTAINERWINDOW "ContainerWindow"
#define WC_TABCONTAINERWINDOWCLASS "TabContainerWindowClass"

#define MAINMENU_ENTITIES_IMPORTENTITY 1

#ifndef WINVER
#define WINVER 0x0601
//#pragma message (LOCATION "  WINVER: " PRINTF(WINVER))
#endif

void glCheckError();

struct Direct2DBase
{
	static ID2D1Factory *factory;
	static IWICImagingFactory *imager;
	static IDWriteFactory *writer;
	static IDWriteTextFormat *texter;

	static void Init(wchar_t* fontName=L"Verdana",float fontSize=10);
	static void Release();

	static void CreateRawBitmap(const wchar_t* fname,unsigned char*& buffer,float& width,float& height);

	static void DrawText(ID2D1RenderTarget*renderer,ID2D1Brush* brush,const char* text,float x,float y, float w,float h,float iAlignPosX=-1,float iAlignPosY=-1);
	static void DrawRectangle(ID2D1RenderTarget*renderer,ID2D1Brush* brush,float x,float y, float w,float h,bool fill=true);
	static void DrawBitmap(ID2D1RenderTarget*renderer,ID2D1Bitmap* bitmap,float x,float y, float w,float h);

	static void PushScissor(ID2D1RenderTarget*renderer,float x,float y,float w,float h);
	static void PopScissor(ID2D1RenderTarget*);

	static void Translate(ID2D1RenderTarget*,float,float);
	static void Identity(ID2D1RenderTarget*);

	static vec2 MeasureText(ID2D1RenderTarget*,const char*,int iSlen=-1);
};




struct ThreadWin32 : ThreadInterface
{
	HANDLE handle;

	ThreadWin32();
	~ThreadWin32();
};


struct GuiImageWin32 : GuiImage
{
	GuiImageWin32();
	~GuiImageWin32();

	ID2D1Bitmap* handle;

	void Release();
	bool Fill(Renderer2DInterface*,unsigned char* iData,float iWidth,float iHeight);
};

struct Renderer2DInterfaceWin32 : Renderer2DInterface
{
	ID2D1HwndRenderTarget* renderer;
	ID2D1SolidColorBrush*  brush;

	Renderer2DInterfaceWin32(HWND);
	~Renderer2DInterfaceWin32();

	void DrawText(const char* iText,float iX,float iY, float iW,float iH,unsigned int iColor,float iAlignPosX,float iAlignPosY);
	void DrawRectangle(float iX,float iY, float iW,float iH,unsigned int iColor,bool iFill=true);
	void DrawRectangle(vec4& iXYWH,unsigned int iColor,bool iFill=true);
	void DrawBitmap(GuiImage* iImage,float iX,float iY, float iW,float iH);

	void PushScissor(float x,float y,float w,float h);
	void PopScissor();

	void Translate(float,float);
	void Identity();

	vec2 MeasureText(const char*,int iSlen=-1);

	ID2D1Brush* SetColorWin32(unsigned int color);

	bool RecreateTarget(HWND);
};

struct OpenGLRenderer : Renderer3DInterface
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

	TabContainerWin32* tabContainerWin32;

	OpenGLRenderer(TabContainerWin32*);
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
	void draw(Script*);

	virtual void Render(GuiViewport*,bool force=false);
	virtual void Render();

	void draw(EntityComponent*);
	void draw(Entity*);
};



struct DirectXRenderer : WindowData ,  Renderer3DInterface
{
	HINSTANCE               hInst;
	D3D_DRIVER_TYPE         driverType;
	D3D_FEATURE_LEVEL       featureLevel;
	ID3D11Device*           pd3dDevice;
	ID3D11DeviceContext*    pImmediateContext;
	IDXGISwapChain*         pSwapChain;
	ID3D11RenderTargetView* pRenderTargetView;

	DirectXRenderer(TabContainer*);
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

struct WindowDataWin32 : WindowData
{
	HWND hwnd;
	UINT msg;
	WPARAM wparam;
	LPARAM lparam;

	operator HWND(){return this->hwnd;};
	void CopyProcedureData(HWND  h,UINT m,WPARAM w,LPARAM l);
	void Create(HWND container);

	void OnSize();
	void OnWindowPosChanging();

	void LinkSibling(WindowData* t,int pos);
	void UnlinkSibling(WindowData* t=0);
	WindowDataWin32* FindSiblingOfSameSize();
	int FindSiblingPosition(WindowData* t);
	bool FindAndGrowSibling();
};




struct TabContainerWin32 : TabContainer
{
	WindowDataWin32*& windowDataWin32;
	EditorWindowContainerWin32*& editorWindowContainerWin32;
	Renderer2DInterfaceWin32* renderer2DWin32;

	static LRESULT CALLBACK TabContainerWindowClassProcedure(HWND,UINT,WPARAM,LPARAM);

	TabContainerWin32(float x,float y,float w,float h,HWND parent);
	~TabContainerWin32();

	operator TabContainerWin32& (){return *this;}

	bool BeginDraw();
	void EndDraw();

	void DrawFrame();

	void OnGuiMouseMove(void* data=0);
	void OnGuiMouseWheel(void* data=0);
	void OnGuiRMouseUp(void* data=0);
	void OnGuiPaint(void* data=0);
	void OnResizeContainer(void* data=0);
	void OnGuiRecreateTarget(void* data=0);

	int TrackGuiSceneViewerPopup(bool iSelected);
	int TrackTabMenuPopup();

	void SetCursor(int);
};

struct SplitterContainerWin32 : SplitterContainer 
{
	TabContainerWin32*& currentTabContainerWin32;

	TabContainerWin32*& floatingTabRefWin32;
	TabContainerWin32*& floatingTabWin32;
	TabContainerWin32*& floatingTabTargetWin32;

	static HMENU popupMenuRoot;
	static HMENU popupMenuCreate;

	RECT floatingTabRc;
	RECT floatingTabTargetRc;

	POINTS splitterPreviousPos;

	std::vector<HWND> resizingWindows1;
	std::vector<HWND> resizingWindows2;
	HWND hittedWindow1;
	HWND hittedWindow2;

	SplitterContainerWin32();
	~SplitterContainerWin32();

	void OnLButtonDown(HWND,LPARAM);
	void OnLButtonUp(HWND);
	void OnMouseMove(HWND,LPARAM);
	void OnSize(HWND,WPARAM,LPARAM);

	std::vector<HWND> findWindoswAtPos(HWND mainWindow,RECT &srcRect,int rectPosition);

	void EnableChilds(HWND hwnd,int enable=-1,int show=-1);
	void EnableAllChildsDescendants(HWND hwnd,int enable=-1,int show=-1);

	void CreateFloatingTab(TabContainer*);
	void DestroyFloatingTab();
};

struct EditorWindowContainerWin32 : EditorWindowContainer
{
	EditorWindowContainerWin32();

	void OnSizing();
	void OnSize();

	WindowDataWin32*& windowDataWin32;
	SplitterContainerWin32*& splitterContainerWin32;

	TabContainerWin32* CreateTabContainer(float x,float y,float w,float h);
};

struct EditorMainAppWindowWin32 : EditorMainAppWindow
{
	static LRESULT CALLBACK MainWindowProcedure(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);

	HMENU menuMain;
	HMENU menuEntities;

	void Init();

	EditorWindowContainerWin32* CreateContainer();
};

struct TimerWin32 : Timer
{
	virtual void update();
	virtual unsigned int GetTime();
};

struct AppWin32 : AppInterface
{
	AppWin32();

	int Initialize();	
	void Deinitialize();

	void Run();

	void CreateNodes(String,ResourceNodeDir*);
	void ScanDir(String);
};

struct ScriptWin32 : Script
{
	HMODULE dllModule;

	ScriptWin32();

	bool Run();
	bool Exit();
};

struct CompilerInterfaceWin32 : CompilerInterface
{
	CompilerInterfaceWin32();

	bool Compile(Script*);
};



#endif //WIN32_H
#ifndef WIN32_H
#define WIN32_H



//#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

//#pragma warning(disable:4996) //

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <windowsx.h>

#include <winuser.h>
#include <wingdi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <objbase.h>
#include <CommCtrl.h>
#include <TlHelp32.h>

#include <array>
#include <map>

#include <gl/gl.h> //platform dependent
#include <gl/glext.h>
#include <gl/wgl.h>
#include <gl/wglext.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

#include <d3d11.h>

#include "interfaces.h"

DLLBUILD void ___saferelease(IUnknown* iPtr);

#define SAFERELEASE(_ptr) ___saferelease(_ptr)

struct DLLBUILD TabWin32;
struct DLLBUILD ContainerWin32;
struct DLLBUILD Renderer2DWin32;

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

struct DLLBUILD Direct2D
{
	static ID2D1Factory			*factory;
	static IWICImagingFactory	*imager;
	static IDWriteFactory		*writer;
	static IDWriteTextFormat	*texter;

	static const int fontLogicSize;
	static const wchar_t* fontFaceName;

	static char charsWidth[255];

	static void Init();
	static void Release();

	static void CreateRawBitmap(const wchar_t* fname,unsigned char*& buffer,float& width,float& height);

	static void DrawText(ID2D1RenderTarget*renderer,ID2D1Brush* brush,const char* text,float x,float y, float w,float h,float iAlignPosX=-1,float iAlignPosY=-1,bool iClip=true);
	static void DrawText(ID2D1RenderTarget*renderer,ID2D1Brush* brush,const wchar_t* text,int iLength,float x,float y, float w,float h,float iAlignPosX=-1,float iAlignPosY=-1,bool iClip=true);

	static void DrawRectangle(ID2D1RenderTarget*renderer,ID2D1Brush* brush,float x,float y, float w,float h,bool fill=true);
	static void DrawBitmap(ID2D1RenderTarget*renderer,ID2D1Bitmap* bitmap,float x,float y, float w,float h);

	static void PushScissor(ID2D1RenderTarget*renderer,float x,float y,float w,float h);
	static void PopScissor(ID2D1RenderTarget*);

	static void Translate(ID2D1RenderTarget*,float,float);
	static void Identity(ID2D1RenderTarget*);

	static vec2 MeasureText(ID2D1RenderTarget*,const char*,int iSlen=-1);
	static vec2 MeasureText(ID2D1RenderTarget*,const wchar_t*,int iSlen=-1);

	static float GetFontHeight(ID2D1HwndRenderTarget*);
	static float GetFontSize();

	static float GetCharWidth(char iCharacter);
};




struct DLLBUILD ThreadWin32 : Thread
{
	HANDLE handle;

	ThreadWin32();
	~ThreadWin32();
};


struct DLLBUILD GuiImageWin32 : GuiImage
{
	GuiImageWin32();
	~GuiImageWin32();

	ID2D1Bitmap* handle;

	void Release();
	bool Fill(Renderer2D*,unsigned char* iData,float iWidth,float iHeight);
};

struct DLLBUILD Renderer2DWin32 : Renderer2D
{
	ID2D1HwndRenderTarget*		renderer;
	ID2D1SolidColorBrush*		brush;

	Renderer2DWin32(Tab*,HWND);
	~Renderer2DWin32();

	void DrawText(const char* iText,float iX,float iY, float iWw,float iH,unsigned int iColor=COLOR_TEXT,float iAlignPosX=-1,float iAlignPosY=-1,bool iClip=true);
	void DrawText(const wchar_t* iText,float iX,float iY, float iWw,float iH,unsigned int iColor=COLOR_TEXT,float iAlignPosX=-1,float iAlignPosY=-1,bool iClip=true);
	void DrawRectangle(float iX,float iY, float iW,float iH,unsigned int iColor,bool iFill=true);
	void DrawRectangle(vec4& iXYWH,unsigned int iColor,bool iFill=true);
	void DrawBitmap(GuiImage* iImage,float iX,float iY, float iW,float iH);

	void PushScissor(float x,float y,float w,float h);
	void PopScissor();

	void Translate(float,float);
	void Identity();

	vec2 MeasureText(const char*,int iSlen=-1);
	vec2 MeasureText(const wchar_t*,int iSlen=-1);

	float GetFontSize();
	float GetFontHeight();
	float GetCharWidth(char iCharacter);

	void SetTabSpaces(unsigned int iNumOfSpaces);

	ID2D1Brush* SetColorWin32(unsigned int color);

	bool RecreateTarget(HWND);

	void DrawCaret();
	void SetCaret(GuiRect* iGuiRect,vec2 iPosition,vec2 iRect);
	void EnableCaret(bool);
};

struct DLLBUILD Renderer3DOpenGL : Renderer3D
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

	TabWin32* tabContainerWin32;

	Renderer3DOpenGL(TabWin32*);
	~Renderer3DOpenGL();

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



struct DLLBUILD DirectXRenderer : WindowData ,  Renderer3D
{
	HINSTANCE               hInst;
	D3D_DRIVER_TYPE         driverType;
	D3D_FEATURE_LEVEL       featureLevel;
	ID3D11Device*           pd3dDevice;
	ID3D11DeviceContext*    pImmediateContext;
	IDXGISwapChain*         pSwapChain;
	ID3D11RenderTargetView* pRenderTargetView;

	DirectXRenderer(Tab*);
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

struct DLLBUILD WindowDataWin32 : WindowData
{
	HWND hwnd;
	UINT msg;
	WPARAM wparam;
	LPARAM lparam;
	HDC    hdc;

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




struct DLLBUILD TabWin32 : Tab
{
	WindowDataWin32*& windowDataWin32;
	ContainerWin32*& editorWindowContainerWin32;
	Renderer2DWin32* renderer2DWin32;

	static LRESULT CALLBACK TabContainerWindowClassProcedure(HWND,UINT,WPARAM,LPARAM);

	TabWin32(float x,float y,float w,float h,HWND parent);
	~TabWin32();

	operator TabWin32& (){return *this;}

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
	int TrackProjectFileViewerPopup(ResourceNode*);

	void SetCursor(int);

};

struct DLLBUILD SplitterWin32 : Splitter
{
	TabWin32*& currentTabContainerWin32;

	TabWin32*& floatingTabRefWin32;
	TabWin32*& floatingTabWin32;
	TabWin32*& floatingTabTargetWin32;

	static HMENU popupMenuRoot;
	static HMENU popupMenuCreate;

	RECT floatingTabRc;
	RECT floatingTabTargetRc;

	POINTS splitterPreviousPos;

	std::vector<HWND> resizingWindows1;
	std::vector<HWND> resizingWindows2;
	HWND hittedWindow1;
	HWND hittedWindow2;

	SplitterWin32();
	~SplitterWin32();

	void OnLButtonDown(HWND,LPARAM);
	void OnLButtonUp(HWND);
	void OnMouseMove(HWND,LPARAM);
	void OnSize(HWND,WPARAM,LPARAM);

	std::vector<HWND> findWindoswAtPos(HWND mainWindow,RECT &srcRect,int rectPosition);

	void EnableChilds(HWND hwnd,int enable=-1,int show=-1);
	void EnableAllChildsDescendants(HWND hwnd,int enable=-1,int show=-1);

	void CreateFloatingTab(Tab*);
	void DestroyFloatingTab();
};

struct DLLBUILD ContainerWin32 : Container
{
	ContainerWin32();

	void OnSizing();
	void OnSize();

	WindowDataWin32*& windowDataWin32;
	SplitterWin32*& splitterContainerWin32;

	TabWin32* CreateTabContainer(float x,float y,float w,float h);
};

struct DLLBUILD MainContainerWin32 : MainContainer
{
	static LRESULT CALLBACK MainWindowProcedure(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);

	HMENU menuMain;
	HMENU menuEntities;

	void Init();

	ContainerWin32* CreateContainer();
};

struct DLLBUILD TimerWin32 : Timer
{
	TimerWin32();

	virtual void update();
	virtual unsigned int GetTime();
};

struct DLLBUILD IdeWin32 : Ide
{
	IdeWin32();

	HANDLE processThreadHandle;
	HANDLE processHandle;

	int Initialize();
	void Deinitialize();

	void Run();

	void ScanDir(String,ResourceNodeDir*);
	void Sleep(int iMilliseconds=1);
};

struct DLLBUILD SubsystemWin32 : Subsystem
{
	bool Execute(String iPath,String iCmdLine,String iOutputFile="",bool iInput=false,bool iError=false,bool iOutput=false,bool iNewConsole=false);
	unsigned int FindProcessId(String iProcessName);
	unsigned int FindThreadId(unsigned int iProcessId,String iThreadName);
};


struct DLLBUILD CompilerWin32 : Compiler
{
	std::map<Script*,HMODULE*> ideScriptSourceModules;

	bool Compile(Script*);
	String Compose(unsigned int iCompiler,Script*);
	bool LoadScript(Script*);
	bool UnloadScript(Script*);
	bool CreateAndroidTarget();
};

struct DLLBUILD DebuggerWin32 : Debugger
{
	HANDLE debuggeeThread;
	DWORD  debuggeeThreadId;

	CONTEXT*	threadContext;

	DebuggerWin32();
	~DebuggerWin32();

	void RunDebuggeeFunction(Script* iDebuggee,unsigned char iFunctionIndex);
	void SuspendDebuggee();
	void ResumeDebuggee();

	void SetBreakpoint(Breakpoint&,bool);

	void BreakDebuggee(Breakpoint&);
	void ContinueDebuggee();


	int HandleHardwareBreakpoint(void*);
	void SetHardwareBreakpoint(Breakpoint&,bool);

	void PrintThreadContext(void*);
};


struct DLLBUILD StringEditorWin32 : StringEditor
{
	ID2D1Bitmap* background;

	StringEditorWin32();
	~StringEditorWin32();

	void Draw(Tab*);
};


#endif //WIN32_H

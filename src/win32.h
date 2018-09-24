#ifndef WIN32_H
#define WIN32_H



//#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

//#pragma warning(disable:4996) //

#define _CRT_SECURE_NO_WARNINGS
#define UNICODE 1

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

#define WC_MAINAPPWINDOW L"MainAppWindow 0.1"
#define WC_CONTAINERWINDOW L"ContainerWindow"
#define WC_TABCONTAINERWINDOWCLASS L"TabContainerWindowClass"

#define MAINMENU_ENTITIES_IMPORTENTITY 1

#ifndef WINVER
#define WINVER 0x0601
//#pragma message (LOCATION "  WINVER: " PRINTF(WINVER))
#endif

void glCheckError();

struct DLLBUILD GuiFontWin32 : GuiFont
{
	IDWriteTextFormat* texter;
};

struct DLLBUILD Direct2D
{
	static ID2D1Factory			*factory;
	static IWICImagingFactory	*imager;
	static IDWriteFactory		*writer;

	static void Init();
	static void Release();

	static bool CreateFont(String,float iFontSize);

	static void CreateRawBitmap(const wchar_t* fname,unsigned char*& buffer,float& width,float& height);

	static void DrawText(ID2D1RenderTarget*renderer,IDWriteTextFormat* texter,ID2D1Brush* brush,const String& text,float x,float y, float w,float h,float iAlignPosX=-1,float iAlignPosY=-1,bool iClip=true);

	static void DrawRectangle(ID2D1RenderTarget*renderer,ID2D1Brush* brush,float x,float y, float w,float h,bool fill=true,float op=1.0f);
	static void DrawBitmap(ID2D1RenderTarget*renderer,ID2D1Bitmap* bitmap,float x,float y, float w,float h);
	static void DrawCaret(ID2D1RenderTarget* renderer,ID2D1Geometry* caret,ID2D1Brush* iBrush);

	static void PushScissor(ID2D1RenderTarget*renderer,float x,float y,float w,float h);
	static void PopScissor(ID2D1RenderTarget*);

	static void Translate(ID2D1RenderTarget*,float,float);
	static void Identity(ID2D1RenderTarget*);
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

	void DrawText(const String& iText,float left,float top, float right,float bottom,unsigned int iColor=GuiString::COLOR_TEXT);
	void DrawText(const String& iText,float left,float top, float right,float bottom,vec2 iSpot,vec2 iAlign,unsigned int iColor=GuiString::COLOR_TEXT);
	void DrawRectangle(float iX,float iY, float iW,float iH,unsigned int iColor,bool iFill=true,float op=1.0f);
	void DrawRectangle(vec4& iXYWH,unsigned int iColor,bool iFill=true);
	void DrawBitmap(GuiImage* iImage,float iX,float iY, float iW,float iH);

	void PushScissor(float x,float y,float w,float h);
	void PopScissor();

	void Translate(float,float);
	void Identity();
	ID2D1Brush* SetColorWin32(unsigned int color,float opaque=1.0f);

	bool RecreateTarget(HWND);

	void DrawCaret();
	void SetCaretPos(float x,float y);
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

	PFNWGLCHOOSEPIXELFORMATEXTPROC wglChoosePixelFormatARB;
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;

	PFNGLATTACHSHADERPROC glAttachShader;
	PFNGLBINDBUFFERPROC glBindBuffer;
	PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
	PFNGLBUFFERDATAPROC glBufferData;
	PFNGLCOMPILESHADERPROC glCompileShader;
	PFNGLCREATEPROGRAMPROC glCreateProgram;
	PFNGLCREATESHADERPROC glCreateShader;
	PFNGLDELETEBUFFERSPROC glDeleteBuffers;
	PFNGLDELETEPROGRAMPROC glDeleteProgram;
	PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
	PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
	PFNGLDELETESHADERPROC glDeleteShader;
	PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
	PFNGLDETACHSHADERPROC glDetachShader;
	PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
	PFNGLENABLEVERTEXARRAYATTRIBPROC glEnableVertexArrayAttrib;
	PFNGLGENBUFFERSPROC glGenBuffers;
	PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
	PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
	PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
	PFNGLGETPROGRAMIVPROC glGetProgramiv;
	PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
	PFNGLGETSHADERIVPROC glGetShaderiv;
	PFNGLLINKPROGRAMPROC glLinkProgram;
	PFNGLSHADERSOURCEPROC glShaderSource;
	PFNGLUSEPROGRAMPROC glUseProgram;
	PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
	PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
	PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
	PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
	PFNGLACTIVETEXTUREPROC glActiveTexture;
	PFNGLUNIFORM1IPROC glUniform1i;
	PFNGLUNIFORM1FPROC glUniform1f;
	PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
	PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
	PFNGLUNIFORM3FVPROC glUniform3fv;
	PFNGLUNIFORM4FVPROC glUniform4fv;
	PFNGLTEXBUFFERPROC glTexBuffer;
	PFNGLTEXTUREBUFFERPROC glTextureBuffer;
	PFNGLBUFFERSUBDATAPROC glBufferSubData;
	PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
	PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
	PFNGLREADNPIXELSPROC glReadnPixels;
	PFNGLUNIFORM2FPROC glUniform2f;
	PFNGLUNIFORM2FVPROC glUniform2fv;
	PFNGLUNIFORM3FPROC glUniform3f;
	PFNGLUNIFORM4FPROC glUniform4f;
	PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB;

	HGLRC hglrc;
	HDC   hdc;

	int pixelFormat;

	TabWin32* tab;

	Renderer3DOpenGL(TabWin32*);
	~Renderer3DOpenGL();

	virtual void Initialize();
	virtual void Deinitialize();

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


	void draw(EntityComponent*);
	void draw(Entity*);

	int		CreateShader(const char* name,int shader_type, const char* shader_src);
	Shader* CreateProgram(const char* name,const char* vertexsh,const char* fragmentsh);
	Shader* CreateShaderProgram(const char* name,const char* pix,const char* frag);
};

struct DLLBUILD ShaderOpenGL : Shader
{
	Renderer3DOpenGL* renderer3DOpenGL;

	ShaderOpenGL(Renderer3DOpenGL*);
	~ShaderOpenGL();

	int GetProgram();
	void SetProgram(int);

	int GetUniform(int slot,char* var);
	int GetAttrib(int slot,char* var);

	void Use();

	const char* GetPixelShader();
	const char* GetFragmentShader();

	int GetAttribute(const char*);
	int GetUniform(const char*);

	int init();

	int GetPositionSlot();
	int GetColorSlot();
	int GetProjectionSlot();
	int GetModelviewSlot();
	int GetTexcoordSlot();
	int GetTextureSlot();
	int GetLightposSlot();
	int GetLightdiffSlot();
	int GetLightambSlot();
	int GetNormalSlot();
	int GetMouseSlot();
	int GetHoveringSlot();
	int GetPointSize();

	void SetSelectionColor(bool pick,void* ptr,vec2 iMpos,vec2 iRectSize);

	bool SetMatrix4f(int slot,float* mtx);

	unsigned int& GetBufferObject();

	void SetProjectionMatrix(float*);
	void SetModelviewMatrix(float*);
	void SetMatrices(float* view,float* mdl);
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

	operator HWND(){return this->hwnd;};
	void CopyProcedureData(HWND  h,UINT m,WPARAM w,LPARAM l);
	void Create(HWND container);

	void LinkSibling(WindowData* t,int pos);
	void UnlinkSibling(WindowData* t=0);
	WindowDataWin32* FindSiblingOfSameSize();
	int FindSiblingPosition(WindowData* t);
	bool FindAndGrowSibling();

	vec2 Size();
};


struct DLLBUILD TabWin32 : Tab
{
	WindowDataWin32*& windowDataWin32;
	ContainerWin32*& containerWin32;
	Renderer2DWin32*& renderer2DWin32;
	Renderer3DOpenGL*& renderer3DOpenGL;
	ThreadWin32*& threadRenderWin32;

	static LRESULT CALLBACK TabContainerWindowClassProcedure(HWND,UINT,WPARAM,LPARAM);

	TabWin32(float x,float y,float w,float h,HWND parent,bool child=true,bool overlapped=false);
	~TabWin32();

	operator TabWin32& (){return *this;}

	bool BeginDraw();
	void EndDraw();

	void Destroy();

	void Create3DRenderer();
	void Destroy3DRenderer();

	void OnGuiMouseMove(void* data=0);
	void OnGuiMouseWheel(void* data=0);
	void OnGuiRMouseUp(void* data=0);
	void OnResizeContainer(void* data=0);
	void OnGuiRecreateTarget(void* data=0);

	int TrackGuiSceneViewerPopup(bool iSelected);
	int TrackTabMenuPopup();
	int TrackProjectFileViewerPopup(ResourceNode*);

	void SetCursor(int);

};

struct DLLBUILD GuiViewportWin32 : GuiViewport
{
	ID2D1Bitmap*	renderBitmap;

	GuiViewportWin32();
	~GuiViewportWin32();

	void Render(Tab*);
	void DrawBuffer(Tab*,vec4&);
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
	~ContainerWin32();

	void OnSizing();
	void OnSize();

	WindowDataWin32*& windowDataWin32;
	SplitterWin32*& splitterContainerWin32;

	TabWin32* CreateTab(float x,float y,float w,float h);
	TabWin32* CreateModalTab(float w,float h);
	void DestroyTab(Tab*);

	void Enable(bool);
};

struct DLLBUILD MainContainerWin32 : MainContainer
{
	static LRESULT CALLBACK MainWindowProcedure(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);

	ContainerWin32* mainContainerWin32;

	MainContainerWin32();
	~MainContainerWin32();

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
	~IdeWin32();

	HANDLE processThreadHandle;
	HANDLE processHandle;

	void Run();

	void ScanDir(String,ResourceNodeDir*);
	void Sleep(int iMilliseconds=1);
};

struct DLLBUILD SubsystemWin32 : Subsystem
{
	bool Execute(String iPath,String iCmdLine,String iOutputFile=L"",bool iInput=false,bool iError=false,bool iOutput=false,bool iNewConsole=false);
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

struct DLLBUILD PluginSystemWin32 : PluginSystem
{
	std::vector<HMODULE> tPluginDlls;

	void ScanPluginsDirectory();

	PluginSystemWin32();
	~PluginSystemWin32();
};


#endif //WIN32_H

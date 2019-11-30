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

struct DLLBUILD FrameWin32;
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

	GuiFontWin32();
	~GuiFontWin32();
};

struct DLLBUILD Direct2D
{
	
	static ID2D1Factory			*factory;
	static IWICImagingFactory	*imager;
	static IDWriteFactory		*writer;

	static void Init();
	static void Release();

	static GuiFont* CreateFont(String,float iFontSize);

	static bool CreateRawBitmap(const wchar_t* fname,bool iAction,ID2D1RenderTarget* renderer,ID2D1Bitmap* iBitmap,unsigned char** buffer,float* width,float* height);

	static void DrawText(Renderer2D*,const GuiFont*,unsigned int iColor,const String& text,float x,float y, float w,float h,float iAlignPosX=-1,float iAlignPosY=-1,bool iClip=true);

	static void DrawLine(ID2D1RenderTarget*renderer,ID2D1Brush* brush,vec2,vec2,float iWidth=0.5f,float iOpacity=1.0f);
	static void DrawRectangle(ID2D1RenderTarget*renderer,ID2D1Brush* brush,float x,float y, float w,float h,float iStroke=0,float iOpacity=1.0f);
	static void DrawRoundRectangle(ID2D1RenderTarget*renderer,ID2D1Brush* brush,float x,float y, float w,float h,float iRadiusA,float iRadiusB,float iStroke=0,float iOpacity=1.0f);
	static void DrawCircle(ID2D1RenderTarget*renderer,ID2D1Brush* brush,float x,float y,float iRadius,float iStroke=0,float iOpacity=1.0f);
	static void DrawEllipse(ID2D1RenderTarget*renderer,ID2D1Brush* brush,float x,float y,float iRadiusA,float iRadiusB,float iStroke=0,float iOpacity=1.0f);
	static void DrawBitmap(ID2D1RenderTarget*renderer,ID2D1Bitmap* bitmap,float x,float y, float w,float h);
	

	static void PushScissor(ID2D1RenderTarget*renderer,float x,float y,float w,float h);
	static void PopScissor(ID2D1RenderTarget*);

	static void Translate(ID2D1RenderTarget*,float,float);
	static void Identity(ID2D1RenderTarget*);

	static bool LoadBitmapRef(ID2D1RenderTarget* renderer,ID2D1Bitmap*& iHandle,unsigned char* iData,float iWidth,float iHeight);
	static bool LoadBitmapFile(ID2D1RenderTarget* renderer,String iFilename,ID2D1Bitmap*& iHandle,float& iWidth,float& iHeight);

	static void SetAntialiasing(ID2D1RenderTarget* renderer,bool iAntialiasing);

	static float ScaleFloatDpi(float& iCoord);
	static void ScaleEdgesDpi(float& iCoordX, float& iCoordY, float& iCoordZ, float& iCoordW);
};




struct DLLBUILD ThreadWin32 : Thread
{
	HANDLE handle;

	ThreadWin32();
	~ThreadWin32();
};


struct DLLBUILD Renderer2DWin32 : Renderer2D
{
	FrameWin32*					framewin32;
	ID2D1HwndRenderTarget*		renderer;
	ID2D1SolidColorBrush*		brush;

	Renderer2DWin32(Frame*,HWND);
	~Renderer2DWin32();

	void DrawText(const String& iText,float left,float top, float right,float bottom,unsigned int iColor=GuiString::COLOR_TEXT,const GuiFont* iFont=GuiFont::GetDefaultFont());
	void DrawText(const String& iText,float left,float top, float right,float bottom,vec2 iSpot,vec2 iAlign,unsigned int iColor=GuiString::COLOR_TEXT,const GuiFont* iFont=GuiFont::GetDefaultFont());
	void DrawLine(vec2 p1,vec2 p2,unsigned int iColor,float iWidth=0.5f,float iOpacity=1.0f);
	void DrawRectangle(float iX,float iY, float iW,float iH,unsigned int iColor,float iStroke=0,float op=1.0f);
	void DrawRoundRectangle(float x,float y, float w,float h,float iRadiusA,float iRadiusB,unsigned iColor,float iStroke=0,float iOpacity=1.0f);
	void DrawCircle(float x,float y,float iRadius,unsigned iColor,float iStroke=0,float iOpacity=1.0f);
	void DrawEllipse(float x,float y,float iRadiusA,float iRadiusB,unsigned iColor,float iStroke=0,float iOpacity=1.0f);
	void DrawBitmap(Picture* iImage,float iX,float iY, float iW,float iH);
	bool LoadBitmap(Picture*);

	unsigned int	ReadPixel(float x,float y);

	void			PushScissor(float x,float y,float w,float h);
	void			PopScissor();

	void			Translate(float,float);
	void			Identity();
	ID2D1Brush*		SetColorWin32(unsigned int color,float opaque=1.0f);
	void			SetAntialiasing(bool);

	bool			RecreateTarget(HWND);
};


struct DLLBUILD Renderer3DOpenGL : Renderer3D
{
	FrameWin32* frameWin32;

	HWND	hwnd;
	HGLRC	hglrc;
	HDC		hdc;

	int	  pixelFormat;

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

	Renderer3DOpenGL(FrameWin32*);
	~Renderer3DOpenGL();

	PFNWGLCHOOSEPIXELFORMATEXTPROC wglChoosePixelFormatARB;
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;

	PFNGLATTACHSHADERPROC glAttachShader;
	PFNGLBINDBUFFERPROC glBindBuffer;
	PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
	PFNGLBUFFERDATAPROC glBufferData;
	PFNGLNAMEDBUFFERDATAPROC glNamedBufferData;
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

	

	virtual void Initialize();
	virtual void Deinitialize();

	char* Name();
	void ChangeContext();

	void DrawPoint(vec3,float psize=1.0f,vec3 color=vec3(1,1,1));
	void draw(vec2);
	void DrawLine(vec3,vec3,vec3 color=vec3(1,1,1));
	void draw(vec4);
	void draw(AABB,vec3 color=vec3(1,1,1));
	void draw(mat4 mtx,float size,vec3 color=vec3(1,1,1));
	//void draw(Font*,char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4);
	void DrawText(char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4);

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
	Renderer3DOpenGL* renderer;

	ShaderOpenGL(Renderer3DOpenGL*);
	~ShaderOpenGL();

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

	DirectXRenderer(Frame*);
	~DirectXRenderer();

	virtual char* Name();
	virtual void  Create(HWND container);
	virtual void Render();

	virtual void DrawPoint(vec3,float psize=1.0f,vec3 color=vec3(1,1,1)){}
	virtual void draw(vec2){}
	virtual void DrawLine(vec3,vec3,vec3 color=vec3(1,1,1)){}
	virtual void draw(vec4){}
	//virtual void draw(Font*,char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4){}
	virtual void DrawText(char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4){}

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
	HDC  hdc;

	WindowDataWin32();

	void Enable(bool);
	bool IsEnabled();

	vec2 GetSize();
	vec2 GetPos();
	void SetPos(float x,float y);
	void Show(bool);
	bool IsVisible();

	virtual int GetWindowHandle();

	void SendMessage(unsigned iCode=0,unsigned data1=0,unsigned data2=0);
	void PostMessage(unsigned iCode=0,unsigned data1=0,unsigned data2=0);
};

struct DLLBUILD FrameWin32 : Frame
{
	WindowDataWin32*& windowDataWin32;
	Renderer2DWin32*& renderer2DWin32;
	Renderer3DOpenGL*& renderer3DOpenGL;
	ThreadWin32*& threadRenderWin32;

	static LRESULT CALLBACK FrameWin32Procedure(HWND,UINT,WPARAM,LPARAM);

	FrameWin32(float x,float y,float w,float h);
	~FrameWin32();

	bool BeginDraw(void*);
	void EndDraw();

	void Destroy();

	void Create3DRenderer();
	void Destroy3DRenderer();

	void OnRecreateTarget();

	int TrackGuiSceneViewerPopup(bool iSelected);
	int TrackTabMenuPopup();
	int TrackProjectFileViewerPopup(ResourceNode*);

	void SetCursor(int);
};

struct DLLBUILD TimerWin32 : Timer
{
	TimerWin32();
	virtual void update();
};

struct DLLBUILD IdeWin32 : Ide
{
	IdeWin32();
	~IdeWin32();

	HANDLE processThreadHandle;
	HANDLE processHandle;
	HANDLE projectDirChangeHandle;

	bool projectDirHasChanged;

	void Run();
	void ScanProjectDirectoryForFileChanges();

	void ScanDir(String,ResourceNodeDir*);
	void Sleep(int iMilliseconds=1);
};



struct DLLBUILD DebuggerWin32 : Debugger
{
	static DWORD WINAPI debuggeeThreadFunc(LPVOID iDebuggerWin32);

	HANDLE debuggeeThread;
	DWORD  debuggeeThreadId;

	CONTEXT*	threadContext;

	DebuggerWin32();
	~DebuggerWin32();

	void SuspendDebuggee();
	void ResumeDebuggee();

	void SetBreakpoint(Breakpoint&,bool);

	int HandleHardwareBreakpoint(void*);
	void SetHardwareBreakpoint(Breakpoint&,bool);

	void PrintThreadContext(void*);
	
};
#endif //WIN32_H

#ifndef WIN32_H
#define WIN32_H

#include "win32includes.h"




struct Direct2DGuiBase
{
	static ID2D1Factory *factory;
	static IWICImagingFactory *imager;
	static IDWriteFactory *writer;
	static IDWriteTextFormat *texter;

	static void Init(wchar_t* fontName,float fontSize);
	static void Release();

	static ID2D1HwndRenderTarget* InitHWNDRenderer(HWND hwnd);
	static ID2D1DCRenderTarget* InitDCRenderer();

	static ID2D1TransformedGeometry* CreateTransformedGeometry(ID2D1Geometry* geometry);
	static ID2D1RectangleGeometry* CreateRectangle(float x,float y, float w,float h);
	static ID2D1RoundedRectangleGeometry* CreateRoundRectangle(float x,float y, float w,float h,float rx,float ry);
	static ID2D1EllipseGeometry* CreateEllipse(float x,float y, float w,float h);
	static ID2D1PathGeometry* CreatePathGeometry();
	static ID2D1Bitmap* CreateRawBitmap(ID2D1RenderTarget*renderer,const wchar_t* fname);
	static void CreateRawBitmap(const wchar_t* fname,unsigned char*& buffer,float& width,float& height);

	static ID2D1SolidColorBrush* SolidColorBrush(ID2D1RenderTarget* renderer,float r,float g,float b,float a);
	static ID2D1LinearGradientBrush* LinearGradientBrush(ID2D1RenderTarget* renderer,float x1,float y1,float x2,float y2,float position,float r,float g,float b,float a,D2D1::Matrix3x2F mtx=D2D1::Matrix3x2F::Identity());
	static ID2D1BitmapBrush* BitmapBrush();
	static ID2D1RadialGradientBrush* RadialGradientBrush(ID2D1RenderTarget* renderer,float x1,float y1,float x2,float y2,float position,float r,float g,float b,float a,D2D1::Matrix3x2F mtx=D2D1::Matrix3x2F::Identity());

	static void Draw(ID2D1RenderTarget*renderer,ID2D1Geometry* geometry,ID2D1Brush* brush,bool filled=true);

	static void DrawRectangle(ID2D1RenderTarget*renderer,float x,float y,float w,float h,ID2D1Brush* brush,bool filled=true);
	static void DrawRoundRectangle(ID2D1RenderTarget*renderer,float x,float y,float w,float h,float rx,float ry,ID2D1Brush* brush,bool filled=true);
	static void DrawEllipse(ID2D1RenderTarget*renderer,float x,float y, float rx,float ry,ID2D1Brush* brush,bool filled=true);
	static void DrawLine(ID2D1RenderTarget*renderer,float x1,float y1, float x2,float y2,ID2D1Brush* brush);
	static void DrawBitmap(ID2D1RenderTarget*renderer,ID2D1Bitmap* bitmap,float x,float y, float w,float h);
	static void DrawText(ID2D1RenderTarget*renderer,ID2D1Brush* brush,const char* text,float x,float y, float w,float h,float ax=-1,float ay=-1);

	static bool OnSize(ID2D1Geometry* geometry,D2D1::Matrix3x2F& mtx,float x,float y);
	static bool OnSize(ID2D1Geometry* geometry,float x,float y);
};



struct WindowData
{
	HWND hwnd;
	float width;
	float height;

	std::list<WindowData*> siblings[4];

	void LinkSibling(WindowData* t,int pos);
	void UnlinkSibling(WindowData* t=0);
	WindowData* FindSiblingOfSameSize();
	int FindSiblingPosition(WindowData* t);
	bool FindAndGrowSibling();

	UINT msg;
	WPARAM wparam;
	LPARAM lparam;

	operator HWND(){return this->hwnd;}
	void CopyProcedureData(HWND  h,UINT m,WPARAM w,LPARAM l){hwnd=h,msg=m,wparam=w,lparam=l;}
	
	virtual void Create(HWND container)=0;
	

	virtual void OnSize()
	{
		width=LOWORD(lparam);
		height=HIWORD(lparam);
	}
	virtual void OnWindowPosChanging()
	{
		width=(float)((LPWINDOWPOS)lparam)->cx;
		height=(float)((LPWINDOWPOS)lparam)->cy;
	}

};

struct SplitterContainer;
struct ContainerWindow;

struct TabContainer : WindowData , TPoolVector<TabContainer>
{
	ContainerWindow* parentContainer;

	static const unsigned int COLOR_TAB_BACKGROUND=0x808080;
	static const unsigned int COLOR_TAB_SELECTED=0x0000FF;
	static const unsigned int COLOR_GUI_BACKGROUND = 0x707070;
	static const unsigned int COLOR_MAIN_BACKGROUND = 0x505050;
	static const unsigned int COLOR_TEXT=0xFFFFFF;
	static const unsigned int COLOR_TEXT_SELECTED=0x0000ff;
	static const unsigned int COLOR_TEXT_HOVERED=0x0000f1;
	

	static const int CONTAINER_HEIGHT=30;
	static const int TAB_WIDTH=80;
	static const int TAB_HEIGHT=25;

	static const int CONTAINER_ICON_WH=20;
	static const int CONTAINER_ICON_STRIDE=CONTAINER_ICON_WH*4;
	

	static unsigned char* rawUpArrow;
	static unsigned char* rawRightArrow;
	static unsigned char* rawDownArrow;
	static unsigned char* rawFolder;
	static unsigned char* rawFile;

	static LRESULT CALLBACK TabContainerWindowClassProcedure(HWND,UINT,WPARAM,LPARAM);

	ID2D1HwndRenderTarget* renderTarget;
	ID2D1SolidColorBrush*  brush;

	ID2D1Bitmap* iconUp;
	ID2D1Bitmap* iconRight;
	ID2D1Bitmap* iconDown;
	ID2D1Bitmap* iconFolder;
	ID2D1Bitmap* iconFile;

	GuiRootRect	tabs;

	SplitterContainer* splitterContainer;

	RendererInterface *renderer;
	
	int selected;
	bool mouseDown;
	bool isRender;
	bool recreateTarget;
	bool resizeTarget;

	GuiRect* drawRect;
	bool	drawFrame;
	Task*	drawTask;

	float mousex,mousey;

	int nPainted;

	bool buttonLeftMouseDown;
	bool buttonControlDown;

	unsigned int lastFrameTime;

	bool skip;

	Thread thread;

	TabContainer(float x,float y,float w,float h,HWND parent);
	~TabContainer();

	operator TabContainer& (){return *this;}
	
	void Create(HWND){}//@mic no more used, delete from WindowData

	virtual void OnGuiPaint(void* data=0);
	virtual void OnGuiSize(void* data=0);
	virtual void OnWindowPosChanging(void* data=0);
	virtual void OnGuiLMouseDown(void* data=0);
	virtual void OnGuiLMouseUp(void* data=0);
	virtual void OnGuiMouseMove(void* data=0);
	virtual void OnRMouseUp(void* data=0);
	virtual void OnGuiUpdate(void* data=0);
	virtual void OnGuiRender(void* data=0);
	virtual void OnGuiMouseWheel(void* data=0);
	virtual void OnResizeContainer(void* data=0);
	virtual void OnEntitiesChange(void* data=0);
	virtual void OnGuiActivate(void* data=0);
	virtual void OnGuiDeactivate(void* data=0);
	virtual void OnGuiEntitySelected(void* data=0);

	void RecreateTarget();
	void DrawFrame();
	

	virtual void OnGuiRecreateTarget(void* data=0);

	ID2D1Brush* SetColor(unsigned int color);

	GuiRect* GetSelected();

	void BroadcastToSelected(void (GuiRect::*func)(TabContainer*,void*),void* data=0);
	void BroadcastToAll(void (GuiRect::*func)(TabContainer*,void*),void* data=0);
	template<class C> void BroadcastToSelected(void (GuiRect::*func)(TabContainer*,void*),void*);
	template<class C> void BroadcastToAll(void (GuiRect::*func)(TabContainer*,void*),void*);
	static void BroadcastToPoolSelecteds(void (GuiRect::*func)(TabContainer*,void*),void* data=0)
	{
		for(std::vector<TabContainer*>::iterator tabContainer=TPoolVector<TabContainer>::pool.begin();tabContainer!=TPoolVector<TabContainer>::pool.end();tabContainer++)
			(*tabContainer)->BroadcastToSelected(func,data);
	}

	void Draw();

	bool BeginDraw();
	void EndDraw();


	void SetDraw(GuiRect* iRect,bool iFrame);
};

struct SplitterContainer 
{
	ContainerWindow* GetContainer(){return (ContainerWindow*)this;}

	static HMENU popupMenuRoot;
	static HMENU popupMenuCreate;

	TabContainer* currentTabContainer;

	TabContainer* floatingTabRef;
	TabContainer* floatingTab;
	TabContainer* floatingTabTarget;
	int  floatingTabRefTabIdx;
	int  floatingTabRefTabCount;
	int  floatingTabTargetAnchorPos;
	int	 floatingTabTargetAnchorTabIndex;
	RECT floatingTabRc;
	RECT floatingTabTargetRc;

	const int   splitterSize;
	LPCSTR		splitterCursor;
	POINTS splitterPreviousPos;

	std::vector<HWND> resizingWindows1;
	std::vector<HWND> resizingWindows2;
	HWND hittedWindow1;
	HWND hittedWindow2;

	SplitterContainer();
	~SplitterContainer();

	void OnLButtonDown(HWND,LPARAM);
	void OnLButtonUp(HWND);
	void OnMouseMove(HWND,LPARAM);
	void OnSize(HWND,WPARAM,LPARAM);

	void CreateFloatingTab(TabContainer*);
	void DestroyFloatingTab();

	std::vector<HWND> findWindoswAtPos(HWND mainWindow,RECT &srcRect,int rectPosition);

	void EnableChilds(HWND hwnd,int enable=-1,int show=-1);
	void EnableAllChildsDescendants(HWND hwnd,int enable=-1,int show=-1);
};


struct ContainerWindow : WindowData , SplitterContainer
{
	std::vector<TabContainer*> tabContainers;

	int ContainerWindow_currentVisibleChildIdx;

	int resizeDiffHeight;
	int resizeDiffWidth;
	int resizeEnumType;
	int resizeCheckWidth;
	int resizeCheckHeight;

	ContainerWindow();

	void Create(HWND hwnd=0);
	void OnCreate(HWND);
	void OnSizing();
	void OnSize();
	
};

struct App;

struct MainAppContainerWindow : ContainerWindow
{
	MainAppContainerWindow();

	App *application;

	static std::vector<ContainerWindow> windows;

	HMENU menuMain;
	HMENU menuEntities;

	void Create(HWND hwnd=0);
	void OnCreate(HWND);
};

struct TimerWin32 : Timer
{
	virtual void update();
	virtual unsigned int GetTime();
};

struct App : AppInterface , TStaticInstance<App>
{

	TimerWin32 timerMain;
	MainAppContainerWindow mainAppWindow;

	
	String projectFolder;

	int Init();	
	void Close();

	void CreateMainWindow();
	void Run();
};


struct RenderSurface
{
	ID2D1Bitmap* renderBitmap;
	unsigned char* renderBuffer;
	TabContainer* tab;
	unsigned int lastFrameTime;

	RenderSurface():renderBitmap(0),renderBuffer(0){}
	~RenderSurface(){SAFERELEASE(renderBitmap);SAFEDELETEARRAY(renderBuffer);}
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

struct OpenGLRenderer : RendererInterface
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

	virtual void Render(GuiViewport*,bool force=false);
	virtual void Render();
};



struct DirectXRenderer : WindowData ,  RendererInterface
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











struct GuiSceneViewer : GuiScrollRect
{
	GuiSceneViewer();
	~GuiSceneViewer();

	Entity* entityRoot;

	std::vector<Entity*> selection;

	void OnPaint(TabContainer*,void* data=0);
	void OnLMouseDown(TabContainer*,void* data=0);
	void OnEntitiesChange(TabContainer*,void* data=0);
	void OnEntitySelected(TabContainer*,void* data=0);
	void OnRecreateTarget(TabContainer*,void* data=0);
	void OnRMouseUp(TabContainer*,void* data=0);
	void OnMouseWheel(TabContainer*,void* data=0);


	bool ProcessMouseInput(vec2&,vec2&,Entity* node,Entity*& expChanged,Entity*& selChanged);
	void DrawNodes(TabContainer*,Entity*,vec2&);
	int UpdateNodes(Entity*);
	void UnselectNodes(Entity*);
	void ExpandUntil(Entity* iTarget);
};	



struct GuiEntityViewer : GuiScrollRect
{
	GuiEntityViewer();
	~GuiEntityViewer();

	Entity* entity;

	void OnActivate(TabContainer*,void* data=0);

	virtual void OnEntitySelected(TabContainer*,void* data=0);
	virtual void OnLMouseDown(TabContainer*,void* data=0);
	virtual void OnPaint(TabContainer*,void* data=0);
	virtual void OnExpandos(TabContainer*,void* data=0);
	virtual void OnMouseMove(TabContainer*,void* data=0);
	virtual void OnMouseWheel(TabContainer*,void* data=0);

	bool ProcessMouseInput(vec2&,vec2&,GuiRect* node);
	void DrawNodes(TabContainer*,GuiRect*,vec2&);
	int UpdateNodes(GuiRect*);
};




struct GuiProjectViewer : GuiRect
{
	struct ResourceNode
	{
		ResourceNode* parent;

		String fileName;

		bool selectedLeft;
		bool selectedRight;
		int level;
		bool isDir;

		ResourceNode();
		~ResourceNode();
	};

	struct ResourceNodeDir : ResourceNode
	{
		bool expanded;

		std::list<ResourceNodeDir*> dirs;
		std::list<ResourceNode*>	files;

		ResourceNodeDir();
		~ResourceNodeDir();
	};

	struct GuiDirView : GuiScrollRect
	{
		void DrawNodes(TabContainer*,ResourceNodeDir* node,vec2&,bool& terminated);
		bool ProcessMouseInput(vec2&,vec2&,float& drawFromY,ResourceNodeDir* root,ResourceNodeDir* node,ResourceNodeDir*& expChanged,ResourceNodeDir*& selChanged);
		int CalcNodesHeight(ResourceNodeDir*);
		void UnselectNodes(ResourceNodeDir*);
		std::vector<ResourceNodeDir*> selectedDirs;
		ResourceNodeDir* rootResource;

		void OnLMouseDown(TabContainer*,void* data=0);
		void OnPaint(TabContainer*,void* data=0);

	}left;

	struct GuiFileView : GuiScrollRect
	{
		void DrawNodes(TabContainer*,ResourceNodeDir* node,vec2&);
		bool ProcessMouseInput(vec2&,vec2&,float& drawFromY,ResourceNodeDir* root,ResourceNodeDir* node,ResourceNodeDir*& expChanged,ResourceNode*& selChanged);
		int CalcNodesHeight(ResourceNodeDir*);
		void UnselectNodes(ResourceNodeDir*);
		ResourceNodeDir* rootResource;
		std::vector<ResourceNodeDir*> selectedDirs;
		std::vector<ResourceNode*> selectedFiles;

		void OnLMouseDown(TabContainer*,void* data=0);
		void OnPaint(TabContainer*,void* data=0);
	}right;


	ResourceNodeDir rootResource;

	GuiProjectViewer();
	~GuiProjectViewer();

	bool lMouseDown;
	bool splitterMoving;
	
	void OnPaint(TabContainer*,void* data=0);
	void OnLMouseDown(TabContainer*,void* data=0);
	void OnLMouseUp(TabContainer*,void* data=0);
	void OnMouseMove(TabContainer*,void* data=0);
	void OnReparent(TabContainer*,void* data=0);
	void OnActivate(TabContainer*,void* data=0);

	void ScanDir(String dir);
	void CreateNodes(String dir,ResourceNodeDir*);
};
bool InitSplitter();



#endif //WIN32_H
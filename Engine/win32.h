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
	

	virtual void OnSize(){width=LOWORD(lparam),height=HIWORD(lparam);}
	virtual void OnWindowPosChanging(){width=(float)((LPWINDOWPOS)lparam)->cx,height=(float)((LPWINDOWPOS)lparam)->cy;}

};

struct SplitterContainer;
struct ContainerWindow;

struct TabContainer : WindowData , GuiInterface , TPoolVector<TabContainer>
{
	ContainerWindow* parentContainer;

	static const unsigned int COLOR_TAB_BACKGROUND=0x808080;
	static const unsigned int COLOR_TAB_SELECTED=0x0000FF;
	

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

	float mousex,mousey;

	int nPainted;

	bool buttonLeftMouseDown;
	bool buttonControlDown;

	TabContainer(float x,float y,float w,float h,HWND parent);
	~TabContainer();

	operator TabContainer& (){return *this;}
	
	void Create(HWND){}//@mic no more used, delete from WindowData

	virtual void OnGuiPaint();
	virtual void OnGuiSize();
	virtual void OnWindowPosChanging();
	virtual void OnGuiLMouseDown();
	virtual void OnGuiLMouseUp();
	virtual void OnGuiMouseMove();
	virtual void OnRMouseUp();
	virtual void OnGuiUpdate();
	virtual void OnGuiRender();
	virtual void OnGuiMouseWheel();
	virtual void OnResizeContainer();
	virtual void OnEntitiesChange();
	virtual void OnGuiActivate();
	virtual void OnGuiDeactivate();
	virtual void OnGuiEntitySelected();
	

	virtual void OnGuiRecreateTarget();

	ID2D1Brush* SetColor(unsigned int color);

	GuiRect* GetSelected();

	void BroadcastToSelected(void (GuiRect::*func)(TabContainer*));
	void BroadcastToAll(void (GuiRect::*func)(TabContainer*));
	template<class C> void BroadcastToSelected(void (GuiRect::*func)(TabContainer*));
	template<class C> void BroadcastToAll(void (GuiRect::*func)(TabContainer*));

	void BeginDraw();
	void EndDraw();
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


struct ContainerWindow : WindowData , SplitterContainer , GuiInterface
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
	void OnGuiPaint();
	
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
	void update();
	unsigned int GetTime();
};

struct App : AppInterface , TStaticInstance<App>
{

	TimerWin32 timerMain;
	MainAppContainerWindow mainAppWindow;

	bool threadLockedEntities;
	bool threadUpdateNeeded;
	bool threadPaintNeeded;
	static int threadGuiTab;
	
	String projectFolder;

	App();

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

struct OpenGLRenderer : RendererInterface , TPoolVector<OpenGLRenderer>
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

	virtual void Render(vec4 rectangle,mat4 _projection,mat4 _view,mat4 _model);
	virtual void Render(GuiViewport*,bool paint=true);
	virtual void RenderViewports();
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








struct SceneEntityNode
{
	SceneEntityNode* parent;

	Entity* entity;

	float x;
	float y;
	bool expanded;
	bool selected;
	int textWidth;
	int level;
	int hasChilds;

	std::list<SceneEntityNode> childs;

	SceneEntityNode();
	~SceneEntityNode();

	struct SceneEntityPropertyNode
	{
		SceneEntityPropertyNode* parent;

		Entity* entity;

		GuiRect root;
		
		void insert(Entity* entity,HDC hdc,float& width,float& height,SceneEntityPropertyNode* parent=0,int expandUntilLevel=1);
		void update(float& width,float& height);
		void clear();

	}properties;

	void insert(Entity* entity,HDC hdc,float& width,float& height,SceneEntityNode* parent=0,int expandUntilLevel=1);
	void update(float& width,float& height);
	void clear();
};



struct GuiEntityViewer : GuiRect
{
	GuiEntityViewer();
	~GuiEntityViewer();

	void OnActivate(TabContainer*);

	virtual void OnEntitySelected(TabContainer*);
};

struct GuiSceneViewer : GuiRect
{
	GuiSceneViewer();
	~GuiSceneViewer();

	static const int TREEVIEW_ROW_HEIGHT=20;
	static const int TREEVIEW_ROW_ADVANCE=TREEVIEW_ROW_HEIGHT;

	SceneEntityNode elements;

	GuiScrollBar scrollBar;
	
	float bitmapWidth;
	float bitmapHeight;
	float frameWidth;
	float frameHeight;

	void OnPaint(TabContainer*);
	void OnSize(TabContainer*);
	void OnLMouseDown(TabContainer*);
	void OnEntitiesChange(TabContainer*);
	void OnUpdate(TabContainer*);
	void OnReparent(TabContainer*);
	void OnRecreateTarget(TabContainer*);
	

	bool OnNodePressed(vec2&,SceneEntityNode& node,SceneEntityNode*& expChanged,SceneEntityNode*& selChanged);
	void DrawNodeSelectionRecursive(TabContainer*,SceneEntityNode& node);
	void DrawNodeRecursive(TabContainer*,SceneEntityNode&);
};	




struct GuiProjectViewer : GuiRect
{
	struct ResourceNode
	{
		ResourceNode* parent;

		String fileName;

		float x;
		float y;
		bool expanded;
		bool selected;
		int textWidth;
		int level;
		int nChilds;
		int isDir;

		std::list<ResourceNode> childsDirs;

		ResourceNode(){clear();}
		~ResourceNode(){clear();}
			
		void insertDirectory(String &path,HANDLE handle,WIN32_FIND_DATA found,HDC hdc,float& width,float& height,ResourceNode* parent=0,int expandUntilLevel=1);
		void insertFiles(GuiProjectViewer::ResourceNode& directory,HDC hdc,float& width,float& height);
		void update(float& width,float& height);
		void drawdirlist(TabContainer*,GuiProjectViewer* tv);
		void drawfilelist(TabContainer*,GuiProjectViewer* tv);
		void clear();
		ResourceNode* onmousepressedLeftPane(TabContainer*,GuiProjectViewer* tv,float& x,float& y,float& width,float& height);
		ResourceNode* onmousepressedRightPane(TabContainer*,GuiProjectViewer* tv,float& x,float& y,float& width,float& height);

		static bool ScanDir(const char* dir,HANDLE&,WIN32_FIND_DATA& data,int opt=-1);
	};

	ResourceNode leftElements;
	ResourceNode rightElements;
	std::vector<ResourceNode*> selectedDirs;
	std::vector<ResourceNode*> selectedFiles;

	GuiProjectViewer();
	~GuiProjectViewer();

	GuiScrollBar leftScrollBar;
	GuiScrollBar rightScrollBar;

	float leftBitmapWidth;
	float leftBitmapHeight;
	float rightBitmapWidth;
	float rightBitmapHeight;
	float leftFrameWidth;
	float frameHeight;
	bool lMouseDown;
	bool splitterMoving;
	

	void OnPaint(TabContainer*);
	void OnSize(TabContainer*);
	void OnLMouseDown(TabContainer*);
	void OnMouseWheel(TabContainer*);
	void OnLMouseUp(TabContainer*);
	void OnMouseMove(TabContainer*);
	void OnReparent(TabContainer*);
	void OnActivate(TabContainer*);

	void SetLeftScrollBar();
	void SetRightScrollBar();
};
bool InitSplitter();



#endif //WIN32_H
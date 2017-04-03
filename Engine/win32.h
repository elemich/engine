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
	static ID2D1Bitmap* CreateBitmap(ID2D1RenderTarget*renderer,const wchar_t* fname);
	static void CreateBitmap(ID2D1RenderTarget*renderer,const wchar_t* fname,unsigned char*& buffer);

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
	static void DrawText(ID2D1RenderTarget*renderer,ID2D1Brush* brush,const char* text,float x,float y, float w,float h);


	static bool OnSize(ID2D1Geometry* geometry,D2D1::Matrix3x2F& mtx,float x,float y);
	static bool OnSize(ID2D1Geometry* geometry,float x,float y);
};

struct WindowData
{
	HWND hwnd;
	int width;
	int height;
	
	virtual void Create(HWND container)=0;

	virtual void OnSize(LPARAM lparam){width=LOWORD(lparam),height=HIWORD(lparam);}
	virtual void OnWindowPosChanging(LPARAM lparam){width=((LPWINDOWPOS)lparam)->cx,height=((LPWINDOWPOS)lparam)->cy;}

};

struct SplitterContainer;

struct TabContainer : WindowData
{
	static LRESULT CALLBACK TabContainerWindowClassProcedure(HWND,UINT,WPARAM,LPARAM);

	static const int CONTAINER_HEIGHT=30;
	static const int CONTAINER_TAB_WIDTH=80;
	static const int CONTAINER_TAB_HEIGHT=25;
	
	ID2D1HwndRenderTarget* tabcontainer_renderer;
	ID2D1DCRenderTarget*   tabcontainer_dcRenderer;
	ID2D1SolidColorBrush*  tabcontainer_tabBackgroundBrush;
	ID2D1SolidColorBrush*  tabcontainer_backgroundBrush;
	ID2D1SolidColorBrush*  tabcontainer_tabBrush;
	ID2D1SolidColorBrush*  tabcontainer_textBrush;

	std::vector<GuiInterface*> tabcontainer_tabs;

	SplitterContainer* splitterContainer;
	
	int tabcontainer_selected;
	bool tabcontainer_mouseDown;
	bool tabcontainer_isRender;
	bool tabcontainer_recreateTarget;

	float tabcontainer_mousex,tabcontainer_mousey;

	TabContainer(int x,int y,int w,int h,HWND parent);
	~TabContainer(){};
	
	void Create(HWND){}//@mic no more used, delete from WindowData

	virtual void OnPaint();
	virtual void OnSize(LPARAM lparam);
	virtual void OnWindowPosChanging(LPARAM lparam);
	virtual void OnLMouseDown(LPARAM lparam);
	virtual void OnLMouseUp(LPARAM lparam);
	virtual void OnMouseMove(LPARAM lparam);
	virtual void OnRMouseUp(LPARAM lparam);
	virtual void OnRun();

	virtual void RecreateTarget();

	GuiInterface* AddTab(GuiInterface*,int position=-1);
	int RemoveTab(GuiInterface* tab);


};

struct SplitterContainer 
{
	std::vector<TabContainer*> containers;

	static HMENU popupMenuRoot;
	static HMENU popupMenuCreate;

	int tabContainerCount;

	TabContainer* currentTabContainer;

	TabContainer* childMovingRef;
	TabContainer* childMoving;
	TabContainer* childMovingTarget;
	int  childMovingRefTabIdx;
	int  childMovingRefTabCount;
	int  childMovingTargetAnchorPos;
	int	 childMovingTargetAnchorTabIndex;
	RECT childMovingRc;
	RECT childMovingTargetRc;

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

	HWND GetWindowBelowMouse(HWND,WPARAM,LPARAM);

	void CreateFloatingTab(TabContainer*);
	void DestroyFloatingTab();
	int OnTabContainerRButtonUp(HWND,LPARAM);
	void OnTabContainerSize(HWND);

	int GetTabSelectionIdx(HWND);
	void SetTabSelectionIdx(HWND hwnd,int idx);
	HWND GetTabSelectionWindow(HWND hwnd);


	RECT GetTabContainerClientSize(HWND);
	void EnableAndShowTabContainerChild(HWND hwnd,int idx);


	std::vector<HWND> findWindoswAtPos(HWND mainWindow,RECT &srcRect,int rectPosition);
	//std::vector<HWND> findAttachedWindos(HWND mainWindow,RECT &srcRect,int rectPosition);

	HWND CreateTabContainer(int x,int y,int w,int h,HWND parent);
	int	 CreateTabChildren(HWND parent,char* text=0,int pos=-1,HWND from=0);
	void RemoveTabChildren(HWND hwnd,int idx);
	void ReparentTabChildren(HWND src,HWND dst,int idx);
	void EnableChilds(HWND hwnd,int enable=-1,int show=-1);
	void EnableAllChildsDescendants(HWND hwnd,int enable=-1,int show=-1);

	bool CreateNewPanel(HWND,int popupMenuItem);
};


struct ContainerWindow : WindowData , SplitterContainer
{
	int ContainerWindow_currentVisibleChildIdx;

	ContainerWindow();

	void Create(HWND hwnd=0);
	void OnCreate(HWND);
};

struct MainAppContainerWindow : ContainerWindow
{
	MainAppContainerWindow();

	static std::vector<ContainerWindow> windows;

	HMENU menuMain;
	HMENU menuEntities;

	void Create(HWND hwnd=0);
	void OnCreate(HWND);
};


/*



struct ProjectFolderBrowser : WindowData , FolderBrowserInterface
{
	IExplorerBrowser* browser;
		
	ProjectFolderBrowser();
	~ProjectFolderBrowser();

	PIDLIST_ABSOLUTE SelectProjectFolder();

	void Create(HWND container);
};

struct ProjectFolderBrowser2 : WindowData , FolderBrowserInterface
{
	INameSpaceTreeControl2* browser;

	ProjectFolderBrowser2();
	~ProjectFolderBrowser2();

	PIDLIST_ABSOLUTE SelectProjectFolder();

	void Create(HWND container);
};

struct SceneEntities : WindowData , SceneEntitiesInterface
{
	std::vector<HTREEITEM> items;

	SceneEntities();
	~SceneEntities();

	SceneEntities* GetSceneEntities(){return this;}

	void Create(HWND container);

	void Fill();
	void Expand();

	virtual void OnEntitiesChange(){Fill();Expand();}
};


struct EntityProperty : WindowData , PropertyInterface
{
	

	void Create(HWND container);
};

struct Logger : WindowData , LoggerInterface
{
	void Create(HWND container);
};
*/

struct App : AppInterface
{
	MainAppContainerWindow mainAppWindow;
	
	PIDLIST_ABSOLUTE projectFolder;

	App();

	int Init();
	void Close();

	void CreateMainWindow();
	void Run();


	
};

struct OpenGLRenderer : RendererInterface , RendererViewportInterface
{
	static GLuint vertexArrayObject;
	static GLuint vertexBufferObject;
	static GLuint textureBufferObject;
	static GLuint indicesBufferObject;
	static GLuint frameBuffer;
	static GLuint textureColorbuffer;
	static GLuint textureRenderbuffer;
	static GLuint pixelBuffer;
	static GLuint renderBufferColor;
	static GLuint renderBufferDepth;

	unsigned char* buffer;

	HGLRC hglrc;
	HDC sourceDc;
	HDC   hdc;
	HBITMAP hbitmap;

	int width;
	int height;

#if USE_MULTIPLE_OPENGL_CONTEXTS
	GLEWContext* glewContext;
#endif

	std::vector<OpenGLRenderer*> shared_renderers;

	ID2D1BitmapRenderTarget* openglrenderer_bitmapRenderTarget;
	ID2D1Bitmap* openglrenderer_bitmap;

	OpenGLRenderer(TabContainer*);

	virtual void Create(HWND container);

	//OpenGLRenderer* CreateSharedContext(HWND container);

	char* Name();
	void Render();
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

	/*operator RendererInterface&(){return *this;}
	operator OpenGLRenderer&(){return *this;}
	operator RendererViewportInterface&(){return *this;}*/

	
	void OnMouseWheel(float);
	void OnMouseRightDown();
	void OnViewportSize(int width,int height);
	void OnMouseMotion(int x,int y,bool leftButtonDown,bool altIsDown);
	void OnMouseDown(int,int);


	void OnPaint();
	void OnSize();
	void OnLMouseDown();
	void OnEntitiesChange();
	void OnRun();
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


struct ScrollBar
{
	static const int SCROLLBAR_WIDTH=20;
	static const int SCROLLBAR_TIP_HEIGHT=SCROLLBAR_WIDTH;
	static const int SCROLLBAR_AMOUNT=10;

};

struct TreeView : GuiInterface
{
	TreeView(TabContainer*);
	~TreeView();

	static const int TREEVIEW_ROW_HEIGHT=20;
	static const int TREEVIEW_ROW_ADVANCE=TREEVIEW_ROW_HEIGHT;

	

	struct TVNODE
	{
		TVNODE* parent;

		Entity* entity;
		float x;
		float y;
		bool expanded;
		bool selected;
		float textWidth;
		int level;
		int hasChilds;

		std::list<TVNODE> childs;

		TVNODE();
		~TVNODE();	

		static void insert(TVNODE& node,Entity* entity,HDC hdc,float& width,float& height,TVNODE* parent=0,int expandUntilLevel=1);
		static void update(TVNODE&,float& width,float& height);
		static void drawlist(TVNODE&,TreeView* tv);
		static void drawselection(TVNODE&,TreeView* tv);
		static void clear(TVNODE&);
		static bool onmousepressed(TVNODE& node,TreeView* tv,float& x,float& y,float& width,float& height);
	};

	TVNODE elements;

	ID2D1BitmapRenderTarget* treeview_bitmapRenderTarget;
	ID2D1Bitmap* treeview_bitmap;

	static unsigned char* rightArrow;
	static unsigned char* downArrow;

	ID2D1Bitmap* treeview_rightArrow;
	ID2D1Bitmap* treeview_downArrow;
	
	float treeview_tvWidth;
	float treeview_tvHeight;
	int treeview_scroll;
	int treeview_framex;
	int treeview_framey;

	void OnPaint();
	void OnSize();
	void OnLMouseDown();
	void OnEntitiesChange();
	void OnRun();
	void OnReparent();

	void DrawItems();

	void RecreateTarget();

	ID2D1SolidColorBrush *colorGray;
	ID2D1SolidColorBrush *colorGraySel;
	ID2D1SolidColorBrush *m_pFeatureBrush;
	ID2D1SolidColorBrush *m_pFeatureBrushSelection;
	ID2D1SolidColorBrush *m_pTextBrush;
};


bool InitSplitter();



#endif //WIN32_H
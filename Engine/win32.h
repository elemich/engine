#ifndef WIN32_H
#define WIN32_H

#include "win32includes.h"

#define D2DRELEASE(_ptr) \
	if(_ptr!=0)\
	{\
		printf("releasing d2d ptr %p\n",_ptr);\
		_ptr->Release();\
		_ptr=0;\
	}\

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
	static void CreateRawBitmap(const wchar_t* fname,unsigned char*& buffer);

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
	float width;
	float height;

	UINT msg;
	WPARAM wparam;
	LPARAM lparam;
	
	virtual void Create(HWND container)=0;

	virtual void OnSize(LPARAM lparam){width=LOWORD(lparam),height=HIWORD(lparam);}
	virtual void OnWindowPosChanging(LPARAM lparam){width=(float)((LPWINDOWPOS)lparam)->cx,height=(float)((LPWINDOWPOS)lparam)->cy;}

};

struct SplitterContainer;

struct TabContainer : WindowData , TClassPool<TabContainer>
{
	std::vector<TabContainer*> siblings[4];

	void LinkSibling(TabContainer* t,int pos)
	{
		if(!t)
			return;

		int reciprocal = pos<3 ? pos+2 : pos-2;

		this->siblings[pos].push_back(t);
		t->siblings[reciprocal].push_back(this);
	}

	void UnlinkSibling(TabContainer* t)
	{
		for(int i=0;i<4;i++)
		{
			this->siblings[i].erase(std::find(this->siblings[i].begin(),this->siblings[i].end(),t));
			t->siblings[i].erase(std::find(t->siblings[i].begin(),t->siblings[i].end(),this));
		}
	}

	static const unsigned int COLOR_TAB_BACKGROUND=0x808080;
	static const unsigned int COLOR_TAB_SELECTED=0x0000FF;
	static const unsigned int COLOR_TEXT=0xFFFFFF;

	static const int CONTAINER_HEIGHT=30;
	static const int TAB_WIDTH=80;
	static const int TAB_HEIGHT=25;

	static const int CONTAINER_ARROW_WH=20;
	static const int CONTAINER_ARROW_STRIDE=CONTAINER_ARROW_WH*4;
	

	static unsigned char* rawUpArrow;
	static unsigned char* rawRightArrow;
	static unsigned char* rawDownArrow;
	static unsigned char* rawFolder;

	static LRESULT CALLBACK TabContainerWindowClassProcedure(HWND,UINT,WPARAM,LPARAM);

	ID2D1HwndRenderTarget* renderer;
	ID2D1SolidColorBrush*  brush;

	ID2D1Bitmap* upExpandos;
	ID2D1Bitmap* rightExpandos;
	ID2D1Bitmap* downExpandos;
	ID2D1Bitmap* folderIcon;

	std::vector<Gui*> tabs;

	SplitterContainer* splitterContainer;
	
	int selected;
	bool mouseDown;
	bool isRender;
	bool recreateTarget;

	float mousex,mousey;

	TabContainer(float x,float y,float w,float h,HWND parent);
	~TabContainer();
	
	void Create(HWND){}//@mic no more used, delete from WindowData

	virtual void OnPaint();
	virtual void OnSize(LPARAM lparam);
	virtual void OnWindowPosChanging(LPARAM lparam);
	virtual void OnLMouseDown(LPARAM lparam);
	virtual void OnLMouseUp(LPARAM lparam);
	virtual void OnMouseMove(LPARAM lparam);
	virtual void OnRMouseUp(LPARAM lparam);
	virtual void OnRun();
	virtual void OnMouseWheel();

	virtual void RecreateTarget();

	

	Gui* AddTab(Gui*,int position=-1);
	int RemoveTab(Gui* tab);

	ID2D1Brush* SetColor(unsigned int color){brush->SetColor(D2D1::ColorF(color));return brush;}

	Gui* GetSelected()
	{
		return tabs.size() ? tabs[selected] : 0;
	}
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

struct App : AppInterface , TStaticClass<App>
{
	MainAppContainerWindow mainAppWindow;
	
	String projectFolder;

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
	float GetProjectionHalfWidth();
	float GetProjectionHalfHeight();


	void OnSize();
	void OnLMouseDown();
	void OnMouseMove();
	void OnEntitiesChange();
	void OnRun();
	void OnRender();
	void OnPaint();
	void OnMouseWheel();
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

struct ScrollBar : GuiInterface<ScrollBar>
{
	static const int SCROLLBAR_WIDTH=20;
	static const int SCROLLBAR_TIP_HEIGHT=SCROLLBAR_WIDTH;
	static const int SCROLLBAR_AMOUNT=10;

	float x,y,width,height;
	float scroller,scrollerFactor;
	float scrollerClick;

	ScrollBar(TabContainer*);
	~ScrollBar();

	void Set(float _x,float _y,float _width,float _height);
	void SetScrollerFactor(float contentHeight,float containerHeight);
	void SetScrollerPosition(float contentHeight);
	float GetScrollValue();

	float OnPressed();
	void OnReleased();
	bool OnScrolled();
	void OnPaint();
};



struct TreeView : GuiInterface<TreeView>
{
	TreeView(TabContainer*);
	~TreeView();

	static const int TREEVIEW_ROW_HEIGHT=20;
	static const int TREEVIEW_ROW_ADVANCE=TREEVIEW_ROW_HEIGHT;

	struct TreeViewNode
	{
		TreeViewNode* parent;

		Entity* entity;

		float x;
		float y;
		bool expanded;
		bool selected;
		int textWidth;
		int level;
		int hasChilds;

		std::list<TreeViewNode> childs;

		TreeViewNode();
		~TreeViewNode();

		void insert(Entity* entity,HDC hdc,float& width,float& height,TreeViewNode* parent=0,int expandUntilLevel=1);
		void update(float& width,float& height);
		void drawlist(TreeView* tv);
		void drawselection(TreeView* tv);
		void clear();
		TreeViewNode* onmousepressed(TreeView* tv,float& x,float& y,float& width,float& height);
		std::vector<Entity*> getselection();
	};

	TreeViewNode elements;

	ID2D1BitmapRenderTarget* bitmaprenderer;
	ID2D1Bitmap* bitmap;

	ScrollBar scrollBar;
	
	float bitmapWidth;
	float bitmapHeight;
	float frameWidth;
	float frameHeight;

	void OnPaint();
	void OnSize();
	void OnLMouseDown();
	void OnEntitiesChange();
	void OnRun();
	void OnReparent();

	void DrawItems();

	void RecreateTarget();

};



struct Properties : GuiInterface<Properties>
{
	struct PropertiesNode
	{
		PropertiesNode* parent;

		Entity* entity;

		struct PanelData
		{
			String name;
			ID2D1Bitmap* bitmap;
			bool expanded;

			PanelData(const char* nm):name(nm),bitmap(0),expanded(0){}
		};

		std::vector<PanelData> panels;

		std::list<PropertiesNode> childs;
		
		PropertiesNode(){clear();}
		~PropertiesNode(){clear();}

		void insert(Entity* entity,HDC hdc,float& width,float& height,PropertiesNode* parent=0,int expandUntilLevel=1);

		void update(float& width,float& height);

		void draw(Properties* tv);
		void drawselection(Properties* tv);
		void clear();
		PropertiesNode* onmousepressed(Properties* tv,float& x,float& y,float& width,float& height);
	};

	PropertiesNode elements;
	

	Properties(TabContainer* tc);
	~Properties();

	void OnPaint();
	void OnSize(){};
	void OnLMouseDown(){};
	void OnEntitiesChange();
	void OnEntitySelected(){};
	void OnRun(){};
	void OnReparent(){};

	void DrawItems();

	void RecreateTarget(){}
};


struct Resources : GuiInterface<Resources>
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
		void insertFiles(Resources::ResourceNode& directory,HDC hdc,float& width,float& height);
		void update(float& width,float& height);
		void drawdirlist(Resources* tv);
		void drawfilelist(Resources* tv);
		void drawdirselection(Resources* tv);
		void drawfileselection(Resources* tv);
		void clear();
		ResourceNode* onmousepressedLeftPane(Resources* tv,float& x,float& y,float& width,float& height);
		ResourceNode* onmousepressedRightPane(Resources* tv,float& x,float& y,float& width,float& height);

		static bool ScanDir(const char* dir,HANDLE&,WIN32_FIND_DATA& data,int opt=-1);
	};

	ResourceNode leftElements;
	ResourceNode rightElements;
	std::vector<ResourceNode*> selectedDirs;
	std::vector<ResourceNode*> selectedFiles;

	Resources(TabContainer* tc);
	~Resources();

	ID2D1BitmapRenderTarget* leftbitmaprenderer;
	ID2D1BitmapRenderTarget* rightbitmaprenderer;
	ID2D1Bitmap* leftBitmap;
	ID2D1Bitmap* rightBitmap;

	ScrollBar leftScrollBar;
	ScrollBar rightScrollBar;

	float leftBitmapWidth;
	float leftBitmapHeight;
	float rightBitmapWidth;
	float rightBitmapHeight;
	float leftFrameWidth;
	float frameHeight;
	bool lMouseDown;
	bool splitterMoving;
	

	void OnPaint();
	void OnSize();
	void OnLMouseDown();
	void OnMouseWheel();
	void OnLMouseUp();
	void OnMouseMove();
	void OnEntitiesChange();
	void OnRun();
	void OnReparent();

	void DrawItems();
	void DrawLeftItems();
	void DrawRightItems();

	void RecreateTarget();

	void SetLeftScrollBar();
	void SetRightScrollBar();
};
bool InitSplitter();



#endif //WIN32_H
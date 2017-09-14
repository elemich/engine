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

	operator HWND(){return this->hwnd;}
	void CopyProcedureData(HWND  h,UINT m,WPARAM w,LPARAM l){hwnd=h,msg=m,wparam=w,lparam=l;}
	
	virtual void Create(HWND container)=0;
	

	virtual void OnSize(){width=LOWORD(lparam),height=HIWORD(lparam);}
	virtual void OnWindowPosChanging(){width=(float)((LPWINDOWPOS)lparam)->cx,height=(float)((LPWINDOWPOS)lparam)->cy;}

};

struct SplitterContainer;
struct ContainerWindow;

struct TabContainer : WindowData , GuiInterface , TClassPool<TabContainer>
{
	ContainerWindow* parentContainer;

	std::list<TabContainer*> siblings[4];

	void LinkSibling(TabContainer* t,int pos);
	void UnlinkSibling(TabContainer* t=0);
	TabContainer* FindSiblingOfSameSize();
	int FindSiblingPosition(TabContainer* t);
	bool FindAndGrowSibling();

	
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

	ID2D1HwndRenderTarget* renderer;
	ID2D1SolidColorBrush*  brush;

	ID2D1Bitmap* iconUp;
	ID2D1Bitmap* iconRight;
	ID2D1Bitmap* iconDown;
	ID2D1Bitmap* iconFolder;
	ID2D1Bitmap* iconFile;

	std::vector<GuiTab*> tabs;

	SplitterContainer* splitterContainer;
	
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

	virtual void OnPaint();
	virtual void OnSize();
	virtual void OnWindowPosChanging();
	virtual void OnLMouseDown();
	virtual void OnLMouseUp();
	virtual void OnMouseMove();
	virtual void OnRMouseUp();
	virtual void OnUpdate();
	virtual void OnMouseWheel();
	virtual void OnResizeContainer();

	virtual void OnRecreateTarget();

	
	GuiTab* AddTab(GuiTab*,int position=-1);
	int RemoveTab(GuiTab* tab);

	ID2D1Brush* SetColor(unsigned int color)
	{
		brush->SetColor(D2D1::ColorF(color));
		return brush;
	}

	GuiTab* GetSelected()
	{
		return tabs.size() ? tabs[selected] : 0;
	}

	void SelectTab();

	void BroadcastToSelected(void (GuiTab::*func)());

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

struct OpenGLRenderer : RendererInterface , RendererViewportInterface , TClassPool<OpenGLRenderer>
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

	unsigned char* renderBuffer;

	HGLRC hglrc;
	HDC   hdc;

	float width;
	float height;

#if USE_MULTIPLE_OPENGL_CONTEXTS
	GLEWContext* glewContext;
#endif

	std::vector<OpenGLRenderer*> shared_renderers;

	ID2D1BitmapRenderTarget* bitmapTarget;
	ID2D1Bitmap* renderBitmap;

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
	void OnMouseMotion(float x,float y,bool leftButtonDown,bool altIsDown);
	void OnMouseDown(float,float);
	float GetProjectionHalfWidth();
	float GetProjectionHalfHeight();


	void OnSize();
	void OnLMouseDown();
	void OnMouseMove();
	void OnEntitiesChange();
	void OnUpdate();
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

	struct SceneEntityPropertyNode
	{
		SceneEntityPropertyNode* parent;

		Entity* entity;

		GuiTabElement root;

		SceneEntityPropertyNode(){clear();}
		~SceneEntityPropertyNode(){clear();}

		void insert(Entity* entity,HDC hdc,float& width,float& height,SceneEntityPropertyNode* parent=0,int expandUntilLevel=1);
		void update(float& width,float& height);
		void clear();

	}properties;

	SceneEntityNode();
	~SceneEntityNode();

	void insert(Entity* entity,HDC hdc,float& width,float& height,SceneEntityNode* parent=0,int expandUntilLevel=1);
	void update(float& width,float& height);
	void clear();
};



struct Properties : GuiTab
{
	Properties(TabContainer* tc);
	~Properties();

	void OnPaint();
	void OnSelected();
};

struct SceneViewer : GuiTab
{
	SceneViewer(TabContainer*);
	~SceneViewer();

	static const int TREEVIEW_ROW_HEIGHT=20;
	static const int TREEVIEW_ROW_ADVANCE=TREEVIEW_ROW_HEIGHT;

	SceneEntityNode elements;

	ScrollBar scrollBar;
	
	float bitmapWidth;
	float bitmapHeight;
	float frameWidth;
	float frameHeight;

	void OnPaint();
	void OnSize();
	void OnLMouseDown();
	void OnEntitiesChange();
	void OnUpdate();
	void OnReparent();
	void OnRecreateTarget();

	SceneEntityNode* OnNodePressed(SceneEntityNode& node);
	void DrawNodeSelectionRecursive(SceneEntityNode& node);
	void DrawNodeRecursive(SceneEntityNode&);
};


struct EqSolver : GuiTab
{
	mat4 mat;
	mat4 sol;

	EqSolver(TabContainer* tab):
		GuiTab(tab)
	{
		name="EqSolver";

		this->guiTabRootElement.CreateTabElementRow(0,"Equation System",String("0,0,0,0\n0,0,0,0\n0,0,0,0\n0,0,0,0"));
	}


};



struct Resources : GuiTab
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
	void OnUpdate();
	void OnReparent();

	void DrawItems();
	void DrawLeftItems();
	void DrawRightItems();

	void OnRecreateTarget();

	void SetLeftScrollBar();
	void SetRightScrollBar();
};
bool InitSplitter();



#endif //WIN32_H
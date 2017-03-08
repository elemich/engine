#ifndef WIN32_H
#define WIN32_H

#include "win32includes.h"

struct WindowData
{
	HWND hwnd;

	virtual void Create(HWND container)=0;

};

struct SplitterContainer 
{
	static HMENU popupMenuRoot;
	static HMENU popupMenuCreate;

	int tabContainerCount;

	HWND childMovingRef;
	HWND childMoving;
	HWND childMovingTarget;
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

	void OnTabContainerLButtonDown(HWND);
	void OnTabContainerLButtonUp(HWND);
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

	HMENU menuMain;
	HMENU menuEntities;

	void Create(HWND hwnd=0);
	void OnCreate(HWND);
};



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

struct App : AppInterface
{
	MainAppContainerWindow mainAppWindow;
	
	PIDLIST_ABSOLUTE projectFolder;

	App();

	int Init();

	void CreateMainWindow();
	void AppLoop();
};

struct OpenGLRenderer : WindowData ,  RendererInterface , RendererViewportInterface
{
	static GLuint vertexArrayObject;
	static GLuint vertexBufferObject;
	static GLuint textureBufferObject;
	static GLuint indicesBufferObject;

	HGLRC hglrc;
	HDC   hdc;

	

#if USE_MULTIPLE_OPENGL_CONTEXTS
	GLEWContext* glewContext;
#endif

	std::vector<OpenGLRenderer*> shared_renderers;

	OpenGLRenderer();

	virtual void Create(HWND container);

	OpenGLRenderer* CreateSharedContext(HWND container);

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


bool InitSplitter();



#endif //WIN32_H
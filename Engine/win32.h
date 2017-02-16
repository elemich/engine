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

	WINDOWNODE* MainNode;
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

	std::vector<HWND> vwin1;
	std::vector<HWND> vwin2;
	HWND win1;
	HWND win2;

	SplitterContainer();
	~SplitterContainer();

	
	void OnLButtonDown(HWND,LPARAM);
	void OnLButtonUp(HWND);
	void OnMouseMove(HWND,LPARAM);

	void OnTabContainerLButtonDown(HWND);
	void OnTabContainerLButtonUp(HWND);
	int OnTabContainerRButtonUp(HWND,LPARAM);

};



struct ContainerWindow : WindowData , SplitterContainer
{
	ContainerWindow();

	void Create(HWND hwnd=0);
	void OnCreate(HWND);
};

struct MainAppContainerWindow : ContainerWindow
{
	MainAppContainerWindow();

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

struct Logger : WindowData , LoggerInterface
{
	void Create(HWND container);
};

struct App : AppInterface
{
	MainAppContainerWindow mainAppWindow;
	
	std::vector<ContainerWindow> containers;
	std::vector<RendererInterface*> renderers;
	std::vector<BROWSER*> browsers;
	std::vector<LoggerInterface*> loggers;

	PIDLIST_ABSOLUTE projectFolder;

	App();

	int Init();

	void CreateMainWindow();
	void AppLoop();
};

struct OpenGLFixedRenderer : WindowData ,  RendererInterface
{
	HGLRC hglrc;
	HDC   hdc;
	PIXELFORMATDESCRIPTOR pfd;
	int pixelFormat;

	OpenGLFixedRenderer();

	char* Name();
	void  Create(HWND container);
	void Render();
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
};


bool InitSplitter();

HWND CreateTabContainer(int x,int y,int w,int h,HWND parent,int currentCount);
int CreateTab(HWND parent,char* text=0,int pos=-1,HWND from=0);
void RemoveTab(HWND hwnd,int idx);
void ReparentTabChild(HWND src,HWND dst,int idx);
RECT GetTabClientSize(HWND);
void OnTabSizeChanged(HWND);
void EnableChilds(HWND hwnd,int enable=-1,int show=-1);
void EnableAllChildsDescendants(HWND hwnd,int enable=-1,int show=-1);
void EnableAndShowContainerChild(HWND hwnd,int idx);
void CreateProjectFolder(HWND);

void CreateProjectFolder(HWND hwnd);
void CreateOpenglWindow(HWND hwnd);
void CreateLogger(HWND hwnd);

#endif //WIN32_H
#ifndef WIN32_H
#define WIN32_H

#include "interfaces.h"

#include "datatypes.h"

#include <windows.h>
#include <windowsx.h>

#include <winuser.h>
#include <wingdi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <objbase.h>
#include <CommCtrl.h>

#include <gl/GL.h>
#include <d3d11.h>

#define SPLITTER_DEBUG true
#define BROWSER_DEBUG true
#define TMP_DEBUG true

//#define RENDERER DirectXRenderer()
#define RENDERER OpenGLFixedRenderer
#define BROWSER  ProjectFolderBrowser2

#define WC_MAINAPPWINDOW "MainAppWindow"
#define WC_TABCONTAINER  "TabContainer"
#define WC_OPENGLWINDOW  "OpenGLWindow"

LRESULT CALLBACK MainWindowProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);
LRESULT CALLBACK TabProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);
LRESULT CALLBACK OpenGLProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);

bool InitSplitter();

HWND CreateTabContainer(int x,int y,int w,int h);
int CreateTab(HWND parent,char* text=0,int pos=-1,HWND from=0);

struct ProjectFolderBrowser
{
	IExplorerBrowser* browser;
		
	HWND hwnd;

	ProjectFolderBrowser();
	~ProjectFolderBrowser();

	PIDLIST_ABSOLUTE SelectProjectFolder();

	void Create(HWND container);
};

struct ProjectFolderBrowser2
{
	INameSpaceTreeControl2* browser;


	HWND hwnd;

	ProjectFolderBrowser2();
	~ProjectFolderBrowser2();

	PIDLIST_ABSOLUTE SelectProjectFolder();

	void Create(HWND container);
};



struct App : AppInterface
{
	HWND hwnd;
	

	std::vector<RendererInterface*> renderers;
	std::vector<BROWSER*> browsers;

	PIDLIST_ABSOLUTE projectFolder;

	App();

	int Init();

	void CreateMainWindow();
	void AppLoop();
};



struct OpenGLFixedRenderer : RendererInterface
{
	HWND hwnd;
	HGLRC hglrc;
	HDC   hdc;
	PIXELFORMATDESCRIPTOR pfd;
	int pixelFormat;

	OpenGLFixedRenderer();

	char* Name();
	void  Create(HWND container);
	void Render();
};

struct DirectXRenderer : RendererInterface
{
	HWND					hwnd;
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


#endif //WIN32_H
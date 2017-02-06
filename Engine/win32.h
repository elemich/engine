#ifndef WIN32_H
#define WIN32_H

#include "interfaces.h"

#include "win32helper.h"

#include <gl/GL.h>
#include <d3d11.h>



struct ProjectFolderBrowser
{
	IExplorerBrowser* browser;
	

	HWND hwnd;

	ProjectFolderBrowser();
	~ProjectFolderBrowser();

	PIDLIST_ABSOLUTE SelectProjectFolder();

	void Create();
};

struct ProjectFolderBrowser2
{
	INameSpaceTreeControl2* browser;


	HWND hwnd;

	ProjectFolderBrowser2();
	~ProjectFolderBrowser2();

	PIDLIST_ABSOLUTE SelectProjectFolder();

	void Create();
};



struct App : AppInterface
{
	HWND hwnd;
	MSG msg;
	RendererInterface* renderer;
	ProjectFolderBrowser2 browser;
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
	void  Init();
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
	virtual void  Init();
	virtual void Render();
};


#endif //WIN32_H
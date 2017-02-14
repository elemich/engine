
#include "win32.h"


#define PRINT(x) #x
#define PRINTF(x) PRINT(x)

#define LOCATION "@mic (" __FILE__ " : " PRINTF(__LINE__) ")"

#ifndef WINVER
#define WINVER 0x0601
#pragma message (LOCATION " WINVER: " PRINTF(WINVER))
#endif


#pragma message("@mic \"LNK1123: Failure during conversion to COFF: file invalid or corrupt\" was resolved renaming cvtres.exe to cvtres1.exe.")
//#pragma message("@mic \"LNK1123: Failure during conversion to COFF: file invalid or corrupt\" was resolved renaming cvtres.exe to cvtres1.exe.")

#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")






App* ___app;







//--------------------ProjectFolderBrowser-------------------------


ProjectFolderBrowser::ProjectFolderBrowser()
{
	browser=0;
}
ProjectFolderBrowser::~ProjectFolderBrowser()
{

}

PIDLIST_ABSOLUTE ProjectFolderBrowser::SelectProjectFolder()
{
	char _pszDisplayName[MAX_PATH];

	BROWSEINFO bi={0};
	bi.pszDisplayName=_pszDisplayName;
	bi.lpszTitle="Select Project Directory";

	return SHBrowseForFolder(&bi); 
}



void ProjectFolderBrowser::Create(HWND container)
{
	PIDLIST_ABSOLUTE projectFolder=SelectProjectFolder();

	if(!projectFolder)
		return;
	
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (!SUCCEEDED(hr))
		__debugbreak();

	hr = CoCreateInstance(CLSID_ExplorerBrowser, NULL, CLSCTX_INPROC,IID_PPV_ARGS(&browser));
	if (!SUCCEEDED(hr))
		__debugbreak();

	FOLDERSETTINGS fs = {0};
	fs.ViewMode = FVM_DETAILS;
	fs.fFlags = FWF_AUTOARRANGE;

	RECT tabcontrol_rectangle={50,50,101,101};//WHGetClientRect(container);

	hr = browser->Initialize(container, &tabcontrol_rectangle, &fs);

	if (!SUCCEEDED(hr))
		__debugbreak();

	browser->SetOptions(EBO_NOWRAPPERWINDOW);
	browser->BrowseToIDList(projectFolder,SBSP_DEFBROWSER);

	hr=IUnknown_GetWindow(browser,&hwnd);

	if (!SUCCEEDED(hr))
		__debugbreak();

	if(BROWSER_DEBUG)
		printf("browser window %p\n",hwnd);
}

ProjectFolderBrowser2::ProjectFolderBrowser2()
{
	browser=0;
}
ProjectFolderBrowser2::~ProjectFolderBrowser2()
{

}

PIDLIST_ABSOLUTE ProjectFolderBrowser2::SelectProjectFolder()
{
	char _pszDisplayName[MAX_PATH];

	BROWSEINFO bi={0};
	bi.pszDisplayName=_pszDisplayName;
	bi.lpszTitle="Select Project Directory";

	return SHBrowseForFolder(&bi); 
}



void ProjectFolderBrowser2::Create(HWND container)
{
	PIDLIST_ABSOLUTE projectFolder=SelectProjectFolder();

	if(!projectFolder)
		return;

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (!SUCCEEDED(hr))
		__debugbreak();

	hr = CoCreateInstance(CLSID_NamespaceTreeControl, NULL, CLSCTX_INPROC,IID_PPV_ARGS(&browser));
	if (!SUCCEEDED(hr))
		__debugbreak();

	browser->SetControlStyle(NSTCS_HASEXPANDOS|NSTCS_SPRINGEXPAND|NSTCS_ROOTHASEXPANDO,NSTCS_HASEXPANDOS|NSTCS_SPRINGEXPAND);

	const NSTCSTYLE style = NSTCS_HASEXPANDOS |            // Show expandos
		NSTCS_ROOTHASEXPANDO |         // Root nodes have expandos
		NSTCS_FADEINOUTEXPANDOS |      // Fade-in-out based on focus
		NSTCS_NOINFOTIP |              // Don't show infotips
		NSTCS_ALLOWJUNCTIONS |         // Show folders such as zip folders and libraries
		NSTCS_SHOWSELECTIONALWAYS |    // Show selection when NSC doesn't have focus
		NSTCS_FULLROWSELECT;           // Select full width of item

	RECT tabcontrol_rectangle={50,50,100,100};//WHGetClientRect(container);

	hr = browser->Initialize(container, &tabcontrol_rectangle, style);
	if (!SUCCEEDED(hr))
		__debugbreak();

	IShellItem* shellItem;
	hr=SHCreateShellItem(0,0,projectFolder,&shellItem);
	if (!SUCCEEDED(hr))
		__debugbreak();

	browser->AppendRoot(shellItem, SHCONTF_CHECKING_FOR_CHILDREN|SHCONTF_FOLDERS, NSTCRS_HIDDEN | NSTCRS_EXPANDED, NULL); // ignore result
	
	
	hr=IUnknown_GetWindow(browser,&hwnd);

	if (!SUCCEEDED(hr))
		__debugbreak();

	if(BROWSER_DEBUG)
		printf("browser window %p\n",hwnd);

	
}




//--------------------AppData-------------------------


App::App()
{
	___app=this;

}

int App::Init()
{
	int error=ERROR_SUCCESS;

	INITCOMMONCONTROLSEX iccex={0};
	iccex.dwSize=sizeof(INITCOMMONCONTROLSEX);
	iccex.dwICC=ICC_STANDARD_CLASSES|ICC_TREEVIEW_CLASSES|ICC_LISTVIEW_CLASSES|ICC_TAB_CLASSES;

	if(!InitCommonControlsEx(&iccex))
	{
		printf("cannot initialize common controls!\n");
		error=-1;
	}

	InitSplitter();

	this->CreateMainWindow();

	return error;
}

void App::CreateMainWindow()
{
	hwnd=CreateWindow(WC_MAINAPPWINDOW,WC_MAINAPPWINDOW,WS_OVERLAPPEDWINDOW|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,0,0,0,0);

	//browser.Create();

}


void App::AppLoop()
{
#pragma message (LOCATION " @mic: PeekMessage has 0 as hwnd to let other windows work")

	MSG msg;

	while(WM_CLOSE != PeekMessage(&msg,0,0,0,PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}







//--------------------OpenGLRenderer-------------------------

LRESULT CALLBACK OpenGLProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	OpenGLFixedRenderer* renderer=(OpenGLFixedRenderer*)GetWindowLong(hwnd,GWL_USERDATA);

	LRESULT result=0;

	if(renderer)
		renderer->Render();

	switch(msg)
	{
		case WM_ERASEBKGND:
			return (LRESULT)1;
		default:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
	}

	return result;
}


OpenGLFixedRenderer::OpenGLFixedRenderer()
{
	hglrc=0;
}

char* OpenGLFixedRenderer::Name()
{
	return 0;
}

void OpenGLFixedRenderer::Create(HWND container)
{
	hwnd=CreateWindow(WC_OPENGLWINDOW,"OpenGLFixedRenderer",WS_CHILD,CW_USEDEFAULT,CW_USEDEFAULT,100,100,container,0,0,0);
	
	hdc=GetDC(hwnd);

	if(!hdc)
		MessageBox(0,"Getting Device Context","GetDC",MB_OK|MB_ICONEXCLAMATION);

	PIXELFORMATDESCRIPTOR _pfd=         
	{
		sizeof(PIXELFORMATDESCRIPTOR), 
		1,                             
		PFD_DRAW_TO_WINDOW |           
		PFD_SUPPORT_OPENGL |           
		PFD_SUPPORT_GDI ,//PFD_DOUBLEBUFFER,              
		PFD_TYPE_RGBA,                 
		32,                            
		0, 0, 0, 0, 0, 0,              
		0,                             
		0,                             
		0,                             
		0, 0, 0, 0,                    
		32,                            
		0,                             
		0,                             
		PFD_MAIN_PLANE,                
		0,                             
		0, 0, 0                        
	};  

	pfd = _pfd;

	pixelFormat = ChoosePixelFormat(hdc,&pfd);

	if(pixelFormat==0)
		MessageBox(0,"pixel format error","ChoosePixelFormat",MB_OK|MB_ICONEXCLAMATION);

	if(!SetPixelFormat(hdc,pixelFormat,&pfd))
		MessageBox(0,"pixel format error","DescribePixelFormat",MB_OK|MB_ICONEXCLAMATION);

	hglrc = wglCreateContext(hdc);

	if(!hglrc)
		MessageBox(0,"creating context error","wglCreateContext",MB_OK|MB_ICONEXCLAMATION);

	wglMakeCurrent(hdc,0);
	wglMakeCurrent(hdc,hglrc);

	ReleaseDC(hwnd,hdc);

	SetWindowLongPtr(hwnd,GWL_USERDATA,(LONG)this);
}

void OpenGLFixedRenderer::Render()
{
	if(!hglrc)
		return;

	hdc=GetDC(hwnd);

	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	glClearColor(0,0,0,1);

	RECT rc;
	GetClientRect(hwnd,&rc);
	glViewport(rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top);

	
	SwapBuffers(hdc);
	ReleaseDC(hwnd,hdc);
}



//--------------------DirectXRendererData-------------------------




DirectXRenderer::DirectXRenderer()
{
	driverType = D3D_DRIVER_TYPE_NULL;
	featureLevel = D3D_FEATURE_LEVEL_11_0;
	pd3dDevice = nullptr;
	pImmediateContext = nullptr;
	pSwapChain = nullptr;
	pRenderTargetView = nullptr;
}

DirectXRenderer::~DirectXRenderer()
{
	if( pImmediateContext ) pImmediateContext->ClearState();

	if( pRenderTargetView ) pRenderTargetView->Release();
	if( pSwapChain ) pSwapChain->Release();
	if( pImmediateContext ) pImmediateContext->Release();
	if(pd3dDevice ) pd3dDevice->Release();
}

char* DirectXRenderer::Name()
{
	return 0;
}

void DirectXRenderer::Create(HWND container)
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect( container, &rc );
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	DXGI_SWAP_CHAIN_DESC scd = {0};
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;    // how the swap chain should be used
	scd.BufferCount = 2;                                  // a front buffer and a back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; 
	scd.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;    // the recommended flip mode
	scd.SampleDesc.Count = 1;
	scd.OutputWindow = container;
	scd.Windowed=true;


	//hr = D3D11CreateDevice( nullptr, D3D_DRIVER_TYPE_HARDWARE ,nullptr, 0, nullptr, 0,D3D11_SDK_VERSION, &pd3dDevice, &featureLevel, &pImmediateContext );
	hr = D3D11CreateDeviceAndSwapChain( nullptr, D3D_DRIVER_TYPE_HARDWARE ,nullptr, 0, nullptr, 0,D3D11_SDK_VERSION,&scd,&pSwapChain, &pd3dDevice, &featureLevel, &pImmediateContext );


	if(S_OK!=hr)
		return;
}


void DirectXRenderer::Render()
{
	pSwapChain->Present( 1, 0 );
}


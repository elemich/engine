
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


//#define RENDERER DirectXRenderer()
#define RENDERER OpenGLFixedRenderer()




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



void ProjectFolderBrowser::Create()
{
	PIDLIST_ABSOLUTE projectFolder=SelectProjectFolder();

	if(!projectFolder)
		return;

	hwnd=CreateWindow(WC_TABCONTROL,"TabControlWindow",WS_OVERLAPPEDWINDOW|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,0,0,0,0);

	TCITEM item={0};

	item.pszText="ProjectFolderBrowser";
	item.mask=TCIF_TEXT;

	SendMessage(hwnd,TCM_INSERTITEM,0,(LPARAM)(LPTCITEM)&item);
	
	RECT tabcontrol_rectangle=WHGetClientRect(hwnd);

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (!SUCCEEDED(hr))
		__debugbreak();

	hr = CoCreateInstance(CLSID_ExplorerBrowser, NULL, CLSCTX_INPROC,IID_PPV_ARGS(&browser));
	if (!SUCCEEDED(hr))
		__debugbreak();

	FOLDERSETTINGS fs = {0};
	fs.ViewMode = FVM_DETAILS;
	fs.fFlags = FWF_AUTOARRANGE/* | FWF_NOWEBVIEW*/;
	hr = browser->Initialize(hwnd, &tabcontrol_rectangle, &fs);

	if (!SUCCEEDED(hr))
		__debugbreak();

	browser->BrowseToIDList(projectFolder,SBSP_DEFBROWSER);
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



void ProjectFolderBrowser2::Create()
{
	PIDLIST_ABSOLUTE projectFolder=SelectProjectFolder();

	if(!projectFolder)
		return;

	hwnd=CreateWindow(WC_TABCONTROL,"TabControlWindow",WS_OVERLAPPEDWINDOW|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,0,0,0,0);
	
	TCITEM item={0};

	item.pszText="ProjectFolderBrowser";
	item.mask=TCIF_TEXT;

	SendMessage(hwnd,TCM_INSERTITEM,0,(LPARAM)(LPTCITEM)&item);

	RECT tabcontrol_rectangle=WHGetClientRect(hwnd);

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
	hr = browser->Initialize(hwnd, &tabcontrol_rectangle, style);
	if (!SUCCEEDED(hr))
		__debugbreak();

	IShellItem* shellItem;
	hr=SHCreateShellItem(0,0,projectFolder,&shellItem);
	if (!SUCCEEDED(hr))
		__debugbreak();

	browser->AppendRoot(shellItem, SHCONTF_CHECKING_FOR_CHILDREN|SHCONTF_NONFOLDERS|SHCONTF_FOLDERS, NSTCRS_HIDDEN | NSTCRS_EXPANDED, NULL); // ignore result
}




//--------------------AppData-------------------------

LRESULT CALLBACK MainWindowProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	App* app=(App*)GetWindowLong(hwnd,GWL_USERDATA);

	if(!app)
		return DefDlgProc(hwnd,msg,wparam,lparam);

	switch(msg)
	{
		case WM_PARENTNOTIFY:
		{
			switch(LOWORD(wparam))
			{
				case WM_CREATE:
				{
					HWND newChild=(HWND)lparam;
					HWND child=GetWindow(hwnd,GW_CHILD);

					if(newChild==child)
					{
						IntSize size=(WHGetWindowSize(hwnd));
						WHSetWindowSize(newChild,size.x,size.y);
					}

					TDLList<HWND> childs;

					
				}

				break;
			}
		}
		
		

		break;
	}



	return DefDlgProc(hwnd,msg,wparam,lparam);
}



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

	this->CreateMainWindow();

	return error;
}

void App::CreateMainWindow()
{
	hwnd=CreateWindow(WC_DIALOG,"MainWindow",WS_OVERLAPPEDWINDOW|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,0,0,0,0);

	WHSetWindowDataAndProcedure(hwnd,this,MainWindowProc);

	//browser.Create();

	renderer=new RENDERER;

	renderer->Init();
}


void App::AppLoop()
{
#pragma message (LOCATION " @mic: PeekMessage has 0 as hwnd to let other windows work")

	while(WM_CLOSE != PeekMessage(&msg,0,0,0,PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		renderer->Render();
	}
}







//--------------------OpenGLRenderer-------------------------


LRESULT CALLBACK OpenGLFixedRenderProcedure(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	OpenGLFixedRenderer* renderer=(OpenGLFixedRenderer*)GetWindowLong(hwnd,GWL_USERDATA);

	if(!renderer)
		return DefDlgProc(hwnd,msg,wparam,lparam);

	return DefDlgProc(hwnd,msg,wparam,lparam);
}


OpenGLFixedRenderer::OpenGLFixedRenderer()
{
	hglrc=0;
}

char* OpenGLFixedRenderer::Name()
{
	return 0;
}

void OpenGLFixedRenderer::Init()
{
	hwnd=CreateWindowEx(WS_EX_TOOLWINDOW,WC_DIALOG,"OpenGLFixedRenderer",WS_OVERLAPPEDWINDOW|WS_VISIBLE|WS_CHILD,CW_USEDEFAULT,CW_USEDEFAULT,/*CW_USEDEFAULT,CW_USEDEFAULT*/100,100,___app->hwnd,0,0,0);

	WHSetWindowDataAndProcedure(hwnd,this,MainWindowProc);

	hdc=GetDC(hwnd);

	if(!hdc)
		MessageBox(0,"Getting Device Context","GetDC",MB_OK|MB_ICONEXCLAMATION);

	PIXELFORMATDESCRIPTOR _pfd=         
	{
		sizeof(PIXELFORMATDESCRIPTOR), 
		1,                             
		PFD_DRAW_TO_WINDOW |           
		PFD_SUPPORT_OPENGL |           
		PFD_DOUBLEBUFFER,              
		PFD_TYPE_RGBA,                 
		24,                            
		0, 0, 0, 0, 0, 0,              
		0,                             
		0,                             
		0,                             
		0, 0, 0, 0,                    
		16,                            
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

}

void OpenGLFixedRenderer::Render()
{
	if(!hglrc)
		return;

	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	glClearColor(0,0,0,1);


	SwapBuffers(hdc);
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

void DirectXRenderer::Init()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect( hwnd, &rc );
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	DXGI_SWAP_CHAIN_DESC scd = {0};
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;    // how the swap chain should be used
	scd.BufferCount = 2;                                  // a front buffer and a back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; 
	scd.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;    // the recommended flip mode
	scd.SampleDesc.Count = 1;
	scd.OutputWindow = hwnd;
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


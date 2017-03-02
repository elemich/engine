
#include "win32.h"

#pragma message("@mic \"LNK1123: Failure during conversion to COFF: file invalid or corrupt\" was resolved renaming cvtres.exe to cvtres1.exe.")
//#pragma message("@mic \"LNK1123: Failure during conversion to COFF: file invalid or corrupt\" was resolved renaming cvtres.exe to cvtres1.exe.")

WNDPROC SystemOriginalSysTreeView32ControlProcedure;


App* ___app;




HWND CreateTabContainer(int x,int y,int w,int h,HWND parent,int currentCount)
{
	char tabContainerName[CHAR_MAX];
	sprintf_s(tabContainerName,"TabContainer%d",currentCount);
	HWND tabWindow= CreateWindow(WC_TABCONTAINER,tabContainerName,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|TCS_FOCUSNEVER,x,y,w,h,parent,(HMENU)currentCount,0,0);

	return tabWindow;
}

int CreateTab(HWND dst,char* text,int pos,HWND src)
{
	int tmpPos=-1;
	char tabLabel[CHAR_MAX];
	TCITEM t={0};
	t.mask=TCIF_TEXT;
	t.pszText=tabLabel;
	t.cchTextMax=CHAR_MAX;

	tmpPos=(pos<0 ? SendMessage(dst,TCM_GETITEMCOUNT,0,0) : pos);

	if(!src)
	{
		if(tmpPos<0)
			t.pszText=text;
		else
			sprintf_s(tabLabel,"Tab%d",tmpPos);
	}
	else
	{
		int tIdx=SendMessage(src,TCM_GETCURSEL,0,0);
		SendMessage(src,TCM_GETITEM,tIdx,(LPARAM)&t);
	}

	SendMessage(dst,TCM_INSERTITEM,tmpPos,(LPARAM)&t);

	return tmpPos;
}

void RemoveTab(HWND hwnd,int idx)
{
	int itemCount=SendMessage(hwnd,TCM_GETITEMCOUNT,0,0);
	if(!SendMessage(hwnd,TCM_DELETEITEM,idx,0))//tabcontrol automatically scales remaining items
		__debugbreak();
	int removeCount=(itemCount-1)-idx;
	for(int i=0;i<removeCount;i++)
	{
		int srcIdx=idx+i+1;
		int dstIdx=srcIdx-1;

		HWND child=GetDlgItem(hwnd,srcIdx);

		if(!child)
			__debugbreak();

		SetWindowLong(child,GWL_ID,(LONG)dstIdx);
	}
}

void ReparentTabChild(HWND src,HWND dst,int idx)
{
	int newTabContentIdx=CreateTab(dst,0,0,src);
	HWND tabContentWindow=GetDlgItem(src,idx);
	SetParent(tabContentWindow,dst);
	SetWindowLong(tabContentWindow,GWL_ID,(LONG)newTabContentIdx);
	SendMessage(dst,WM_SIZE,0,0);
}

RECT GetTabClientSize(HWND hwnd)
{
	RECT clientRect,itemRect,adj0Rect,adj1Rect,result;

	GetClientRect(hwnd,&clientRect);
	SendMessage(hwnd,TCM_GETITEMRECT,0,(LPARAM)&itemRect);

	adj0Rect=itemRect;
	adj1Rect=itemRect;

	SendMessage(hwnd,TCM_ADJUSTRECT,0,(LPARAM)&adj0Rect);
	SendMessage(hwnd,TCM_ADJUSTRECT,1,(LPARAM)&adj1Rect);

	result.top=clientRect.top+adj0Rect.top-itemRect.top;
	result.bottom=clientRect.bottom-(itemRect.bottom-adj0Rect.bottom);
	result.right=clientRect.right-(itemRect.right-adj0Rect.right);
	result.left=clientRect.left+adj0Rect.left-itemRect.left;

	/*result.top=clientRect.top+adj1Rect.top;
	result.bottom=clientRect.bottom-(adj1Rect.bottom-itemRect.bottom);
	result.right=clientRect.right-(adj1Rect.right-itemRect.right);
	result.left=clientRect.left+itemRect.left-adj1Rect.left;*/

	return result;
}



void OnTabSizeChanged(HWND hwnd)
{
	printf("OnTabSizeChanged %p\n",hwnd);

	int		tabIdx=-1;
	HWND	child=0;	

	tabIdx=0;//SendMessage(hwnd,TCM_GETCURSEL,0,0);
	

	if(tabIdx<0)
		return;

	child=GetDlgItem(hwnd,tabIdx);

	if(child==0)
		return;

	RECT size=GetTabClientSize(hwnd);

	MoveWindow(child,size.left,size.top,size.right-size.left,size.bottom-size.top,true);

}

void EnableChilds(HWND hwnd,int enable,int show)
{
	HWND child=0;
	while(child=FindWindowEx(hwnd,child ? child : 0,0,0))
	{
		if(enable>=0)EnableWindow(child,enable);
		if(show>=0)ShowWindow(child,show);
	}
}

void EnableAllChildsDescendants(HWND hwnd,int enable,int show)
{
	HWND child=0;
	while(child=FindWindowEx(hwnd,child ? child : 0,0,0))
		EnableChilds(child,enable,show);
}

void EnableAndShowContainerChild(HWND hwnd,int idx)
{
	HWND child=GetDlgItem(hwnd,idx);

	if(!child)
	{
		__debugbreak();
		return;
	}

	EnableChilds(hwnd,false,false);

	SendMessage(hwnd,TCM_SETCURSEL,idx,0);
	EnableWindow(child,true);
	ShowWindow(child,SW_SHOW);

	/*if(TabProc==(WNDPROC)GetClassLong(hwnd,GWL_WNDPROC))//is a tab container ?
		SetWindowLongPtr(hwnd,GWL_USERDATA,(LONG_PTR)idx);//set child window index in the tab container user data*/

	RECT tabRect=GetTabClientSize(hwnd);

	SetWindowPos(child,HWND_TOP,tabRect.left,tabRect.top,tabRect.right-tabRect.left,tabRect.bottom-tabRect.top,SWP_SHOWWINDOW|SWP_ASYNCWINDOWPOS);
}


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

	browser->SetOptions(EBO_NOWRAPPERWINDOW|EBO_SHOWFRAMES);
	browser->BrowseToIDList(projectFolder,SBSP_DEFBROWSER);

	/*INameSpaceTreeControl2* shellItem;
	hr=browser->GetCurrentView(EP_NavPane,(void**)&shellItem);

	if (!SUCCEEDED(hr))
		__debugbreak();*/

	

	hr=IUnknown_GetWindow(browser,&hwnd);

	if (!SUCCEEDED(hr))
		__debugbreak();

	if(BROWSER_DEBUG)
		printf("browser window %p\n",hwnd);
}



//--------------------ProjectFolderBrowser2-------------------------

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
	//hr=SHCreateItemWithParent(0,);
	if (!SUCCEEDED(hr))
		__debugbreak();

	browser->AppendRoot(shellItem, SHCONTF_CHECKING_FOR_CHILDREN|SHCONTF_FOLDERS, NSTCRS_HIDDEN | NSTCRS_EXPANDED, NULL); // ignore result
	
	hr=IUnknown_GetWindow(browser,&hwnd);

	if (!SUCCEEDED(hr))
		__debugbreak();

	if(BROWSER_DEBUG)
		printf("browser window %p\n",hwnd);

	
}

//--------------------SceneEntities-------------------------

LRESULT CALLBACK SceneEntitiesProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	SceneEntities* sceneEntities=(SceneEntities*)GetWindowLongPtr(hwnd,GWL_USERDATA);

	return CallWindowProc(SystemOriginalSysTreeView32ControlProcedure,hwnd,msg,wparam,lparam);
}


SceneEntities::SceneEntities()
{

}

SceneEntities::~SceneEntities()
{
	
}

void SceneEntities::Create(HWND container)
{
	hwnd=CreateWindow(WC_SCENEENTITIESWINDOW,0,WS_CHILD,0,0,100,100,container,0,0,0);
	SetWindowLongPtr(hwnd,GWL_USERDATA,(LONG_PTR)this);
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
	
	mainAppWindow.Create();
}


void App::AppLoop()
{
#pragma message (LOCATION " @mic: PeekMessage has 0 as hwnd to let other windows work")

	MSG msg;

	while(true)
	{
		if(PeekMessage(&msg,0,0,0,PM_REMOVE))
		{
			if(msg.message==WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			for(int i=0;i<RendererInterface::renderers.size();i++)
				RendererInterface::renderers[i]->Render();
		}
	}
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

//--------------------Logger-------------------------


void Logger::Create(HWND container)
{
	hwnd=CreateWindow(WC_EDIT,0,WS_CHILD|ES_READONLY|WS_BORDER,CW_USEDEFAULT,CW_USEDEFAULT,100,100,container,0,0,0);
}
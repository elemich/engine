
#include "win32.h"

#include "SceneEntitiesResource.h"

#pragma message(LOCATION " LNK1123: Failure during conversion to COFF: file invalid or corrupt\" was resolved renaming cvtres.exe to cvtres1.exe.")

WNDPROC SystemOriginalSysTreeView32ControlProcedure;



/*
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

	/ *INameSpaceTreeControl2* shellItem;
	hr=browser->GetCurrentView(EP_NavPane,(void**)&shellItem);

	if (!SUCCEEDED(hr))
		__debugbreak();* /

	

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
	SceneEntities* se=(SceneEntities*)GetWindowLongPtr(hwnd,GWL_USERDATA);

	LRESULT result=CallWindowProc(SystemOriginalSysTreeView32ControlProcedure,hwnd,msg,wparam,lparam);

	switch(msg)
	{
	case WM_MOUSEMOVE:
		SetFocus(hwnd);
	break;
	}

	return result;
}

INT_PTR CALLBACK SceneEntitiesDialogProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	INT_PTR result=false;//DefDlgProc(hwnd,msg,wparam,lparam);

	switch(msg)
	{
	case WM_ERASEBKGND:
		return true;
	case WM_SIZE:
		{
			HWND tabContainer=GetParent(hwnd);

			ContainerWindow* cw=(ContainerWindow*)GetWindowLongPtr(GetParent(tabContainer),GWL_USERDATA);

			int width=LOWORD(lparam);
			int height=HIWORD(lparam);

			RECT statusRect;
			GetClientRect(GetDlgItem(hwnd,IDC_STATUSDOWN),&statusRect);
			int statusHeight=statusRect.bottom-statusRect.top;

			HDWP hdwp=BeginDeferWindowPos(4);
			DeferWindowPos(hdwp,GetDlgItem(hwnd,IDC_STATUSUP),0,0,0,width,statusHeight,SWP_SHOWWINDOW);
			DeferWindowPos(hdwp,GetDlgItem(hwnd,IDC_ENTITIES),0,0,statusHeight,width,height-2*statusHeight,SWP_SHOWWINDOW);
			DeferWindowPos(hdwp,GetDlgItem(hwnd,IDC_STATUSDOWN),0,0,height-statusHeight,width,statusHeight,SWP_SHOWWINDOW);
			DeferWindowPos(hdwp,GetDlgItem(hwnd,IDC_EXPAND),HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
			EndDeferWindowPos(hdwp);
		}
	break;
	}

	return result;
}


SceneEntities::SceneEntities()
{

}

SceneEntities::~SceneEntities()
{
	
}

void ProcessEntity(SceneEntities& se,Entity* e,int itemIdx,HTREEITEM parent)
{
	if(!e)
		return;

	TVITEM tvi={0};
	TVINSERTSTRUCT tvis={0};

	tvi.mask=TVIF_TEXT|TVIF_PARAM|TVIF_CHILDREN;
	tvi.pszText=(CHAR*)e->entity_name.Buf();
	tvi.cchTextMax=e->entity_name.Count();
	tvi.lParam=(LPARAM)e;
	tvi.cChildren=e->entity_childs.size() ? 1 : 0;

	int parentIndex=e->entity_parent ? (itemIdx - e->entity_parent->entity_childs.size()) : 0;

	tvis.hParent=parent;//se.items[parentIndex];
	//tvis.hInsertAfter=parent;//se.items[parentIndex];
	tvis.item=tvi;

	HTREEITEM item=(HTREEITEM)SendMessage(GetDlgItem(se.hwnd,IDC_ENTITIES),TVM_INSERTITEM,0,(LPARAM)&tvis);

	se.items.push_back(item);

	itemIdx++;

	for(std::list<Entity*>::iterator ChildNode=e->entity_childs.begin();ChildNode!=e->entity_childs.end();ChildNode++,itemIdx++)
		ProcessEntity(se,*ChildNode,itemIdx,item);
}

void SceneEntities::Expand()
{
	for(int i=0;i<(int)items.size();i++)
		SendMessage(GetDlgItem(hwnd,IDC_ENTITIES),TVM_EXPAND,TVE_EXPAND,(LPARAM)items[i]);

}

void SceneEntities::Fill()
{
	items.clear();

	SendMessage(GetDlgItem(hwnd,IDC_ENTITIES),TVM_DELETEITEM,0,0);//delete all items

	HTREEITEM parent=TVI_ROOT;

	std::list<Entity*>::iterator eList=Entity::pool.begin();

	if(eList!=Entity::pool.end())
		ProcessEntity(*this,*eList,0,parent);

	Expand();
}

void SceneEntities::Create(HWND container)
{
	hwnd=CreateDialog(0,MAKEINTRESOURCE(IDD_SCENEENTITIESDIALOG),container,SceneEntitiesDialogProc);

	SendMessage(GetDlgItem(hwnd,IDC_ENTITIES),TVM_SETEXTENDEDSTYLE,(WPARAM)TVS_EX_FADEINOUTEXPANDOS,(LPARAM)TVS_EX_FADEINOUTEXPANDOS);
	SendMessage(GetDlgItem(hwnd,IDC_ENTITIES),TVM_SETEXTENDEDSTYLE,(WPARAM)TVS_EX_DOUBLEBUFFER,(LPARAM)TVS_EX_DOUBLEBUFFER);
	SendMessage(GetDlgItem(hwnd,IDC_ENTITIES),TVM_SETEXTENDEDSTYLE,(WPARAM)TVS_EX_NOINDENTSTATE,(LPARAM)TVS_EX_NOINDENTSTATE);

	SetWindowLongPtr(hwnd,GWL_USERDATA,(LONG_PTR)this);

	Fill();
}*/

//--------------------AppData-------------------------



int App::Init()
{
	HRESULT result;
	
	result=CoInitialize(0);
	
	{
		char _pszDisplayName[MAX_PATH]="";

		BROWSEINFO bi={0};
		bi.pszDisplayName=_pszDisplayName;
		bi.lpszTitle="Select Project Directory";

		PIDLIST_ABSOLUTE tmpProjectFolder=SHBrowseForFolder(&bi);

		DWORD err=GetLastError();

		if(tmpProjectFolder)
		{
			char path[MAX_PATH];

			if(SHGetPathFromIDList(tmpProjectFolder,path))
			{
				this->projectFolder=path;
			}
		}
	}



	Direct2DGuiBase::Init(L"Verdana",10);

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

void App::Close()
{
	Direct2DGuiBase::Release();
	CoUninitialize();
}

void App::CreateMainWindow()
{
	
	mainAppWindow.Create();
}


void App::Run()
{
#pragma message (LOCATION " PeekMessage has 0 as hwnd to let other windows work")

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
		//else
		{
			/*for(int i=0;i<(int)RendererViewportInterface::Pool().size();i++)
			{
				if(RendererViewportInterface::Pool()[i]->IsSelected())
					RendererViewportInterface::Pool()[i]->OnRender();
			}*/
		}
		
	}

	this->Close();
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


/*
void Logger::Create(HWND container)
{
	hwnd=CreateWindow(WC_EDIT,0,WS_CHILD|ES_READONLY|WS_BORDER,CW_USEDEFAULT,CW_USEDEFAULT,100,100,container,0,0,0);
}*/




/*
INT_PTR CALLBACK EntityPropertyDialogProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	INT_PTR result=false;//DefDlgProc(hwnd,msg,wparam,lparam);


	return result;
}




void EntityProperty::Create(HWND container)
{
	hwnd=CreateDialog(0,MAKEINTRESOURCE(IDD_ENTITYPROPERTIESDIALOG),container,SceneEntitiesDialogProc);


}*/
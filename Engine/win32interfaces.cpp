
#include "win32.h"

#include "SceneEntitiesResource.h"

#pragma message(LOCATION " LNK1123: Failure during conversion to COFF: file invalid or corrupt\" was resolved renaming cvtres.exe to cvtres1.exe.")

WNDPROC SystemOriginalSysTreeView32ControlProcedure;


void WindowData::LinkSibling(WindowData* t,int pos)
{
	if(!t)
		return;

	int reciprocal = pos<2 ? pos+2 : pos-2;

	this->siblings[pos].push_back(t);
	t->siblings[reciprocal].push_back(this);
}

void WindowData::UnlinkSibling(WindowData* t)
{
	RECT rc;
	GetClientRect(this->hwnd,&rc);

	if(t)
	{
		for(int i=0;i<4;i++)
			t->siblings[i].remove(this);
	}


	for(int i=0;i<4;i++)
	{
		if(t)
		{
			this->siblings[i].remove(t);
		}
		else
		{
			for(std::list<WindowData*>::iterator it=this->siblings[i].begin();it!=this->siblings[i].end();it++)
			{
				WindowData* tabToUnlinkFromThis=(*it);

				for(int i=0;i<4;i++)
					tabToUnlinkFromThis->siblings[i].remove(this);
			}

			this->siblings[i].clear();
		}
	}
}

WindowData* WindowData::FindSiblingOfSameSize()
{
	for(int i=0;i<4;i++)
	{
		for(std::list<WindowData*>::iterator it=this->siblings[i].begin();it!=this->siblings[i].end();it++)
		{
			WindowData* tabContainer=(*it);

			if(tabContainer->width==this->width || tabContainer->height==this->height)
				return tabContainer;
		}
	}

	return 0;
}

int WindowData::FindSiblingPosition(WindowData* t)
{
	for(int i=0;i<4;i++)
	{
		if(std::find(this->siblings[i].begin(),this->siblings[i].end(),t)!=this->siblings[i].end())
			return i;
	}

	return -1;
}

bool WindowData::FindAndGrowSibling()
{
	WindowData* growingTab=this->FindSiblingOfSameSize();

	if(growingTab)
	{
		int tabReflinkPosition=this->FindSiblingPosition(growingTab);

		RECT growingTabRc,thisRc;
		GetClientRect(growingTab->hwnd,&growingTabRc);
		GetClientRect(this->hwnd,&thisRc);
		MapWindowRect(growingTab->hwnd,GetParent(growingTab->hwnd),&growingTabRc);
		MapWindowRect(this->hwnd,GetParent(growingTab->hwnd),&thisRc);

		switch(tabReflinkPosition)
		{
		case 0:
			SetWindowPos(growingTab->hwnd,0,growingTabRc.left,thisRc.top,thisRc.right-growingTabRc.left,thisRc.bottom-thisRc.top,SWP_SHOWWINDOW);
			break;
		case 1:
			SetWindowPos(growingTab->hwnd,0,thisRc.left,growingTabRc.top,thisRc.right-thisRc.left,thisRc.bottom-growingTabRc.top,SWP_SHOWWINDOW);
			break;
		case 2:
			SetWindowPos(growingTab->hwnd,0,thisRc.left,thisRc.top,growingTabRc.right-thisRc.left,thisRc.bottom-thisRc.top,SWP_SHOWWINDOW);
			break;
		case 3:
			SetWindowPos(growingTab->hwnd,0,thisRc.left,thisRc.top,thisRc.right-thisRc.left,growingTabRc.bottom-thisRc.top,SWP_SHOWWINDOW);
			break;
		default:
			__debugbreak();
			break;
		}

		this->UnlinkSibling();

		return true;
	}

	return false;
}

///////////////////////////////////////// APP

DWORD WINAPI threadGuiTabFunc(LPVOID);

int App::threadGuiTab=0;

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

	if(!threadGuiTab)
			CreateThread(0,0,threadGuiTabFunc,this,0,(DWORD*)&threadGuiTab);


	this->CreateMainWindow();

	return error;
}

void App::Close()
{
	if(TPoolVector<GuiTab>::pool.size()==1 && threadGuiTab)
		ExitThread(threadGuiTab);

	Direct2DGuiBase::Release();
	CoUninitialize();
}

void App::CreateMainWindow()
{
	
	mainAppWindow.Create();
	mainAppWindow.application=this;
}

bool sem=0;

App::App():
threadLockedEntities(true),
threadUpdateNeeded(true),
threadPaintNeeded(false)
{}

DWORD WINAPI threadGuiTabFunc(LPVOID _app)
{

	/*App* app=(App*)_app;
	while(true)
	{
		if(!app->threadLockedEntities && app->threadUpdateNeeded)
		{
			app->threadPaintNeeded=false;
			sem=true;

			for(int i=0;i<(int)GetPool<TabContainer>().size();i++)
			{
				if(GetPool<TabContainer>()[i]->GetSelected())
				{
					GetPool<TabContainer>()[i]->OnUpdate();
				}
			}

			app->threadUpdateNeeded=false;
			app->threadPaintNeeded=true;
		}
	}*/
	

	return 0;
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
		else
		{
			/*if(!threadLockedEntities && this->threadPaintNeeded)
			{
				this->threadUpdateNeeded=false;
				sem=true;*/
				this->timerMain.update();


				for(int i=0;i<(int)GetPool<OpenGLRenderer>().size();i++)
				{
					GetPool<OpenGLRenderer>()[i]->RenderViewports();
				}

				/*this->threadPaintNeeded=false;
				this->threadUpdateNeeded=true;
			}*/
			
		}
	}

	this->Close();
}

void TimerWin32::update()
{
	last=current;
	current=timeGetTime();
	delta=current-last;
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
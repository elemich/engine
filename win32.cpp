#include "win32.h"


DWORD WINAPI threadFunc(LPVOID data)
{
	Thread* t=(Thread*)data;

	while(true)
	{
		Timer::instance->update();

		for(std::list<Task*>::iterator tsk=t->tasks.begin();tsk!=t->tasks.end();)
		{
			Task* task=(*tsk);

			if(!t->pause && task->owner==t && !task->pause)
			{
				t->executing=task;

				if(task->func)
					task->func();
				if(task->remove)
				{
					task->func=nullptr;
					tsk=t->tasks.erase(tsk);
				}
				else
					tsk++;

				t->executing=0;
			}
			else
				tsk++;
		}

		Sleep(t->sleep);
	}


	ExitThread(0);
}


Thread::Thread()
{
	handle=(int)(HANDLE)CreateThread(0,0,threadFunc,this,/*CREATE_SUSPENDED*/0,(DWORD*)(int*)&id);
	pause=false;
	executing=0;
	sleep=1;
}

Thread::~Thread()
{
	TerminateThread((HANDLE)handle,0);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
ID2D1Factory*		Direct2DGuiBase::factory=0;
IWICImagingFactory*	Direct2DGuiBase::imager=0;
IDWriteFactory*		Direct2DGuiBase::writer=0;
IDWriteTextFormat*	Direct2DGuiBase::texter=0;

void Direct2DGuiBase::Init(wchar_t* fontName,float fontSize)
{
	if(!factory)
	{
		HRESULT res=S_OK;

		res=CoCreateInstance(CLSID_WICImagingFactory,NULL,CLSCTX_INPROC_SERVER,IID_IWICImagingFactory,(LPVOID*)&imager);
		if(S_OK!=res)
			__debugbreak();

		res=D2D1CreateFactory(/*D2D1_FACTORY_TYPE_SINGLE_THREADED*/D2D1_FACTORY_TYPE_MULTI_THREADED, &factory);
		if(S_OK!=res)
			__debugbreak();

		res=DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,__uuidof(writer),reinterpret_cast<IUnknown **>(&writer));
		if(S_OK!=res)
			__debugbreak();

		res=writer->CreateTextFormat(fontName,NULL,DWRITE_FONT_WEIGHT_NORMAL,DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,fontSize,L"",&texter);
		if(S_OK!=res)
			__debugbreak();

		

		// Center the text horizontally and vertically.
		//textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

		res=texter->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		if(S_OK!=res)
			__debugbreak();
	}
}

void Direct2DGuiBase::Release()
{
	SAFERELEASE(factory);
	SAFERELEASE(imager);
	SAFERELEASE(writer);
	SAFERELEASE(texter);
};

ID2D1HwndRenderTarget* Direct2DGuiBase::InitHWNDRenderer(HWND hwnd)
{
	ID2D1HwndRenderTarget* renderer=0;

	if (hwnd!=0)
	{
		RECT rc;
		GetClientRect(hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left,rc.bottom - rc.top);

		D2D1_RENDER_TARGET_PROPERTIES rtp=D2D1::RenderTargetProperties();

		rtp.type=D2D1_RENDER_TARGET_TYPE_HARDWARE;
		rtp.usage=D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;//we wants resource sharing

		HRESULT res=factory->CreateHwndRenderTarget(rtp,D2D1::HwndRenderTargetProperties(hwnd, size,D2D1_PRESENT_OPTIONS_IMMEDIATELY),&renderer);
		if(S_OK!=res)
			__debugbreak();
	}

	return renderer;
}

void Direct2DGuiBase::CreateRawBitmap(const wchar_t* fname,unsigned char*& buffer,float &width,float &height)
{
	ID2D1Bitmap *bitmap=0;

	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICStream *pStream = NULL;
	IWICFormatConverter *pConverter = NULL;
	IWICBitmapScaler *pScaler = NULL;

	HRESULT hr;

	hr = imager->CreateDecoderFromFilename(fname,NULL,GENERIC_READ,WICDecodeMetadataCacheOnLoad,&pDecoder);

	if (!SUCCEEDED(hr))
		__debugbreak();

	unsigned int frameCount;
	pDecoder->GetFrameCount(&frameCount);

	hr = pDecoder->GetFrame(0, &pSource);

	if (!SUCCEEDED(hr))
		__debugbreak();

	hr = imager->CreateFormatConverter(&pConverter);

	if (!SUCCEEDED(hr))
		__debugbreak();

	hr = pConverter->Initialize(pSource,GUID_WICPixelFormat32bppPBGRA,WICBitmapDitherTypeNone,NULL,0.f,WICBitmapPaletteTypeMedianCut);

	if(buffer)
	delete [] buffer;

	UINT _width,_height;
	pSource->GetSize(&_width,&_height);

	buffer=new unsigned char[_width*_height*4];

	hr=pSource->CopyPixels(0,_width*4,_width*_height*4,buffer);

	width=(float)_width;
	height=(float)_height;

	if (!SUCCEEDED(hr))
		__debugbreak();

	if (SUCCEEDED(hr))
	{
		SAFERELEASE(pDecoder);
		SAFERELEASE(pSource);
		SAFERELEASE(pStream);
		SAFERELEASE(pConverter);
		SAFERELEASE(pScaler);
	}
}

void Direct2DGuiBase::DrawText(ID2D1RenderTarget*renderer,ID2D1Brush* brush,const char* inputText,float x1,float y1, float x2,float y2,float ax,float ay)
{
	if(!inputText)
		return;

	size_t slen=sizeof(const char)*strlen(inputText);
	size_t retLen=slen;
	wchar_t retText[CHAR_MAX];
	size_t sizewInWords=sizeof(wchar_t)*CHAR_MAX;
	int err=mbstowcs(retText,inputText,slen);

	//printf("");

	renderer->PushAxisAlignedClip(D2D1::RectF(x1,y1,x2,y2),D2D1_ANTIALIAS_MODE_ALIASED);

	if(ax<0 && ay<0)
		renderer->DrawText(retText,retLen,texter,D2D1::RectF(x1,y1,x2,y2),brush,D2D1_DRAW_TEXT_OPTIONS_NONE,DWRITE_MEASURING_MODE_GDI_CLASSIC);
	else
	{
		ID2D1HwndRenderTarget* hwndRenderer=(ID2D1HwndRenderTarget*)(renderer);
		
		if(!hwndRenderer)
			__debugbreak();

		HDC hdc=GetDC(hwndRenderer->GetHwnd());

		SIZE tSize;
		GetTextExtentPoint32(hdc,inputText,slen,&tSize);

		ReleaseDC(hwndRenderer->GetHwnd(),hdc);//@mic avoid this causes serious performance degradation on windowing

		vec4 rect(-tSize.cx/2.0f,-tSize.cy/2.0f,(float)tSize.cx,(float)tSize.cy);

		float ww=x2-x1;
		float hh=y2-y1;

		ax>=0 ? rect.x+=x1+ax*ww : rect.x=x1;
		ay>=0 ? rect.y+=y1+ay*hh : rect.y=y1;

		/*rect.x = x > rect.x ? x : (x+w < rect.x+rect.z ? rect.x - (rect.x+rect.z - (x+w)) - tSize.cx/2.0f: rect.x);
		rect.y = y > rect.y ? y : (y+h < rect.y+rect.w ? rect.y - (rect.y+rect.w - (y+h)) - tSize.c/2.0f: rect.y);*/


		renderer->DrawText(retText,retLen,texter,D2D1::RectF(rect.x,rect.y,rect.x+rect.z,rect.y+rect.w),brush,D2D1_DRAW_TEXT_OPTIONS_NONE,DWRITE_MEASURING_MODE_GDI_CLASSIC);
	}

	renderer->PopAxisAlignedClip();
}





///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////



WNDPROC SystemOriginalTabControlProcedure;

ContainerWindow::ContainerWindow()
{
	ContainerWindow_currentVisibleChildIdx=0;
	resizeDiffHeight=0;
	resizeDiffWidth=0;
	resizeEnumType=-1;
	resizeCheckWidth=0;
	resizeCheckHeight=0;
}
void ContainerWindow::Create(HWND container){}

void ContainerWindow::OnSizing()
{
	LPRECT sz=(LPRECT)this->lparam;
	
	RECT trc;
	GetWindowRect(this->hwnd,&trc);

	this->resizeDiffWidth=(sz->right-sz->left)-(trc.right-trc.left);
	this->resizeDiffHeight=(sz->bottom-sz->top)-(trc.bottom-trc.top);

	switch(this->wparam)
	{
	case WMSZ_LEFT:this->resizeEnumType=0;break;
	case WMSZ_TOP:this->resizeEnumType=1;break;
	case WMSZ_RIGHT:this->resizeEnumType=2;break;
	case WMSZ_BOTTOM:this->resizeEnumType=3;break;
	default:
		__debugbreak();
	}
}

void ContainerWindow::OnSize()
{
	WindowData::OnSize();
	//SplitterContainer::OnSize(this->hwnd,this->wparam,this->lparam);

	if(this->resizeEnumType<0)
		return;

	this->resizeCheckWidth=0;
	this->resizeCheckHeight=0;

	std::vector<TabContainer*>::iterator tvec;
	std::list<TabContainer*>::iterator   tlst;

	for(tvec=this->tabContainers.begin();tvec!=this->tabContainers.end();tvec++)
	{
		if((*tvec)->siblings[this->resizeEnumType].empty())
			(*tvec)->OnResizeContainer();
	}
}


std::vector<ContainerWindow> MainAppContainerWindow::windows;

MainAppContainerWindow::MainAppContainerWindow(){};
void MainAppContainerWindow::Create(HWND)
{
	menuMain=CreateMenu();
	menuEntities=CreateMenu();

	InsertMenu(menuMain,0,MF_BYPOSITION|MF_POPUP,(UINT_PTR)menuEntities,"Entities");
	{
		InsertMenu(menuEntities,0,MF_BYPOSITION|MF_STRING,MAINMENU_ENTITIES_IMPORTENTITY,"Import...");
	}

	hwnd=CreateWindow(WC_MAINAPPWINDOW,WC_MAINAPPWINDOW,WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,0,menuMain,0,this);

	RECT rc;
	GetClientRect(hwnd,&rc);
	TabContainer* tabContainer1=new TabContainer(0.0f,0.0f,(float)300,(float)200,hwnd);
	TabContainer* tabContainer2=new TabContainer(0.0f,204.0f,(float)300,(float)200,hwnd);
	TabContainer* tabContainer3=new TabContainer(0.0f,408.0f,(float)300,(float)rc.bottom-(rc.top+408),hwnd);
	TabContainer* tabContainer4=new TabContainer(304.0f,0.0f,(float)rc.right-(rc.left+304),(float)rc.bottom-rc.top,hwnd);

	GuiSceneViewer* scene=tabContainer1->tabs.SceneViewer();
	GuiViewport* viewport=tabContainer4->tabs.Viewport();

	viewport->rootEntity=scene->entityRoot;

	tabContainer3->tabs.ProjectViewer();
	tabContainer2->tabs.EntityViewer();

	TabContainer::BroadcastToPoolSelecteds(&GuiRect::OnSize);
	TabContainer::BroadcastToPoolSelecteds(&GuiRect::OnActivate);

	ShowWindow(hwnd,true);
}





LRESULT CALLBACK MainWindowProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	MainAppContainerWindow* mainw=(MainAppContainerWindow*)GetWindowLongPtr(hwnd,GWLP_USERDATA);

	LRESULT result=0;

	if(mainw)
		mainw->CopyProcedureData(hwnd,msg,wparam,lparam);

	switch(msg)
	{
		/*case WM_PAINT:
			//result=DefWindowProc(hwnd,msg,wparam,lparam);
			printf("main painting\n");
		break;*/
		case WM_CREATE:
		{
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)((LPCREATESTRUCT)lparam)->lpCreateParams);
		}
		break;
		case WM_CLOSE:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			PostQuitMessage(1);
		break;
		case WM_SIZING:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			mainw->CopyProcedureData(hwnd,msg,wparam,lparam);
			mainw->OnSizing();
		break;
		case WM_SIZE:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			mainw->CopyProcedureData(hwnd,msg,wparam,lparam);
			mainw->OnSize();
		break;
		/*case WM_ERASEBKGND:
			return (LRESULT)1;*/
		case WM_LBUTTONDOWN:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			mainw->CopyProcedureData(hwnd,msg,wparam,lparam);
			mainw->OnLButtonDown(hwnd,lparam);
		break;
		case WM_LBUTTONUP:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			mainw->CopyProcedureData(hwnd,msg,wparam,lparam);
			mainw->OnLButtonUp(hwnd);
		break;
		case WM_MOUSEMOVE:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			mainw->CopyProcedureData(hwnd,msg,wparam,lparam);
			mainw->OnMouseMove(hwnd,lparam);
		break;
		case WM_COMMAND:
			{
				if(!HIWORD(wparam))//menu notification
				{
					SetFocus(hwnd);

					switch(LOWORD(wparam))
					{
						case MAINMENU_ENTITIES_IMPORTENTITY:
						{
							char charpretval[5000]={0};

							OPENFILENAME openfilename={0};
							openfilename.lStructSize=sizeof(OPENFILENAME);
							openfilename.hwndOwner=hwnd;
							openfilename.lpstrFilter="Fbx File Format (*.fbx)\0*.fbx\0\0";
							openfilename.nFilterIndex=1;
							openfilename.lpstrFile=charpretval;
							openfilename.nMaxFile=5000;
							 
							if(GetOpenFileName(&openfilename) && openfilename.lpstrFile!=0)
							{

								HMODULE fbxImporter=(HMODULE)LoadLibrary("fbximporter.dll");

								if(!fbxImporter)
									__debugbreak();

								typedef Entity* (*ImportFbxScene)(char*);

								ImportFbxScene func=(ImportFbxScene)GetProcAddress(fbxImporter,"ImportFbxScene");

								if(func)
								{
									Entity* importedEntities=func(openfilename.lpstrFile);
									
									TabContainer::BroadcastToPool(&TabContainer::OnEntitiesChange,importedEntities);
								}
								else
									__debugbreak();

								FreeLibrary(fbxImporter);
							}
						}
						break;
					}
				}
			}
		break;
		default:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
	}

	return result;
}




///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


#pragma message(LOCATION " LNK1123: Failure during conversion to COFF: file invalid or corrupt\" was resolved renaming cvtres.exe to cvtres1.exe.")

WNDPROC SystemOriginalSysTreeView32ControlProcedure;

void WindowData::CopyProcedureData(HWND  h,UINT m,WPARAM w,LPARAM l){hwnd=h,msg=m,wparam=w,lparam=l;}


void WindowData::OnSize()
{
	width=LOWORD(lparam);
	height=HIWORD(lparam);
}
void WindowData::OnWindowPosChanging()
{
	width=(float)((LPWINDOWPOS)lparam)->cx;
	height=(float)((LPWINDOWPOS)lparam)->cy;
}

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
	mainAppWindow.application=this;
}

bool sem=0;

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
		/*Timer::instance->update();
		TabContainer::BroadcastToPool(&TabContainer::Draw);*/

		if(GetMessage(&msg,0,0,0))
		{
			if(msg.message==WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		
		
	}

	this->Close();
}

unsigned int TimerWin32::GetTime()
{
	return (unsigned int)timeGetTime();
}

void TimerWin32::update()
{
	lastFrameTime=currentFrameTime;
	currentFrameTime=GetTime();
	currentFrameDeltaTime=currentFrameTime-lastFrameTime;
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

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

#pragma message (LOCATION " this should go to common opengl part of the project cause there is no os-related call within")




void glCheckError()
{
	GLenum err=glGetError();
	if(err!=GL_NO_ERROR)
	{
		HGLRC currentContext=wglGetCurrentContext();
		HDC currentContextDC=wglGetCurrentDC();

		printf("OPENGL ERROR %d, HGLRC: %p, HDC: %p\n",err,currentContext,currentContextDC);

		__debugbreak();
	}
}



int simple_shader(const char* name,int shader_type, const char* shader_src)
{
	GLint compile_success = 0;
	GLchar message[1024];
	int len=0;
	int shader_id;

	shader_id = glCreateShader(shader_type);glCheckError();

	if(!shader_id)
	{
		printf("glCreateShader error for %s,%s\n",shader_type,shader_src);glCheckError();
		__debugbreak();
		return 0;
	}

	glShaderSource(shader_id, 1, &shader_src,NULL);glCheckError();
	glCompileShader(shader_id);glCheckError();
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_success);glCheckError();

	if (GL_FALSE==compile_success)
	{
		sprintf(message,"glCompileShader[%s] error:\n",name);
		glGetShaderInfoLog(shader_id, sizeof(message), &len, &message[strlen(message)]);
		MessageBox(0,message,"Engine",MB_OK|MB_ICONEXCLAMATION);
		__debugbreak();
	}

	return shader_id;
}


int create_program(const char* name,const char* vertexsh,const char* fragmentsh)
{
	GLint link_success=0;
	GLint program=0;
	GLint vertex_shader=0;
	GLint fragment_shader=0;
	GLchar message[1024]={0};
	GLint len=0;

	program = glCreateProgram();glCheckError();
	
	if(!program)
	{
		printf("glCreateProgram error for %s,%s\n",vertexsh,fragmentsh);
		__debugbreak();
		return 0;
	}

	vertex_shader=simple_shader(name,GL_VERTEX_SHADER, vertexsh);
	fragment_shader=simple_shader(name,GL_FRAGMENT_SHADER, fragmentsh);

	glAttachShader(program, vertex_shader);glCheckError();
	glAttachShader(program, fragment_shader);glCheckError();
	glLinkProgram(program);glCheckError();
	glGetProgramiv(program, GL_LINK_STATUS, &link_success);glCheckError();

	if (GL_FALSE==link_success)
	{
		printf("glLinkProgram error for %s\n",message);
		__debugbreak();
	}

	glGetProgramInfoLog(program,sizeof(message),&len,message);glCheckError();


	//print infos
	/*if(len && len<sizeof(message))
		printf("%s",message);*/
	

	return program;
}


//--------------------OpenGLShader-------------------------


ShaderInterface* OpenGLShader::Create(const char* name,const char* pix,const char* frag)
{
	int program=create_program(name,pix,frag);

	if(program)
	{
		ShaderInterface* shader=new OpenGLShader;

		shader->SetName(name);
		shader->SetProgram(program);
		shader->Use();
		shader->init();
		
		return shader;
	}
	else

		printf("error creating shader %s\n",name);

	return 0;
}



unsigned int& OpenGLShader::GetBufferObject()
{
	return vbo;
}

int OpenGLShader::GetProgram(){return program;}
void OpenGLShader::SetProgram(int p){program=p;}

int OpenGLShader::GetUniform(int slot,char* var)
{
	return glGetUniformLocation(slot,var);glCheckError();
}
int OpenGLShader::GetAttrib(int slot,char* var)
{
	return glGetAttribLocation(slot,var);glCheckError();
}

void OpenGLShader::SetProjectionMatrix(float* pm)
{
	this->SetMatrix4f(this->GetProjectionSlot(),pm);
}
void OpenGLShader::SetModelviewMatrix(float* mm)
{
	this->SetMatrix4f(this->GetModelviewSlot(),mm);
}

void OpenGLShader::SetMatrices(float* view,float* mdl)
{
	if(view)
		this->SetProjectionMatrix(view);

	if(mdl)
		this->SetModelviewMatrix(mdl);
}


void OpenGLShader::Use()
{
	glUseProgram(program);glCheckError();
}

const char* OpenGLShader::GetPixelShader(){return 0;}
const char* OpenGLShader::GetFragmentShader(){return 0;}

int OpenGLShader::init()
{
	mat4 m;

	int proj=GetProjectionSlot();
	int mdlv=GetModelviewSlot();

	bool bOk = this->SetMatrix4f(proj,m) && this->SetMatrix4f(mdlv,m);

	return bOk;
}

int OpenGLShader::GetAttribute(const char* attrib)
{
	int location=glGetAttribLocation(program,attrib);glCheckError();
	return location;
}

int OpenGLShader::GetUniform(const char* uniform)
{
	int location=glGetUniformLocation(program,uniform);glCheckError();
	return location;
}

int OpenGLShader::GetPositionSlot()
{
	return GetAttribute("position");
}
int OpenGLShader::GetColorSlot()
{
	return GetAttribute("color");
}
int OpenGLShader::GetProjectionSlot()
{
	return GetUniform("projection");
}
int OpenGLShader::GetModelviewSlot()
{
	return GetUniform("modelview");
}
int OpenGLShader::GetTexcoordSlot()
{
	return GetAttribute("texcoord");
}
int OpenGLShader::GetTextureSlot()
{
	return GetUniform("texture");
}
int OpenGLShader::GetMouseSlot()
{
	return GetUniform("mpos");
}
int OpenGLShader::GetLightposSlot()
{
	return GetUniform("lightpos");
}
int OpenGLShader::GetLightdiffSlot()
{
	return GetUniform("lightdiff");
}
int OpenGLShader::GetLightambSlot()
{
	return GetUniform("lightamb");
}
int OpenGLShader::GetNormalSlot()
{
	return GetAttribute("normal");
}
int OpenGLShader::GetHoveringSlot()
{
	return GetAttribute("hovered");
}

void OpenGLShader::SetSelectionColor(bool pick,void* ptr,vec2 mposNorm)
{
	int _mousepos=this->GetMouseSlot();
	int _ptrclr=this->GetUniform("ptrclr");

	if(_ptrclr>=0)
	{
		if(pick)
		{
			unsigned char* clr1=(unsigned char*)&ptr;

			float fcx=clr1[3]/255.0f;
			float fcy=clr1[2]/255.0f;
			float fcz=clr1[1]/255.0f;
			float fcw=clr1[0]/255.0f;

			glUniform4f(_ptrclr,fcx,fcy,fcz,fcw);
		}
		else glUniform4f(_ptrclr,0,0,0,0);

		if(_mousepos>=0)
			glUniform2f(_mousepos,mposNorm.x,mposNorm.y);
	}
}

bool OpenGLShader::SetMatrix4f(int slot,float* mtx)
{
	if(slot<0)
		return false;

	glUniformMatrix4fv(slot,1,0,mtx);glCheckError();
	return true;
}

void OpenGLShader::SetName(const char* n)
{
	name=n;
}
const char* OpenGLShader::GetName()
{
	return name;
}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

#pragma message (LOCATION " processNode should go to common part (maybe entities.h) of the project cause there is no os-related call within")
#pragma message (LOCATION " multiple opengl context needs glew32mx.lib")
#pragma message (LOCATION " TODO: move OpenGL bitmap render in the RendererViewportInterface cause is for the gui only")




///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


static const char* unlit_color_vert =
	"attribute vec3 position;\n"
	"varying vec3 vcolor;\n"
	"uniform mat4 modelview;\n"
	"uniform mat4 projection;\n"
	"uniform vec3 color;\n"
	"void main() {\n"
	"	 vcolor=color;"
	"    gl_Position = projection * modelview * vec4(position.xyz,1.0);\n"
	"}\n";

static const char* unlit_color_frag =
	"varying vec3 vcolor;\n"
	"uniform vec4 ptrclr;\n"
	"void main() {\n"
	"	 gl_FragColor = (ptrclr.x!=0.0 || ptrclr.y!=0.0 || ptrclr.z!=0.0 || ptrclr.w!=0.0) ? ptrclr : vec4(vcolor.xyz,1.0);"
	"}\n";


static const char* unlit_vert =
	"attribute vec4 position;\n"
	"attribute vec3 color;\n"
	"varying vec3 vcolor;\n"
	"uniform mat4 modelview;\n"
	"uniform mat4 projection;\n"
	"void main() {\n"
	"	 vcolor=color;"
	"    gl_Position = projection * modelview * vec4(position.xyz,1.0);\n"
	"}\n";

static const char* unlit_frag =
	"varying vec3 vcolor;\n"
	"void main() {\n"
	"    gl_FragColor = vec4(vcolor,1.0);\n"
	"}\n";

/////////////////////////////////////////////////////////////////////

static const char* unlit_texture_vs =
	"attribute vec4 position;\n"
	"attribute vec2 texcoord;\n"
	"attribute vec3 normal;\n"
	"uniform mat4 modelview;\n"
	"uniform mat4 projection;\n"
	"varying vec2 varying_texcoord;\n"
	"varying vec3 varying_normal;\n"
	"\n"
	"void main() {\n"
	"    varying_texcoord = texcoord;\n"
	"    varying_normal = normal;\n"
	"    gl_Position = projection * modelview * position;\n"
	"}\n";

static const char* unlit_texture_fs =
	"uniform sampler2D texture;\n"
	"varying vec2 varying_texcoord;\n"
	"varying vec3 varying_normal;\n"
	"uniform vec4 ptrclr;\n"
	"\n"
	"void main() {\n"
	"    gl_FragColor = (ptrclr.x!=0.0 || ptrclr.y!=0.0 || ptrclr.z!=0.0 || ptrclr.w!=0.0) ? ptrclr : texture2D(texture, varying_texcoord);\n"
	"}\n";

//////////////////////////////////////////////////////////////////////////////

static const char* texture_vertex_shaded_vert =
	"attribute vec3 position;\n"
	"attribute vec2 texcoord;\n"
	"attribute vec3 normal;\n"
	"uniform sampler2D texture;\n"
	"uniform bool textured;\n"
	"uniform mat4 modelview;\n"
	"uniform mat4 projection;\n"
	"uniform vec3 lightpos;\n"
	"uniform vec3 lightdiff;\n"
	"uniform vec3 lightamb;\n"
	"varying vec4 varying_color;\n"
	"varying vec4 varying_texcolor;\n"
	"vec3 lightdir;\n"
	"float cosine;\n"
	"float lambert;\n"
	"float luminosity;\n"
	"\n"
	"void main() {\n"
	"    vec4 pos = vec4(position,1.0);\n"
	"    varying_texcolor = textured ? texture2D(texture, texcoord) : vec4(1.0,1.0,1.0,1.0);\n"
	"    lightdir = lightpos - vec3(pos);\n"
	"	 cosine = dot(normal, normalize(lightdir));\n"
	"	 lambert = max(cosine, 0.0);\n"
	"    luminosity = 1.0 / (sqrt(length(lightdir)) * sqrt(length(lightdir)));\n"
	"    varying_color = vec4(lightdiff * lightamb * lambert * luminosity,1.0);\n"
	"    gl_Position = projection * modelview * pos;\n"
	"}\n";


static const char* texture_vertex_shaded_frag =
	"varying vec4 varying_texcolor;\n"
	"varying vec4 varying_color;\n"
	"\n"
	"void main() {\n"
	"    gl_FragColor = varying_color * varying_texcolor;\n"
	"}\n";




static const char* font_pixsh =
	"attribute vec4 position;\n"
	"attribute vec2 texcoord;\n"
	"varying vec2 v_texcoord;\n"
	"void main() {\n"
	"gl_Position = position;\n"
	"v_texcoord = texcoord;\n"
	"}\n";

static const char* font_frgsh =
	"varying vec2 v_texcoord;\n"
	"uniform sampler2D texture;\n"
	"uniform vec4 color;\n"
	"\n"
	"void main() {\n"
	"	gl_FragColor = texture2D(texture, v_texcoord);\n"
	"}\n";

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

/*
GLuint OpenGLRenderer::vertexArrayObject=0;
GLuint OpenGLRenderer::vertexBufferObject=0;
GLuint OpenGLRenderer::textureBufferObject=0;
GLuint OpenGLRenderer::indicesBufferObject=0;
GLuint OpenGLRenderer::frameBuffer=0;
GLuint OpenGLRenderer::textureColorbuffer=0;
GLuint OpenGLRenderer::textureRenderbuffer=0;
GLuint OpenGLRenderer::pixelBuffer=0;
GLuint OpenGLRenderer::renderBufferColor=0;
GLuint OpenGLRenderer::renderBufferDepth=0;*/

PFNWGLCHOOSEPIXELFORMATEXTPROC wglChoosePixelFormatARB = 0;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 0;

#ifndef GL_GLEXT_PROTOTYPES
extern PFNGLATTACHSHADERPROC glAttachShader=0;
extern PFNGLBINDBUFFERPROC glBindBuffer=0;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray=0;
extern PFNGLBUFFERDATAPROC glBufferData=0;
extern PFNGLCOMPILESHADERPROC glCompileShader=0;
extern PFNGLCREATEPROGRAMPROC glCreateProgram=0;
extern PFNGLCREATESHADERPROC glCreateShader=0;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers=0;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram=0;
extern PFNGLDELETESHADERPROC glDeleteShader=0;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays=0;
extern PFNGLDETACHSHADERPROC glDetachShader=0;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray=0;
extern PFNGLENABLEVERTEXARRAYATTRIBPROC glEnableVertexArrayAttrib=0;
extern PFNGLGENBUFFERSPROC glGenBuffers=0;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays=0;
extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation=0;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog=0;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv=0;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog=0;
extern PFNGLGETSHADERIVPROC glGetShaderiv=0;
extern PFNGLLINKPROGRAMPROC glLinkProgram=0;
extern PFNGLSHADERSOURCEPROC glShaderSource=0;
extern PFNGLUSEPROGRAMPROC glUseProgram=0;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer=0;
extern PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation=0;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation=0;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv=0;
extern PFNGLACTIVETEXTUREPROC glActiveTexture=0;
extern PFNGLUNIFORM1IPROC glUniform1i=0;
extern PFNGLUNIFORM1FPROC glUniform1f=0;
extern PFNGLGENERATEMIPMAPPROC glGenerateMipmap=0;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray=0;
extern PFNGLUNIFORM3FVPROC glUniform3fv=0;
extern PFNGLUNIFORM4FVPROC glUniform4fv=0;
extern PFNGLTEXBUFFERPROC glTexBuffer=0;
extern PFNGLTEXTUREBUFFERPROC glTextureBuffer=0;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData=0;
extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers=0;
extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers=0;
extern PFNGLREADNPIXELSPROC glReadnPixels=0;
extern PFNGLUNIFORM2FPROC glUniform2f=0;
extern PFNGLUNIFORM2FVPROC glUniform2fv=0;
extern PFNGLUNIFORM3FPROC glUniform3f=0;
extern PFNGLUNIFORM4FPROC glUniform4f=0;
extern PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB=0;
#endif


bool GLEW_INITED=false;

#define USE_COMMON_PIXELFORMAT_SYSTEM 1


vec3 rayStart;
vec3 rayEnd;


OpenGLRenderer::OpenGLRenderer(TabContainer* _tabContainer):
tabContainer(_tabContainer)
{
	hglrc=0;
}

OpenGLRenderer::~OpenGLRenderer()
{
}

char* OpenGLRenderer::Name()
{
	return 0;
}

void OpenGLRenderer::Create(HWND hwnd)
{
	hdc=GetDC(hwnd);

	RECT r;
	GetClientRect(hwnd,&r);

	int width=(int)(r.right-r.left);
	int height=(int)(r.bottom-r.top);

	DWORD error=0;


	int pixelFormat;

	for(int i=0;i<1;i++)
	{
		if(!hdc)
			MessageBox(0,"Getting Device Context","GetDC",MB_OK|MB_ICONEXCLAMATION);

		PIXELFORMATDESCRIPTOR pfd={0};
		pfd.nVersion=1;
		pfd.nSize=sizeof(PIXELFORMATDESCRIPTOR);
		pfd.dwFlags=PFD_DOUBLEBUFFER|PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL;
		pfd.iPixelType=PFD_TYPE_RGBA;
		pfd.cColorBits=32;
		pfd.cDepthBits=32;
		pfd.cStencilBits=32;

		pixelFormat = ChoosePixelFormat(hdc,&pfd);

		error=GetLastError();

		if(error!=NO_ERROR && error!=ERROR_OLD_WIN_VERSION)
			__debugbreak();

		if(pixelFormat==0)
			__debugbreak();

		if(!SetPixelFormat(hdc,pixelFormat,&pfd))
			__debugbreak();

		if(!(hglrc = wglCreateContext(hdc)))
			__debugbreak();

		if(!wglMakeCurrent(hdc,hglrc))
			__debugbreak();

		if(!wglChoosePixelFormatARB)wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATEXTPROC) wglGetProcAddress("wglChoosePixelFormatARB");
		if(!wglCreateContextAttribsARB)wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");

		#ifndef GL_GLEXT_PROTOTYPES

		if(!glAttachShader)glAttachShader = (PFNGLATTACHSHADERPROC) wglGetProcAddress("glAttachShader");
		if(!glBindBuffer)glBindBuffer = (PFNGLBINDBUFFERPROC) wglGetProcAddress("glBindBuffer");
		if(!glBindVertexArray) glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) wglGetProcAddress("glBindVertexArray");
		if(!glBufferData) glBufferData = (PFNGLBUFFERDATAPROC) wglGetProcAddress("glBufferData");
		if(!glCompileShader) glCompileShader = (PFNGLCOMPILESHADERPROC) wglGetProcAddress("glCompileShader");
		if(!glCreateProgram) glCreateProgram = (PFNGLCREATEPROGRAMPROC) wglGetProcAddress("glCreateProgram");
		if(!glCreateShader) glCreateShader = (PFNGLCREATESHADERPROC) wglGetProcAddress("glCreateShader");
		if(!glDeleteBuffers) glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) wglGetProcAddress("glDeleteBuffers");
		if(!glDeleteProgram) glDeleteProgram = (PFNGLDELETEPROGRAMPROC) wglGetProcAddress("glDeleteProgram");
		if(!glDeleteShader) glDeleteShader = (PFNGLDELETESHADERPROC) wglGetProcAddress("glDeleteShader");
		if(!glDeleteVertexArrays) glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC) wglGetProcAddress("glDeleteVertexArrays");
		if(!glDetachShader) glDetachShader = (PFNGLDETACHSHADERPROC) wglGetProcAddress("glDetachShader");
		if(!glEnableVertexAttribArray) glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) wglGetProcAddress("glEnableVertexAttribArray");
		if(!glEnableVertexArrayAttrib) glEnableVertexArrayAttrib = (PFNGLENABLEVERTEXARRAYATTRIBPROC) wglGetProcAddress("glEnableVertexArrayAttrib");
		if(!glGenBuffers) glGenBuffers = (PFNGLGENBUFFERSPROC) wglGetProcAddress("glGenBuffers");
		if(!glGenVertexArrays) glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) wglGetProcAddress("glGenVertexArrays");
		if(!glGetAttribLocation) glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) wglGetProcAddress("glGetAttribLocation");
		if(!glGetProgramInfoLog) glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) wglGetProcAddress("glGetProgramInfoLog");
		if(!glGetProgramiv) glGetProgramiv = (PFNGLGETPROGRAMIVPROC) wglGetProcAddress("glGetProgramiv");
		if(!glGetShaderInfoLog) glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) wglGetProcAddress("glGetShaderInfoLog");
		if(!glGetShaderiv) glGetShaderiv = (PFNGLGETSHADERIVPROC) wglGetProcAddress("glGetShaderiv");
		if(!glLinkProgram) glLinkProgram = (PFNGLLINKPROGRAMPROC) wglGetProcAddress("glLinkProgram");
		if(!glShaderSource) glShaderSource = (PFNGLSHADERSOURCEPROC) wglGetProcAddress("glShaderSource");
		if(!glUseProgram) glUseProgram = (PFNGLUSEPROGRAMPROC) wglGetProcAddress("glUseProgram");
		if(!glVertexAttribPointer) glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) wglGetProcAddress("glVertexAttribPointer");
		if(!glBindAttribLocation) glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) wglGetProcAddress("glBindAttribLocation");
		if(!glGetUniformLocation) glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) wglGetProcAddress("glGetUniformLocation");
		if(!glUniformMatrix4fv) glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) wglGetProcAddress("glUniformMatrix4fv");
		if(!glActiveTexture) glActiveTexture = (PFNGLACTIVETEXTUREPROC) wglGetProcAddress("glActiveTexture");
		if(!glUniform1i) glUniform1i = (PFNGLUNIFORM1IPROC) wglGetProcAddress("glUniform1i");
		if(!glUniform1f) glUniform1f = (PFNGLUNIFORM1FPROC) wglGetProcAddress("glUniform1f");
		if(!glUniform3f) glUniform3f = (PFNGLUNIFORM3FPROC) wglGetProcAddress("glUniform3f");
		if(!glGenerateMipmap) glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) wglGetProcAddress("glGenerateMipmap");
		if(!glDisableVertexAttribArray) glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) wglGetProcAddress("glDisableVertexAttribArray");
		if(!glUniform3fv) glUniform3fv = (PFNGLUNIFORM3FVPROC) wglGetProcAddress("glUniform3fv");
		if(!glUniform4fv) glUniform4fv = (PFNGLUNIFORM4FVPROC) wglGetProcAddress("glUniform4fv");
		if(!glTexBuffer) glTexBuffer = (PFNGLTEXBUFFERPROC) wglGetProcAddress("glTexBuffer");
		if(!glTextureBuffer) glTextureBuffer = (PFNGLTEXTUREBUFFERPROC) wglGetProcAddress("glTextureBuffer");
		if(!glBufferSubData) glBufferSubData = (PFNGLBUFFERSUBDATAPROC) wglGetProcAddress("glBufferSubData");
		if(!glGenFramebuffers)glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) wglGetProcAddress("glGenFramebuffers");
		if(!glGenRenderbuffers)glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) wglGetProcAddress("glGenRenderbuffers");
		/*if(!glBufferSubData)glViewportIndexedf = (PFNGLVIEWPORTINDEXEDFPROC) wglGetProcAddress("glViewportIndexedf");
		if(!glAddSwapHintRectWIN)glAddSwapHintRectWIN = (PFNGLADDSWAPHINTRECTWINPROC) wglGetProcAddress("glAddSwapHintRectWIN");
		if(!glBindFramebuffer)glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) wglGetProcAddress("glBindFramebuffer");
		if(!glNamedRenderbufferStorage)glNamedRenderbufferStorage = (PFNGLNAMEDRENDERBUFFERSTORAGEPROC) wglGetProcAddress("glNamedRenderbufferStorage");
		if(!glRenderbufferStorage)glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) wglGetProcAddress("glRenderbufferStorage");
		if(!glBindRenderbuffer)glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) wglGetProcAddress("glBindRenderbuffer");
		if(!glFramebufferRenderbuffer)glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) wglGetProcAddress("glFramebufferRenderbuffer");
		if(!glBlitFramebuffer)glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC) wglGetProcAddress("glBlitFramebuffer");
		if(!glCheckFramebufferStatus)glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) wglGetProcAddress("glCheckFramebufferStatus");
		if(!glDrawBuffers)glDrawBuffers = (PFNGLDRAWBUFFERSPROC) wglGetProcAddress("glDrawBuffers");
		if(!glBlitNamedFramebuffer) glBlitNamedFramebuffer = (PFNGLBLITNAMEDFRAMEBUFFERPROC) wglGetProcAddress("glBlitNamedFramebuffer");
		if(!glFramebufferTexture)glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC)wglGetProcAddress("glFramebufferTexture");
		if(!glFramebufferTexture2D)glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) wglGetProcAddress("glFramebufferTexture2D");*/
		if(!glReadnPixels)glReadnPixels = (PFNGLREADNPIXELSPROC) wglGetProcAddress("glReadnPixels");
		if(!glUniform2f)glUniform2f = (PFNGLUNIFORM2FPROC) wglGetProcAddress("glUniform2f");
		if(!glUniform2fv)glUniform2fv = (PFNGLUNIFORM2FVPROC) wglGetProcAddress("glUniform2fv");
		if(!glUniform3f)glUniform3f = (PFNGLUNIFORM3FPROC) wglGetProcAddress("glUniform3f");
		if(!glUniform4f)glUniform4f = (PFNGLUNIFORM4FPROC) wglGetProcAddress("glUniform4f");
		if(!wglGetPixelFormatAttribivARB)wglGetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC) wglGetProcAddress("wglGetPixelFormatAttribivARB");
#endif

		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hglrc);
	}
	
	const int pixelFormatAttribList[] = 
	{
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_DEPTH_BITS_ARB, 32,
		WGL_STENCIL_BITS_ARB, 32,
		//WGL_SWAP_COPY_ARB,GL_TRUE,        //End
		0
	};

	UINT numFormats;

	if(!wglChoosePixelFormatARB(hdc, pixelFormatAttribList, NULL, 1, &pixelFormat, &numFormats))
		MessageBox(0,"wglChoosePixelFormatARB fails","Engine",MB_OK|MB_ICONEXCLAMATION);
	

	const int versionAttribList[] = 
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB,1,
		WGL_CONTEXT_MINOR_VERSION_ARB,0, 
		0,        //End
	};

	if(!(hglrc = wglCreateContextAttribsARB(hdc, 0, versionAttribList)))
		MessageBox(0,"wglCreateContextAttribsARB fails","Engine",MB_OK|MB_ICONEXCLAMATION);


	if(hglrc)
		printf("TABCONTAINER: %p, HGLRC: %p, HDC: %p\n",this->tabContainer,hglrc,hdc);
	
	if(!wglMakeCurrent(hdc,hglrc))
		__debugbreak();

	//if(!vertexArrayObject)
	{
		{
			glGenFramebuffers(1,&frameBuffer);glCheckError();
			
			glGenTextures(1,&textureColorbuffer);glCheckError();
			glGenTextures(1,&textureRenderbuffer);glCheckError();

			glGenRenderbuffers(1,&renderBufferColor);glCheckError();
			glGenRenderbuffers(1,&renderBufferDepth);glCheckError();
		}
		
		glGenVertexArrays(1, &vertexArrayObject);glCheckError();
		glBindVertexArray(vertexArrayObject);glCheckError();

		glGenBuffers(1,&vertexBufferObject);glCheckError();
		glGenBuffers(1,&textureBufferObject);glCheckError();
		/*
		//glGenBuffers(1,&indicesBufferObject);

		glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);
		//glBindBuffer(GL_ARRAY_BUFFER,indicesBufferObject);

		glBufferData(GL_ARRAY_BUFFER,100000,0,GL_DYNAMIC_DRAW);*/


		glGenBuffers(1, &pixelBuffer);
	}

	printf("Status: Using GL %s\n", glGetString(GL_VERSION));
	printf("Status: GLSL ver %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));

	this->unlit=OpenGLShader::Create("unlit",unlit_vert,unlit_frag);
	this->unlit_color=OpenGLShader::Create("unlit_color",unlit_color_vert,unlit_color_frag);
	this->unlit_texture=OpenGLShader::Create("unlit_texture",unlit_texture_vs,unlit_texture_fs);
	this->font=OpenGLShader::Create("font",font_pixsh,font_frgsh);
	this->shaded_texture=OpenGLShader::Create("shaded_texture",texture_vertex_shaded_vert,texture_vertex_shaded_frag);

	this->shaders.push_back(this->unlit);
	this->shaders.push_back(this->unlit_color);
	this->shaders.push_back(this->unlit_texture);
	this->shaders.push_back(this->font);
	this->shaders.push_back(this->shaded_texture);
}


void OpenGLRenderer::ChangeContext()
{
	if(!hglrc || !hdc)
	{
		__debugbreak();
		return;
	}

	if(hglrc != wglGetCurrentContext() || hdc!=wglGetCurrentDC())
	{
		if(!wglMakeCurrent(hdc,hglrc))
			__debugbreak();
	}
}




void OpenGLRenderer::draw(Light*)
{

}

void OpenGLRenderer::draw(vec2)
{

}

void OpenGLRenderer::draw(Gizmo* gizmo)
{
	this->draw(vec3(0,0,0),vec3(10,0,0),vec3(1,0,0));
	this->draw(vec3(0,0,0),vec3(0,10,0),vec3(0,1,0));
	this->draw(vec3(0,0,0),vec3(0,0,10),vec3(0,0,1));
}

void OpenGLRenderer::draw(Piped* piped)
{
	
}


void OpenGLRenderer::draw(vec3 point,float psize,vec3 col)
{
	ShaderInterface* shader=this->unlit_color;

	if(!shader)
		return;

	shader->Use();

	////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());

	shader->SetSelectionColor(this->picking,0,vec2(this->tabContainer->mousex/this->tabContainer->width,this->tabContainer->mousey/this->tabContainer->height));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glPointSize(psize);

	int ps=shader->GetPositionSlot();
	int uniform_color=shader->GetUniform("color");

	if(uniform_color>=0)
	{glUniform3fv(uniform_color,1,col);glCheckError();}

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER,3*sizeof(float),point.v,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(ps);glCheckError();
	glVertexAttribPointer(ps, 3, GL_FLOAT, GL_FALSE, 0,0);glCheckError();
		
	glDrawArrays(GL_POINTS,0,1);glCheckError();

	glDisableVertexAttribArray(ps);glCheckError();

	glPointSize(1.0f);
	glDisable(GL_PROGRAM_POINT_SIZE);
	glDisable(GL_DEPTH_TEST);
}

void OpenGLRenderer::draw(vec4 rect)
{
	ShaderInterface* shader=this->unlit_color;

	int position_slot=-1;
	int modelview_slot=-1;

	if(shader)
	{
		shader->Use();
		
		////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());
		int mdl=shader->GetModelviewSlot();
		int view=shader->GetProjectionSlot();
		int ptrclr=shader->GetUniform("ptrclr");

		if(ptrclr>=0)
			glUniform4f(ptrclr,0,0,0,0);

	}
	else
		return;

	if(position_slot<0)
		return;

	float dx=rect[2]-rect[0];
	float dy=rect[3]-rect[1];

	float data[]=
	{
		rect[0],	rect[1],		0,
		rect[0],	rect[1]+dy,		0,
		rect[0]+dx,	rect[1]+dy,		0,
		rect[0]+dx,	rect[1],		0,
		rect[0],	rect[1],		0
	};

	glVertexAttribPointer(position_slot, 3, GL_FLOAT, GL_FALSE, 0,data);
	glEnableVertexAttribArray(position_slot);
	glDrawArrays(GL_LINE_STRIP, 0, 5);
}


void OpenGLRenderer::draw(mat4 mtx,float size,vec3 color)
{
	ShaderInterface* shader=this->unlit_color;

	if(!shader)
		return;

	shader->Use();
	////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());

	int mdl=shader->GetModelviewSlot();
	int view=shader->GetProjectionSlot();
	int ptrclr=shader->GetUniform("ptrclr");

	if(ptrclr>=0)
		glUniform4f(ptrclr,0,0,0,0);

	/*MatrixStack::Push();
	MatrixStack::Multiply(mtx);*/

	vec3 axes[6];

	vec3 zero=mtx.transform(0,0,0);

	axes[0]=axes[2]=axes[4]=zero;

	mtx.axes(axes[1],axes[3],axes[5]);

	glEnable(GL_DEPTH_TEST);

	int pos=shader->GetPositionSlot();
	int col=shader->GetUniform("color");

	glUniform3fv(col,1,color);

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER,18*sizeof(float),axes,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(pos);glCheckError();

	glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();

	glDrawArrays(GL_LINES,0,6);glCheckError();
	//glDrawElements(GL_TRIANGLES,3,GL_UNSIGNED_INT,)

	glDisableVertexAttribArray(pos);


	glDisable(GL_DEPTH_TEST);

	

	//MatrixStack::Pop();
}

void OpenGLRenderer::draw(AABB aabb,vec3 color)
{
	ShaderInterface* shader=this->unlit_color;

	if(!shader)
		return;

	shader->Use();

	////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());

	shader->SetSelectionColor(false,0,vec2(this->tabContainer->mousex/this->tabContainer->width,this->tabContainer->mousey/this->tabContainer->height));

	vec3 &a=aabb.a;
	vec3 &b=aabb.b;

	float dx=b.x-a.x;
	float dy=b.y-a.y;
	float dz=b.z-a.z;

	if(dx<0.00000001f && dy<0.00000001f && dz<0.00000001f)
		return;

	float parallelepiped[72]=
	{
		//lower quad
		a.x,a.y,a.z,		a.x+dx,a.y,a.z,
		a.x+dx,a.y,a.z,		a.x+dx,a.y+dy,a.z, 
		a.x+dx,a.y+dy,a.z,	a.x,a.y+dy,a.z,
		a.x,a.y+dy,a.z,     a.x,a.y,a.z,

		//upper quad
		a.x,a.y,a.z+dz,			a.x+dx,a.y,a.z+dz,
		a.x+dx,a.y,a.z+dz,		a.x+dx,a.y+dy,a.z+dz, 
		a.x+dx,a.y+dy,a.z+dz,	a.x,a.y+dy,a.z+dz,
		a.x,a.y+dy,a.z+dz,		a.x,a.y,a.z+dz,

		//staffs
		a.x,a.y,a.z,		a.x,a.y,a.z+dz,
		a.x+dx,a.y,a.z,		a.x+dx,a.y,a.z+dz,
		a.x+dx,a.y+dy,a.z,	a.x+dx,a.y+dy,a.z+dz,
		a.x,a.y+dy,a.z,   	a.x,a.y+dy,a.z+dz
	};

	glEnable(GL_DEPTH_TEST);

	int pos=shader->GetPositionSlot();
	int col=shader->GetUniform("color");

	glUniform3fv(col,1,color);

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER,72*sizeof(float),parallelepiped,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(pos);glCheckError();

	glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();

	glDrawArrays(GL_LINES,0,24);glCheckError();
	//glDrawElements(GL_TRIANGLES,3,GL_UNSIGNED_INT,)

	glDisableVertexAttribArray(pos);

	glBindBuffer(GL_ARRAY_BUFFER,0);

	glDisable(GL_DEPTH_TEST);
}

void OpenGLRenderer::draw(vec3 a,vec3 b,vec3 color)
{
	ShaderInterface* shader=this->unlit_color;

	if(!shader)
		return;

	float line[]=
	{
		a[0],a[1],a[2],
		b[0],b[1],b[2], 
	};
	
	shader->Use();

	shader->SetMatrices(MatrixStack::GetViewMatrix()*MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelMatrix());

	shader->SetSelectionColor(false,0,vec2(this->tabContainer->mousex/this->tabContainer->width,this->tabContainer->mousey/this->tabContainer->height));
	
	glEnable(GL_DEPTH_TEST);

	int pos=shader->GetPositionSlot();
	int col=shader->GetUniform("color");
	
	glUniform3fv(col,1,color);

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER,6*sizeof(float),line,GL_DYNAMIC_DRAW);
	
	glEnableVertexAttribArray(pos);glCheckError();

	glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();

	glDrawArrays(GL_LINES,0,2);glCheckError();
	//glDrawElements(GL_TRIANGLES,3,GL_UNSIGNED_INT,)

	glDisableVertexAttribArray(pos);


	glDisable(GL_DEPTH_TEST);

}

void OpenGLRenderer::draw(char* text,float x,float y,float width,float height,float sizex,float sizey,float* color4)
{
	ShaderInterface* shader=0;//line_color_shader

	if(!shader || !text)
		return;

	shader->Use();

	////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());

	/*Font* font=FontManager::Instance()->Head()->Data();

	this->draw(font,text,x,y,width,height,sizex,sizey,color4);*/
}


/*
void OpenGLFixedRenderer::draw(Window* w)
{
	line_shader->Use();

	int position_slot=line_shader->GetPositionSlot();

	vec2& pos=w->window_pos;
	vec2& dim=w->window_dim;

	float data[]=
	{
		pos[0]-dim[0]/2,	pos[1]+dim[1]/2,		0,//low-left
		pos[0]+dim[0]/2,	pos[1]+dim[1]/2,		0,//low-right
		pos[0]+dim[0]/2,	pos[1]-dim[1]/2,		0,//up-right
		pos[0]-dim[0]/2,	pos[1]-dim[1]/2,		0,//up-left
		pos[0]-dim[0]/2,	pos[1]+dim[1]/2,		0,//low-left
	};

	glVertexAttribPointer(position_slot, 3, GL_FLOAT, GL_FALSE, 0,data);
	glEnableVertexAttribArray(position_slot);
	glDrawArrays(GL_LINE_STRIP, 0, 5);
}*/

/*
void OpenGLFixedRenderer::draw(WindowManager* windowManager)
{
	this->line_shader->Use();

	mat4 mat;
	mat.ortho(-0.5f,0.5f,-0.5f,0.5f,0,10);
	this->line_shader->SetMatrix4f(this->line_shader->GetProjectionSlot(),mat);


	glDisable(GL_CULL_FACE);

	float width=(float)KernelServer::Instance()->GetApp()->GetWidth();
	float height=(float)KernelServer::Instance()->GetApp()->GetHeight();

	for(Node<Window*> *node=windowManager->Head();node;node=node->Next())
		this->draw(node->Data());

	glEnable(GL_CULL_FACE);
}*/

/*
void OpenGLFixedRenderer::draw(EntityManager& entityManager)
{
	glDisable(GL_CULL_FACE);

	//entityManager.draw();

	glEnable(GL_CULL_FACE);
}
*/



/*
void OpenGLFixedRenderer::draw(Font* font,char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4) 
{
	//https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_01

	if(!font || !phrase)
		return;

	ShaderInterface* shader=font_shader;

	if(!shader)
		return;

	shader->Use();

	GLuint tid;

	glEnable(GL_BLEND);glCheckError();
	glBlendFunc(GL_CONSTANT_COLOR, GL_ONE_MINUS_SRC_COLOR);

	if(color4)
		glBlendColor(color4[0],color4[1],color4[2],color4[3]);
	else
		glBlendColor(1,1,1,1);

	glActiveTexture(GL_TEXTURE0);glCheckError();
	glGenTextures(1,(GLuint*)&tid);glCheckError();
	glBindTexture(GL_TEXTURE_2D,tid);glCheckError();

	float sx=2.0f/width;
	float sy=2.0f/height;

	char* p=phrase;

	while(*p)
	{
		int err=font->loadchar(*p,sizex,sizey,width,height);

		if(!err)
		{
			unsigned char*	_buf=(unsigned char*)font->GetBuffer();
			int				_width=font->GetWidth();
			int				_rows=font->GetRows();
			float			_left=(float)font->GetLeft();
			float			_top=(float)font->GetTop();
			int				_advx=font->GetAdvanceX();
			int				_advy=font->GetAdvanceY();

			float			x2 = x + _left * sx;
			float			y2 = -y - _top * sy;
			float			w = _width * sx;
			float			h = _rows * sy;

			float box[] = {x2,-y2,0,1,x2 + w,-y2,0,1,x2,-y2-h,0,1,x2+w,-y2-h,0,1};
			float uv[] = {0,0,1,0,0,1,1,1};

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);glCheckError();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);glCheckError();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);glCheckError();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);glCheckError();

			glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,_width,_rows,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,_buf);glCheckError();

			glUniform1i(shader->GetTextureSlot(), 0);glCheckError();

			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			glEnableVertexAttribArray(shader->GetPositionSlot());glCheckError();
			glEnableVertexAttribArray(shader->GetTexcoordSlot());glCheckError();
			glVertexAttribPointer(shader->GetPositionSlot(), 4, GL_FLOAT, GL_FALSE,0,box);glCheckError();
			glVertexAttribPointer(shader->GetTexcoordSlot(), 2, GL_FLOAT, GL_FALSE,0,uv);glCheckError();
			glDrawArrays(GL_TRIANGLE_STRIP,0,4);glCheckError();
			glDisableVertexAttribArray(shader->GetPositionSlot());glCheckError();
			glDisableVertexAttribArray(shader->GetTexcoordSlot());glCheckError();

			x += (_advx >> 6) * sx;
			y += (_advy >> 6) * sy;
		}

		p++;
	}

	glDeleteTextures(1,(GLuint*)&tid);glCheckError();

	glDisable(GL_BLEND);glCheckError();
}*/


void OpenGLRenderer::draw(Texture* _t)
{
	return;
	ShaderInterface* shader=0;//unlit_texture

	Texture* texture=(Texture*)_t;
	//TextureFile* texture=(TextureFile*)_t;
	

	if(!shader || !texture->GetBuffer())
		return;

	shader->Use();

	const GLfloat rect_data[] = 
	{
		-1.0f, -1.0f,
		1.0f, -1.0f,
		-1.0f,  1.0f,
		1.0f,  1.0f
	};

	const GLfloat uv_data[] = 
	{
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f,  1.0f,
		0.0f,  0.0f,
	};

	int width=texture->GetWidth();
	int height=texture->GetHeight();
	void* buf=texture->GetBuffer();
	int  size=texture->GetSize();

	if(width<100)
		return;

	GLuint tid;
	glGenTextures(1,&tid);glCheckError();

	glActiveTexture(GL_TEXTURE0);glCheckError();
	glBindTexture(GL_TEXTURE_2D,tid);glCheckError();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glCheckError();

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,width,height, 0, GL_RGBA, GL_UNSIGNED_BYTE,buf);glCheckError();

	mat4 mat;

	int projection=shader->GetProjectionSlot();
	int modelview=shader->GetModelviewSlot();
	int textureslot=shader->GetTextureSlot();
	int position=shader->GetPositionSlot();
	int texcoord=shader->GetTexcoordSlot();

	mat.ortho(-3,3,-3,3,0,1000);
	glUniformMatrix4fv(projection,1,0,mat);glCheckError();
	mat.identity();
	glUniformMatrix4fv(modelview,1,0,mat);glCheckError();

	glUniform1i(textureslot, 0);glCheckError();
	glEnableVertexAttribArray(position);glCheckError();
	glEnableVertexAttribArray(texcoord);glCheckError();
	glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE,0,rect_data);glCheckError();
	glVertexAttribPointer(texcoord, 2, GL_FLOAT, GL_FALSE,0,uv_data);glCheckError();
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);glCheckError();

	glDisableVertexAttribArray(position);glCheckError();
	glDisableVertexAttribArray(texcoord);glCheckError();

	glDeleteTextures(1,&tid);glCheckError();
}

void OpenGLRenderer::draw(Mesh* mesh,std::vector<GLuint>& textureIndices,int texture_slot,int texcoord_slot)
{
	for(int i=0;i<(int)mesh->mesh_materials.size();i++)
	{
		for(int j=0;j<(int)mesh->mesh_materials[i]->textures.size() && !textureIndices.size();j++)
		{
			Texture* texture=mesh->mesh_materials[i]->textures[j];
			
			int		texture_width=texture->GetWidth();
			int		texture_height=texture->GetHeight();
			void	*texture_buffer=texture->GetBuffer();

			if(texture_buffer)
			{
				GLuint tid;
				glGenTextures(1,&tid);glCheckError();
				textureIndices.push_back(tid);

				glActiveTexture(GL_TEXTURE0);glCheckError();
				glBindTexture(GL_TEXTURE_2D,tid);glCheckError();

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);glCheckError();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);glCheckError();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);glCheckError();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glCheckError();

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,texture_width,texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE,texture_buffer);glCheckError();

				glBindBuffer(GL_ARRAY_BUFFER,textureBufferObject);
				glBufferData(GL_ARRAY_BUFFER,mesh->mesh_ntexcoord*2*sizeof(float),mesh->mesh_texcoord,GL_DYNAMIC_DRAW);

				glUniform1i(texture_slot, 0);glCheckError();
				glEnableVertexAttribArray(texcoord_slot);glCheckError();
				glVertexAttribPointer(texcoord_slot,2,GL_FLOAT,GL_FALSE,0,0);glCheckError();

				glBindBuffer(GL_ARRAY_BUFFER,0);
			}
		}
	}
}

void OpenGLRenderer::draw(Mesh* mesh)
{
	drawUnlitTextured(mesh);
}

void OpenGLRenderer::drawUnlitTextured(Mesh* mesh)
{
	ShaderInterface* shader = mesh->mesh_materials.size() ? this->unlit_texture : this->unlit_color;

	if(!shader || !mesh)
		return;

	vec3 lightpos(0,200,-100);

	if(shader==this->shaded_texture)
		this->draw(lightpos,5);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glFrontFace(mesh->mesh_isCCW ? GL_CCW : GL_CW);
	//glFrontFace(GL_CCW);

	shader->Use();

	shader->SetMatrices(MatrixStack::GetViewMatrix()*MatrixStack::GetProjectionMatrix(),mesh->entity->world);


	shader->SetSelectionColor(this->picking,mesh->entity,vec2(this->tabContainer->mousex/this->tabContainer->width,this->tabContainer->mousey/this->tabContainer->height));
	
	int position_slot = shader->GetPositionSlot();
	int texcoord_slot = shader->GetTexcoordSlot();
	int texture_slot = shader->GetTextureSlot();
	int normal_slot = shader->GetNormalSlot();

	std::vector<unsigned int> textureIndices;

	if(mesh->mesh_materials.size())
	{
		draw(mesh,textureIndices,texture_slot,texcoord_slot);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER,mesh->mesh_ncontrolpoints*3*sizeof(float),mesh->mesh_controlpoints,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(position_slot);glCheckError();
	if(normal_slot>=0)glEnableVertexAttribArray(normal_slot);glCheckError();
	glVertexAttribPointer(position_slot,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();
	if(normal_slot>=0)glVertexAttribPointer(normal_slot,3,GL_FLOAT,GL_FALSE,0,mesh->mesh_normals);glCheckError();

	glBindBuffer(GL_ARRAY_BUFFER,0);

	glDrawArrays(GL_TRIANGLES,0,mesh->mesh_npolygons*3);glCheckError();
	//glDrawElements(GL_TRIANGLES,mesh->mesh_nvertexindices*3,GL_UNSIGNED_INT,mesh->mesh_vertexindices);glCheckError();


	glDisableVertexAttribArray(position_slot);glCheckError();
	if(normal_slot>=0)glDisableVertexAttribArray(normal_slot);glCheckError();

	for(int i=textureIndices.size();i>0;i--)
	{
		glDeleteTextures(1,&textureIndices[i-1]);glCheckError();
		glDisableVertexAttribArray(texcoord_slot);glCheckError();
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}


void OpenGLRenderer::draw(Skin* skin)
{
	ShaderInterface* shader = skin->mesh_materials.size() ? this->unlit_texture : this->unlit_color;

	if(!skin || !skin->skin_vertexcache || !shader)
	{
		this->drawUnlitTextured(skin);
		return;
	}

	vec3 lightpos(0,200,-100);

	if(shader==this->shaded_texture)
		this->draw(lightpos,5);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glFrontFace(skin->mesh_isCCW ? GL_CCW : GL_CW);
	//glFrontFace(GL_CW);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	shader->Use();

	shader->SetMatrices(MatrixStack::GetViewMatrix()*MatrixStack::GetProjectionMatrix(),skin->entity->local);


	shader->SetSelectionColor(this->picking,skin->entity,vec2(this->tabContainer->mousex/this->tabContainer->width,this->tabContainer->mousey/this->tabContainer->height));

	int position_slot = shader->GetPositionSlot();
	int texcoord_slot = shader->GetTexcoordSlot();
	int texture_slot = shader->GetTextureSlot();
	int lightpos_slot = shader->GetLightposSlot();
	int lightdiff_slot = shader->GetLightdiffSlot();
	int lightamb_slot = shader->GetLightambSlot();
	int normal_slot = shader->GetNormalSlot();
	int color_slot = shader->GetColorSlot();
	
	std::vector<GLuint> textureIndices;

	draw(skin,textureIndices,texture_slot,texcoord_slot);

	int uniformTextured=shader->GetUniform("textured");

	glUniform1f(uniformTextured,(GLfloat)textureIndices.size());glCheckError();

	if(lightdiff_slot>=0)glEnableVertexAttribArray(lightdiff_slot);glCheckError();
	if(lightamb_slot>=0)glEnableVertexAttribArray(lightamb_slot);glCheckError();
	if(lightpos_slot>=0)glEnableVertexAttribArray(lightpos_slot);glCheckError();

	float v[3]={255,255,255};

	if(lightdiff_slot>=0)glUniform3f(lightdiff_slot,v[0],v[1],v[2]);glCheckError();
	if(lightamb_slot>=0)glUniform3f(lightamb_slot,v[0]+128,v[1]+128,v[2]+255);glCheckError();
	if(lightpos_slot>=0)glUniform3fv(lightpos_slot,1,lightpos);glCheckError();
	
	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER,skin->mesh_ncontrolpoints*3*sizeof(float),skin->skin_vertexcache,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(position_slot);glCheckError();
	if(normal_slot>=0)glEnableVertexAttribArray(normal_slot);glCheckError();
	glVertexAttribPointer(position_slot,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();
	if(normal_slot>=0)glVertexAttribPointer(normal_slot,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();

	glDrawArrays(GL_TRIANGLES,0,skin->mesh_npolygons*3);glCheckError();
	//glDrawElements(GL_TRIANGLES,skin->mesh_ntriangleindices*3,GL_UNSIGNED_INT,skin->mesh_triangleindices);glCheckError();

	if(lightdiff_slot>=0)glDisableVertexAttribArray(position_slot);glCheckError();
	if(lightdiff_slot>=0)glDisableVertexAttribArray(normal_slot);glCheckError();

	if(lightdiff_slot>=0)glDisableVertexAttribArray(lightdiff_slot);glCheckError();
	if(lightamb_slot>=0)glDisableVertexAttribArray(lightamb_slot);glCheckError();
	if(lightpos_slot>=0)glDisableVertexAttribArray(lightpos_slot);glCheckError();
	
	for(int i=textureIndices.size();i>0;i--)
	{
		glDeleteTextures(1,&textureIndices[i-1]);glCheckError();
		glDisableVertexAttribArray(texcoord_slot);glCheckError();
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}

void OpenGLRenderer::draw(Camera*)
{

}

void OpenGLRenderer::draw(Bone* bone)
{
	ShaderInterface* shader=this->unlit_color;

	if(!shader)
		return;

	vec3 &a=bone->entity->parent->world.position();
	vec3 &b=bone->entity->world.position();

	float line[]=
	{
		a[0],a[1],a[2],
		b[0],b[1],b[2], 
	};

	shader->Use();

	shader->SetMatrices(MatrixStack::GetViewMatrix()*MatrixStack::GetProjectionMatrix(),mat4());

	shader->SetSelectionColor(this->picking,bone->entity,vec2(this->tabContainer->mousex/this->tabContainer->width,this->tabContainer->mousey/this->tabContainer->height));

	glEnable(GL_DEPTH_TEST);

	int pos=shader->GetPositionSlot();
	int col=shader->GetUniform("color");

	glUniform3fv(col,1,bone->color);

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER,6*sizeof(float),line,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(pos);glCheckError();

	glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();

	glDrawArrays(GL_LINES,0,2);glCheckError();
	//glDrawElements(GL_TRIANGLES,3,GL_UNSIGNED_INT,)

	glDisableVertexAttribArray(pos);

	glDisable(GL_DEPTH_TEST);
}

float signof(float num){return (num>0 ? 1.0f : (num<0 ? -1.0f : 0.0f));}




void OpenGLRenderer::Render(GuiViewport* viewport,bool force)
{
	

	vec4 canvas=viewport->rect;
	vec2 mouse(tabContainer->mousex,tabContainer->mousey);

	ID2D1Bitmap*& rBitmap=(ID2D1Bitmap*&)viewport->renderBitmap;
	unsigned char*& rBuffer=(unsigned char*&)viewport->renderBuffer;

	if(!rBitmap || rBitmap->GetSize().width!=canvas.z || rBitmap->GetSize().height!=canvas.w || !viewport->renderBuffer || !viewport->renderBitmap)
	{
		SAFERELEASE(rBitmap);
		SAFEDELETEARRAY(viewport->renderBuffer);

		D2D1_BITMAP_PROPERTIES bp=D2D1::BitmapProperties();
		bp.pixelFormat=tabContainer->renderTarget->GetPixelFormat();

		tabContainer->renderTarget->CreateBitmap(D2D1::SizeU((unsigned int)canvas.z,(unsigned int)canvas.w),bp,&rBitmap);

		rBuffer=new unsigned char[(int)canvas.z*canvas.w*4];
	}

	if(viewport->rootEntity)
	{
		viewport->rootEntity->world=viewport->model;
		viewport->rootEntity->update();
	}

	tabContainer->renderer->ChangeContext();

	glViewport((int)0,(int)0,(int)canvas.z,(int)canvas.w);glCheckError();
	glScissor((int)0,(int)0,(int)canvas.z,(int)canvas.w);glCheckError();

	glEnable(GL_DEPTH_TEST);

	{
		glClearColor(0.43f,0.43f,0.43f,0.0f);glCheckError();
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glCheckError();

		MatrixStack::Push(MatrixStack::PROJECTION,viewport->projection);
		MatrixStack::Push(MatrixStack::VIEW,viewport->view);
		MatrixStack::Push(MatrixStack::MODEL,viewport->model);

		tabContainer->renderer->draw(vec3(0,0,0),vec3(1000,0,0),vec3(1,0,0));
		tabContainer->renderer->draw(vec3(0,0,0),vec3(0,1000,0),vec3(0,1,0));
		tabContainer->renderer->draw(vec3(0,0,0),vec3(0,0,1000),vec3(0,0,1));

		if(viewport->rootEntity)
			viewport->rootEntity->draw(tabContainer->renderer);

		MatrixStack::Pop(MatrixStack::MODEL);
		MatrixStack::Pop(MatrixStack::VIEW);
		MatrixStack::Pop(MatrixStack::PROJECTION);

		glReadBuffer(GL_BACK);glCheckError();
		glReadPixels((int)0,(int)0,(int)canvas.z,(int)canvas.w,GL_BGRA,GL_UNSIGNED_BYTE,rBuffer);glCheckError();//@mic should implement pbo for performance

		rBitmap->CopyFromMemory(&D2D1::RectU(0,0,(int)canvas.z,(int)canvas.w),rBuffer,(int)(canvas.z*4));

		tabContainer->renderTarget->DrawBitmap(rBitmap,D2D1::RectF(canvas.x,canvas.y,canvas.x+canvas.z,canvas.y+canvas.w));
	}

	if(viewport->needsPicking && mouse.y-canvas.y>=0)
	{
		glDisable(GL_DITHER);

		tabContainer->renderer->picking=true;

		glClearColor(0.0f,0.0f,0.0f,0.0f);glCheckError();
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glCheckError();

		MatrixStack::Push(MatrixStack::PROJECTION,viewport->projection);
		MatrixStack::Push(MatrixStack::VIEW,viewport->view);
		MatrixStack::Push(MatrixStack::MODEL,viewport->model);

		if(viewport->rootEntity)
			viewport->rootEntity->draw(tabContainer->renderer);

		MatrixStack::Pop(MatrixStack::MODEL);
		MatrixStack::Pop(MatrixStack::VIEW);
		MatrixStack::Pop(MatrixStack::PROJECTION);

		glReadBuffer(GL_BACK);glCheckError();

		viewport->pickedPixel;
		glReadPixels((int)mouse.x,(int)mouse.y-canvas.y,(int)1,(int)1,GL_RGBA,GL_UNSIGNED_BYTE,&viewport->pickedPixel);glCheckError();//@mic should implement pbo for performance

		unsigned int address=0;

		unsigned char* ptrPixel=(unsigned char*)&viewport->pickedPixel;
		unsigned char* ptrAddress=(unsigned char*)&address;

		{
			viewport->pickedEntity=viewport->pickedPixel ? 

				ptrAddress[0]=ptrPixel[3],
				ptrAddress[1]=ptrPixel[2],
				ptrAddress[2]=ptrPixel[1],
				ptrAddress[3]=ptrPixel[0],

				dynamic_cast<Entity*>((Entity*)address)

				: 0 ;
		}

		tabContainer->renderer->picking=false;

		glEnable(GL_DITHER);

		viewport->needsPicking=false;
	}
}


void OpenGLRenderer::Render()
{
	for(std::list<GuiViewport*>::iterator vport=this->viewports.begin();vport!=this->viewports.end();vport++)
		(*vport)->OnPaint(this->tabContainer);
	
}


/*float ratio=this->width/this->height;
	float calcRatio=ratio*tan(45*PI/180.0f);

	draw(vec3(0,1,-1),15);
	draw(vec3(calcRatio*this->RendererViewportInterface_farPlane,0,-this->RendererViewportInterface_farPlane+1),15);*/



/*
class Ray 
{ 
public: 
	Ray(const vec3 &orig, const vec3 &dir) : orig(orig), dir(dir) 
	{ 
		invdir = 1 / dir; 
		sign[0] = (invdir.x < 0); 
		sign[1] = (invdir.y < 0); 
		sign[2] = (invdir.z < 0); 
	} 
	vec3 orig, dir;       // ray orig and dir 
	vec3 invdir; 
	int sign[3]; 
}; 

bool intersect(AABB &bounds,const Ray &r) const 
{ 
	float tmin, tmax, tymin, tymax, tzmin, tzmax; 

	tmin = (bounds[r.sign[0]].x - r.orig.x) * r.invdir.x; 
	tmax = (bounds[1-r.sign[0]].x - r.orig.x) * r.invdir.x; 
	tymin = (bounds[r.sign[1]].y - r.orig.y) * r.invdir.y; 
	tymax = (bounds[1-r.sign[1]].y - r.orig.y) * r.invdir.y; 

	if ((tmin > tymax) || (tymin > tmax)) 
		return false; 
	if (tymin > tmin) 
		tmin = tymin; 
	if (tymax < tmax) 
		tmax = tymax; 

	tzmin = (bounds[r.sign[2]].z - r.orig.z) * r.invdir.z; 
	tzmax = (bounds[1-r.sign[2]].z - r.orig.z) * r.invdir.z; 

	if ((tmin > tzmax) || (tzmin > tmax)) 
		return false; 
	if (tzmin > tmin) 
		tmin = tzmin; 
	if (tzmax < tmax) 
		tmax = tzmax; 

	return true; 
}
*/




/*
void OpenGLRenderer::OnGuiLMouseDown()
{
	/ *float ratio=this->width/this->height;
	float calcRatioX=ratio*tan(45*PI/180.0f);
	float calcRatioY=-1;

	/ *vec3 ndc(2.0f * tabContainer->mousex / this->width -1.0f,-2.0f * (tabContainer->mousey-TabContainer::CONTAINER_HEIGHT) / this->height + 1.0f,0);
	mat4 viewInv=MatrixStack::view.inverse();
	rayStart=viewInv.transform(ndc.x*calcRatioX,ndc.y*calcRatioY,-1);
	rayEnd=viewInv.transform(ndc.x*calcRatioX*this->RendererViewportInterface_farPlane,ndc.y*calcRatioY*this->RendererViewportInterface_farPlane,-this->RendererViewportInterface_farPlane+1);* /
* /

}*/
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

#pragma message(LOCATION " remember to use WindowData::CopyProcedureData to properly synchronize messages by the wndproc and the window herself")

LRESULT CALLBACK TabContainer::TabContainerWindowClassProcedure(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	TabContainer* tc=(TabContainer*)GetWindowLongPtr(hwnd,GWLP_USERDATA);

	LPARAM result=0;

	//printf("skipping tab %x procedure message %s\n",tc,findMsg(msg));

	if(tc && tc->skip)
		return 1;//@mic skip tabcontainer procedure

	switch(msg)
	{
		case WM_CREATE:
		{
			result=DefWindowProc(hwnd,msg,wparam,lparam);

			LPCREATESTRUCT lpcs=(LPCREATESTRUCT)lparam;

			if(lpcs)
			{
				TabContainer* tabContainer=(TabContainer*)lpcs->lpCreateParams;
				SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)tabContainer);
				tabContainer->hwnd=hwnd;
			}
		}
		break;
		case WM_ERASEBKGND:
			/*if(tc->splitterContainer->floatingTabRef)
				tc->OnPaint();*/
			return 1;
		case WM_SIZE:
			tc->CopyProcedureData(hwnd,msg,wparam,lparam);
			tc->OnGuiSize();
		break;
		case WM_DESTROY:
			printf("window %p of TabContainer %p destroyed\n",hwnd,tc);
		break;
		case WM_WINDOWPOSCHANGED:
			tc->CopyProcedureData(hwnd,msg,wparam,lparam);
			tc->OnWindowPosChanging();
		break;
		case WM_LBUTTONDOWN:
			tc->CopyProcedureData(hwnd,msg,wparam,lparam);
			tc->OnGuiLMouseDown();
		break;
		case WM_MOUSEWHEEL:
			{
				tc->CopyProcedureData(hwnd,msg,wparam,lparam);
				float wheelValue=GET_WHEEL_DELTA_WPARAM(wparam)>0 ? 1.0f : (GET_WHEEL_DELTA_WPARAM(wparam)<0 ? -1.0f : 0);
				tc->OnGuiMouseWheel(&wheelValue);
			}
		break;
		case WM_MOUSEMOVE:
			tc->CopyProcedureData(hwnd,msg,wparam,lparam);
			tc->OnGuiMouseMove();
		break;
		case WM_NCHITTEST:
			if(tc->splitterContainer->floatingTabRef)
				return HTTRANSPARENT;
			else 
				result=DefWindowProc(hwnd,msg,wparam,lparam);
			break;
		case WM_LBUTTONUP:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			tc->CopyProcedureData(hwnd,msg,wparam,lparam);
			tc->OnGuiLMouseUp();
		break;
		case WM_RBUTTONUP:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			tc->CopyProcedureData(hwnd,msg,wparam,lparam);
			tc->OnRMouseUp();
		break;
		case WM_PAINT:
		{
			tc->CopyProcedureData(hwnd,msg,wparam,lparam);
			PAINTSTRUCT ps;
			BeginPaint(hwnd,&ps);
			//tc->OnGuiPaint();
			EndPaint(hwnd,&ps);
			tc->SetDraw(0,true);
			result=0;
		}
		break;
		default:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			
	}

	if(tc)
	{
		tc->msg=0;
		tc->wparam;
		tc->lparam=0;
	}

	return result;
}


unsigned char* TabContainer::rawUpArrow=0;
unsigned char* TabContainer::rawRightArrow=0;
unsigned char* TabContainer::rawDownArrow=0;
unsigned char* TabContainer::rawFolder=0;
unsigned char* TabContainer::rawFile=0;

TabContainer::TabContainer(float x,float y,float w,float h,HWND parent):
	tabs(this),
	parentContainer(0),
	renderTarget(0),
	brush(0),
	selected(0),
	mouseDown(false),
	isRender(false),
	splitterContainer(0),
	recreateTarget(true),
	resizeTarget(true),
	reloadScript(0),
	iconUp(0),
	iconRight(0),
	iconDown(0),
	iconFolder(0),
	iconFile(0),
	nPainted(0),
	buttonLeftMouseDown(false),
	buttonControlDown(false),
	drawRect(0),
	drawFrame(true),
	drawTask(0),
	lastFrameTime(0),
	skip(false)
{
	width=w;
	height=h;

	hwnd=CreateWindow(WC_TABCONTAINERWINDOWCLASS,WC_TABCONTAINERWINDOWCLASS,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,(int)x,(int)y,(int)w,(int)h,parent,0,0,this);

	if(!hwnd)
		__debugbreak();

	this->RecreateTarget();

	printf("creating TabContainer %p\n",this);

	HWND parentWindow=GetParent(hwnd);

	this->parentContainer=(ContainerWindow*)GetWindowLongPtr(parent,GWLP_USERDATA);
	this->parentContainer->tabContainers.push_back(this);

	splitterContainer=(SplitterContainer*)(ContainerWindow*)GetWindowLongPtr(GetParent(hwnd),GWLP_USERDATA);

	if(!splitterContainer)
		__debugbreak();

	OpenGLRenderer* _oglRenderer=new OpenGLRenderer(this);

	if(!_oglRenderer)
		__debugbreak();

	this->renderer=_oglRenderer;

	Task* openGLContextTask=this->thread.NewTask(std::function<void()>(std::bind(&OpenGLRenderer::Create,_oglRenderer,hwnd)));//_oglRenderer->Create(this->hwnd);
	while(openGLContextTask->func);

	this->drawTask=this->thread.NewTask(std::function<void()>(std::bind(&TabContainer::Draw,this)),false);
}

TabContainer::~TabContainer()
{
	if(this->renderer)
		delete this->renderer;

	printf("deleting TabContainer %p\n",this);

	if(!this->FindAndGrowSibling())
		this->UnlinkSibling();

	this->parentContainer->tabContainers.erase(std::find(parentContainer->tabContainers.begin(),parentContainer->tabContainers.end(),this));

	SAFERELEASE(renderTarget);
	SAFERELEASE(brush);
	SAFERELEASE(iconRight);
	SAFERELEASE(iconDown);
	SAFERELEASE(iconFolder);
	SAFERELEASE(iconFile);

	DestroyWindow(this->hwnd);
}

ID2D1Brush* TabContainer::SetColor(unsigned int color)
{
	brush->SetColor(D2D1::ColorF(color));
	return brush;
}

GuiRect* TabContainer::GetSelected()
{
	return selected<tabs.childs.size() ? tabs.childs[selected] : 0;
}
/*
#ifdef _M_X64 //_WIN64 bug in vs2010 la flag seems off but is false
char* cl_arguments="/MDd /I\"c:\\sdk\\Autodesk\\FBX\\FBX SDK\\2014.1\\include\" /Ic:\\sdk\\openGL /I\"C:\\Sdk\\Windows\\v7.1\\Include\"  /I\"C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\VC\\include\" \"C:\\USERS\\MICHELE\\DOCUMENTS\\VISUAL STUDIO 2010\\PROJECTS\\ENGINE\\ENGINE\\EC.CPP\" /link /DEBUG /NOENTRY /LIBPATH:\"C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\VC\\lib\\amd64\" /LIBPATH:\"C:\\Sdk\\Windows\\v7.1\\Lib\\x64\" /LIBPATH:\"C:\\sdk\\Autodesk\\FBX\\FBX SDK\\2014.1\\lib\\vs2010\\x64\\debug\" opengl32.lib d3d11.lib comctl32.lib Shlwapi.lib d2d1.lib dwrite.lib windowscodecs.lib dxgi.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib anim.obj bone.obj buildpath.obj Direct2DGui.obj EC.obj entity.obj inputmanager.obj interfaces.obj jpgdecoder.obj light.obj main.obj material.obj matrixstack.obj mesh.obj picopng.obj primitives.obj shadermanager.obj skin.obj texture.obj tga_reader.obj win32containers.obj win32interfaces.obj win32msg.obj win32openglrenderer.obj win32openglshader.obj win32procedures.obj win32splitter.obj /DLL ec.obj /OUT:ec.dll";
#else
char* cl_arguments="/MDd /I\"c:\\sdk\\Autodesk\\FBX\\FBX SDK\\2014.1\\include\" /Ic:\\sdk\\openGL /I\"C:\\Sdk\\Windows\\v7.1\\Include\"  /I\"C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\VC\\include\" \"C:\\USERS\\MICHELE\\DOCUMENTS\\VISUAL STUDIO 2010\\PROJECTS\\ENGINE\\ENGINE\\EC.CPP\" /link /DEBUG /NOENTRY /LIBPATH:\"C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\VC\\lib\" /LIBPATH:\"C:\\Sdk\\Windows\\v7.1\\Lib\" /LIBPATH:\"C:\\sdk\\Autodesk\\FBX\\FBX SDK\\2014.1\\lib\\vs2010\\x86\\debug\" opengl32.lib d3d11.lib comctl32.lib Shlwapi.lib d2d1.lib dwrite.lib windowscodecs.lib dxgi.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib anim.obj bone.obj buildpath.obj Direct2DGui.obj EC.obj entity.obj inputmanager.obj interfaces.obj jpgdecoder.obj light.obj main.obj material.obj matrixstack.obj mesh.obj picopng.obj primitives.obj shadermanager.obj skin.obj texture.obj tga_reader.obj win32containers.obj win32interfaces.obj win32msg.obj win32openglrenderer.obj win32openglshader.obj win32procedures.obj win32splitter.obj /DLL ec.obj /OUT:ec.dll";
#endif*/


void TabContainer::Draw()
{
	if(this->drawRect && this->drawFrame)
		__debugbreak();

	if(this->reloadScript)
	{
		typedef  EntityScript* (*Create)();

		SetCurrentDirectory("C:\\Users\\Michele\\Documents\\Visual Studio 2010\\Projects\\Engine");

		if(this->reloadScript->script)
		{
			this->reloadScript->script->deinit();

			if(!FreeLibrary((HMODULE)this->reloadScript->module))
				__debugbreak();

			delete this->reloadScript->script;
			this->reloadScript->script=0;
		}

		{
			SHELLEXECUTEINFO sei={sizeof(SHELLEXECUTEINFO),SEE_MASK_NOCLOSEPROCESS,0,0,"cl.bat","/MDd /Zi /EHsc ec.cpp /link /MANIFEST:NO /DLL /NOENTRY engine.lib /OUT:ec.dll",0,/*show*/1,0,0,0,0,0,0};

			if(!ShellExecuteEx(&sei))
				__debugbreak();

			if(!sei.hProcess)
				__debugbreak();

			WaitForSingleObject(sei.hProcess,INFINITE);
		}

		this->reloadScript->module=(HMODULE)LoadLibrary("EC.dll");

		if(!this->reloadScript->module)
			__debugbreak();

		Create createScriptFunc=(Create)GetProcAddress((HMODULE)this->reloadScript->module,"Create");
		
		if(createScriptFunc)
		{
			this->reloadScript->script=createScriptFunc();
			this->reloadScript->script->entity=this->reloadScript;

			this->reloadScript->script->init();
		}

		this->reloadScript=0;
	}
	else if(this->drawRect)
	{
		this->drawRect->OnPaint(this);
		this->drawRect=0;
	}
	else if(this->drawFrame)
	{
		this->OnGuiPaint();
		this->drawFrame=0;
	}
	else
	{
		if(Timer::instance->GetTime()-this->lastFrameTime>(1000.0f/Timer::instance->renderFps))
		{
			this->lastFrameTime=Timer::instance->GetTime();
			this->renderer->Render();
		}
	}
}

void TabContainer::SetDraw(GuiRect* iRect,bool iFrame)
{
	if(this->drawRect && this->drawFrame)
		__debugbreak();

	this->drawRect=iRect;
	this->drawFrame=iFrame;
}


bool TabContainer::BeginDraw()
{
	if(!this->isRender)
	{
		if(this->recreateTarget)
		{
			this->OnGuiRecreateTarget();
		}
		else if(this->resizeTarget)
		{
			HRESULT result=renderTarget->Resize(D2D1::SizeU((int)this->width,(int)this->height));

			if(S_OK!=result)
				__debugbreak();
		}
		
		this->renderTarget->BeginDraw();

		if(this->recreateTarget==0 || this->resizeTarget==0)
			this->DrawFrame();

		this->recreateTarget=0;
		this->resizeTarget=0;

		this->isRender=true;

		return true;
	}
	else
		__debugbreak();
	
	return false;
	
}
void TabContainer::EndDraw()
{
	if(this->isRender)
	{
		renderTarget->DrawRectangle(D2D1::RectF(0.5f,0.5f,this->width-0.5f,this->height-0.5f),this->SetColor(D2D1::ColorF::Red));

		HRESULT endDrawError =renderTarget->EndDraw();

		this->recreateTarget=endDrawError==D2DERR_RECREATE_TARGET;

		if(endDrawError!=0)
		{
			printf("D2D1HwndRenderTarget::EndDraw error: %x\n",endDrawError);

			HRESULT flushError=renderTarget->Flush();

			if(flushError!=0)
				printf("D2D1HwndRenderTarget::Flush error: %x\n",flushError);
		}

		this->isRender=false;
	}
	else
		__debugbreak();
}

void TabContainer::BroadcastToSelected(void (GuiRect::*func)(TabContainer*,void*),void* data)
{
	GuiRect* selectedTab=this->GetSelected();
	
	if(selectedTab)
		(selectedTab->*func)(this,data);
}

void TabContainer::BroadcastToAll(void (GuiRect::*func)(TabContainer*,void*),void* data)
{
	(this->tabs.*func)(this,data);
}


template<class C> void TabContainer::BroadcastToSelected(void (GuiRect::*func)(TabContainer*,void*),void* data)
{
	GuiRect* selectedTab=this->GetSelected();

	if(selectedTab)
		selectedTab->BroadcastTo<C>(func,data);
}

template<class C> void TabContainer::BroadcastToAll(void (GuiRect::*func)(TabContainer*,void*),void* data)
{
	this->tabs.BroadcastTo<C>(func,data);
}


void TabContainer::DrawText(unsigned int iColor,const char* iText,float x,float y, float w,float h,float iAlignX,float iAlignY)
{
	Direct2DGuiBase::DrawText(this->renderTarget,this->SetColor(iColor),iText,x,y,w,h,iAlignX,iAlignY);
}

void TabContainer::OnGuiSize(void* data)
{
	WindowData::OnSize();

	this->tabs.rect.make(0.0f,(float)TabContainer::CONTAINER_HEIGHT,width,height-TabContainer::CONTAINER_HEIGHT);
	
	this->BroadcastToSelected(&GuiRect::OnSize,data);
	
	this->resizeTarget=true;
	//this->OnGuiPaint();
	
}


void TabContainer::OnGuiRecreateTarget(void* data)
{
	this->RecreateTarget();
	/*Task* tabContainerRecreateTarget=ThreadPool::instance->NewTask(std::function<void()>(std::bind(&TabContainer::RecreateTarget,this)));
	tabContainerRecreateTarget->Wait();*/

	
	this->BroadcastToSelected(&GuiRect::OnRecreateTarget,data);

	this->recreateTarget=false;

	//printf("tab %p target recreated\n",this);
}

void TabContainer::OnWindowPosChanging(void* data)
{
	WindowData::OnWindowPosChanging();

	this->tabs.rect.make(0,TabContainer::CONTAINER_HEIGHT,width,height-TabContainer::CONTAINER_HEIGHT);

	this->BroadcastToSelected(&GuiRect::OnSize,data);
	this->resizeTarget=true;
}

void TabContainer::OnGuiMouseMove(void* data)
{
	SetFocus(this->hwnd);

	this->buttonLeftMouseDown=(this->wparam & MK_LBUTTON)!=0;
	this->buttonControlDown=(this->wparam & MK_CONTROL)!=0;

	float tmx=(float)LOWORD(lparam);
	float tmy=(float)HIWORD(lparam);

	splitterContainer->currentTabContainer=this;

	if(mouseDown && tmx!=mousex && tmy!=mousey)
		splitterContainer->CreateFloatingTab(this);

	mousex=tmx;
	mousey=tmy;

	//if(mousey>TabContainer::CONTAINER_HEIGHT)
		this->BroadcastToSelected(&GuiRect::OnMouseMove,vec2(this->mousex,this->mousey));
}

void TabContainer::OnGuiLMouseUp(void* data)
{
	mouseDown=false;

	this->BroadcastToSelected(&GuiRect::OnLMouseUp,vec2(this->mousex,this->mousey));
}

void TabContainer::OnGuiMouseWheel(void* data)
{
	float factor=GET_WHEEL_DELTA_WPARAM(this->wparam)>0 ? 1.0f : (GET_WHEEL_DELTA_WPARAM(this->wparam)<0 ? -1.0f : 0);

	if(this->mousey>TabContainer::CONTAINER_HEIGHT)
		this->BroadcastToSelected(&GuiRect::OnMouseWheel,&factor);
}								  
		
void TabContainer::OnGuiLMouseDown(void* data)
{
	//this->OnGuiMouseMove();

	float &x=this->mousex;
	float &y=this->mousey;


	if(y<=CONTAINER_HEIGHT)
	{
		int prevSel=selected;

		for(int i=0;i<(int)tabs.childs.size();i++)
		{
			if(x>(i*TAB_WIDTH) && x< (i*TAB_WIDTH+TAB_WIDTH) && y > (CONTAINER_HEIGHT-TAB_HEIGHT) &&  y<CONTAINER_HEIGHT)
			{
				this->BroadcastToSelected(&GuiRect::OnDeactivate);

				selected=i;

				mouseDown=true;

				this->BroadcastToSelected(&GuiRect::OnActivate);
				this->BroadcastToSelected(&GuiRect::OnSize);

				this->drawRect=0;
				this->SetDraw(0,true);

				break;
			}
		}
	}
	else
	{
		this->BroadcastToSelected(&GuiRect::OnLMouseDown,vec2(this->mousex,this->mousey));
	}
}

void TabContainer::OnGuiUpdate(void* data)
{
	this->BroadcastToSelected(&GuiRect::OnUpdate,data);
}

void TabContainer::OnRMouseUp(void* data)
{
	float &x=this->mousex;
	float &y=this->mousey;

	if(y<=TabContainer::CONTAINER_HEIGHT)
	{
		RECT rc;
		GetWindowRect(hwnd,&rc);

		int tabNumberHasChanged=this->tabs.childs.size();

		for(int i=0;i<(int)tabs.childs.size();i++)
		{
			if(x>(i*TAB_WIDTH) && x< (i*TAB_WIDTH+TAB_WIDTH) && y > (CONTAINER_HEIGHT-TAB_HEIGHT) &&  y<CONTAINER_HEIGHT)
			{
				int menuResult=TrackPopupMenu(SplitterContainer::popupMenuRoot,TPM_RETURNCMD |TPM_LEFTALIGN|TPM_TOPALIGN,rc.left+LOWORD(lparam),rc.top+HIWORD(lparam),0,GetParent(hwnd),0);

				switch(menuResult)
				{
				case TAB_MENU_COMMAND_OPENGLWINDOW:
					this->tabs.Viewport();
					this->selected=this->tabs.childs.size()-1;
					break;
				case TAB_MENU_COMMAND_SCENEENTITIES:
					this->tabs.SceneViewer();
					this->selected=this->tabs.childs.size()-1;
					break;
				case TAB_MENU_COMMAND_ENTITYPROPERTIES:
					this->tabs.EntityViewer();
					this->selected=this->tabs.childs.size()-1;
					break;
				case TAB_MENU_COMMAND_PROJECTFOLDER:
					this->tabs.ProjectViewer();
					this->selected=this->tabs.childs.size()-1;
					break;
				case TAB_MENU_COMMAND_REMOVE:
					if((int)GetPool<TabContainer>().size()>1)
					{
						printf("total TabContainer before destroying: %d\n",(int)GetPool<TabContainer>().size());
						this->~TabContainer();
						printf("total TabContainer after destroying: %d\n",(int)GetPool<TabContainer>().size());
					}
					break;
				case TAB_MENU_COMMAND_LOGGER:


					break;
				}

				break;


			}
		}

		if(tabNumberHasChanged!=this->tabs.childs.size())
		{
			this->drawRect=0;
			this->SetDraw(0,true);
		}
	}
	else
	{
		this->BroadcastToSelected(&GuiRect::OnRMouseUp,vec2(this->mousex,this->mousey));
	}
	
}

void TabContainer::OnGuiRender(void* data)
{
	this->BroadcastToSelected(&GuiRect::OnRender,data);
}

void TabContainer::DrawFrame()
{
	renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

	renderTarget->FillRectangle(D2D1::RectF(0,0,(float)width,(float)CONTAINER_HEIGHT),this->SetColor(COLOR_TAB_BACKGROUND));
	renderTarget->FillRectangle(D2D1::RectF((float)(selected*TAB_WIDTH),(float)(CONTAINER_HEIGHT-TAB_HEIGHT),(float)(selected*TAB_WIDTH+TAB_WIDTH),(float)((CONTAINER_HEIGHT-TAB_HEIGHT)+TAB_HEIGHT)),this->SetColor(COLOR_TAB_SELECTED));

	for(int i=0;i<(int)tabs.childs.size();i++)
		Direct2DGuiBase::DrawText(renderTarget,this->SetColor(TabContainer::COLOR_TEXT),tabs.childs[i]->name,(float)i*TAB_WIDTH,(float)CONTAINER_HEIGHT-TAB_HEIGHT,(float)i*TAB_WIDTH + (float)TAB_WIDTH,(float)(CONTAINER_HEIGHT-TAB_HEIGHT) + (float)TAB_HEIGHT,0.5f,0.5f);
}

void TabContainer::OnGuiPaint(void* data)
{
	bool isDrawing=this->BeginDraw();

	renderTarget->Clear(D2D1::ColorF(TabContainer::COLOR_GUI_BACKGROUND));

	this->DrawFrame();

	this->BroadcastToSelected(&GuiRect::OnPaint,data);

	this->EndDraw();

	nPainted++;
}

void TabContainer::OnResizeContainer(void* data)
{
	RECT rc;

	GetClientRect(this->hwnd,&rc);
	MapWindowRect(this->hwnd,this->parentContainer->hwnd,&rc);

	int size=0;
	int diff=0;
	int side=0;

	switch(this->parentContainer->resizeEnumType)
	{
		case 0:
			side=(rc.right-rc.left);
			diff=this->parentContainer->resizeDiffWidth;
			size=side+diff;
			SetWindowPos(this->hwnd,0,0,rc.top,size,rc.bottom-rc.top,/*SWP_NOREDRAW|*/SWP_NOZORDER|SWP_NOOWNERZORDER);
			this->parentContainer->resizeCheckWidth+=size;
			break;
		case 1:
			side=rc.bottom;
			diff=this->parentContainer->resizeDiffHeight;
			size=side+diff;
			SetWindowPos(this->hwnd,0,rc.left,rc.top,rc.right-rc.left,size,/*SWP_NOREDRAW|*/SWP_NOZORDER|SWP_NOOWNERZORDER);
			this->parentContainer->resizeCheckHeight+=size;
			break;
		case 2:
			side=(rc.right-rc.left);
			diff=this->parentContainer->resizeDiffWidth;
			size=side+diff;
			SetWindowPos(this->hwnd,0,rc.left,rc.top,size,rc.bottom-rc.top,/*SWP_NOREDRAW|*/SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOMOVE);
			this->parentContainer->resizeCheckWidth+=size;
			break;
		case 3:
			side=rc.bottom;
			diff=this->parentContainer->resizeDiffHeight;
			size=side+diff;
			SetWindowPos(this->hwnd,0,rc.left,rc.top,rc.right-rc.left,size,/*SWP_NOREDRAW|*/SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOMOVE);
			this->parentContainer->resizeCheckHeight+=size;
		break;
		default:
			__debugbreak();
	}
}

void TabContainer::OnEntitiesChange(void* data)
{
	this->BroadcastToSelected(&GuiRect::OnEntitiesChange,data);
}
void TabContainer::OnGuiActivate(void* data)
{
	this->BroadcastToSelected(&GuiRect::OnActivate,data);

	
}
void TabContainer::OnGuiDeactivate(void* data)
{
	this->BroadcastToSelected(&GuiRect::OnDeactivate,data);
}
void TabContainer::OnGuiEntitySelected(void* data)
{
	this->BroadcastToSelected(&GuiRect::OnEntitySelected,data);
}


void TabContainer::RecreateTarget()
{
	HRESULT hr=S_OK;

	float w,h;

	if(!rawUpArrow)
		Direct2DGuiBase::CreateRawBitmap(L"uparrow.png",rawUpArrow,w,h);

	if(!rawRightArrow)
		Direct2DGuiBase::CreateRawBitmap(L"rightarrow.png",rawRightArrow,w,h);

	if(!rawDownArrow)
		Direct2DGuiBase::CreateRawBitmap(L"downarrow.png",rawDownArrow,w,h);

	if(!rawFolder)
		Direct2DGuiBase::CreateRawBitmap(L"folder.png",rawFolder,w,h);

	if(!rawFile)
		Direct2DGuiBase::CreateRawBitmap(L"file.png",rawFile,w,h);

	SAFERELEASE(renderTarget);
	SAFERELEASE(brush);

	SAFERELEASE(iconUp);
	SAFERELEASE(iconRight);
	SAFERELEASE(iconDown);
	SAFERELEASE(iconFolder);
	SAFERELEASE(iconFile);

	renderTarget=Direct2DGuiBase::InitHWNDRenderer(hwnd);

	if(!renderTarget)
		__debugbreak();

	renderTarget->CreateSolidColorBrush(D2D1::ColorF(COLOR_TAB_BACKGROUND),&brush);

	if(!brush)
		__debugbreak();

	D2D1_BITMAP_PROPERTIES bp=D2D1::BitmapProperties();
	bp.pixelFormat=renderTarget->GetPixelFormat();
	bp.pixelFormat.alphaMode=D2D1_ALPHA_MODE_PREMULTIPLIED;

	hr=renderTarget->CreateBitmap(D2D1::SizeU(TabContainer::CONTAINER_ICON_WH,TabContainer::CONTAINER_ICON_WH),TabContainer::rawUpArrow,TabContainer::CONTAINER_ICON_STRIDE,bp,&iconUp);

	if(!iconUp || hr!=S_OK)
		__debugbreak();

	hr=renderTarget->CreateBitmap(D2D1::SizeU(TabContainer::CONTAINER_ICON_WH,TabContainer::CONTAINER_ICON_WH),TabContainer::rawRightArrow,TabContainer::CONTAINER_ICON_STRIDE,bp,&iconRight);

	if(!iconRight || hr!=S_OK)
		__debugbreak();

	hr=renderTarget->CreateBitmap(D2D1::SizeU(TabContainer::CONTAINER_ICON_WH,TabContainer::CONTAINER_ICON_WH),TabContainer::rawDownArrow,TabContainer::CONTAINER_ICON_STRIDE,bp,&iconDown);

	if(!iconDown || hr!=S_OK)
		__debugbreak();

	hr=renderTarget->CreateBitmap(D2D1::SizeU(TabContainer::CONTAINER_ICON_WH,TabContainer::CONTAINER_ICON_WH),TabContainer::rawFolder,TabContainer::CONTAINER_ICON_STRIDE,bp,&iconFolder);

	if(!iconFolder || hr!=S_OK)
		__debugbreak();

	hr=renderTarget->CreateBitmap(D2D1::SizeU(TabContainer::CONTAINER_ICON_WH,TabContainer::CONTAINER_ICON_WH),TabContainer::rawFile,TabContainer::CONTAINER_ICON_STRIDE,bp,&iconFile);

	if(!iconFile || hr!=S_OK)
		__debugbreak();

}
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

#pragma message (LOCATION " beware to new POD initialization (parhentesized or not)")

extern WNDPROC SystemOriginalTabControlProcedure;
extern WNDPROC SystemOriginalSysTreeView32ControlProcedure;

SplitterContainer::SplitterContainer():
splitterSize(4)
{
	floatingTabRef=0;
	floatingTab=0;
	floatingTabTarget=0;
	floatingTabRefTabIdx=-1;
	floatingTabRefTabCount=-1;
	floatingTabTargetAnchorPos=-1;
	floatingTabTargetAnchorTabIndex=-1;

	splitterCursor=IDC_ARROW;
}
SplitterContainer::~SplitterContainer()
{
}

HMENU SplitterContainer::popupMenuRoot=CreatePopupMenu();
HMENU SplitterContainer::popupMenuCreate=CreatePopupMenu();

void SplitterContainer::OnLButtonDown(HWND hwnd,LPARAM lparam)
{
	if(!GetCapture() && !floatingTabRef && splitterCursor!=IDC_ARROW)
	{
		splitterPreviousPos=MAKEPOINTS(lparam);

		SetCursor(LoadCursor(0,splitterCursor));

		int edge1=(splitterCursor==IDC_SIZEWE ? 2 : 3);//right - bottom
		int edge2=(splitterCursor==IDC_SIZEWE ? 0 : 1);//left - top

		RECT rc1,rc2;
		GetClientRect(hittedWindow1,&rc1);
		GetClientRect(hittedWindow2,&rc2);
		MapWindowRect(hittedWindow1,hwnd,&rc1);
		MapWindowRect(hittedWindow2,hwnd,&rc2);

		{
			resizingWindows1=findWindoswAtPos(hwnd,rc1,edge1);
			resizingWindows2=findWindoswAtPos(hwnd,rc2,edge2);
		}

		SetCapture(hwnd);
	}
}

void SplitterContainer::OnLButtonUp(HWND hwnd)
{
	if(floatingTabRef)
	{
		EnableAllChildsDescendants(hwnd,true);

		if(floatingTab)
		{
			if(floatingTabTarget)
			{
				if(floatingTabTargetAnchorPos>=0)
				{
					TabContainer* newTabContainer=0;

					if(1==(int)floatingTabRef->tabs.childs.size())
					{
						newTabContainer=floatingTabRef;

						if(!newTabContainer->FindAndGrowSibling())
							__debugbreak();

						RECT floatingTabTargetRc;
						GetClientRect(floatingTabTarget->hwnd,&floatingTabTargetRc);
						MapWindowRect(floatingTabTarget->hwnd,GetParent(floatingTabTarget->hwnd),&floatingTabTargetRc);

						int floatingTabTargetRcWidth=floatingTabTargetRc.right-floatingTabTargetRc.left;
						int floatingTabTargetRcHeight=floatingTabTargetRc.bottom-floatingTabTargetRc.top;


						switch(floatingTabTargetAnchorPos)
						{
						case 0:
							SetWindowPos(floatingTabTarget->hwnd,0,floatingTabTargetRc.left+floatingTabTargetRcWidth/2,floatingTabTargetRc.top,floatingTabTargetRc.right-(floatingTabTargetRc.left+floatingTabTargetRcWidth/2),floatingTabTargetRc.bottom-floatingTabTargetRc.top,SWP_SHOWWINDOW);
							SetWindowPos(floatingTabRef->hwnd,0,floatingTabTargetRc.left,floatingTabTargetRc.top,floatingTabTargetRc.right/2-floatingTabTargetRc.left-splitterSize,floatingTabTargetRc.bottom-floatingTabTargetRc.top,SWP_SHOWWINDOW);
						break;
						case 1:
							SetWindowPos(floatingTabTarget->hwnd,0,floatingTabTargetRc.left,floatingTabTargetRc.top+floatingTabTargetRcHeight/2,floatingTabTargetRc.right-floatingTabTargetRc.left,floatingTabTargetRc.bottom-(floatingTabTargetRc.top+floatingTabTargetRcHeight/2),SWP_SHOWWINDOW);
							SetWindowPos(floatingTabRef->hwnd,0,floatingTabTargetRc.left,floatingTabTargetRc.top,floatingTabTargetRc.right-floatingTabTargetRc.left,floatingTabTargetRc.bottom/2-floatingTabTargetRc.top-splitterSize,SWP_SHOWWINDOW);
						break;
						case 2:
							SetWindowPos(floatingTabTarget->hwnd,0,floatingTabTargetRc.left,floatingTabTargetRc.top,floatingTabTargetRc.right/2-floatingTabTargetRc.left-splitterSize,floatingTabTargetRc.bottom-floatingTabTargetRc.top,SWP_SHOWWINDOW);
							SetWindowPos(floatingTabRef->hwnd,0,floatingTabTargetRc.left+floatingTabTargetRcWidth/2,floatingTabTargetRc.top,floatingTabTargetRc.right-(floatingTabTargetRc.left+(floatingTabTargetRcWidth/2)),floatingTabTargetRc.bottom-floatingTabTargetRc.top,SWP_SHOWWINDOW);
						break;
						case 3:
							SetWindowPos(floatingTabTarget->hwnd,0,floatingTabTargetRc.left,floatingTabTargetRc.top,floatingTabTargetRc.right-floatingTabTargetRc.left,floatingTabTargetRc.bottom/2-floatingTabTargetRc.top-splitterSize,SWP_SHOWWINDOW);
							SetWindowPos(floatingTabRef->hwnd,0,floatingTabTargetRc.left,floatingTabTargetRc.top+floatingTabTargetRcHeight/2,floatingTabTargetRc.right-floatingTabTargetRc.left,floatingTabTargetRc.bottom-(floatingTabTargetRc.top+(floatingTabTargetRcHeight/2)),SWP_SHOWWINDOW);
						break;
						default:
							__debugbreak();
						break;
						}
					}
					else
					{
						newTabContainer=new TabContainer((float)floatingTabRc.left,(float)floatingTabRc.top,(float)(floatingTabRc.right-floatingTabRc.left),(float)(floatingTabRc.bottom-floatingTabRc.top),hwnd);
						
						GuiRect* reparentTab=floatingTabRef->tabs.childs[floatingTabRefTabIdx];
						floatingTabRef->selected>0 ? floatingTabRef->selected-=1 : floatingTabRef->selected=0;
						floatingTabRef->OnGuiActivate();
						reparentTab->SetParent(&newTabContainer->tabs);
						
						newTabContainer->selected=newTabContainer->tabs.childs.size()-1;

						SetWindowPos(floatingTabTarget->hwnd,0,floatingTabTargetRc.left,floatingTabTargetRc.top,floatingTabTargetRc.right-floatingTabTargetRc.left,floatingTabTargetRc.bottom-floatingTabTargetRc.top,SWP_SHOWWINDOW);		

						
					}

					floatingTabTarget->LinkSibling(newTabContainer,floatingTabTargetAnchorPos);
				}
				else
				{

				}
			}
		}
		this->DestroyFloatingTab();
	}
	else if(GetCapture())
	{
		ReleaseCapture();
		SetCursor(LoadCursor(0,IDC_ARROW));
	}
}

void SplitterContainer::OnMouseMove(HWND hwnd,LPARAM lparam)
{
	POINTS p=MAKEPOINTS(lparam);

	if(floatingTabRef)
	{
		RECT targetRc;
		RECT tmpRc;

		GetWindowRect(floatingTabRef->hwnd,&tmpRc);

		int scalesize=3;

		int floatingTabScaledWidthX=(tmpRc.right-tmpRc.left)/scalesize;
		int floatingTabScaledWidthY=(tmpRc.bottom-tmpRc.top)/scalesize;

		POINT cp={p.x,p.y};

		HWND target=ChildWindowFromPointEx(hwnd,cp,CWP_SKIPDISABLED);

		if(target!=GetParent(*floatingTab) && target!=floatingTab->hwnd && /*floatingTabTarget!=floatingTabRef*/(floatingTabRefTabCount==1 ? target!=floatingTabRef->hwnd : true))
		{
			floatingTabTarget=(TabContainer*)GetWindowLongPtr(target,GWLP_USERDATA);

			int hh=TabContainer::CONTAINER_HEIGHT;

			GetClientRect(target,&targetRc);
			tmpRc=targetRc;
			MapWindowRect(target,hwnd,&tmpRc);

			int w=tmpRc.right-tmpRc.left;
			int h=tmpRc.bottom-tmpRc.top;

			int wd=w/3;
			int hd=h/3;

			floatingTabTargetRc=floatingTabRc=tmpRc;
			int anchor=false;

			/*if(cp.y>tmpRc.top && cp.y<tmpRc.top+hh)
			{
				floatingTabTargetAnchorPos=-1;
				anchor=2;
			}
			else */if(cp.x>tmpRc.left && cp.x<tmpRc.left+wd)
			{
				floatingTabTargetAnchorPos=0;
				floatingTabRc.right=tmpRc.left+wd;
				floatingTabTargetRc.left=floatingTabRc.right+splitterSize;
				anchor=1;
			}
			else if(cp.y>tmpRc.top+hh && cp.y<tmpRc.top+hd)
			{
				floatingTabTargetAnchorPos=1;
				floatingTabRc=tmpRc;
				floatingTabRc.bottom=tmpRc.top+hd;
				floatingTabTargetRc.top=floatingTabRc.bottom+splitterSize;
				anchor=1;
			}
			else if(cp.x>tmpRc.right-wd && cp.x<tmpRc.right)
			{
				floatingTabTargetAnchorPos=2;
				floatingTabRc=tmpRc;
				floatingTabRc.left=tmpRc.right-wd;
				floatingTabTargetRc.right=floatingTabRc.left-splitterSize;
				anchor=1;
			}
			else if(cp.y>tmpRc.bottom-hd && cp.y<tmpRc.bottom)
			{
				floatingTabTargetAnchorPos=3;
				floatingTabRc=tmpRc;
				floatingTabRc.top=tmpRc.bottom-hd;
				floatingTabTargetRc.bottom=floatingTabRc.top-splitterSize;
				anchor=1;
			}
			else
				floatingTabTargetAnchorPos=-1;

			

			if(anchor==2)
			{
				/*ShowWindow(floatingTab,false);
				if(floatingTabTargetAnchorTabIndex<0)
					floatingTabTargetAnchorTabIndex=CreateTabChildren(floatingTabTarget,0,floatingTabRefTabIdx,floatingTab);*/

			}
			else
			{
				/*if(floatingTabTargetAnchorTabIndex>=0)
				{
					SendMessage(floatingTabTarget,TCM_DELETEITEM,floatingTabTargetAnchorTabIndex,0);
					floatingTabTargetAnchorTabIndex=-1;
				}
				ShowWindow(floatingTab,true);*/

				if(anchor==1)
					SetWindowPos(floatingTab->hwnd,0,floatingTabRc.left,floatingTabRc.top,floatingTabRc.right-floatingTabRc.left,floatingTabRc.bottom-floatingTabRc.top,SWP_SHOWWINDOW);
				else
					SetWindowPos(floatingTab->hwnd,0,p.x-(floatingTabScaledWidthX)/2,p.y-(floatingTabScaledWidthY)/2,floatingTabScaledWidthX,floatingTabScaledWidthY,SWP_SHOWWINDOW);
			}
		}
		else
		{
			//UpdateWindow(floatingTabRef->hwnd);
			SetWindowPos(floatingTab->hwnd,0,p.x-(floatingTabScaledWidthX)/2,p.y-(floatingTabScaledWidthY)/2,floatingTabScaledWidthX,floatingTabScaledWidthY,SWP_SHOWWINDOW);
			floatingTabTarget=0;
			floatingTabTargetAnchorPos=-1;
		}
	}
	else
	{
		if(!GetCapture())
		{
			TabContainer::BroadcastToPool(&TabContainer::OnGuiMouseMove,(void*)0);

			int pixelDistance=6;

			POINT probePoints[4]={{p.x-pixelDistance,p.y},{p.x+pixelDistance,p.y},{p.x,p.y-pixelDistance},{p.x,p.y+pixelDistance}};

			HWND hittedWindows[4]={0,0,0,0};

			for(int i=0;i<4;i++)
			{
				HWND found=ChildWindowFromPoint(hwnd,probePoints[i]);
				if(found!=0 && hwnd!=found)
					hittedWindows[i]=found;
			}

			if(hittedWindows[0]!=hittedWindows[1] && hittedWindows[0] && hittedWindows[1])
			{
				SetCursor(LoadCursor(0,splitterCursor=IDC_SIZEWE));
				hittedWindow1=hittedWindows[0];
				hittedWindow2=hittedWindows[1];
			}
			else if(hittedWindows[2]!=hittedWindows[3] && hittedWindows[2] && hittedWindows[3])
			{
				SetCursor(LoadCursor(0,splitterCursor=IDC_SIZENS));
				hittedWindow1=hittedWindows[2];
				hittedWindow2=hittedWindows[3];
			}
			else
				SetCursor(LoadCursor(0,splitterCursor=IDC_ARROW));
		}
		else
		{
			POINTS mouseMovedDelta={splitterPreviousPos.x-p.x,splitterPreviousPos.y-p.y};

			if(mouseMovedDelta.x==0 && mouseMovedDelta.y==0)
				return;

			int numwindows=resizingWindows1.size()+resizingWindows2.size();

			HDWP hdwp=BeginDeferWindowPos(numwindows);

			RECT rc;

			DWORD flags1=SWP_NOMOVE|SWP_SHOWWINDOW;
			DWORD flags2=SWP_SHOWWINDOW;
			for(int i=0;i<(int)resizingWindows1.size();i++)
			{
				GetWindowRect(resizingWindows1[i],&rc);
				MapWindowRect(HWND_DESKTOP,hwnd,&rc);

				if(splitterCursor==IDC_SIZEWE)
					DeferWindowPos(hdwp,resizingWindows1[i],0,0,0,(rc.right-rc.left)-mouseMovedDelta.x,rc.bottom-rc.top,flags1);
				else 
					DeferWindowPos(hdwp,resizingWindows1[i],0,0,0,rc.right-rc.left,(rc.bottom-rc.top)-mouseMovedDelta.y,flags1);
			}

			for(int i=0;i<(int)resizingWindows2.size();i++)
			{
				GetWindowRect(resizingWindows2[i],&rc);
				MapWindowRect(HWND_DESKTOP,hwnd,&rc);

				if(splitterCursor==IDC_SIZEWE)
					DeferWindowPos(hdwp,resizingWindows2[i],0,rc.left-mouseMovedDelta.x,rc.top,(rc.right-rc.left)+mouseMovedDelta.x,rc.bottom-rc.top,flags2);
				else
					DeferWindowPos(hdwp,resizingWindows2[i],0,rc.left,rc.top-mouseMovedDelta.y,rc.right-rc.left,(rc.bottom-rc.top)+mouseMovedDelta.y,flags2);
			}

			EndDeferWindowPos(hdwp);
		}

	}

	splitterPreviousPos=p;
}

std::vector<HWND> SplitterContainer::findWindoswAtPos(HWND mainWindow,RECT &srcRect,int rectPosition)
{
	std::vector<HWND> foundWindows;

	int srcRectRef[4]={srcRect.left,srcRect.top,srcRect.right,srcRect.bottom};

	HWND child=0;

	while(child=FindWindowEx(mainWindow,child ? child : 0,0,0))
	{
		RECT rc;
		GetClientRect(child,&rc);
		MapWindowRect(child,mainWindow,&rc);


		int rect[4]={rc.left,rc.top,rc.right,rc.bottom};

		if(rect[rectPosition]==srcRectRef[rectPosition])
			foundWindows.push_back(child);

	}

	return foundWindows;

}


#pragma message (LOCATION " should SplitterContainer::OnMouseWheel call directly the child OnMouseWheel method?")



void SplitterContainer::CreateFloatingTab(TabContainer* tab)
{
	if(floatingTabRef)
		return;

	floatingTabRef=tab;
	floatingTabRefTabIdx=tab->selected;
	floatingTabRefTabCount=(int)tab->tabs.childs.size();

	RECT tmpRc;

	GetWindowRect(floatingTabRef->hwnd,&tmpRc);

	floatingTab=new TabContainer((float)tmpRc.left,(float)tmpRc.top,(float)(tmpRc.right-tmpRc.left),(float)(tmpRc.bottom-tmpRc.top),GetParent(tab->hwnd));

	EnableWindow(floatingTab->hwnd,false);

	printf("creating floating TabContainer %p\n",floatingTab);

}

void SplitterContainer::DestroyFloatingTab()
{
	if(floatingTabRef)
	{
		printf("deleting floating TabContainer %p\n",this);

		floatingTab->~TabContainer();//remove floating window
		floatingTabRef->mouseDown=false;
		floatingTabRef=0;
		floatingTab=0;
	}
}







void SplitterContainer::EnableChilds(HWND hwnd,int enable,int show)
{
	HWND child=0;
	while(child=FindWindowEx(hwnd,child ? child : 0,0,0))
	{
		if(enable>=0)EnableWindow(child,enable);
		if(show>=0)ShowWindow(child,show);
	}
}

void SplitterContainer::EnableAllChildsDescendants(HWND hwnd,int enable,int show)
{
	HWND child=0;
	while(child=FindWindowEx(hwnd,child ? child : 0,0,0))
		EnableChilds(child,enable,show);
}



bool InitSplitter()
{
	bool returnValue=true;

	{
		WNDCLASSEX wc={0};
		wc.cbSize=sizeof(WNDCLASSEX);
		wc.hCursor=LoadCursor(NULL, IDC_ARROW);
		wc.lpszClassName=WC_MAINAPPWINDOW;
		wc.lpfnWndProc=MainWindowProc;
		wc.hbrBackground=CreateSolidBrush(TabContainer::COLOR_MAIN_BACKGROUND);

		if(!RegisterClassEx(&wc))
			__debugbreak();
	}


	{
		WNDCLASS wc={0};

		wc.lpszClassName=WC_TABCONTAINERWINDOWCLASS;
		wc.lpfnWndProc=TabContainer::TabContainerWindowClassProcedure;
		wc.hbrBackground=CreateSolidBrush(RGB(0,255,0));
		wc.hCursor=(HCURSOR)LoadCursor(0,IDC_ARROW);
		wc.style=CS_VREDRAW|CS_HREDRAW|CS_PARENTDC;

		if(!RegisterClass(&wc))
			__debugbreak();
	}

	HMENU& popupMenuRoot=SplitterContainer::popupMenuRoot;
	HMENU& popupMenuCreate=SplitterContainer::popupMenuCreate;

	InsertMenu(popupMenuRoot,0,MF_BYPOSITION|MF_POPUP,(UINT_PTR)popupMenuCreate,"New Tab");
	{
		InsertMenu(popupMenuCreate,0,MF_BYPOSITION|MF_STRING,TAB_MENU_COMMAND_OPENGLWINDOW,"OpenGL Renderer");
		InsertMenu(popupMenuCreate,5,MF_BYPOSITION|MF_STRING,TAB_MENU_COMMAND_SHAREDOPENGLWINDOW,"Shared OpenGL Renderer");
		InsertMenu(popupMenuCreate,1,MF_BYPOSITION|MF_STRING,TAB_MENU_COMMAND_PROJECTFOLDER2,"Project Folder2");
		InsertMenu(popupMenuCreate,4,MF_BYPOSITION|MF_STRING,TAB_MENU_COMMAND_PROJECTFOLDER,"Project Folder");
		InsertMenu(popupMenuCreate,2,MF_BYPOSITION|MF_STRING,TAB_MENU_COMMAND_LOGGER,"Logger");
		InsertMenu(popupMenuCreate,3,MF_BYPOSITION|MF_STRING,TAB_MENU_COMMAND_SCENEENTITIES,"Scene Entities");
		InsertMenu(popupMenuCreate,6,MF_BYPOSITION|MF_STRING,TAB_MENU_COMMAND_ENTITYPROPERTIES,"EntityProperties");
	}
	InsertMenu(popupMenuRoot,1,MF_BYPOSITION|MF_SEPARATOR,0,0);
	InsertMenu(popupMenuRoot,2,MF_BYPOSITION|MF_STRING,TAB_MENU_COMMAND_REMOVE,"Remove Tab");

	return returnValue;
}



///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


void GuiProjectViewer::ScanDir(String dir)
{
	HANDLE handle;
	WIN32_FIND_DATA data;

	String scanDir=dir;
	scanDir+="\\*";

	handle=FindFirstFile(scanDir,&data); //. dir

	//printf("GuiProjectViewer::ScanDir(%s)\n",scanDir);

	if(!handle || INVALID_HANDLE_VALUE == handle)
	{
		__debugbreak();
		return;
	}
	else
		FindNextFile(handle,&data);

	while(FindNextFile(handle,&data))
	{
		if(data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
			continue;

		if(strstr(data.cFileName,".engine"))
		{
			String s = dir + "\\" + String(data.cFileName);

			if(!PathFileExists(s.Buf()))
			{
				//delete .engine file
				continue;
			}
		}
		else
		{
			String engineFile = dir + "\\" + String(data.cFileName) + ".engine";

			if(!PathFileExists(engineFile.Buf()))
			{
				FILE* f=fopen(engineFile.Buf(),"w");

				if(f)
				{
					unsigned char _true=1;
					unsigned char _false=0;

					fwrite(&_false,1,1,f);//selectedLeft
					fwrite(&_false,1,1,f);//selectedRight
					fwrite((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)? &_true : &_false,1,1,f);//isDir
					fwrite(&_false,1,1,f);//expanded
					fclose(f);

					if(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
						this->ScanDir(dir +"\\"+data.cFileName);

				}
			}
		}
	}

	FindClose(handle);
	handle=0;
}

void GuiProjectViewer::CreateNodes(String dir,ResourceNodeDir* iParent)
{
	HANDLE handle;
	WIN32_FIND_DATA data;

	String scanDir=dir;
	scanDir+="\\*";

	handle=FindFirstFile(scanDir,&data); //. dir

	//printf("GuiProjectViewer::CreateNodes(%s)\n",scanDir);

	if(!handle || INVALID_HANDLE_VALUE == handle)
	{
		__debugbreak();
		return;
	}
	else
		FindNextFile(handle,&data);

	while(FindNextFile(handle,&data))
	{
		if(data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
			continue;

		if(strstr(data.cFileName,".engine"))
		{
			String engineFile=dir + "\\" + String(data.cFileName);

			bool selectedLeft;
			bool selectedRight;
			bool isDir;
			bool expanded;

			{
				FILE* f=fopen(engineFile.Buf(),"r");

				if(f)
				{
					fread(&selectedLeft,1,1,f);
					fread(&selectedRight,1,1,f);
					fread(&isDir,1,1,f);
					fread(&expanded,1,1,f);

					fclose(f);
				}
			}

			std::string fileName(data.cFileName,&data.cFileName[strlen(data.cFileName)-7]);

			if(isDir)
			{
				ResourceNodeDir* dirNode=new ResourceNodeDir;

				iParent->dirs.push_back(dirNode);
				dirNode->parent=iParent;
				dirNode->fileName=fileName.c_str();
				dirNode->level = iParent ? iParent->level+1 : 0;
				dirNode->selectedLeft=selectedLeft;
				dirNode->selectedRight=selectedRight;
				dirNode->expanded=expanded;
				dirNode->isDir=isDir;

				this->CreateNodes(dir + "\\" + dirNode->fileName,dirNode);
			}
			else
			{
				ResourceNode* fileNode=new ResourceNode;

				iParent->files.push_back(fileNode);
				fileNode->parent=iParent;
				fileNode->fileName=fileName.c_str();
				fileNode->level = iParent ? iParent->level+1 : 0;
				fileNode->selectedLeft=selectedLeft;
				fileNode->selectedRight=selectedRight;
				fileNode->isDir=isDir;
			}
		}
	}

	FindClose(handle);
	handle=0;
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

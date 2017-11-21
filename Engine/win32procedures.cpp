#include "win32.h"

#include "win32msg.cpp"

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

void TabContainer::Draw()
{
	if(this->drawRect && this->drawFrame)
		__debugbreak();

	if(this->drawRect)
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




void TabContainer::OnGuiSize(void* data)
{
	WindowData::OnSize();

	this->tabs.rect.make(0,TabContainer::CONTAINER_HEIGHT,width,height-TabContainer::CONTAINER_HEIGHT);
	
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









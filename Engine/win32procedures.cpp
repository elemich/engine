#include "win32.h"

#pragma message(LOCATION " remember to use WindowData::CopyProcedureData to properly synchronize messages by the wndproc and the window herself")

LRESULT CALLBACK TabContainer::TabContainerWindowClassProcedure(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	TabContainer* tc=(TabContainer*)GetWindowLongPtr(hwnd,GWL_USERDATA);

	LPARAM result=0;

	if(tc)
		tc->CopyProcedureData(hwnd,msg,wparam,lparam);

	switch(msg)
	{
		case WM_CREATE:
		{
			result=DefWindowProc(hwnd,msg,wparam,lparam);

			LPCREATESTRUCT lpcs=(LPCREATESTRUCT)lparam;

			if(lpcs)
			{
				TabContainer* tabContainer=(TabContainer*)lpcs->lpCreateParams;
				SetWindowLongPtr(hwnd,GWL_USERDATA,(LONG_PTR)tabContainer);
				tabContainer->hwnd=hwnd;
				tabContainer->RecreateTarget();
			}
		}
		break;
		case WM_ERASEBKGND:
			/*if(tc->splitterContainer->floatingTabRef)
				tc->OnPaint();*/
			return 1;
		case WM_SIZE:
			tc->OnSize();
		break;
		case WM_DESTROY:
			printf("destroying\n");
			tc->~TabContainer();
			break;
		case WM_WINDOWPOSCHANGED:
			tc->OnWindowPosChanging();
		break;
		case WM_LBUTTONDOWN:
			tc->OnLMouseDown();
		break;
		case WM_MOUSEWHEEL:
			tc->OnMouseWheel();
		break;
		case WM_MOUSEMOVE:
			tc->OnMouseMove();
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
			tc->OnLMouseUp();
		break;
		case WM_RBUTTONUP:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			tc->CopyProcedureData(hwnd,msg,wparam,lparam);
			tc->OnRMouseUp();
		break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hwnd,&ps);
			
			tc->OnPaint();
			EndPaint(hwnd,&ps);
		}
		break;
		default:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			
	}

	return result;
}


unsigned char* TabContainer::rawUpArrow=0;
unsigned char* TabContainer::rawRightArrow=0;
unsigned char* TabContainer::rawDownArrow=0;
unsigned char* TabContainer::rawFolder=0;

TabContainer::TabContainer(float x,float y,float w,float h,HWND parent):
	parentContainer(0),
	renderer(0),
	brush(0),
	selected(0),
	mouseDown(false),
	isRender(false),
	splitterContainer(0),
	recreateTarget(true),
	upExpandos(0),
	rightExpandos(0),
	downExpandos(0),
	folderIcon(0)
{
	width=w;
	height=h;

	hwnd=CreateWindow(WC_TABCONTAINERWINDOWCLASS,WC_TABCONTAINERWINDOWCLASS,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,(int)x,(int)y,(int)w,(int)h,parent,0,0,this);

	if(!hwnd)
		__debugbreak();

	HWND parentWindow=GetParent(hwnd);

	this->parentContainer=(ContainerWindow*)GetWindowLongPtr(parent,GWL_USERDATA);
	this->parentContainer->tabContainers.push_back(this);

	splitterContainer=(SplitterContainer*)(ContainerWindow*)GetWindowLongPtr(GetParent(hwnd),GWL_USERDATA);

	if(!splitterContainer)
		__debugbreak();

	RecreateTarget();
}

TabContainer::~TabContainer()
{
	this->parentContainer->tabContainers.erase(std::find(parentContainer->tabContainers.begin(),parentContainer->tabContainers.end(),this));

	D2DRELEASE(renderer);
	D2DRELEASE(brush);
	D2DRELEASE(rightExpandos);
	D2DRELEASE(downExpandos);
	D2DRELEASE(folderIcon);
}

void TabContainer::OnSize()
{
	WindowData::OnSize();
	
	this->RecreateTarget();

	renderer->Resize(D2D1::SizeU((int)width,(int)height));

	if(this->GetSelected())
		this->GetSelected()->OnSize();

	this->OnPaint();
	
}

Gui* TabContainer::AddTab(Gui* gui,int position)
{
	if(gui)
	{
		gui->tab=this;

		std::vector<Gui*>& tabs=this->tabs;

		tabs.insert(position<0 ? tabs.end() : (tabs.begin() + position),gui);
		this->selected = position<0 ? (int)tabs.size()-1 : position;

		gui->OnReparent();

		this->OnPaint();

		return gui;
	}
	return 0;
}

int TabContainer::RemoveTab(Gui* gui)
{
	if(gui)
	{
		gui->tab=0;

		std::vector<Gui*>& tabs=this->tabs;

		std::vector<Gui*>::iterator iIt=std::find(tabs.begin(),tabs.end(),gui);

		if(iIt!=tabs.end())
		{
			int position=std::distance(tabs.begin(),iIt);
			tabs.erase(iIt);
			position>0 ? this->selected-- : 0;
			
			return position;
		}
	}
	return -1;
}


void TabContainer::RecreateTarget()
{
	if(!this->recreateTarget)
		return;

	HRESULT hr=S_OK;

	if(!rawUpArrow)
		Direct2DGuiBase::CreateRawBitmap(L"uparrow.png",rawUpArrow);

	if(!rawRightArrow)
		Direct2DGuiBase::CreateRawBitmap(L"rightarrow.png",rawRightArrow);

	if(!rawDownArrow)
		Direct2DGuiBase::CreateRawBitmap(L"downarrow.png",rawDownArrow);

	if(!rawFolder)
		Direct2DGuiBase::CreateRawBitmap(L"folder.png",rawFolder);

	


	D2DRELEASE(renderer);
	D2DRELEASE(brush);

	D2DRELEASE(upExpandos);
	D2DRELEASE(rightExpandos);
	D2DRELEASE(downExpandos);
	D2DRELEASE(folderIcon);

	renderer=Direct2DGuiBase::InitHWNDRenderer(hwnd);

	if(!renderer)
		__debugbreak();

	renderer->CreateSolidColorBrush(D2D1::ColorF(COLOR_TAB_BACKGROUND),&brush);

	if(!brush)
		__debugbreak();

	D2D1_BITMAP_PROPERTIES bp=D2D1::BitmapProperties();
	bp.pixelFormat=renderer->GetPixelFormat();
	bp.pixelFormat.alphaMode=D2D1_ALPHA_MODE_PREMULTIPLIED;

	hr=renderer->CreateBitmap(D2D1::SizeU(TabContainer::CONTAINER_ARROW_WH,TabContainer::CONTAINER_ARROW_WH),TabContainer::rawUpArrow,TabContainer::CONTAINER_ARROW_STRIDE,bp,&upExpandos);

	if(!upExpandos || hr!=S_OK)
		__debugbreak();

	hr=renderer->CreateBitmap(D2D1::SizeU(TabContainer::CONTAINER_ARROW_WH,TabContainer::CONTAINER_ARROW_WH),TabContainer::rawRightArrow,TabContainer::CONTAINER_ARROW_STRIDE,bp,&rightExpandos);

	if(!rightExpandos || hr!=S_OK)
		__debugbreak();

	hr=renderer->CreateBitmap(D2D1::SizeU(TabContainer::CONTAINER_ARROW_WH,TabContainer::CONTAINER_ARROW_WH),TabContainer::rawDownArrow,TabContainer::CONTAINER_ARROW_STRIDE,bp,&downExpandos);

	if(!downExpandos || hr!=S_OK)
		__debugbreak();

	hr=renderer->CreateBitmap(D2D1::SizeU(TabContainer::CONTAINER_ARROW_WH,TabContainer::CONTAINER_ARROW_WH),TabContainer::rawFolder,TabContainer::CONTAINER_ARROW_STRIDE,bp,&folderIcon);

	if(!folderIcon || hr!=S_OK)
		__debugbreak();

	if(this->GetSelected())
		this->GetSelected()->RecreateTarget();

	this->recreateTarget=false;
}

void TabContainer::OnWindowPosChanging()
{
	WindowData::OnWindowPosChanging();

	this->RecreateTarget();

	renderer->Resize(D2D1::SizeU((int)width,(int)height));

	if(this->GetSelected())
		this->GetSelected()->OnSize();

	//this->OnPaint();
}

void TabContainer::OnMouseMove()
{
	SetFocus(this->hwnd);

	mousex=(float)LOWORD(lparam);
	mousey=(float)HIWORD(lparam);

	splitterContainer->currentTabContainer=this;

	if(mouseDown)
		splitterContainer->CreateFloatingTab(this);

	if(this->mousey<=TabContainer::CONTAINER_HEIGHT)
		return;

	if(this->GetSelected())
		this->GetSelected()->OnMouseMove();
}

void TabContainer::OnLMouseUp()
{
	mouseDown=false;

	if(this->GetSelected())
		this->GetSelected()->OnLMouseUp();
}

void TabContainer::OnMouseWheel()
{
	if(this->mousey<=TabContainer::CONTAINER_HEIGHT)
		return;

	if(this->GetSelected())
		this->GetSelected()->OnMouseWheel();
}

void TabContainer::OnLMouseDown()
{
	this->OnMouseMove();

	float &x=this->mousex;
	float &y=this->mousey;

	if(y<CONTAINER_HEIGHT)
	{
		int prevSel=selected;

		for(int i=0;i<(int)tabs.size();i++)
		{
			if(x>(i*TAB_WIDTH) && x< (i*TAB_WIDTH+TAB_WIDTH) && y > (CONTAINER_HEIGHT-TAB_HEIGHT) &&  y<CONTAINER_HEIGHT)
			{
				selected=i;

				mouseDown=true;

				if(prevSel!=selected)
					this->OnPaint();

				break;
			}
		}
	}
	else
	{
		if(tabs[selected])
			tabs[selected]->OnLMouseDown();
	}
}

void TabContainer::OnRun()
{
	/*if(tabcontainer_mouseDown)
	{
		if(tabcontainer_tabs[tabcontainer_selected])
			tabcontainer_tabs[tabcontainer_selected]->OnRun();
	}*/

	/*for(int i=0;i<(int)tabcontainer_tabs.size();i++)
	{
		GuiInterface* renderer=tabcontainer_tabs[i]->GetRendererViewport();
		if(renderer)				
			renderer->OnPaint();
	}*/
}

void TabContainer::OnRMouseUp()
{
	this->OnMouseMove();

	float &x=this->mousex;
	float &y=this->mousey;

	RECT rc;
	GetWindowRect(hwnd,&rc);

	for(int i=0;i<(int)tabs.size();i++)
	{
		if(x>(i*TAB_WIDTH) && x< (i*TAB_WIDTH+TAB_WIDTH) && y > (CONTAINER_HEIGHT-TAB_HEIGHT) &&  y<CONTAINER_HEIGHT)
		{
			int menuResult=TrackPopupMenu(SplitterContainer::popupMenuRoot,TPM_RETURNCMD |TPM_LEFTALIGN|TPM_TOPALIGN,rc.left+LOWORD(lparam),rc.top+HIWORD(lparam),0,GetParent(hwnd),0);

			switch(menuResult)
			{
				case TAB_MENU_COMMAND_OPENGLWINDOW:
					this->AddTab(new OpenGLRenderer(this));
				break;
				case TAB_MENU_COMMAND_SCENEENTITIES:
					this->AddTab(new TreeView(this));
				break;
				case TAB_MENU_COMMAND_ENTITYPROPERTIES:
					this->AddTab(new Properties(this));
				break;
				case TAB_MENU_COMMAND_PROJECTFOLDER:
					this->AddTab(new Resources(this));
				break;
			}

			break;
		}
	}
}

void TabContainer::OnPaint()
{
	printf("painting %p\n",this);

	if(this->recreateTarget)
		this->RecreateTarget();
	
	isRender=true;

	renderer->BeginDraw();

	renderer->Clear(D2D1::ColorF(D2D1::ColorF::Black));

	renderer->SetTransform(D2D1::Matrix3x2F::Identity());

	renderer->FillRectangle(D2D1::RectF(0,0,(float)width,(float)CONTAINER_HEIGHT),this->SetColor(COLOR_TAB_BACKGROUND));
	renderer->FillRectangle(D2D1::RectF((float)(selected*TAB_WIDTH),(float)(CONTAINER_HEIGHT-TAB_HEIGHT),(float)(selected*TAB_WIDTH+TAB_WIDTH),(float)((CONTAINER_HEIGHT-TAB_HEIGHT)+TAB_HEIGHT)),this->SetColor(COLOR_TAB_SELECTED));

	Direct2DGuiBase::texter->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	for(int i=0;i<(int)tabs.size();i++)
	{
		Direct2DGuiBase::DrawText(renderer,this->SetColor(COLOR_TEXT),tabs[i]->name,(float)i*TAB_WIDTH,(float)CONTAINER_HEIGHT-TAB_HEIGHT,(float)TAB_WIDTH,(float)TAB_HEIGHT);
	}
	Direct2DGuiBase::texter->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	
	if(this->GetSelected())
		this->GetSelected()->OnPaint();

	this->recreateTarget=(renderer->EndDraw() & D2DERR_RECREATE_TARGET) != 0;

	isRender=false;
}

void TabContainer::OnResizeContainer()
{
	RECT rc;

	GetClientRect(this->hwnd,&rc);
	MapWindowRect(this->hwnd,this->parentContainer->hwnd,&rc);

	switch(this->parentContainer->resizeEnumType)
	{
		case 0:
			SetWindowPos(this->hwnd,0,0,rc.top,rc.right+this->parentContainer->resizeDiffWidth,rc.bottom-rc.top,/*SWP_NOREDRAW|*/SWP_NOZORDER|SWP_NOOWNERZORDER);
			break;
		case 1:
			SetWindowPos(this->hwnd,0,rc.left,rc.top,rc.right-rc.left,rc.bottom+this->parentContainer->resizeDiffHeight,/*SWP_NOREDRAW|*/SWP_NOZORDER|SWP_NOOWNERZORDER);
			break;
		case 2:
			SetWindowPos(this->hwnd,0,rc.left,rc.top,rc.right+this->parentContainer->resizeDiffWidth,rc.bottom-rc.top,/*SWP_NOREDRAW|*/SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOMOVE);
			break;
		case 3:
			SetWindowPos(this->hwnd,0,rc.left,rc.top,rc.right-rc.left,rc.bottom+this->parentContainer->resizeDiffHeight,/*SWP_NOREDRAW|*/SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOMOVE);
		break;
		default:
			__debugbreak();
	}
}
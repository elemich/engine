#include "win32.h"



LRESULT CALLBACK TabContainer::TabContainerWindowClassProcedure(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	TabContainer* tc=(TabContainer*)GetWindowLongPtr(hwnd,GWL_USERDATA);

	LPARAM result=0;

	switch(msg)
	{
		case WM_CREATE:
		{
			result=DefWindowProc(hwnd,msg,wparam,lparam);

			LPCREATESTRUCT lpcs=(LPCREATESTRUCT)lparam;

			if(lpcs)
				SetWindowLongPtr(hwnd,GWL_USERDATA,(LONG_PTR)lpcs->lpCreateParams);
		}
		break;
		case WM_ERASEBKGND:
			/*if(tc->splitterContainer->childMovingRef)
				tc->OnPaint();*/
			return 1;
		case WM_SIZE:
			tc->OnSize(lparam);
		break;
		case WM_DESTROY:
			printf("destroying\n");
			delete tc;
			break;
		case WM_WINDOWPOSCHANGED:
			tc->OnWindowPosChanging(lparam);
		break;
		case WM_LBUTTONDOWN:
			tc->OnLMouseDown(lparam);
		break;
		case WM_MOUSEMOVE:
			tc->OnMouseMove(lparam);
		break;
		case WM_NCHITTEST:
			if(tc->splitterContainer->childMovingRef)
				return HTTRANSPARENT;
			else 
				result=DefWindowProc(hwnd,msg,wparam,lparam);
			break;
		case WM_LBUTTONUP:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			tc->OnLMouseUp(lparam);
		break;
		case WM_RBUTTONUP:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			tc->OnRMouseUp(lparam);
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


unsigned char* TabContainer::rawRightArrow=0;
unsigned char* TabContainer::rawDownArrow=0;

TabContainer::TabContainer(int x,int y,int w,int h,HWND parent):
	renderer(0),
	brush(0),
	selected(0),
	mouseDown(false),
	isRender(false),
	splitterContainer(0),
	recreateTarget(false)
{
	width=w;
	height=h;

	hwnd=CreateWindow(WC_TABCONTAINERWINDOWCLASS,WC_TABCONTAINERWINDOWCLASS,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,x,y,w,h,parent,0,0,this);

	if(!hwnd)
		__debugbreak();

	splitterContainer=(SplitterContainer*)(ContainerWindow*)GetWindowLongPtr(GetParent(hwnd),GWL_USERDATA);

	if(!splitterContainer)
		__debugbreak();

	RecreateTarget();
}


void TabContainer::OnSize(LPARAM lparam)
{
	WindowData::OnSize(lparam);
	renderer->Resize(D2D1::SizeU(width,height));

	if(tabs.size() && tabs[selected])
		tabs[selected]->OnSize();

	this->OnPaint();
}

GuiInterface* TabContainer::AddTab(GuiInterface* gui,int position)
{
	if(gui)
	{
		gui->tab=this;

		std::vector<GuiInterface*>& tabs=this->tabs;

		tabs.insert(position<0 ? tabs.end() : tabs.begin() + position,gui);
		this->selected = position<0 ? (int)tabs.size()-1 : position;

		
		gui->OnReparent();

		this->OnPaint();

		return gui;
	}
	return 0;
}

int TabContainer::RemoveTab(GuiInterface* gui)
{
	if(gui)
	{
		gui->tab=0;

		std::vector<GuiInterface*>& tabs=this->tabs;

		std::vector<GuiInterface*>::iterator iIt=std::find(tabs.begin(),tabs.end(),gui);

		if(iIt!=tabs.end())
		{
			int position=std::distance(tabs.begin(),iIt);
			tabs.erase(iIt);
			position>0 ? this->selected-- : this->selected++;
			
			return position;
		}
	}
	return -1;
}


void TabContainer::RecreateTarget()
{
	if(!rawRightArrow)
		Direct2DGuiBase::CreateRawBitmap(L"rightarrow.png",rawRightArrow);

	if(!rawDownArrow)
		Direct2DGuiBase::CreateRawBitmap(L"downarrow.png",rawDownArrow);

	if(renderer)renderer->Release();
	if(brush)brush->Release();

	renderer=Direct2DGuiBase::InitHWNDRenderer(hwnd);

	renderer->CreateSolidColorBrush(D2D1::ColorF(COLOR_TAB_BACKGROUND),&brush);

	if(tabs.size() && tabs[selected])
		tabs[selected]->RecreateTarget();
}

void TabContainer::OnWindowPosChanging(LPARAM lparam)
{
	WindowData::OnWindowPosChanging(lparam);

	renderer->Resize(D2D1::SizeU(width,height));

	if(tabs.size() && tabs[selected])
		tabs[selected]->OnSize();

	this->OnPaint();
}

void TabContainer::OnMouseMove(LPARAM lparam)
{
	mousex=(float)LOWORD(lparam);
	mousey=(float)HIWORD(lparam);

	splitterContainer->currentTabContainer=this;

	if(mouseDown)
		splitterContainer->CreateFloatingTab(this);
}

void TabContainer::OnLMouseUp(LPARAM lparam)
{
	mouseDown=false;
}

void TabContainer::OnLMouseDown(LPARAM lparam)
{
	OnMouseMove(lparam);

	

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

void TabContainer::OnRMouseUp(LPARAM lparam)
{
	OnMouseMove(lparam);

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
			}

			break;
		}
	}
}

void TabContainer::OnPaint()
{
	if(this->recreateTarget)
		this->RecreateTarget();
	
	isRender=true;

	renderer->BeginDraw();

	renderer->SetTransform(D2D1::Matrix3x2F::Identity());

	renderer->FillRectangle(D2D1::RectF(0,0,(float)width,(float)CONTAINER_HEIGHT),this->SetColor(COLOR_TAB_BACKGROUND));
	renderer->FillRectangle(D2D1::RectF((float)(selected*TAB_WIDTH),(float)(CONTAINER_HEIGHT-TAB_HEIGHT),(float)(selected*TAB_WIDTH+TAB_WIDTH),(float)((CONTAINER_HEIGHT-TAB_HEIGHT)+TAB_HEIGHT)),this->SetColor(COLOR_TAB_SELECTED));

	Direct2DGuiBase::texter->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	for(int i=0;i<(int)tabs.size();i++)
	{
		Direct2DGuiBase::DrawText(renderer,this->SetColor(COLOR_TEXT),tabs[i]->name,(float)i*TAB_WIDTH,(float)CONTAINER_HEIGHT-TAB_HEIGHT,(float)TAB_WIDTH,(float)TAB_HEIGHT);
	}
	Direct2DGuiBase::texter->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	
	if(tabs.size() && tabs[selected])
		tabs[selected]->OnPaint();

	this->recreateTarget=(renderer->EndDraw() & D2DERR_RECREATE_TARGET) != 0;

	isRender=false;
}


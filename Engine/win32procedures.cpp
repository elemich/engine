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


TabContainer::TabContainer(int x,int y,int w,int h,HWND parent):
	tabcontainer_renderer(0),
	tabcontainer_dcRenderer(0),
	tabcontainer_tabBackgroundBrush(0),
	tabcontainer_backgroundBrush(0),
	tabcontainer_tabBrush(0),
	tabcontainer_textBrush(0),
	tabcontainer_selected(0),
	tabcontainer_mouseDown(false),
	tabcontainer_isRender(false),
	splitterContainer(0),
	tabcontainer_recreateTarget(false)
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
	tabcontainer_renderer->Resize(D2D1::SizeU(width,height));

	RECT rc={0,0,width,height};
	tabcontainer_dcRenderer->BindDC(GetDC(hwnd),&rc);

	if(tabcontainer_tabs.size() && tabcontainer_tabs[tabcontainer_selected])
		tabcontainer_tabs[tabcontainer_selected]->OnSize();

	this->OnPaint();
}

GuiInterface* TabContainer::AddTab(GuiInterface* gui,int position)
{
	if(gui)
	{
		gui->guiinterface_tabcontainer=this;

		std::vector<GuiInterface*>& tabs=this->tabcontainer_tabs;

		tabs.insert(position<0 ? tabs.end() : tabs.begin() + position,gui);
		this->tabcontainer_selected = position<0 ? (int)tabs.size()-1 : position;

		
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
		gui->guiinterface_tabcontainer=0;

		std::vector<GuiInterface*>& tabs=this->tabcontainer_tabs;

		std::vector<GuiInterface*>::iterator iIt=std::find(tabs.begin(),tabs.end(),gui);

		if(iIt!=tabs.end())
		{
			int position=std::distance(tabs.begin(),iIt);
			tabs.erase(iIt);
			position>0 ? this->tabcontainer_selected-- : this->tabcontainer_selected++;
			
			return position;
		}
	}
	return -1;
}


void TabContainer::RecreateTarget()
{
	if(tabcontainer_renderer)tabcontainer_renderer->Release();
	if(tabcontainer_tabBackgroundBrush)tabcontainer_tabBackgroundBrush->Release();
	if(tabcontainer_backgroundBrush)tabcontainer_backgroundBrush->Release();
	if(tabcontainer_tabBrush)tabcontainer_tabBrush->Release();
	if(tabcontainer_textBrush)tabcontainer_textBrush->Release();

	tabcontainer_renderer=Direct2DGuiBase::InitHWNDRenderer(hwnd);

	tabcontainer_renderer->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightGray),&tabcontainer_tabBackgroundBrush);
	tabcontainer_renderer->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray),&tabcontainer_backgroundBrush);
	tabcontainer_renderer->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue),&tabcontainer_tabBrush);
	tabcontainer_renderer->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),&tabcontainer_textBrush);

	if(tabcontainer_tabs.size() && tabcontainer_tabs[tabcontainer_selected])
		tabcontainer_tabs[tabcontainer_selected]->RecreateTarget();
}

void TabContainer::OnWindowPosChanging(LPARAM lparam)
{
	WindowData::OnWindowPosChanging(lparam);

	tabcontainer_renderer->Resize(D2D1::SizeU(width,height));

	if(tabcontainer_tabs.size() && tabcontainer_tabs[tabcontainer_selected])
		tabcontainer_tabs[tabcontainer_selected]->OnSize();

	this->OnPaint();
}

void TabContainer::OnMouseMove(LPARAM lparam)
{
	tabcontainer_mousex=(float)LOWORD(lparam);
	tabcontainer_mousey=(float)HIWORD(lparam);

	splitterContainer->currentTabContainer=this;

	if(tabcontainer_mouseDown)
		splitterContainer->CreateFloatingTab(this);
}

void TabContainer::OnLMouseUp(LPARAM lparam)
{
	tabcontainer_mouseDown=false;
}

void TabContainer::OnLMouseDown(LPARAM lparam)
{
	OnMouseMove(lparam);

	

	float &x=this->tabcontainer_mousex;
	float &y=this->tabcontainer_mousey;

	if(y<CONTAINER_HEIGHT)
	{
		int prevSel=tabcontainer_selected;

		for(int i=0;i<(int)tabcontainer_tabs.size();i++)
		{
			if(x>(i*CONTAINER_TAB_WIDTH) && x< (i*CONTAINER_TAB_WIDTH+CONTAINER_TAB_WIDTH) && y > (CONTAINER_HEIGHT-CONTAINER_TAB_HEIGHT) &&  y<CONTAINER_HEIGHT)
			{
				tabcontainer_selected=i;

				tabcontainer_mouseDown=true;

				if(prevSel!=tabcontainer_selected)
					this->OnPaint();

				break;
			}
		}
	}
	else
	{
		if(tabcontainer_tabs[tabcontainer_selected])
			tabcontainer_tabs[tabcontainer_selected]->OnLMouseDown();
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

	float &x=this->tabcontainer_mousex;
	float &y=this->tabcontainer_mousey;

	RECT rc;
	GetWindowRect(hwnd,&rc);

	for(int i=0;i<(int)tabcontainer_tabs.size();i++)
	{
		if(x>(i*CONTAINER_TAB_WIDTH) && x< (i*CONTAINER_TAB_WIDTH+CONTAINER_TAB_WIDTH) && y > (CONTAINER_HEIGHT-CONTAINER_TAB_HEIGHT) &&  y<CONTAINER_HEIGHT)
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
	if(this->tabcontainer_recreateTarget)
		this->RecreateTarget();
	
	tabcontainer_isRender=true;

	tabcontainer_renderer->BeginDraw();

	tabcontainer_renderer->SetTransform(D2D1::Matrix3x2F::Identity());

	Direct2DGuiBase::DrawRectangle(tabcontainer_renderer,0,0,width,CONTAINER_HEIGHT,tabcontainer_tabBackgroundBrush);
	Direct2DGuiBase::DrawRectangle(tabcontainer_renderer,tabcontainer_selected*CONTAINER_TAB_WIDTH,CONTAINER_HEIGHT-CONTAINER_TAB_HEIGHT,CONTAINER_TAB_WIDTH,CONTAINER_TAB_HEIGHT,tabcontainer_tabBrush);

	Direct2DGuiBase::texter->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	for(int i=0;i<(int)tabcontainer_tabs.size();i++)
	{
		Direct2DGuiBase::DrawText(tabcontainer_renderer,tabcontainer_textBrush,tabcontainer_tabs[i]->guiinterface_tabName,i*CONTAINER_TAB_WIDTH,CONTAINER_HEIGHT-CONTAINER_TAB_HEIGHT,CONTAINER_TAB_WIDTH,CONTAINER_TAB_HEIGHT);
	}
	Direct2DGuiBase::texter->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	
	if(tabcontainer_tabs.size() && tabcontainer_tabs[tabcontainer_selected])
		tabcontainer_tabs[tabcontainer_selected]->OnPaint();

	this->tabcontainer_recreateTarget=tabcontainer_renderer->EndDraw() & D2DERR_RECREATE_TARGET;

	tabcontainer_isRender=false;
}


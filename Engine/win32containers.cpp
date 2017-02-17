#include "win32.h"

extern App* ___app;

WNDPROC SystemOriginalTabControlProcedure;

ContainerWindow::ContainerWindow(){}
void ContainerWindow::Create(HWND){}

MainAppContainerWindow::MainAppContainerWindow(){};
void MainAppContainerWindow::Create(HWND)
{
	hwnd=CreateWindow(WC_MAINAPPWINDOW,WC_MAINAPPWINDOW,WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,0,0,0,0);
	SetWindowLongPtr(hwnd,GWL_USERDATA,(LONG)this);

	RECT rc;
	GetClientRect(hwnd,&rc);
	HWND firstChild=CreateTabContainer(0,0,rc.right-rc.left,rc.bottom-rc.top,hwnd,tabContainerCount);
	CreateProjectFolder(firstChild);
	MainNode=new WINDOWNODE(firstChild,0,0,0,0);

	ShowWindow(hwnd,true);
}

void CreateProjectFolder(HWND hwnd)
{
	int tabIdx=CreateTab(hwnd);
	BROWSER* folderBrowser=new BROWSER();
	if(folderBrowser)
	{
		folderBrowser->Create(hwnd);
		___app->browsers.push_back(folderBrowser);
		SetParent(hwnd,folderBrowser->hwnd);
		SetWindowLong(folderBrowser->hwnd,GWL_ID,(LONG)tabIdx);
		EnableAndShowContainerChild(hwnd,tabIdx);
	}
}

void CreateOpenglWindow(HWND hwnd)
{
	int tabIdx=CreateTab(hwnd);
	OpenGLFixedRenderer* renderer=new OpenGLFixedRenderer();
	if(renderer)
	{
		renderer->Create(hwnd);
		___app->renderers.push_back(renderer);
		SetWindowLong(renderer->hwnd,GWL_ID,(LONG)tabIdx);
		EnableAndShowContainerChild(hwnd,tabIdx);
	}
}

void CreateLogger(HWND hwnd)
{
	int tabIdx=CreateTab(hwnd);
	Logger* logger=new Logger();
	if(logger)
	{
		logger->Create(hwnd);
		___app->loggers.push_back(logger);
		SetWindowLong(logger->hwnd,GWL_ID,(LONG)tabIdx);
		EnableAndShowContainerChild(hwnd,tabIdx);
	}
}




LRESULT CALLBACK MainWindowProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	MainAppContainerWindow* mainw=(MainAppContainerWindow*)GetWindowLongPtr(hwnd,GWL_USERDATA);

	LRESULT result=0;

	switch(msg)
	{
		case WM_CLOSE:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			PostQuitMessage(1);
		break;
		case WM_ERASEBKGND:
			return (LRESULT)1;
		case WM_LBUTTONDOWN:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			mainw->OnLButtonDown(hwnd,lparam);
		break;
		
		case WM_LBUTTONUP:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			mainw->OnLButtonUp(hwnd);
		break;
		case WM_MOUSEMOVE:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			mainw->OnMouseMove(hwnd,lparam);
		break;
		
			default:
				result=DefWindowProc(hwnd,msg,wparam,lparam);
	}

	return result;
}


LRESULT CALLBACK TabProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	ContainerWindow* cw=(ContainerWindow*)GetWindowLongPtr(GetParent(hwnd),GWL_USERDATA);

	LRESULT result=0;

	switch(msg)
	{
		case WM_LBUTTONDOWN:
			result=CallWindowProc(SystemOriginalTabControlProcedure,hwnd,msg,wparam,lparam);
			cw->OnTabContainerLButtonDown(hwnd);
		break;
		case WM_LBUTTONUP:
			result=CallWindowProc(SystemOriginalTabControlProcedure,hwnd,msg,wparam,lparam);
			cw->OnTabContainerLButtonUp(hwnd);
		break;
		case WM_SIZE:
		case WM_WINDOWPOSCHANGED:	
			//result=CallWindowProc(TabControlProcedure,hwnd,msg,wparam,lparam);
			OnTabSizeChanged(hwnd);
		break;
		case WM_RBUTTONUP:	
			result=CallWindowProc(SystemOriginalTabControlProcedure,hwnd,msg,wparam,lparam);
			switch(cw->OnTabContainerRButtonUp(hwnd,lparam))
			{
				case 1:
					CreateOpenglWindow(hwnd);
				break;
				case 2:
					CreateProjectFolder(hwnd);
				break;
				case 3:
					CreateLogger(hwnd);
				break;
				/*case 2://remove
					CreateTab(hwnd);
					break;*/
			}
		break;
		default:
			result=CallWindowProc(SystemOriginalTabControlProcedure,hwnd,msg,wparam,lparam);
		
	}
	return result;
}


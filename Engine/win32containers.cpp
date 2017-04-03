#include "win32.h"



WNDPROC SystemOriginalTabControlProcedure;

ContainerWindow::ContainerWindow():ContainerWindow_currentVisibleChildIdx(0){}
void ContainerWindow::Create(HWND container){}

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
	TabContainer* tabContainer=new TabContainer(0,0,rc.right-rc.left,rc.bottom-rc.top,hwnd);

	if(tabContainer)
		tabContainer->AddTab(new OpenGLRenderer(tabContainer));

	/*CreateNewPanel(firstTabContainerChild,TAB_MENU_COMMAND_OPENGLWINDOW);
	CreateNewPanel(firstTabContainerChild,TAB_MENU_COMMAND_SCENEENTITIES);

	*/

	ShowWindow(hwnd,true);
}





LRESULT CALLBACK MainWindowProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	MainAppContainerWindow* mainw=(MainAppContainerWindow*)GetWindowLongPtr(hwnd,GWL_USERDATA);

	LRESULT result=0;

	switch(msg)
	{
		case WM_CREATE:
		{
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			SetWindowLongPtr(hwnd,GWL_USERDATA,(LONG_PTR)((LPCREATESTRUCT)lparam)->lpCreateParams);
		}
		break;
		case WM_CLOSE:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			PostQuitMessage(1);
		break;
		/*case WM_ERASEBKGND:
			return (LRESULT)1;*/
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
								InitFbxSceneLoad(openfilename.lpstrFile);

								for(int i=0;i<(int)GuiInterface::guiInterfacesPool.size();i++)
									GuiInterface::guiInterfacesPool[i]->OnEntitiesChange();
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


LRESULT CALLBACK TabProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	ContainerWindow* cw=(ContainerWindow*)GetWindowLongPtr(GetParent(hwnd),GWL_USERDATA);

	LRESULT result=0;

	/*switch(msg)
	{
		case WM_LBUTTONDOWN:
			result=CallWindowProc(SystemOriginalTabControlProcedure,hwnd,msg,wparam,lparam);
			cw->OnTabContainerLButtonDown(hwnd);
		break;
		/ *case WM_MOUSEWHEEL:
		{
			
			printf("tabcontainerwindow");
			//result=CallWindowProc(SystemOriginalTabControlProcedure,hwnd,msg,wparam,lparam);
			SendMessage(GetDlgItem(hwnd,cw->ContainerWindow_currentVisibleChildIdx),WM_MOUSEWHEEL,0,0);
		}
		break;* /
		/ *case WM_MOUSEMOVE:
			result=CallWindowProc(SystemOriginalTabControlProcedure,hwnd,msg,wparam,lparam);
			SetFocus(hwnd);
		break;* /
		case WM_LBUTTONUP:
			result=CallWindowProc(SystemOriginalTabControlProcedure,hwnd,msg,wparam,lparam);
			cw->OnTabContainerLButtonUp(hwnd);
			cw->ContainerWindow_currentVisibleChildIdx=cw->childMovingRefTabIdx;
		break;
		/ *case WM_SIZING:
			result=CallWindowProc(SystemOriginalTabControlProcedure,hwnd,msg,wparam,lparam);
			break;* /
		case WM_SIZE:
		case WM_WINDOWPOSCHANGED:	
			//result=CallWindowProc(SystemOriginalTabControlProcedure,hwnd,msg,wparam,lparam);
			cw->OnTabContainerSize(hwnd);
		break;
		
		case WM_RBUTTONUP:	
		{	
			result=CallWindowProc(SystemOriginalTabControlProcedure,hwnd,msg,wparam,lparam);
			cw->CreateNewPanel(hwnd,cw->OnTabContainerRButtonUp(hwnd,lparam));
		}
		break;
		default:
			result=CallWindowProc(SystemOriginalTabControlProcedure,hwnd,msg,wparam,lparam);
		
	}*/
	return result;
}


#include "win32.h"


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
	MainAppContainerWindow* mainw=(MainAppContainerWindow*)GetWindowLongPtr(hwnd,GWL_USERDATA);

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
			SetWindowLongPtr(hwnd,GWL_USERDATA,(LONG_PTR)((LPCREATESTRUCT)lparam)->lpCreateParams);
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
								Entity* importedEntities=ImportFbxScene(openfilename.lpstrFile);

								TabContainer::BroadcastToPool(&TabContainer::OnEntitiesChange,importedEntities);
								
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

Task* Thread::NewTask(std::function<void()> iFunction,bool iRemove,bool iBlock)
{
	Task* task=new Task;

	task->func=iFunction;
	task->remove=iRemove;
	task->executing=false;
	task->pause=iBlock;
	task->owner=this;
	
	tasks.push_back(task);

	return task;
}





#include "win32.h"


#pragma message (LOCATION " beware to new POD initialization (parhentesized or not)")

extern WNDPROC SystemOriginalTabControlProcedure;
extern WNDPROC SystemOriginalSysTreeView32ControlProcedure;

SplitterContainer::SplitterContainer():splitterSize(4)
{
	tabContainerCount=0;

	childMovingRef=0;
	childMoving=0;
	childMovingTarget=0;
	childMovingRefTabIdx=-1;
	childMovingRefTabCount=-1;
	childMovingTargetAnchorPos=-1;
	childMovingTargetAnchorTabIndex=-1;

	splitterCursor=IDC_ARROW;
}
SplitterContainer::~SplitterContainer()
{
}

HMENU SplitterContainer::popupMenuRoot=CreatePopupMenu();
HMENU SplitterContainer::popupMenuCreate=CreatePopupMenu();

void SplitterContainer::OnLButtonDown(HWND hwnd,LPARAM lparam)
{
	if(!GetCapture() && !childMovingRef && splitterCursor!=IDC_ARROW)
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
	printf("main L button up\n");

	if(childMovingRef)
	{
		EnableAllChildsDescendants(hwnd,true);

		if(childMoving)
		{
			if(childMovingTarget)
			{
				if(childMovingTargetAnchorPos!=-1)
				{
					TabContainer* newChild=new TabContainer(childMovingRc.left,childMovingRc.top,childMovingRc.right-childMovingRc.left,childMovingRc.bottom-childMovingRc.top,hwnd);
					SetWindowPos(childMovingTarget->hwnd,0,childMovingTargetRc.left,childMovingTargetRc.top,childMovingTargetRc.right-childMovingTargetRc.left,childMovingTargetRc.bottom-childMovingTargetRc.top,SWP_SHOWWINDOW);

					childMovingTarget->LinkSibling(newChild,childMovingTargetAnchorPos);

					Gui* reparentTab=childMovingRef->tabs[childMovingRefTabIdx];
					childMovingRef->RemoveTab(reparentTab);
					newChild->AddTab(reparentTab);

					
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

		if(SPLITTER_DEBUG)
			printf("unset capture\n");
	}
}

void SplitterContainer::OnMouseMove(HWND hwnd,LPARAM lparam)
{
	POINTS p=MAKEPOINTS(lparam);

	if(childMovingRef)
	{
		RECT targetRc;
		RECT tmpRc;

		GetWindowRect(childMovingRef->hwnd,&tmpRc);

		int scalesize=3;

		int childMovingScaledWidthX=(tmpRc.right-tmpRc.left)/scalesize;
		int childMovingScaledWidthY=(tmpRc.bottom-tmpRc.top)/scalesize;

		POINT cp={p.x,p.y};

		HWND target=ChildWindowFromPointEx(hwnd,cp,CWP_SKIPDISABLED);

		if(target!=childMoving->hwnd && /*childMovingTarget!=childMovingRef*/(childMovingRefTabCount==1 ? target!=childMovingRef->hwnd : true))
		{
			childMovingTarget=(TabContainer*)GetWindowLongPtr(target,GWL_USERDATA);

			int hh=TabContainer::CONTAINER_HEIGHT;

			GetClientRect(target,&targetRc);
			tmpRc=targetRc;
			MapWindowRect(target,hwnd,&tmpRc);

			int w=tmpRc.right-tmpRc.left;
			int h=tmpRc.bottom-tmpRc.top;

			int wd=w/3;
			int hd=h/3;

			childMovingTargetRc=childMovingRc=tmpRc;
			int anchor=false;

			/*if(cp.y>tmpRc.top && cp.y<tmpRc.top+hh)
			{
				childMovingTargetAnchorPos=-1;
				anchor=2;
			}
			else */if(cp.x>tmpRc.left && cp.x<tmpRc.left+wd)
			{
				childMovingTargetAnchorPos=0;
				childMovingRc.right=tmpRc.left+wd;
				childMovingTargetRc.left=childMovingRc.right+splitterSize;
				anchor=1;
			}
			else if(cp.y>tmpRc.top+hh && cp.y<tmpRc.top+hd)
			{
				childMovingTargetAnchorPos=1;
				childMovingRc=tmpRc;
				childMovingRc.bottom=tmpRc.top+hd;
				childMovingTargetRc.top=childMovingRc.bottom+splitterSize;
				anchor=1;
			}
			else if(cp.x>tmpRc.right-wd && cp.x<tmpRc.right)
			{
				childMovingTargetAnchorPos=2;
				childMovingRc=tmpRc;
				childMovingRc.left=tmpRc.right-wd;
				childMovingTargetRc.right=childMovingRc.left-splitterSize;
				anchor=1;
			}
			else if(cp.y>tmpRc.bottom-hd && cp.y<tmpRc.bottom)
			{
				childMovingTargetAnchorPos=3;
				childMovingRc=tmpRc;
				childMovingRc.top=tmpRc.bottom-hd;
				childMovingTargetRc.bottom=childMovingRc.top-splitterSize;
				anchor=1;
			}

			

			if(anchor==2)
			{
				/*ShowWindow(childMoving,false);
				if(childMovingTargetAnchorTabIndex<0)
					childMovingTargetAnchorTabIndex=CreateTabChildren(childMovingTarget,0,childMovingRefTabIdx,childMoving);*/

			}
			else
			{
				/*if(childMovingTargetAnchorTabIndex>=0)
				{
					SendMessage(childMovingTarget,TCM_DELETEITEM,childMovingTargetAnchorTabIndex,0);
					childMovingTargetAnchorTabIndex=-1;
				}
				ShowWindow(childMoving,true);*/

				if(anchor==1)
					SetWindowPos(childMoving->hwnd,0,childMovingRc.left,childMovingRc.top,childMovingRc.right-childMovingRc.left,childMovingRc.bottom-childMovingRc.top,SWP_SHOWWINDOW);
				else
					SetWindowPos(childMoving->hwnd,0,p.x-(childMovingScaledWidthX)/2,p.y-(childMovingScaledWidthY)/2,childMovingScaledWidthX,childMovingScaledWidthY,SWP_SHOWWINDOW);
			}
		}
		else
		{
			SetWindowPos(childMoving->hwnd,0,p.x-(childMovingScaledWidthX)/2,p.y-(childMovingScaledWidthY)/2,childMovingScaledWidthX,childMovingScaledWidthY,SWP_SHOWWINDOW);
			childMovingTarget=0;
		}
	}
	else
	{
		if(!GetCapture())
		{
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



HWND SplitterContainer::GetWindowBelowMouse(HWND hwnd,WPARAM wparam,LPARAM lparam)
{
	POINTS mousePos=MAKEPOINTS(lparam);
	POINT p={mousePos.x,mousePos.y};
	return ChildWindowFromPointEx(hwnd,p,CWP_SKIPDISABLED|CWP_SKIPINVISIBLE|CWP_SKIPTRANSPARENT);
}

void SplitterContainer::OnSize(HWND hwnd,WPARAM wparam,LPARAM lparam)
{	
	/*std::vector<HWND> windows;

	HWND child=0;

	while(child=FindWindowEx(hwnd,child ? child : 0,0,0))
		windows.push_back(child);

	HDWP hdwp=BeginDeferWindowPos(tabContainerCount);

	switch(wparam)
	{
		case WMSZ_RIGHT:
			{
				std::vector<HWND> leftZero=findWindoswAtPos(hwnd,rc,3);

				for(int i=0;i<leftZero.size();i++)
				{
					RECT rc;
					GetClientRect(leftZero[i],&rc);
					SetWindowPos()
				}
				
			}
		break;
	}

	EndDeferWindowPos(hdwp);*/
}



void SplitterContainer::CreateFloatingTab(TabContainer* tab)
{
	if(childMovingRef)
		return;

	childMovingRef=tab;
	childMovingRefTabIdx=tab->selected;
	childMovingRefTabCount=(int)tab->tabs.size();

	RECT tmpRc;

	GetWindowRect(childMovingRef->hwnd,&tmpRc);

	childMoving=new TabContainer(tmpRc.left,tmpRc.top,tmpRc.right-tmpRc.left,tmpRc.bottom-tmpRc.top,GetParent(tab->hwnd));

	EnableWindow(childMoving->hwnd,false);

	printf("creating childmoving %p\n",childMoving);

}

void SplitterContainer::DestroyFloatingTab()
{
	if(childMovingRef)
	{
		EnableAndShowTabContainerChild(childMovingRef->hwnd,childMovingRefTabIdx);
		DestroyWindow(childMoving->hwnd);//remove floating window
		childMovingRef->mouseDown=false;
		childMovingRef=0;
		childMoving=0;

		printf("%p deleting childmoving\n",this);
	}
}

int SplitterContainer::OnTabContainerRButtonUp(HWND hwnd,LPARAM lparam)
{
	RECT rc;
	GetWindowRect(hwnd,&rc);
	return (int)TrackPopupMenu(popupMenuRoot,TPM_RETURNCMD |TPM_LEFTALIGN|TPM_TOPALIGN,rc.left+LOWORD(lparam),rc.top+HIWORD(lparam),0,GetParent(hwnd),0);
}

int SplitterContainer::GetTabSelectionIdx(HWND hwnd)
{
	return SendMessage(hwnd,TCM_GETCURSEL,0,0);
}

void SplitterContainer::SetTabSelectionIdx(HWND hwnd,int idx)
{
	SendMessage(hwnd,TCM_SETCURSEL,idx,0);
}

HWND SplitterContainer::GetTabSelectionWindow(HWND hwnd)
{
	return GetDlgItem(hwnd,GetTabSelectionIdx(hwnd));
}

void SplitterContainer::OnTabContainerSize(HWND hwnd)
{
	printf("OnTabSizeChanged %p\n",hwnd);

	int		tabIdx=-1;
	HWND	child=0;	

	tabIdx=0;//SendMessage(hwnd,TCM_GETCURSEL,0,0);


	if(tabIdx<0)
		return;

	child=GetDlgItem(hwnd,tabIdx);

	if(child==0)
		return;

	RECT size=GetTabContainerClientSize(hwnd);

	MoveWindow(child,size.left,size.top,size.right-size.left,size.bottom-size.top,true);
}

RECT SplitterContainer::GetTabContainerClientSize(HWND hwnd)
{
	RECT clientRect,itemRect,adj0Rect,adj1Rect,result;

	GetClientRect(hwnd,&clientRect);
	SendMessage(hwnd,TCM_GETITEMRECT,0,(LPARAM)&itemRect);

	adj0Rect=itemRect;
	adj1Rect=itemRect;

	SendMessage(hwnd,TCM_ADJUSTRECT,0,(LPARAM)&adj0Rect);
	SendMessage(hwnd,TCM_ADJUSTRECT,1,(LPARAM)&adj1Rect);

	result.top=clientRect.top+adj0Rect.top-itemRect.top;
	result.bottom=clientRect.bottom-(itemRect.bottom-adj0Rect.bottom);
	result.right=clientRect.right-(itemRect.right-adj0Rect.right);
	result.left=clientRect.left+adj0Rect.left-itemRect.left;

	/*result.top=clientRect.top+adj1Rect.top;
	result.bottom=clientRect.bottom-(adj1Rect.bottom-itemRect.bottom);
	result.right=clientRect.right-(adj1Rect.right-itemRect.right);
	result.left=clientRect.left+itemRect.left-adj1Rect.left;*/

	return result;
}

void SplitterContainer::EnableAndShowTabContainerChild(HWND hwnd,int idx)
{
	/*HWND child=GetDlgItem(hwnd,idx);

	if(!child)
	{
		__debugbreak();
		return;
	}

	EnableChilds(hwnd,false,false);

	SendMessage(hwnd,TCM_SETCURSEL,idx,0);
	EnableWindow(child,true);
	ShowWindow(child,SW_SHOW);

	/ *if(TabProc==(WNDPROC)GetClassLong(hwnd,GWL_WNDPROC))//is a tab container ?
		SetWindowLongPtr(hwnd,GWL_USERDATA,(LONG_PTR)idx);//set child window index in the tab container user data* /

	RECT tabRect=GetTabContainerClientSize(hwnd);

	SetWindowPos(child,HWND_TOP,tabRect.left,tabRect.top,tabRect.right-tabRect.left,tabRect.bottom-tabRect.top,SWP_SHOWWINDOW|SWP_ASYNCWINDOWPOS);*/
}


HWND SplitterContainer::CreateTabContainer(int x,int y,int w,int h,HWND parent)
{
	return CreateWindow(WC_TABCONTAINERWINDOWCLASS,WC_TABCONTAINERWINDOWCLASS,WS_CHILD|WS_VISIBLE,x,y,w,h,parent,(HMENU)tabContainerCount++,0,0);
}

int SplitterContainer::CreateTabChildren(HWND dst,char* text,int pos,HWND src)
{
	int tmpPos=-1;
	char tabLabel[CHAR_MAX];
	TCITEM t={0};
	t.mask=TCIF_TEXT;
	t.pszText=tabLabel;
	t.cchTextMax=CHAR_MAX;

	tmpPos=(pos<0 ? SendMessage(dst,TCM_GETITEMCOUNT,0,0) : pos);

	if(!src)
	{
		if(tmpPos<0)
			t.pszText=text;
		else
			sprintf_s(tabLabel,"Tab%d",tmpPos);
	}
	else
	{
		int tIdx=SendMessage(src,TCM_GETCURSEL,0,0);
		SendMessage(src,TCM_GETITEM,tIdx,(LPARAM)&t);
	}

	SendMessage(dst,TCM_INSERTITEM,tmpPos,(LPARAM)&t);

	return tmpPos;
}

void SplitterContainer::RemoveTabChildren(HWND hwnd,int idx)
{
	int itemCount=SendMessage(hwnd,TCM_GETITEMCOUNT,0,0);
	if(!SendMessage(hwnd,TCM_DELETEITEM,idx,0))//tabcontrol automatically scales remaining items
		__debugbreak();
	int removeCount=(itemCount-1)-idx;
	for(int i=0;i<removeCount;i++)
	{
		int srcIdx=idx+i+1;
		int dstIdx=srcIdx-1;

		HWND child=GetDlgItem(hwnd,srcIdx);

		if(!child)
			__debugbreak();

		SetWindowLong(child,GWL_ID,(LONG)dstIdx);
	}
}

void SplitterContainer::ReparentTabChildren(HWND src,HWND dst,int idx)
{
	int newTabContentIdx=CreateTabChildren(dst,0,0,src);
	HWND tabContentWindow=GetDlgItem(src,idx);
	SetParent(tabContentWindow,dst);
	SetWindowLong(tabContentWindow,GWL_ID,(LONG)newTabContentIdx);
	SendMessage(dst,WM_SIZE,0,0);
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

bool SplitterContainer::CreateNewPanel(HWND hwnd,int popupMenuItem)
{
	/*WindowData* newClassIsCreated=0;

	switch(popupMenuItem)
	{
		case TAB_MENU_COMMAND_OPENGLWINDOW:
		{
			OpenGLRenderer* or=new OpenGLRenderer();
			newClassIsCreated=or;
			OpenGLRenderer::interfacesPool.push_back(or);
			OpenGLRenderer::guiInterfacesPool.push_back(or);
		}
		break;
		case TAB_MENU_COMMAND_SHAREDOPENGLWINDOW:
		{
			OpenGLRenderer* renderer=(OpenGLRenderer*)GetWindowLongPtr(GetTabSelectionWindow(hwnd),GWL_USERDATA);
			if(renderer)
			{
				newClassIsCreated=renderer->CreateSharedContext(hwnd);
				OpenGLRenderer::interfacesPool.push_back(renderer);
				OpenGLRenderer::guiInterfacesPool.push_back(renderer);
			}
		}
		break;
		case TAB_MENU_COMMAND_PROJECTFOLDER2:
		{
			ProjectFolderBrowser2* cpf2=new ProjectFolderBrowser2();
			newClassIsCreated=cpf2;
			ProjectFolderBrowser2::guiInterfacesPool.push_back(cpf2);
		}
		break;
		case TAB_MENU_COMMAND_PROJECTFOLDER:
		{
			ProjectFolderBrowser* cpf2=new ProjectFolderBrowser();
			newClassIsCreated=cpf2;
			ProjectFolderBrowser::guiInterfacesPool.push_back(cpf2);
		}
		break;
		case TAB_MENU_COMMAND_LOGGER:
		{
			Logger* logger=new Logger();
			newClassIsCreated=logger;
			Logger::guiInterfacesPool.push_back(logger);
		}
		break;
		case TAB_MENU_COMMAND_SCENEENTITIES:
		{
			SceneEntities* se=new SceneEntities;
			newClassIsCreated=se;
			GuiInterface::guiInterfacesPool.push_back(se);
		}
		break;
		case TAB_MENU_COMMAND_ENTITYPROPERTIES:
		{
			EntityProperty* ep=new EntityProperty();
			newClassIsCreated=ep;
			EntityProperty::guiInterfacesPool.push_back(ep);
		}
		break;
	}

	if(newClassIsCreated)
	{
		newClassIsCreated->Create(hwnd);
		int tabIdx=this->CreateTabChildren(hwnd);
		SetWindowLong(newClassIsCreated->hwnd,GWL_ID,(LONG)tabIdx);
		this->EnableAndShowTabContainerChild(hwnd,tabIdx);

		return true;
	}*/

	return false;
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
		wc.hbrBackground=CreateSolidBrush(Gui::COLOR_GUI_BACKGROUND);

		if(!RegisterClassEx(&wc))
			__debugbreak();
	}

	{
		WNDCLASS wc={0};

		wc.style=CS_VREDRAW|CS_HREDRAW|CS_OWNDC;//CS_OWNDC have always the same dc xevery window
		wc.hCursor=LoadCursor(NULL, IDC_ARROW);
		wc.lpszClassName=WC_OPENGLWINDOW;
		wc.lpfnWndProc=OpenGLProc;
		wc.hbrBackground=0;

		if(!RegisterClass(&wc))
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



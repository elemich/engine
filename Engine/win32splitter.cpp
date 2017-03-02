#include "win32.h"

extern WNDPROC SystemOriginalTabControlProcedure;
extern WNDPROC SystemOriginalSysTreeView32ControlProcedure;






SplitterContainer::SplitterContainer():splitterSize(0)
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

		resizingWindows1=findWindoswAtPos(hwnd,rc1,edge1);
		resizingWindows2=findWindoswAtPos(hwnd,rc2,edge2);

		SetCapture(hwnd);
	}
}

void SplitterContainer::OnLButtonUp(HWND hwnd)
{
	if(childMovingRef)
	{
		EnableAllChildsDescendants(hwnd,true);

		if(childMoving)
		{
			if(childMovingTarget)
			{
				if(childMovingTargetAnchorPos!=-1)//
				{
					HWND newChild=CreateTabContainer(childMovingRc.left,childMovingRc.top,childMovingRc.right-childMovingRc.left,childMovingRc.bottom-childMovingRc.top,hwnd,tabContainerCount);
					ReparentTabChild(childMovingRef,newChild,childMovingRefTabIdx);

					if(SPLITTER_DEBUG)
						printf("creating new container %p\n",newChild);

					SetWindowPos(childMovingTarget,0,childMovingTargetRc.left,childMovingTargetRc.top,childMovingTargetRc.right-childMovingTargetRc.left,childMovingTargetRc.bottom-childMovingTargetRc.top,SWP_SHOWWINDOW);
					RemoveTab(childMovingRef,childMovingRefTabIdx);

					EnableAndShowContainerChild(childMovingRef,0);

					if(SPLITTER_DEBUG)
						printf("changing position to sibling %p\n",childMovingTarget);

					DestroyWindow(childMoving);//remove floating window

				}
				else
				{

				}
			}
			else
				DestroyWindow(childMoving);
		}

		childMovingRef=0;
		childMoving=0;
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

	//printf("moving parent %d,%d\n",p.x,p.y);

	if(childMovingRef)
	{
		RECT tmpRc;

		GetWindowRect(childMovingRef,&tmpRc);

		int scalesize=3;

		int childMovingScaledWidthX=(tmpRc.right-tmpRc.left)/scalesize;
		int childMovingScaledWidthY=(tmpRc.bottom-tmpRc.top)/scalesize;

		POINT cp={p.x,p.y};

		childMovingTarget=ChildWindowFromPointEx(hwnd,cp,CWP_SKIPDISABLED);

		if(childMovingTarget!=hwnd && childMovingTarget!=childMoving && (childMovingRefTabCount==1 ? childMovingTarget!=childMovingRef : true))
		{
			SendMessage(childMovingTarget,TCM_GETITEMRECT,0,(LPARAM)&tmpRc);

			int hh=tmpRc.bottom-tmpRc.top;//header height

			GetClientRect(childMovingTarget,&tmpRc);
			MapWindowRect(childMovingTarget,hwnd,&tmpRc);

			int w=tmpRc.right-tmpRc.left;
			int h=tmpRc.bottom-tmpRc.top;

			int wd=w/3;
			int hd=h/3;

			childMovingTargetRc=childMovingRc=tmpRc;
			int anchor=false;//0 not anchor,1 anchor client,2 anchor header

			if(cp.y>tmpRc.top && cp.y<tmpRc.top+hh)
			{
				childMovingTargetAnchorPos=-1;
				anchor=2;
			}
			else if(cp.x>tmpRc.left && cp.x<tmpRc.left+wd)
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
				ShowWindow(childMoving,false);
				if(childMovingTargetAnchorTabIndex<0)
					childMovingTargetAnchorTabIndex=CreateTab(childMovingTarget,0,childMovingRefTabIdx,childMoving);

			}
			else
			{
				if(childMovingTargetAnchorTabIndex>=0)
				{
					SendMessage(childMovingTarget,TCM_DELETEITEM,childMovingTargetAnchorTabIndex,0);
					childMovingTargetAnchorTabIndex=-1;
				}
				ShowWindow(childMoving,true);

				if(anchor==1)
					SetWindowPos(childMoving,0,childMovingRc.left,childMovingRc.top,childMovingRc.right-childMovingRc.left,childMovingRc.bottom-childMovingRc.top,SWP_SHOWWINDOW);
				else
					SetWindowPos(childMoving,0,p.x-(childMovingScaledWidthX)/2,p.y-(childMovingScaledWidthY)/2,childMovingScaledWidthX,childMovingScaledWidthY,SWP_SHOWWINDOW);
			}
		}
		else
		{
			SetWindowPos(childMoving,0,p.x-(childMovingScaledWidthX)/2,p.y-(childMovingScaledWidthY)/2,childMovingScaledWidthX,childMovingScaledWidthY,SWP_SHOWWINDOW);
			childMovingTarget=0;
		}
	}
	else
	{
		if(!GetCapture())
		{
			int d=6;

			POINT probePoints[4]={{p.x-d,p.y},{p.x+d,p.y},{p.x,p.y-d},{p.x,p.y+d}};

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


#pragma message (LOCATION " @mic: should SplitterContainer::OnMouseWheel call directly the child OnMouseWheel method?")


void SplitterContainer::OnMouseWheel(HWND hwnd,WPARAM wparam,LPARAM lparam)
{
	POINTS p=MAKEPOINTS(lparam);
	POINT ps={p.x,p.y};
	HWND onChild=ChildWindowFromPointEx(hwnd,ps,CWP_SKIPDISABLED);

	if(onChild!=hwnd)
		SendMessage(onChild,WM_MOUSEWHEEL,wparam,lparam);
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



void SplitterContainer::OnTabContainerLButtonDown(HWND hwnd)
{
	EnableAndShowContainerChild(hwnd,SendMessage(hwnd,TCM_GETCURSEL,0,0));

	if(childMovingRef)
		return;

	childMovingRef=hwnd;
	childMovingRefTabIdx=SendMessage(hwnd,TCM_GETCURSEL,0,0);
	childMovingRefTabCount=SendMessage(hwnd,TCM_GETITEMCOUNT,0,0);

	RECT tmpRc;

	GetWindowRect(childMovingRef,&tmpRc);

	childMoving=CreateWindow(WC_TABCONTAINER,"TmpFloatingTab",WS_CHILD|WS_VISIBLE,tmpRc.left,tmpRc.top,tmpRc.right-tmpRc.left,tmpRc.bottom-tmpRc.top,GetParent(hwnd),0,0,0);
	CreateTab(childMoving,0,0,childMovingRef);

	EnableWindow(childMoving,false);

	EnableAllChildsDescendants(GetParent(hwnd),false);

}

void SplitterContainer::OnTabContainerLButtonUp(HWND hwnd)
{
	if(childMovingRef)
	{
		EnableAndShowContainerChild(childMovingRef,childMovingRefTabIdx);
		DestroyWindow(childMoving);//remove floating window
		childMovingRef=0;
	}
}

int SplitterContainer::OnTabContainerRButtonUp(HWND hwnd,LPARAM lparam)
{
	RECT rc;
	GetWindowRect(hwnd,&rc);
	return (int)TrackPopupMenu(popupMenuRoot,TPM_RETURNCMD |TPM_LEFTALIGN|TPM_TOPALIGN,rc.left+LOWORD(lparam),rc.top+HIWORD(lparam),0,GetParent(hwnd),0);
}


bool InitSplitter()
{
	bool returnValue=true;

	WNDCLASSEX wc={0};
	wc.cbSize=sizeof(WNDCLASSEX);
	wc.hCursor=LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName=WC_MAINAPPWINDOW;
	wc.lpfnWndProc=MainWindowProc;
	wc.hbrBackground=CreateSolidBrush(RGB(255,0,0));
	wc.hbrBackground=0;
	if(!RegisterClassEx(&wc))
		returnValue=false;

	wc.style=CS_VREDRAW|CS_HREDRAW|CS_PARENTDC;
	wc.hCursor=LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName=WC_OPENGLWINDOW;
	wc.lpfnWndProc=OpenGLProc;
	wc.hbrBackground=0;
	if(!RegisterClassEx(&wc))
		returnValue=false;

	if(GetClassInfoEx(0,WC_TABCONTROL,&wc))
	{
		wc.hCursor=LoadCursor(NULL, IDC_ARROW);
		wc.style=CS_VREDRAW|CS_HREDRAW|CS_PARENTDC;
		wc.lpszClassName=WC_TABCONTAINER;
		SystemOriginalTabControlProcedure=wc.lpfnWndProc;
		wc.lpfnWndProc=TabProc;
		//wc.hbrBackground=0;
		if(!RegisterClassEx(&wc))
			returnValue=false;
	}
	else 
		returnValue=false;

	if(GetClassInfoEx(0,"SysTreeView32",&wc))
	{
		//wc.style=CS_VREDRAW|CS_HREDRAW|CS_PARENTDC;
		wc.lpszClassName=WC_SCENEENTITIESWINDOW;
		SystemOriginalSysTreeView32ControlProcedure=wc.lpfnWndProc;
		wc.lpfnWndProc=SceneEntitiesProc;
		//wc.hbrBackground=0;
		if(!RegisterClassEx(&wc))
			returnValue=false;
	}
	else 
		returnValue=false;

	
	//scintilla text control
	/*{
		#define WC_CODEWINDOW	"CodeWindow"

		LRESULT CALLBACK CodeWindowProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);
		WNDPROC ScintillaOriginalWindowProc;

		HMODULE hmod = LoadLibrary("SciLexer.DLL");
	
		if (hmod==NULL)
			MessageBox(0,"The Scintilla DLL could not be loaded.","Error loading Scintilla",MB_OK | MB_ICONERROR);

		if(GetClassInfoEx(0,"Scintilla",&wc))
		{
			//wc.hCursor=LoadCursor(NULL, IDC_ARROW);
			wc.style=CS_VREDRAW|CS_HREDRAW|CS_PARENTDC;
			wc.lpszClassName=WC_CODEWINDOW;
			ScintillaOriginalWindowProc=wc.lpfnWndProc;
			wc.lpfnWndProc=CodeWindowProc;
			//wc.hbrBackground=0;
			if(!RegisterClassEx(&wc))
				returnValue=false;
		}
		else 
			returnValue=false;

		HWND hwndScintilla = CreateWindowEx(0,"Scintilla","Scintilla", WS_OVERLAPPEDWINDOW|WS_VISIBLE | WS_TABSTOP | WS_CLIPCHILDREN,10,10,500,400,0,0,0,0);
	}*/

	HMENU& popupMenuRoot=SplitterContainer::popupMenuRoot;
	HMENU& popupMenuCreate=SplitterContainer::popupMenuCreate;

	InsertMenu(popupMenuRoot,0,MF_BYPOSITION|MF_POPUP,(UINT_PTR)popupMenuCreate,"New Tab");
	{
		InsertMenu(popupMenuCreate,0,MF_BYPOSITION|MF_STRING,TAB_MENU_COMMAND_OPENGLWINDOW,"OpenGLFixedRenderer");
		InsertMenu(popupMenuCreate,1,MF_BYPOSITION|MF_STRING,TAB_MENU_COMMAND_PROJECTFOLDER2,"Project Folder2");
		InsertMenu(popupMenuCreate,4,MF_BYPOSITION|MF_STRING,TAB_MENU_COMMAND_PROJECTFOLDER,"Project Folder");
		InsertMenu(popupMenuCreate,2,MF_BYPOSITION|MF_STRING,TAB_MENU_COMMAND_LOGGER,"Logger");
		InsertMenu(popupMenuCreate,3,MF_BYPOSITION|MF_STRING,TAB_MENU_COMMAND_SCENEENTITIES,"Scene Entities");
	}
	InsertMenu(popupMenuRoot,1,MF_BYPOSITION|MF_SEPARATOR,0,0);
	InsertMenu(popupMenuRoot,2,MF_BYPOSITION|MF_STRING,TAB_MENU_COMMAND_REMOVE,"Remove Tab");

	return returnValue;
}



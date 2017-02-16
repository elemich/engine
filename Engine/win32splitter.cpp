#include "win32.h"

extern WNDPROC SystemOriginalTabControlProcedure;

SplitterContainer::SplitterContainer():splitterSize(0)
{
	MainNode=0;
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
	SAFEDELETE(MainNode);
}

HMENU SplitterContainer::popupMenuRoot=CreatePopupMenu();
HMENU SplitterContainer::popupMenuCreate=CreatePopupMenu();




void SplitterContainer::OnLButtonDown(HWND hwnd,LPARAM lparam)
{
	if(!GetCapture())
	{
		splitterPreviousPos=MAKEPOINTS(lparam);

		SetCapture(hwnd);
		SetCursor(LoadCursor(0,splitterCursor));

		vwin1.clear();
		vwin2.clear();

		WINDOWNODE* node=MainNode->Find(win1),*t=node;

		vwin1.push_back(win1);

		int dir1=(splitterCursor==IDC_SIZEWE ? 1 : 0);//top - left
		int dir2=(splitterCursor==IDC_SIZEWE ? 3 : 2);//bottom - right

		while(t && t->links[dir1])
		{
			vwin1.push_back(t->links[dir1]->data);
			t=t->links[dir1];
		}

		t=node;

		while(t && t->links[dir2])
		{
			vwin1.push_back(t->links[dir2]->data);
			t=t->links[dir2];
		}

		//

		node=MainNode->Find(win2),t=node;

		vwin2.push_back(win2);

		while(t && t->links[dir1])
		{
			vwin2.push_back(t->links[dir1]->data);
			t=t->links[dir1];
		}

		t=node;

		while(t && t->links[dir2])
		{
			vwin2.push_back(t->links[dir2]->data);
			t=t->links[dir2];
		}
	}
}

void SplitterContainer::OnLButtonUp(HWND hwnd)
{
	if(GetCapture())
	{
		ReleaseCapture();
		SetCursor(LoadCursor(0,IDC_ARROW));

		if(SPLITTER_DEBUG)
			printf("unset capture\n");
	}

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

					WINDOWNODE* nodeFound=MainNode->Find(childMovingTarget);

					if(nodeFound)
					{
						WINDOWNODE* node=new WINDOWNODE(newChild,0,0,0,0);
						nodeFound->LinkWith(node,childMovingTargetAnchorPos);

						WINDOWNODE* checkNodeInsert=MainNode->Find(newChild);
					}
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
}

void SplitterContainer::OnMouseMove(HWND hwnd,LPARAM lparam)
{
	POINTS p=MAKEPOINTS(lparam);

	if(childMovingRef)
	{
		RECT tmpRc;

		GetWindowRect(childMovingRef,&tmpRc);

		int scalesize=3;

		int childMovingScaledWidthX=(tmpRc.right-tmpRc.left)/scalesize;
		int childMovingScaledWidthY=(tmpRc.bottom-tmpRc.top)/scalesize;


		if(!childMoving)
		{
			childMoving=CreateWindow(WC_TABCONTAINER,"TmpFloatingTab",WS_CHILD|WS_VISIBLE,tmpRc.left,tmpRc.top,tmpRc.right-tmpRc.left,tmpRc.bottom-tmpRc.top,hwnd,0,0,0);
			CreateTab(childMoving,0,0,childMovingRef);

			EnableWindow(childMoving,false);

			EnableAllChildsDescendants(hwnd,false);
		}
		else
		{
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

				//printf("%d,%d , %d,%d,%d,%d , %d,%d,%d,%d , %d , %s\n",cp.x,cp.y,tmpRc.left,tmpRc.top,tmpRc.right,tmpRc.bottom,childMovingRc.left,childMovingRc.top,childMovingRc.right,childMovingRc.bottom,childMovingTargetPos,n);				
				//EnableWindow(childMovingTarget,true);

			}
			else
			{
				SetWindowPos(childMoving,0,p.x-(childMovingScaledWidthX)/2,p.y-(childMovingScaledWidthY)/2,childMovingScaledWidthX,childMovingScaledWidthY,SWP_SHOWWINDOW);
				childMovingTarget=0;
			}
		}
	}
	else
	{
		if(!GetCapture())
		{
			int d=6;

			POINT pp[4]={{p.x-d,p.y},{p.x+d,p.y},{p.x,p.y-d},{p.x,p.y+d}};

			HWND resh[4]={0,0,0,0};

			for(int i=0;i<4;i++)
			{
				HWND found=ChildWindowFromPoint(hwnd,pp[i]);
				if(found!=0 && hwnd!=found)
					resh[i]=found;
			}

			if(resh[0]!=resh[1] && resh[0] && resh[1])
			{
				SetCursor(LoadCursor(0,splitterCursor=IDC_SIZEWE));
				win1=resh[0];
				win2=resh[1];
			}
			else if(resh[2]!=resh[3] && resh[2] && resh[3])
			{
				SetCursor(LoadCursor(0,splitterCursor=IDC_SIZENS));
				win1=resh[2];
				win2=resh[3];
			}
			else
				SetCursor(LoadCursor(0,splitterCursor=IDC_ARROW));
		}
		else
		{
			POINTS delta={splitterPreviousPos.x-p.x,splitterPreviousPos.y-p.y};

			if(delta.x==0 && delta.y==0)
				return;

			//printf("%d,%d+%d\n",vwin1.size()+vwin2.size(),vwin1.size(),vwin2.size());

			int numwindows=vwin1.size()+vwin2.size();

			HDWP hdwp=BeginDeferWindowPos(numwindows);

			RECT rc;

			DWORD flags1=SWP_NOMOVE|SWP_SHOWWINDOW;
			DWORD flags2=SWP_SHOWWINDOW;
			for(int i=0;i<(int)vwin1.size();i++)
			{
				GetWindowRect(vwin1[i],&rc);
				MapWindowRect(HWND_DESKTOP,hwnd,&rc);

				if(splitterCursor==IDC_SIZEWE)
					DeferWindowPos(hdwp,vwin1[i],0,0,0,(rc.right-rc.left)-delta.x,rc.bottom-rc.top,flags1);
				else 
					DeferWindowPos(hdwp,vwin1[i],0,0,0,rc.right-rc.left,(rc.bottom-rc.top)-delta.y,flags1);
			}

			for(int i=0;i<(int)vwin2.size();i++)
			{
				GetWindowRect(vwin2[i],&rc);
				MapWindowRect(HWND_DESKTOP,hwnd,&rc);

				if(splitterCursor==IDC_SIZEWE)
					DeferWindowPos(hdwp,vwin2[i],0,rc.left-delta.x,rc.top,(rc.right-rc.left)+delta.x,rc.bottom-rc.top,flags2);
				else
					DeferWindowPos(hdwp,vwin2[i],0,rc.left,rc.top-delta.y,rc.right-rc.left,(rc.bottom-rc.top)+delta.y,flags2);
			}

			EndDeferWindowPos(hdwp);
		}

	}

	splitterPreviousPos=p;
}


void SplitterContainer::OnTabContainerLButtonDown(HWND hwnd)
{
	EnableAndShowContainerChild(hwnd,SendMessage(hwnd,TCM_GETCURSEL,0,0));

	if(childMovingRef)
		return;

	childMovingRef=hwnd;
	childMovingRefTabIdx=SendMessage(hwnd,TCM_GETCURSEL,0,0);
	childMovingRefTabCount=SendMessage(hwnd,TCM_GETITEMCOUNT,0,0);
}

void SplitterContainer::OnTabContainerLButtonUp(HWND hwnd)
{
	if(childMovingRef)
		childMovingRef=0;
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

	HMENU& popupMenuRoot=SplitterContainer::popupMenuRoot;
	HMENU& popupMenuCreate=SplitterContainer::popupMenuCreate;

	InsertMenu(popupMenuRoot,0,MF_BYPOSITION|MF_POPUP,(UINT_PTR)popupMenuCreate,"New Tab");
	{
		InsertMenu(popupMenuCreate,0,MF_BYPOSITION|MF_STRING,1,"OpenGLFixedRenderer");
		InsertMenu(popupMenuCreate,1,MF_BYPOSITION|MF_STRING,2,"Project Folder");
		InsertMenu(popupMenuCreate,2,MF_BYPOSITION|MF_STRING,3,"Logger");
	}
	InsertMenu(popupMenuRoot,1,MF_BYPOSITION|MF_SEPARATOR,0,0);
	InsertMenu(popupMenuRoot,2,MF_BYPOSITION|MF_STRING,4,"Remove Tab");

	return returnValue;
}



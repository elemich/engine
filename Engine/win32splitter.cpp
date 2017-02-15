//#include "win32splitter.h"

#include "win32.h"

extern App* ___app;


typedef FourLinkNode<HWND> FLNODE;

WNDPROC TabControlProcedure;

HWND mainAppContainerWindow=0;

FLNODE* MainNode=0;


HWND childMovingRef=0;
HWND childMoving=0;
int  childTabIdx;
int  childTabCount;
TCITEM childMovingItem={0};

#pragma message ("@mic change childMovingItem with a bool\n")

HWND  childMovingSibling=0;
int   childMovingSiblingPos=0;
RECT  childMovingRc;
RECT  childMovingSiblingRc;
const int   splitterSize=0;
LPCSTR splitterCursor;
int split1;
int split2;
int split3;
POINTS splitterPreviousPos;
std::vector<HWND> vwin1;
std::vector<HWND> vwin2;
HWND win1;
HWND win2;
HMENU popupMenuRoot=CreatePopupMenu();
HMENU popupMenuCreate=CreatePopupMenu();

int tabContainerCount=0;

HWND CreateTabContainer(int x,int y,int w,int h)
{
	char tabContainerName[CHAR_MAX];
	sprintf_s(tabContainerName,"TabContainer%d",tabContainerCount);
	HWND tabWindow= CreateWindow(WC_TABCONTAINER,tabContainerName,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,x,y,w,h,mainAppContainerWindow,(HMENU)tabContainerCount,0,0);

	if(tabWindow)
		SetWindowLongPtr(tabWindow,GWL_USERDATA,(LONG)MAKEWORD(0,0));

	return tabWindow;
}

int CreateTab(HWND dst,char* text,int pos,HWND src)
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

void RemoveTab(HWND hwnd,int idx)
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

void ReparentTabChild(HWND src,HWND dst,int idx)
{
	int newTabContentIdx=CreateTab(dst,0,0,src);
	HWND tabContentWindow=GetDlgItem(src,idx);
	SetParent(tabContentWindow,dst);
	SetWindowLong(tabContentWindow,GWL_ID,(LONG)newTabContentIdx);
	SendMessage(dst,WM_SIZE,0,0);
}

RECT GetTabClientSize(HWND hwnd)
{
	RECT clientRect,itemRect,adj0Rect,result;

	GetClientRect(hwnd,&clientRect);
	SendMessage(hwnd,TCM_GETITEMRECT,0,(LPARAM)&itemRect);

	adj0Rect=itemRect;

	SendMessage(hwnd,TCM_ADJUSTRECT,0,(LPARAM)&adj0Rect);

	result.top=clientRect.top+adj0Rect.top-itemRect.top;
	result.bottom=clientRect.bottom-(itemRect.bottom-adj0Rect.bottom);
	result.right=clientRect.right-(itemRect.right-adj0Rect.right);
	result.left=clientRect.left+adj0Rect.left-itemRect.left;

	return result;
}



void OnTabSizeChanged(HWND hwnd)
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

	RECT size=GetTabClientSize(hwnd);

	MoveWindow(child,size.left,size.top,size.right-size.left,size.bottom-size.top,true);

}

void EnableChilds(HWND hwnd,int enable=-1,int show=-1)
{
	HWND child=0;
	while(child=FindWindowEx(hwnd,child ? child : 0,0,0))
	{
		if(enable>=0)EnableWindow(child,enable);
		if(show>=0)ShowWindow(child,show);
	}
}

void EnableAllChildsDescendants(HWND hwnd,int enable=-1,int show=-1)
{
	HWND child=0;
	while(child=FindWindowEx(hwnd,child ? child : 0,0,0))
		EnableChilds(child,enable,show);
}

void EnableAndShowContainerChild(HWND hwnd,int idx)
{
	HWND child=GetDlgItem(hwnd,idx);

	if(!child)
	{
		__debugbreak();
		return;
	}

	EnableChilds(hwnd,false,false);

	SendMessage(hwnd,TCM_SETCURSEL,idx,0);
	EnableWindow(child,true);
	ShowWindow(child,SW_SHOW);

	RECT tabRect=GetTabClientSize(hwnd);

	SetWindowPos(child,HWND_TOP,tabRect.left,tabRect.top,tabRect.right-tabRect.left,tabRect.bottom-tabRect.top,SWP_SHOWWINDOW|SWP_ASYNCWINDOWPOS);
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



LRESULT CALLBACK MainWindowProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	LRESULT result=0;

	switch(msg)
	{
		case WM_CREATE:
			{
				result=DefWindowProc(hwnd,msg,wparam,lparam);

				mainAppContainerWindow=hwnd;

				RECT rc;
				GetClientRect(hwnd,&rc);
				HWND firstChild=CreateTabContainer(0,0,rc.right-rc.left,rc.bottom-rc.top);
				CreateProjectFolder(firstChild);
				MainNode=new FLNODE(firstChild,0,0,0,0);

				InsertMenu(popupMenuRoot,0,MF_BYPOSITION|MF_POPUP,(UINT_PTR)popupMenuCreate,"New Tab");
				{
					InsertMenu(popupMenuCreate,0,MF_BYPOSITION|MF_STRING,0,"Renderer");
					InsertMenu(popupMenuCreate,1,MF_BYPOSITION|MF_STRING,1,"Project Folder");
				}
				InsertMenu(popupMenuRoot,1,MF_BYPOSITION|MF_SEPARATOR,0,0);
				InsertMenu(popupMenuRoot,2,MF_BYPOSITION|MF_STRING,2,"Remove Tab");
			}
			break;
		case WM_ERASEBKGND:
			return (LRESULT)1;
		case WM_LBUTTONDOWN:
		{
			result=DefWindowProc(hwnd,msg,wparam,lparam);

			if(!GetCapture())
			{
				splitterPreviousPos=MAKEPOINTS(lparam);

				SetCapture(hwnd);
				SetCursor(LoadCursor(0,splitterCursor));

				vwin1.clear();
				vwin2.clear();

				FLNODE* node=MainNode->Find(win1),*t=node;

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
		break;
		
		case WM_LBUTTONUP:
		{
			result=DefWindowProc(hwnd,msg,wparam,lparam);

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
					if(childMovingSibling)
					{
						if(childMovingSiblingPos!=-1)//
						{
							HWND newChild=CreateTabContainer(childMovingRc.left,childMovingRc.top,childMovingRc.right-childMovingRc.left,childMovingRc.bottom-childMovingRc.top);
							ReparentTabChild(childMovingRef,newChild,childTabIdx);
								
							if(SPLITTER_DEBUG)
								printf("creating new container %p\n",newChild);
									
							SetWindowPos(childMovingSibling,0,childMovingSiblingRc.left,childMovingSiblingRc.top,childMovingSiblingRc.right-childMovingSiblingRc.left,childMovingSiblingRc.bottom-childMovingSiblingRc.top,SWP_SHOWWINDOW);
							RemoveTab(childMovingRef,childTabIdx);

							EnableAndShowContainerChild(childMovingRef,0);

							if(SPLITTER_DEBUG)
								printf("changing position to sibling %p\n",childMovingSibling);

							DestroyWindow(childMoving);//remove floating window

							FLNODE* nodeFound=MainNode->Find(childMovingSibling);

							if(nodeFound)
							{
								FLNODE* node=new FLNODE(newChild,0,0,0,0);
								nodeFound->LinkWith(node,childMovingSiblingPos);

								FLNODE* checkNodeInsert=MainNode->Find(newChild);
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
		break;
		case WM_MOUSEMOVE:
			{
				result=DefWindowProc(hwnd,msg,wparam,lparam);

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

					POINT cp={p.x,p.y};

					childMovingSibling=ChildWindowFromPointEx(hwnd,cp,CWP_SKIPDISABLED);

					if(childMovingSibling!=hwnd && childMovingSibling!=childMoving && (childTabCount==1 ? childMovingSibling!=childMovingRef : true))
					{
						int otherChildTabCount=(childMovingSibling!=childMovingRef ? SendMessage(childMovingSibling,TCM_GETITEMCOUNT,0,0) : childTabCount);

						SendMessage(childMovingSibling,TCM_GETITEMRECT,0,(LPARAM)&tmpRc);

						int hh=tmpRc.bottom-tmpRc.top;//header height

						GetClientRect(childMovingSibling,&tmpRc);
						MapWindowRect(childMovingSibling,hwnd,&tmpRc);

						int w=tmpRc.right-tmpRc.left;
						int h=tmpRc.bottom-tmpRc.top;
						
						int wd=w/3;
						int hd=h/3;

						childMovingSiblingRc=childMovingRc=tmpRc;
						int anchor=false;//0 not anchor,1 anchor client,2 anchor header

						if(cp.y>tmpRc.top && cp.y<tmpRc.top+hh)
						{
							childMovingSiblingPos=-1;
							anchor=2;
						}
						else if(cp.x>tmpRc.left && cp.x<tmpRc.left+wd)
						{
							childMovingSiblingPos=0;
							childMovingRc.right=tmpRc.left+wd;
							childMovingSiblingRc.left=childMovingRc.right+splitterSize;
							anchor=1;
						}
						else if(cp.y>tmpRc.top+hh && cp.y<tmpRc.top+hd)
						{
							childMovingSiblingPos=1;
							childMovingRc=tmpRc;
							childMovingRc.bottom=tmpRc.top+hd;
							childMovingSiblingRc.top=childMovingRc.bottom+splitterSize;
							anchor=1;
						}
						else if(cp.x>tmpRc.right-wd && cp.x<tmpRc.right)
						{
							childMovingSiblingPos=2;
							childMovingRc=tmpRc;
							childMovingRc.left=tmpRc.right-wd;
							childMovingSiblingRc.right=childMovingRc.left-splitterSize;
							anchor=1;
						}
						else if(cp.y>tmpRc.bottom-hd && cp.y<tmpRc.bottom)
						{
							childMovingSiblingPos=3;
							childMovingRc=tmpRc;
							childMovingRc.top=tmpRc.bottom-hd;
							childMovingSiblingRc.bottom=childMovingRc.top-splitterSize;
							anchor=1;
						}
						
						
						if(anchor==2)
						{
							ShowWindow(childMoving,false);
							if(childMovingItem.mask==0)
							{
								childMovingItem.mask=TCIF_TEXT;
								childMovingItem.pszText="new";
								SendMessage(childMovingSibling,TCM_INSERTITEM,otherChildTabCount,(LPARAM)&childMovingItem);
							}
						}
						else
						{
							if(childMovingItem.mask!=0)
							{
								SendMessage(childMovingSibling,TCM_DELETEITEM,otherChildTabCount,0);
								memset(&childMovingItem,0,sizeof(TCITEM));
							}
							ShowWindow(childMoving,true);

							if(anchor==1)
								SetWindowPos(childMoving,0,childMovingRc.left,childMovingRc.top,childMovingRc.right-childMovingRc.left,childMovingRc.bottom-childMovingRc.top,SWP_SHOWWINDOW);
							else
								SetWindowPos(childMoving,0,p.x-(childMovingScaledWidthX)/2,p.y-(childMovingScaledWidthY)/2,childMovingScaledWidthX,childMovingScaledWidthY,SWP_SHOWWINDOW);
						}

						//printf("%d,%d , %d,%d,%d,%d , %d,%d,%d,%d , %d , %s\n",cp.x,cp.y,tmpRc.left,tmpRc.top,tmpRc.right,tmpRc.bottom,childMovingRc.left,childMovingRc.top,childMovingRc.right,childMovingRc.bottom,childMovingSiblingPos,n);				
						//EnableWindow(childMovingSibling,true);
						
					}
					else
					{
						SetWindowPos(childMoving,0,p.x-(childMovingScaledWidthX)/2,p.y-(childMovingScaledWidthY)/2,childMovingScaledWidthX,childMovingScaledWidthY,SWP_SHOWWINDOW);
						childMovingSibling=0;
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
							 return 0;

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
			break;
		
			default:
				result=DefWindowProc(hwnd,msg,wparam,lparam);
		
	}

	return result;
}


LRESULT CALLBACK TabProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	LRESULT result=0;

	switch(msg)
	{
		case WM_LBUTTONDOWN:
		{
			result=CallWindowProc(TabControlProcedure,hwnd,msg,wparam,lparam);

			EnableAndShowContainerChild(hwnd,SendMessage(hwnd,TCM_GETCURSEL,0,0));

			if(childMovingRef)
				break;

			childMovingRef=hwnd;
			childTabIdx=SendMessage(hwnd,TCM_GETCURSEL,0,0);
			childTabCount=SendMessage(hwnd,TCM_GETITEMCOUNT,0,0);
		}
		break;
		case WM_LBUTTONUP:
			{
				result=CallWindowProc(TabControlProcedure,hwnd,msg,wparam,lparam);

				if(childMovingRef)
					childMovingRef=0;
				
					
			}
			break;
		case WM_SIZE:
		case WM_WINDOWPOSCHANGED:
			{
				printf("sizing %s\n",msg==WM_SIZE ? "WM_SIZE" : "WM_WINDOWPOSCHANGED");

				//result=CallWindowProc(TabControlProcedure,hwnd,msg,wparam,lparam);

				OnTabSizeChanged(hwnd);
			}
			break;
		case WM_RBUTTONUP:
			{
				result=CallWindowProc(TabControlProcedure,hwnd,msg,wparam,lparam);

				RECT rc;
				GetWindowRect(hwnd,&rc);
				switch(TrackPopupMenu(popupMenuRoot,TPM_RETURNCMD |TPM_LEFTALIGN|TPM_TOPALIGN,rc.left+LOWORD(lparam),rc.top+HIWORD(lparam),0,GetParent(hwnd),0))
				{
					
				case 0://renderer
						CreateOpenglWindow(hwnd);
					break;
				case 1://proj folder
						CreateProjectFolder(hwnd);
					break;
				case 2://remove
					CreateTab(hwnd);
					break;
				}

				
			}
			break;
		default:
			result=CallWindowProc(TabControlProcedure,hwnd,msg,wparam,lparam);
		
	}
	return result;
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
		TabControlProcedure=wc.lpfnWndProc;
		wc.lpfnWndProc=TabProc;
		//wc.hbrBackground=0;
		if(!RegisterClassEx(&wc))
			returnValue=false;
	}
	else 
		returnValue=false;

	return returnValue;
}
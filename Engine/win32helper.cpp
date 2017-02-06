#include "win32helper.h"


#define TABCONTROL_HAS_ACTIVITY (WM_USER + 0x0001)

IntSize WHGetWindowSize(HWND hwnd)
{
	RECT r;
	GetWindowRect(hwnd,&r);
	return IntSize(r.right-r.left,r.bottom-r.top);
}

IntSize WHGetClientSize(HWND hwnd)
{
	RECT r;
	GetClientRect(hwnd,&r);
	return IntSize(r.right-r.left,r.bottom-r.top);
}

void WHSetWindowSize(HWND hwnd,int x,int y)
{
	if(!SetWindowPos(hwnd,0,0,0,x,y,SWP_NOMOVE))
		MessageBox(0,"could not resize window!","resize error",MB_ICONEXCLAMATION|MB_OK);
}

void WHSetWindowDataAndProcedure(HWND hwnd,void* inputData,WNDPROC inputProc)
{
	if(inputProc!=0)
		SetWindowLongPtr(hwnd,GWL_WNDPROC,(LONG)inputProc);
	if(inputData!=0)
		SetWindowLongPtr(hwnd,GWL_USERDATA,(LONG)inputData);
}

void WHMapWindowRect(HWND from,HWND to,RECT* rc){MapWindowPoints(from, to, (LPPOINT)rc, 2);}

RECT WHGetClientRect(HWND hwnd)
{
	char className[CHAR_MAX];
	GetClassName(hwnd,className,CHAR_MAX);

	RECT returnRect={0};

	if(0==strcmp(className,WC_TABCONTROL))
	{
		RECT clientRect,itemRect,adj0Rect;

		GetClientRect(hwnd,&clientRect);
		SendMessage(hwnd,TCM_GETITEMRECT,0,(LPARAM)&itemRect);

		adj0Rect=itemRect;

		SendMessage(hwnd,TCM_ADJUSTRECT,0,(LPARAM)&adj0Rect);

		returnRect.left=clientRect.left+adj0Rect.left-itemRect.left;
		returnRect.top=clientRect.top+adj0Rect.top-itemRect.top;
		returnRect.right=clientRect.right-(itemRect.right-adj0Rect.right);
		returnRect.bottom=clientRect.bottom-(itemRect.bottom-adj0Rect.bottom);
	}
	else
	{
		GetClientRect(hwnd,&returnRect);
	}


	
	return returnRect;
}


TVector<HWND> WHEnumFirstLevelWindowChilds(HWND parent)
{
	TVector<HWND> childWindows;

	HWND tmpChild=0;

	while(tmpChild=FindWindowEx(parent,tmpChild ? tmpChild : 0,0,0))
		childWindows+=tmpChild;

	return childWindows;
}
void WHOrganizeWindowContent(HWND hwnd)
{
	TVector<HWND> childs=WHEnumFirstLevelWindowChilds(hwnd);
	IntSize wSize=WHGetClientSize(hwnd);
	int nSplitters=childs.size-1;
	int splitterSize=5;
	int w=(wSize.x-(splitterSize*nSplitters))/childs.size;

	for(int i=0;i<childs.size;i++)
	{
		int x=i*w+i*splitterSize;
		
		SetWindowPos(childs[i],0,x,0,w,wSize.y,SWP_SHOWWINDOW);
	}
}


WHMainWindow::WHMainWindow()
{

}


WHMovingChild::WHMovingChild()
{
	hwnd=0;
}

void WHMovingChild::OnChildMouseButtonDown(HWND child,LPARAM lparam)
{
	hwnd=child;
	point=MAKEPOINTS(lparam);
	POINT p={point.x,point.y};
	MapWindowPoints(hwnd,HWND_DESKTOP,&p,1);
	point.x=p.x;
	point.y=p.y;
}
void WHMovingChild::OnChildMouseButtonUp()
{
	hwnd=0;
}

void WHMovingChild::OnMainWindowProcedure(HWND parent)
{
	if(hwnd)
	{
		//move the child window
		{
			POINT cp;
			GetCursorPos(&cp);
			ScreenToClient(GetParent(hwnd)==parent ? parent : HWND_DESKTOP,&cp);
			SetWindowPos(hwnd,0,cp.x-point.x,cp.y-point.y,0,0,SWP_NOSIZE);
		}

		//unchild the window and grow the remaining child
		{
			DWORD style=(DWORD)GetWindowLongPtr(hwnd,GWL_STYLE);

			if(style & WS_CHILD)
			{
				int itemId=(int)GetMenu(hwnd);
				HWND otherChildWindow=GetDlgItem(parent,itemId ? 0 :1);

				style^=WS_CHILD;
				SetWindowLongPtr(hwnd,GWL_STYLE,(LONG)style);
				
				

				if(otherChildWindow)
				{
					RECT otherChildWindowRect;
					GetClientRect(parent,&otherChildWindowRect);

					MoveWindow(otherChildWindow,0,0,otherChildWindowRect.right-otherChildWindowRect.left,otherChildWindowRect.bottom-otherChildWindowRect.top,true);

				}
			}
		}


	}
}

WHSplitterWindow::WHSplitterWindow()
{
	type=WHSD_VERTICAL;
	hwnd=0;
	splitting=false;
}
int WHSplitterWindow::width=4;





LRESULT CALLBACK SplitterWindowProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	WHSplitterWindow* sw=(WHSplitterWindow*)GetWindowLongPtr(hwnd,GWL_USERDATA);
	
	
	
	if(sw)
		sw->movingChild.OnMainWindowProcedure(hwnd);


	switch(msg)
	{
	case WM_LBUTTONDOWN: sw->splitting=true; sw->delta=MAKEPOINTS(lparam); break;
	case WM_LBUTTONUP: sw->splitting=false; break;
	case WM_MOUSEMOVE: 
		if(sw->splitting)
		{
			POINTS p=MAKEPOINTS(lparam);

			if((sw->delta.x-p.x)!=0)
			{
				HWND hwnd1=GetDlgItem(hwnd,0);
				HWND hwnd2=GetDlgItem(hwnd,1);

				RECT rect1;
				RECT rect2;

				GetWindowRect(hwnd1,&rect1);
				GetWindowRect(hwnd2,&rect2);

				WHMapWindowRect(HWND_DESKTOP,hwnd,&rect1);
				WHMapWindowRect(HWND_DESKTOP,hwnd,&rect2);

				int delta=p.x-sw->delta.x;

				MoveWindow(hwnd1,rect1.left,rect1.top,rect1.right-rect1.left+delta,rect1.bottom-rect1.top,true);
				MoveWindow(hwnd2,rect2.left+delta,rect2.top,rect2.right-rect2.left-delta,rect2.bottom-rect2.top,true);
			}
			sw->delta=p;

		}
		break;
	}

	LRESULT result=DefDlgProc(hwnd,msg,wparam,lparam);

	return result;
}




LRESULT CALLBACK ChildTabProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	WNDPROC oldproc=(WNDPROC)GetWindowLongPtr(hwnd,GWL_USERDATA);
	
	LRESULT result=CallWindowProc(oldproc,hwnd,msg,wparam,lparam);

	switch(msg)
	{
		case WM_LBUTTONDOWN: ((WHSplitterWindow*)GetWindowLongPtr(GetParent(hwnd),GWL_USERDATA))->movingChild.OnChildMouseButtonDown(hwnd,lparam);break;
		case WM_LBUTTONUP:   ((WHSplitterWindow*)GetWindowLongPtr(GetParent(hwnd),GWL_USERDATA))->movingChild.OnChildMouseButtonUp();break;
	}

	return result;
}


void TestSplitter()
{
	WHSplitterWindow* splitterWindow=new WHSplitterWindow;
	HWND hwnd=CreateWindow(WC_DIALOG,"SplitterTest",WS_OVERLAPPEDWINDOW|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,0,0,0,0);

	WHSetWindowDataAndProcedure(hwnd,splitterWindow,SplitterWindowProc);	

	TCITEM tci;
	tci.mask=TCIF_TEXT;
	char idx[CHAR_MAX]="child_";

	for(int i=0;i<2;i++)
	{
		itoa(i,&idx[6],10);
		tci.pszText=idx;

		HWND child=CreateWindow(WC_TABCONTROL,"child",TCS_FOCUSNEVER|WS_VISIBLE|WS_CHILD,i*100,0,100,100,hwnd,(HMENU)i,0,0);
		
		WNDPROC oldTabProc=(WNDPROC)GetWindowLongPtr(child,GWL_WNDPROC);
		WHSetWindowDataAndProcedure(child,oldTabProc,ChildTabProc);
		SendMessage(child,TCM_INSERTITEM,i,(LPARAM)&tci);
	}


	WHOrganizeWindowContent(hwnd);
};
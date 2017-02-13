#include "win32helper.h"

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

IntSize WHRectToSize(RECT& rc)
{
	return IntSize(rc.right-rc.left,rc.bottom-rc.top);
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
	char wName[CHAR_MAX];
	GetWindowText(hwnd,wName,CHAR_MAX);
	TVector<HWND> childs=WHEnumFirstLevelWindowChilds(hwnd);
	IntSize wSize=WHGetClientSize(hwnd);
	int nSplitters=childs.size-1;
	int splitterSize=5;
	int w=(wSize.x-(splitterSize*nSplitters))/childs.size;

	for(int i=0;i<childs.size;i++)
	{
		char cName[CHAR_MAX];
		GetWindowText(childs[i],cName,CHAR_MAX);


		int x=i*w+i*splitterSize;
		
		MoveWindow(childs[i],x,0,w,wSize.y,true);
	}
}


void WHFillChildInParent(HWND child)
{
	RECT r=WHGetClientRect(GetParent(child));
	MoveWindow(child,r.left,r.top,r.right,r.bottom,true);
}



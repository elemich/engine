#ifndef WIN32_HELPER_H
#define WIN32_HELPER_H

#include "datatypes.h"

#include <windows.h>

#include <winuser.h>
#include <wingdi.h>
#include <shlobj.h>
#include <objbase.h>
#include <CommCtrl.h>
#include <cstdio>

IntSize WHGetWindowSize(HWND hwnd);
IntSize WHGetClientSize(HWND hwnd);

void WHSetWindowSize(HWND hwnd,int x,int y);

void WHSetWindowDataAndProcedure(HWND hwnd,void* inputData,WNDPROC inputProc);

void WHMapWindowRect(HWND from,HWND to,RECT* rc);
RECT WHGetClientRect(HWND hwnd);

TVector<HWND> WHEnumFirstLevelWindowChilds(HWND);
void WHOrganizeWindowContent(HWND);






struct WHMovingChild
{
	HWND	hwnd;
	POINTS	point;

	WHMovingChild();

	void OnChildMouseButtonDown(HWND child,LPARAM lparam);
	void OnChildMouseButtonUp();
	void OnChildWindowProcedure();
	void OnMainWindowProcedure(HWND);
};

struct WHSplitterWindow
{
	enum SplitterType
	{
		WHSD_VERTICAL,
		WHSD_HORIZONTAL
	};

	static int	width;
	SplitterType type;
	HWND		 hwnd;
	bool		splitting;
	POINTS		delta;

	WHMovingChild     movingChild;

	WHSplitterWindow();
};

struct WHMainWindow
{
	WHSplitterWindow *splitters;
	WHMovingChild     movingChild;

	WHMainWindow();
};

LRESULT CALLBACK TestSplitterProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);

void TestSplitter();


#endif //WIN32_HELPER_H

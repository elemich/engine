#ifndef WIN32_HELPER_H
#define WIN32_HELPER_H

#include "datatypes.h"

#include <windows.h>
#include <windowsx.h>

#include <winuser.h>
#include <wingdi.h>
#include <shlobj.h>
#include <objbase.h>
#include <CommCtrl.h>
#include <cstdio>

#include "win32splitter.h"

IntSize WHGetWindowSize(HWND hwnd);
IntSize WHGetClientSize(HWND hwnd);

void WHSetWindowSize(HWND hwnd,int x,int y);

void WHSetWindowDataAndProcedure(HWND hwnd,void* inputData,WNDPROC inputProc);

void WHMapWindowRect(HWND from,HWND to,RECT* rc);
RECT WHGetClientRect(HWND hwnd);

TVector<HWND> WHEnumFirstLevelWindowChilds(HWND);
void WHOrganizeWindowContent(HWND);
void WHFillChildInParent(HWND);
IntSize WHRectToSize(RECT& rc);





#endif //WIN32_HELPER_H

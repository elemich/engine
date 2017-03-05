#ifndef WIN32INCLUDES_H
#define WIN32INCLUDES_H

#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

 //#pragma warning(disable:4996) //


#include <windows.h>
#include <windowsx.h>

#include <winuser.h>
#include <wingdi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <objbase.h>
#include <CommCtrl.h>

#include <array>
	
#include "gl/gl.h"
//#define GL_GLEXT_PROTOTYPES
#include <glext.h>
#include <wgl.h>
#include <wglext.h>

#include <d3d11.h>

#include "datatypes.h"

#include "gui_interfaces.h"

#include "win32defines.h"

#define PRINT(x) #x
#define PRINTF(x) PRINT(x)

#define LOCATION "@mic (" __FILE__ " : " PRINTF(__LINE__) ")"

#ifndef WINVER
#define WINVER 0x0601
#pragma message (LOCATION " WINVER: " PRINTF(WINVER))
#endif

void glCheckError();

#endif
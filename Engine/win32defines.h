#ifndef WIN32DEFINES_H
#define WIN32DEFINES_H

#define SPLITTER_DEBUG true
#define BROWSER_DEBUG true
#define TMP_DEBUG true

//#define RENDERER DirectXRenderer()
#define RENDERER OpenGLFixedRenderer
#define BROWSER  ProjectFolderBrowser2

#define WC_MAINAPPWINDOW "MainAppWindow"
#define WC_CONTAINERWINDOW "ContainerWindow"
#define WC_TABCONTAINER  "TabContainer"
#define WC_OPENGLWINDOW  "OpenGLWindow"


LRESULT CALLBACK MainWindowProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);
LRESULT CALLBACK TabProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);
LRESULT CALLBACK OpenGLProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);

typedef FourLinkNode<HWND> WINDOWNODE;



#endif
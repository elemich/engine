#include "win32.h"



int WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
{
	AppInterface *app = new App();

	app->Init();

	app->AppLoop();

	return 0;
}
#include "win32.h"

//int WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
int main()
{
	EngineIDE *app = new EngineIDEWin32;

	app->Initialize();

	app->Run();

	return 0;
}

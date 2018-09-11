#include "win32.h"

//int WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
int main()
{
	Ide* app = new IdeWin32;;

	app->Initialize();

	app->Run();

	return 0;
}

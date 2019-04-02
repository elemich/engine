#include "win32.h"

//int WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
int main()
{
	Ide* app = Ide::Instance();

	Ide::Instance();

	app->Run();

	return 0;
}

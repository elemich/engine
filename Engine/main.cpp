#include "win32.h"



//int WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
int main()
{
	AppInterface *app = new App();

	app->Init();

	app->AppLoop();

	vec2 v2;
	vec3 v3;
	vec4 v4;


	return 0;
}
#include "win32.h"

//int WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
int main()
{
	AppInterface *app = new App();

	app->Init();
/*

	MatrixStack::view.translate(100,100,100);
	MatrixStack::view.lookat(vec3(),vec3(0,0,1));

	MatrixStack::SetProjectionMatrix(MatrixStack::projection);
	MatrixStack::SetModelviewMatrix(MatrixStack::model);

	ShadersPool::SetMatrices(MatrixStack::projection,MatrixStack::model);
*/

	app->Run();

	return 0;
}
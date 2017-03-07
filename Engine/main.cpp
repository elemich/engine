#include "win32.h"



//int WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
int main()
{
	AppInterface *app = new App();

	app->Init();

	MatrixStack::SetProjectionMatrix(MatrixStack::projection);
	MatrixStack::SetModelviewMatrix(MatrixStack::modelview);

	ShadersPool::SetMatrices(MatrixStack::projection,MatrixStack::modelview);

	app->AppLoop();


	return 0;
}
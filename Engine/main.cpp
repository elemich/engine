#include "win32.h"

template <int i> struct C
{
	char d[i];
};

static C<1> CC;

#define aa sizeof(CC)-1
#define bb CC(C<aa+2>);

//int WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
int main()
{
	AppInterface *app = new App();

	app->Init();

	MatrixStack::SetProjectionMatrix(MatrixStack::projection);
	MatrixStack::SetModelviewMatrix(MatrixStack::modelview);

	ShadersPool::SetMatrices(MatrixStack::projection,MatrixStack::modelview);

	app->Run();

	return 0;
}
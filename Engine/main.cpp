#include "win32.h"

struct i
{
	virtual i* geta(){return 0;}
	virtual i* getb(){return 0;}
};

struct a : i{
	a()
	{
		printf("");
	}
	virtual a* geta(){return this;}
};

struct b: i{
	b()
	{
		printf("");
	}
	virtual b* getb(){return this;}
};


struct c: b{
	c()
	{
		printf("");
	}
	c* getb(){return this;}
};
//int WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
int main()
{
	

	i* aa=new c;

	aa->geta();
	aa->getb();


	AppInterface *app = new App();

	app->Init();

	MatrixStack::SetProjectionMatrix(MatrixStack::projection);
	MatrixStack::SetModelviewMatrix(MatrixStack::modelview);

	ShadersPool::SetMatrices(MatrixStack::projection,MatrixStack::modelview);

	app->AppLoop();

	


	return 0;
}
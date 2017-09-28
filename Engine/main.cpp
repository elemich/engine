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
	vec4 a(1,1,0,1);
	vec4 b(2,1,0,1);
	vec4 c(2,2,1,1);

	vec4 p=plane(a,b,c);

	bool lie;
	lie=isPointOnPlane(p,b);
	lie=isPointOnPlane(p,b);
	lie=isPointOnLine(vec3(0,0,1),vec3(0,0,3),vec3(0,0,10));

	float* eqlist[3]={a,b,c};
	vec4 result;
	eqSolve(result,3,4,eqlist);

	AppInterface *app = new App();

	app->Init();

	MatrixStack::SetProjectionMatrix(MatrixStack::projection);
	MatrixStack::SetModelviewMatrix(MatrixStack::model);

	ShadersPool::SetMatrices(MatrixStack::projection,MatrixStack::model);

	app->Run();

	return 0;
}
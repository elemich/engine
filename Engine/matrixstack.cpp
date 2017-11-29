#include "datatypes.h"


#define MATRIXSTACK_ARRAY_SIZES 64


float matrixstack[MatrixStack::MATRIXMODE_MAX][MATRIXSTACK_ARRAY_SIZES][16];
int	  levels[MatrixStack::MATRIXMODE_MAX];
int	  mode;

mat4 MatrixStack::model;
mat4 MatrixStack::projection;
mat4 MatrixStack::view;

void MatrixStack::Reset()
	
{
	mode=MatrixStack::MODEL;

	for(int i=0;i<MatrixStack::MATRIXMODE_MAX;i++)
		for(int j=0;j<MATRIXSTACK_ARRAY_SIZES;j++)
			MatrixMathNamespace::identity(matrixstack[i][j]);

	levels[0]=levels[1]=0;
}



float* MatrixStack::Get(MatrixStack::matrixmode m,int lev)
{
	return matrixstack[m][(lev<0 ? levels[m] : lev)];
}

float* MatrixStack::Get()
{
	return Get((MatrixStack::matrixmode)mode);
}


void MatrixStack::SetProjectionMatrix(float* pm)
{
	memcpy(matrixstack[MatrixStack::PROJECTION][levels[MatrixStack::PROJECTION]],pm,sizeof(float)*16);
}
void MatrixStack::SetModelMatrix(float* mm)
{
	memcpy(matrixstack[MatrixStack::MODEL][levels[MatrixStack::MODEL]],mm,sizeof(float)*16);
}
void MatrixStack::SetViewMatrix(float* mm)
{
	memcpy(matrixstack[MatrixStack::VIEW][levels[MatrixStack::VIEW]],mm,sizeof(float)*16);
}

mat4 MatrixStack::GetProjectionMatrix()
{
	if(!matrixstack[PROJECTION][levels[PROJECTION]])
		__debugbreak();

	return matrixstack[PROJECTION][levels[PROJECTION]];
}
mat4 MatrixStack::GetModelMatrix()
{
	if(!matrixstack[MODEL][levels[MODEL]])
		__debugbreak();

	return matrixstack[MODEL][levels[MODEL]];
}

mat4 MatrixStack::GetViewMatrix()
{
	if(!matrixstack[VIEW][levels[VIEW]])
		__debugbreak();

	return matrixstack[VIEW][levels[VIEW]];
}


void MatrixStack::Push()
{
	Push((MatrixStack::matrixmode)mode);
}

void MatrixStack::Pop()
{
	Pop((MatrixStack::matrixmode)mode);
}



void MatrixStack::Identity()
{
	Identity((MatrixStack::matrixmode)mode);
}

void MatrixStack::Identity(MatrixStack::matrixmode m)
{
	MatrixMathNamespace::identity(Get(m));
}

void MatrixStack::Load(float* m)
{
	memcpy(matrixstack[mode][levels[mode]],m,sizeof(float)*16);
}

void MatrixStack::Load(MatrixStack::matrixmode md,float* m)
{
	memcpy(matrixstack[md][levels[md]],m,sizeof(float)*16);
}

void MatrixStack::Multiply(float* m)
{
	/*MatrixMathNamespace::multiply(m,matrixstack[mode][levels[mode]]);
	SetMatrix((MatrixStack::matrixmode)mode,m);*/
	MatrixMathNamespace::multiply(matrixstack[mode][levels[mode]],m);
}

void MatrixStack::Multiply(MatrixStack::matrixmode m,float* mtx)
{
	MatrixMathNamespace::multiply(Get(m),mtx);
}

void MatrixStack::Push(MatrixStack::matrixmode m)
{
	if(levels[m]<(MATRIXSTACK_ARRAY_SIZES-1))
	{
		levels[m]++;
		memcpy(matrixstack[m][levels[m]],matrixstack[m][levels[m]-1],sizeof(float)*16);
	}
}

void MatrixStack::Push(MatrixStack::matrixmode m,float* mtx)
{
	if(levels[m]<(MATRIXSTACK_ARRAY_SIZES-1))
	{
		levels[m]++;
		memcpy(matrixstack[m][levels[m]],mtx,sizeof(float)*16);
	}
}



void MatrixStack::Pop(MatrixStack::matrixmode m)
{
	if(levels[m]>0)
	{
		levels[m]--;
	}
}



void MatrixStack::Rotate(float a,float x,float y,float z)
{	
	MatrixMathNamespace::rotate(Get(),a,x,y,z);
}

void MatrixStack::Translate(float x,float y,float z)
{
	float f[3]={x,y,z};
	MatrixMathNamespace::translate(Get(),f);
}

void MatrixStack::Scale(float x,float y,float z)
{
	MatrixMathNamespace::scale(Get(),Get(),x,y,z);
}

MatrixStack::matrixmode MatrixStack::GetMode()
{
	return (MatrixStack::matrixmode)mode;
}

void MatrixStack::SetMode(MatrixStack::matrixmode m)
{
	mode=m;
}




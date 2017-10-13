#include "datatypes.h"


#define MATRIXSTACK_ARRAY_SIZES 64


float matrixstack[MatrixStack::MATRIXMODE_MAX][MATRIXSTACK_ARRAY_SIZES][16];
int	  levels[MatrixStack::MATRIXMODE_MAX];
int	  mode;

mat4 MatrixStack::model;
mat4 MatrixStack::projection;
mat4 MatrixStack::view;

void MatrixStack::Init()
	
{
	mode=MatrixStack::MODELVIEW;

	for(int i=0;i<MatrixStack::MATRIXMODE_MAX;i++)
		for(int j=0;j<MATRIXSTACK_ARRAY_SIZES;j++)
			MatrixMathNamespace::identity(matrixstack[i][j]);

	levels[0]=levels[1]=0;
}



float* MatrixStack::GetMatrix(MatrixStack::matrixmode m,int lev)
{
	return matrixstack[m][(lev<0 ? levels[m] : lev)];
}

float* MatrixStack::GetMatrix()
{
	return GetMatrix((MatrixStack::matrixmode)mode);
}


void MatrixStack::SetProjectionMatrix(float* pm)
{
	memcpy(matrixstack[MatrixStack::PROJECTION][levels[MatrixStack::PROJECTION]],pm,sizeof(float)*16);
}
void MatrixStack::SetModelviewMatrix(float* mm)
{
	memcpy(matrixstack[MatrixStack::MODELVIEW][levels[MatrixStack::MODELVIEW]],mm,sizeof(float)*16);
}

float* MatrixStack::GetProjectionMatrix()
{
	if(!matrixstack[PROJECTION][levels[PROJECTION]])
		__debugbreak();

	return matrixstack[PROJECTION][levels[PROJECTION]];
}
float* MatrixStack::GetModelviewMatrix()
{
	if(!matrixstack[MODELVIEW][levels[MODELVIEW]])
		__debugbreak();

	return matrixstack[MODELVIEW][levels[MODELVIEW]];
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
	MatrixMathNamespace::identity(GetMatrix(m));
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
	MatrixMathNamespace::multiply(GetMatrix(m),mtx);
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
	MatrixMathNamespace::rotate(GetMatrix(),a,x,y,z);
}

void MatrixStack::Translate(float x,float y,float z)
{
	float f[3]={x,y,z};
	MatrixMathNamespace::translate(GetMatrix(),f);
}

void MatrixStack::Scale(float x,float y,float z)
{
	MatrixMathNamespace::scale(GetMatrix(),GetMatrix(),x,y,z);
}

MatrixStack::matrixmode MatrixStack::GetMode()
{
	return (MatrixStack::matrixmode)mode;
}

void MatrixStack::SetMode(MatrixStack::matrixmode m)
{
	mode=m;
}




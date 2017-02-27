#ifndef DATATYPES_H
#define DATATYPES_H

#include "interfaces.h"



struct MatrixStack
{
	enum matrixmode
	{
		PROJECTION=0,
		MODELVIEW,
		MATRIXMODE_MAX
	};

	static void Init();


	static void Push();
	static void Pop();
	static void Identity();
	static float* GetMatrix();
	static void SetShaderMatrix();
	static void Load(float* m);
	static void Multiply(float* m);

	static void Pop(MatrixStack::matrixmode);
	static void Push(MatrixStack::matrixmode);
	static void Push(MatrixStack::matrixmode,float*);
	static void Identity(MatrixStack::matrixmode);
	static float* GetMatrix(MatrixStack::matrixmode,int lev=-1);
	static void SetShaderMatrix(MatrixStack::matrixmode);
	static void SetShaderMatrix(MatrixStack::matrixmode,float*);
	static void Load(MatrixStack::matrixmode,float*);
	static void Multiply(MatrixStack::matrixmode,float*);

	static void Rotate(float a,float x,float y,float z);
	static void Translate(float x,float y,float z);
	static void Scale(float x,float y,float z);

	static float* GetProjectionMatrix();
	static float* GetModelviewMatrix();

	static void SetProjectionMatrix(float*);
	static void SetModelviewMatrix(float*);

	static  MatrixStack::matrixmode GetMode();
	static  void SetMode(MatrixStack::matrixmode m);
};

struct ShaderManager
{
	static TDLList<ShaderInterface*> shaders;
	static ShaderInterface			  *current;


	static ShaderInterface* Find(const char*,bool exact=true);

	static void SetMatrices(float* proj,float* mdlv);

	static ShaderInterface* GetCurrent();
	static void SetCurrent(ShaderInterface*);
};

int simple_shader(int shader_type, const char* shader_src);
int create_program(const char* vertexsh,const char* fragmentsh);

struct Entity
{
	int animate(float);
	void update();
	void draw();
};

struct EntityManager
{
	static TDLList<Entity*> entities;
	static TDLList<Entity*> typed_entities[10];

	static void draw();
	static void update();
	static int  animate(float);

	static Entity* Find(const char*,bool exact=true);

	static void debugProcessed();
};





struct OpenGLShader : ShaderInterface
{
	static ShaderInterface* Create(const char* shader_name,const char* pixel_shader,const char* fragment_shader);

	int GetProgram();
	void SetProgram(int);

	int GetUniform(int slot,char* var);
	int GetAttrib(int slot,char* var);

	void Use();

	const char* GetPixelShader();
	const char* GetFragmentShader();

	int GetAttribute(const char*);
	int GetUniform(const char*);

	int init();

	int GetPositionSlot();
	int GetColorSlot();
	int GetProjectionSlot();
	int GetModelviewSlot();
	int GetTexcoordSlot();
	int GetTextureSlot();
	int GetLightposSlot();
	int GetLightdiffSlot();
	int GetLightambSlot();
	int GetNormalSlot();

	bool SetMatrix4f(int slot,float* mtx);

	unsigned int& GetBufferObject();

	void SetProjectionMatrix(float*);
	void SetModelviewMatrix(float*);

	void SetByMatrixStack();

	void SetName(const char*);
	const char* GetName();

	//char* name;
	String name;
	int   program;
	unsigned int	  vbo;
	unsigned int	  vao;
	unsigned int	  ibo;
};




#endif //DATATYPES_H




#ifndef DATATYPES_H
#define DATATYPES_H



#include "interfaces.h"
#include "entities.h"

#include "fbxutil.h"


struct MatrixStack
{
	enum matrixmode
	{
		PROJECTION=0,
		MODEL,
		VIEW,
		MATRIXMODE_MAX
	};

	static void Reset();


	static void Push();
	static void Pop();
	static void Identity();
	static float* Get();
	static void Load(float* m);
	static void Multiply(float* m);

	static void Pop(MatrixStack::matrixmode);
	static void Push(MatrixStack::matrixmode);
	static void Push(MatrixStack::matrixmode,float*);
	static void Identity(MatrixStack::matrixmode);
	static float* Get(MatrixStack::matrixmode,int lev=-1);
	static void Load(MatrixStack::matrixmode,float*);
	static void Multiply(MatrixStack::matrixmode,float*);

	static void Rotate(float a,float x,float y,float z);
	static void Translate(float x,float y,float z);
	static void Scale(float x,float y,float z);

	static mat4 GetProjectionMatrix();
	static mat4 GetModelMatrix();
	static mat4 GetViewMatrix();

	static void SetProjectionMatrix(float*);
	static void SetModelMatrix(float*);
	static void SetViewMatrix(float*);

	static  MatrixStack::matrixmode GetMode();
	static  void SetMode(MatrixStack::matrixmode m);

	static mat4 model;
	static mat4 projection;
	static mat4 view;
};

int simple_shader(const char* name,int shader_type, const char* shader_src);
int create_program(const char* name,const char* vertexsh,const char* fragmentsh);

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
	int GetMouseSlot();
	int GetHoveringSlot();

	void SetSelectionColor(bool pick,void* ptr,vec2 mposNrm);

	bool SetMatrix4f(int slot,float* mtx);

	unsigned int& GetBufferObject();

	void SetProjectionMatrix(float*);
	void SetModelviewMatrix(float*);
	void SetMatrices(float* view,float* mdl);

	void SetName(const char*);
	const char* GetName();

	//char* name;
	String name;
	int   program;
	unsigned int	  vbo;
	unsigned int	  vao;
	unsigned int	  ibo;
};

struct TouchInput : InputInterface
{
	enum
	{
		TOUCH_DOWN,
		TOUCH_UP,
		TOUCH_MAX
	};

#define MAX_TOUCH_INPUTS 10

	bool pressed[MAX_TOUCH_INPUTS];
	bool released[MAX_TOUCH_INPUTS];

	vec2 position[MAX_TOUCH_INPUTS];

	TouchInput()
	{
		for(int i=0;i<10;i++)
		{
			pressed[i]=0;
			released[i]=0;
			position[i].make(0,0);
		}
	}

	bool IsPressed(int i){return pressed[i];}
	bool IsReleased(int i){return released[i];}

	void SetPressed(bool b,int i){pressed[i]=b;}
	void SetReleased(bool b,int i){released[i]=b;}


	vec2& GetPosition(int i){return position[i];}
	void   SetPosition(vec2& pos,int i){position[i]=pos;}


};

struct KeyboardInput : InputInterface
{
	enum
	{
		KEY_PRESSED=0,
		KEY_RELEASED,
		KEY_INACTIVE
	};

	int keys[255];
	int nkeys;

	KeyboardInput()
	{
		for(int i=0;i<255;i++)
			SetKey(i,KEY_INACTIVE);
		nkeys=0;
	}

	void SetKey(unsigned char c,int state)//pressed,released,inactive
	{
		keys[c]=state;

		switch(state)
		{
		case KEY_PRESSED:{if(nkeys<sizeof(keys))nkeys++;}break;
		case KEY_RELEASED:{if(nkeys>0)nkeys--;}break;
		}
	}

	int GetKey(unsigned char c)//pressed,released,inactive
	{
		return keys[c];
	}

	bool GetPressed(unsigned char c)
	{
		return keys[c]==KEY_PRESSED;
	}

	bool GetReleased(unsigned char c)
	{
		return keys[c]==KEY_RELEASED;
	}

};

struct MouseInput : InputInterface
{
	enum
	{
		MOUSE_STATE_PRESSED=0,
		MOUSE_STATE_RELEASED,
		MOUSE_STATE_CLICKED,
		MOUSE_STATE_DBLCLICKED,
		MOUSE_STATE_INACTIVE,
		MOUSE_STATE_MAX
	};

	enum
	{
		BUTTON_LEFT=0,
		BUTTON_CENTER,
		BUTTON_RIGHT,
		BUTTON_MAX
	};

	enum
	{
		DIR_LEFT=0,
		DIR_RIGHT,
		DIR_UP,
		DIR_BOTTOM
	};

	int		mouse_states[BUTTON_MAX];
	vec2	mouse_pos;
	vec2	mouse_posold;
	vec2	mouse_posnorm;
	float   mouse_timers[BUTTON_MAX];

	

	MouseInput();


	bool IsClick(int);
	virtual void Update();
};


struct InputManager
{
	static TouchInput touchInput;
	static MouseInput mouseInput;
	static KeyboardInput keyboardInput;
	static InputInterface voiceInput;
	static InputInterface joystickInput;

	void update()
	{
		for(int i=0;i<255;i++)
		{
			int state=keyboardInput.GetKey(i);
			keyboardInput.SetKey(i,state==KeyboardInput::KEY_RELEASED ? KeyboardInput::KEY_INACTIVE : state);
		}
	}
};


#endif //DATATYPES_H




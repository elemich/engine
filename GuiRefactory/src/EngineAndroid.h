#include "entities.h"

#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

struct Renderer3DAndroid;
struct ShaderAndroid;

extern "C" {
	JNIEXPORT void JNICALL Java_com_android_Engine_EngineLib_init(JNIEnv * env, jobject obj,  jint width, jint height);
	JNIEXPORT void JNICALL Java_com_android_Engine_EngineLib_step(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_android_Engine_EngineView_SetTouchEvent(JNIEnv * env, jobject obj,jint action,jint id,jint idx,jfloat x,jfloat y);
};

jint JNI_OnLoad(JavaVM*, void*);

bool JniInit();


struct TimerAndroid : Timer
{
	void update();
};

struct ShaderAndroid : Shader
{
	Renderer3DAndroid* renderer;

	ShaderAndroid(Renderer3DAndroid*);
	~ShaderAndroid();

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
	int GetPointSize();

	void SetSelectionColor(bool pick,void* ptr,vec2 iMpos,vec2 iRectSize);

	bool SetMatrix4f(int slot,float* mtx);

	unsigned int& GetBufferObject();

	void SetProjectionMatrix(float*);
	void SetModelviewMatrix(float*);
	void SetMatrices(float* view,float* mdl);
};

struct Renderer3DAndroid : Renderer3DBase
{
	PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOES;
	PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOES;
	PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOES;
	PFNGLISVERTEXARRAYOESPROC glIsVertexArrayOES;

	GLuint vertexArrayObject;
	GLuint vertexBufferObject;
	GLuint textureBufferObject;
	GLuint indicesBufferObject;
	GLuint frameBuffer;
	GLuint textureColorbuffer;
	GLuint textureRenderbuffer;
	GLuint pixelBuffer;
	GLuint renderBufferColor;
	GLuint renderBufferDepth;

	Renderer3DAndroid();
	~Renderer3DAndroid();

	int		CreateShader(const char* name,int shader_type, const char* shader_src);
	Shader* CreateProgram(const char* name,const char* vertexsh,const char* fragmentsh);
	Shader* CreateShaderProgram(const char* name,const char* pix,const char* frag);

	void draw(vec3,float psize=1.0f,vec3 color=vec3(1,1,1));
	void draw(vec2);
	void draw(vec3,vec3,vec3 color=vec3(1,1,1));
	void draw(vec4);
	void draw(AABB,vec3 color=vec3(1,1,1));
	void draw(mat4 mtx,float size,vec3 color=vec3(1,1,1));
	//void draw(Font*,char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4);
	void draw(char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4);

	void draw(Light*);
	void draw(Mesh*);
	void draw(Skin*);
	void draw(Bone*);
	void draw(Texture*);
	void drawUnlitTextured(Mesh*);
	void draw(Mesh*,std::vector<unsigned int>& textureIndices,int texture_slot,int texcoord_slot);
	void draw(Camera*);
	void draw(Gizmo*);
	void draw(Script*);
};



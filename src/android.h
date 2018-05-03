#include "entities.h"

#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

extern "C" {
	JNIEXPORT void JNICALL Java_com_android_Engine_EngineLib_init(JNIEnv * env, jobject obj,  jint width, jint height);
	JNIEXPORT void JNICALL Java_com_android_Engine_EngineLib_step(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_android_Engine_EngineView_SetTouchEvent(JNIEnv * env, jobject obj,jint action,jint id,jint idx,jfloat x,jfloat y);
};

jint JNI_OnLoad(JavaVM*, void*);

bool JniInit();


struct ShaderAndroid : Shader
{
	static Shader* Create(const char* shader_name,const char* pixel_shader,const char* fragment_shader);

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

struct Renderer3DAndroid : Renderer3DBase
{
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

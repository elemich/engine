#include "android.h"

#include <dlfcn.h>

void glCheckError();
void __debugbreak()
{
	assert(0);
}

const char*     current_dir=NULL;

JavaVM			*globalJvm=NULL;
JNIEnv			*globalJniEnv=NULL;

//Resources

AAssetManager 	*globalAssetManager=NULL;

extern FILE*                        resourceData;
extern unsigned int                 resourceDataSize;
extern unsigned int                 resourceTableSize;
extern unsigned int                 resourceTableStart;
extern unsigned int                 resourceTableEnd;
extern unsigned int                 resourceDataStart;
extern unsigned int                 resourceDataEnd;

int asset_read(void* iFile,char* iBuffer,int iCount)
{
    return AAsset_read((AAsset*)iFile,iBuffer,iCount);
}

int asset_write(void* iFile,const char* iBuffer,int iCount)
{
    printf("fwrite not implemented\n");
    return 0;
}

fpos_t asset_seek(void* iFile,fpos_t iOffset,int iOrigin)
{
    return AAsset_seek((AAsset*)iFile,iOffset,iOrigin);
}

int asset_close(void* iFile)
{
    AAsset_close((AAsset*)iFile);
    return 0;
}

FILE* asset_open(const char* iFilename,const char* iMode)
{
    if(0!=strstr(iMode,"w") || 0!=strstr(iMode,"W"))
    {
        printf("error: trying to open asset in r mode\n");
        return 0;
    }

    AAsset* tAsset=AAssetManager_open(globalAssetManager,iFilename,AASSET_MODE_STREAMING);

    if(tAsset)
    {
        return funopen((const void*)tAsset,asset_read,asset_write,asset_seek,asset_close);
    }

    return 0;
}

jint JNI_OnLoad(JavaVM* iJavaVM, void* aReserved)
{
    const int	    tJVer=JNI_VERSION_1_6;
    const char*     tJavaActivityPath="com/android/Engine/EngineActivity";
    jclass          tActivityClass;

	globalJvm = iJavaVM;

	printf("Engine: JNI_OnLoad\n");

	if (globalJvm->GetEnv((void**)&globalJniEnv, tJVer) != JNI_OK)
	{
		printf("Engine: JNI_OnLoad() failed to get the JVM\n");
		return -1;
	}

	//Get SBCEngine activity class
    tActivityClass = globalJniEnv->FindClass(tJavaActivityPath);

	if(!tActivityClass)
	{
		printf("Engine: JNI_OnLoad() failed to get %s class reference through JVM\n",tJavaActivityPath);
		return -1;
	}

    //Get SBCEngine activity class
	globalJniEnv->NewGlobalRef(tActivityClass);

	JniInit();

	return tJVer;
}

bool JniInit()
{
    printf("Engine: JniInit\n");

	jmethodID   pfAssetManagerFunction=NULL;
	jmethodID   pfCurrentPathFunction=NULL;

	const char* tJavaViewPath="com/android/Engine/EngineView";

	jclass      tEngineViewClass = globalJniEnv->FindClass(tJavaViewPath);

	if(tEngineViewClass)
	{
		pfCurrentPathFunction = globalJniEnv->GetStaticMethodID(tEngineViewClass, "GetCurrentPath", "()Ljava/lang/String;");

		if(pfCurrentPathFunction)
			current_dir = globalJniEnv->GetStringUTFChars((jstring)globalJniEnv->CallStaticObjectMethod(tEngineViewClass, pfCurrentPathFunction ), 0);
		else
        {
            printf("Engine: JniInit() failed to GetCurrentPath from JVM\n");
            return false;
        }

		pfAssetManagerFunction = globalJniEnv->GetStaticMethodID(tEngineViewClass, "GetAssetManager", "()Landroid/content/res/AssetManager;");

		if(pfAssetManagerFunction)
        {
            //create manager
			globalAssetManager = AAssetManager_fromJava(globalJniEnv,globalJniEnv->CallStaticObjectMethod(tEngineViewClass, pfAssetManagerFunction ));

			//open asset
            resourceData=asset_open("asset.mp3","r");

            if(resourceData)
            {
                printf("Resource: loading asset\n");

                fread(&resourceTableSize,sizeof(int),1,resourceData);
                fread(&resourceDataSize,sizeof(int),1,resourceData);

                resourceTableStart=sizeof(int)*2;
                resourceTableEnd=resourceTableStart + resourceTableSize;
                resourceDataStart=resourceTableEnd;
                resourceDataEnd=resourceDataStart + resourceDataSize;

                printf("Resource: asset table size is %d\n",resourceTableSize);
                printf("Resource: asset data size is %d\n",resourceDataSize);

                {
                    const char* tResource="\\pippo\\1.txt";
                    const char* tResource2="\\pippo\\peppo\\2.txt";

                    char* tBuffer=(char*)Resource::Load(tResource);
                    char* tBuffer2=(char*)Resource::Load(tResource2);

                    if(tBuffer && tBuffer2)
                    {
                        printf("resource %s %s found\n",tResource,tResource2);

                        printf("result %s %s \n",tBuffer,tBuffer2);

                        SAFEDELETE(tBuffer);
                        SAFEDELETE(tBuffer2);
                    }
                    else
                        printf("resource %s not found\n",tResource);


                }
            }
            else
                printf("error opening asset file\n");

        }
		else
        {
            printf("Engine: JniInit() failed to GetAssetManager from JVM\n");
            return false;
        }

		return true;
	}

	return false;
}

JNIEXPORT void JNICALL Java_com_android_Engine_EngineLib_init(JNIEnv * env,jobject  obj,  jint width, jint height)
{

}

JNIEXPORT void JNICALL Java_com_android_Engine_EngineLib_step(JNIEnv * env, jobject obj)
{

}

JNIEXPORT void JNICALL Java_com_android_Engine_EngineView_SetTouchEvent(JNIEnv * env, jobject obj,jint action,jint id,jint idx,jfloat x,jfloat y)
{


}

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////ShaderAndroid//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

#include "shaders.cpp"

int simple_shader(const char* name,int shader_type, const char* shader_src)
{
	GLint compile_success = 0;
	GLchar message[1024];
	int len=0;
	int shader_id;

	shader_id = glCreateShader(shader_type);glCheckError();

	if(!shader_id)
	{
		printf("glCreateShader error for %d,%s\n",shader_type,shader_src);glCheckError();
		__debugbreak();
		return 0;
	}

	glShaderSource(shader_id, 1, &shader_src,NULL);glCheckError();
	glCompileShader(shader_id);glCheckError();
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_success);glCheckError();

	if (GL_FALSE==compile_success)
	{
		sprintf(message,"glCompileShader[%s] error:\n",name);
		glGetShaderInfoLog(shader_id, sizeof(message), &len, &message[strlen(message)]);
		__debugbreak();
	}

	return shader_id;
}


int create_program(const char* name,const char* vertexsh,const char* fragmentsh)
{
	GLint link_success=0;
	GLint program=0;
	GLint vertex_shader=0;
	GLint fragment_shader=0;
	GLchar message[1024]={0};
	GLint len=0;

	program = glCreateProgram();glCheckError();

	if(!program)
	{
		printf("glCreateProgram error for %s,%s\n",vertexsh,fragmentsh);
		__debugbreak();
		return 0;
	}

	vertex_shader=simple_shader(name,GL_VERTEX_SHADER, vertexsh);
	fragment_shader=simple_shader(name,GL_FRAGMENT_SHADER, fragmentsh);

	glAttachShader(program, vertex_shader);glCheckError();
	glAttachShader(program, fragment_shader);glCheckError();
	glLinkProgram(program);glCheckError();
	glGetProgramiv(program, GL_LINK_STATUS, &link_success);glCheckError();

	if (GL_FALSE==link_success)
	{
		printf("glLinkProgram error for %s\n",message);
		__debugbreak();
	}

	glGetProgramInfoLog(program,sizeof(message),&len,message);glCheckError();


	//print infos
	/*if(len && len<sizeof(message))
		printf("%s",message);*/


	return program;
}



Shader* ShaderAndroid::Create(const char* name,const char* pix,const char* frag)
{
	int program=create_program(name,pix,frag);

	if(program)
	{
		Shader* shader=new ShaderAndroid;

		shader->SetName(name);
		shader->SetProgram(program);
		shader->Use();
		shader->init();

		return shader;
	}
	else

		printf("error creating shader %s\n",name);

	return 0;
}



unsigned int& ShaderAndroid::GetBufferObject()
{
	return vbo;
}

int ShaderAndroid::GetProgram(){return program;}
void ShaderAndroid::SetProgram(int p){program=p;}

int ShaderAndroid::GetUniform(int slot,char* var)
{
	return glGetUniformLocation(slot,var);glCheckError();
}
int ShaderAndroid::GetAttrib(int slot,char* var)
{
	return glGetAttribLocation(slot,var);glCheckError();
}

void ShaderAndroid::SetProjectionMatrix(float* pm)
{
	this->SetMatrix4f(this->GetProjectionSlot(),pm);
}
void ShaderAndroid::SetModelviewMatrix(float* mm)
{
	this->SetMatrix4f(this->GetModelviewSlot(),mm);
}

void ShaderAndroid::SetMatrices(float* view,float* mdl)
{
	if(view)
		this->SetProjectionMatrix(view);

	if(mdl)
		this->SetModelviewMatrix(mdl);
}


void ShaderAndroid::Use()
{
	glUseProgram(program);glCheckError();
}

const char* ShaderAndroid::GetPixelShader(){return 0;}
const char* ShaderAndroid::GetFragmentShader(){return 0;}

int ShaderAndroid::init()
{
	mat4 m;

	int proj=GetProjectionSlot();
	int mdlv=GetModelviewSlot();

	bool bOk = this->SetMatrix4f(proj,m) && this->SetMatrix4f(mdlv,m);

	return bOk;
}

int ShaderAndroid::GetAttribute(const char* attrib)
{
	int location=glGetAttribLocation(program,attrib);glCheckError();
	return location;
}

int ShaderAndroid::GetUniform(const char* uniform)
{
	int location=glGetUniformLocation(program,uniform);glCheckError();
	return location;
}

int ShaderAndroid::GetPositionSlot()
{
	return GetAttribute("position");
}
int ShaderAndroid::GetColorSlot()
{
	return GetAttribute("color");
}
int ShaderAndroid::GetProjectionSlot()
{
	return GetUniform("projection");
}
int ShaderAndroid::GetModelviewSlot()
{
	return GetUniform("modelview");
}
int ShaderAndroid::GetTexcoordSlot()
{
	return GetAttribute("texcoord");
}
int ShaderAndroid::GetTextureSlot()
{
	return GetUniform("texture");
}
int ShaderAndroid::GetMouseSlot()
{
	return GetUniform("mpos");
}
int ShaderAndroid::GetLightposSlot()
{
	return GetUniform("lightpos");
}
int ShaderAndroid::GetLightdiffSlot()
{
	return GetUniform("lightdiff");
}
int ShaderAndroid::GetLightambSlot()
{
	return GetUniform("lightamb");
}
int ShaderAndroid::GetNormalSlot()
{
	return GetAttribute("normal");
}
int ShaderAndroid::GetHoveringSlot()
{
	return GetAttribute("hovered");
}

void ShaderAndroid::SetSelectionColor(bool pick,void* ptr,vec2 mposNorm)
{
	int _mousepos=this->GetMouseSlot();
	int _ptrclr=this->GetUniform("ptrclr");

	if(_ptrclr>=0)
	{
		if(pick)
		{
			unsigned char* clr1=(unsigned char*)&ptr;

			float fcx=clr1[3]/255.0f;
			float fcy=clr1[2]/255.0f;
			float fcz=clr1[1]/255.0f;
			float fcw=clr1[0]/255.0f;

			glUniform4f(_ptrclr,fcx,fcy,fcz,fcw);
		}
		else glUniform4f(_ptrclr,0,0,0,0);

		if(_mousepos>=0)
			glUniform2f(_mousepos,mposNorm.x,mposNorm.y);
	}
}

bool ShaderAndroid::SetMatrix4f(int slot,float* mtx)
{
	if(slot<0)
		return false;

	glUniformMatrix4fv(slot,1,0,mtx);glCheckError();
	return true;
}

void ShaderAndroid::SetName(const char* n)
{
	name=n;
}
const char* ShaderAndroid::GetName()
{
	return name;
}


///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////Renderer3DAndroid//////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOES;
PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOES;
PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOES;
PFNGLISVERTEXARRAYOESPROC glIsVertexArrayOES;

void glCheckError()
{
	GLenum err=glGetError();
	if(err!=GL_NO_ERROR)
	{
		__debugbreak();
	}
}

Renderer3DAndroid::Renderer3DAndroid()
{
	void *libhandle = dlopen("libGLESv2.so", RTLD_LAZY);

	glGenVertexArraysOES = (PFNGLGENVERTEXARRAYSOESPROC)dlsym(libhandle, "glGenVertexArraysOES" );
	glBindVertexArrayOES = (PFNGLBINDVERTEXARRAYOESPROC)dlsym(libhandle, "glBindVertexArrayOES" );
	glDeleteVertexArraysOES = (PFNGLDELETEVERTEXARRAYSOESPROC)dlsym(libhandle, "glDeleteVertexArraysOES" );
	glIsVertexArrayOES = (PFNGLISVERTEXARRAYOESPROC)dlsym(libhandle, "glIsVertexArrayOES" );

	glGenFramebuffers(1,&frameBuffer);glCheckError();

	glGenTextures(1,&textureColorbuffer);glCheckError();
	glGenTextures(1,&textureRenderbuffer);glCheckError();

	glGenRenderbuffers(1,&renderBufferColor);glCheckError();
	glGenRenderbuffers(1,&renderBufferDepth);glCheckError();

	glGenVertexArraysOES(1, &vertexArrayObject);glCheckError();
	glBindVertexArrayOES(vertexArrayObject);glCheckError();

	glGenBuffers(1,&vertexBufferObject);glCheckError();
	glGenBuffers(1,&textureBufferObject);glCheckError();
	/*
	//glGenBuffers(1,&indicesBufferObject);

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);
	//glBindBuffer(GL_ARRAY_BUFFER,indicesBufferObject);

	glBufferData(GL_ARRAY_BUFFER,100000,0,GL_DYNAMIC_DRAW);*/

	glGenBuffers(1, &pixelBuffer);

	printf("Status: Using GL %s\n", glGetString(GL_VERSION));
	printf("Status: GLSL ver %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));

	this->unlit=ShaderAndroid::Create("unlit",unlit_vert,unlit_frag);
	this->unlit_color=ShaderAndroid::Create("unlit_color",unlit_color_vert,unlit_color_frag);
	this->unlit_texture=ShaderAndroid::Create("unlit_texture",unlit_texture_vs,unlit_texture_fs);
	this->font=ShaderAndroid::Create("font",font_pixsh,font_frgsh);
	this->shaded_texture=ShaderAndroid::Create("shaded_texture",texture_vertex_shaded_vert,texture_vertex_shaded_frag);

	this->shaders.push_back(this->unlit);
	this->shaders.push_back(this->unlit_color);
	this->shaders.push_back(this->unlit_texture);
	this->shaders.push_back(this->font);
	this->shaders.push_back(this->shaded_texture);
}

void Renderer3DAndroid::draw(Light*)
{

}

void Renderer3DAndroid::draw(vec2)
{

}

void Renderer3DAndroid::draw(Script*)
{

}

void Renderer3DAndroid::draw(Gizmo* gizmo)
{
	this->draw(vec3(0,0,0),vec3(10,0,0),vec3(1,0,0));
	this->draw(vec3(0,0,0),vec3(0,10,0),vec3(0,1,0));
	this->draw(vec3(0,0,0),vec3(0,0,10),vec3(0,0,1));
}

void Renderer3DAndroid::draw(vec3 point,float psize,vec3 col)
{
	Shader* shader=this->unlit_color;

	if(!shader)
		return;

	shader->Use();

	////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());

	//@mic shader->SetSelectionColor(this->picking,0,vec2(this->tabContainer->mousex/this->tabContainerWin32->windowData->width,this->tabContainer->mousey/this->tabContainerWin32->windowData->height));

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_PROGRAM_POINT_SIZE);
	//glPointSize(psize);

	int ps=shader->GetPositionSlot();
	int uniform_color=shader->GetUniform("color");

	if(uniform_color>=0)
	{glUniform3fv(uniform_color,1,col);glCheckError();}

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER,3*sizeof(float),point.v,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(ps);glCheckError();
	glVertexAttribPointer(ps, 3, GL_FLOAT, GL_FALSE, 0,0);glCheckError();

	glDrawArrays(GL_POINTS,0,1);glCheckError();

	glDisableVertexAttribArray(ps);glCheckError();

	//glPointSize(1.0f);
	//glDisable(GL_PROGRAM_POINT_SIZE);
	glDisable(GL_DEPTH_TEST);
}

void Renderer3DAndroid::draw(vec4 rect)
{
	Shader* shader=this->unlit_color;

	int position_slot=-1;
	int modelview_slot=-1;

	if(shader)
	{
		shader->Use();

		////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());
		int mdl=shader->GetModelviewSlot();
		int view=shader->GetProjectionSlot();
		int ptrclr=shader->GetUniform("ptrclr");

		if(ptrclr>=0)
			glUniform4f(ptrclr,0,0,0,0);

	}
	else
		return;

	if(position_slot<0)
		return;

	float dx=rect[2]-rect[0];
	float dy=rect[3]-rect[1];

	float data[]=
	{
		rect[0],	rect[1],		0,
		rect[0],	rect[1]+dy,		0,
		rect[0]+dx,	rect[1]+dy,		0,
		rect[0]+dx,	rect[1],		0,
		rect[0],	rect[1],		0
	};

	glVertexAttribPointer(position_slot, 3, GL_FLOAT, GL_FALSE, 0,data);
	glEnableVertexAttribArray(position_slot);
	glDrawArrays(GL_LINE_STRIP, 0, 5);
}


void Renderer3DAndroid::draw(mat4 mtx,float size,vec3 color)
{
	Shader* shader=this->unlit_color;

	if(!shader)
		return;

	shader->Use();
	////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());

	int mdl=shader->GetModelviewSlot();
	int view=shader->GetProjectionSlot();
	int ptrclr=shader->GetUniform("ptrclr");

	if(ptrclr>=0)
		glUniform4f(ptrclr,0,0,0,0);

	/*MatrixStack::Push();
	MatrixStack::Multiply(mtx);*/

	vec3 axes[6];

	vec3 zero=mtx.transform(0,0,0);

	axes[0]=axes[2]=axes[4]=zero;

	mtx.axes(axes[1],axes[3],axes[5]);

	glEnable(GL_DEPTH_TEST);

	int pos=shader->GetPositionSlot();
	int col=shader->GetUniform("color");

	glUniform3fv(col,1,color);

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER,18*sizeof(float),axes,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(pos);glCheckError();

	glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();

	glDrawArrays(GL_LINES,0,6);glCheckError();
	//glDrawElements(GL_TRIANGLES,3,GL_UNSIGNED_INT,)

	glDisableVertexAttribArray(pos);


	glDisable(GL_DEPTH_TEST);



	//MatrixStack::Pop();
}

void Renderer3DAndroid::draw(AABB aabb,vec3 color)
{
	Shader* shader=this->unlit_color;

	if(!shader)
		return;

	shader->Use();

	////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());

	//shader->SetSelectionColor(false,0,vec2(this->tabContainer->mousex/this->tabContainerWin32->windowData->width,this->tabContainer->mousey/this->tabContainerWin32->windowData->height));

	vec3 &a=aabb.a;
	vec3 &b=aabb.b;

	float dx=b.x-a.x;
	float dy=b.y-a.y;
	float dz=b.z-a.z;

	if(dx<0.00000001f && dy<0.00000001f && dz<0.00000001f)
		return;

	float parallelepiped[72]=
	{
		//lower quad
		a.x,a.y,a.z,		a.x+dx,a.y,a.z,
		a.x+dx,a.y,a.z,		a.x+dx,a.y+dy,a.z,
		a.x+dx,a.y+dy,a.z,	a.x,a.y+dy,a.z,
		a.x,a.y+dy,a.z,     a.x,a.y,a.z,

		//upper quad
		a.x,a.y,a.z+dz,			a.x+dx,a.y,a.z+dz,
		a.x+dx,a.y,a.z+dz,		a.x+dx,a.y+dy,a.z+dz,
		a.x+dx,a.y+dy,a.z+dz,	a.x,a.y+dy,a.z+dz,
		a.x,a.y+dy,a.z+dz,		a.x,a.y,a.z+dz,

		//staffs
		a.x,a.y,a.z,		a.x,a.y,a.z+dz,
		a.x+dx,a.y,a.z,		a.x+dx,a.y,a.z+dz,
		a.x+dx,a.y+dy,a.z,	a.x+dx,a.y+dy,a.z+dz,
		a.x,a.y+dy,a.z,   	a.x,a.y+dy,a.z+dz
	};

	glEnable(GL_DEPTH_TEST);

	int pos=shader->GetPositionSlot();
	int col=shader->GetUniform("color");

	glUniform3fv(col,1,color);

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER,72*sizeof(float),parallelepiped,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(pos);glCheckError();

	glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();

	glDrawArrays(GL_LINES,0,24);glCheckError();
	//glDrawElements(GL_TRIANGLES,3,GL_UNSIGNED_INT,)

	glDisableVertexAttribArray(pos);

	glBindBuffer(GL_ARRAY_BUFFER,0);

	glDisable(GL_DEPTH_TEST);
}

void Renderer3DAndroid::draw(vec3 a,vec3 b,vec3 color)
{
	Shader* shader=this->unlit_color;

	if(!shader)
		return;

	float line[]=
	{
		a[0],a[1],a[2],
		b[0],b[1],b[2],
	};

	shader->Use();

	shader->SetMatrices(MatrixStack::GetViewMatrix()*MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelMatrix());

	//shader->SetSelectionColor(false,0,vec2(this->tabContainer->mousex/this->tabContainerWin32->windowData->width,this->tabContainer->mousey/this->tabContainerWin32->windowData->height));

	glEnable(GL_DEPTH_TEST);

	int pos=shader->GetPositionSlot();
	int col=shader->GetUniform("color");

	glUniform3fv(col,1,color);

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER,6*sizeof(float),line,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(pos);glCheckError();

	glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();

	glDrawArrays(GL_LINES,0,2);glCheckError();
	//glDrawElements(GL_TRIANGLES,3,GL_UNSIGNED_INT,)

	glDisableVertexAttribArray(pos);


	glDisable(GL_DEPTH_TEST);

}

void Renderer3DAndroid::draw(char* text,float x,float y,float width,float height,float sizex,float sizey,float* color4)
{
	Shader* shader=0;//line_color_shader

	if(!shader || !text)
		return;

	shader->Use();

	////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());

	/*Font* font=FontManager::Instance()->Head()->Data();

	this->draw(font,text,x,y,width,height,sizex,sizey,color4);*/
}


/*
void OpenGLFixedRenderer::draw(Window* w)
{
	line_shader->Use();

	int position_slot=line_shader->GetPositionSlot();

	vec2& pos=w->window_pos;
	vec2& dim=w->window_dim;

	float data[]=
	{
		pos[0]-dim[0]/2,	pos[1]+dim[1]/2,		0,//low-left
		pos[0]+dim[0]/2,	pos[1]+dim[1]/2,		0,//low-right
		pos[0]+dim[0]/2,	pos[1]-dim[1]/2,		0,//up-right
		pos[0]-dim[0]/2,	pos[1]-dim[1]/2,		0,//up-left
		pos[0]-dim[0]/2,	pos[1]+dim[1]/2,		0,//low-left
	};

	glVertexAttribPointer(position_slot, 3, GL_FLOAT, GL_FALSE, 0,data);
	glEnableVertexAttribArray(position_slot);
	glDrawArrays(GL_LINE_STRIP, 0, 5);
}*/

/*
void OpenGLFixedRenderer::draw(WindowManager* windowManager)
{
	this->line_shader->Use();

	mat4 mat;
	mat.ortho(-0.5f,0.5f,-0.5f,0.5f,0,10);
	this->line_shader->SetMatrix4f(this->line_shader->GetProjectionSlot(),mat);


	glDisable(GL_CULL_FACE);

	float width=(float)KernelServer::Instance()->GetApp()->GetWidth();
	float height=(float)KernelServer::Instance()->GetApp()->GetHeight();

	for(Node<Window*> *node=windowManager->Head();node;node=node->Next())
		this->draw(node->Data());

	glEnable(GL_CULL_FACE);
}*/

/*
void OpenGLFixedRenderer::draw(EntityManager& entityManager)
{
	glDisable(GL_CULL_FACE);

	//entityManager.draw();

	glEnable(GL_CULL_FACE);
}
*/



/*
void OpenGLFixedRenderer::draw(Font* font,char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4)
{
	//https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_01

	if(!font || !phrase)
		return;

	ShaderInterface* shader=font_shader;

	if(!shader)
		return;

	shader->Use();

	GLuint tid;

	glEnable(GL_BLEND);glCheckError();
	glBlendFunc(GL_CONSTANT_COLOR, GL_ONE_MINUS_SRC_COLOR);

	if(color4)
		glBlendColor(color4[0],color4[1],color4[2],color4[3]);
	else
		glBlendColor(1,1,1,1);

	glActiveTexture(GL_TEXTURE0);glCheckError();
	glGenTextures(1,(GLuint*)&tid);glCheckError();
	glBindTexture(GL_TEXTURE_2D,tid);glCheckError();

	float sx=2.0f/width;
	float sy=2.0f/height;

	char* p=phrase;

	while(*p)
	{
		int err=font->loadchar(*p,sizex,sizey,width,height);

		if(!err)
		{
			unsigned char*	_buf=(unsigned char*)font->GetBuffer();
			int				_width=font->GetWidth();
			int				_rows=font->GetRows();
			float			_left=(float)font->GetLeft();
			float			_top=(float)font->GetTop();
			int				_advx=font->GetAdvanceX();
			int				_advy=font->GetAdvanceY();

			float			x2 = x + _left * sx;
			float			y2 = -y - _top * sy;
			float			w = _width * sx;
			float			h = _rows * sy;

			float box[] = {x2,-y2,0,1,x2 + w,-y2,0,1,x2,-y2-h,0,1,x2+w,-y2-h,0,1};
			float uv[] = {0,0,1,0,0,1,1,1};

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);glCheckError();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);glCheckError();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);glCheckError();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);glCheckError();

			glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,_width,_rows,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,_buf);glCheckError();

			glUniform1i(shader->GetTextureSlot(), 0);glCheckError();

			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			glEnableVertexAttribArray(shader->GetPositionSlot());glCheckError();
			glEnableVertexAttribArray(shader->GetTexcoordSlot());glCheckError();
			glVertexAttribPointer(shader->GetPositionSlot(), 4, GL_FLOAT, GL_FALSE,0,box);glCheckError();
			glVertexAttribPointer(shader->GetTexcoordSlot(), 2, GL_FLOAT, GL_FALSE,0,uv);glCheckError();
			glDrawArrays(GL_TRIANGLE_STRIP,0,4);glCheckError();
			glDisableVertexAttribArray(shader->GetPositionSlot());glCheckError();
			glDisableVertexAttribArray(shader->GetTexcoordSlot());glCheckError();

			x += (_advx >> 6) * sx;
			y += (_advy >> 6) * sy;
		}

		p++;
	}

	glDeleteTextures(1,(GLuint*)&tid);glCheckError();

	glDisable(GL_BLEND);glCheckError();
}*/


void Renderer3DAndroid::draw(Texture* _t)
{
	return;
	Shader* shader=0;//unlit_texture

	Texture* texture=(Texture*)_t;
	//TextureFile* texture=(TextureFile*)_t;


	if(!shader || !texture->GetBuffer())
		return;

	shader->Use();

	const GLfloat rect_data[] =
	{
		-1.0f, -1.0f,
		1.0f, -1.0f,
		-1.0f,  1.0f,
		1.0f,  1.0f
	};

	const GLfloat uv_data[] =
	{
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f,  1.0f,
		0.0f,  0.0f,
	};

	int width=texture->GetWidth();
	int height=texture->GetHeight();
	void* buf=texture->GetBuffer();
	int  size=texture->GetSize();

	if(width<100)
		return;

	GLuint tid;
	glGenTextures(1,&tid);glCheckError();

	glActiveTexture(GL_TEXTURE0);glCheckError();
	glBindTexture(GL_TEXTURE_2D,tid);glCheckError();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glCheckError();

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,width,height, 0, GL_RGBA, GL_UNSIGNED_BYTE,buf);glCheckError();

	mat4 mat;

	int projection=shader->GetProjectionSlot();
	int modelview=shader->GetModelviewSlot();
	int textureslot=shader->GetTextureSlot();
	int position=shader->GetPositionSlot();
	int texcoord=shader->GetTexcoordSlot();

	mat.ortho(-3,3,-3,3,0,1000);
	glUniformMatrix4fv(projection,1,0,mat);glCheckError();
	mat.identity();
	glUniformMatrix4fv(modelview,1,0,mat);glCheckError();

	glUniform1i(textureslot, 0);glCheckError();
	glEnableVertexAttribArray(position);glCheckError();
	glEnableVertexAttribArray(texcoord);glCheckError();
	glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE,0,rect_data);glCheckError();
	glVertexAttribPointer(texcoord, 2, GL_FLOAT, GL_FALSE,0,uv_data);glCheckError();
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);glCheckError();

	glDisableVertexAttribArray(position);glCheckError();
	glDisableVertexAttribArray(texcoord);glCheckError();

	glDeleteTextures(1,&tid);glCheckError();
}

void Renderer3DAndroid::draw(Mesh* mesh,std::vector<GLuint>& textureIndices,int texture_slot,int texcoord_slot)
{
	for(int i=0;i<(int)mesh->materials.size();i++)
	{
		for(int j=0;j<(int)mesh->materials[i]->textures.size() && !textureIndices.size();j++)
		{
			Texture* texture=mesh->materials[i]->textures[j];

			int		texture_width=texture->GetWidth();
			int		texture_height=texture->GetHeight();
			void	*texture_buffer=texture->GetBuffer();

			if(texture_buffer)
			{
				GLuint tid;
				glGenTextures(1,&tid);glCheckError();
				textureIndices.push_back(tid);

				glActiveTexture(GL_TEXTURE0);glCheckError();
				glBindTexture(GL_TEXTURE_2D,tid);glCheckError();

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);glCheckError();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);glCheckError();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);glCheckError();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glCheckError();

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,texture_width,texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE,texture_buffer);glCheckError();

				glBindBuffer(GL_ARRAY_BUFFER,textureBufferObject);
				glBufferData(GL_ARRAY_BUFFER,mesh->ntexcoord*2*sizeof(float),mesh->texcoord,GL_DYNAMIC_DRAW);

				glUniform1i(texture_slot, 0);glCheckError();
				glEnableVertexAttribArray(texcoord_slot);glCheckError();
				glVertexAttribPointer(texcoord_slot,2,GL_FLOAT,GL_FALSE,0,0);glCheckError();

				glBindBuffer(GL_ARRAY_BUFFER,0);
			}
		}
	}
}

void Renderer3DAndroid::draw(Mesh* mesh)
{
	drawUnlitTextured(mesh);
}

void Renderer3DAndroid::drawUnlitTextured(Mesh* mesh)
{
	Shader* shader = mesh->materials.size() ? this->unlit_texture : this->unlit_color;

	if(!shader || !mesh)
		return;

	vec3 lightpos(0,200,-100);

	if(shader==this->shaded_texture)
		this->draw(lightpos,5);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glFrontFace(mesh->isCCW ? GL_CCW : GL_CW);
	//glFrontFace(GL_CCW);

	shader->Use();

	shader->SetMatrices(MatrixStack::GetViewMatrix()*MatrixStack::GetProjectionMatrix(),mesh->entity->world);


	//shader->SetSelectionColor(this->picking,mesh->entity,vec2(this->tabContainer->mousex/this->tabContainerWin32->windowData->width,this->tabContainer->mousey/this->tabContainerWin32->windowData->height));

	int position_slot = shader->GetPositionSlot();
	int texcoord_slot = shader->GetTexcoordSlot();
	int texture_slot = shader->GetTextureSlot();
	int normal_slot = shader->GetNormalSlot();

	std::vector<unsigned int> textureIndices;

	if(mesh->materials.size())
	{
		draw(mesh,textureIndices,texture_slot,texcoord_slot);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	/*else
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);*/

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER,mesh->ncontrolpoints*3*sizeof(float),mesh->controlpoints,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(position_slot);glCheckError();
	if(normal_slot>=0)glEnableVertexAttribArray(normal_slot);glCheckError();
	glVertexAttribPointer(position_slot,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();
	if(normal_slot>=0)glVertexAttribPointer(normal_slot,3,GL_FLOAT,GL_FALSE,0,mesh->normals);glCheckError();

	glBindBuffer(GL_ARRAY_BUFFER,0);

	glDrawArrays(GL_TRIANGLES,0,mesh->npolygons*3);glCheckError();
	//glDrawElements(GL_TRIANGLES,mesh->mesh_nvertexindices*3,GL_UNSIGNED_INT,mesh->mesh_vertexindices);glCheckError();


	glDisableVertexAttribArray(position_slot);glCheckError();
	if(normal_slot>=0)glDisableVertexAttribArray(normal_slot);glCheckError();

	for(int i=textureIndices.size();i>0;i--)
	{
		glDeleteTextures(1,&textureIndices[i-1]);glCheckError();
		glDisableVertexAttribArray(texcoord_slot);glCheckError();
	}

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}


void Renderer3DAndroid::draw(Skin* skin)
{
	Shader* shader = skin->materials.size() ? this->unlit_texture : this->unlit_color;

	if(!skin || !skin->vertexcache || !shader)
	{
		this->drawUnlitTextured(skin);
		return;
	}

	vec3 lightpos(0,200,-100);

	if(shader==this->shaded_texture)
		this->draw(lightpos,5);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glFrontFace(skin->isCCW ? GL_CCW : GL_CW);
	//glFrontFace(GL_CW);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	shader->Use();

	shader->SetMatrices(MatrixStack::GetViewMatrix()*MatrixStack::GetProjectionMatrix(),skin->entity->local);


	//shader->SetSelectionColor(this->picking,skin->entity,vec2(this->tabContainer->mousex/this->tabContainerWin32->windowData->width,this->tabContainer->mousey/this->tabContainerWin32->windowData->height));

	int position_slot = shader->GetPositionSlot();
	int texcoord_slot = shader->GetTexcoordSlot();
	int texture_slot = shader->GetTextureSlot();
	int lightpos_slot = shader->GetLightposSlot();
	int lightdiff_slot = shader->GetLightdiffSlot();
	int lightamb_slot = shader->GetLightambSlot();
	int normal_slot = shader->GetNormalSlot();
	int color_slot = shader->GetColorSlot();

	std::vector<GLuint> textureIndices;

	draw(skin,textureIndices,texture_slot,texcoord_slot);

	int uniformTextured=shader->GetUniform("textured");

	glUniform1f(uniformTextured,(GLfloat)textureIndices.size());glCheckError();

	if(lightdiff_slot>=0)glEnableVertexAttribArray(lightdiff_slot);glCheckError();
	if(lightamb_slot>=0)glEnableVertexAttribArray(lightamb_slot);glCheckError();
	if(lightpos_slot>=0)glEnableVertexAttribArray(lightpos_slot);glCheckError();

	float v[3]={255,255,255};

	if(lightdiff_slot>=0)glUniform3f(lightdiff_slot,v[0],v[1],v[2]);glCheckError();
	if(lightamb_slot>=0)glUniform3f(lightamb_slot,v[0]+128,v[1]+128,v[2]+255);glCheckError();
	if(lightpos_slot>=0)glUniform3fv(lightpos_slot,1,lightpos);glCheckError();

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER,skin->ncontrolpoints*3*sizeof(float),skin->vertexcache,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(position_slot);glCheckError();
	if(normal_slot>=0)glEnableVertexAttribArray(normal_slot);glCheckError();
	glVertexAttribPointer(position_slot,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();
	if(normal_slot>=0)glVertexAttribPointer(normal_slot,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();

	glDrawArrays(GL_TRIANGLES,0,skin->npolygons*3);glCheckError();
	//glDrawElements(GL_TRIANGLES,skin->mesh_ntriangleindices*3,GL_UNSIGNED_INT,skin->mesh_triangleindices);glCheckError();

	if(lightdiff_slot>=0)glDisableVertexAttribArray(position_slot);glCheckError();
	if(lightdiff_slot>=0)glDisableVertexAttribArray(normal_slot);glCheckError();

	if(lightdiff_slot>=0)glDisableVertexAttribArray(lightdiff_slot);glCheckError();
	if(lightamb_slot>=0)glDisableVertexAttribArray(lightamb_slot);glCheckError();
	if(lightpos_slot>=0)glDisableVertexAttribArray(lightpos_slot);glCheckError();

	for(int i=textureIndices.size();i>0;i--)
	{
		glDeleteTextures(1,&textureIndices[i-1]);glCheckError();
		glDisableVertexAttribArray(texcoord_slot);glCheckError();
	}

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}

void Renderer3DAndroid::draw(Camera*)
{

}

void Renderer3DAndroid::draw(Bone* bone)
{
	Shader* shader=this->unlit_color;

	if(!shader)
		return;

	vec3 a=bone->entity->parent->world.position();
	vec3 b=bone->entity->world.position();

	float line[]=
	{
		a[0],a[1],a[2],
		b[0],b[1],b[2],
	};

	shader->Use();

	shader->SetMatrices(MatrixStack::GetViewMatrix()*MatrixStack::GetProjectionMatrix(),mat4());

	//shader->SetSelectionColor(this->picking,bone->entity,vec2(this->tabContainer->mousex/this->tabContainerWin32->windowData->width,this->tabContainer->mousey/this->tabContainerWin32->windowData->height));

	glEnable(GL_DEPTH_TEST);

	int pos=shader->GetPositionSlot();
	int col=shader->GetUniform("color");

	glUniform3fv(col,1,bone->color);

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER,6*sizeof(float),line,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(pos);glCheckError();

	glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();

	glDrawArrays(GL_LINES,0,2);glCheckError();
	//glDrawElements(GL_TRIANGLES,3,GL_UNSIGNED_INT,)

	glDisableVertexAttribArray(pos);

	glDisable(GL_DEPTH_TEST);
}

float signof(float num){return (num>0 ? 1.0f : (num<0 ? -1.0f : 0.0f));}


/*

void Renderer3DAndroid::Render(GuiViewport* viewport,bool force)
{
	vec4 canvas=viewport->rect;
	vec2 mouse(tabContainerWin32->mousex,tabContainerWin32->mousey);

	ID2D1Bitmap*& rBitmap=(ID2D1Bitmap*&)viewport->renderBitmap;
	unsigned char*& rBuffer=(unsigned char*&)viewport->renderBuffer;


	if(!rBitmap || rBitmap->GetSize().width!=canvas.z || rBitmap->GetSize().height!=canvas.w || !viewport->renderBuffer || !viewport->renderBitmap)
	{
		SAFERELEASE(rBitmap);
		SAFEDELETEARRAY(viewport->renderBuffer);

		D2D1_BITMAP_PROPERTIES bp=D2D1::BitmapProperties();
		bp.pixelFormat=tabContainerWin32->renderer2DWin32->renderer->GetPixelFormat();

		tabContainerWin32->renderer2DWin32->renderer->CreateBitmap(D2D1::SizeU((unsigned int)canvas.z,(unsigned int)canvas.w),bp,&rBitmap);

		rBuffer=new unsigned char[(int)canvas.z*canvas.w*4];
	}

	if(viewport->rootEntity)
	{
		viewport->rootEntity->world=viewport->model;

		viewport->rootEntity->update();

		for(std::vector<GuiEntityViewer*>::iterator it=GuiEntityViewer::pool.begin();it!=GuiEntityViewer::pool.end();it++)
		{
			EditorEntity* eEntity=(EditorEntity*)(*it)->entity;
			if(eEntity && (*it)->tabContainer)
				eEntity->OnPropertiesUpdate((*it)->tabContainer);
		}
	}

	tabContainerWin32->renderer3D->ChangeContext();

	glViewport((int)0,(int)0,(int)canvas.z,(int)canvas.w);glCheckError();
	glScissor((int)0,(int)0,(int)canvas.z,(int)canvas.w);glCheckError();

	glEnable(GL_DEPTH_TEST);

	{
		glClearColor(0.43f,0.43f,0.43f,0.0f);glCheckError();
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glCheckError();

		MatrixStack::Push(MatrixStack::PROJECTION,viewport->projection);
		MatrixStack::Push(MatrixStack::VIEW,viewport->view);
		MatrixStack::Push(MatrixStack::MODEL,viewport->model);

		tabContainer->renderer3D->draw(vec3(0,0,0),vec3(1000,0,0),vec3(1,0,0));
		tabContainer->renderer3D->draw(vec3(0,0,0),vec3(0,1000,0),vec3(0,1,0));
		tabContainer->renderer3D->draw(vec3(0,0,0),vec3(0,0,1000),vec3(0,0,1));

		if(viewport->rootEntity)
			this->draw(viewport->rootEntity);

		MatrixStack::Pop(MatrixStack::MODEL);
		MatrixStack::Pop(MatrixStack::VIEW);
		MatrixStack::Pop(MatrixStack::PROJECTION);

		glReadBuffer(GL_BACK);glCheckError();
		glReadPixels((int)0,(int)0,(int)canvas.z,(int)canvas.w,GL_BGRA,GL_UNSIGNED_BYTE,rBuffer);glCheckError();//@mic should implement pbo for performance

		rBitmap->CopyFromMemory(&D2D1::RectU(0,0,(int)canvas.z,(int)canvas.w),rBuffer,(int)(canvas.z*4));

		tabContainerWin32->renderer2DWin32->renderer->DrawBitmap(rBitmap,D2D1::RectF(canvas.x,canvas.y,canvas.x+canvas.z,canvas.y+canvas.w));
	}

	if(viewport->needsPicking && mouse.y-canvas.y>=0)
	{
		glDisable(GL_DITHER);

		tabContainer->renderer3D->picking=true;

		glClearColor(0.0f,0.0f,0.0f,0.0f);glCheckError();
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glCheckError();

		MatrixStack::Push(MatrixStack::PROJECTION,viewport->projection);
		MatrixStack::Push(MatrixStack::VIEW,viewport->view);
		MatrixStack::Push(MatrixStack::MODEL,viewport->model);

		if(viewport->rootEntity)
			this->draw(viewport->rootEntity);

		MatrixStack::Pop(MatrixStack::MODEL);
		MatrixStack::Pop(MatrixStack::VIEW);
		MatrixStack::Pop(MatrixStack::PROJECTION);

		glReadBuffer(GL_BACK);glCheckError();

		viewport->pickedPixel;
		glReadPixels((int)mouse.x,(int)mouse.y-canvas.y,(int)1,(int)1,GL_RGBA,GL_UNSIGNED_BYTE,&viewport->pickedPixel);glCheckError();//@mic should implement pbo for performance

		unsigned int address=0;

		unsigned char* ptrPixel=(unsigned char*)&viewport->pickedPixel;
		unsigned char* ptrAddress=(unsigned char*)&address;

		{
			viewport->pickedEntity=viewport->pickedPixel ?

				ptrAddress[0]=ptrPixel[3],
				ptrAddress[1]=ptrPixel[2],
				ptrAddress[2]=ptrPixel[1],
				ptrAddress[3]=ptrPixel[0],

				dynamic_cast<EditorEntity*>((EditorEntity*)address)

				: 0 ;
		}

		tabContainer->renderer3D->picking=false;

		glEnable(GL_DITHER);

		viewport->needsPicking=false;
	}
}
*/



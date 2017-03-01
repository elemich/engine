#include "win32.h"

extern App* ___app;

bool GLEW_INITED=false;


void glCheckError()
{
	GLenum err=glGetError();
	if(err!=GL_NO_ERROR)
	{
		printf("glErr %d",err);
		__debugbreak();
	}
}


int simple_shader(int shader_type, const char* shader_src)
{
	GLint compile_success = 0;
	GLchar message[1024];
	int len=0;
	int shader_id;

	shader_id = glCreateShader(shader_type);glCheckError();

	if(!shader_id)
	{
		printf("glCreateShader error for %s,%s\n",shader_type,shader_src);glCheckError();
		__debugbreak();
		return 0;
	}

	glShaderSource(shader_id, 1, &shader_src,NULL);glCheckError();
	glCompileShader(shader_id);glCheckError();
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_success);glCheckError();

	if (GL_FALSE==compile_success)
	{
		printf( "\nglCompileShader error for %s\n",shader_src);
		glGetShaderInfoLog(shader_id, sizeof(message), &len, message);
		__debugbreak();
	}

	glGetShaderInfoLog(shader_id, sizeof(message), &len, message);

	if(len && len<sizeof(message))
	{
		char shaderName[100];

		switch(shader_type)
		{
		case GL_VERTEX_SHADER:
			sprintf_s(shaderName,"%s","GL_VERTEX_SHADER");
			break;
		case GL_FRAGMENT_SHADER:
			sprintf_s(shaderName,"%s","GL_FRAGMENT_SHADER");
			break;
		}

		char* s=strrchr(message,'\n');
		if(s)
			*s=0;
		printf("%s:%s",shaderName,message);
	}

	return shader_id;
}


int create_program(const char* vertexsh,const char* fragmentsh)
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

	vertex_shader=simple_shader(GL_VERTEX_SHADER, vertexsh);
	fragment_shader=simple_shader(GL_FRAGMENT_SHADER, fragmentsh);

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

	if(len && len<sizeof(message))
		printf("%s",message);
	

	return program;
}


//--------------------OpenGLRenderer-------------------------


ShaderInterface* OpenGLShader::Create(const char* name,const char* pix,const char* frag)
{
	int program=create_program(pix,frag);

	if(program)
	{
		ShaderInterface* shader=new OpenGLShader();
		shader->SetName(name);
		shader->SetProgram(program);

		glUseProgram(program);glCheckError();

		shader->init();
		shader->Use();
		
		ShaderInterface::shaders.Push(shader);

		printf("adding %s to shaders list\n",name);

		return shader;
	}
	else

		printf("error creating shader %s\n",name);

	return 0;
}



unsigned int& OpenGLShader::GetBufferObject()
{
	return vbo;
}

int OpenGLShader::GetProgram(){return program;}
void OpenGLShader::SetProgram(int p){program=p;}

int OpenGLShader::GetUniform(int slot,char* var)
{
	return glGetUniformLocation(slot,var);glCheckError();
}
int OpenGLShader::GetAttrib(int slot,char* var)
{
	return glGetAttribLocation(slot,var);glCheckError();
}

void OpenGLShader::SetProjectionMatrix(float* pm)
{
	if(!pm)
		return;

	this->SetMatrix4f(this->GetProjectionSlot(),pm);

}
void OpenGLShader::SetModelviewMatrix(float* mm)
{
	if(!mm)
		return;

	this->SetMatrix4f(this->GetModelviewSlot(),mm);
}

void OpenGLShader::SetByMatrixStack()
{
	this->SetProjectionMatrix(MatrixStack::GetProjectionMatrix());
	this->SetModelviewMatrix(MatrixStack::GetModelviewMatrix());
}

void OpenGLShader::Use()
{
	ShaderInterface* current_shader=ShaderInterface::GetCurrent();

	if(!program || current_shader==this)
		return;
	
	glUseProgram(program);glCheckError();

	this->SetByMatrixStack();
	
	ShaderInterface::SetCurrent(this);
}

const char* OpenGLShader::GetPixelShader(){return 0;}
const char* OpenGLShader::GetFragmentShader(){return 0;}

int OpenGLShader::init()
{
	mat4 m;

	int proj=GetProjectionSlot();
	int mdlv=GetModelviewSlot();

	bool bOk = this->SetMatrix4f(proj,m) && this->SetMatrix4f(mdlv,m);

	return bOk;
}

int OpenGLShader::GetAttribute(const char* attrib)
{
	int location=glGetAttribLocation(program,attrib);glCheckError();
	return location;
}

int OpenGLShader::GetUniform(const char* uniform)
{
	int location=glGetUniformLocation(program,uniform);glCheckError();
	return location;
}

int OpenGLShader::GetPositionSlot()
{
	return GetAttribute("position");
}
int OpenGLShader::GetColorSlot()
{
	return GetAttribute("color");
}
int OpenGLShader::GetProjectionSlot()
{
	return GetUniform("projection");
}
int OpenGLShader::GetModelviewSlot()
{
	return GetUniform("modelview");
}
int OpenGLShader::GetTexcoordSlot()
{
	return GetAttribute("texcoord");
}
int OpenGLShader::GetTextureSlot()
{
	return GetUniform("texture");
}
int OpenGLShader::GetLightposSlot()
{
	return GetUniform("lightpos");
}
int OpenGLShader::GetLightdiffSlot()
{
	return GetUniform("lightdiff");
}
int OpenGLShader::GetLightambSlot()
{
	return GetUniform("lightamb");
}
int OpenGLShader::GetNormalSlot()
{
	return GetAttribute("normal");
}

bool OpenGLShader::SetMatrix4f(int slot,float* mtx)
{
	if(slot<0 || !mtx)
		return false;

	glUniformMatrix4fv(slot,1,0,mtx);glCheckError();
	return true;
}

void OpenGLShader::SetName(const char* n)
{
	name=n;
}
const char* OpenGLShader::GetName()
{
	return name;
}




//--------------------OpenGLRenderer-------------------------

LRESULT CALLBACK OpenGLProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	OpenGLFixedRenderer* renderer=(OpenGLFixedRenderer*)GetWindowLongPtr(hwnd,GWL_USERDATA);

	LRESULT result=0;

	switch(msg)
	{
	case WM_ERASEBKGND:
		return (LRESULT)1;
	default:
		result=DefWindowProc(hwnd,msg,wparam,lparam);
	}

	if(renderer)
		renderer->Render();

	return result;
}


OpenGLFixedRenderer::OpenGLFixedRenderer()
{
	hglrc=0;
}

char* OpenGLFixedRenderer::Name()
{
	return 0;
}

void OpenGLFixedRenderer::Create(HWND container)
{
	hwnd=CreateWindow(WC_OPENGLWINDOW,"OpenGLFixedRenderer",WS_CHILD,CW_USEDEFAULT,CW_USEDEFAULT,100,100,container,0,0,0);

	hdc=GetDC(hwnd);

	if(!hdc)
		MessageBox(0,"Getting Device Context","GetDC",MB_OK|MB_ICONEXCLAMATION);

	memset(&pfd,0,sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize=sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion=1;
	pfd.dwFlags=PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
	pfd.iPixelType=PFD_TYPE_RGBA;
	pfd.cColorBits=32;
	pfd.cDepthBits=32;
	pfd.iLayerType=PFD_MAIN_PLANE;

	pixelFormat = ChoosePixelFormat(hdc,&pfd);

	if(pixelFormat==0)
		MessageBox(0,"pixel format error","ChoosePixelFormat",MB_OK|MB_ICONEXCLAMATION);

	if(!SetPixelFormat(hdc,pixelFormat,&pfd))
		MessageBox(0,"pixel format error","DescribePixelFormat",MB_OK|MB_ICONEXCLAMATION);

	hglrc = wglCreateContext(hdc);

	if(!hglrc)
		MessageBox(0,"creating context error","wglCreateContext",MB_OK|MB_ICONEXCLAMATION);

	if(!wglMakeCurrent(hdc,hglrc))
		MessageBox(0,"error making current context","wglMakeCurrent",MB_OK|MB_ICONEXCLAMATION);

	glCheckError();

	ReleaseDC(hwnd,hdc);

	if(!GLEW_INITED)
	{
		GLenum err = glewInit();glCheckError();
		if (GLEW_OK != err)
		{
			/* Problem: glewInit failed, something is seriously wrong. */
			printf("Error: %s\n", glewGetErrorString(err));
			__debugbreak();
		}

		GLEW_INITED=true;
	}
	
	printf("Status: Using GL %s\n", glGetString(GL_VERSION));
	printf("Status: GLSL ver %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	SetWindowLongPtr(hwnd,GWL_USERDATA,(LONG_PTR)this);

	renderers.push_back(this);
}

int processNode(RendererInterface* renderer,TDLAutoNode<Entity*>* node,int type,float time)
{
	int retValue=0;

#if PROCESS_ENTITIES_RECURSIVELY
	if(node)
#else
	for(;node;node=node->next)
#endif
	{
		switch(type)
		{
		case 0:
			retValue+=node->data->animate(time);
			break;
		case 1:
			node->data->update();
			break;
		case 2:
			node->data->draw(renderer);
			break;
		}

#if PROCESS_ENTITIES_RECURSIVELY
		retValue+=processNode(node->data->entity_childs.Head(),type,time);
#endif
	}

	return retValue;
}

void OpenGLFixedRenderer::Render()
{
	if(!hglrc)
		return;

	hdc=GetDC(hwnd);

	wglMakeCurrent(hdc,hglrc);glCheckError();
	
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);glCheckError();
	glClearColor(0,0,0,0);glCheckError();

	RECT rc;
	GetClientRect(hwnd,&rc);
	glViewport(rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top);glCheckError();

	draw(vec3(0,0,0),15);
	draw(vec3(0,0,0),vec3(1000,0,0));

	processNode(this,Entity::entities.Head(),0,0);
	processNode(this,Entity::entities.Head(),1,0);
	processNode(this,Entity::entities.Head(),2,0);

	SwapBuffers(hdc);
	ReleaseDC(hwnd,hdc);
}




////-------------------------------------------------------------------


void OpenGLFixedRenderer::draw(Light*)
{

}

void OpenGLFixedRenderer::draw(vec2)
{

}

void OpenGLFixedRenderer::draw(vec3 point,float psize,vec3 col)
{
	ShaderInterface* shader=ShaderInterface::Find("unlit_color");

	if(!shader)
		return;

	shader->Use();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glPointSize(psize);

	int ps=shader->GetPositionSlot();glCheckError();
	int uniform_color=shader->GetUniform("color");glCheckError();

	if(uniform_color>=0)
		glUniform3fv(uniform_color,1,col);

	glEnableVertexAttribArray(ps);glCheckError();
	glVertexAttribPointer(ps, 3, GL_FLOAT, GL_FALSE, 0,point.v);glCheckError();
	
	glDrawArrays(GL_POINTS,0,1);glCheckError();

	glDisableVertexAttribArray(ps);glCheckError();

	glPointSize(1.0f);
	glDisable(GL_PROGRAM_POINT_SIZE);
	glDisable(GL_DEPTH_TEST);
}

void OpenGLFixedRenderer::draw(vec4 rect)
{
	ShaderInterface* shader=ShaderInterface::Find("unlit_color");

	int position_slot=-1;
	int modelview_slot=-1;

	if(shader)
	{
		shader->Use();
		position_slot=shader->GetPositionSlot();
		modelview_slot=shader->GetModelviewSlot();
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

/*
void OpenGLFixedRenderer::drawmarker(mat4 &mtx,float size,vec3 color)
{
	MatrixStack::Instance()->Push();

	MatrixStack::Instance()->Multiply(mtx);

	float ms=size/2;

	vec3 a(-ms,0,0),b(ms/2,0,0),g(ms/2,0,0),l(ms,0,0);
	vec3 c(0,-ms,0),d(0,ms/2,0),h(0,ms/2,0),m(0,ms,0);
	vec3 e(0,0,-ms),f(0,0,ms/2),i(0,0,ms/2),n(0,0,ms);

	OpenGLFixedRenderer::Instance()->draw(a,b,color);
	OpenGLFixedRenderer::Instance()->draw(c,d,color);
	OpenGLFixedRenderer::Instance()->draw(e,f,color);
	OpenGLFixedRenderer::Instance()->draw(g,l,vec3(.3f,0,0));
	OpenGLFixedRenderer::Instance()->draw(h,m,vec3(0,.3f,0));
	OpenGLFixedRenderer::Instance()->draw(i,n,vec3(0,0,.7f));

	MatrixStack::Instance()->Pop();
}*/



void OpenGLFixedRenderer::draw(vec3 a,vec3 b,vec3 color)
{
	ShaderInterface* shader=ShaderInterface::Find("unlit_color");

	if(!shader)
		return;

	float line[]=
	{
		a[0],a[1],a[2],1,
		b[0],b[1],b[2],1 
	};
	
	shader->Use();

	glEnable(GL_DEPTH_TEST);

	int pos=shader->GetPositionSlot();
	int col=shader->GetUniform("color");

	glUniform3fv(col,1,color);
	
	glEnableVertexAttribArray(pos);glCheckError();

	glVertexAttribPointer(pos,4,GL_FLOAT,GL_FALSE,0,line);glCheckError();

	glDrawArrays(GL_LINES,0,2);glCheckError();
	glDisableVertexAttribArray(pos);

	glDisable(GL_DEPTH_TEST);

}

void OpenGLFixedRenderer::draw(char* text,float x,float y,float width,float height,float sizex,float sizey,float* color4)
{
	ShaderInterface* shader=0;//line_color_shader

	if(!shader || !text)
		return;

	shader->Use();

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


void OpenGLFixedRenderer::draw(Texture* _t)
{
	return;
	ShaderInterface* shader=0;//unlit_texture

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

void OpenGLFixedRenderer::draw(Mesh* mesh,std::vector<GLuint>& textureIndices,int texture_slot,int texcoord_slot)
{
	for(int i=0;i<mesh->mesh_materials.Count();i++)
	{
		for(int j=0;j<mesh->mesh_materials[i]->textures.Count() && !textureIndices.size();j++)
		{
			switch(mesh->mesh_materials[i]->textures[j]->texture_type)
			{
			case TEXTURE_FILE:
				{
					TextureFile* texture=(TextureFile*)mesh->mesh_materials[i]->textures[j];


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

						glUniform1i(texture_slot, 0);glCheckError();
						glEnableVertexAttribArray(texcoord_slot);glCheckError();
						glVertexAttribPointer(texcoord_slot,2,GL_FLOAT,GL_FALSE,0,mesh->mesh_texcoord);glCheckError();
					}
				}
				break;
			case TEXTURE_LAYERED:
				{
					TextureLayered* texturelayered=(TextureLayered*)mesh->mesh_materials[i]->textures[j];

					for(int i=0;i<texturelayered->textures.Count();i++)
					{
						if(texturelayered->textures[i]->texture_type!=TEXTURE_FILE)
							continue;

						TextureFile* texture=(TextureFile*)texturelayered->textures[i];

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

							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);glCheckError();
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);glCheckError();
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);glCheckError();
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glCheckError();

							glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,texture_width,texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE,texture_buffer);glCheckError();

							glUniform1i(texture_slot, 0);glCheckError();
							glEnableVertexAttribArray(texcoord_slot);glCheckError();
							glVertexAttribPointer(texcoord_slot,2,GL_FLOAT,GL_FALSE,0,mesh->mesh_texcoord);glCheckError();
						}

					}
				}
				break;
			}
		}
	}
}

void OpenGLFixedRenderer::draw(Mesh* mesh)
{
	drawUnlitTextured(mesh);
}

void OpenGLFixedRenderer::drawUnlitTextured(Mesh* mesh)
{
	ShaderInterface* shader=ShaderInterface::Find("unlit_texture");

	if(!shader || !mesh)
		return;

	vec3 lightpos(0,200,-100);
	this->draw(lightpos,5);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glFrontFace(mesh->mesh_isCCW ? GL_CCW : GL_CW);
	//glFrontFace(GL_CW);

	

	shader->Use();

	int position_slot = shader->GetPositionSlot();
	int model_slot = shader->GetModelviewSlot();
	int projection_slot = shader->GetProjectionSlot();
	int texcoord_slot = shader->GetTexcoordSlot();
	int texture_slot = shader->GetTextureSlot();
	int normal_slot = shader->GetNormalSlot();

	std::vector<unsigned int> textureIndices;

	draw(mesh,textureIndices,texture_slot,texcoord_slot);

	if(!textureIndices.size())
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnableVertexAttribArray(position_slot);glCheckError();
	if(normal_slot>=0)glEnableVertexAttribArray(normal_slot);glCheckError();
	glVertexAttribPointer(position_slot,3,GL_FLOAT,GL_FALSE,0,mesh->mesh_controlpoints);glCheckError();
	if(normal_slot>=0)glVertexAttribPointer(normal_slot,3,GL_FLOAT,GL_FALSE,0,mesh->mesh_normals);glCheckError();

	glDrawArrays(GL_TRIANGLES,0,mesh->mesh_npolygons*3);glCheckError();
	//glDrawElements(GL_TRIANGLES,mesh->mesh_nvertexindices*3,GL_UNSIGNED_INT,mesh->mesh_vertexindices);glCheckError();


	glDisableVertexAttribArray(position_slot);glCheckError();
	if(normal_slot>=0)glDisableVertexAttribArray(normal_slot);glCheckError();

	for(int i=textureIndices.size();i>0;i--)
	{
		glDeleteTextures(1,&textureIndices[i-1]);glCheckError();
		glDisableVertexAttribArray(texcoord_slot);glCheckError();
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}


void OpenGLFixedRenderer::draw(Skin *skin)
{
	/*this->draw((Mesh*)skin);
	return;*/

	ShaderInterface* shader=ShaderInterface::Find("unlit_texture");

	if(!skin || !skin->skin_vertexcache || !shader)
	{
		this->drawUnlitTextured(skin);
		return;
	}

	vec3 lightpos(0,200,-100);
	this->draw(lightpos,10.0f);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glFrontFace(skin->mesh_isCCW ? GL_CCW : GL_CW);
	//glFrontFace(GL_CW);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	shader->Use();

	int position_slot = shader->GetPositionSlot();
	int model_slot = shader->GetModelviewSlot();
	int projection_slot = shader->GetProjectionSlot();
	int texcoord_slot = shader->GetTexcoordSlot();
	int texture_slot = shader->GetTextureSlot();
	int lightpos_slot = shader->GetLightposSlot();
	int lightdiff_slot = shader->GetLightdiffSlot();
	int lightamb_slot = shader->GetLightambSlot();
	int normal_slot = shader->GetNormalSlot();
	int color_slot = shader->GetColorSlot();

	

	std::vector<GLuint> textureIndices;

	draw(skin,textureIndices,texture_slot,texcoord_slot);

	glUniform1f(shader->GetUniform("textured"),(GLfloat)textureIndices.size());glCheckError();

	if(lightdiff_slot>=0)glEnableVertexAttribArray(lightdiff_slot);glCheckError();
	if(lightamb_slot>=0)glEnableVertexAttribArray(lightamb_slot);glCheckError();
	if(lightpos_slot>=0)glEnableVertexAttribArray(lightpos_slot);glCheckError();

	float v[3]={255,255,255};

	if(lightdiff_slot>=0)glUniform3f(lightdiff_slot,v[0],v[1],v[2]);glCheckError();
	if(lightamb_slot>=0)glUniform3f(lightamb_slot,v[0]+128,v[1]+128,v[2]+255);glCheckError();
	if(lightpos_slot>=0)glUniform3fv(lightpos_slot,1,lightpos);glCheckError();
	

	glEnableVertexAttribArray(position_slot);glCheckError();
	if(normal_slot>=0)glEnableVertexAttribArray(normal_slot);glCheckError();
	glVertexAttribPointer(position_slot,3,GL_FLOAT,GL_FALSE,0,skin->skin_vertexcache);glCheckError();
	if(normal_slot>=0)glVertexAttribPointer(normal_slot,3,GL_FLOAT,GL_FALSE,0,skin->mesh_normals);glCheckError();

	glDrawArrays(GL_TRIANGLES,0,skin->mesh_npolygons*3);glCheckError();
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

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}


void OpenGLFixedRenderer::draw(Bone* bone)
{
	if(!bone)
		return;

	TDLAutoNode<Entity*>* nb=bone->entity_childs.Head();
	
	if(bone->entity_parent->IsA(ENTITY_BONE))
	{
		vec3 b2p=bone->entity_parent->entity_world.position();
		vec3 b1p=bone->entity_world.position();
		this->draw(b1p,b2p,bone->bone_color);
	}
}




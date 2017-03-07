#ifndef INTERFACES_H
#define INTERFACES_H

#include "primitives.h"

struct Interface
{
	static std::vector<Interface*> interfacesPool;

	virtual Interface* GetApp(){return 0;}
	virtual Interface* GetRenderer(){return 0;}
	virtual Interface* GetShader(){return 0;}
};

struct EntityInterface
{
	static std::vector<EntityInterface*> entityInterfacesPool;

	EntityInterface* GetBone(){return 0;}
	EntityInterface* GetMesh(){return 0;}
	EntityInterface* GetSkin(){return 0;}
	EntityInterface* GetLight(){return 0;}
};

struct ResourceInterface
{
	static std::vector<ResourceInterface*> resourceInterfacesPool;

	ResourceInterface* GetTexture(){return 0;}
	ResourceInterface* GetMaterial(){return 0;}
};



struct AppInterface : Interface
{
	virtual int Init()=0;
	virtual void AppLoop()=0;
	virtual void CreateMainWindow()=0;

	virtual AppInterface* GetApp(){return this;}
	
};

struct RendererInterface : Interface
{
	virtual RendererInterface* GetRenderer(){return this;}
	
	static std::vector<RendererInterface*> renderers;


	virtual char* Name()=0;
	virtual void Render()=0;

	virtual void draw(vec3,float psize=1.0f,vec3 color=vec3(1,1,1))=0;
	virtual void draw(vec2)=0;
	virtual void draw(vec3,vec3,vec3 color=vec3(1,1,1))=0;
	virtual void draw(vec4)=0;
	virtual void draw(AABB,vec3 color=vec3(1,1,1))=0;
	virtual void draw(mat4 mtx,float size,vec3 color=vec3(1,1,1))=0;
	//virtual void draw(Font*,char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4)=0;
	virtual void draw(char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4)=0;

	
	virtual void draw(Bone*)=0;
	virtual void draw(Mesh*)=0;
	virtual void draw(Skin*)=0;
	virtual void draw(Texture*)=0;
	virtual void draw(Light*)=0;
	virtual void drawUnlitTextured(Mesh*)=0;
	virtual void draw(Mesh*,std::vector<unsigned int>& textureIndices,int texture_slot,int texcoord_slot)=0;

	//virtual operator RendererInterface&()=0;

};

struct ShaderInterface;

struct ShadersPool
{
	static std::vector<ShaderInterface*> pool;

	static ShaderInterface* Find(const char*,bool exact=true);
	static void SetMatrices(float* proj,float* mdlv);
	static ShaderInterface* GetCurrent();
	static void SetCurrent(ShaderInterface*);
};


struct ShaderInterface : Interface
{
	static ShadersPool shadersPool;

	virtual ShaderInterface* GetShader(){return this;}

	virtual int GetProgram()=0;
	virtual void SetProgram(int)=0;

	virtual void SetName(const char*)=0;
	virtual const char* GetName()=0;

	virtual int GetUniform(int slot,char* var)=0;
	virtual int GetAttrib(int slot,char* var)=0;

	virtual void Use()=0;

	virtual const char* GetPixelShader()=0;
	virtual const char* GetFragmentShader()=0;

	virtual int GetAttribute(const char*)=0;
	virtual int GetUniform(const char*)=0;

	virtual int init()=0;

	virtual int GetPositionSlot()=0;
	virtual int GetColorSlot()=0;
	virtual int GetProjectionSlot()=0;
	virtual int GetModelviewSlot()=0;
	virtual int GetTexcoordSlot()=0;
	virtual int GetTextureSlot()=0;
	virtual int GetLightposSlot()=0;
	virtual int GetLightdiffSlot()=0;
	virtual int GetLightambSlot()=0;
	virtual int GetNormalSlot()=0;

	virtual bool SetMatrix4f(int slot,float* mtx)=0;

	virtual unsigned int& GetBufferObject()=0;

	virtual void SetProjectionMatrix(float*)=0;
	virtual void SetModelviewMatrix(float*)=0;

	virtual void SetByMatrixStack()=0;
};

struct InputInterface : Interface
{

};




#endif //INTERFACES_H



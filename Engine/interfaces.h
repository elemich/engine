#ifndef INTERFACES_H
#define INTERFACES_H

#include "primitives.h"

struct ObjectInterface
{

};

struct StringInterface : ObjectInterface
{

};

struct NumberInterface : ObjectInterface
{

};



struct AppInterface
{
	virtual int Init()=0;
	virtual void AppLoop()=0;
	virtual void CreateMainWindow()=0;
};

struct RendererInterface
{
	virtual char* Name()=0;
	virtual void Render()=0;
};

struct LoggerInterface
{

};

struct FolderBrowserInterface
{

};

struct SceneEntitiesInterface
{

};

struct PropertyInterface
{

};



struct ShaderInterface
{
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




#endif //INTERFACES_H



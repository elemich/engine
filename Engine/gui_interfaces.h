#ifndef GUI_INTERFACES_H
#define GUI_INTERFACES_H

#include "interfaces.h"

struct TabContainer;

struct Gui : TClassPool<Gui>
{
	static const unsigned int COLOR_GUI_BACKGROUND = 0x707070;
	
	String name;
	TabContainer* tab;

	Gui()
	{
		name="Generic Gui";
		tab=0;
	}

	~Gui(){};

	virtual void OnPaint(){}
	virtual void OnEntitiesChange(){}
	virtual void OnSize(){}
	virtual void OnLMouseDown(){}
	virtual void OnLMouseUp(){};
	virtual void OnMouseMove(){}
	virtual void OnRun(){}
	virtual void OnReparent(){this->OnSize();}
	virtual void OnSelected(Entity*){}
	virtual void OnRender(){}
	virtual void OnMouseWheel(){}

	virtual void RecreateTarget(){}

	bool IsSelected();

	static std::vector<Gui*>& GuiPool(){return pool;}
};

template<class T> struct GuiInterface : Gui , TClassPool<GuiInterface<T>>
{
	static std::vector<GuiInterface<T>*>& Pool(){return TClassPool<GuiInterface<T>>::pool;}
};


struct LoggerInterface : GuiInterface<LoggerInterface>
{
};

struct FolderBrowserInterface : GuiInterface<FolderBrowserInterface>
{
};

struct SceneEntitiesInterface : GuiInterface<SceneEntitiesInterface>
{
	virtual void OnEntitiesChange()=0;
};

struct PropertyInterface : GuiInterface<PropertyInterface>
{
};


struct RendererViewportInterface : GuiInterface<RendererViewportInterface> //should get the window handle from the WindowData class of the inherited class
{
	float RendererViewportInterface_viewScale;
	float RendererViewportInterface_farPlane;

	virtual void OnMouseWheel(float)=0;
	virtual void OnMouseRightDown()=0;
	virtual void OnViewportSize(int,int)=0;
	virtual void OnMouseMotion(int,int,bool leftButtonDown,bool altIsDown)=0;
	virtual void OnMouseDown(int,int)=0;
	virtual float GetProjectionHalfWidth()=0;
	virtual float GetProjectionHalfHeight()=0;

};




#endif //GUI_INTERFACES_H
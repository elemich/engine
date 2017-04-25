#ifndef GUI_INTERFACES_H
#define GUI_INTERFACES_H

#include "interfaces.h"

struct TabContainer;

struct Gui
{
	static const unsigned int COLOR_GUI_BACKGROUND = 0x707070;

	static std::vector<Gui*> guiPool;

	String name;
	TabContainer* tab;

	Gui();
	~Gui();

	virtual void OnPaint(){}
	virtual void OnEntitiesChange(){}
	virtual void OnSize(){}
	virtual void OnLMouseDown(){}
	virtual void OnRun(){}
	virtual void OnReparent(){this->OnSize();}
	virtual void OnSelected(Entity*){}

	virtual void RecreateTarget(){}

	virtual Gui* GetLogger(){return 0;}
	virtual Gui* GetFolderBrowser(){return 0;}
	virtual Gui* GetSceneEntities(){return 0;}
	virtual Gui* GetProperty(){return 0;}
	virtual Gui* GetRendererViewport(){return 0;}
};

/*
template <class T> struct GuiInterface : Gui
{
	static std::vector<GuiInterface*> guiInterfacePool;

	GuiInterface()
	{
		guiInterfacePool.push_back(this);	
	}

	~GuiInterface()
	{
		guiInterfacePool.erase(std::remove(guiInterfacePool.begin(),guiInterfacePool.end(),this),guiInterfacePool.end());
	}

	virtual GuiInterface* GetLogger(){return 0;}
	virtual GuiInterface* GetFolderBrowser(){return 0;}
	virtual GuiInterface* GetSceneEntities(){return 0;}
	virtual GuiInterface* GetProperty(){return 0;}
	virtual GuiInterface* GetRendererViewport(){return 0;}
};

template<class T> std::vector<GuiInterface<T>*> GuiInterface<T>::guiInterfacePool;*/


struct LoggerInterface : Gui
{
	virtual LoggerInterface* GetLogger(){return this;}
};

struct FolderBrowserInterface : Gui
{
	virtual FolderBrowserInterface* GetFolderBrowser(){return this;}
};

struct SceneEntitiesInterface : Gui
{
	SceneEntitiesInterface* GetSceneEntitiesInterface(){return this;}
	SceneEntitiesInterface* GetSceneEntities(){return 0;}

	virtual void OnEntitiesChange()=0;
};

struct PropertyInterface : Gui
{
	virtual PropertyInterface* GetProperty(){return this;}
};


struct RendererViewportInterface : Gui //should get the window handle from the WindowData class of the inherited class
{
	virtual RendererViewportInterface* GetRendererViewport(){return this;}


	float RendererViewportInterface_viewScale;
	float RendererViewportInterface_farPlane;

	virtual void OnMouseWheel(float)=0;
	virtual void OnMouseRightDown()=0;
	virtual void OnViewportSize(int,int)=0;
	virtual void OnMouseMotion(int,int,bool leftButtonDown,bool altIsDown)=0;
	virtual void OnMouseDown(int,int)=0;

	///virtual operator RendererViewportInterface&()=0;

	virtual RendererInterface* GetRenderer()=0;
};




#endif //GUI_INTERFACES_H
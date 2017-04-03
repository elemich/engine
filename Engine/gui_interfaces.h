#ifndef GUI_INTERFACES_H
#define GUI_INTERFACES_H

#include "interfaces.h"

struct TabContainer;

struct GuiInterface
{
	static std::vector<GuiInterface*> guiInterfacesPool;

	GuiInterface();
	~GuiInterface();

	String guiinterface_tabName;
	TabContainer* guiinterface_tabcontainer;

	virtual GuiInterface* GetLogger(){return 0;}
	virtual GuiInterface* GetFolderBrowser(){return 0;}
	virtual GuiInterface* GetSceneEntities(){return 0;}
	virtual GuiInterface* GetProperty(){return 0;}
	virtual GuiInterface* GetRendererViewport(){return 0;}


	virtual void OnPaint(){}
	virtual void OnEntitiesChange(){}
	virtual void OnSize(){}
	virtual void OnLMouseDown(){}
	virtual void OnRun(){}
	virtual void OnReparent(){this->OnSize();}

	virtual void RecreateTarget(){}
};



struct LoggerInterface : GuiInterface
{
	virtual LoggerInterface* GetLogger(){return this;}
};

struct FolderBrowserInterface : GuiInterface
{
	virtual FolderBrowserInterface* GetFolderBrowser(){return this;}
};

struct SceneEntitiesInterface : GuiInterface
{
	SceneEntitiesInterface* GetSceneEntitiesInterface(){return this;}
	SceneEntitiesInterface* GetSceneEntities(){return 0;}

	virtual void OnEntitiesChange()=0;
};

struct PropertyInterface : GuiInterface
{
	virtual PropertyInterface* GetProperty(){return this;}
};


struct RendererViewportInterface : GuiInterface //should get the window handle from the WindowData class of the inherited class
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
};




#endif //GUI_INTERFACES_H
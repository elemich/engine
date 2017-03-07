#ifndef GUI_INTERFACES_H
#define GUI_INTERFACES_H

#include "interfaces.h"

struct GuiInterface
{
	static std::vector<GuiInterface*> guiInterfacesPool;

	virtual GuiInterface* GetLogger(){return 0;}
	virtual GuiInterface* GetFolderBrowser(){return 0;}
	virtual GuiInterface* GetSceneEntities(){return 0;}
	virtual GuiInterface* GetProperty(){return 0;}
	virtual GuiInterface* GetRendererViewport(){return 0;}
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
	virtual SceneEntitiesInterface* GetSceneEntities(){return this;}
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
#ifndef GUI_INTERFACES_H
#define GUI_INTERFACES_H

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


struct RendererViewportInterface //should get the window handle from the WindowData class of the inherited class
{
	float RendererViewportInterface_viewScale;
	float RendererViewportInterface_farPlane;

	virtual void OnMouseWheel(float)=0;
	virtual void OnMouseRightDown()=0;
	virtual void OnViewportSize(int,int)=0;
	virtual void OnMouseMotion(int,int,bool leftButtonDown,bool altIsDown)=0;
};



#endif //GUI_INTERFACES_H
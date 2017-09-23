#ifndef GUI_INTERFACES_H
#define GUI_INTERFACES_H

#include "interfaces.h"

struct TabContainer;
struct GuiTab;
struct SceneEntityNode;
struct GuiTabElement;
struct GuiTabElementString;
struct GuiTabElementPropertyString;
struct GuiLabel;
struct GuiButton;
struct GuiTabImage;


struct Editor
{
	static std::vector<SceneEntityNode*> selection;
};




struct GuiInterface  : TPoolVector<GuiInterface>
{
	String name;

	GuiInterface():name("GuiInterface"){}

	static const unsigned int COLOR_GUI_BACKGROUND = 0x707070;
	static const unsigned int COLOR_MAIN_BACKGROUND = 0x505050;
	static const unsigned int COLOR_TEXT=0xFFFFFF;
	static const unsigned int COLOR_TEXT_SELECTED=0x0000ff;
	static const unsigned int COLOR_TEXT_HOVERED=0x0000f1;

	virtual void OnElementPaint(){}
	virtual void OnEntitiesChange(){}
	virtual void OnElementSize(){}
	virtual void OnGuiLMouseDown(){}
	virtual void OnGuiLMouseUp(){};
	virtual void OnGuiMouseMove(){}
	virtual void OnGuiUpdate(){}
	virtual void OnGuiReparent(){}
	virtual void OnGuiSelected(){}
	virtual void OnGuiRender(){}
	virtual void OnGuiMouseWheel(){}

	virtual void OnGuiRecreateTarget(){}

	static void BroadcastToGui(void (GuiInterface::*func)());
};


struct GuiTabImage
{
	unsigned char*	image;
	void*			data;
	float				width;
	float				height;

	GuiTabImage();

	virtual bool Load(const char* fileName);
	virtual void Draw(GuiTab*,vec4&);

	operator bool () {return image!=0;}
};




struct GuiTabElementRow;


struct GuiTabElement : GuiInterface , PtrHierarchyNode<GuiTabElement>
{
	vec4 rect;
	

	float &x,&y,&width,&height;

	unsigned int colorBackground;
	unsigned int colorForeground;
	unsigned int colorHovering;
	unsigned int colorPressing;
	unsigned int colorBorder;
	unsigned int colorChecked;

	GuiTabImage imageBackground;
	GuiTabImage imageHovering;
	GuiTabImage imagePressed;

	bool pressing;
	bool hovering;
	bool checked;

	int container;

	vec2 alignPos;
	vec2 alignRect;
	

	GuiTabElement* sibling[4];
	
	int	sequence;
	
	int animFrame;

	GuiTabElement(GuiTabElement* iParent=0,float ix=0, float iy=0, float iw=0,float ih=0,vec2 _alignPos=vec2(-1,-1),vec2 _alignRect=vec2(-1,-1));
	~GuiTabElement();

	virtual void OnPaint(GuiTab*);
	virtual void OnEntitiesChange(GuiTab*);
	virtual void OnSize(GuiTab*);
	virtual void OnLMouseDown(GuiTab*);
	virtual void OnLMouseUp(GuiTab*);
	virtual void OnMouseMove(GuiTab*);
	virtual void OnUpdate(GuiTab*);
	virtual void OnReparent(GuiTab*);
	virtual void OnSelected(GuiTab*);
	virtual void OnRender(GuiTab*);
	virtual void OnMouseWheel(GuiTab*);

	bool _contains(vec4& quad,vec2);

	void BroadcastToChilds(void (GuiTabElement::*func)(GuiTab*),GuiTab*);

	GuiTabElementString* CreateTabElementContainer(const char* iText);

	GuiTabElement* CreateTabElement(float ix=0, float iy=0, float iw=0,float ih=0);
	GuiTabElement* CreateTabElement(vec2 _alignPos=vec2(-1,-1),vec2 _alignRect=vec2(-1,-1));

	GuiTabElementString* CreateTabElementString(String str,float ix=0, float iy=0, float iw=0,float ih=0,vec2 _alignText=vec2(-1,-1));
	GuiTabElementString* CreateTabElementString(String str,vec2 _alignPos=vec2(-1,-1),vec2 _alignRect=vec2(-1,-1),vec2 _alignText=vec2(-1,-1));

	GuiTabElementPropertyString* CreateTabElementPropertyString(const char* iLeft,const char* iRight);
	GuiTabElementPropertyString* CreateTabElementPropertyString(const char* iLeft,vec3 iRight);
};

struct GuiTabElementString : GuiTabElement
{
	vec4 textRect;
	vec2 alignText;
	String text;

	virtual void OnPaint(GuiTab*);
	virtual void OnSize(GuiTab*);
};

struct GuiTabElementPropertyString : GuiTabElement
{
	String prp;
	String val;

	virtual void OnPaint(GuiTab*);
};

struct GuiTab : GuiInterface , TPoolVector<GuiTab>
{
	TabContainer* tabContainer;
	GuiTabElement guiTabRootElement;

	GuiTab(TabContainer* tc);

	~GuiTab();

	virtual void OnGuiPaint();
	virtual void OnEntitiesChange();
	virtual void OnGuiSize();
	virtual void OnGuiLMouseDown();
	virtual void OnGuiLMouseUp();
	virtual void OnGuiMouseMove();
	virtual void OnGuiUpdate();
	virtual void OnGuiReparent();
	virtual void OnGuiSelected();
	virtual void OnGuiRender();
	virtual void OnGuiMouseWheel();

	bool IsSelected();
};






struct ScrollBar
{
	static const int SCROLLBAR_WIDTH=20;
	static const int SCROLLBAR_TIP_HEIGHT=SCROLLBAR_WIDTH;
	static const int SCROLLBAR_AMOUNT=10;

	TabContainer* tab;

	float x,y,width,height;
	float scroller,scrollerFactor;
	float scrollerClick;

	ScrollBar(TabContainer* tab,float ix=0, float iy=0, float iw=0,float ih=0);
	~ScrollBar();

	void Set(float _x,float _y,float _width,float _height);
	void SetScrollerFactor(float contentHeight,float containerHeight);
	void SetScrollerPosition(float contentHeight);
	float GetScrollValue();

	float OnPressed();
	void OnReleased();
	bool OnScrolled();
	void OnPaint();
	void OnReparent(TabContainer*);
};



/*
struct GuiCheckBox : GuiTabElement
{
	bool checked;

	GuiCheckBox(float ix=0, float iy=0, float iw=0,float ih=0,GuiTabElement* te=0):GuiTabElement(ix,iy,iw,ih,te),checked(0){}

	virtual void OnLMouseDown();
};

struct GuiButton : GuiTabElement
{
	GuiTabElement label;
	GuiImage image;

	GuiButton(float ix=0, float iy=0, float iw=0,float ih=0,GuiTabElement* te=0):GuiTabElement(ix,iy,iw,ih,te),label(0,0,0,0,te),image(0,0,0,0,te){};

	virtual void OnMouseMove();
};*/



struct RendererViewportInterface
{
	float RendererViewportInterface_viewScale;
	float RendererViewportInterface_farPlane;

	String mousePositionString;

	RendererViewportInterface():RendererViewportInterface_viewScale(0),RendererViewportInterface_farPlane(0){}

	virtual void OnRendererMouseWheel(float)=0;
	virtual void OnRendererMouseRightDown()=0;
	virtual void OnRendererViewportSize(int,int)=0;
	virtual void OnRendererMouseMotion(float,float,bool leftButtonDown,bool altIsDown)=0;
	virtual void OnRendererMouseDown(float,float)=0;
	virtual float GetRendererProjectionHalfWidth()=0;
	virtual float GetRendererProjectionHalfHeight()=0;

};




#endif //GUI_INTERFACES_H
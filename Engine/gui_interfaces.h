#ifndef GUI_INTERFACES_H
#define GUI_INTERFACES_H

#include "interfaces.h"

struct TabContainer;
struct GuiTab;
struct SceneEntityNode;
struct GuiTabElement;
struct GuiLabel;
struct GuiButton;
struct GuiTabImage;


struct Editor
{
	static std::vector<SceneEntityNode*> selection;
};


struct GuiInterface  : TClassPool<GuiInterface>
{
	String name;

	GuiInterface():name("GuiInterface"){}

	static const unsigned int COLOR_GUI_BACKGROUND = 0x707070;
	static const unsigned int COLOR_MAIN_BACKGROUND = 0x505050;
	static const unsigned int COLOR_TEXT=0xFFFFFF;
	static const unsigned int COLOR_TEXT_SELECTED=0x0000ff;
	static const unsigned int COLOR_TEXT_HOVERED=0x0000f1;

	virtual void OnPaint(){}
	virtual void OnEntitiesChange(){}
	virtual void OnSize(){}
	virtual void OnLMouseDown(){}
	virtual void OnLMouseUp(){};
	virtual void OnMouseMove(){}
	virtual void OnUpdate(){}
	virtual void OnReparent(){}
	virtual void OnSelected(){}
	virtual void OnRender(){}
	virtual void OnMouseWheel(){}

	virtual void OnRecreateTarget(){}

	static void Broadcast(void (GuiInterface::*func)());
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
	vec4 textRect;

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

	String text;

	bool pressing;
	bool hovering;
	bool checked;

	int container;

	vec2 alignPos;
	vec2 alignText;
	vec2 alignRect;

	GuiTabElement* sibling[4];
	
	int	sequence;
	
	int animFrame;

	GuiTabElement(GuiTabElement* iParent=0,float ix=0, float iy=0, float iw=0,float ih=0,vec2 _alignPos=vec2(-1,-1),vec2 _alignText=vec2(-1,-1),vec2 _alignRect=vec2(-1,-1));
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

	virtual void _draw(GuiTab*);
	virtual bool _mousemove(GuiTab*);
	virtual void _reset();

	GuiTabElement* CreateTabElementContainer(GuiTabElement* iSibling,const char* iText);

	GuiTabElement* CreateTabElement(float ix=0, float iy=0, float iw=0,float ih=0,vec2 _alignPos=vec2(-1,-1),vec2 _alignRect=vec2(-1,-1),vec2 _alignText=vec2(-1,-1));

	GuiTabElement* CreateTabElementLabel(float ix=0, float iy=0, float iw=0,float ih=0,vec2 _alignPos=vec2(-1,-1),vec2 _alignRect=vec2(-1,-1),vec2 _alignText=vec2(-1,-1));

	GuiTabElementRow* CreateTabElementRow(GuiTabElement* iSibling,const char* iLeft,const char* iRight);
	GuiTabElementRow* CreateTabElementRow(GuiTabElement* iSibling,const char* iLeft,vec3 iRight);

	void SetAlignment(vec2 aP,vec2 fP,vec2 aT)
	{
		this->alignPos=aP,this->alignText=aT,this->alignRect=fP;
	}

	
};

struct GuiTab : GuiInterface , TClassPool<GuiTab>
{
	TabContainer* tabContainer;
	GuiTabElement guiTabRootElement;

	GuiTab(TabContainer* tc);

	~GuiTab(){}

	virtual void OnPaint();
	virtual void OnEntitiesChange();
	virtual void OnSize();
	virtual void OnLMouseDown();
	virtual void OnLMouseUp();
	virtual void OnMouseMove();
	virtual void OnUpdate();
	virtual void OnReparent();
	virtual void OnSelected();
	virtual void OnRender();
	virtual void OnMouseWheel();

	bool IsSelected();

	GuiTabElement* CreateTabElement(float ix=0,float iy=0,float iw=0,float ih=0,GuiTabElement* iParent=0)
	{
		GuiTabElement* newTabElement=new GuiTabElement(iParent ? iParent : &this->guiTabRootElement,ix,iy,iw,ih);
		return newTabElement;
	}

};


struct GuiTabElementRow : GuiTabElement
{
	GuiTabElement left;
	
	GuiTabElementRow(GuiTabElement*iParent,const char* iLeft);
};

struct GuiTabElementRowString : GuiTabElementRow
{
	GuiTabElement right;

	GuiTabElementRowString(GuiTabElement*iParent,const char* iLeft,const char* iRight);
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



struct RendererViewportInterface : GuiTab , TClassPool<RendererViewportInterface> 
{
	float RendererViewportInterface_viewScale;
	float RendererViewportInterface_farPlane;

	String mousePositionString;

	RendererViewportInterface(TabContainer* tc):GuiTab(tc){}

	virtual void OnMouseWheel(float)=0;
	virtual void OnMouseRightDown()=0;
	virtual void OnViewportSize(int,int)=0;
	virtual void OnMouseMotion(float,float,bool leftButtonDown,bool altIsDown)=0;
	virtual void OnMouseDown(float,float)=0;
	virtual float GetProjectionHalfWidth()=0;
	virtual float GetProjectionHalfHeight()=0;

};




#endif //GUI_INTERFACES_H
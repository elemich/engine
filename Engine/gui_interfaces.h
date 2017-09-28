#ifndef GUI_INTERFACES_H
#define GUI_INTERFACES_H

#include "interfaces.h"

struct TabContainer;
struct GuiTab;
struct SceneEntityNode;
struct GuiRect;
struct GuiString;
struct GuiButton;
struct GuiPropertyString;
struct GuiPropertySlider;
struct GuiPropertyAnimation;
struct GuiLabel;
struct GuiButton;
struct GuiTabImage;
struct AnimationController;


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







struct GuiRect : GuiInterface , PtrHierarchyNode<GuiRect>
{
	vec4 rect;
	
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
	

	GuiRect* sibling[4];
	
	int	sequence;
	
	int animFrame;

	GuiRect(GuiRect* iParent=0,float ix=0, float iy=0, float iw=0,float ih=0,vec2 _alignPos=vec2(-1,-1),vec2 _alignRect=vec2(-1,-1));
	~GuiRect();

	void Set(GuiRect* iParent=0,GuiRect* sibling=0,int sibIdx=0,int container=-1,float ix=0.0f, float iy=0.0f, float iw=0.0f,float ih=0.0f,float apx=-1.0f,float apy=-1.0f,float arx=-1.0f,float ary=-1.0f);

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

	void BroadcastToChilds(void (GuiRect::*func)(GuiTab*),GuiTab*);

	GuiString* Container(const char* iText);

	GuiRect* Rect(float ix=0, float iy=0, float iw=0,float ih=0);
	GuiRect* Rect(vec2 _alignPos=vec2(-1,-1),vec2 _alignRect=vec2(-1,-1));

	GuiString* Text(String str,float ix=0, float iy=0, float iw=0,float ih=0,vec2 _alignText=vec2(-1,-1));
	GuiString* Text(String str,vec2 _alignPos=vec2(-1,-1),vec2 _alignRect=vec2(-1,-1),vec2 _alignText=vec2(-1,-1));

	GuiPropertyString* Property(const char* iLeft,const char* iRight);
	GuiPropertyString* Property(const char* iLeft,vec3 iRight);

	GuiPropertySlider* Slider(const char* iLeft,float* ref);

	GuiButton* Button(String text,float ix=0, float iy=0, float iw=0,float ih=0);
	GuiButton* Button(String text,vec2 _alignPos=vec2(-1,-1),vec2 _alignRect=vec2(-1,-1),vec2 _alignText=vec2(-1,-1));

	GuiPropertyAnimation* PropertyAnimControl(AnimationController*);
};



struct GuiString : GuiRect
{
	vec4 textRect;
	vec2 alignText;
	String text;

	virtual void OnPaint(GuiTab*);
	virtual void OnSize(GuiTab*);
};

struct GuiButton : GuiString
{
	bool* referenceValue;
	int		updateMode;

	GuiButton():referenceValue(0),mouseUpFunc(0),updateMode(-1){}

	void (GuiRect::*mouseUpFunc)();

	void OnLMouseUp(GuiTab* tab);
};

struct GuiPropertyString : GuiRect
{
	String prp;
	String val;

	virtual void OnPaint(GuiTab*);
};

struct GuiSlider : GuiRect
{
	float *referenceValue;

	float minimum;
	float maximum;

	GuiSlider():referenceValue(0),minimum(0),maximum(1){}

	virtual void OnPaint(GuiTab*);
	virtual void OnMouseMove(GuiTab*);
};

struct GuiPropertySlider : GuiRect
{
	String prp;
	
	GuiSlider slider;

	virtual void OnPaint(GuiTab*);
};

struct GuiPropertyAnimation : GuiRect
{
	AnimationController* animController;

	GuiPropertyAnimation():animController(0){}

	GuiString text;

	GuiButton play;
	GuiButton stop;

	GuiSlider slider;

	virtual void OnMouseMove(GuiTab*);
};

struct GuiTab : GuiInterface , TPoolVector<GuiTab>
{
	TabContainer* tabContainer;
	GuiRect guiTabRootElement;

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
#ifndef GUI_INTERFACES_H
#define GUI_INTERFACES_H

#include "interfaces.h"

struct TabContainer;
struct SceneEntityNode;
struct GuiRect;
struct GuiRootRect;
struct GuiString;
struct GuiButton;
struct GuiPropertyString;
struct GuiPropertySlider;
struct GuiPropertyAnimation;
struct GuiLabel;
struct GuiButton;
struct GuiViewport;
struct GuiSceneViewer;
struct GuiEntityViewer;
struct GuiProjectViewer;
struct GuiImage;
struct AnimationController;


struct Editor
{
	static std::vector<SceneEntityNode*> selection;
};


struct GuiInterface
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
	virtual void OnGuiEntitySelected(){}
	virtual void OnGuiRender(){}
	virtual void OnGuiMouseWheel(){}
	virtual void OnGuiPaint(){}
	virtual void OnGuiDestroy(){}
	virtual void OnGuiActivate(){}
	virtual void OnGuiDeactivate(){}

	virtual void OnGuiRecreateTarget(){}
};


struct GuiImage
{
	unsigned char*	image;
	void*			data;
	float				width;
	float				height;

	GuiImage();

	virtual bool Load(const char* fileName);
	virtual void Draw(TabContainer*,vec4&);

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

	GuiImage imageBackground;
	GuiImage imageHovering;
	GuiImage imagePressed;

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

	void SetParent(GuiRect*);

	virtual void OnRecreateTarget(TabContainer*){}
	virtual void OnPaint(TabContainer*);
	virtual void OnEntitiesChange(TabContainer*);
	virtual void OnSize(TabContainer*);
	virtual void OnLMouseDown(TabContainer*);
	virtual void OnLMouseUp(TabContainer*);
	virtual void OnMouseMove(TabContainer*);
	virtual void OnUpdate(TabContainer*);
	virtual void OnReparent(TabContainer*);
	virtual void OnSelected(TabContainer*);
	virtual void OnRender(TabContainer*);
	virtual void OnMouseWheel(TabContainer*);
	virtual void OnActivate(TabContainer*);
	virtual void OnDeactivate(TabContainer*);
	virtual void OnEntitySelected(TabContainer*);

	virtual GuiRect* GetRoot(); 

	bool _contains(vec4& quad,vec2);

	void BroadcastToChilds(void (GuiRect::*func)(TabContainer*),TabContainer*);
	void BroadcastToRoot(void (GuiRect::*func)(TabContainer*));
	template<class C> void BroadcastTo(void (GuiRect::*func)(TabContainer* iTabContainer))
	{
		C* isaC=dynamic_cast<C*>(this);

		if(isaC)
			(this->*func)(iTabContainer);

		for_each(this->childs.begin(),this->childs.end(),std::bind(func,std::placeholders::_1,iTabContainer));
	}

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
	GuiViewport* Viewport(vec3 pos=vec3(100,100,100),vec3 target=vec3(0,0,0),vec3 up=vec3(0,0,1),bool perspective=true);
	GuiSceneViewer* SceneViewer();
	GuiEntityViewer* EntityViewer();
	GuiProjectViewer* ProjectViewer();
};

struct GuiRootRect : GuiRect , TPoolVector<GuiRootRect>
{
	TabContainer* tabContainer;

	void OnSize(TabContainer*);

	GuiRootRect(TabContainer* t):tabContainer(t){this->Set(0,0,0,-1,0,0,0,0,0,0,1,1);}
};

struct GuiString : GuiRect
{
	vec4 textRect;
	vec2 alignText;
	String text;

	virtual void OnPaint(TabContainer*);
	virtual void OnSize(TabContainer*);
};

struct GuiButton : GuiString
{
	bool* referenceValue;
	int		updateMode;

	GuiButton():referenceValue(0),mouseUpFunc(0),updateMode(-1){}

	void (GuiRect::*mouseUpFunc)();

	virtual void OnLMouseUp(TabContainer* tab);
};

struct GuiPropertyString : GuiRect
{
	String prp;
	String val;

	virtual void OnPaint(TabContainer*);
};

struct GuiSlider : GuiRect
{
	float *referenceValue;

	float minimum;
	float maximum;

	GuiSlider():referenceValue(0),minimum(0),maximum(1){}

	virtual void OnPaint(TabContainer*);
	virtual void OnMouseMove(TabContainer*);
};

struct GuiPropertySlider : GuiRect
{
	String prp;
	
	GuiSlider slider;

	virtual void OnPaint(TabContainer*);
};

struct GuiPropertyAnimation : GuiRect
{
	AnimationController* animController;

	GuiPropertyAnimation():animController(0){}

	GuiString text;

	GuiButton play;
	GuiButton stop;

	GuiSlider slider;

	virtual void OnMouseMove(TabContainer*);
};

struct GuiViewport : GuiRect
{
	Entity* reference;
	
	mat4 projection;
	mat4 view;
	mat4 model;

	RenderSurface *surface;

	GuiViewport();
	~GuiViewport();

	void Register();
	void Unregister();
	bool Registered();

	virtual void OnSize(TabContainer*);
	virtual void OnPaint(TabContainer*);
	virtual void OnMouseWheel(TabContainer*);
	virtual void OnMouseMove(TabContainer*);
	virtual void OnActivate(TabContainer*);
	virtual void OnDeactivate(TabContainer*);
	virtual void OnReparent(TabContainer*);
};









struct GuiScrollBar : GuiRect
{
	static const int SCROLLBAR_WIDTH=20;
	static const int SCROLLBAR_TIP_HEIGHT=SCROLLBAR_WIDTH;
	static const int SCROLLBAR_AMOUNT=10;

	float scroller,scrollerFactor;
	float scrollerClick;

	GuiScrollBar();
	~GuiScrollBar();

	void SetScrollerFactor(float contentHeight,float containerHeight);
	void SetScrollerPosition(float contentHeight);
	float GetScrollValue();

	void OnLMouseDown(TabContainer* tab);
	void OnLMouseUp(TabContainer* tab);
	void OnMouseMove(TabContainer* tab);
	void OnPaint(TabContainer* tab);
};





#endif //GUI_INTERFACES_H
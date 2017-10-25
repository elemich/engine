#ifndef GUI_INTERFACES_H
#define GUI_INTERFACES_H

#include "interfaces.h"

struct TabContainer;
struct SceneEntityNode;
struct GuiRect;
struct GuiRootRect;
struct GuiString;
struct GuiButton;
struct GuiScrollBar;
struct GuiScrollRect;
struct GuiPropertyString;
struct GuiPropertySlider;
struct GuiPropertyAnimation;
struct GuiLabel;
struct GuiButton;
struct GuiViewport;
struct GuiSceneViewer;
struct GuiEntityViewer;
struct GuiProjectViewer;
struct EntityNode;
struct GuiImage;
struct AnimationController;



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

struct GuiRect : THierarchyVector<GuiRect>
{
	static const int TREEVIEW_ROW_HEIGHT=20;
	static const int TREEVIEW_ROW_ADVANCE=TREEVIEW_ROW_HEIGHT;
	static const int SCROLLBAR_WIDTH=20;
	static const int SCROLLBAR_TIP_HEIGHT=SCROLLBAR_WIDTH;
	static const int SCROLLBAR_AMOUNT=10;

	String name;

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

	bool active;

	int container;

	vec2 alignPos;
	vec2 alignRect;

	GuiRect* sibling[4];
	
	int	sequence;
	
	int animFrame;

	GuiScrollRect* clip;

	GuiRect(GuiRect* iParent=0,float ix=0, float iy=0, float iw=0,float ih=0,vec2 _alignPos=vec2(0,0),vec2 _alignRect=vec2(1,1));
	~GuiRect();

	virtual void Set(GuiRect* iParent=0,GuiRect* sibling=0,int sibIdx=0,int container=-1,float ix=0.0f, float iy=0.0f, float iw=0.0f,float ih=0.0f,float apx=-1.0f,float apy=-1.0f,float arx=-1.0f,float ary=-1.0f);

	void SetParent(GuiRect*);

	virtual void OnRecreateTarget(TabContainer*,void* data=0){}
	virtual void OnPaint(TabContainer*,void* data=0);
	virtual void OnEntitiesChange(TabContainer*,void* data=0);
	virtual void OnSize(TabContainer*,void* data=0);
	virtual void OnLMouseDown(TabContainer*,void* data=0);
	virtual void OnLMouseUp(TabContainer*,void* data=0);
	virtual void OnRMouseUp(TabContainer*,void* data=0);
	virtual void OnMouseMove(TabContainer*,void* data=0);
	virtual void OnUpdate(TabContainer*,void* data=0);
	virtual void OnReparent(TabContainer*,void* data=0);
	virtual void OnSelected(TabContainer*,void* data=0);
	virtual void OnRender(TabContainer*,void* data=0);
	virtual void OnMouseWheel(TabContainer*,void* data=0);
	virtual void OnActivate(TabContainer*,void* data=0);
	virtual void OnDeactivate(TabContainer*,void* data=0);
	virtual void OnEntitySelected(TabContainer*,void* data=0);
	virtual void OnExpandos(TabContainer*,void* data=0);



	virtual bool SelfRender(TabContainer*);
	virtual void SelfRenderEnd(TabContainer*,bool&);

	virtual bool SelfClip(TabContainer*);
	virtual void SelfClipEnd(TabContainer*,bool&);

	virtual void SetClip(GuiScrollRect*);

	virtual GuiRect* GetRoot(); 

	bool _contains(vec4& quad,vec2);

	void BroadcastToChilds(void (GuiRect::*func)(TabContainer*,void*),TabContainer*,void* data=0);
	void BroadcastToRoot(void (GuiRect::*func)(TabContainer*,void*),void* data=0);
	template<class C> void BroadcastTo(void (GuiRect::*func)(TabContainer* iTabContainer))
	{
		C* isaC=dynamic_cast<C*>(this);

		if(isaC)
			(this->*func)(iTabContainer);

		for_each(this->childs.begin(),this->childs.end(),std::bind(func,std::placeholders::_1,iTabContainer));
	}

	GuiString* Container(const char* iText);

	GuiRect* Rect(float ix=0, float iy=0, float iw=0,float ih=0,float apx=0, float apy=0, float arx=1,float ary=1);

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

	GuiRootRect(TabContainer* t):tabContainer(t){this->name="RootRect";this->Set(0,0,0,-1,0,0,0,0,0,0,1,1);}
};

struct GuiString : GuiRect
{
	vec4 textRect;
	vec2 alignText;
	String text;

	GuiString(){this->name="String";}

	virtual void OnPaint(TabContainer*,void* data=0);
};

struct GuiButton : GuiString
{
	bool* referenceValue;
	int		updateMode;

	GuiButton():referenceValue(0),mouseUpFunc(0),updateMode(-1){this->name="Button";}

	void (GuiRect::*mouseUpFunc)();

	virtual void OnLMouseUp(TabContainer* tab,void* data=0);
};


struct GuiScrollBar : GuiRect
{
	float scrollerPosition,scrollerRatio;
	float scrollerPressed;

	float parentAlignRectX;

	GuiRect* guiRect;

	GuiScrollBar();
	~GuiScrollBar();


	bool SetScrollerRatio(float contentHeight,float containerHeight);
	bool SetScrollerPosition(float contentHeight);
	bool Scroll(float upOrDown);
	bool IsVisible();

	void OnLMouseDown(TabContainer* tab,void* data=0);
	void OnLMouseUp(TabContainer* tab,void* data=0);
	void OnMouseMove(TabContainer* tab,void* data=0);
	void OnPaint(TabContainer* tab,void* data=0);

	float GetContainerHeight();
	float GetContainerTop();
	float GetContainerBottom();
	float GetScrollerTop();
	float GetScrollerBottom();
	float GetScrollerHeight();

};


struct GuiScrollRect : GuiRect
{
	GuiScrollRect();
	~GuiScrollRect();

	float contentHeight;
	float width;

	GuiScrollBar	*scrollBar;

	bool isClipped;

	virtual void OnMouseWheel(TabContainer*,void* data=0);
	virtual void OnSize(TabContainer*,void* data=0);
};

struct GuiPropertyString : GuiRect
{
	String prp;
	String val;

	GuiPropertyString(){this->name="PropertyString";}

	virtual void OnPaint(TabContainer*,void* data=0);
};

struct GuiSlider : GuiRect
{
	float *referenceValue;

	float minimum;
	float maximum;

	GuiSlider():referenceValue(0),minimum(0),maximum(1){this->name="Slider";}

	virtual void OnPaint(TabContainer*,void* data=0);
	virtual void OnMouseMove(TabContainer*,void* data=0);
	virtual void OnSize(TabContainer*,void* data=0);
};

struct GuiPropertySlider : GuiRect
{
	String prp;
	
	GuiSlider slider;

	GuiPropertySlider(){this->name="PropertySlider";}

	virtual void OnPaint(TabContainer*,void* data=0);
};

struct GuiPropertyAnimation : GuiRect
{
	AnimationController* animController;

	GuiPropertyAnimation():animController(0){this->name="PropertyAnimation";}

	GuiString text;

	GuiButton play;
	GuiButton stop;

	GuiSlider slider;

	virtual void OnMouseMove(TabContainer*,void* data=0);
};

struct GuiViewport : GuiRect
{
	Entity* rootEntity;

	mat4 projection;
	mat4 view;
	mat4 model;

	RenderSurface *surface;

	GuiViewport();
	~GuiViewport();

	void Register();
	void Unregister();
	bool Registered();

	virtual void OnSize(TabContainer*,void* data=0);
	virtual void OnPaint(TabContainer*,void* data=0);
	virtual void OnMouseWheel(TabContainer*,void* data=0);
	virtual void OnMouseMove(TabContainer*,void* data=0);
	virtual void OnActivate(TabContainer*,void* data=0);
	virtual void OnDeactivate(TabContainer*,void* data=0);
	virtual void OnReparent(TabContainer*,void* data=0);
};






#endif //GUI_INTERFACES_H
#ifndef INTERFACES_H
#define INTERFACES_H

#include "primitives.h"

#include "entities.h"

struct ShaderInterface;
struct Renderer3dInterface;
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

struct SplitterContainer;
struct ContainerWindow;

struct AppInterface
{
	virtual int Init()=0;
	virtual void Run()=0;
	virtual void CreateMainWindow()=0;
};



struct ShaderInterface : TPoolVector<ShaderInterface>
{
	static ShaderInterface* Find(const char*,bool exact=true);

	virtual int GetProgram()=0;
	virtual void SetProgram(int)=0;

	virtual void SetName(const char*)=0;
	virtual const char* GetName()=0;

	virtual int GetUniform(int slot,char* var)=0;
	virtual int GetAttrib(int slot,char* var)=0;

	virtual void Use()=0;

	virtual const char* GetPixelShader()=0;
	virtual const char* GetFragmentShader()=0;

	virtual int GetAttribute(const char*)=0;
	virtual int GetUniform(const char*)=0;

	virtual int init()=0;

	virtual int GetPositionSlot()=0;
	virtual int GetColorSlot()=0;
	virtual int GetProjectionSlot()=0;
	virtual int GetModelviewSlot()=0;
	virtual int GetTexcoordSlot()=0;
	virtual int GetTextureSlot()=0;
	virtual int GetLightposSlot()=0;
	virtual int GetMouseSlot()=0;
	virtual int GetLightdiffSlot()=0;
	virtual int GetLightambSlot()=0;
	virtual int GetNormalSlot()=0;
	virtual int GetHoveringSlot()=0;

	virtual void SetSelectionColor(bool pick,void* ptr,vec2 mposNrm)=0;

	virtual bool SetMatrix4f(int slot,float* mtx)=0;

	virtual unsigned int& GetBufferObject()=0;

	virtual void SetProjectionMatrix(float*)=0;
	virtual void SetModelviewMatrix(float*)=0;
	virtual void SetMatrices(float* view,float* mdl)=0;

	ShaderInterface();
};

struct Renderer3dInterface : TPoolVector<Renderer3dInterface>
{
	std::list<GuiViewport*> viewports;
	Task* rendererTask;
	bool picking;
	std::vector<ShaderInterface*> shaders;

	ShaderInterface* FindShader(const char* name,bool exact);
	void SetMatrices(const float* view,const float* mdl);

	void Register(GuiViewport*);
	void Unregister(GuiViewport*);

	virtual char* Name()=0;

	virtual void draw(vec3,float psize=1.0f,vec3 color=vec3(1,1,1))=0;
	virtual void draw(vec2)=0;
	virtual void draw(vec3,vec3,vec3 color=vec3(1,1,1))=0;
	virtual void draw(vec4)=0;
	virtual void draw(AABB,vec3 color=vec3(1,1,1))=0;
	virtual void draw(mat4 mtx,float size,vec3 color=vec3(1,1,1))=0;
	//virtual void draw(Font*,char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4)=0;
	virtual void draw(char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4)=0;


	virtual void draw(Bone*)=0;
	virtual void draw(Mesh*)=0;
	virtual void draw(Skin*)=0;
	virtual void draw(Texture*)=0;
	virtual void draw(Light*)=0;
	virtual void drawUnlitTextured(Mesh*)=0;
	virtual void draw(Mesh*,std::vector<unsigned int>& textureIndices,int texture_slot,int texcoord_slot)=0;
	virtual void draw(Camera*)=0;
	virtual void draw(Gizmo*)=0;
	virtual void draw(Piped*)=0;
	virtual void draw(Sphere*)=0;
	virtual void draw(Cylinder*)=0;
	virtual void draw(Tetrahedron*)=0;

	virtual void ChangeContext()=0;

	virtual void Render(GuiViewport*,bool)=0;
	virtual void Render()=0;

	Renderer3dInterface();
	virtual ~Renderer3dInterface(){};

	ShaderInterface* unlit;
	ShaderInterface* unlit_color;
	ShaderInterface* unlit_texture;
	ShaderInterface* font;
	ShaderInterface* shaded_texture;
};

struct InputInterface
{

};

struct Game
{

};


struct MatrixStack
{
	enum matrixmode
	{
		PROJECTION=0,
		MODEL,
		VIEW,
		MATRIXMODE_MAX
	};

	static void Reset();


	static void Push();
	static void Pop();
	static void Identity();
	static float* Get();
	static void Load(float* m);
	static void Multiply(float* m);

	static void Pop(MatrixStack::matrixmode);
	static void Push(MatrixStack::matrixmode);
	static void Push(MatrixStack::matrixmode,float*);
	static void Identity(MatrixStack::matrixmode);
	static float* Get(MatrixStack::matrixmode,int lev=-1);
	static void Load(MatrixStack::matrixmode,float*);
	static void Multiply(MatrixStack::matrixmode,float*);

	static void Rotate(float a,float x,float y,float z);
	static void Translate(float x,float y,float z);
	static void Scale(float x,float y,float z);

	static mat4 GetProjectionMatrix();
	static mat4 GetModelMatrix();
	static mat4 GetViewMatrix();

	static void SetProjectionMatrix(float*);
	static void SetModelMatrix(float*);
	static void SetViewMatrix(float*);

	static  MatrixStack::matrixmode GetMode();
	static  void SetMode(MatrixStack::matrixmode m);

	static mat4 model;
	static mat4 projection;
	static mat4 view;
};

int simple_shader(const char* name,int shader_type, const char* shader_src);
int create_program(const char* name,const char* vertexsh,const char* fragmentsh);

struct OpenGLShader : ShaderInterface
{
	static ShaderInterface* Create(const char* shader_name,const char* pixel_shader,const char* fragment_shader);

	int GetProgram();
	void SetProgram(int);

	int GetUniform(int slot,char* var);
	int GetAttrib(int slot,char* var);

	void Use();

	const char* GetPixelShader();
	const char* GetFragmentShader();

	int GetAttribute(const char*);
	int GetUniform(const char*);

	int init();

	int GetPositionSlot();
	int GetColorSlot();
	int GetProjectionSlot();
	int GetModelviewSlot();
	int GetTexcoordSlot();
	int GetTextureSlot();
	int GetLightposSlot();
	int GetLightdiffSlot();
	int GetLightambSlot();
	int GetNormalSlot();
	int GetMouseSlot();
	int GetHoveringSlot();

	void SetSelectionColor(bool pick,void* ptr,vec2 mposNrm);

	bool SetMatrix4f(int slot,float* mtx);

	unsigned int& GetBufferObject();

	void SetProjectionMatrix(float*);
	void SetModelviewMatrix(float*);
	void SetMatrices(float* view,float* mdl);

	void SetName(const char*);
	const char* GetName();

	//char* name;
	String name;
	int   program;
	unsigned int	  vbo;
	unsigned int	  vao;
	unsigned int	  ibo;
};

struct TouchInput : InputInterface
{
	enum
	{
		TOUCH_DOWN,
		TOUCH_UP,
		TOUCH_MAX
	};

#define MAX_TOUCH_INPUTS 10

	bool pressed[MAX_TOUCH_INPUTS];
	bool released[MAX_TOUCH_INPUTS];

	vec2 position[MAX_TOUCH_INPUTS];

	TouchInput()
	{
		for(int i=0;i<10;i++)
		{
			pressed[i]=0;
			released[i]=0;
			position[i].make(0,0);
		}
	}

	bool IsPressed(int i){return pressed[i];}
	bool IsReleased(int i){return released[i];}

	void SetPressed(bool b,int i){pressed[i]=b;}
	void SetReleased(bool b,int i){released[i]=b;}


	vec2& GetPosition(int i){return position[i];}
	void   SetPosition(vec2& pos,int i){position[i]=pos;}


};

struct KeyboardInput : InputInterface
{
	enum
	{
		KEY_PRESSED=0,
		KEY_RELEASED,
		KEY_INACTIVE
	};

	int keys[255];
	int nkeys;

	KeyboardInput()
	{
		for(int i=0;i<255;i++)
			SetKey(i,KEY_INACTIVE);
		nkeys=0;
	}

	void SetKey(unsigned char c,int state)//pressed,released,inactive
	{
		keys[c]=state;

		switch(state)
		{
		case KEY_PRESSED:{if(nkeys<sizeof(keys))nkeys++;}break;
		case KEY_RELEASED:{if(nkeys>0)nkeys--;}break;
		}
	}

	int GetKey(unsigned char c)//pressed,released,inactive
	{
		return keys[c];
	}

	bool GetPressed(unsigned char c)
	{
		return keys[c]==KEY_PRESSED;
	}

	bool GetReleased(unsigned char c)
	{
		return keys[c]==KEY_RELEASED;
	}

};

struct MouseInput : InputInterface
{
	enum
	{
		MOUSE_STATE_PRESSED=0,
		MOUSE_STATE_RELEASED,
		MOUSE_STATE_CLICKED,
		MOUSE_STATE_DBLCLICKED,
		MOUSE_STATE_INACTIVE,
		MOUSE_STATE_MAX
	};

	enum
	{
		BUTTON_LEFT=0,
		BUTTON_CENTER,
		BUTTON_RIGHT,
		BUTTON_MAX
	};

	enum
	{
		DIR_LEFT=0,
		DIR_RIGHT,
		DIR_UP,
		DIR_BOTTOM
	};

	int		mouse_states[BUTTON_MAX];
	vec2	mouse_pos;
	vec2	mouse_posold;
	vec2	mouse_posnorm;
	float   mouse_timers[BUTTON_MAX];



	MouseInput();


	bool IsClick(int);
	virtual void Update();
};


struct InputManager
{
	static TouchInput touchInput;
	static MouseInput mouseInput;
	static KeyboardInput keyboardInput;
	static InputInterface voiceInput;
	static InputInterface joystickInput;

	void update()
	{
		for(int i=0;i<255;i++)
		{
			int state=keyboardInput.GetKey(i);
			keyboardInput.SetKey(i,state==KeyboardInput::KEY_RELEASED ? KeyboardInput::KEY_INACTIVE : state);
		}
	}
};

/*
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
};*/

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

	/*GuiImage imageBackground;
	GuiImage imageHovering;
	GuiImage imagePressed;*/

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

struct GuiViewport : GuiRect , TPoolVector<GuiViewport>
{
	Entity* rootEntity;

	mat4 projection;
	mat4 view;
	mat4 model;

	void* renderBuffer;
	void* renderBitmap;

	unsigned int lastFrameTime;

	bool needsPicking;

	unsigned char pickedPixel[4];
	Entity*		  pickedEntity;

	GuiViewport();
	~GuiViewport();

	virtual void OnPaint(TabContainer*,void* data=0);
	virtual void OnSize(TabContainer*,void* data=0);
	virtual void OnMouseWheel(TabContainer*,void* data=0);
	virtual void OnMouseMove(TabContainer*,void* data=0);
	virtual void OnActivate(TabContainer*,void* data=0);
	virtual void OnDeactivate(TabContainer*,void* data=0);
	virtual void OnReparent(TabContainer*,void* data=0);
	virtual void OnLMouseUp(TabContainer*,void* data=0);
};

#ifdef _WIN32	
#include "windows.h"
#include <d2d1.h>
typedef HWND WINDOWHANDLE;
typedef HMENU MENUHANDLE;
typedef UINT UINTHANDLE;
typedef WPARAM WPARAMHANDLE;
typedef LPARAM LPARAMHANDLE;
typedef ID2D1HwndRenderTarget* GUIRENDERTARGET;
typedef ID2D1SolidColorBrush* GUISOLIDCOLORBRUSH;
typedef ID2D1Bitmap* GUIBITMAP;
typedef ID2D1Brush* GUIBRUSH;
#endif

/*
struct Renderer2DInterface
{
	void SetTransform()=0;
	void FillRectangle()=0;
	void Clear()=0;
	void CreateSolidColorBrush()=0;
	void GetPixelFormat()=0;
	void CreateBitmap()=0;
	void DrawRectangle()=0;
	void FillRoundedRectangle()=0;
	void DrawBitmap()=0;
	void PushAxisAlignedClip()=0;
	void PopAxisAlignedClip()=0;
	void BeginDraw()=0;
	void EndDraw()=0;
	void Flush()=0;
};*/

struct WindowData
{
	WINDOWHANDLE hwnd;
	float width;
	float height;

	std::list<WindowData*> siblings[4];

	void LinkSibling(WindowData* t,int pos);
	void UnlinkSibling(WindowData* t=0);
	WindowData* FindSiblingOfSameSize();
	int FindSiblingPosition(WindowData* t);
	bool FindAndGrowSibling();

	UINTHANDLE msg;
	WPARAMHANDLE wparam;
	LPARAMHANDLE lparam;

	operator WINDOWHANDLE(){return this->hwnd;};
	void CopyProcedureData(WINDOWHANDLE  h,UINTHANDLE m,WPARAMHANDLE w,LPARAMHANDLE l);
	virtual void Create(WINDOWHANDLE container)=0;


	virtual void OnSize();
	virtual void OnWindowPosChanging();
};

struct TabContainer : WindowData , TPoolVector<TabContainer>
{
	ContainerWindow* parentContainer;

	static const unsigned int COLOR_TAB_BACKGROUND=0x808080;
	static const unsigned int COLOR_TAB_SELECTED=0x0000FF;
	static const unsigned int COLOR_GUI_BACKGROUND = 0x707070;
	static const unsigned int COLOR_MAIN_BACKGROUND = 0x505050;
	static const unsigned int COLOR_TEXT=0xFFFFFF;
	static const unsigned int COLOR_TEXT_SELECTED=0x0000ff;
	static const unsigned int COLOR_TEXT_HOVERED=0x0000f1;


	static const int CONTAINER_HEIGHT=30;
	static const int TAB_WIDTH=80;
	static const int TAB_HEIGHT=25;

	static const int CONTAINER_ICON_WH=20;
	static const int CONTAINER_ICON_STRIDE=CONTAINER_ICON_WH*4;


	static unsigned char* rawUpArrow;
	static unsigned char* rawRightArrow;
	static unsigned char* rawDownArrow;
	static unsigned char* rawFolder;
	static unsigned char* rawFile;

	static LRESULT CALLBACK TabContainerWindowClassProcedure(WINDOWHANDLE,UINT,WPARAM,LPARAM);

	GUIRENDERTARGET renderTarget;
	GUISOLIDCOLORBRUSH  brush;

	GUIBITMAP iconUp;
	GUIBITMAP iconRight;
	GUIBITMAP iconDown;
	GUIBITMAP iconFolder;
	GUIBITMAP iconFile;

	GuiRootRect	tabs;

	SplitterContainer* splitterContainer;

	Renderer3dInterface *renderer;

	unsigned int selected;
	bool mouseDown;
	bool isRender;
	bool recreateTarget;
	bool resizeTarget;
	Entity* reloadScript;

	GuiRect* drawRect;
	bool	drawFrame;
	Task*	drawTask;

	float mousex,mousey;

	int nPainted;

	bool buttonLeftMouseDown;
	bool buttonControlDown;

	unsigned int lastFrameTime;

	bool skip;

	Thread thread;

	TabContainer(float x,float y,float w,float h,WINDOWHANDLE parent);
	~TabContainer();

	operator TabContainer& (){return *this;}

	void Create(WINDOWHANDLE){}//@mic no more used, delete from WindowData

	virtual void OnGuiPaint(void* data=0);
	virtual void OnGuiSize(void* data=0);
	virtual void OnWindowPosChanging(void* data=0);
	virtual void OnGuiLMouseDown(void* data=0);
	virtual void OnGuiLMouseUp(void* data=0);
	virtual void OnGuiMouseMove(void* data=0);
	virtual void OnRMouseUp(void* data=0);
	virtual void OnGuiUpdate(void* data=0);
	virtual void OnGuiRender(void* data=0);
	virtual void OnGuiMouseWheel(void* data=0);
	virtual void OnResizeContainer(void* data=0);
	virtual void OnEntitiesChange(void* data=0);
	virtual void OnGuiActivate(void* data=0);
	virtual void OnGuiDeactivate(void* data=0);
	virtual void OnGuiEntitySelected(void* data=0);

	void RecreateTarget();
	void DrawFrame();

	void DrawText(unsigned int iColor,const char* iText,float x,float y, float w,float h,float iAlignX=-1,float iAlignY=-1);

	virtual void OnGuiRecreateTarget(void* data=0);

	GUIBRUSH SetColor(unsigned int color);

	GuiRect* GetSelected();

	void BroadcastToSelected(void (GuiRect::*func)(TabContainer*,void*),void* data=0);
	void BroadcastToAll(void (GuiRect::*func)(TabContainer*,void*),void* data=0);
	template<class C> void BroadcastToSelected(void (GuiRect::*func)(TabContainer*,void*),void*);
	template<class C> void BroadcastToAll(void (GuiRect::*func)(TabContainer*,void*),void*);
	static void BroadcastToPoolSelecteds(void (GuiRect::*func)(TabContainer*,void*),void* data=0)
	{
		for(std::vector<TabContainer*>::iterator tabContainer=TPoolVector<TabContainer>::pool.begin();tabContainer!=TPoolVector<TabContainer>::pool.end();tabContainer++)
			(*tabContainer)->BroadcastToSelected(func,data);
	}

	void Draw();

	bool BeginDraw();
	void EndDraw();


	void SetDraw(GuiRect* iRect,bool iFrame);
};

struct SplitterContainer 
{
	ContainerWindow* GetContainer(){return (ContainerWindow*)this;}

	static MENUHANDLE popupMenuRoot;
	static MENUHANDLE popupMenuCreate;

	TabContainer* currentTabContainer;

	TabContainer* floatingTabRef;
	TabContainer* floatingTab;
	TabContainer* floatingTabTarget;
	int  floatingTabRefTabIdx;
	int  floatingTabRefTabCount;
	int  floatingTabTargetAnchorPos;
	int	 floatingTabTargetAnchorTabIndex;
	RECT floatingTabRc;
	RECT floatingTabTargetRc;

	const int   splitterSize;
	LPCSTR		splitterCursor;
	POINTS splitterPreviousPos;

	std::vector<WINDOWHANDLE> resizingWindows1;
	std::vector<WINDOWHANDLE> resizingWindows2;
	WINDOWHANDLE hittedWindow1;
	WINDOWHANDLE hittedWindow2;

	SplitterContainer();
	~SplitterContainer();

	void OnLButtonDown(WINDOWHANDLE,LPARAM);
	void OnLButtonUp(WINDOWHANDLE);
	void OnMouseMove(WINDOWHANDLE,LPARAM);
	void OnSize(WINDOWHANDLE,WPARAM,LPARAM);

	void CreateFloatingTab(TabContainer*);
	void DestroyFloatingTab();

	std::vector<WINDOWHANDLE> findWindoswAtPos(WINDOWHANDLE mainWindow,RECT &srcRect,int rectPosition);

	void EnableChilds(WINDOWHANDLE hwnd,int enable=-1,int show=-1);
	void EnableAllChildsDescendants(WINDOWHANDLE hwnd,int enable=-1,int show=-1);
};


struct ContainerWindow : WindowData , SplitterContainer
{
	std::vector<TabContainer*> tabContainers;

	int ContainerWindow_currentVisibleChildIdx;

	int resizeDiffHeight;
	int resizeDiffWidth;
	int resizeEnumType;
	int resizeCheckWidth;
	int resizeCheckHeight;

	ContainerWindow();

	void Create(WINDOWHANDLE hwnd=0);
	void OnCreate(WINDOWHANDLE);
	void OnSizing();
	void OnSize();

};



struct App;

struct MainAppContainerWindow : ContainerWindow
{
	MainAppContainerWindow();

	App *application;

	static std::vector<ContainerWindow> windows;

	MENUHANDLE menuMain;
	MENUHANDLE menuEntities;

	void Create(WINDOWHANDLE hwnd=0);
	void OnCreate(WINDOWHANDLE);
};

struct TimerWin32 : Timer
{
	virtual void update();
	virtual unsigned int GetTime();
};

struct App : AppInterface , TStaticInstance<App>
{

	TimerWin32 timerMain;
	MainAppContainerWindow mainAppWindow;


	String projectFolder;

	int Init();	
	void Close();

	void CreateMainWindow();
	void Run();
};


struct RenderSurface
{
	GUIBITMAP renderBitmap;
	unsigned char* renderBuffer;
	TabContainer* tab;
	unsigned int lastFrameTime;

	RenderSurface():renderBitmap(0),renderBuffer(0){}
	~RenderSurface(){SAFERELEASE(renderBitmap);SAFEDELETEARRAY(renderBuffer);}
};

struct GuiSceneViewer : GuiScrollRect
{
	GuiSceneViewer();
	~GuiSceneViewer();

	Entity* entityRoot;

	std::vector<Entity*> selection;

	void OnPaint(TabContainer*,void* data=0);
	void OnLMouseDown(TabContainer*,void* data=0);
	void OnEntitiesChange(TabContainer*,void* data=0);
	void OnEntitySelected(TabContainer*,void* data=0);
	void OnRecreateTarget(TabContainer*,void* data=0);
	void OnRMouseUp(TabContainer*,void* data=0);
	void OnMouseWheel(TabContainer*,void* data=0);


	bool ProcessMouseInput(vec2&,vec2&,Entity* node,bool iGetHovered,Entity*& expChanged,Entity*& selChanged,Entity*& curHover);
	void DrawNodes(TabContainer*,Entity*,vec2&);
	int UpdateNodes(Entity*);
	void UnselectNodes(Entity*);
	void ExpandUntil(Entity* iTarget);
};	



struct GuiEntityViewer : GuiScrollRect
{
	GuiEntityViewer();
	~GuiEntityViewer();

	Entity* entity;

	void OnActivate(TabContainer*,void* data=0);

	virtual void OnEntitySelected(TabContainer*,void* data=0);
	virtual void OnLMouseDown(TabContainer*,void* data=0);
	virtual void OnPaint(TabContainer*,void* data=0);
	virtual void OnExpandos(TabContainer*,void* data=0);
	virtual void OnMouseMove(TabContainer*,void* data=0);
	virtual void OnMouseWheel(TabContainer*,void* data=0);

	bool ProcessMouseInput(vec2&,vec2&,GuiRect* node);
	void DrawNodes(TabContainer*,GuiRect*,vec2&);
	int UpdateNodes(GuiRect*);
};




struct GuiProjectViewer : GuiRect
{
	struct ResourceNode
	{
		ResourceNode* parent;

		String fileName;

		bool selectedLeft;
		bool selectedRight;
		int level;
		bool isDir;

		ResourceNode();
		~ResourceNode();
	};

	struct ResourceNodeDir : ResourceNode
	{
		bool expanded;

		std::list<ResourceNodeDir*> dirs;
		std::list<ResourceNode*>	files;

		ResourceNodeDir();
		~ResourceNodeDir();
	};

	struct GuiDirView : GuiScrollRect
	{
		void DrawNodes(TabContainer*,ResourceNodeDir* node,vec2&,bool& terminated);
		bool ProcessMouseInput(vec2&,vec2&,float& drawFromY,ResourceNodeDir* root,ResourceNodeDir* node,ResourceNodeDir*& expChanged,ResourceNodeDir*& selChanged);
		int CalcNodesHeight(ResourceNodeDir*);
		void UnselectNodes(ResourceNodeDir*);
		std::vector<ResourceNodeDir*> selectedDirs;
		ResourceNodeDir* rootResource;

		void OnLMouseDown(TabContainer*,void* data=0);
		void OnPaint(TabContainer*,void* data=0);

	}left;

	struct GuiFileView : GuiScrollRect
	{
		void DrawNodes(TabContainer*,ResourceNodeDir* node,vec2&);
		bool ProcessMouseInput(vec2&,vec2&,float& drawFromY,ResourceNodeDir* root,ResourceNodeDir* node,ResourceNodeDir*& expChanged,ResourceNode*& selChanged);
		int CalcNodesHeight(ResourceNodeDir*);
		void UnselectNodes(ResourceNodeDir*);
		ResourceNodeDir* rootResource;
		std::vector<ResourceNodeDir*> selectedDirs;
		std::vector<ResourceNode*> selectedFiles;

		void OnLMouseDown(TabContainer*,void* data=0);
		void OnPaint(TabContainer*,void* data=0);
	}right;


	ResourceNodeDir rootResource;

	GuiProjectViewer();
	~GuiProjectViewer();

	bool lMouseDown;
	bool splitterMoving;

	void OnPaint(TabContainer*,void* data=0);
	void OnLMouseDown(TabContainer*,void* data=0);
	void OnLMouseUp(TabContainer*,void* data=0);
	void OnMouseMove(TabContainer*,void* data=0);
	void OnReparent(TabContainer*,void* data=0);
	void OnActivate(TabContainer*,void* data=0);

	void ScanDir(String dir);
	void CreateNodes(String dir,ResourceNodeDir*);
};
bool InitSplitter();


#endif //INTERFACES_H



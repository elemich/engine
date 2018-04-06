#ifndef INTERFACES_H
#define INTERFACES_H

#include "entities.h"

struct DLLBUILD EditorEntity;

struct DLLBUILD GuiRect;
struct DLLBUILD GuiRootRect;
struct DLLBUILD GuiString;
struct DLLBUILD GuiButton;
struct DLLBUILD GuiImage;
struct DLLBUILD GuiButtonBool;
struct DLLBUILD GuiScrollBar;
struct DLLBUILD GuiScrollRect;
struct DLLBUILD GuiPropertyString;
struct DLLBUILD GuiPropertySlider;
struct DLLBUILD GuiAnimationController;
struct DLLBUILD GuiLabel;
struct DLLBUILD GuiViewport;
struct DLLBUILD GuiSceneViewer;
struct DLLBUILD GuiEntityViewer;
struct DLLBUILD GuiCompilerViewer;
struct DLLBUILD GuiProjectViewer;
struct DLLBUILD GuiScriptViewer;
struct DLLBUILD GuiPropertyVec3;
struct DLLBUILD GuiPropertyPtr;
struct DLLBUILD GuiPropertyFloat;
struct DLLBUILD GuiPropertyBool;
struct DLLBUILD GuiPropertyAnimationController;

struct DLLBUILD Tab;
struct DLLBUILD Container;
struct DLLBUILD Splitter;
struct DLLBUILD MainContainer;
struct DLLBUILD ResourceNodeDir;
struct DLLBUILD Compiler;
struct DLLBUILD Subsystem;
struct DLLBUILD Debugger;
struct DLLBUILD EditorScript;

//entity forward declaration 

struct DLLBUILD Entity;
struct DLLBUILD AnimationController;
struct DLLBUILD Script;

#define MAX_TOUCH_INPUTS 10


struct DLLBUILD MatrixStack
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


struct DLLBUILD InputInterface
{
};

struct DLLBUILD TouchInput : InputInterface
{
	enum
	{
		TOUCH_DOWN,
		TOUCH_UP,
		TOUCH_MAX
	};

	bool pressed[MAX_TOUCH_INPUTS];
	bool released[MAX_TOUCH_INPUTS];

	vec2 position[MAX_TOUCH_INPUTS];

	TouchInput();

	bool IsPressed(int i);
	bool IsReleased(int i);

	void SetPressed(bool b,int i);
	void SetReleased(bool b,int i);


	vec2& GetPosition(int i);
	void   SetPosition(vec2& pos,int i);


};

struct DLLBUILD KeyboardInput : InputInterface
{
	bool IsPressed(unsigned int iCharCode);
};

struct DLLBUILD MouseInput : InputInterface
{
	vec2	pos;
	vec2	posold;

	bool left;
	bool right;
	bool middle;

	unsigned int lastLeft;
	unsigned int lastRight;
	unsigned int lastMiddle;

	MouseInput();

	bool Left();
	bool Right();
	bool Middle();
};


struct DLLBUILD InputManager
{
	static TouchInput touchInput;
	static MouseInput mouseInput;
	static KeyboardInput keyboardInput;
	/*static InputInterface voiceInput;
	static InputInterface joystickInput;*/
};



struct DLLBUILD EngineIDE : TStaticInstance<EngineIDE>
{
	Timer*					timerMain;
	MainContainer*			mainAppWindow;
	String					projectFolder;
	FilePath				ideExecutable;
	String					applicationDataFolder;
	Compiler*				compiler; 
	InputManager            inputManager;
	unsigned int            caretLastTime;
	Subsystem*				subsystem;
	Debugger*				debugger;
	unsigned int			processId;
	unsigned int			processThreadId;
	//Thread*					threadUpdate;

	EngineIDE();

	virtual int Initialize()=0;	
	virtual void Deinitialize()=0;
	virtual void Run()=0;

	virtual void CreateNodes(String,ResourceNodeDir*)=0;
	virtual void ScanDir(String)=0;

	const char* GetSceneExtension();
	const char* GetEntityExtension();

	virtual void Sleep(int iMilliseconds=1)=0;
};

struct DLLBUILD Debugger
{
	struct DLLBUILD Breakpoint
	{	
		void* address;
		int   line;
		Script* script;
		bool breaked;

		Breakpoint():address(0),line(0),script(0),breaked(0){}

		bool operator==(const Breakpoint& iLineAddress){return address==iLineAddress.address && line==iLineAddress.line;}
	};

	std::vector<Breakpoint> allAvailableBreakpoints;
	std::vector<Breakpoint> breakpointSet; 

	bool   breaked;
	Breakpoint* currentBreakpoint;

	void* lastBreakedAddress;

	bool threadSuspendend;

	int debuggerCode;

	Script*			runningScript;
	unsigned char	runningScriptFunction;

	Debugger();

	virtual void RunDebuggeeFunction(Script* iDebuggee,unsigned char iFunctionIndex)=0;
	virtual void SuspendDebuggee()=0;
	virtual void ResumeDebuggee()=0;

	virtual void SetBreakpoint(Breakpoint&,bool)=0;

	virtual void BreakDebuggee(Breakpoint&)=0;
	virtual void ContinueDebuggee()=0;

	virtual int HandleHardwareBreakpoint(void*)=0;
	virtual void SetHardwareBreakpoint(Breakpoint&,bool)=0;

	virtual void PrintThreadContext(void*)=0;
};

struct DLLBUILD Renderer2D
{
	static const unsigned int COLOR_TAB_BACKGROUND=0x808080;
	static const unsigned int COLOR_TAB_SELECTED=0x0000FF;
	static const unsigned int COLOR_GUI_BACKGROUND = 0x707070;
	static const unsigned int COLOR_MAIN_BACKGROUND = 0x505050;
	static const unsigned int COLOR_TEXT=0xFFFFFF;
	static const unsigned int COLOR_TEXT_SELECTED=0x0000ff;
	static const unsigned int COLOR_TEXT_HOVERED=0x0000f1;

	struct DLLBUILD Caret
	{
		static const unsigned int BLINKRATE=300;

		GuiRect*		guiRect;

		unsigned int	blinkingRate;

		unsigned int	lastBlinkTime;
		bool			blinking;
		vec2			rect;
		vec2            position;

		vec2			newPosition;
		vec2            newRect;

		bool			enabled;

		Caret(Renderer2D*);

		virtual void set(GuiRect* iGuiRect,vec2 iPosition,vec2 iRect)=0;
		virtual void draw(Renderer2D*)=0;
		virtual void enable(bool)=0;
	};

	unsigned int	colorBackgroud;
	unsigned int	colorText;

	Tab*			tabContainer;

	Caret*			caret;     

	Renderer2D(Tab*);

	virtual void DrawText(const char* iText,float iX,float iY, float iWw,float iH,unsigned int iColor=COLOR_TEXT,float iAlignPosX=-1,float iAlignPosY=-1,bool iClip=true)=0;
	virtual void DrawText(const wchar_t* iText,float iX,float iY, float iWw,float iH,unsigned int iColor=COLOR_TEXT,float iAlignPosX=-1,float iAlignPosY=-1,bool iClip=true)=0;
	virtual void DrawRectangle(float iX,float iY, float iWw,float iH,unsigned int iColor,bool iFill=true)=0;
	virtual void DrawRectangle(vec4& iXYWH,unsigned int iColor,bool iFill=true)=0;
	virtual void DrawBitmap(GuiImage* bitmap,float x,float y, float w,float h)=0;
	 
	virtual void PushScissor(float x,float y,float w,float h)=0;
	virtual void PopScissor()=0;
	 
	virtual void Translate(float,float)=0;
	virtual void Identity()=0;
	 
	virtual vec2 MeasureText(const char*,int iSlen=-1)=0;
	virtual float GetFontSize()=0;
	virtual float GetFontHeight()=0;

	virtual void DrawCaret()=0;
	virtual void SetCaret(GuiRect* iGuiRect,vec2 iPosition,vec2 iRect)=0;
	virtual void EnableCaret(bool)=0;

	virtual float GetCharWidth(char iCharacter)=0;
};

struct DLLBUILD Renderer3D : Renderer3DBase
{
	std::list<GuiViewport*> viewports;
	
	Tab* tabContainer;

	Shader* FindShader(const char* name,bool exact);
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
	virtual void draw(Script*)=0;

	virtual void draw(EntityComponent*)=0;
	virtual void draw(Entity*)=0;

	virtual void ChangeContext()=0;

	virtual void Render(GuiViewport*,bool)=0;
	virtual void Render()=0;

	bool Block();
	bool Release();

	Renderer3D(Tab*);
	virtual ~Renderer3D(){};

	
};



struct DLLBUILD Game
{

};



int simple_shader(const char* name,int shader_type, const char* shader_src);
int create_program(const char* name,const char* vertexsh,const char* fragmentsh);

struct DLLBUILD ShaderOpenGL : Shader
{
	static Shader* Create(const char* shader_name,const char* pixel_shader,const char* fragment_shader);

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

struct DLLBUILD GuiRect : THierarchyVector<GuiRect>
{
	static const int TREEVIEW_ROW_HEIGHT=20;
	static const int TREEVIEW_ROW_ADVANCE=TREEVIEW_ROW_HEIGHT;
	static const int SCROLLBAR_WIDTH=20;
	static const int SCROLLBAR_TIP_HEIGHT=SCROLLBAR_WIDTH;
	static const int SCROLLBAR_AMOUNT=10;

	String name;

	vec4 fixed;
	vec4 rect;
	vec4 offset;
	vec2 alignPos;
	vec2 alignRect;

	unsigned int colorBackground;
	unsigned int colorForeground;
	unsigned int colorHovering;
	unsigned int colorPressing;
	unsigned int colorChecked;

	bool pressing;
	bool hovering;
	bool checked;

	bool active;

	int container;

	GuiRect* sibling[4];

	int	sequence;

	int animFrame;

	GuiScrollRect* clip;

	GuiRect(GuiRect* iParent=0,float ix=0, float iy=0, float iw=0,float ih=0,vec2 _alignPos=vec2(0,0),vec2 _alignRect=vec2(1,1));
	~GuiRect();

	virtual void Set(GuiRect* iParent=0,GuiRect* sibling=0,int sibIdx=0,int container=-1,float ix=0.0f, float iy=0.0f, float iw=0.0f,float ih=0.0f,float iAlignPosX=-1.0f,float iAlignPosY=-1.0f,float iAlignRectX=-1.0f,float iAlignRectY=-1.0f);

	void SetParent(GuiRect*);

	virtual void OnRecreateTarget(Tab*,void* data=0){}
	virtual void OnPaint(Tab*,void* data=0);
	virtual void OnEntitiesChange(Tab*,void* data=0);
	virtual void OnSize(Tab*,void* data=0);
	virtual void OnLMouseDown(Tab*,void* data=0);
	virtual void OnDLMouseDown(Tab*,void* data=0);
	virtual void OnLMouseUp(Tab*,void* data=0);
	virtual void OnRMouseUp(Tab*,void* data=0);
	virtual void OnMouseMove(Tab*,void* data=0);
	virtual void OnUpdate(Tab*,void* data=0);
	virtual void OnReparent(Tab*,void* data=0);
	virtual void OnSelected(Tab*,void* data=0);
	virtual void OnRender(Tab*,void* data=0);
	virtual void OnMouseWheel(Tab*,void* data=0);
	virtual void OnActivate(Tab*,void* data=0);
	virtual void OnDeactivate(Tab*,void* data=0);
	virtual void OnEntitySelected(Tab*,void* data=0);
	virtual void OnExpandos(Tab*,void* data=0);
	virtual void OnKeyDown(Tab*,void* data=0);
	virtual void OnKeyUp(Tab*,void* data=0);
	virtual void OnMouseEnter(Tab*,void* data=0);
	virtual void OnMouseExit(Tab*,void* data=0);
	virtual void OnEnterFocus(Tab*,void* data=0);
	virtual void OnExitFocus(Tab*,void* data=0);

	virtual void OnButtonPressed(Tab*,GuiButton*){}

	virtual void DrawBackground(Tab*);

	virtual void SelfRenderEnd(Tab*,bool&);

	virtual bool BeginSelfClip(Tab*);
	virtual void EndSelfClip(Tab*,bool&);

	virtual void SetClip(GuiScrollRect*);

	virtual GuiRect* GetRoot(); 
	GuiRootRect* GetRootRect(); 

	bool _contains(vec4& quad,vec2);

	void BroadcastToChilds(void (GuiRect::*func)(Tab*,void*),Tab*,void* data=0);
	void BroadcastToRoot(void (GuiRect::*func)(Tab*,void*),void* data=0);

	template<class C> void BroadcastTo(void (GuiRect::*func)(Tab*,void*),Tab* iTabContainer,void* iData=0)
	{
		C* isaC=dynamic_cast<C*>(this);

		if(isaC)
			(this->*func)(iTabContainer,iData);

		for(std::vector<GuiRect*>::iterator tRect=this->childs.begin();tRect!=this->childs.end();tRect++)
			((*tRect)->*func)(iTabContainer,iData);
	}

	GuiString* Container(const char* iText);

	GuiRect* Rect(float ix=0, float iy=0, float iw=0,float ih=0,float apx=0, float apy=0, float arx=1,float ary=1);

	GuiString* Text(String str,float ix=0, float iy=0, float iw=0,float ih=0,vec2 _alignText=vec2(-1,-1));
	GuiString* Text(String str,vec2 _alignPos=vec2(-1,-1),vec2 _alignRect=vec2(-1,-1),vec2 _alignText=vec2(-1,-1));

	GuiPropertyString* Property(const char* iDescription,void* iValuePointer1,unsigned int iValueType,void* iValuePointer2=0,unsigned int iValueParameter1=3,unsigned int iValueParameter2=2);
	
	GuiPropertyAnimationController* AnimationControllerProperty(AnimationController&);
	GuiPropertySlider* SliderProperty(const char* iDescription,float& ref,float& imin,float& imax);

	GuiViewport* Viewport(vec3 pos=vec3(100,100,100),vec3 target=vec3(0,0,0),vec3 up=vec3(0,0,1),bool perspective=true);
	GuiSceneViewer* SceneViewer();
	GuiEntityViewer* EntityViewer();
	GuiProjectViewer* ProjectViewer();
	GuiScriptViewer* ScriptViewer();
	GuiCompilerViewer* CompilerViewer();

	void AppendChild(GuiRect*);

	void DestroyChilds();

	template<class C> C* Create(int iSibling=-1,int iContainer=-1,float ix=0.0f, float iy=0.0f, float iw=0.0f,float ih=0.0f,float iAlignPosX=0,float iAlignPosY=0,float iAlignRectX=1,float iAlignRectY=1);
};

struct DLLBUILD GuiRootRect : GuiRect , TPoolVector<GuiRootRect>
{
	Tab* tabContainer;

	void OnSize(Tab*);

	GuiRootRect(Tab* t);
};

struct DLLBUILD GuiString : GuiRect
{
	vec2 alignText;
	std::string text;
	std::wstring wText;
	bool clipText;
	
	GuiString();

	void DrawTheText(Tab*,vec2 iOffset=vec2(0,0));

	virtual void OnPaint(Tab*,void* data=0);
};

struct DLLBUILD GuiButton : GuiString
{
	GuiButton();

	virtual void OnLMouseUp(Tab* tab,void* data=0);
	virtual void OnPaint(Tab* tab,void* data=0);
};

struct DLLBUILD GuiButtonFunc : GuiButton
{
	GuiButtonFunc(void (*iFunc)(void*)=0,void* iParam=0);

	void (*func)(void*);
	void* param;

	virtual void OnLMouseUp(Tab* tab,void* data=0);
};

struct DLLBUILD GuiButtonBool : GuiButton
{
	bool& referenceValue;
	int		updateMode;

	GuiButtonBool(bool& iBool):referenceValue(iBool),updateMode(-1){this->name="Button";}

	virtual void OnLMouseUp(Tab* tab,void* data=0);
};

struct DLLBUILD GuiScrollBar : GuiRect
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

	void OnLMouseDown(Tab* tab,void* data=0);
	void OnLMouseUp(Tab* tab,void* data=0);
	void OnMouseMove(Tab* tab,void* data=0);
	void OnPaint(Tab* tab,void* data=0);

	float GetContainerHeight();
	float GetContainerTop();
	float GetContainerBottom();
	float GetScrollerTop();
	float GetScrollerBottom();
	float GetScrollerHeight();

	void SetRect(GuiRect*);
};

struct DLLBUILD GuiScrollRect : GuiRect
{
	GuiScrollRect();
	~GuiScrollRect();

	float contentHeight;
	float width;

	GuiScrollBar	*scrollBar;

	bool isClipped;

	virtual void OnMouseWheel(Tab*,void* data=0);
	virtual void OnSize(Tab*,void* data=0);
};	

struct DLLBUILD GuiSlider : GuiRect
{
	float& referenceValue;

	float& minimum;
	float& maximum;

	GuiSlider(float& iRef,float& iMin,float& iMax):referenceValue(iRef),minimum(iMin),maximum(iMax){this->name="GuiSlider";}

	virtual void OnPaint(Tab*,void* data=0);
	virtual void OnMouseMove(Tab*,void* data=0);
	virtual void OnSize(Tab*,void* data=0);
	void DrawSliderTip(Tab*,void* data=0);
};

struct DLLBUILD GuiProperty : GuiRect
{
	String description;
};

struct DLLBUILD GuiPropertyString : GuiProperty
{
	static const unsigned int scDefaultParameter1=3;
	static const unsigned int scDefaultParameter2=2;

	enum 
	{
		STRING=0,
		BOOL,
		INT,
		FLOAT,
		VEC2,
		VEC3,
		VEC4,
		PTR,
		MAT4POS,
		ENTITYVECSIZE,
		ANIMATIONVECSIZE,
		ISBONECOMPONENT,
		FLOAT2MINUSFLOAT1,
		MAXVALUE
	};

	void* valuePointer1;
	void* valuePointer2;
	unsigned int valueType;
	unsigned char valueParameter1;
	unsigned char valueParameter2;

	GuiPropertyString(const char* iDescription,void* iValuePointer1,unsigned int iValueType,void* iValuePointer2=0,unsigned int iValueParameter1=scDefaultParameter1,unsigned int iValueParameter2=scDefaultParameter2);

	virtual void OnPaint(Tab*,void* data=0);
};

struct DLLBUILD GuiPropertySlider : GuiProperty
{
	GuiSlider slider;

	GuiPropertySlider(const char* iDescription,float& iRefVal,float& iMin,float& iMax);

	virtual void OnPaint(Tab*,void* data=0);
};

struct DLLBUILD GuiAnimationController : GuiRect
{
	AnimationController& animationController;

	GuiAnimationController(AnimationController&);

	GuiButtonBool play;
	GuiButtonBool stop;

	GuiSlider slider;

	virtual void OnMouseMove(Tab*,void* data=0);

	void OnButtonPressed(Tab* tabContainer,GuiButtonBool*);
};

struct DLLBUILD GuiPropertyAnimationController : GuiProperty
{
	GuiAnimationController guiAnimationController;

	GuiPropertyAnimationController(AnimationController&);
};

struct DLLBUILD GuiViewport : GuiRect , TPoolVector<GuiViewport>
{
	EditorEntity* rootEntity;

	GuiButtonFunc* playStopButton;

	mat4 projection;
	mat4 view;
	mat4 model;

	vec2 mouseold;

	void*	renderBitmap;
	void*	renderBuffer;

	unsigned int lastFrameTime;

	bool needsPicking;

	unsigned char pickedPixel[4];
	EditorEntity*		  pickedEntity;

	GuiViewport();
	~GuiViewport();

	virtual void OnPaint(Tab*,void* data=0);
	virtual void OnSize(Tab*,void* data=0);
	virtual void OnMouseWheel(Tab*,void* data=0);
	virtual void OnMouseMove(Tab*,void* data=0);
	virtual void OnActivate(Tab*,void* data=0);
	virtual void OnDeactivate(Tab*,void* data=0);
	virtual void OnReparent(Tab*,void* data=0);
	virtual void OnLMouseUp(Tab*,void* data=0);
};

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////GuiViewers///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

struct DLLBUILD GuiScriptViewer : GuiScrollRect , TPoolVector<GuiScriptViewer>
{
	struct DLLBUILD GuiPaper : GuiString
	{
		GuiScriptViewer* scriptViewer;

		vec2 textOffset;
		unsigned int lineCount;
		bool lineNumbers;

		GuiPaper();

		void DrawLineNumbers(Tab*);
		void DrawBreakpoints(Tab*);

		virtual void OnPaint(Tab*,void* data=0);
		virtual void OnLMouseDown(Tab*,void* data=0);
	};

	EditorScript* script;

	int			cursor;
	GuiPaper*	paper;

	bool		lineNumbers;

	GuiScriptViewer();

	void Open(Script*);
	bool Save();
	bool Compile();

	void OnKeyDown(Tab*,void* data=0);
	void OnKeyUp(Tab*,void* data=0);
	void OnLMouseDown(Tab*,void* data=0);
	void OnMouseMove(Tab*,void* data=0);
	void OnSize(Tab*,void* data=0);

	int CountScriptLines();
	void SetCaretPosition(Tab*);
	vec4 GetCaretPosition(Tab*);
};

struct DLLBUILD GuiCompilerViewer : GuiScrollRect , TPoolVector<GuiCompilerViewer>
{
	GuiCompilerViewer();

	bool ParseCompilerOutputFile(wchar_t*);
	void OnSize(Tab*,void* data=0);
};

struct DLLBUILD GuiSceneViewer : GuiScrollRect , TPoolVector<GuiSceneViewer>
{
	GuiSceneViewer();
	~GuiSceneViewer();

	EditorEntity* entityRoot;

	std::string sceneName;

	std::vector<EditorEntity*> selection;

	void OnPaint(Tab*,void* data=0);
	void OnLMouseDown(Tab*,void* data=0);
	void OnEntitiesChange(Tab*,void* data=0);
	void OnEntitySelected(Tab*,void* data=0);
	void OnRecreateTarget(Tab*,void* data=0);
	void OnRMouseUp(Tab*,void* data=0);
	void OnMouseWheel(Tab*,void* data=0);
	void OnKeyDown(Tab*,void* data=0);

	EditorEntity* GetHoveredRow(EditorEntity* node,vec2& mpos,vec2& pos,bool& oExpandos);
	void DrawNodes(Tab*,EditorEntity*,vec2&);
	int UpdateNodes(EditorEntity*);
	void UnselectNodes(EditorEntity*);
	void ExpandUntil(EditorEntity* iTarget);

	void Save(const char*);
	void Load(const char*);


};	

struct DLLBUILD GuiEntityViewer : GuiScrollRect , TPoolVector<GuiEntityViewer>
{
	GuiEntityViewer();
	~GuiEntityViewer();

	EditorEntity* entity;

	Tab* tabContainer;

	void OnActivate(Tab*,void* data=0);

	virtual void OnEntitySelected(Tab*,void* data=0);
	virtual void OnPaint(Tab*,void* data=0);
	virtual void OnExpandos(Tab*,void* data=0);
	virtual void OnMouseWheel(Tab*,void* data=0);

	bool ProcessMouseInput(vec2&,vec2&,GuiRect* node);
	void DrawNodes(Tab*,GuiRect*,vec2&);
	int CalcNodesHeight(GuiRect*);
};

struct DLLBUILD GuiConsoleViewer : GuiScrollRect
{
	GuiConsoleViewer();
	~GuiConsoleViewer();
};

struct DLLBUILD ResourceNode
{
	ResourceNode* parent;

	FilePath fileName;

	bool selectedLeft;
	bool selectedRight;
	int level;
	bool isDir;

	ResourceNode();
	~ResourceNode();
};

struct DLLBUILD ResourceNodeDir : ResourceNode
{
	bool expanded;

	std::list<ResourceNodeDir*> dirs;
	std::list<ResourceNode*>	files;

	ResourceNodeDir();
	~ResourceNodeDir();
};

struct DLLBUILD GuiProjectViewer : GuiRect , TPoolVector<GuiProjectViewer>
{
	struct DLLBUILD GuiProjectDirViewer : GuiScrollRect
	{
		ResourceNodeDir* rootResource;

		void DrawNodes(Tab*,ResourceNodeDir* node,vec2&,bool& terminated);
		ResourceNodeDir* GetHoveredRow(ResourceNodeDir* node,vec2& mpos,vec2& pos,bool& oExpandos);
		int CalcNodesHeight(ResourceNodeDir*);
		void UnselectNodes(ResourceNodeDir*);
		std::vector<ResourceNodeDir*> selectedDirs;

		void OnLMouseDown(Tab*,void* data=0);
		void OnPaint(Tab*,void* data=0);

	}dirViewer;

	struct DLLBUILD GuiProjectFileViewer : GuiScrollRect
	{
		ResourceNodeDir* rootResource;
		std::vector<ResourceNodeDir*> selectedDirs;
		std::vector<ResourceNode*> selectedFiles;

		void DrawNodes(Tab*,ResourceNodeDir* node,vec2&);
		ResourceNode* GetHoveredRow(ResourceNodeDir* node,vec2& mpos,vec2& pos,bool& oExpandos);
		int CalcNodesHeight(ResourceNodeDir*);
		void UnselectNodes(ResourceNodeDir*);
		
		void OnLMouseDown(Tab*,void* data=0);
		void OnPaint(Tab*,void* data=0);
		void OnRMouseUp(Tab*,void* data=0);
		void OnDLMouseDown(Tab*,void* data=0);
	}fileViewer;

	struct DLLBUILD GuiProjectDataViewer : GuiScrollRect
	{
		
	}resourceViewer;


	ResourceNodeDir rootResource;

	bool splitterLeft;
	bool splitterRight;
	float hotspotDist;

	GuiProjectViewer();
	~GuiProjectViewer();


	void OnPaint(Tab*,void* data=0);
	void OnLMouseDown(Tab*,void* data=0);
	void OnLMouseUp(Tab*,void* data=0);
	void OnMouseMove(Tab*,void* data=0);
	void OnReparent(Tab*,void* data=0);
	void OnActivate(Tab*,void* data=0);
	void OnDeactivate(Tab*,void* data=0);
	void OnSize(Tab*,void* data=0);

	void findResources(std::vector<ResourceNode*>& oResultArray,ResourceNode* iResourceNode,const char* iExtension)
	{
		if(iResourceNode->isDir)
		{
			ResourceNodeDir* tResourceNodeDir=(ResourceNodeDir*)iResourceNode;

			for(std::list<ResourceNode*>::iterator nCh=tResourceNodeDir->files.begin();nCh!=tResourceNodeDir->files.end();nCh++)
				this->findResources(oResultArray,*nCh,iExtension);

			for(std::list<ResourceNodeDir*>::iterator nCh=tResourceNodeDir->dirs.begin();nCh!=tResourceNodeDir->dirs.end();nCh++)
				this->findResources(oResultArray,*nCh,iExtension);
		}
		else if(iResourceNode->fileName.PointedExtension()==iExtension)
				oResultArray.push_back(iResourceNode);
	}

	std::vector<ResourceNode*> findResources(const char* iExtension)
	{
		std::vector<ResourceNode*> oResultArray;

		this->findResources(oResultArray,&this->rootResource,iExtension);

		return oResultArray;
	}
};

struct DLLBUILD WindowData
{
	float width;
	float height;

	std::list<WindowData*> siblings[4];

	virtual void LinkSibling(WindowData* t,int pos)=0;
	virtual void UnlinkSibling(WindowData* t=0)=0;
	virtual WindowData* FindSiblingOfSameSize()=0;
	virtual int FindSiblingPosition(WindowData* t)=0;
	virtual bool FindAndGrowSibling()=0;

	virtual void OnSize()=0;
	virtual void OnWindowPosChanging()=0;
};

struct DLLBUILD GuiImage
{
	int width,height;
	int bpp;

	GuiImage();
	~GuiImage();

	virtual void Release()=0;
	virtual bool Fill(Renderer2D*,unsigned char* iData,float iWidth,float iHeight)=0;
};

struct DLLBUILD DrawInstance
{
	int		 code;
	bool	 frame;
	GuiRect* rect;

	bool remove;

	DrawInstance(int a,bool b,GuiRect* c,bool iRemove):code(a),frame(b),rect(c),remove(true){}
};

struct DLLBUILD Tab : TPoolVector<Tab>
{
	WindowData* windowData;

	Container* parentWindowContainer;

	static const int CONTAINER_HEIGHT=30;
	static const int TAB_WIDTH=80;
	static const int TAB_HEIGHT=25;

	static const int CONTAINER_ICON_WH=20;
	static const int CONTAINER_ICON_STRIDE=CONTAINER_ICON_WH*4;

	static unsigned char rawUpArrow[];
	static unsigned char rawRightArrow[];
	static unsigned char rawDownArrow[];
	static unsigned char rawFolder[];
	static unsigned char rawFile[];

	GuiRootRect	tabs;

	GuiRect* focused;

	Splitter* splitterContainer;

	Renderer2D *renderer2D;
	Renderer3D *renderer3D;

	GuiImage* iconUp;
	GuiImage* iconRight;
	GuiImage* iconDown;
	GuiImage* iconFolder;
	GuiImage* iconFile;

	unsigned int selected;
	bool mouseDown;
	bool isRender;
	bool recreateTarget;
	bool resizeTarget;

	std::list<DrawInstance*> drawInstances;

	Task*	 taskDraw;

	float mousex,mousey;

	unsigned int lastFrameTime;

	Thread* threadRender;

	Tab(float x,float y,float w,float h);
	virtual ~Tab();

	operator Tab& (){return *this;}

	virtual void OnGuiPaint(void* data=0);
	virtual void OnGuiSize(void* data=0);
	virtual void OnWindowPosChanging(void* data=0);
	virtual void OnGuiLMouseDown(void* data=0);
	virtual void OnGuiDLMouseDown(void* data=0);
	virtual void OnGuiLMouseUp(void* data=0);
	virtual void OnGuiMouseMove(void* data=0);
	virtual void OnGuiRMouseUp(void* data=0);
	virtual void OnGuiUpdate(void* data=0);
	virtual void OnGuiRender(void* data=0);
	virtual void OnGuiMouseWheel(void* data=0);
	virtual void OnResizeContainer(void* data=0);
	virtual void OnEntitiesChange(void* data=0);
	virtual void OnGuiActivate(void* data=0);
	virtual void OnGuiDeactivate(void* data=0);
	virtual void OnGuiEntitySelected(void* data=0);
	virtual void OnGuiKeyDown(void* data=0);
	virtual void OnGuiKeyUp(void* data=0);

	virtual void DrawFrame()=0;

	

	virtual void OnGuiRecreateTarget(void* data=0);

	GuiRect* GetSelected();

	void BroadcastToSelected(void (GuiRect::*func)(Tab*,void*),void* data=0);
	void BroadcastToAll(void (GuiRect::*func)(Tab*,void*),void* data=0);
	template<class C> void BroadcastToSelected(void (GuiRect::*func)(Tab*,void*),void*);
	template<class C> void BroadcastToAll(void (GuiRect::*func)(Tab*,void*),void*);
	static void BroadcastToPoolSelecteds(void (GuiRect::*func)(Tab*,void*),void* data=0)
	{
		for(std::vector<Tab*>::iterator tabContainer=TPoolVector<Tab>::pool.begin();tabContainer!=TPoolVector<Tab>::pool.end();tabContainer++)
			(*tabContainer)->BroadcastToSelected(func,data);
	}

	void Draw();

	virtual bool BeginDraw()=0;
	virtual void EndDraw()=0;

	DrawInstance* SetDraw(int iCode=1/*1:allTab,2:rect*/,bool iFrame=true,GuiRect* iRect=0,bool iRemove=true);

	virtual int TrackGuiSceneViewerPopup(bool iSelected)=0;
	virtual int TrackTabMenuPopup()=0;
	virtual int TrackProjectFileViewerPopup(ResourceNode*)=0;

	void SetSelection(GuiRect* iRect);

	virtual void SetCursor(int)=0;

	void SetFocus(GuiRect*);
	GuiRect* GetFocus();
};

struct DLLBUILD Splitter 
{
	Tab* currentTabContainer;

	Tab* floatingTabRef;
	Tab* floatingTab;
	Tab* floatingTabTarget;
	int  floatingTabRefTabIdx;
	int  floatingTabRefTabCount;
	int  floatingTabTargetAnchorPos;
	int	 floatingTabTargetAnchorTabIndex;

	const int   splitterSize;
	char*		splitterCursor;

	Splitter();
	~Splitter();

	virtual void CreateFloatingTab(Tab*)=0;
	virtual void DestroyFloatingTab()=0;
};


struct DLLBUILD Container
{
	std::vector<Tab*> tabContainers;

	WindowData* window;
	Splitter* splitter;

	int resizeDiffHeight;
	int resizeDiffWidth;
	int resizeEnumType;
	int resizeCheckWidth;
	int resizeCheckHeight;

	Container();

	virtual void OnSizing()=0;
	virtual void OnSize()=0;

	virtual Tab* CreateTabContainer(float x,float y,float w,float h)=0;

	template<class GuiViewerDerived> GuiViewerDerived* SpawnViewer(Tab* iTabContainer=0,bool skipExist=true)
	{
		Tab* tTabContainer=iTabContainer ? iTabContainer : tabContainers[0];

		if(GuiViewerDerived::pool.empty())
			return tabContainers[0]->tabs.Create<GuiViewerDerived>();
		else if(skipExist)
			return GuiViewerDerived::pool.front();
		else
			return tabContainers[0]->tabs.Create<GuiViewerDerived>();
	}
};


struct DLLBUILD MainContainer
{
	std::vector<Container*> containers;

	virtual Container* CreateContainer()=0;
};

struct DLLBUILD Subsystem
{
	virtual bool Execute(String iPath,String iCmdLine,String iOutputFile="",bool iInput=false,bool iError=false,bool iOutput=false)=0;
	virtual unsigned int FindProcessId(String iProcessName)=0;
	virtual unsigned int FindThreadId(unsigned int iProcessId,String iThreadName)=0;
};





struct DLLBUILD Compiler
{
	String compilerPath;
	String linkerPath;
	String includePath;
	String libPath;
	String runBefore;
	String runAfter;

	virtual bool Compile(Script*)=0;
	virtual String ComposeMS(Script*)=0;
	virtual String ComposeMingW(Script*)=0;
	virtual bool LoadScript(Script*)=0;
	virtual bool UnloadScript(Script*)=0;
	virtual bool CreateAndroidTarget()=0;
	virtual String CreateRandomDir(String iDirWhere)=0;
	
};

struct DLLBUILD EditorProperties
{
	GuiString properties;

	EditorProperties();

	virtual void OnPropertiesCreate(){};
	virtual void OnResourcesCreate(){};
	virtual void OnPropertiesUpdate(Tab*){};
};

template<class T> struct DLLBUILD EditorObject : T , EditorProperties{};

struct DLLBUILD EditorEntity : EditorObject<Entity> 
{
	bool					selected;
	bool					expanded;
	int						level;

	EditorEntity();

	void OnPropertiesCreate();

	void SetParent(Entity*);
	void SetLevel(EditorEntity*);

	template<class C> C* CreateComponent()
	{
		C* component=this->Entity::CreateComponent<C>();
		component->OnResourcesCreate();
		component->OnPropertiesCreate();
		return component;
	}

	void OnPropertiesUpdate(Tab*);
};
				

struct DLLBUILD EditorAnimationController : EditorObject<AnimationController>
{
	float oldCursor;
	GuiPropertyAnimationController*  guiPropertyAnimationController;

	void OnPropertiesCreate();
	void OnPropertiesUpdate(Tab*);

};

struct DLLBUILD EditorMesh : EditorObject<Mesh>
{
	void OnPropertiesCreate(); 
	void OnPropertiesUpdate(Tab*);
};
struct DLLBUILD EditorRoot : EditorObject<Root>
{
	void OnPropertiesCreate(); 
	void OnPropertiesUpdate(Tab*);
};
struct DLLBUILD EditorSkeleton : EditorObject<Skeleton>
{
	void OnPropertiesCreate(); 
	void OnPropertiesUpdate(Tab*);
};
struct DLLBUILD EditorGizmo : EditorObject<Gizmo>
{
	void OnPropertiesCreate(); 
	void OnPropertiesUpdate(Tab*);
};
struct DLLBUILD EditorAnimation : EditorObject<Animation>
{
	void OnPropertiesCreate(); 
	void OnPropertiesUpdate(Tab*);
};
struct DLLBUILD EditorBone : EditorObject<Bone>
{
	void OnPropertiesCreate(); 
	void OnPropertiesUpdate(Tab*);
};
struct DLLBUILD EditorLight : EditorObject<Light>
{
	void OnPropertiesCreate(); 
	void OnPropertiesUpdate(Tab*);
};
struct DLLBUILD EditorScript : EditorObject<Script>
{
	GuiButtonFunc* buttonLaunch;

	GuiScriptViewer* scriptViewer;

	EditorScript();

	void OnPropertiesCreate(); 
	void OnPropertiesUpdate(Tab*);
	void OnResourcesCreate();

	//for the debugger
	void update()
	{
		this->runtime ? EngineIDE::instance->debugger->RunDebuggeeFunction(this,1),true : false;
	}
};
struct DLLBUILD EditorCamera : EditorObject<Camera>
{
	void OnPropertiesCreate(); 
	void OnPropertiesUpdate(Tab*);
};
struct DLLBUILD EditorSkin : EditorObject<Skin>
{
	void OnPropertiesCreate(); 
	void OnPropertiesUpdate(Tab*);
};


namespace Serialization
{
	const unsigned char Root=0;
	const unsigned char Skeleton=1;
	const unsigned char Animation=2;
	const unsigned char Gizmo=3;
	const unsigned char AnimationController=4;
	const unsigned char Bone=5;
	const unsigned char Light=6;
	const unsigned char Mesh=7;
	const unsigned char Script=8;
	const unsigned char Camera=9;
	const unsigned char Unknown=10;
};


#endif //INTERFACES_H



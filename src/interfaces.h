#ifndef INTERFACES_H
#define INTERFACES_H

#include "entities.h"

struct DLLBUILD EditorEntity;

struct DLLBUILD Renderer2D;
struct DLLBUILD Renderer3D;

struct DLLBUILD GuiRect;
struct DLLBUILD GuiRootRect;
struct DLLBUILD GuiContainer;
template<typename T> struct DLLBUILD GuiContainerRow;
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
struct DLLBUILD GuiPanel;
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

struct DLLBUILD DrawInstance;
struct DLLBUILD Tab;
struct DLLBUILD Container;
struct DLLBUILD Splitter;
struct DLLBUILD MainContainer;
struct DLLBUILD ResourceNodeDir;
struct DLLBUILD Compiler;
struct DLLBUILD Subsystem;
struct DLLBUILD Debugger;
struct DLLBUILD StringEditor;
struct DLLBUILD EditorScript;
struct DLLBUILD ResourceNode;
struct DLLBUILD PluginSystem;

//entity forward declaration

struct DLLBUILD Entity;
struct DLLBUILD AnimationController;
struct DLLBUILD Script;

//---root menus----
#define MENUFILE 0
#define MENUPLUGINS 1
#define MENUMAX 2
//---MENUFILE childs----
	//#define MENUCONFIGUREPLUGINS 0
//---MENUPLUGINS childs----
#define MENUCONFIGUREPLUGINS 0

#define MAX_TOUCH_INPUTS 10

#define ENABLE_RENDERER 1

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

struct DLLBUILD Ide : TStaticInstance<Ide>
{
	Timer*					timer;
	MainContainer*			mainAppWindow;
	Compiler*				compiler;
	Subsystem*				subsystem;
	Debugger*				debugger;
	StringEditor*			stringEditor;
	PluginSystem*			pluginSystem;

	InputManager            inputManager;
	
	FilePath				folderProject;
	FilePath				pathExecutable;
	FilePath				folderAppData;
	
	unsigned int            caretLastTime;
	unsigned int			processId;
	unsigned int			processThreadId;
	
	Ide();
	virtual ~Ide();

	virtual void Run()=0;

	virtual void ScanDir(String,ResourceNodeDir*)=0;

	String GetSceneExtension();
	String GetEntityExtension();

	virtual void Sleep(int iMilliseconds=1)=0;

	static Ide* GetInstance();
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

	int sleep;

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

struct DLLBUILD StringEditor
{
	static const unsigned int BLINKRATE=300;

	enum
	{
		CARET_DONTCARE=0,
		CARET_RECALC,
		CARET_MOUSEPOS,
		CARET_CANCEL,
		CARET_BACKSPACE,
		CARET_ADD,
		CARET_ARROWLEFT,
		CARET_ARROWRIGHT,
		CARET_ARROWUP,
		CARET_ARROWDOWN,
		CARET_MAX
	};

	struct DLLBUILD Cursor
	{
		const wchar_t*  cursor;
		vec2			rowcol;
		vec4			caret;

		Cursor();
	};

	GuiString*		string;
	Tab*			tab;

	unsigned int	blinkingRate;
	unsigned int	lastBlinkTime;
	bool			blinking;

	vec4			newCaret;

	bool			enabled;

	bool			recalcBackground;

	StringEditor();

	bool EditText(unsigned int iCaretOp,void* iParam);

	void Bind(GuiString* iString);
	void Enable(bool);
	
	virtual void Draw(Tab*)=0;
};


struct DLLBUILD Game
{

};



struct GuiEvent
{
	Tab* tab;
	GuiRect* sender;
	GuiRect* target;
	void (GuiRect::*func)(const GuiEvent&);
	void* data;

	GuiEvent(Tab* iTab,GuiRect* iSender=0,GuiRect* iTarget=0,void (GuiRect::*iFunction)(const GuiEvent&)=0,void* iData=0);
};

struct DLLBUILD GuiRect : THierarchyVector<GuiRect>
{
	static const int ROW_HEIGHT=20;
	static const int ROW_ADVANCE=GuiRect::ROW_HEIGHT;

	static const int COLOR_BACK=0x505050;
	static const int COLOR_HOVERED=0xb845d8;
	static const int COLOR_PRESSED=0x6c45d8;
	static const int COLOR_CHECKED=0x45d8b0;

	String name;

	struct Edges{float *left,*top,*right,*bottom;};
	Edges refedges;
	vec2 minimums;
	vec4 offsets;
	vec4 scalars;
	vec4 fixed;
	vec4 edges;
	vec4 rect;
	vec2 content;

	unsigned int colorBackground;
	unsigned int colorHovering;
	unsigned int colorPressing;
	unsigned int colorChecked;

	bool pressing;
	bool hovering;
	bool checked;

	bool active;
	void* userData;

	GuiScrollRect* clip;

	GuiRect(GuiRect* iParent=0,float ix=0, float iy=0, float iw=0,float ih=0,vec2 _alignPos=vec2(0,0),vec2 _alignRect=vec2(1,1));
	virtual ~GuiRect();

	virtual void SetEdges(float* iLeft=0,float* iTop=0,float* iRight=0,float* iBottom=0);

	virtual void SetParent(GuiRect*);

	virtual void OnSizePre();
	virtual void OnSizePost();

	virtual void Insert(GuiRect* iProperty);

	virtual void OnRecreateTarget(const GuiEvent&){}
	virtual void OnPaint(const GuiEvent&);
	virtual void OnEntitiesChange(const GuiEvent&);
	virtual void OnSize(const GuiEvent&);
	virtual void OnLMouseDown(const GuiEvent&);
	virtual void OnDLMouseDown(const GuiEvent&);
	virtual void OnLMouseUp(const GuiEvent&);
	virtual void OnRMouseUp(const GuiEvent&);
	virtual void OnMouseMove(const GuiEvent&);
	virtual void OnUpdate(const GuiEvent&);
	virtual void OnReparent(const GuiEvent&);
	virtual void OnSelected(const GuiEvent&);
	virtual void OnRender(const GuiEvent&);
	virtual void OnMouseWheel(const GuiEvent&);
	virtual void OnActivate(const GuiEvent&);
	virtual void OnDeactivate(const GuiEvent&);
	virtual void OnEntitySelected(const GuiEvent&);
	virtual void OnExpandos(const GuiEvent&);
	virtual void OnKeyDown(const GuiEvent&);
	virtual void OnKeyUp(const GuiEvent&);
	virtual void OnMouseEnter(const GuiEvent&);
	virtual void OnMouseExit(const GuiEvent&);
	virtual void OnEnterFocus(const GuiEvent&);
	virtual void OnExitFocus(const GuiEvent&);

	virtual void OnButtonPressed(const GuiEvent&){}

	virtual void DrawBackground(Tab*);

	virtual GuiRect* GetRoot();
	virtual GuiRootRect* GetRootRect();

	virtual void SetClip(GuiScrollRect*);
	virtual void BeginClip(Tab*);
	virtual void EndClip(Tab*);

	bool Contains(vec4& quad,vec2);

	unsigned int GetColor();

	virtual void BroadcastToChilds(void (GuiRect::*func)(const GuiEvent&),const GuiEvent&);
	virtual void BroadcastToRoot(void (GuiRect::*func)(const GuiEvent&),const GuiEvent&);

	template<class C> void BroadcastTo(void (GuiRect::*func)(const GuiEvent&),const GuiEvent& iMsg)
	{
		C* isaC=dynamic_cast<C*>(this);

		if(isaC)
			(this->*func)(iMsg);

		for(std::vector<GuiRect*>::iterator tRect=this->childs.begin();tRect!=this->childs.end();tRect++)
			((*tRect)->*func)(iMsg);
	}

	GuiContainer* Container(String iText);
	template<typename T> GuiContainerRow<T>* ContainerRow(T);

	GuiString* Text(String str);

	GuiPropertyString* Property(String iDescription,void* iValuePointer1,unsigned int iValueType,void* iValuePointer2=0,unsigned int iValueParameter1=3,unsigned int iValueParameter2=2);

	GuiPropertyAnimationController* AnimationControllerProperty(AnimationController&);
	GuiPropertySlider* SliderProperty(String iDescription,float& ref,float& imin,float& imax);

	GuiViewport* Viewport(vec3 pos=vec3(100,100,100),vec3 target=vec3(0,0,0),vec3 up=vec3(0,0,1),bool perspective=true);
	GuiSceneViewer* SceneViewer();
	GuiEntityViewer* EntityViewer();
	GuiProjectViewer* ProjectViewer();
	GuiScriptViewer* ScriptViewer();
	GuiCompilerViewer* CompilerViewer();
	GuiPanel* Panel();

	void DestroyChilds();

	template<class C> C* Create(int iSibling=-1,int iContainer=-1,float ix=0.0f, float iy=0.0f, float iw=0.0f,float ih=0.0f,float iAlignPosX=0,float iAlignPosY=0,float iAlignRectX=1,float iAlignRectY=1);
};

struct DLLBUILD GuiRootRect : GuiRect , TPoolVector<GuiRootRect>
{
	Tab* tab;

	GuiRootRect(Tab* t);
	~GuiRootRect();

	virtual void OnSize(const GuiEvent&);
};

struct DLLBUILD GuiString : GuiRect
{
	static const unsigned int COLOR_TEXT=0xFFFFFF;

	struct DLLBUILD GuiStringBase
	{
	private:
		String* _______text;
		bool ________dtor;
	public:
		GuiStringBase();
		~GuiStringBase();
	
		void SetStringMode(String&,bool isReference);

		String& Text();

		operator String&();
		String& operator=(const String& iString);
		String* operator->();
	};

	GuiStringBase text;

	vec4 margins;
	vec4 textEdges;
	vec2 textRect;

	vec2 textSpot;
	vec2 textAlign;

	bool canEdit;
	bool adaptRect;

	int textColor;

	StringEditor::Cursor* cursor;

	GuiString();
	virtual ~GuiString();

	void SetStringMode(String&,bool isReference);

	virtual void OnSizePre(Tab*);
	void DrawTheText(Tab*);

	bool ParseKeyInput(const GuiEvent&);

	virtual void OnPaint(const GuiEvent&);
	virtual void OnLMouseDown(const GuiEvent&);
	virtual void OnKeyDown(const GuiEvent&);
	virtual void OnSize(const GuiEvent&);
};

struct DLLBUILD GuiContainer : GuiString
{
	bool		state;

	GuiContainer();

	virtual void OnSizePre(Tab*);
	virtual void OnSizePost(Tab*);

	void Insert(GuiRect* iProperty);

	void BroadcastToChilds(void (GuiRect::*iFunction)(const GuiEvent&),const GuiEvent&);
	
	void OnLMouseDown(const GuiEvent&);
	void OnSize(const GuiEvent&);
	void OnPaint(const GuiEvent&);
};

template<typename TPointer> struct DLLBUILD GuiContainerRow : GuiContainer
{
	TPointer rowData;

	GuiContainerRow();

	void UnselectAll(GuiRect*);
	virtual void OnSizePre(Tab*);
	void DrawBackground(const GuiEvent&);
	void OnPaint(const GuiEvent&);
	void OnSize(const GuiEvent&);
	void OnMouseMove(const GuiEvent&);
	void OnLMouseUp(const GuiEvent&);
	vec4 GetRowRectangle();
};


struct DLLBUILD GuiButton : GuiString
{
	GuiButton();

	virtual void OnLMouseUp(const GuiEvent&);
};

struct DLLBUILD GuiButtonFunc : GuiButton
{
	GuiButtonFunc(void (*iFunc)(void*)=0,void* iParam=0);

	void (*func)(void*);
	void* param;

	virtual void OnLMouseUp(const GuiEvent&);
};

struct DLLBUILD GuiButtonBool : GuiButton
{
	bool& referenceValue;
	int		updateMode;

	GuiButtonBool(bool& iBool);

	virtual void OnLMouseUp(const GuiEvent&);
};

struct DLLBUILD GuiScrollBar : GuiRect
{
	static const int SCROLLBAR_TICK=20;
	static const int SCROLLBAR_TIP_SIZE=SCROLLBAR_TICK;
	static const int SCROLLBAR_SCROLL_AMOUNT=10;

	enum
	{
		SCROLLBAR_VERTICAL=0,
		SCROLLBAR_HORIZONTAL,
		SCROLLBAR_INVALID
	};

	unsigned int scrollbarType;

	float scrollerPosition;
	float scrollerRatio;
	float scrollerPressed;

	GuiRect* guiRect;

	GuiScrollBar(unsigned int iScrollbarType=SCROLLBAR_VERTICAL);
	~GuiScrollBar();

	void SetType(unsigned int iScrollbarType);

	void SetScrollerRatio(float contentHeight,float containerHeight);
	void SetScrollerPosition(float contentHeight);
	void Scroll(Tab*,float upOrDown);
	bool IsVisible();

	void OnLMouseDown(const GuiEvent&);
	void OnLMouseUp(const GuiEvent&);
	void OnMouseMove(const GuiEvent&);
	void OnPaint(const GuiEvent&);

	float GetContainerLength();
	float GetContainerBegin();
	float GetContainerEnd();
	float GetScrollerBegin();
	float GetScrollerEnd();
	float GetScrollerLength();

	void SetRect(GuiRect*);
};

struct DLLBUILD GuiScrollRect : GuiRect
{
	GuiScrollRect();
	~GuiScrollRect();

	GuiScrollBar	vScrollbar;
	GuiScrollBar	hScrollbar;

	bool clipped;

	vec2 GetClippedMouse(vec2&);

	virtual void BroadcastToChilds(void (GuiRect::*iFunction)(const GuiEvent&),const GuiEvent&);

	virtual void OnSize(const GuiEvent&);
	virtual void OnPaint(const GuiEvent&);
	virtual void OnLMouseDown(const GuiEvent&);
	virtual void OnMouseMove(const GuiEvent&);
	virtual void OnMouseWheel(const GuiEvent&);

	virtual void BeginClip(Tab*);
	virtual void EndClip(Tab*);
};

struct DLLBUILD GuiSlider : GuiRect
{
	float& referenceValue;

	float& minimum;
	float& maximum;

	GuiSlider(float& iRef,float& iMin,float& iMax):referenceValue(iRef),minimum(iMin),maximum(iMax){this->name=L"GuiSlider";}

	virtual void OnPaint(const GuiEvent&);
	virtual void OnMouseMove(const GuiEvent&);
	virtual void OnSize(const GuiEvent&);

	void DrawSliderTip(Tab*,void* data=0);
};

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////GuiProperty///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

struct DLLBUILD GuiProperty : GuiRect
{
	GuiString description;

	GuiProperty();
	virtual ~GuiProperty();
};

struct DLLBUILD GuiPropertyString : GuiProperty
{
	static const unsigned int scDefaultParameter1=3;
	static const unsigned int scDefaultParameter2=2;

	enum
	{
		STRING=0,
		BOOL,
		BOOLPTR,
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
		VEC32MINUSVEC31,
		MAXVALUE
	};

	void* valuePointer1;
	void* valuePointer2;
	unsigned int valueType;
	unsigned char valueParameter1;
	unsigned char valueParameter2;

	GuiString value;

	GuiPropertyString(String iDescription,void* iValuePointer1,unsigned int iValueType,void* iValuePointer2=0,unsigned int iValueParameter1=scDefaultParameter1,unsigned int iValueParameter2=scDefaultParameter2);
	~GuiPropertyString();

	virtual void OnPaint(const GuiEvent&);
	virtual void OnSize(const GuiEvent&);

	virtual void RefreshReference(Tab*);
};

struct DLLBUILD GuiPropertySlider : GuiProperty
{
	GuiSlider slider;

	GuiPropertySlider(String iDescription,float& iRefVal,float& iMin,float& iMax);

	virtual void OnPaint(const GuiEvent&);
};

struct DLLBUILD GuiAnimationController : GuiRect
{
	AnimationController& animationController;

	GuiAnimationController(AnimationController&);

	GuiButtonBool play;
	GuiButtonBool stop;

	GuiSlider slider;

	virtual void OnMouseMove(const GuiEvent&);

	void OnButtonPressed(const GuiEvent&);
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

	unsigned char*	renderBuffer;

	unsigned int lastFrameTime;

	bool needsPicking;

	unsigned char pickedPixel[4];
	EditorEntity*		  pickedEntity;

	DrawInstance* renderDrawInstance;

	unsigned int renderFps;

	GuiViewport();
	virtual ~GuiViewport();

	virtual void OnPaint(const GuiEvent&);
	virtual void OnSize(const GuiEvent&);
	virtual void OnMouseWheel(const GuiEvent&);
	virtual void OnMouseMove(const GuiEvent&);
	virtual void OnActivate(const GuiEvent&);
	virtual void OnDeactivate(const GuiEvent&);
	virtual void OnReparent(const GuiEvent&);
	virtual void OnLMouseUp(const GuiEvent&);

	virtual void Render(Tab*)=0;
	virtual void DrawBuffer(Tab*,vec4&)=0;
};

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////GuiViewers///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

struct DLLBUILD GuiScriptViewer : GuiScrollRect , TPoolVector<GuiScriptViewer>
{
	EditorScript* script;

	GuiString editor;
	GuiString lines;

	bool		lineNumbers;
	int			lineCount;

	GuiScriptViewer();
	~GuiScriptViewer();

	void Open(Script*);
	bool Save();
	bool Compile();

	void OnKeyDown(const GuiEvent&);
	void OnKeyUp(const GuiEvent&);
	void OnLMouseDown(const GuiEvent&);
	void OnMouseMove(const GuiEvent&);
	void OnSize(const GuiEvent&);
	void OnDeactivate(const GuiEvent&);

	int CountScriptLines();

	void DrawLineNumbers(Tab*);
	void DrawBreakpoints(Tab*);
	
};

struct DLLBUILD GuiCompilerViewer : GuiScrollRect , TPoolVector<GuiCompilerViewer>
{
	GuiCompilerViewer();

	bool ParseCompilerOutputFile(String);

	void OnSize(const GuiEvent&);
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
	virtual ~ResourceNode();
};

struct DLLBUILD ResourceNodeDir : ResourceNode
{
	bool expanded;

	std::list<ResourceNodeDir*> dirs;
	std::list<ResourceNode*> files;

	GuiContainerRow<ResourceNodeDir*> directoryViewerRow;

	ResourceNodeDir();
	virtual ~ResourceNodeDir();
};

struct DLLBUILD GuiPanel : GuiScrollRect , TPoolVector<GuiPanel>
{
	GuiPanel();
};


struct DLLBUILD GuiSceneViewer : GuiScrollRect , TPoolVector<GuiSceneViewer>
{
	Scene						scene;
	EditorEntity*&				entityRoot;
	std::vector<EditorEntity*>	selection;

	GuiSceneViewer();
	~GuiSceneViewer();

	void OnLMouseDown(const GuiEvent&);
	void OnEntitiesChange(const GuiEvent&);
	void OnEntitySelected(const GuiEvent&);
	void OnRMouseUp(const GuiEvent&);
	void OnKeyDown(const GuiEvent&);

	void Save(String);
	void Load(String);
};

struct DLLBUILD GuiEntityViewer : GuiScrollRect , TPoolVector<GuiEntityViewer>
{
	GuiEntityViewer();
	~GuiEntityViewer();

	EditorEntity* entity;

	Tab* tabContainer;

	void OnActivate(const GuiEvent&);

	virtual void OnEntitySelected(const GuiEvent&);
	virtual void OnExpandos(const GuiEvent&);
};

struct DLLBUILD GuiConsoleViewer : GuiScrollRect
{
	GuiConsoleViewer(){}
	~GuiConsoleViewer(){}
};

struct DLLBUILD GuiProjectViewer : GuiRect , TPoolVector<GuiProjectViewer>
{
	struct DLLBUILD DirViewer : GuiScrollRect
	{
		GuiProjectViewer* projectViewer;

		ResourceNodeDir* projectDirectory;

		std::vector<ResourceNodeDir*> selectedDirs;

		void OnLMouseDown(const GuiEvent&);
	};

	struct DLLBUILD FileViewer : GuiScrollRect
	{
		GuiProjectViewer* projectViewer;

		ResourceNodeDir* currentDirectory;

		std::vector<ResourceNodeDir*> selectedDirs;
		std::vector<ResourceNode*> selectedFiles;

		void SetDirectory(ResourceNodeDir*);

		void OnLMouseDown(const GuiEvent&);
		void OnRMouseUp(const GuiEvent&);
		void OnDLMouseDown(const GuiEvent&);
	};

	struct DLLBUILD DataViewer : GuiScrollRect
	{
		GuiProjectViewer* projectViewer;
	};

	DirViewer dirViewer;
	FileViewer fileViewer;
	DataViewer resViewer;

	ResourceNodeDir* projectDirectory;

	bool splitterLeftActive;
	bool splitterRightActive;

	float splitterLeft;
	float splitterRight;

	GuiProjectViewer();
	~GuiProjectViewer();

	void OnPaint(const GuiEvent&);
	void OnLMouseDown(const GuiEvent&);
	void OnLMouseUp(const GuiEvent&);
	void OnMouseMove(const GuiEvent&);
	void OnReparent(const GuiEvent&);
	void OnActivate(const GuiEvent&);
	void OnDeactivate(const GuiEvent&);
	void OnSize(const GuiEvent&);

	void findResources(std::vector<ResourceNode*>& oResultArray,ResourceNode* iResourceNode,String iExtension)
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

	std::vector<ResourceNode*> findResources(String iExtension)
	{
		std::vector<ResourceNode*> oResultArray;

		this->findResources(oResultArray,this->projectDirectory,iExtension);

		return oResultArray;
	}
};

struct DLLBUILD WindowData
{
	std::list<WindowData*> siblings[4];

	virtual void LinkSibling(WindowData* t,int pos)=0;
	virtual void UnlinkSibling(WindowData* t=0)=0;
	virtual WindowData* FindSiblingOfSameSize()=0;
	virtual int FindSiblingPosition(WindowData* t)=0;
	virtual bool FindAndGrowSibling()=0;

	virtual vec2 Size()=0;
};

struct DLLBUILD GuiImage
{
	int width,height;
	int bpp;

	GuiImage();
	virtual ~GuiImage();

	virtual void Release()=0;
	virtual bool Fill(Renderer2D*,unsigned char* iData,float iWidth,float iHeight)=0;
};

struct DLLBUILD DrawInstance
{
	int		 code;
	bool	 frame;
	GuiRect* rect;
	String	name;

	bool remove;

	DrawInstance(int iNoneAllRect,bool iFrame,GuiRect* iRect,String iname,bool iRemove=true);
};

struct DLLBUILD Tab : TPoolVector<Tab>
{
	WindowData* windowData;

	Container* container;

	static const int COLOR_BACK=0x707070;
	static const int COLOR_LABEL=GuiRect::COLOR_BACK;

	static const int BAR_HEIGHT=30;
	static const int LABEL_WIDTH=80;
	static const int LABEL_HEIGHT=25;

	static const int ICON_WH=20;
	static const int ICON_STRIDE=ICON_WH*4;

	static unsigned char rawUpArrow[];
	static unsigned char rawRightArrow[];
	static unsigned char rawLeftArrow[];
	static unsigned char rawDownArrow[];
	static unsigned char rawFolder[];
	static unsigned char rawFile[];

	GuiRootRect	rects;
	GuiRootRect	rectsLayered;

	GuiRect* focused;
	GuiRect* hovered;
	GuiRect* pressed;

	Splitter* splitterContainer;

	Renderer2D *renderer2D;
	Renderer3D *renderer3D;

	GuiImage* iconUp;
	GuiImage* iconRight;
	GuiImage* iconLeft;
	GuiImage* iconDown;
	GuiImage* iconFolder;
	GuiImage* iconFile;

	unsigned int selected;
	bool mouseDown;
	bool isRender;
	bool recreateTarget;
	bool resizeTarget;
	bool resizing;

	bool destroy;

	std::list<DrawInstance*> drawInstances;

	Task*	 drawTask;

	vec2 mouse;

	unsigned int lastFrameTime;

	Thread* thread;

	//std::list< std::function<void()> > evtQueue;

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

	virtual vec2 Size();

	virtual void PaintBackground();
	virtual void DrawFrame();
	virtual void Destroy()=0;

	virtual void OnGuiRecreateTarget(void* data=0);

	GuiRect* GetSelected();

	virtual void BroadcastToSelected(void (GuiRect::*func)(const GuiEvent&),void* iData=0);
	virtual void BroadcastToAll(void (GuiRect::*func)(const GuiEvent&),void* iData=0);

	template<class C> void BroadcastToSelected(void (GuiRect::*func)(const GuiEvent&),void* iData=0);
	template<class C> void BroadcastToAll(void (GuiRect::*func)(const GuiEvent&),void* iData=0);

	static void BroadcastToPoolSelecteds(void (GuiRect::*func)(const GuiEvent&),void* iData=0)
	{
		for(std::vector<Tab*>::iterator tabContainer=TPoolVector<Tab>::GetPool().begin();tabContainer!=TPoolVector<Tab>::GetPool().end();tabContainer++)
			(*tabContainer)->BroadcastToSelected(func,iData);
	}

	void Draw();

	virtual bool BeginDraw()=0;
	virtual void EndDraw()=0;

	virtual void Create3DRenderer()=0;
	virtual void Destroy3DRenderer()=0;

	DrawInstance* SetDraw(int iNoneAllRect=1,bool iFrame=true,GuiRect* iRect=0,String iName=L"",bool iRemove=true);

	virtual int TrackGuiSceneViewerPopup(bool iSelected)=0;
	virtual int TrackTabMenuPopup()=0;
	virtual int TrackProjectFileViewerPopup(ResourceNode*)=0;

	void SetSelection(GuiRect* iRect);

	virtual void SetCursor(int)=0;

	void SetFocus(GuiRect*);
	void SetHover(GuiRect*);
	void SetPressed(GuiRect*);
	GuiRect* GetFocus();
	GuiRect* GetHover();
	GuiRect* GetPressed();
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
	wchar_t*		splitterCursor;

	Splitter();
	~Splitter();

	virtual void CreateFloatingTab(Tab*)=0;
	virtual void DestroyFloatingTab()=0;
};


struct DLLBUILD Container
{
	std::vector<Tab*> tabs;

	WindowData* window;
	Splitter* splitter;

	int resizeDiffHeight;
	int resizeDiffWidth;
	int resizeEnumType;
	int resizeCheckWidth;
	int resizeCheckHeight;

	Container();
	virtual ~Container();

	virtual void Enable(bool)=0;

	virtual void OnSizing()=0;
	virtual void OnSize()=0;

	virtual Tab* CreateTab(float x,float y,float w,float h)=0;
	virtual Tab* CreateModalTab(float w,float h)=0;
	virtual void DestroyTab(Tab*)=0;

	template<class GuiViewerDerived> GuiViewerDerived* SpawnViewer(Tab* iTabContainer=0,bool skipExist=true)
	{
		Tab* tTabContainer=iTabContainer ? iTabContainer : tabs[0];

		if(GuiViewerDerived::GetPool().empty())
			return tabs[0]->rects.Create<GuiViewerDerived>();
		else if(skipExist)
			return GuiViewerDerived::GetPool().front();
		else
			return tabs[0]->rects.Create<GuiViewerDerived>();
	}
};

struct DLLBUILD MenuInterface
{
	virtual void OnMenuPressed(int iIdx)=0;
	int Menu(String iName,bool tPopup);

	static int GetMenuId();
	static int IncrementMenuId();
};

struct DLLBUILD MainContainer : MenuInterface
{
	static const int COLOR_BACK=0x202020;

	int MenuBuild;
	int MenuPlugins;
	int MenuFile;
	int MenuInfo;
	int MenuActionBuildPC;
	int MenuActionExit;
	int MenuActionConfigurePlugin;
	int MenuActionProgramInfo;

	MainContainer();
	virtual ~MainContainer();

	Container* mainContainer;

	std::vector<Container*> containers;

	virtual Container* CreateContainer()=0;

	void OnMenuPressed(int iIdx);
	

};

struct DLLBUILD Subsystem
{
	virtual bool Execute(String iPath,String iCmdLine,String iOutputFile=L"",bool iInput=false,bool iError=false,bool iOutput=false,bool iNewConsole=false)=0;
	virtual unsigned int FindProcessId(String iProcessName)=0;
	virtual unsigned int FindThreadId(unsigned int iProcessId,String iThreadName)=0;
};

struct DLLBUILD Compiler
{
	String ideSrcPath;
	String ideLibPath;

	struct COMPILER
	{
		String name;
		String version;
		String compilerFile;
		String linkerFile;
		String compilerFlags;
		String linkerFlags;
		String outputFlag;
		String engineLibraryName;
		String engineLibraryExtension;
		String includeHeaders;
		String includeLib;
	};

	std::vector<COMPILER> compilers;

	enum
	{
		COMPILER_MS=0,
		COMPILER_MINGW,
		COMPILER_LLVM
	};

	Compiler();
	~Compiler();

	String outputDirectory;

	virtual bool Compile(Script*)=0;
	virtual String Compose(unsigned int iCompiler,Script*)=0;
	virtual bool LoadScript(Script*)=0;
	virtual bool UnloadScript(Script*)=0;
	//virtual bool CreateAndroidTarget()=0;
};

struct DLLBUILD PluginSystem
{
	struct DLLBUILD Plugin : MenuInterface
	{
		String name;
		bool loaded;

		Plugin();

		virtual void Load()=0;
		virtual void Unload()=0;
		virtual void OnMenuPressed(int iIdx)=0;
	};

	std::vector<Plugin*> plugins;

	void ShowConfigurationPanel();
	virtual void ScanPluginsDirectory()=0;

	Tab* configurationPanel;

	GuiButtonFunc* exitButton;

	PluginSystem();
	~PluginSystem();
};


struct DLLBUILD IdeViewerProperties
{
	GuiContainer* container;

	IdeViewerProperties();

	virtual void OnPropertiesCreate(){};
	virtual void OnResourcesCreate(){};
	virtual void OnPropertiesUpdate(Tab*){};
};

template<class T> struct DLLBUILD EditorObject : T , IdeViewerProperties{};

struct DLLBUILD EditorEntity : EditorObject<Entity>
{
	bool					selected;
	bool					expanded;
	int						level;

	GuiContainer					entityViewerProperties;
	GuiContainerRow<EditorEntity*>	sceneViewerLabel;

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
		this->runtime ? Ide::GetInstance()->debugger->RunDebuggeeFunction(this,1),true : false;
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


struct DLLBUILD GuiFont
{
	String	name;
	float	widths[255];
	float	height;

	float GetHeight();
	vec2 MeasureText(const wchar_t*);
	float GetCharWidth(wchar_t);
};

struct DLLBUILD Renderer2D
{
	static std::vector<GuiFont*> fonts;

	unsigned int	colorBackgroud;
	unsigned int	colorText;
	unsigned int	tabSpaces;

	Tab*			tab;

	Renderer2D(Tab*);
	~Renderer2D();

	virtual void DrawText(const String& iText,float left,float top, float right,float bottom,unsigned int iColor=GuiString::COLOR_TEXT)=0;
	virtual void DrawText(const String& iText,float left,float top, float right,float bottom,vec2 iSpot,vec2 iAlign,unsigned int iColor=GuiString::COLOR_TEXT)=0;
	virtual void DrawRectangle(float iX,float iY, float iWw,float iH,unsigned int iColor,bool iFill=true,float op=1.0f)=0;
	virtual void DrawRectangle(vec4& iXYWH,unsigned int iColor,bool iFill=true)=0;
	virtual void DrawBitmap(GuiImage* bitmap,float x,float y, float w,float h)=0;

	virtual void PushScissor(float x,float y,float w,float h)=0;
	virtual void PopScissor()=0;

	virtual void Translate(float,float)=0;
	virtual void Identity()=0;

	virtual vec2 MeasureText(const wchar_t*);

	virtual void DrawCaret()=0;
	virtual void SetCaretPos(float x,float y)=0;
};

struct DLLBUILD Renderer3D : Renderer3DBase
{
	Tab* tab;

	virtual Shader* CreateShaderProgram(const char* shader_name,const char* pixel_shader,const char* fragment_shader)=0;

	Shader* FindShader(const char* name,bool exact);
	void SetMatrices(const float* view,const float* mdl);

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

	bool LoadTexture(String iFilename,Texture* iTexture);

	Renderer3D(Tab*);
	virtual ~Renderer3D(){};
};


#endif //INTERFACES_H



#ifndef INTERFACES_H
#define INTERFACES_H

#include "entities.h"
#include <map>

struct DLLBUILD EditorEntity;

struct DLLBUILD Renderer2D;
struct DLLBUILD Renderer3D;

struct DLLBUILD GuiFont;
struct DLLBUILD Gui;
struct DLLBUILD GuiRoot;
struct DLLBUILD GuiTreeViewItem;
struct DLLBUILD GuiRootRect;
struct DLLBUILD GuiContainer;
template <typename T> struct DLLBUILD GuiProperty;
struct DLLBUILD GuiString;
struct DLLBUILD GuiButton;
struct DLLBUILD Picture;
struct DLLBUILD GuiButtonBool;
struct DLLBUILD GuiScrollBar;
struct DLLBUILD GuiScrollRect;
struct DLLBUILD GuiPropertyString;
struct DLLBUILD GuiPropertySlider;
struct DLLBUILD AnimationProperty;
struct DLLBUILD GuiLabel;
struct DLLBUILD GuiViewport;
struct DLLBUILD GuiPanel;
struct DLLBUILD GuiSceneViewer;
struct DLLBUILD GuiEntityViewer;
struct DLLBUILD GuiCompilerViewer;
struct DLLBUILD GuiConsoleViewer;
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
	FilePath				folderApplication;
	FilePath				folderPlugins;
	
	unsigned int            caretLastTime;
	unsigned int			processId;
	unsigned int			processThreadId;

	Thread*					timerThread;
	Thread*					projectDirChangedThread;

private:
	Tab*					popup;
public:

	static Ide* GetInstance();
	static bool IsInstanced();


protected:
	Ide();
	Ide(Ide const&){};
	void operator=(Ide const&){}
public:

	virtual ~Ide();

	virtual void Run()=0;
	virtual void ProjectDirHasChangedFunc()=0;

	virtual void ScanDir(String,ResourceNodeDir*)=0;

	String GetSceneExtension();
	String GetEntityExtension();

	virtual void Sleep(int iMilliseconds=1)=0;

	GuiProjectViewer*	GetProjectViewer();
	GuiSceneViewer*		GetSceneViewer();
	GuiCompilerViewer*	GetCompilerViewer();
	GuiConsoleViewer*	GetConsoleViewer();

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
		vec2			caret;

		Cursor();
	};

protected:

	GuiString*		string;
	Tab*			tab;

	unsigned int	blinkingRate;
	unsigned int	lastBlinkTime;
	bool			blinking;
	vec2			caret;
	vec2			caretPast;
	float			caretHeight;
	bool			enabled;

public:

	StringEditor();
	~StringEditor();

	bool EditText(unsigned int iCaretOp,void* iParam);

	void Bind(GuiString* iString);
	void Enable(bool);
	GuiString* Binded();
	bool Enabled();
	
	virtual void Draw(Tab*);
};


struct DLLBUILD Game
{

};

struct DLLBUILD Picture
{
	void* handle;

	float width;
	float height;
	float bpp;

	Picture();
	virtual ~Picture();

	virtual void Release();
};

struct DLLBUILD PictureRef : Picture
{
	unsigned char* refData;

	PictureRef(unsigned char* iRefData,float iWidth,float iHeight);
};

struct DLLBUILD PictureFile : Picture
{
	String fileName;

	PictureFile(String iFilename);
};

struct DLLBUILD Event{};
struct DLLBUILD EventMouse : Event{vec2 mouse;int  button;};
struct DLLBUILD EventKey : Event{char key;};


struct DLLBUILD Gui
{
	static const int COLOR_BACK=0x505050;

protected:
	String						name;
	Gui*						owner;
	Gui*						parent;
	std::list<Gui*>				childs;
	bool						pressing;
	bool						hovering;		
	bool						active;
	Gui*						clip;
public:
	virtual void						AddChild(Gui*);
	virtual void						RemoveChild(Gui*);
	virtual void						RemoveAllChilds();

	virtual vec4						GetEdges();
	virtual String						GetName();
	virtual float						GetWidth();
	virtual float						GetHeight();
	const Gui*							GetParent();
	const std::list<Gui*>&				GetChilds();
	Gui*								GetOwner();
	GuiRoot*							GetRoot();
	unsigned int						GetColor();

	virtual void						SetEdges(vec4){};
	virtual void						SetName(String);
	void								SetOwner(Gui*);
	void								SetActive(bool);

	bool								IsHovering();
	bool								IsPressing();
	bool								IsActive();
	template <typename T> bool			Is(){return dynamic_cast<T*>(this);}

	virtual bool						Contains(const vec2&);

	vec2								GetClippedMouse(const vec2& iMouse){return iMouse;}

	Gui();
	virtual ~Gui();

	virtual void FindHover(Tab*,vec2&,Gui*&){};

	//mouse
	virtual void OnMouseMove(Tab* iTab,EventMouse* iData=0){}
	virtual void OnDoubleClick(Tab* iTab,EventMouse* iData=0){}
	virtual void OnButtonDown(Tab* iTab,EventMouse* iData=0){if(iData->button==1)this->pressing=true;}
	virtual void OnButtonUp(Tab* iTab,EventMouse* iData=0){if(iData->button==1)this->pressing=false;}
	virtual void OnMouseWheel(Tab* iTab,EventMouse* iData=0){}
	virtual void OnMouseEnter(Tab* iTab,EventMouse* iData=0){this->hovering=true;}
	virtual void OnMouseExit(Tab* iTab,EventMouse* iData=0){this->hovering=false;}
	
	virtual void OnDestroy(Tab*,Event* iData=0){}
	virtual void OnBeginClip(Tab*,Event* iData=0){}
	virtual void OnEndClip(Tab*,Event* iData=0){}
	virtual void OnRecreateTarget(Tab*,Event* iData=0){}
	virtual void OnPaint(Tab*,Event* iData=0){}
	virtual void OnEntitiesChange(Tab*,Event* iData=0){}
	virtual void OnResize(Tab*,Event* iData=0){}
	
	
	virtual void OnUpdate(Tab*,Event* iData=0){}
	virtual void OnReparent(Tab*,Event* iData=0){}
	virtual void OnSelected(Tab*,Event* iData=0){}
	
	virtual void OnActivate(Tab* iTab,Event* iData=0){this->SetActive(false);}
	virtual void OnDeactivate(Tab*,Event* iData=0){this->SetActive(false);}
	virtual void OnEntitySelected(Tab*,Event* iData=0){}
	virtual void OnExpandos(Tab*,Event* iData=0){}
	virtual void OnKeyDown(Tab*,Event* iData=0){}
	virtual void OnKeyUp(Tab*,Event* iData=0){}
	
	virtual void OnEnterFocus(Tab*,Event* iData=0){}
	virtual void OnExitFocus(Tab*,Event* iData=0){}
};

struct DLLBUILD GuiRect : Gui
{
	vec4 edges;

	virtual void		SetEdges(vec4 iEdges){this->edges=iEdges;}
	virtual vec4		GetEdges(){return this->edges;}

	virtual void		FindHover(Tab* iTab,vec2& iMouse,Gui*& iGui){if(iMouse.x>this->edges.x && iMouse.x<this->edges.z && iMouse.y>this->edges.y && iMouse.y<this->edges.w)iGui=this;};
};

struct DLLBUILD GuiRoot : GuiRect
{
private:
	friend Tab;
	Tab* tab;
public:

	GuiRoot(Tab*);

	Tab*		 GetTab();
	void		 OnResize(Tab*,Event* iData=0);
	void		 FindHover(Tab*,vec2&,Gui*&);
};

struct DLLBUILD GuiContainer : GuiRect
{
	bool		expanded;
	String		description;

	GuiContainer();
	GuiContainer(String);

	void Insert(Gui* iGui);

	//void OnMouseMove(Tab*,Event* iData=0);
	void OnButtonDown(Tab* iTab,EventMouse* iData=0);
	void OnResize(Tab*,Event* iData=0);
	void OnPaint(Tab*,Event* iData=0);
};


struct DLLBUILD GuiString : GuiRect
{
	static const unsigned int COLOR_TEXT=0xFFFFFF;

	String			text;
	GuiFont*		font;
	unsigned int	color;
	vec2			spot;
	vec2			align;

	GuiString();

	void OnPaint(Tab*,Event* iData=0);
};

struct DLLBUILD GuiTextBox : GuiString
{
	StringEditor::Cursor* cursor;

	GuiTextBox();
	~GuiTextBox();

	bool ParseKeyInput(Tab*,Event* iData=0);

	virtual void OnPaint(Tab*,Event* iData=0);
	virtual void OnLMouseDown(Tab* iTab,EventMouse* iData=0);
	virtual void OnKeyDown(Tab*,Event* iData=0);
};

struct DLLBUILD GuiComboBox : Gui
{
	GuiString*          string;
	GuiButton*			button;
	GuiScrollRect*	    list;

	Tab*				popupPointer;

	int					selectedItem;

	std::vector<String> items;

	void RecreateList();

	GuiComboBox();
	~GuiComboBox();
};

struct GuiImage : Gui
{
	Picture* image;

	GuiImage();
	GuiImage(unsigned char* iRefData,float iWidth,float iHeight);
	GuiImage(String iFilename);
	~GuiImage();

	void OnPaint(Tab*,Event* iData=0);
	void OnActivate(Tab*,Event* iData=0);
};



struct DLLBUILD GuiButton : GuiString
{
	GuiButton();

	void (*func)(Event*);
	void* param;

	bool*	value;
	int		mode;

	
	virtual void OnPaint(Tab*,Event* iData=0);
	virtual void OnMouseEnter(Tab*,EventMouse* iData=0);
	virtual void OnMouseExit(Tab*,EventMouse* iData=0);
	virtual void OnButtonDown(Tab*,EventMouse* iData=0);
	virtual void OnButtonUp(Tab* iTab,EventMouse* iData=0);
};


struct DLLBUILD GuiScrollBar : GuiRect
{
	static const int COLOR_BACK=0x404040;
	static const int TICK=20;
	static const int SCROLL=10;

	enum ScrollBarType
	{
		SCROLLBAR_VERTICAL=0,
		SCROLLBAR_HORIZONTAL,
		SCROLLBAR_INVALID
	};

private:

	ScrollBarType scrollbarType;

	float position;
	float ratio;
	float scrollerPressed;
	float contentLength;
	float containerLength;
	
	float GetContainerLength();
	float GetContainerBegin();
	float GetContainerEnd();
	float GetScrollerBegin();
	float GetScrollerEnd();
	float GetScrollerLength();

	void Scroll(Tab*,float upOrDown);

public:

	GuiScrollBar(ScrollBarType iScrollbarType=SCROLLBAR_VERTICAL);
	~GuiScrollBar();

	void			SetType(ScrollBarType iScrollbarType);
	ScrollBarType	GetType(){return this->scrollbarType;}

	void SetRatio(float contentHeight,float containerHeight);
	void SetPosition(float contentHeight);
	
	bool IsVisible();
	float GetPosition();

	void OnButtonDown(Tab* iTab,EventMouse* iData=0);
	void OnButtonUp(Tab* iTab,EventMouse* iData=0);
	void OnMouseMove(Tab* iTab,EventMouse* iData=0);
	void OnPaint(Tab*,Event* iData=0);
	
};

struct DLLBUILD GuiTreeView : GuiRect
{
	struct DLLBUILD Item : Gui
	{
	private:
		friend GuiTreeView;

		GuiTreeView*	treeview;

		float						top;
		bool						expanded;
		int							level;
		bool						selected;
		String						name;
	public:
		Item();

		vec4 GetEdges();
		vec4 GetExpandosEdges();
		float GetLeadEdge();

		bool ExpandosContains(const vec2& iMouse);

		virtual void OnButtonUp(Tab* iTab,EventMouse* iData=0);
		virtual void OnButtonDown(Tab* iTab,EventMouse* iData=0);
		virtual void OnMouseEnter(Tab* iTab,EventMouse* iData=0);
		virtual void OnMouseExit(Tab* iTab,EventMouse* iData=0);
		virtual	void OnPaint(Tab*,Event* iData=0);
		virtual	void OnBeginClip(Tab*,Event* iData=0);
		virtual	void OnEndClip(Tab*,Event* iData=0);

		virtual void SetName(String);
		virtual void Insert(Item&);
		virtual GuiTreeView* GetTreeView();
	};
private:
	GuiScrollBar vscroll;
	GuiScrollBar hscroll;

	float contentWidth;
	float contentHeight;

	vec4 clippedEdges;

	float leftEdge;

	bool clipped;

	void BeginClip(Tab*);
	void EndClip(Tab*);

	void	RecalcScrollBars();
	void	RecalcItems();

	void		SetContentWidth(float);
	void		SetContentHeight(float);
	const float GetContentWidth();
	const float GetContentHeight();

	void SetContent(GuiTreeView::Item* iLabel,float& iTop,unsigned int iLevel,GuiRoot*);
	void FindItemHovered(Tab* iTab,vec2& iMouse,Item* iItem,Gui*& iGui);
	void DrawItems(Tab* iTab,Item*);

	Gui::AddChild;
public:

	GuiTreeView();

	void	Insert(Item* iWhere,Item* iElement);
	
	const vec4& GetClippedEdges();

	vec2 GetClippedMouse(const vec2& iMouse);

	virtual void OnPaint(Tab*,Event* iData=0);
	virtual void OnResize(Tab*,Event* iData=0);
	virtual void FindHover(Tab*,vec2&,Gui*&);
};

struct DLLBUILD GuiPropertyTree : GuiRect
{
	struct DLLBUILD Container : Gui
	{
		struct DLLBUILD Property;

		struct DLLBUILD PropertyContainer : Gui
		{
		private:
			Gui::AddChild;

			friend		GuiPropertyTree;
			friend		Container;

			Container*			container;
			String				description;
			float				top;
			Property*			property;
		public:
			PropertyContainer(Property*);
			PropertyContainer(String,Property*);

			vec4 GetEdges();
			float GetLeadEdge();

			const Container* GetContainer();

			virtual void OnBeginClip(Tab*,Event* iData=0);
			virtual void OnEndClip(Tab*,Event* iData=0);
			virtual void OnResize(Tab*,Event* iData=0);
			virtual void OnPaint(Tab*,Event* iData=0);
			virtual void OnMouseEnter(Tab* iTab,EventMouse* iData=0);
			virtual void OnMouseExit(Tab* iTab,EventMouse* iData=0);
			virtual void OnButtonDown(Tab* iTab,EventMouse* iData=0);
			virtual void OnButtonUp(Tab* iTab,EventMouse* iData=0);
			virtual void OnMouseMove(Tab* iTab,EventMouse* iData=0);

			virtual void FindHover(Tab* iTab,vec2& iMouse,Gui*& iGui);
		};

		struct DLLBUILD Property : Gui
		{
		private:
			friend GuiPropertyTree;

			PropertyContainer propertyContainer;
		public:
			Property();
			Property(String);

			vec4 GetEdges();

			virtual float GetHeight(){return 0;}
			virtual bool  HasDescription(){return true;}

			virtual void OnPaint(Tab*,Event* iData=0);
			virtual void OnMouseEnter(Tab* iTab,EventMouse* iData=0);
			virtual void OnMouseExit(Tab* iTab,EventMouse* iData=0);

			virtual void FindHover(Tab*,vec2&,Gui*&);
		};

	private:
		friend GuiPropertyTree;
		friend PropertyContainer;

		Gui::AddChild;

		GuiPropertyTree*	propertytree;
		bool				expanded;
		float				top;
		int					level;
		String				name;
		float				splitter;
		float				expandedHeight;
	public:
		Container();

		vec4 GetEdges();
		vec4 GetExpandosEdges();
		float GetLeadEdge()const;
		float GetSplitterEdge()const;

		bool ExpandosContains(const vec2& iMouse);

		virtual void FindHover(Tab* iTab,vec2& iMouse,Gui*& iGui);

		virtual void OnButtonUp(Tab* iTab,EventMouse* iData=0);
		virtual void OnMouseEnter(Tab* iTab,EventMouse* iData=0);
		virtual void OnMouseExit(Tab* iTab,EventMouse* iData=0);
		virtual	void OnBeginClip(Tab*,Event* iData=0);
		virtual	void OnEndClip(Tab*,Event* iData=0);
		virtual	void OnPaint(Tab*,Event* iData=0);

		virtual void SetName(String);

		virtual Container*	InsertContainer(Container*);
		virtual Property*	InsertProperty(Property*);

		virtual GuiPropertyTree* GetPropertyTree();

		bool IsExpanded();
	};
private:
	GuiScrollBar vscroll;
	GuiScrollBar hscroll;

	Container* splitterPressed;

	float contentWidth;
	float contentHeight;

	vec4 clippedEdges;

	float leftEdge;

	bool clipped;

	void BeginClip(Tab*);
	void EndClip(Tab*);

	void	RecalcScrollBars();

	void		SetContentWidth(float);
	void		SetContentHeight(float);
	const float GetContentWidth();
	const float GetContentHeight();

	void SetContent(Gui* iParent,Gui* iGui,float& iTop,unsigned int iLevel,GuiRoot*);
	void ResizeProperties(Tab*,Gui* iGui);
	void DrawRecursive(Tab*,Gui* iGui);
	virtual void FindHover(Tab* iTab,vec2& iMouse,Gui*& iGui);

	Gui::AddChild;
public:

	GuiPropertyTree();

	Container*	InsertContainer(Container* iWhere,Container* iElement);
	void		RecalcItems();

	const vec4& GetClippedEdges();
	vec2 GetClippedMouse(const vec2& iMouse);
	virtual void OnPaint(Tab*,Event* iData=0);
	virtual void OnResize(Tab*,Event* iData=0);
	
};


struct DLLBUILD StringProperty : GuiPropertyTree::Container::Property
{
	static const unsigned int scDefaultParameter1=2;
	static const unsigned int scDefaultParameter2=2;

	enum
	{
		BOOL=0,
		STRING,
		BOOLPTR,
		INT,
		FLOAT,
		VEC2,
		VEC3,
		VEC4,
		PTR,
		MAT4POS,
		ENTITYLISTSIZE,
		ANIMATIONVECSIZE,
		ISBONECOMPONENT,
		FLOAT2MINUSFLOAT1,
		VEC32MINUSVEC31,
		VEC3VECSIZE,
		VEC3LISTSIZE,
		MAXVALUE
	};

	void* valuePointer1;
	void* valuePointer2;
	unsigned int valueType;
	unsigned char valueParameter1;
	unsigned char valueParameter2;

	StringProperty(String iDescription,void* iValuePointer1,unsigned int iValueType,void* iValuePointer2=0,unsigned int iValueParameter1=scDefaultParameter1,unsigned int iValueParameter2=scDefaultParameter2);

	float	GetHeight();
	bool	HasDescription();

	void OnPaint(Tab*,Event* iData=0);
};


template <typename T> struct DLLBUILD GuiProperty : Gui
{
	String		description;
	T*			property;

	GuiProperty(String iDescription,T* iProperty)
	{
		this->description=iDescription;
		this->property=iProperty;
	}
};

struct DLLBUILD GuiSlider : GuiRect
{
	float* referenceValue;

	float* minimum;
	float* maximum;

	GuiSlider(float* iRef,float* iMin,float* iMax);

	virtual void OnPaint(Tab*,Event* iData=0);
	virtual void OnMouseMove(Tab* iTab,EventMouse* iData=0);
};

struct DLLBUILD SliderProperty : GuiPropertyTree::Container::Property
{
	GuiSlider slider;

	SliderProperty(String iDescription,float* iRef,float* iMin,float* iMax);

	float GetHeight();
	virtual void OnResize(Tab*,Event* iData=0);
};

struct DLLBUILD AnimationProperty : GuiPropertyTree::Container::Property
{
	AnimationController& animationController;

	AnimationProperty(String iDescription,AnimationController&);

	GuiButton      play;
	GuiButton      stop;

	GuiSlider  slider;

	float GetHeight();
	bool HasDescription();

	virtual void OnResize(Tab*,Event* iData=0);
};


struct DLLBUILD GuiListBox// : GuiScrollRect
{
	GuiButton* selection;

	void AddItem(String);
	void AddItem(void* iValuePointer1,unsigned int iValueType,void* iValuePointer2=0,unsigned int iValueParameter1=StringProperty::scDefaultParameter1,unsigned int iValueParameter2=StringProperty::scDefaultParameter2);
	
	void DestroyItems();

	GuiListBox();
	~GuiListBox();
};

struct DLLBUILD GuiPath : Gui
{
	GuiTextBox* path;
	GuiButton*  button;

	void (*func)(void*);
	void*  param;

	GuiPath();
	~GuiPath();
};




///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////GuiProperty///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////








///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////GuiViewers///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

struct DLLBUILD GuiViewport : GuiRect
{
	static std::list<GuiViewport*>& GetPool();

	EditorEntity* rootEntity;

	GuiButton* playStopButton;

	mat4 projection;
	mat4 view;
	mat4 model;

	void* renderBitmap;

	vec2 mouseold;

	unsigned char*	renderBuffer;

	unsigned int lastFrameTime;

	bool needsPicking;

	unsigned char pickedPixel[4];
	EditorEntity*		  pickedEntity;

	DrawInstance* renderDrawInstance;

	unsigned int renderFps;

	GuiViewport();
	GuiViewport(vec3 pos,vec3 target,vec3 up,bool perspective);
	virtual ~GuiViewport();

	virtual void OnPaint(Tab*,Event* iData=0);
	virtual void OnResize(Tab*,Event* iData=0);
	virtual void OnMouseWheel(Tab* iTab,EventMouse* iData=0);
	virtual void OnMouseMove(Tab* iTab,EventMouse* iData=0);
	virtual void OnActivate(Tab*,Event* iData=0);
	virtual void OnDeactivate(Tab*,Event* iData=0);
	virtual void OnReparent(Tab*,Event* iData=0);
	virtual void OnButtonUp(Tab* iTab,EventMouse* iData=0);

	virtual void Render(Tab*);
	virtual void DrawBuffer(Tab*,vec4&);
};

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////GuiViewers///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

struct DLLBUILD GuiScriptViewer : GuiRect//: GuiScrollRect
{
	std::list<EditorScript*> scripts;

	GuiTextBox* editor;
	GuiString*	lines;

	bool		lineNumbers;
	int			lineCount;

	GuiScriptViewer();
	~GuiScriptViewer();

	void Open(Script*);
	bool Save();
	bool Compile();

	void OnKeyDown(Tab*,Event* iData=0);
	void OnKeyUp(Tab*,Event* iData=0);
	void OnButtonDown(Tab*,Event* iData=0);
	void OnMouseMove(Tab*,Event* iData=0);
	void OnSize(Tab*,Event* iData=0);
	void OnDeactivate(Tab*,Event* iData=0);

	int CountScriptLines();

	void DrawLineNumbers(Tab*);
	void DrawBreakpoints(Tab*);
	
	static std::list<GuiScriptViewer*>& GetInstances();
};

struct DLLBUILD GuiCompilerViewer// : GuiScrollRect
{
	static GuiCompilerViewer* GetInstance();
	static bool IsInstanced();

private:
	GuiCompilerViewer();
	GuiCompilerViewer(GuiCompilerViewer const&){};
	void operator=(GuiCompilerViewer const&){}
public:

	~GuiCompilerViewer();

	bool ParseCompilerOutputFile(String);

	void OnSize(Tab*,Event* iData=0);
};

struct DLLBUILD GuiSceneViewer : GuiTreeView
{
	struct DLLBUILD SceneLabel : Item
	{
		EditorEntity* entity;

		void OnButtonUp(Tab* iTab,EventMouse* iData=0);
	};

	Scene						scene;
	
private:
	GuiSceneViewer();
	GuiSceneViewer(GuiSceneViewer const&);
	void operator=(GuiSceneViewer const&){}
public:

	static GuiSceneViewer* GetInstance();
	static bool IsInstanced();

	~GuiSceneViewer();

	void OnActivate(Tab* iTab,Event* iData=0);
	void OnDeactivate(Tab* iTab,Event* iData=0);
	void OnButtonDown(Tab* iTab,EventMouse* iData=0);
	void OnButtonUp(Tab* iTab,EventMouse* iData=0);
	void OnMouseMove(Tab* iTab,EventMouse* iData=0);

	void Save(String);
	void Load(String);
};

struct DLLBUILD GuiEntityViewer : GuiPropertyTree
{
	Tab* tab;

	GuiScrollBar	vscroll;
	EditorEntity*	entity;

	GuiEntityViewer();
	~GuiEntityViewer();

	static const std::list<GuiEntityViewer*>& GetInstances();

	virtual void OnEntitySelected(Tab*,Event* iData=0);
	virtual void OnExpandos(Tab*,Event* iData=0);
};

struct DLLBUILD GuiConsoleViewer// : GuiScrollRect
{
	static GuiConsoleViewer* GetInstance();
	static bool IsInstanced();
private:
	GuiConsoleViewer();
	GuiConsoleViewer(GuiConsoleViewer const&){};
	void operator=(GuiConsoleViewer const&){}
public:
	~GuiConsoleViewer();
};

struct DLLBUILD GuiProjectViewer : GuiRect
{
private:
	GuiProjectViewer();
	GuiProjectViewer(GuiProjectViewer const&);
	void operator=(GuiProjectViewer const&){}
public:
	struct DLLBUILD DirViewer : GuiTreeView
	{
		struct DLLBUILD DirLabel : Item 
		{
			ResourceNode* resource;
		};
	};

	struct DLLBUILD FileViewer : GuiTreeView
	{
		struct DLLBUILD FileLabel : Item 
		{
			ResourceNode* resource;

			void OnButtonUp(Tab*,EventMouse* iData=0);
		};
	};

	struct DLLBUILD DataViewer : GuiRect
	{

		void OnPaint(Tab*,Event* iData=0);
	};

	DirViewer dirViewer;
	FileViewer fileViewer;
	DataViewer resViewer;

	ResourceNodeDir*& projectDirectory;

	bool splitterLeftActive;
	bool splitterRightActive;

	float splitterLeft;
	float splitterRight;

	~GuiProjectViewer();

	void RefreshAll();

	static GuiProjectViewer* GetInstance();
	static bool IsInstanced();

	void Delete(Tab*,ResourceNode*);

	void FindHover(Tab* iTab,vec2&,Gui*&);

	void OnMouseMove(Tab* iTab,EventMouse* iData=0);
	void OnButtonDown(Tab* iTab,EventMouse* iData=0);
	void OnButtonUp(Tab* iTab,EventMouse* iData=0);
	void OnReparent(Tab*,Event* iData=0);
	void OnActivate(Tab*,Event* iData=0);
	void OnDeactivate(Tab*,Event* iData=0);
	void OnResize(Tab*,Event* iData=0);
	void OnPaint(Tab*,Event* iData=0);

	void SetEdges(vec4);

	void findResources(std::vector<ResourceNode*>& oResultArray,ResourceNode* iResourceNode,String iExtension);

	std::vector<ResourceNode*> findResources(String iExtension);
};

struct DLLBUILD ResourceNode
{
	ResourceNode* parent;

	FilePath fileName;
	bool isDir;

	GuiProjectViewer::FileViewer::FileLabel fileLabel;

	String BuildPath();

	ResourceNode();
	virtual ~ResourceNode();
};

struct DLLBUILD ResourceNodeDir : ResourceNode
{
	std::list<ResourceNodeDir*> dirs;
	std::list<ResourceNode*> files;

	GuiProjectViewer::DirViewer::DirLabel dirLabel;

	static ResourceNodeDir*	FindDirNode(String);
	static ResourceNode*	FindFileNode(String);
	static ResourceNodeDir* GetRootDirNode();

	ResourceNodeDir();
	virtual ~ResourceNodeDir();
};

struct DLLBUILD WindowData
{
	std::list<WindowData*> siblings[4];

	virtual void Enable(bool)=0;
	virtual bool IsEnabled()=0;

	virtual void LinkSibling(WindowData* t,int pos)=0;
	virtual void UnlinkSibling(WindowData* t=0)=0;
	virtual WindowData* FindSiblingOfSameSize()=0;
	virtual int FindSiblingPosition(WindowData* t)=0;
	virtual bool FindAndGrowSibling()=0;

	virtual vec2 Size()=0;
	virtual vec2 Pos()=0;
	virtual void Show(bool)=0;
	virtual bool IsVisible()=0;
	virtual void Resize(float,float)=0;
};





struct DLLBUILD DrawInstance
{
	Gui*		rect;
	bool		skip;
	bool		remove;

	DrawInstance(Gui* iRect,bool iRemove=true,bool iSkip=false);
};

struct DLLBUILD Tab
{
	WindowData* windowData;

	Container* container;

	static const int COLOR_BACK=0x707070;
	
	static const int BAR_HEIGHT=30;
	static const int LABEL_WIDTH=80;
	static const int LABEL_HEIGHT=25;

	static const int ICON_WH=20;
	static const int ICON_STRIDE=ICON_WH*4;

	static const int LABEL_LEFT_OFFSET=5;
	static const int LABEL_RIGHT_OFFSET=10;

	static unsigned char rawUpArrow[];
	static unsigned char rawRightArrow[];
	static unsigned char rawLeftArrow[];
	static unsigned char rawDownArrow[];
	static unsigned char rawFolder[];
	static unsigned char rawFile[];

	GuiRoot				 guiRoot;
	Gui*				 viewerSelected;

	Gui* focused;
	Gui* hovered;
	Gui* pressed;

	Splitter* splitterContainer;

	Renderer2D *renderer2D;
	Renderer3D *renderer3D;

	PictureRef* iconUp;
	PictureRef* iconRight;
	PictureRef* iconLeft;
	PictureRef* iconDown;
	PictureRef* iconFolder;
	PictureRef* iconFile;

	
	bool mouseDown;
	bool isRender;
	bool recreateTarget;
	bool resizeTarget;
	bool resizing;

	bool isModal;

	bool hasFrame;

	std::list<DrawInstance*>			drawInstances;
	std::list< std::function<void()> >  concurrentInstances;

	Task*	 drawTask;

	vec2 mouse;

	unsigned int lastFrameTime;

	Thread* thread;

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
	virtual void OnGuiMouseWheel(void* data=0);
	virtual void OnResizeContainer(void* data=0);
	virtual void OnEntitiesChange(void* data=0);
	virtual void OnGuiActivate(void* data=0);
	virtual void OnGuiDeactivate(void* data=0);
	virtual void OnGuiEntitySelected(void* data=0);
	virtual void OnGuiKeyDown(void* data=0);
	virtual void OnGuiKeyUp(void* data=0);

	virtual vec2 GetSize();
	virtual vec4 GetEdges();

	virtual void PaintBackground();
	virtual void DrawFrame();
	virtual void Destroy()=0;

	virtual void OnGuiRecreateTarget(void* data=0);

	Gui* GetSelected();

	virtual void BroadcastToSelected(void (Gui::*func)(Tab*,Event* iData),Event* iData);
	virtual void BroadcastToAll(void (Gui::*func)(Tab*,Event* iData),Event* iData);

	template<class C> void BroadcastToSelected(void (Gui::*iFunc)(Tab*,Event* iData),Event* iData)
	{
		Gui* selectedTab=this->GetSelected();

		if(selectedTab)
			selectedTab->BroadcastTo<C>(iFunc,iData);

		this->rectsLayered.BroadcastTo<C>(iFunc,iData);
	}

	template<class C> void BroadcastToAll(void (Gui::*iFunc)(Tab*,Event* iData),Event* iData)
	{
		this->guiRoot.BroadcastTo<C>(iFunc,iData);
		this->rectsLayered.BroadcastTo<C>(iFunc,iData);
	}


	void Draw();
	void DrawBlock(bool);
	

	virtual bool BeginDraw()=0;
	virtual void EndDraw()=0;

	virtual void Create3DRenderer()=0;
	virtual void Destroy3DRenderer()=0;

	DrawInstance* SetDraw(Gui* iRect=0,bool iRemove=true,bool iSkip=false);

	virtual int TrackGuiSceneViewerPopup(bool iSelected)=0;
	virtual int TrackTabMenuPopup()=0;
	virtual int TrackProjectFileViewerPopup(ResourceNode*)=0;

	void SetSelection(Gui* iRect);

	virtual void SetCursor(int)=0;

	void SetFocus(Gui*);
	void SetHover(Gui*);
	void SetPressed(Gui*);
	Gui* GetFocus();
	Gui* GetHover();
	Gui* GetPressed();
};

struct DLLBUILD Splitter
{
	Tab* currentTabContainer;

	Tab* floatingTabRef;
	Tab* floatingTab;
	Tab* floatingTabTarget;
	Gui*  floatingSelectedTebGuiRect;
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

	void BroadcastToTabs(void (Tab::*func)(void*),void* iData=0);
	void BroadcastToSelectedTabRects(void (Gui::*func)(Tab*,Event* iData),Event* iData=0);
	

	WindowData* windowData;
	Splitter* splitter;

	int resizeDiffHeight;
	int resizeDiffWidth;
	int resizeEnumType;
	int resizeCheckWidth;
	int resizeCheckHeight;

	Container();
	virtual ~Container();

	virtual int GetWindowHandle()=0;

	virtual void OnSizing()=0;
	virtual void OnSize()=0;

	virtual Tab* CreateTab(float x,float y,float w,float h)=0;
	virtual Tab* CreateModalTab(float x,float y,float w,float h)=0;
	virtual void DestroyTab(Tab*)=0;
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

	virtual void Initialize()=0;
	virtual void Deintialize()=0;

	virtual Container* CreateContainer()=0;

	void OnMenuPressed(int iIdx);
	
	void BroadcastToTabs(void (Tab::*func)(void*),void* data);
	//void BroadcastToSelectedTabRects(void (GuiInterface::*func)(Tab*,Event* iData=0),const GuiEvent*);

};

struct DLLBUILD Subsystem
{
	virtual bool Execute(String iPath,String iCmdLine,String iOutputFile=L"",bool iInput=false,bool iError=false,bool iOutput=false,bool iNewConsole=false)=0;
	virtual unsigned int FindProcessId(String iProcessName)=0;
	virtual unsigned int FindThreadId(unsigned int iProcessId,String iThreadName)=0;
	virtual String DirectoryChooser(String iDescription,String iExtension)=0;
	virtual String FileChooser(String iDescription,String iExtension)=0;
	virtual std::vector<String> ListDirectories(String iDir)=0;
	virtual bool CreateDirectory(String)=0;
	virtual bool DirectoryExist(String)=0;
	String RandomDir(String iWhere,int iSize,String iAlphabet=L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	virtual void* LoadLibrary(String)=0;
	virtual bool FreeLibrary(void*)=0;
	virtual void* GetProcAddress(void*,String)=0;
};

struct DLLBUILD Compiler
{
	String ideSrcPath;
	String ideLibPath;

	struct COMPILER
	{
		String name;
		String version;
		String compilerExecutable;
		String linkerExecutable;
		String compilerFlags;
		String linkerFlags;
		String outputCommand;
		String engineLibraryName;
		String engineLibraryExtension;
		String includeHeadersPrefix;
		String includeLibPrefix;
		String additionalLibs;
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

	bool Compile(EditorScript*);
	String Compose(unsigned int iCompiler,EditorScript*);
	bool LoadScript(EditorScript*);
	bool UnloadScript(EditorScript*);
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

	Tab* modalTab;

	GuiButton* exitButton;

	PluginSystem();
	~PluginSystem();
};



struct DLLBUILD EditorObjectBase
{
	virtual GuiPropertyTree::Container* GetContainer()=0;

	virtual void OnPropertiesCreate()=0;
	virtual void OnResourcesCreate()=0;
	virtual void OnPropertiesUpdate(Tab*)=0;
};

template<class T> struct DLLBUILD EditorObject : EditorObjectBase , T
{
	GuiPropertyTree::Container container;

	EditorObject(){}
	virtual ~EditorObject(){}

	GuiPropertyTree::Container* GetContainer(){return &this->container;}

	virtual void OnPropertiesCreate(){};
	virtual void OnResourcesCreate(){};
	virtual void OnPropertiesUpdate(Tab*){};
};


struct DLLBUILD EditorEntity : EditorObject<Entity>
{
	GuiSceneViewer::SceneLabel		sceneLabel;

private:
	Entity::childs;
	Entity::parent;
	Entity::components;
public:

	EditorEntity();
	~EditorEntity();

	EditorEntity*							GetParent();
	const std::list<EditorEntity*>&			Childs();
	const std::list<EntityComponent*>&		Components();

	EditorEntity*							AddEntity(EditorEntity*);
	void									RemoveEntity(EditorEntity*);

	EntityComponent*						AddComponent(EntityComponent*);
	void									RemoveComponent(EntityComponent*);
	

	void									DestroyChilds();
	void									OnPropertiesCreate();
	void									OnPropertiesUpdate(Tab*);

	template<class C> C* CreateComponent()
	{
		C* component=this->Entity::CreateComponent<C>();
		//component->OnResourcesCreate();
		component->OnPropertiesCreate();
		return component;
	}
};


struct DLLBUILD EditorAnimationController : EditorObject<AnimationController>
{
	float oldCursor;
	float minSpeed;
	float maxSpeed;

	AnimationProperty*  guiAnimationController;

	EditorAnimationController();
	~EditorAnimationController();

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
struct DLLBUILD EditorLine : EditorObject<Line>
{
private:
	using Line::points;
public:

	GuiListBox* pointListBox;;

	void OnPropertiesCreate();
	void OnPropertiesUpdate(Tab*);

	void DestroyPoints();
	void AddPoint(vec3);

};
struct DLLBUILD EditorLight : EditorObject<Light>
{
	void OnPropertiesCreate();
	void OnPropertiesUpdate(Tab*);
};
struct DLLBUILD EditorScript : EditorObject<Script>
{
	FilePath			module;

	GuiButton*			buttonLaunch;

	GuiScriptViewer*	scriptViewer;
	ResourceNode*		resourceNode;

	EditorScript();

	void OnPropertiesCreate();
	void OnPropertiesUpdate(Tab*);
	void OnResourcesCreate();

	//for the debugger
	void update()
	{
		this->runtime ? Ide::GetInstance()->debugger->RunDebuggeeFunction(this,1),true : false;
	}

	void	SaveScript(String&);
	String	LoadScript();
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
	float	tabSpaces;

	GuiFont();

	float GetHeight()const;
	vec2 MeasureText(const wchar_t*)const;
	float GetCharWidth(wchar_t)const;

	static const GuiFont* GetDefaultFont();
	static const std::vector<GuiFont*>& GetFontPool();
	static GuiFont* CreateFont(String iFontName,float iFontSize);
};


struct DLLBUILD Renderer2D
{
	unsigned int	colorBackgroud;
	unsigned int	colorText;
	unsigned int	tabSpaces;

	Tab*			tab;

	Renderer2D(Tab*);
	~Renderer2D();

	virtual void DrawText(const String& iText,float left,float top, float right,float bottom,vec2 iSpot,vec2 iAlign,unsigned int iColor=GuiString::COLOR_TEXT,const GuiFont* iFont=GuiFont::GetDefaultFont())=0;
	virtual void DrawLine(vec2 p1,vec2 p2,unsigned int iColor,float iWidth=0.5f,float iOpacity=1.0f)=0;
	virtual void DrawRectangle(float iX,float iY, float iWw,float iH,unsigned int iColor,bool iFill=true,float op=1.0f)=0;
	virtual void DrawRectangle(vec4& iXYWH,unsigned int iColor,bool iFill=true)=0;
	virtual void DrawBitmap(Picture* bitmap,float x,float y, float w,float h)=0;

	virtual bool LoadBitmap(Picture*)=0;	

	virtual void PushScissor(float x,float y,float w,float h)=0;
	virtual void PopScissor()=0;

	virtual void Translate(float,float)=0;
	virtual void Identity()=0;

	virtual vec2 MeasureText(const wchar_t*,const GuiFont* iFont=GuiFont::GetDefaultFont());

	static void EnableCaret();
	static void DrawCaret();
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



#ifndef INTERFACES_H
#define INTERFACES_H

#include "entities.h"
#include <map>

struct DLLBUILD EditorEntity;

struct DLLBUILD Renderer2D;
struct DLLBUILD Renderer3D;

struct DLLBUILD GuiRect;
struct DLLBUILD GuiFont;
struct DLLBUILD GuiRect;
struct DLLBUILD GuiRoot;
struct DLLBUILD GuiContainer;
template<typename T> struct DLLBUILD GuiContainerRow;
template<typename T> struct DLLBUILD GuiProperty;
struct DLLBUILD GuiString;
struct DLLBUILD GuiButton;
struct DLLBUILD Picture;
struct DLLBUILD GuiButtonBool;
struct DLLBUILD GuiScrollBar;
struct DLLBUILD GuiRect;
struct DLLBUILD GuiPropertyString;
struct DLLBUILD GuiPropertySlider;
struct DLLBUILD GuiAnimationController;
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

struct DLLBUILD Ide
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

	Tab* CreatePopup(Container*,float,float,float,float);
	void DestroyPopup();
	Tab* GetPopup();

	static Ide*				Instance();

protected:
	Ide();
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


struct DLLBUILD Msg
{
	void*				data;
	vec2				mouse;
	int					button;
	char				key;
	mutable GuiRect*	hit;
	mutable vec4		rect;

	Msg():data(0),button(0),key(0),hit(0){}
	Msg(void* iData):data(iData),button(0),key(0),hit(0){}
	Msg(const vec2& iMouse,int iButton=0):data(0),mouse(iMouse),button(iButton),key(0),hit(0){}
	Msg(char iKey):data(0),button(0),key(iKey),hit(0){}
	Msg(const vec4& iRect):data(0),button(0),key(0),hit(0),rect(iRect){}
};


#define GUIMSGDECL Tab* iTab,const Msg& iMsg=Msg()
#define GUIMSGDEF Tab* iTab,const Msg& iMsg
#define GUIMSGCALL iTab,iMsg

struct DLLBUILD GuiRect
{
	static const int ROW_HEIGHT=20;
	static const int ROW_ADVANCE=GuiRect::ROW_HEIGHT;

	static const int COLOR_BACK=0x505050;
	static const int COLOR_HOVERED=0x101010;
	static const int COLOR_PRESSED=0x151515;
	static const int COLOR_CHECKED=0x202020;

	enum Switch
	{
		FLAGS=0,
		FUNIN,
		FUNOUT,
		MAXSWITCH
	};

	enum Flags
	{
		ACTIVE=0,
		CLIP,
		PRESS,
		HOVER,
		CHECK,
		ISPRESSING,
		ISHOVERING,
		ISCHECKED,
		HSCROLL,
		VSCROLL,
		PARCOLOR,
		PARFLAGS,
		PARCALLS,
		PARPROP,
		HASBORDER,
		HASBACKGROUND,
		MAXFLAGS
	};

	enum Funcs
	{
		ONBEGINCLIP,
		ONENDCLIP,
		ONRECREATETARGET,
		ONPAINT,
		ONENTITIESCHANGE,
		ONSIZE,
		ONBUTTONDOWN,
		ONDOUBLECLICK,
		ONBUTTONUP,
		ONMOUSEMOVE,
		ONUPDATE,
		ONREPARENT,
		ONSELECTED,
		ONRENDER,
		ONMOUSEWHEEL,
		ONACTIVATE,
		ONDEACTIVATE,
		ONENTITYSELECTED,
		ONEXPANDOS,
		ONKEYDOWN,
		ONKEYUP,
		ONMOUSEENTER,
		ONMOUSEEXIT,
		ONENTERFOCUS,
		ONEXITFOCUS,
		MAXFUNCS
	};

private:

	friend Tab;
	friend GuiRoot;
	friend GuiScrollBar;

	GuiRect* parent;
	std::list<GuiRect*> childs;

	GuiRect* next;
	GuiRect* previous;

	unsigned int	flags[MAXSWITCH];

	unsigned int	color;

	String name;

	vec2 content;
	vec4 edges;

	GuiScrollBar* vscroll;
	GuiScrollBar* hscroll;

	void* userData;

	GuiRect* clip;

	void RecalculateScrollbars();
	void CallScrollbarsFunc(void (GuiScrollBar::*iFunc)(GUIMSGDEF),GUIMSGDECL);

public:

	GuiRect();
	GuiRect(unsigned int iFlags,unsigned int iCalls);
	virtual ~GuiRect();

	void						SetFlag(Switch iType,unsigned int,bool);
	void						SetFlags(Switch iType,unsigned int);

	bool						GetFlag(Switch iType,unsigned int);
	unsigned int				GetFlags(Switch iType);

	const std::list<GuiRect*>&  Childs();
	GuiRect*					Next();
	GuiRect*					Previous();

	virtual GuiRect*			Prepend(GuiRect*);
	virtual GuiRect*			Append(GuiRect*);
	virtual GuiRect*			RemoveChild(GuiRect*);
	virtual void				RemoveAllChilds();
	
	GuiRect*					GetParent();

	GuiRoot*					GetRoot();

	void						Edges(const vec4&);
	vec4						Edges();
	vec4						EdgeClip();
	vec2						MouseClip(const vec2&);



	virtual float				left();
	virtual float				top();
	virtual float				right();
	virtual float				bottom();

	void						left(float);
	void						top(float);
	void						right(float);
	void						bottom(float);

	float						Width();
	float						Height();

	unsigned int				Color();
	void						Color(unsigned int);
	unsigned int				BlendColor(unsigned int iBase,unsigned int iHoverColor=0x101010,unsigned int iPressColor=0x202020,unsigned int iCheckColor=0x303030);

	void						SetContent(float,float);
	const vec2&					GetContent();

	void						SetUserData(void* iData){userData=iData;}
	void*						GetUserData(){return userData;}

	void						SetName(String iName);
	String						GetName();

	bool						IsPressing();
	bool						IsHovering();
	bool						IsChecked();
	bool						IsActive();
	bool						IsClipped();
	void						Activate(bool);

	GuiRect*					GetClip(){return clip;}

	virtual void OnBeginClip(GUIMSGDECL){};
	virtual void OnEndClip(GUIMSGDECL){};
	virtual void OnRecreateTarget(GUIMSGDECL){}
	virtual void OnPaint(GUIMSGDECL);
	virtual void OnEntitiesChange(GUIMSGDECL){}
	virtual void OnSize(GUIMSGDECL);
	virtual void OnButtonDown(GUIMSGDECL);
	virtual void OnDoubleClick(GUIMSGDECL);
	virtual void OnButtonUp(GUIMSGDECL);
	virtual void OnMouseMove(GUIMSGDECL);
	virtual void OnUpdate(GUIMSGDECL);
	virtual void OnReparent(GUIMSGDECL);
	virtual void OnSelected(GUIMSGDECL);
	virtual void OnRender(GUIMSGDECL);
	virtual void OnMouseWheel(GUIMSGDECL);
	virtual void OnActivate(GUIMSGDECL);
	virtual void OnDeactivate(GUIMSGDECL);
	virtual void OnEntitySelected(GUIMSGDECL);
	virtual void OnExpandos(GUIMSGDECL);
	virtual void OnKeyDown(GUIMSGDECL);
	virtual void OnKeyUp(GUIMSGDECL);
	virtual void OnMouseEnter(GUIMSGDECL);
	virtual void OnMouseExit(GUIMSGDECL);
	virtual void OnEnterFocus(GUIMSGDECL);
	virtual void OnExitFocus(GUIMSGDECL);


	bool Contains(const vec2&);

	GuiContainer* Container(String iText);
	template<typename T> GuiContainerRow<T>* ContainerRow(T);

	GuiString* Text(String str);

	template<typename T> GuiProperty<T>* Property(String iDescription,T* iProperty=0)
	{
		GuiProperty<T>* tProperty=new GuiProperty<T>(iDescription,iProperty);
		this->Append(tProperty);
		return tProperty;
	}

	template<class C> void Get(std::vector<C*>& iRects)
	{
		C* isaC=dynamic_cast<C*>(this);

		if(isaC)
			iRects.push_back(isaC);

		for(std::list<GuiRect*>::const_iterator tRect=this->Childs().begin();tRect!=this->Childs().end();tRect++)
			(*tRect)->Get(iRects);
	}

	void DestroyChilds();
};


struct DLLBUILD GuiScrollBar
{
	static const int COLOR_BACK=0x404040;
	static const int TICK=20;
	static const int SCROLL=10;

	enum Type
	{
		HORIZONTAL=0,
		VERTICAL,
		MAXTYPE
	};

private:

	GuiRect* rect;

	Type type;

	float position;
	float scrollerPressed;


	float GetContainerLength(const vec4& iEdges);
	float GetContainerBegin(const vec4& iEdges);
	float GetContainerEnd(const vec4& iEdges);
	float GetScrollerBegin(const vec4& iEdges);
	float GetScrollerEnd(const vec4& iEdges);
	float GetScrollerLength(const vec4& iEdges);

	void Scroll(Tab*,float upOrDown);

public:

	GuiScrollBar(GuiRect*,Type iType);
	~GuiScrollBar();

	Type	GetType();

	float Ratio();
	void SetPosition(float contentHeight);

	bool IsVisible();
	float GetPosition();

	bool Contains(const vec2& iPoint);
	vec4  Edges();

	void OnButtonDown(GUIMSGDECL);
	void OnButtonUp(GUIMSGDECL);
	void OnMouseMove(GUIMSGDECL);
	void OnPaint(GUIMSGDECL);
};

struct DLLBUILD GuiRoot : GuiRect
{
private:
	Tab* tab;
public:
	GuiRoot(Tab* t);
	~GuiRoot();

	Tab* GetTab()const{return tab;}

	void OnSize(GUIMSGDECL);
	GuiRect* Append(GuiRect*);
};

struct DLLBUILD GuiTreeView : GuiRect
{
	struct DLLBUILD Item : GuiRect
	{
	private:
		friend GuiTreeView;

		GuiTreeView*	treeview;

		bool						expanded;
		int							level;
		bool						selected;
	public:
		Item();

		virtual float left(){return this->TreeView()->left();}
		virtual float right(){return this->TreeView()->EdgeClip().z;}

		virtual GuiTreeView* TreeView();

		vec4 EdgeExpandos();
		float EdgeLevel();

		bool ExpandosContains(const vec2& iMouse);

		void Expand(bool);
		bool Expanded();

		void Select(bool);
		bool Selected();

		virtual void OnButtonUp(GUIMSGDECL);
		virtual	void OnPaint(GUIMSGDECL);

		virtual void Insert(Item&);
	};
private:
	Item*	items;

	float rightEdge;
	
	void	RecalcItems();

	void ConfigureContent(GuiTreeView::Item* iLabel,float& iTop,unsigned int iLevel,GuiRoot*);

	GuiRect::Append;
public:

	GuiTreeView();

	void	Insert(Item* iWhere,Item* iElement);
};

struct DLLBUILD GuiPropertyTree : GuiRect
{
	struct DLLBUILD Container : GuiRect
	{
		struct DLLBUILD Property : GuiRect
		{
		private:
			GuiRect::Append;

			friend		GuiPropertyTree;
			friend		GuiPropertyTree::Container;

			GuiPropertyTree::Container*						container;

			GuiRect*										property;

			float	height;
		public:
			Property(String iDescription,GuiRect* iProperty,float iHeight=20);

			virtual float left(){return this->Container()->PropertyTree()->left();}
			virtual float right(){return this->Container()->PropertyTree()->EdgeClip().z;}
			virtual float bottom(){return this->top()+this->height;}

			float EdgeLevel();
			void  Height(float iHeight){this->height=iHeight;}

			GuiPropertyTree::Container* Container();

			virtual void OnPaint(GUIMSGDECL);
			virtual void OnSize(GUIMSGDECL);
			virtual void OnButtonDown(GUIMSGDECL);
			virtual void OnButtonUp(GUIMSGDECL);
			virtual void OnMouseMove(GUIMSGDECL);
		};

	private:
		friend GuiPropertyTree;
		friend Property;

		GuiRect::Append;

		GuiPropertyTree*	propertytree;

		bool				expanded;
		int					level;
		float				splitter;
		float				expandedHeight;
	public:
		Container();

		virtual float left(){return this->PropertyTree()->left();}
		virtual float right(){return this->PropertyTree()->EdgeClip().z;}

		GuiPropertyTree* PropertyTree();

		vec4 EdgeExpandos();
		float EdgeLevel();
		float EdgeSplitter();

		void Expand(bool);
		bool Expanded();

		void Splitter(float);
		float Splitter();

		bool ExpandosContains(const vec2& iMouse);

		virtual void OnButtonUp(GUIMSGDECL);
		virtual	void OnPaint(GUIMSGDECL);

		GuiRect* Append(GuiRect*);

		virtual Container*	InsertContainer(String iDescription);
		virtual Property*	InsertProperty(String iDescription,GuiRect* iRect,float iHeight=20);

		
	};
private:
	Container* splitterPressed;

	float leftEdge;

	void ConfigureContent(GuiRect* iParent,GuiRect* iGui,float& iTop,unsigned int iLevel,GuiRoot*);

	GuiRect::Append;
public:

	GuiPropertyTree();

	void		 SplitterPressed(Container*);
	Container*	 SplitterPressed();

	Container*	InsertContainer(Container* iWhere,Container* iElement);
	void		RecalcItems();
};


struct DLLBUILD GuiStringProperty : GuiRect
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

	GuiStringProperty(void* iValuePointer1,unsigned int iValueType,void* iValuePointer2=0,unsigned int iValueParameter1=scDefaultParameter1,unsigned int iValueParameter2=scDefaultParameter2);

	void OnPaint(GUIMSGDECL);
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

	void OnPaint(GUIMSGDECL);
};

struct DLLBUILD GuiComboBox : GuiRect
{
	GuiString*          string;
	GuiButton*			button;
	GuiRect*	    list;

	Tab*				popupPointer;

	int					selectedItem;

	std::vector<String> items;

	void RecreateList();

	GuiComboBox();
	~GuiComboBox();
};

struct GuiImage : GuiRect
{
	Picture* image;

	GuiImage();
	GuiImage(unsigned char* iRefData,float iWidth,float iHeight);
	GuiImage(String iFilename);
	~GuiImage();

	void OnPaint(GUIMSGDECL);
	void OnActivate(GUIMSGDECL);
};

struct DLLBUILD GuiTextBox : GuiString
{
	StringEditor::Cursor* cursor;

	GuiTextBox();
	~GuiTextBox();

	bool ParseKeyInput(GUIMSGDECL);

	virtual void OnPaint(GUIMSGDECL);
	virtual void OnButtonDown(GUIMSGDECL);
	virtual void OnKeyDown(GUIMSGDECL);
};


struct DLLBUILD GuiButton : GuiString
{
	GuiButton();

	void (*func)(void*);
	void* param;

	bool*	value;
	int		mode;

	virtual void OnButtonUp(GUIMSGDECL);
};




struct DLLBUILD GuiSlider : GuiRect
{
	float* referenceValue;

	float* minimum;
	float* maximum;

	GuiSlider(float* iRef,float* iMin,float* iMax);

	virtual void OnPaint(GUIMSGDECL);
	virtual void OnMouseMove(GUIMSGDECL);
};

struct DLLBUILD GuiListBox : GuiRect
{
	GuiButton* selection;

	void AddItem(String);
	void AddItem(void* iValuePointer1,unsigned int iValueType,void* iValuePointer2=0,unsigned int iValueParameter1=GuiStringProperty::scDefaultParameter1,unsigned int iValueParameter2=GuiStringProperty::scDefaultParameter2);
	
	void DestroyItems();

	GuiListBox();
	~GuiListBox();
};

struct DLLBUILD GuiPath : GuiRect
{
	GuiTextBox* path;
	GuiButton*  button;

	void (*func)(void*);
	void*  param;

	GuiPath();
	~GuiPath();
};

struct DLLBUILD GuiAnimationController : GuiRect
{
	AnimationController& animationController;

	GuiAnimationController(AnimationController&);

	GuiButton play;
	GuiButton stop;

	GuiSlider slider;

	virtual void OnMouseMove(GUIMSGDECL);
	virtual void OnSize(GUIMSGDECL);
};



///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////GuiProperty///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

template<typename T> struct DLLBUILD GuiProperty : GuiRect
{
	GuiString*	description;
	T*			property;

	GuiProperty(String iDescription,T* iProperty=0)
	{
		this->description=new GuiString;
//		this->description->GetAutoEdges()->scl.make(1,1,0.5f,1);
		this->description->text=iDescription;
		this->Append(this->description);

		this->property=iProperty ? iProperty : new T;
//		this->property->GetAutoEdges()->ref.x=&this->description->GetEdges().z;
		this->Append(this->property);

//		this->GetAutoEdges()->fix.w=this->property->GetAutoEdges()->fix.w;
	}
};

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////GuiViewers///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

struct DLLBUILD GuiViewport : GuiRect
{
	static std::list<GuiViewport*>& Pool();

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

	virtual void OnPaint(GUIMSGDECL);
	virtual void OnMouseWheel(GUIMSGDECL);
	virtual void OnMouseMove(GUIMSGDECL);
	virtual void OnActivate(GUIMSGDECL);
	virtual void OnDeactivate(GUIMSGDECL);
	virtual void OnButtonUp(GUIMSGDECL);

	virtual void Render(Tab*);
	virtual void DrawBuffer(Tab*,vec4);
};

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////GuiViewers///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

struct DLLBUILD GuiScriptViewer : GuiRect
{
	EditorScript* script;

	GuiTextBox* editor;
	GuiString* lines;

	bool		lineNumbers;
	int			lineCount;

	GuiScriptViewer();
	~GuiScriptViewer();

	void Open(Script*);
	bool Save();
	bool Compile();

	void OnKeyDown(GUIMSGDECL);
	void OnKeyUp(GUIMSGDECL);
	void OnLMouseDown(GUIMSGDECL);
	void OnMouseMove(GUIMSGDECL);
	void OnSize(GUIMSGDECL);
	void OnDeactivate(GUIMSGDECL);

	int CountScriptLines();

	void DrawLineNumbers(Tab*);
	void DrawBreakpoints(Tab*);
	
	static std::list<GuiScriptViewer*>& GetInstances();
};

struct DLLBUILD GuiCompilerViewer : GuiRect
{
private:
	GuiCompilerViewer();
public:

	static GuiCompilerViewer* Instance();

	bool ParseCompilerOutputFile(String);

	void OnSize(GUIMSGDECL);
};



struct DLLBUILD GuiPanel : GuiRect
{
	GuiPanel();
};



struct DLLBUILD GuiSceneViewer : GuiTreeView
{
	Scene						scene;
	
	struct DLLBUILD SceneLabel : GuiTreeView::Item
	{
	private:
		EditorEntity* entity;
	public:

		void			Entity(EditorEntity*);
		EditorEntity*	Entity();


		void OnButtonUp(GUIMSGDECL);
	};
private:
	GuiSceneViewer();
public:

	static GuiSceneViewer* Instance();

	void OnEntitiesChange(GUIMSGDECL);

	void Save(String);
	void Load(String);
};

struct DLLBUILD GuiEntityViewer : GuiPropertyTree
{
	GuiEntityViewer();
	~GuiEntityViewer();

	EditorEntity* entity;

	static std::list<GuiEntityViewer*>& Pool();

	//void OnActivate(GUIMSGDECL);

	virtual void OnEntitySelected(GUIMSGDECL);
	virtual void OnExpandos(GUIMSGDECL);
};

struct DLLBUILD GuiConsoleViewer : GuiRect
{
private:
	GuiConsoleViewer();
public:
	~GuiConsoleViewer();
};

struct DLLBUILD GuiProjectViewer : GuiRect
{
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

			void OnButtonUp(GUIMSGDECL);
		};
	};

	struct DLLBUILD DataViewer : GuiRect
	{
		DataViewer();
	};

	DirViewer dirViewer;
	FileViewer fileViewer;
	DataViewer resViewer;

	ResourceNodeDir*& projectDirectory;

	bool splitterLeftActive;
	bool splitterRightActive;

	float splitterLeft;
	float splitterRight;

private:
	GuiProjectViewer();
public:
	~GuiProjectViewer();

	static GuiProjectViewer* Instance();

	void RefreshAll();

	void Delete(Tab*,ResourceNode*);

	void OnMouseMove(GUIMSGDECL);
	void OnButtonDown(GUIMSGDECL);
	void OnButtonUp(GUIMSGDECL);
	void OnReparent(GUIMSGDECL);
	void OnActivate(GUIMSGDECL);
	void OnDeactivate(GUIMSGDECL);

	void OnSize(GUIMSGDECL);

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
	GuiRect* rect;
	bool	 skip;
	bool	 remove;

	DrawInstance(GuiRect* iRect,bool iRemove=true,bool iSkip=false);
};

struct DLLBUILD Tab
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

	static const int LABEL_LEFT_OFFSET=5;
	static const int LABEL_RIGHT_OFFSET=10;

	static unsigned char rawUpArrow[];
	static unsigned char rawRightArrow[];
	static unsigned char rawLeftArrow[];
	static unsigned char rawDownArrow[];
	static unsigned char rawFolder[];
	static unsigned char rawFile[];

	GuiRoot	rects;

	GuiRect* focused;
	GuiRect* hovered;
	GuiRect* pressed;

	Splitter* splitterContainer;

	Renderer2D *renderer2D;
	Renderer3D *renderer3D;

	PictureRef* iconUp;
	PictureRef* iconRight;
	PictureRef* iconLeft;
	PictureRef* iconDown;
	PictureRef* iconFolder;
	PictureRef* iconFile;

	GuiRect* selected;
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

	virtual void Broadcast(GuiRect::Funcs iFunc,const Msg& iMsg=Msg(),bool iCheckActive=true,bool iCheckFunin=true,bool iCheckFunout=true);
	virtual void BroadcastTo(GuiRect*,GuiRect::Funcs iFunc,const Msg& iMsg=Msg(),bool iCheckActive=true,bool iCheckFunin=true,bool iCheckFunout=true);
	virtual void BroadcastHitTest(GuiRect* iRect,vec2& iMouse,GuiRect*& oHitted);
	virtual void BroadcastPaint(GuiRect* iRect,const Msg& iMsg=Msg());

	template<class C> void GetRects(std::vector<C*>& iRects)
	{
		GuiRect* selectedRect=this->GetSelected();

		if(selectedRect)
			selectedRect->Get(iRects);
	}

	void Draw();
	void DrawBlock(bool);
	

	virtual bool BeginDraw()=0;
	virtual void EndDraw()=0;

	virtual void Create3DRenderer()=0;
	virtual void Destroy3DRenderer()=0;

	DrawInstance* SetDraw(GuiRect* iRect=0,bool iRemove=true,bool iSkip=false);

	virtual int TrackGuiSceneViewerPopup(bool iSelected)=0;
	virtual int TrackTabMenuPopup()=0;
	virtual int TrackProjectFileViewerPopup(ResourceNode*)=0;

	void SetSelection(GuiRect* iRect);
	GuiRect* GetSelected();

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
	GuiRect*  floatingSelectedTebGuiRect;
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

	void Broadcast(GuiRect::Funcs,const Msg& iMsg=Msg());
	
	template<typename T> void GetTabRects(std::vector<T*>& iRects)
	{
		for(std::vector<Tab*>::iterator tTab=this->tabs.begin();tTab!=this->tabs.end();tTab++)
			(*tTab)->GetRects(iRects);
	}

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
	
	void Broadcast(GuiRect::Funcs iFunc,const Msg& iMsg=Msg());

	template<typename T> void GetTabRects(std::vector<T*>& iRects)
	{
		for(std::vector<Container*>::iterator tContainer=this->containers.begin();tContainer!=this->containers.end();tContainer++)
			(*tContainer)->GetTabRects(iRects);
	}
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

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////FileSystem////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

struct DLLBUILD  EditorPropertiesBase
{
	virtual GuiPropertyTree::Container* Properties()=0;

	virtual void OnPropertiesCreate()=0;
	virtual void OnResourcesCreate()=0;
	virtual void OnPropertiesUpdate(Tab*)=0;
};

template<class T> struct DLLBUILD EntityProperties : EditorPropertiesBase , T
{
private:
	GuiPropertyTree::Container properties;
public:
	EntityProperties(){}
	virtual ~EntityProperties(){}

	GuiPropertyTree::Container* Properties(){return &this->properties;}

	virtual void OnPropertiesCreate(){};
	virtual void OnResourcesCreate(){};
	virtual void OnPropertiesUpdate(Tab*){};
};

template<class T> struct DLLBUILD ComponentProperties : EntityProperties<T>
{
	EditorEntity* Entity(){return (EditorEntity*)this->entity;}
};

struct DLLBUILD EditorEntity : EntityProperties<Entity>
{
	GuiSceneViewer::SceneLabel		sceneLabel;

private:
	Entity::childs;
	Entity::parent;
	Entity::components;
public:

	EditorEntity();
	~EditorEntity();

	EditorEntity*							Parent();
	const std::list<EditorEntity*>&			Childs();
	const std::list<EntityComponent*>&		Components();

	EditorEntity*							Append(EditorEntity*);
	void									Remove(EditorEntity*);

	EntityComponent*						AddComponent(EntityComponent*);
	void									RemoveComponent(EntityComponent*);

	void									DestroyChilds();

	void									OnPropertiesCreate();
	void									OnPropertiesUpdate(Tab*);

	Entity*									GetEntity(){return this;}

	void									SetName(String);

	template<class C> C* CreateComponent()
	{
		C* component=this->Entity::CreateComponent<C>();
		//component->OnResourcesCreate();
		component->OnPropertiesCreate();
		return component;
	}
};


struct DLLBUILD EditorAnimationController : ComponentProperties<AnimationController>
{
	float oldCursor;
	float minSpeed;
	float maxSpeed;

	GuiAnimationController*  guiAnimationController;

	EditorAnimationController();
	~EditorAnimationController();

	void OnPropertiesCreate();
	void OnPropertiesUpdate(Tab*);
};

struct DLLBUILD EditorMesh : ComponentProperties<Mesh>
{
	void OnPropertiesCreate();
	void OnPropertiesUpdate(Tab*);
};
struct DLLBUILD EditorRoot : ComponentProperties<Root>
{
	void OnPropertiesCreate();
	void OnPropertiesUpdate(Tab*);
};
struct DLLBUILD EditorSkeleton : ComponentProperties<Skeleton>
{
	void OnPropertiesCreate();
	void OnPropertiesUpdate(Tab*);
};
struct DLLBUILD EditorGizmo : ComponentProperties<Gizmo>
{
	void OnPropertiesCreate();
	void OnPropertiesUpdate(Tab*);
};
struct DLLBUILD EditorAnimation : ComponentProperties<Animation>
{
	void OnPropertiesCreate();
	void OnPropertiesUpdate(Tab*);
};
struct DLLBUILD EditorBone : ComponentProperties<Bone>
{
	void OnPropertiesCreate();
	void OnPropertiesUpdate(Tab*);
};
struct DLLBUILD EditorLine : ComponentProperties<Line>
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
struct DLLBUILD EditorLight : ComponentProperties<Light>
{
	void OnPropertiesCreate();
	void OnPropertiesUpdate(Tab*);
};
struct DLLBUILD EditorScript : ComponentProperties<Script>
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
		this->runtime ? Ide::Instance()->debugger->RunDebuggeeFunction(this,1),true : false;
	}

	void	SaveScript(String&);
	String	LoadScript();
};
struct DLLBUILD EditorCamera : ComponentProperties<Camera>
{
	void OnPropertiesCreate();
	void OnPropertiesUpdate(Tab*);
};
struct DLLBUILD EditorSkin : ComponentProperties<Skin>
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

	static GuiFont* GetDefaultFont();
	static void		SetDefaultFont(GuiFont*);
	static std::vector<GuiFont*>& GetFontPool();
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

	virtual void DrawText(const String& iText,float left,float top, float right,float bottom,unsigned int iColor=GuiString::COLOR_TEXT,const GuiFont* iFont=GuiFont::GetDefaultFont())=0;
	virtual void DrawText(const String& iText,float left,float top, float right,float bottom,vec2 iSpot,vec2 iAlign,unsigned int iColor=GuiString::COLOR_TEXT,const GuiFont* iFont=GuiFont::GetDefaultFont())=0;
	virtual void DrawLine(vec2 p1,vec2 p2,unsigned int iColor,float iWidth=0.5f,float iOpacity=1.0f)=0;
	virtual void DrawRectangle(float iX,float iY, float iWw,float iH,unsigned int iColor,bool iFill=true,float op=1.0f)=0;
	virtual void DrawRectangle(const vec4&,unsigned int iColor,bool iFill=true,float op=1.0f)=0;
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



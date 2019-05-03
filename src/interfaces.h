#ifndef INTERFACES_H
#define INTERFACES_H

#include "entities.h"
#include <map>

struct DLLBUILD EditorEntity;

struct DLLBUILD Renderer2D;
struct DLLBUILD Renderer3D;

struct DLLBUILD GuiRect;
struct DLLBUILD GuiScrollRect;
struct DLLBUILD GuiFont;
struct DLLBUILD GuiViewer;
struct DLLBUILD  EditorPropertiesBase;
template<typename T> struct DLLBUILD GuiProperty;
struct DLLBUILD GuiString;
struct DLLBUILD GuiButton;
struct DLLBUILD Picture;
struct DLLBUILD GuiAnimationController;
struct DLLBUILD GuiViewport;
struct DLLBUILD GuiPanel;
struct DLLBUILD GuiScene;
struct DLLBUILD GuiEntity;
struct DLLBUILD Compiler;
struct DLLBUILD GuiConsole;
struct DLLBUILD GuiProject;
struct DLLBUILD GuiScript;
struct DLLBUILD GuiLogger;
struct DLLBUILD GuiPropertyVec3;
struct DLLBUILD GuiPropertyPtr;
struct DLLBUILD GuiPropertyFloat;
struct DLLBUILD GuiPropertyBool;
struct DLLBUILD GuiPropertyAnimationController;
struct DLLBUILD DrawInstance;
struct DLLBUILD Frame;
#ifdef _WIN32
struct DLLBUILD FrameWin32; //for GuiCaret::Draw friendship
#endif
struct DLLBUILD MainFrame;
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

#define RENDERER_ENABLED 1
#define RENDERER_FRAMED 1
#define RENDERER_THREADED 0
#define PRINTPROCEDUREMESSAGESCALL 0

struct DLLBUILD GuiFont
{
	String	name;
	float	widths[255];
	float	height;
	float	tabSpaces;

	GuiFont();

	float GetHeight()const;
	vec2 MeasureText(const wchar_t*)const;
	vec4 MeasureText2(const wchar_t*)const;
	float GetCharWidth(wchar_t)const;
	float GetStringWidth(const wchar_t*)const;
	float GetStringWidth(const wchar_t*,const wchar_t*)const;

	static GuiFont* GetDefaultFont();
	static void		SetDefaultFont(GuiFont*);
	static std::vector<GuiFont*>& GetFontPool();
	static GuiFont* CreateFont(String iFontName,float iFontSize);
};

struct DLLBUILD TouchInput : Singleton<TouchInput>
{
	enum
	{
		TOUCH_DOWN,
		TOUCH_UP,
		TOUCH_MAX
	};

protected:
	bool pressed[MAX_TOUCH_INPUTS];
	bool released[MAX_TOUCH_INPUTS];

	vec2 position[MAX_TOUCH_INPUTS];

	TouchInput();
public:

	static TouchInput* Instance();

	bool IsPressed(int i);
	bool IsReleased(int i);

	void SetPressed(bool b,int i);
	void SetReleased(bool b,int i);

	vec2& GetPosition(int i);
	void   SetPosition(vec2& pos,int i);
};

struct DLLBUILD Keyboard : Singleton<Keyboard>
{
protected:
	unsigned int keymap[255];
	Keyboard();
public:

	static Keyboard* Instance();

	bool IsPressed(unsigned int iCharCode);
};

struct DLLBUILD Mouse : Singleton<Mouse>
{
protected:
	vec2	pos;
	vec2	posold;

	bool			buttons[3];
	unsigned int	previous[3];

	Mouse();
public:
	static Mouse* Instance();

	bool Left();
	bool Right();
	bool Middle();

	bool*			RawButtons(){return buttons;}
	unsigned int*	RawTiming(){return previous;}
};

struct DLLBUILD MsgData
{
	bool	skipchilds;
	bool	skipall;
	MsgData():skipchilds(false),skipall(false){};
};
struct DLLBUILD HitTestData : MsgData
{
	vec2				mouse;
	vec2				off;
	GuiRect*			hit;
	bool				locked;
	vec4				focusrect;
	GuiRect*			dragrect[4];
};
struct DLLBUILD MouseData : MsgData
{
	vec2					mouse;
	unsigned int			button;
	float					scroller;
	Mouse&					raw;
	MouseData(const vec2& iMouse,unsigned iBut,float iScr,Mouse& iMi):mouse(iMouse),button(iBut),scroller(iScr),raw(iMi){}
};

struct DLLBUILD KeyData : MsgData
{
	wchar_t				key;
	KeyData(wchar_t iKey):key(iKey){}
};
struct DLLBUILD ControlData : MsgData
{
	GuiRect*		control;
	unsigned int	msg;
	void*			data;
	ControlData(GuiRect* iControl,unsigned int iMsg,void* iData):control(iControl),msg(iMsg),data(iData){}
};
struct DLLBUILD PaintData : MsgData
{
	GuiRect* target;
	void*	 data;
	vec4	 clip;
	vec2	 offset;
	PaintData(void* iData=0):data(iData),clip(0,0,100000,100000){}
};
struct DLLBUILD MsgPtrData : MsgData
{
	void* ptr;
	MsgPtrData(void* iData=0):ptr(iData){}
};
struct DLLBUILD SizeData : MsgData
{
	vec4 edges;
	SizeData(const vec4& iEdges):edges(iEdges){}
};
struct DLLBUILD EnterExitData : MsgData
{
	MouseData&	mousedata;
	GuiRect*	enterexit;
	EnterExitData(MouseData& iMouseData,GuiRect* iEnterExit):mousedata(iMouseData),enterexit(iEnterExit){}
};

struct DLLBUILD Ide
{
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
	Frame*					popup;
public:

	Frame* CreatePopup(Frame*,float,float,float,float);
	void DestroyPopup();
	Frame* GetPopup();

	static Ide*				Instance();

protected:
	Ide();
public:

	virtual ~Ide();

	virtual void Run()=0;
	virtual void ScanProjectDirectoryForFileChanges()=0;

	virtual void ScanDir(String,ResourceNodeDir*)=0;

	String GetSceneExtension();
	String GetEntityExtension();
	const FilePath& GetProjectFolder();

	virtual void Sleep(int iMilliseconds=1)=0;
};

struct DLLBUILD GuiCaret : Singleton<GuiCaret>
{
protected:
	static const unsigned int BLINKRATE=300;

	friend MainFrame;
	friend Frame;
	friend FrameWin32;

	GuiRect*			rect;

	vec2				dim;
	vec2				olddim;
	
	vec4				caret;
	vec4				oldcaret;
	
	unsigned int		lasttime;
	unsigned int		blinktime;
	bool				blink;
	unsigned int		colorback;
	unsigned int		colorfront;

	void				Blink();
	void				Draw(unsigned int iCode=0);

	GuiCaret();
public:
	static GuiCaret*	Instance();

	void				SetPos(float,float);
	vec2				GetPos();

	void				SetDim(float,float);
	
	void				Hide();
	void				Show(GuiRect* iRect,unsigned int tBack=0xffffff,unsigned int tFront=0x000000);
	void				SetColors(unsigned int tBack,unsigned int tFront);
};

struct DLLBUILD StringEditor : Singleton<StringEditor>
{
	enum CaretOp
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

	struct DLLBUILD RowData
	{
		float			width;
		unsigned int	cols;

		RowData(float iWidth,unsigned int iNchars):width(iWidth),cols(iNchars){}
	};

	struct DLLBUILD Cursor
	{
	protected:
		friend StringEditor;
		friend GuiScript;

		unsigned int			p;
		unsigned int			row;
		unsigned int			col;
		vec2					caret;
		vec2					textpos;
		std::vector<RowData>	rowdata;
		vec2					maxes;
	public:
		Cursor();

		void Bind(String&);
		void InsertAt(float x,float y);
		void Reset();
		void SetCaretPosition();
	};

protected:
	StringEditor();
public:
	~StringEditor();

	static StringEditor*	Instance();

	void					InitCursorData(String&,Cursor&,GuiFont* iFont=GuiFont::GetDefaultFont());
	void					ParseInput(Frame*,GuiRect*,String&,Cursor&,const KeyData&,GuiFont* iFont=GuiFont::GetDefaultFont());
	void					Edit(Frame*,GuiRect*,String&,Cursor&,CaretOp iCaretOp=CARET_DONTCARE,GuiFont* iFont=GuiFont::GetDefaultFont(),void* iParam=0);
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

////////////GuiRect//////////

//GUIRECTSTATE

enum GuiRectFlag
{
	ACTIVE=0,
	CHILD,
	HIGHLIGHTED,
	PRESSED,
	CHECKED,
	PARENTCOLOR,
	SELFDRAW,
	MAXFLAGS
};

//MESSAGES

enum GuiRectMessages
{
	ONHITTEST=0,
	ONRECREATETARGET,
	ONPREPAINT,
	ONPAINT,
	ONENTITIESCHANGE,
	ONSIZE,
	ONMOUSEDOWN,
	ONMOUSECLICK,
	ONMOUSEUP,
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
	ONCONTROLEVENT,
	ONDRAWBARS,
	MAXMESSAGES
};

//MESSAGESDATA

//MESSAGES END



struct DLLBUILD GuiRect
{
	static const unsigned int COLOR_BACK=0x505050;

	virtual	void Procedure(Frame*,GuiRectMessages,void*);

	GuiRect*			parent;
	std::list<GuiRect*> childs;
	unsigned int		flags;
	vec4				edges;
	unsigned			color;

	const std::list<GuiRect*>& Childs(){return childs;}
	GuiRect*				   Parent(){return parent;}

	void SetFlag(GuiRectFlag iState,bool iValue);
	bool GetFlag(GuiRectFlag iState);

	virtual void		SetColor(unsigned iColor);
	unsigned			GetColor();

	GuiViewer* GetRoot();
	bool	   IsDescendantOf(GuiRect*);

	void Append(GuiRect*,bool isChild=true);
	void Remove(GuiRect*);

	virtual void OnSize(Frame*,const vec4& iNewSize);

	virtual void Draw(Frame*);

	GuiRect(unsigned int iState=1,unsigned int iColor=GuiRect::COLOR_BACK);
	virtual ~GuiRect(){};
};

////////////GuiViewer////////

struct DLLBUILD GuiViewer : Multiton<GuiViewer> , GuiRect
{
	static const unsigned int COLOR_BACK=0x707070;

	virtual void Procedure(Frame*,GuiRectMessages,void* iData=&MsgData());

	static const int LABEL_LEFT_OFFSET=5;
	static const int LABEL_RIGHT_OFFSET=10;
	static const int BAR_HEIGHT=30;
	static const int LABEL_WIDTH=80;
	static const int LABEL_HEIGHT=25;

protected:
	friend MainFrame;
	friend Frame;

	Frame*		frame;

	String				name;

	std::list<GuiViewer*>	siblings[4];

	std::list<GuiRect*>	tabs;
	std::list<String>	labels;
	std::list<vec4>		rects;
	GuiRect*			tab;

	GuiRect*			detachTab;
	float				detachSpot;

	float				labelsend;

	GuiViewer();
	GuiViewer(float x,float y,float z,float w);

public:

	virtual ~GuiViewer();

	static GuiViewer*				Instance();
	static std::list<GuiViewer*>&	GetPool();

	const std::list<GuiRect*>&      GetTabs();
	const std::list<String>&        GetLabels();

	GuiRect*	AddTab(GuiRect*,String iLabel);
	void		RemoveTab(GuiRect*);
	
	void		SetTab(GuiRect*);
	GuiRect*	GetTab();

	Frame*		GetFrame();

	vec4		GetTabEdges();
	vec4		GetBorderEdges();

	void		CalculateTabsRects(GuiRect* tExclude=0,vec4* tExcludeRect=0);
	vec4		GetLabelsArea();

	String		GetTabLabel(GuiRect* iTab);

	int			GetTabIndex(GuiRect*);
	vec4		GetLabelRectEdges(const vec4&);

	void		PrintSiblings();
};

//////////////////////////////////
//////////////////////////////////
////////////GuiScrollRect/////////
//////////////////////////////////
//////////////////////////////////

struct DLLBUILD GuiScrollRect : GuiRect
{
protected:

	virtual void Procedure(Frame*,GuiRectMessages,void* iData=&MsgData());

	static const unsigned int SCROLLH=0;
	static const unsigned int SCROLLV=1;
	static const unsigned int TICKNESS=20;

	float clength[2];
	float ratios[2];
	vec4 sedges[2][4];
	bool active[2];
	float positions[2];
	float content[2];
	float maxes[2];
	unsigned int scrollerhit;
	unsigned int scrollerpressed;
	float scroller;

	void CalculateScrollbarsLayout();
	
	void SetScrollbarPosition(unsigned int iType,float iPosition);
	
public:
	GuiScrollRect();

	void SetContent(float iHor,float iVer);
	void ResetContent();
	vec2 GetContent();

	float GetScrollbarPosition(unsigned int iType);
	float GetScrollerRatio(unsigned int iType);
	vec4 GetScrollerEdges(unsigned int iType);
	bool IsScrollbarVisible(unsigned int iType);

	vec4 GetClippingEdges();
	vec4 GetContentEdges();

	bool IsScrollbarHitted();
};

//////////////////////////////
//////////////////////////////
////////GuiPropertyTree///////
//////////////////////////////
//////////////////////////////

struct DLLBUILD GuiListBox;

struct DLLBUILD GuiListBoxNode
{
	enum Flags
	{
		SELECTED=0,
		HIGHLIGHTED
	};
protected:
	friend GuiListBox;
	unsigned int		flags;
	String				label;
public:

	GuiListBoxNode();
	GuiListBoxNode(const String&);

	void			SetLabel(const String&);
	const String&	GetLabel();

	virtual float	GetWidth();
	virtual float	GetHeight();

	virtual void	OnPaint(GuiListBox*,Frame*,const vec4&);
	virtual void	OnMouseDown(GuiListBox*,Frame*,const vec4&,const MouseData&);
	virtual void	OnMouseUp(GuiListBox*,Frame*,const vec4&,const MouseData&);
	virtual void	OnMouseMove(GuiListBox*,Frame*,const vec4&,const MouseData&);
};

template<typename T> struct DLLBUILD GuiListBoxItem : GuiListBoxNode
{
protected:
	T value;
public:
	GuiListBoxItem(T iT):value(iT){}

	T GetValue(){return value;}
};

struct DLLBUILD GuiListBox : GuiScrollRect
{
	struct DLLBUILD GuiListBoxData
	{
		vec4			contentedges;			
		vec4			labeledges;
		unsigned		idx;
		GuiListBoxNode* hit;
		void*			data;
		bool			skip;
	};

	virtual void Procedure(Frame*,GuiRectMessages,void* iData=&MsgData());
	virtual void ItemProcedure(GuiListBoxNode*,Frame*,GuiRectMessages,GuiListBoxData&);
private:
	std::list<String>						columns;
	std::list<GuiListBoxNode*>				items;
	std::list<GuiListBoxNode*>				selected;
	GuiListBoxNode*							hovered;
	vec4									hoverededges;

	GuiListBoxData	GetListBoxData(void* iData=0);
	virtual void	SetLabelEdges(GuiListBoxNode*,GuiListBoxData&);

	void SetFlag(GuiListBoxNode*,GuiListBoxNode::Flags iFlag,bool);
	bool GetFlag(GuiListBoxNode*,GuiListBoxNode::Flags iFlag);

	void SetHoverHighlight(Frame*,bool);
public:
	GuiListBox();

	void InsertItem(GuiListBoxNode&);
	void InsertItems(const std::list<GuiListBoxNode*>&);
	void RemoveItem(GuiListBoxNode&);
	void RemoveAll();

	void CalculateLayout();

	GuiListBoxNode* GetHoveredNode();
};

//////////////////////////////////
//////////////////////////////////
//////////////GuiImage////////////
//////////////////////////////////
//////////////////////////////////

struct GuiImage : GuiRect
{
	Picture* image;

	GuiImage();
	GuiImage(unsigned char* iRefData,float iWidth,float iHeight);
	GuiImage(String iFilename);
	~GuiImage();

	void OnPaint(Frame*,const PaintData&);
};

//////////////////////////////////
//////////////////////////////////
////////////GuiTreeView///////////
//////////////////////////////////
//////////////////////////////////

struct DLLBUILD GuiTreeView;
struct DLLBUILD GuiPropertyTree;

struct DLLBUILD GuiTreeViewNode
{
	enum Flags
	{
		SELECTED=0,
		HIGHLIGHTED,
		EXPANDED,
		SELECTABLE
	};
protected:
	friend GuiTreeView;
	friend GuiPropertyTree;

	GuiTreeViewNode*			parent;
	std::list<GuiTreeViewNode*>	childs;
	unsigned int				flags;
	String						label;

	GuiTreeViewNode();
public:
	void			SetLabel(const String&);
	const String&	GetLabel();
	
	void Insert(GuiTreeViewNode& iChild);
	void Remove(GuiTreeViewNode& iItem);

	virtual void SetBackgroundColor(unsigned iColor){}

	virtual void Reset();

	virtual void  OnPaint(Frame*,const vec4&,const float& tExpandosEnd);
	virtual float GetWidth(const float& tExpandosEnd);
	virtual float GetHeight();
	virtual void  OnExpandos(Frame*,const bool&);
	virtual void  OnMouseDown(Frame*,const MouseData&);
	virtual void  OnMouseUp(Frame*,const MouseData&);
};

template<typename T> struct DLLBUILD GuiTreeViewItem : GuiTreeViewNode
{
protected:
	T value;
public:
	GuiTreeViewItem(T iT):value(iT){}

	virtual T GetValue(){return this->value;}
};

struct DLLBUILD GuiTreeView : GuiScrollRect
{
	struct DLLBUILD GuiTreeViewData
	{
		unsigned int		level;
		vec4				contentedges;			
		vec4				labeledges;
		unsigned			idx;
		GuiTreeViewNode*    hit;
		void*				data;
		bool				skip;				
	};

	virtual void Procedure(Frame*,GuiRectMessages,void* iData=&MsgData());

protected:
	std::list<GuiListBoxNode*>	selected;
	GuiTreeViewNode*			root;
	GuiTreeViewNode*			hovered;
	vec4						hoveredg;
	unsigned					hoverlvl;
protected:
	virtual void ItemProcedure(GuiTreeViewNode*,Frame*,GuiRectMessages,GuiTreeViewData&);
	virtual void ItemLayout(Frame*,GuiTreeViewNode*,GuiTreeViewData&);
	virtual void ItemRoll(GuiTreeViewNode*,Frame*,GuiRectMessages,GuiTreeViewData&);

	GuiTreeViewData GetTreeViewData(void* iData=0);

	void SetFlag(GuiTreeViewNode* iItem,GuiTreeViewNode::Flags iFlag,bool);
	bool GetFlag(GuiTreeViewNode* iItem,GuiTreeViewNode::Flags iFlag);

	virtual void SetLabelEdges(GuiTreeViewNode*,GuiTreeViewData&);
	
	vec4 GetExpandosEdges(const vec4& iEdges);
public:
	GuiTreeView();

	void InsertRoot(GuiTreeViewNode& iItem);
	void RemoveRoot();

	virtual void CalculateLayout();

	GuiTreeViewNode* GetTreeViewHoveredNode();
	GuiTreeViewNode* GetTreeViewRootNode();
};

////////GuiPropertyTree///////

struct DLLBUILD GuiPropertyTreeNode : GuiTreeViewNode
{
protected:
	GuiPropertyTreeNode(const String& iDescription){this->SetLabel(iDescription);}
public:
};

template<typename T> struct DLLBUILD GuiPropertyTreeContainer : GuiPropertyTreeNode
{
protected:
	T value;
public:
	GuiPropertyTreeContainer(const String& iDescription,T iValue):GuiPropertyTreeNode(iDescription),value(iValue){}
	T GetValue(){return this->value;}
};

struct DLLBUILD GuiPropertyTreeItem : GuiPropertyTreeNode
{
protected:
	GuiRect*	property;
	float		height;
public:
	GuiPropertyTreeItem(const String& iDescription,GuiRect& iProperty,float iHeight=20):GuiPropertyTreeNode(iDescription),property(&iProperty),height(iHeight){}

	void  SetHeight(float iHeight=20){this->height=iHeight;}
	float GetHeight(){return this->height;}
	
	GuiRect* GetProperty(){return this->property;}

	void SetBackgroundColor(unsigned iColor){this->GetProperty()->SetColor(iColor);}
};

struct DLLBUILD GuiPropertyTree : GuiTreeView
{
protected:
	float splitter;
	bool  splitterpressed;
public:
	GuiPropertyTree():splitter(0),splitterpressed(false){}

	virtual void Procedure(Frame*,GuiRectMessages,void* iData=&MsgData());
	virtual void ItemProcedure(GuiTreeViewNode*,Frame*,GuiRectMessages,GuiTreeViewData&);

	void SetLabelEdges(GuiTreeViewNode*,GuiTreeViewData&);

	void CalculateLayout();
	float GetSplitterPos();
};

struct DLLBUILD GuiString : GuiRect
{
	static const unsigned int COLOR_TEXT=0xFFFFFF;

	virtual void Procedure(Frame*,GuiRectMessages,void* iData=&MsgData());

protected:
	friend StringEditor;

	String					text;
	GuiFont*				font;
	unsigned int			textcolor;
	vec2					spot;
	vec2					align;
	StringEditor::Cursor	cursor;
public:

	GuiString();

	const String&	GetText();
	void			SetText(const String&);

	unsigned int	GetTextColor();
	void			SetTextColor(unsigned int);

	void			SetFont(GuiFont*);
	GuiFont*		GetFont();

	void			SetSpot(float,float);
	const vec2&		GetSpot();

	void			SetAlignment(float,float);
	const vec2&		GetAlignment();
};

////////GuiStringProperty/////

struct DLLBUILD GuiStringProperty : GuiString
{
	virtual void Procedure(Frame*,GuiRectMessages,void* iData=&MsgData());

	enum
	{
		NONE=0,
		BOOL,
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

protected:
	static const unsigned int scDefaultParameter1=2;
	static const unsigned int scDefaultParameter2=2;

	void* valuePointer1;
	void* valuePointer2;
	unsigned int valueType;
	unsigned char valueParameter1;
	unsigned char valueParameter2;
public:
	GuiStringProperty(void* iValuePointer1,unsigned int iValueType,void* iValuePointer2=0,unsigned int iValueParameter1=scDefaultParameter1,unsigned int iValueParameter2=scDefaultParameter2);
};



struct DLLBUILD GuiTextBox : GuiString
{
	StringEditor::Cursor* cursor;

	GuiTextBox();
	~GuiTextBox();

	virtual bool ParseKeyInput(Frame*,const KeyData&);

	virtual void OnPaint(Frame*,const PaintData&);
	//virtual void OnMouseDown(Tab*,const MouseData&);
	virtual void OnKeyDown(Frame*,const KeyData&);
};


struct DLLBUILD GuiButton : GuiString
{
	virtual void Procedure(Frame*,GuiRectMessages,void* iData=&MsgData());

	unsigned buttonflags;

	GuiButton();
};

struct DLLBUILD GuiCheckButton : GuiButton
{
	GuiCheckButton();
	~GuiCheckButton();

	virtual void OnMouseUp(Frame*,const MsgData&);
};




struct DLLBUILD GuiSlider : GuiRect
{
	virtual  void Procedure(Frame*,GuiRectMessages,void*);

protected:
	float* referenceValue;

	float* minimum;
	float* maximum;
public:
	GuiSlider(float* iRef,float* iMin,float* iMax);

	void SetPosition(Frame*,const vec2& iMouse);

	float Value();
	float Min();
	float Max();
};




struct DLLBUILD GuiComboBox : GuiRect
{
	
};


struct DLLBUILD GuiPath : GuiRect
{
	GuiTextBox path;
	GuiButton  button;

	void (*func)(void*);
	void*  param;

	GuiPath();
	~GuiPath();
};

struct DLLBUILD GuiAnimationController : GuiRect
{
	virtual  void Procedure(Frame*,GuiRectMessages,void* iData=&MsgData());
protected:
	AnimationController& animationController;

	GuiButton play;
	GuiButton stop;
	GuiSlider position;
public:
	GuiAnimationController(AnimationController&);

	virtual void SetColor(unsigned int iColor)
	{
		GuiRect::SetColor(iColor);
		this->play.SetColor(iColor);
		this->stop.SetColor(iColor);
		this->position.SetColor(iColor);
	}


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
		this->description->SetText(iDescription);
		this->Append(this->description);

		this->property=iProperty ? iProperty : new T;
//		this->property->GetAutoEdges()->ref.x=&this->description->GetEdges().z;
		this->Append(this->property);

//		this->GetAutoEdges()->fix.w=this->property->GetAutoEdges()->fix.w;
	}
};

//////////////////GuiTabs///////////////////

struct DLLBUILD GuiViewport : Multiton<GuiViewport> , GuiRect
{
	static std::list<GuiViewport*>& GetPool();
	static GuiViewport* Instance();

	virtual void Procedure(Frame*,GuiRectMessages,void* iData=&MsgData());

protected:

	friend Frame;
	friend MainFrame;

	GuiViewport();

	Frame* frame;

	EditorEntity*		editorentity;

	mat4				projection;
	mat4				view;
	mat4				model;

	void*				bitmap;
	unsigned char*		buffer;

	vec2				mouseold;

	unsigned int		lastFrameTime;
	unsigned int		renderFps;

	bool				needsPicking;

	unsigned char		pickedPixel[4];
	EditorEntity*		pickedEntity;

	DrawInstance*		renderInstance;

public:

	virtual ~GuiViewport();

	Frame*					GetFrame();

	EditorEntity*			GetEntity();
	void					SetEntity(EditorEntity*);

	virtual int				Render(Frame*);
	virtual void			DrawBuffer(Frame*);
	virtual void			SwapBuffer(Frame*);
	virtual void			ResizeBuffers(Frame*);
	
	mat4&					GetProjectionMatrix();
	mat4&					GetViewMatrix();
	mat4&					GetModelMatrix();

	EditorEntity*			GetHoveredEntity();
	EditorEntity*			GetPickedEntity();
	
	unsigned int			GetLastFrameTime();
	void					SetRenderingRate(unsigned int iFps=60);
	unsigned int			GetRenderingRate();

	virtual void			OnSize(Frame*,const vec4&);
};

//////////////////GuiScript///////////////////

struct DLLBUILD GuiScript : Multiton<GuiScript> , GuiScrollRect
{
	static GuiScript*				Instance();
	static std::list<GuiScript*>&	GetPool();

	virtual void Procedure(Frame*,GuiRectMessages,void* iData=&MsgData());

protected:
	static const unsigned int BREAKPOINT_COLUMN_WIDTH=15;
	static const unsigned int LINENUMBERS_MARGINS=5;

	GuiFont*				font;
	String					string;
	StringEditor::Cursor	cursor;
	EditorScript*			editorscript;

	float					verticaltoolbar;
	unsigned int			horizontaltoolbar;

	unsigned int			colorbackbreak;
	unsigned int			colorfrontbreak;
	unsigned int			colorbacklines;
	unsigned int			colorfrontlines;
	unsigned int			colorbackeditor;
	unsigned int			colorfronteditor;
	
	GuiScript();
public:
	~GuiScript();

	void Save();

	void			SetEditorScript(EditorScript*);
	EditorScript*	GetEditorScript();

	int CountScriptLines();

	void DrawLineNumbers(Frame*);
	void DrawBreakpoints(Frame*);
	
	void CalculateLayout();
	float CalculateVerticalToolbarWidth();
	
};

struct DLLBUILD GuiLogger : Multiton<GuiLogger> , GuiListBox
{
	struct DLLBUILD GuiLoggerItem : GuiListBoxNode
	{
	protected:
		float height;
	public:
		GuiLoggerItem(const String& iString);
		float GetHeight();
	};

protected:
	std::list<GuiLoggerItem*>	logs;	
	GuiLogger();
public:

	~GuiLogger();

	static GuiLogger*				Instance();
	static std::list<GuiLogger*>&	GetPool();

	static GuiLogger* GetDefaultLogger();

	void			Clear();
	void			AppendLog(const String& iString);
	unsigned int	Count();

	static void Log(const String& iString,GuiLogger* iLogger=GuiLogger::GetDefaultLogger());
};

struct DLLBUILD Debugger : Singleton<Debugger> , GuiLogger
{
	struct DLLBUILD Breakpoint
	{
		void*	address;
		int		line;
		Script* script;
		bool	breaked;

		Breakpoint():address(0),line(0),script(0),breaked(0){}

		bool operator==(const Breakpoint& iLineAddress){return address==iLineAddress.address && line==iLineAddress.line;}
	};
protected:
	std::vector<Breakpoint> allAvailableBreakpoints;
	std::vector<Breakpoint> breakpointSet;

	EditorScript*		editorscript;
	Breakpoint*			breakpoint;

	void*				lastBreakedAddress;

	bool				suspended;
	bool				breaked;

	int					debuggerCode;

	unsigned int		function;

	int					sleep;

	Debugger();
public:

	static Debugger* Instance();

	virtual void RunDebuggeeFunction(EditorScript* iDebuggee,unsigned char iFunctionIndex);
	virtual void SuspendDebuggee(){}
	virtual void ResumeDebuggee(){}

	virtual void SetBreakpoint(Breakpoint&,bool){}

	virtual void BreakDebuggee(Breakpoint&){}
	virtual void ContinueDebuggee(){}

	virtual int HandleHardwareBreakpoint(void*){return 0;}
	virtual void SetHardwareBreakpoint(Breakpoint&,bool){}

	virtual void PrintThreadContext(void*){}

	virtual void StackUnwind(EditorScript*,void*){}

	std::vector<Breakpoint>& GetAllBreakpoint();
	std::vector<Breakpoint>& GetBreakpointSet();

	/*Script*			GetRunningScript();
	unsigned char	GetRunningScriptFunction();*/
};

struct DLLBUILD Compiler : Singleton <Compiler> , GuiLogger
{
	enum Type
	{
		COMPILER_MS=0,
		COMPILER_MINGW,
		COMPILER_LLVM
	};

protected:
	String ideSrcPath;
	String ideLibPath;

	struct COMPILER
	{
		String name;
		String version;
		String binDir;
		String libDir;
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

	String outputDirectory;

	Compiler();

public:

	static Compiler* Instance();

	~Compiler();
	
	const String& GetSourcesPath();
	const String& GetLibrariesPath();
	bool Compile(EditorScript*);
	bool LoadScript(EditorScript*);
	bool UnloadScript(EditorScript*);
	bool ParseCompilerOutputFile(String iFileBuffer);
	const COMPILER& GetCompiler(Type);
};




struct DLLBUILD GuiPanel : GuiRect
{
	GuiPanel();
};



struct DLLBUILD GuiScene : Multiton<GuiScene> , GuiTreeView , Scene
{
	virtual void Procedure(Frame*,GuiRectMessages,void* iData=&MsgData());

protected:
	GuiScene();
public:

	EditorEntity*			GetSceneRootEditorEntity(); 
	void					SetSceneRootEditorEntity(EditorEntity*); 

	static GuiScene*  Instance();
	static std::list<GuiScene*>&  GetPool();

	void Save(Frame*,String);
	void Load(Frame*,String);
};

struct DLLBUILD GuiEntity : Multiton<GuiEntity> , GuiPropertyTree
{
	virtual void Procedure(Frame*,GuiRectMessages,void* iData=&MsgData());

private:
	EditorEntity* rootEntityProperties;
	GuiEntity();
public:

	~GuiEntity();

	void			SetEntity(EditorEntity*);
	EditorEntity*	GetEntity();

	static GuiEntity*				Instance();
	static std::list<GuiEntity*>&	GetPool();
};

struct DLLBUILD GuiConsole : GuiRect
{
private:
	GuiConsole();
public:
	~GuiConsole();
};

struct DLLBUILD GuiProject : Singleton<GuiProject> , GuiRect
{
	struct DLLBUILD DirView : GuiTreeView
	{
	};

	struct DLLBUILD FileView : GuiListBox
	{
	};

	struct DLLBUILD DataView : GuiRect
	{
		DataView();
	};

protected:

	virtual void Procedure(Frame*,GuiRectMessages,void* iData=&MsgData());

	DirView dirView;
	FileView fileView;
	DataView resView;

	ResourceNodeDir*& projectDirectory;

	bool splitterLeftActive;
	bool splitterRightActive;

	float splitterLeft;
	float splitterRight;

	bool  leftmousepressing;

private:
	GuiProject();
public:
	~GuiProject();

	static GuiProject* Instance();

	ResourceNodeDir& GetProjectDirectory(){return *projectDirectory;}

	void OnSize(Frame*,const vec4&);

	void Scan();
	void Reset();

	void Delete(Frame*,ResourceNode*);

	float GetLeftSplitter();
	float GetRightSplitter();

	void findResources(std::vector<ResourceNode*>& oResultArray,ResourceNode* iResourceNode,String iExtension);

	std::vector<ResourceNode*> findResources(String iExtension);
};

struct DLLBUILD ResourceNode : GuiListBoxItem<ResourceNode*>
{
protected:
	friend ResourceNodeDir;

	ResourceNode*	parentNode;
	friend			Resource;
	FilePath		fileName;
public:
	
	ResourceNodeDir*		IsDir();
	ResourceNode*			GetParentNode();
	FilePath				GetFilename();
	virtual void			SetFilename(const String& iFilename);
	String					BuildPath();

	void OnMouseUp(GuiListBox*,Frame*,const vec4&,const MouseData&);

	ResourceNode();
	virtual ~ResourceNode();
};

struct DLLBUILD ResourceNodeDir : ResourceNode , GuiTreeViewItem<ResourceNodeDir*>
{
	std::list<ResourceNodeDir*> dirs;
	std::list<ResourceNode*>	files;
	std::list<GuiListBoxNode*>	fileLabels;

	void InsertNode(ResourceNode*);

	static ResourceNodeDir*	FindDirNode(String);
	static ResourceNode*	FindFileNode(String);
	static ResourceNodeDir* GetRootDirNode();

	virtual void			SetFilename(const String& iFilename);

	ResourceNodeDir();
	virtual ~ResourceNodeDir();
};

struct DLLBUILD WindowData
{
	virtual void Enable(bool)=0;
	virtual bool IsEnabled()=0;

	virtual vec2 GetSize()=0;
	virtual vec2 GetPos()=0;
	virtual void SetPos(float x,float y)=0;
	virtual void Show(bool)=0;
	virtual bool IsVisible()=0;

	virtual int GetWindowHandle()=0;

	virtual void SendMessage(unsigned iCode=0,unsigned data1=0,unsigned data2=0)=0;
	virtual void PostMessage(unsigned iCode=0,unsigned data1=0,unsigned data2=0)=0;
};

struct DLLBUILD DrawInstance
{
	GuiRect*		rect;
	bool			skip;
	bool			remove;
	void*			param;

	DrawInstance(GuiRect* iRect,bool iRemove=true,bool iSkip=false,void* iParam=0);
	~DrawInstance();
};


struct DLLBUILD Frame
{
	static const int COLOR_BACK=0x202020;
	static const int COLOR_LABEL=0x404040;

	static const int ICON_WH=20;
	static const int ICON_STRIDE=ICON_WH*4;

	WindowData* windowData;

	static unsigned char rawUpArrow[];
	static unsigned char rawRightArrow[];
	static unsigned char rawLeftArrow[];
	static unsigned char rawDownArrow[];
	static unsigned char rawFolder[];
	static unsigned char rawFile[];

	MainFrame*				mainframe;

	std::list<GuiViewer*>	viewers;
	std::list<GuiViewer*>	draggables[4];

	unsigned	dragcode;
	unsigned	sizeboxcode;

	GuiRect* focused;

	Renderer2D *renderer2D;
	Renderer3D *renderer3D;

	PictureRef* iconUp;
	PictureRef* iconRight;
	PictureRef* iconLeft;
	PictureRef* iconDown;
	PictureRef* iconFolder;
	PictureRef* iconFile;

	HitTestData hittest;
	MouseData	mousedata;

	bool drag;
	void* wasdrawing;
	bool retarget;
	bool trackmouseleave;
	bool skipFrameMouseExit;
	bool dragframe;

	vec2 previousSize;

	bool isModal;

	bool hasFrame;

	std::list<DrawInstance*>			drawInstances;
	std::list< std::function<void()> >  concurrentInstances;

	struct DLLBUILD ClipData
	{
		vec4 clip;
		vec2 coord;
	};

	std::list<ClipData>		clips;
	
	vec2 mouse;

	unsigned int lastFrameTime;

	Thread* renderingThread;
protected:
	Frame(float x,float y,float w,float h);
public:
	virtual ~Frame();

	std::list<Frame*>& GetPool();

	std::list<GuiViewer*>& GetViewers();

	void		RemoveViewer(GuiViewer*);
	GuiViewer*	AddViewer(GuiViewer*,const String&,unsigned int iPos,GuiViewer* iSibling);
	GuiViewer*	AddViewer(GuiViewer*,const String&);
	GuiViewer*	GetMainViewer();

	virtual void OnPaint();
	virtual void OnSize(float,float);
	virtual void OnMouseDown(unsigned int iButton);
	virtual void OnMouseUp(unsigned int iButton);
	virtual void OnMouseClick(unsigned int iButton);
	virtual void OnMouseMove(float,float);
	virtual void OnMouseWheel(float);
	virtual void OnKeyDown(wchar_t iKey);
	virtual void OnKeyUp(wchar_t iKey);
	virtual void OnSizeboxDown(unsigned);
	virtual void OnSizeboxUp();
	virtual void OnMouseLeave();

	MainFrame* GetMainFrame(){return this->mainframe;}
	
	virtual vec2 Size();

	virtual void Destroy(){}

	virtual void OnRecreateTarget();

	virtual void Message(GuiRect*,GuiRectMessages,void* iData=&MsgData());
	virtual void Broadcast(GuiRectMessages,void* iData=&MsgData());
	virtual void BroadcastTo(GuiRect*,GuiRectMessages,void* iData=&MsgData());
	virtual void BroadcastInputTo(GuiRect*,GuiRectMessages,void* iData=&MsgData());
	virtual void BroadcastPaintTo(GuiRect*,void* iData=&PaintData());
	template <typename T> void BroadcastToPool(GuiRectMessages,void* iData=&MsgData());
	template <typename T> void MessagePool(GuiRectMessages,void* iData=&MsgData());

	template<class C> void GetRects(std::vector<C*>& iRects)
	{
		/*GuiRect* selectedRect=this->GetCurrentViewer();

		if(selectedRect)
			selectedRect->Get(iRects);*/
	}

	void Draw();
	
	virtual bool BeginDraw(void*){return false;}
	virtual void EndDraw(){};

	DrawInstance*	SetDraw(GuiRect* iRect=0,bool iRemove=true,bool iSkip=false,void* iParam=0);
	void			SetDraw(GuiViewport*);

	virtual int TrackGuiSceneViewerPopup(bool iSelected){return -1;}
	virtual int TrackTabMenuPopup(){return -1;}
	virtual int TrackProjectFileViewerPopup(ResourceNode*){return -1;}

	virtual void SetCursor(int){};

	void		SetFocus(GuiRect*);
	GuiRect*	GetFocus();

	void PushScissor(vec4,vec2);
	void PopScissor(ClipData* oClipData=0);

	void Enable(bool);
	bool IsEnabled();
};

struct DLLBUILD MenuInterface
{
	virtual void OnMenuPressed(Frame*,int iIdx)=0;
	int Menu(String iName,bool tPopup);

	static int GetMenuId();
	static int IncrementMenuId();
};

struct DLLBUILD MainFrame : Singleton<MainFrame> , MenuInterface
{
	std::list<Frame*>   frames;
	Frame*				frame;

	int MenuBuild;
	int MenuPlugins;
	int MenuFile;
	int MenuInfo;
	int MenuActionBuildPC;
	int MenuActionExit;
	int MenuActionConfigurePlugin;
	int MenuActionProgramInfo;

	Thread*  renderingThread;
	Task*	 renderingTask;

	Thread*  caretThread;
	Task*	 caretTask;

	MainFrame();
	virtual ~MainFrame();

	static MainFrame* Instance();
	static bool		  IsInstanced();

	void Initialize();
	void Deintialize();

	Frame* CreateFrame(float,float,float,float,bool iCentered=false);
	void   DestroyFrame(Frame*);
	void	AddFrame(Frame*);
	void	RemoveFrame(Frame*);
	Frame* GetFrame();
	void OnMenuPressed(Frame*,int iIdx);
	void Enable(bool);
	void Render();
	void BlinkCaret();
};

struct DLLBUILD Subsystem
{
	static bool Execute(String iPath,String iCmdLine,String iOutputFile=L"",bool iInput=false,bool iError=false,bool iOutput=false,bool iNewConsole=false);
	static unsigned int FindProcessId(String iProcessName);
	static unsigned int FindThreadId(unsigned int iProcessId,String iThreadName);
	static String DirectoryChooser(String iDescription,String iExtension);
	static String FileChooser(wchar_t* iFilter=0,unsigned int iFilterIndex=0);
	static std::vector<String> ListDirectories(String iDir);
	static bool CreateDir(String);
	static bool CancelDir(String);
	static bool DirectoryExist(String);
	static String RandomDir(String iWhere,int iSize,String iAlphabet=L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	static void* LoadLib(String);
	static bool FreeLib(void*);
	static void* GetLibFunction(void*,String);
	static void SystemMessage(String iTitle,String iMessage,unsigned iFlags);
	static int TrackMenu(int iPopupMenu,Frame*,float x,float y);

};




struct DLLBUILD PluginSystem : Singleton<PluginSystem>
{
	struct DLLBUILD Plugin : MenuInterface
	{
		String						name;
		bool						loaded;
		void						*handle;

		struct DLLBUILD PluginListBoxItem : GuiListBoxItem<Plugin*>
		{
			PluginListBoxItem(Plugin*);

			void OnPaint(GuiListBox*,Frame*,const vec4&);
			void OnMouseDown(GuiListBox*,Frame*,const vec4&,const MouseData&);
			void OnMouseUp(GuiListBox*,Frame*,const vec4&,const MouseData&);
			void OnMouseMove(GuiListBox*,Frame*,const vec4&,const MouseData&);
		};

		PluginListBoxItem			listBoxItem;

		Plugin();

		virtual void Load();
		virtual void Unload();
		virtual void OnMenuPressed(Frame*,int iIdx)=0;
	};

protected:	
	friend Frame;

	struct DLLBUILD GuiPluginTab : Singleton<GuiPluginTab> , GuiListBox
	{
		virtual void Procedure(Frame*,GuiRectMessages,void* iData=&MsgData());

		PluginSystem*		pluginSystem;
		GuiButton			exitButton;

		GuiPluginTab(PluginSystem*);

		void OnSize(Frame*,const vec4&);
	};

	GuiPluginTab			pluginsTab;
	std::list<Plugin*>		plugins;
	
	virtual void ScanPluginsDirectory();

	PluginSystem();
public:
	~PluginSystem();

	static PluginSystem*	Instance();
	void					ShowGui();
};

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////FileSystem////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

struct DLLBUILD  EditorPropertiesBase
{
protected:
	ResourceNode*			resourceNode;
	GuiPropertyTreeNode& propertytreenode;

	EditorPropertiesBase(GuiPropertyTreeNode& iPropertyTreeNode):propertytreenode(iPropertyTreeNode),resourceNode(0){}
public:

	GuiTreeViewNode& TreeViewNode(){return this->propertytreenode;}

	virtual void OnInitProperties(){};
	virtual void OnInitResources(){};
	virtual void OnUpdateProperties(Frame*){};
};

template<class T> struct DLLBUILD EntityProperties : EditorPropertiesBase , GuiPropertyTreeContainer<T*> , T
{
	EntityProperties():
		EditorPropertiesBase(*dynamic_cast< GuiPropertyTreeContainer<T*>* >(this)),
		GuiPropertyTreeContainer<T*>(L"",this)
		{}
	virtual ~EntityProperties(){}

	GuiPropertyTreeContainer<T*>& TreeViewContainer(){return *dynamic_cast< GuiPropertyTreeContainer<T*>* >(this);}

	virtual void OnInitProperties(){};
	virtual void OnInitResources(){};
	virtual void OnUpdateProperties(Frame*){};
};

template<class T> struct DLLBUILD ComponentProperties : EntityProperties<T>
{
	EditorEntity* Entity(){return (EditorEntity*)this->T::Entity();}
	T& GetComponent(){return *this;}
};

struct DLLBUILD GuiSceneEntityLabel : GuiTreeViewItem<EditorEntity*>
{
	GuiSceneEntityLabel(EditorEntity* iEditorEntity);
	virtual void OnMouseUp(Frame*,const MouseData&);
};

struct DLLBUILD EditorEntity : EntityProperties<Entity>
{
	EditorEntity();
	~EditorEntity();

	EditorEntity*							Parent();
	const std::list<EditorEntity*>&			Childs();

	EditorEntity*							Append(EditorEntity*);
	void									Remove(EditorEntity*);

	void									DestroyChilds();

	void									OnInitProperties();
	void									OnUpdateProperties(Frame*);

	void									SetName(String);
	GuiSceneEntityLabel&						GetSceneLabel();

	static unsigned int						GetInstancedEntitiesNumber();

	template<class C> C* CreateComponent()
	{
		C* tCreatedComponent=this->Entity::CreateComponent<C>();
		//component->OnResourcesCreate();
		tCreatedComponent->OnInitProperties();
		tCreatedComponent->Entity()->TreeViewContainer().Insert(tCreatedComponent->TreeViewContainer());
		return tCreatedComponent;
	}

private:
	GuiStringProperty				spName;
	GuiStringProperty				spPtr;
	GuiStringProperty				spId;
	GuiStringProperty				spPosition;
	GuiStringProperty				spChilds;
	GuiStringProperty				spMin;
	GuiStringProperty				spMax;
	GuiStringProperty				spVolume;

	GuiPropertyTreeItem				pName;
	GuiPropertyTreeItem				pPtr;
	GuiPropertyTreeItem				pId;
	GuiPropertyTreeItem				pPosition;
	GuiPropertyTreeItem				pChilds;
	GuiPropertyTreeItem				pMin;
	GuiPropertyTreeItem				pMax;
	GuiPropertyTreeItem				pVolume;

	GuiPropertyTreeContainer<AABB*> pcAABB;

	GuiSceneEntityLabel				sceneLabel;

	static unsigned int						instancedEntitiesNumber;
public:
};

struct DLLBUILD EditorAnimationController : ComponentProperties<AnimationController> , GuiAnimationController
{
	float oldCursor;
	float minSpeed;
	float maxSpeed;

	GuiStringProperty		spNumNodes;
	GuiSlider				spVelocity;
	GuiStringProperty		spDuration;
	GuiStringProperty		spBegin;
	GuiStringProperty		spEnd;

	GuiPropertyTreeItem pNumNodes;
	GuiPropertyTreeItem pVelocity;
	GuiPropertyTreeItem pDuration;
	GuiPropertyTreeItem pBegin;
	GuiPropertyTreeItem pEnd;
	GuiPropertyTreeItem pPlayer;

	EditorAnimationController();
	~EditorAnimationController();

	void OnInitProperties();
	void OnUpdateProperties(Frame*);
};

struct DLLBUILD EditorMesh : ComponentProperties<Mesh>
{
	GuiStringProperty		spControlpoints;
	GuiStringProperty		spNormals;
	GuiStringProperty		spPolygons;
	GuiStringProperty		spTextCoord;
	GuiStringProperty		spVertexIndices;

	GuiPropertyTreeItem pControlpoints;
	GuiPropertyTreeItem pNormals;
	GuiPropertyTreeItem pPolygons;
	GuiPropertyTreeItem pTextCoord;
	GuiPropertyTreeItem pVertexIndices;

	EditorMesh();

	void OnInitProperties();
	void OnUpdateProperties(Frame*);
};
struct DLLBUILD EditorRoot : ComponentProperties<Root>
{
	void OnInitProperties();
	void OnUpdateProperties(Frame*);
};
struct DLLBUILD EditorSkeleton : ComponentProperties<Skeleton>
{
	void OnInitProperties();
	void OnUpdateProperties(Frame*);
};
struct DLLBUILD EditorGizmo : ComponentProperties<Gizmo>
{
	void OnInitProperties();
	void OnUpdateProperties(Frame*);
};
struct DLLBUILD EditorAnimation : ComponentProperties<Animation>
{
	GuiStringProperty		spIsBone;
	GuiStringProperty		spDuration;
	GuiStringProperty		spBegin;
	GuiStringProperty		spEnd;

	GuiPropertyTreeItem pIsBone;
	GuiPropertyTreeItem pDuration;
	GuiPropertyTreeItem pBegin;
	GuiPropertyTreeItem pEnd;

	EditorAnimation();

	void OnInitProperties();
	void OnUpdateProperties(Frame*);
};
struct DLLBUILD EditorBone : ComponentProperties<Bone>
{
	void OnInitProperties();
	void OnUpdateProperties(Frame*);
};
struct DLLBUILD EditorLine : ComponentProperties<Line>
{
private:
	using Line::points;
public:

	GuiListBox								pointListBox;
	GuiStringProperty						spNumSegments;
	GuiPropertyTreeItem						pNumSegments;

	EditorLine();

	void OnInitProperties();
	void OnUpdateProperties(Frame*);

	void DestroyPoints();
	void Append(vec3);

};
struct DLLBUILD EditorLight : ComponentProperties<Light>
{
	void OnInitProperties();
	void OnUpdateProperties(Frame*);
};
struct DLLBUILD EditorScript : ComponentProperties<Script>
{
	enum State
	{
		IDLE=0,
		INIT,
		UPDATE,
		DEINIT,
		BREAK,
		MAXSTATE
	};

	struct DLLBUILD EditorScriptButtons : GuiRect
	{
		virtual void Procedure(Frame*,GuiRectMessages,void* iData=&MsgData());
	protected:
		EditorScript*		editorscript;
		GuiButton			buttonCompile;
		GuiButton			buttonEdit;
		GuiButton			buttonLaunch;
	public:
		EditorScriptButtons(EditorScript*);

		void SetColor(unsigned int);
	};

protected:
	friend Debugger;
	friend Compiler;
	friend GuiScript;

	GuiScript*			scriptViewer;

	bool				isRunning;

	FilePath			libpath;
	String				sourcetext;

	GuiStringProperty	spFilePath;
	GuiStringProperty	spIsRunning;
	EditorScriptButtons spButtons;

	GuiPropertyTreeItem	pFilePath;
	GuiPropertyTreeItem	pIsRunning;
	GuiPropertyTreeItem	pActions;
public:

	EditorScript();

	void	OnInitProperties();
	void	OnUpdateProperties(Frame*);
	void	OnInitResources();

	void	SaveScript();
	void	LoadScript();
	bool	CompileScript();

	void    init();
	void    deinit();
	void	update();

	GuiScript*	GetViewer();

	FilePath GetLibPath();
};
struct DLLBUILD EditorCamera : ComponentProperties<Camera>
{
	void OnInitProperties();
	void OnUpdateProperties(Frame*);
};
struct DLLBUILD EditorSkin : ComponentProperties<Skin>
{
	GuiStringProperty		spClusters;
	GuiStringProperty		spTextures;

	GuiPropertyTreeItem pClusters;
	GuiPropertyTreeItem pTextures;

	EditorSkin();

	void OnInitProperties();
	void OnUpdateProperties(Frame*);
};





struct DLLBUILD Renderer2D
{
	Frame*			frame;

	unsigned int	tabSpaces;

	Renderer2D(Frame*);
	~Renderer2D();

	virtual void DrawText(const String& iText,float left,float top, float right,float bottom,unsigned int iColor=GuiString::COLOR_TEXT,const GuiFont* iFont=GuiFont::GetDefaultFont())=0;
	virtual void DrawText(const String& iText,float left,float top, float right,float bottom,vec2 iSpot,vec2 iAlign,unsigned int iColor=GuiString::COLOR_TEXT,const GuiFont* iFont=GuiFont::GetDefaultFont())=0;
	virtual void DrawLine(vec2 p1,vec2 p2,unsigned int iColor,float iWidth=0.5f,float iOpacity=1.0f)=0;
	virtual void DrawRectangle(float iX,float iY, float iWw,float iH,unsigned int iColor,float iStroke=0,float op=1.0f)=0;
	virtual void DrawRoundRectangle(float x,float y, float w,float h,float iRadiusA,float iRadiusB,unsigned iColor,float iStroke=0,float iOpacity=1.0f)=0;
	virtual void DrawCircle(float x,float y,float iRadius,unsigned iColor,float iStroke=0,float iOpacity=1.0f)=0;
	virtual void DrawEllipse(float x,float y,float iRadiusA,float iRadiusB,unsigned iColor,float iStroke=0,float iOpacity=1.0f)=0;
	virtual void DrawBitmap(Picture* bitmap,float x,float y, float w,float h)=0;

	virtual bool LoadBitmap(Picture*)=0;

	virtual unsigned int ReadPixel(float x,float y)=0;

	virtual void PushScissor(float x,float y,float w,float h)=0;
	virtual void PopScissor()=0;

	virtual void Translate(float,float)=0;
	virtual void Identity()=0;

	virtual void SetAntialiasing(bool)=0;

	static void EnableCaret();
	static void DrawCaret();
};

struct DLLBUILD Renderer3D : Renderer3DBase
{
	Frame* frame;

	virtual Shader* CreateShaderProgram(const char* shader_name,const char* pixel_shader,const char* fragment_shader)=0;

	Shader* FindShader(const char* name,bool exact);
	void SetMatrices(const float* view,const float* mdl);

	virtual char* Name()=0;

	virtual void DrawPoint(vec3,float psize=1.0f,vec3 color=vec3(1,1,1))=0;
	virtual void draw(vec2)=0;
	virtual void DrawLine(vec3,vec3,vec3 color=vec3(1,1,1))=0;
	virtual void draw(vec4)=0;
	virtual void draw(AABB,vec3 color=vec3(1,1,1))=0;
	virtual void draw(mat4 mtx,float size,vec3 color=vec3(1,1,1))=0;
	//virtual void draw(Font*,char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4)=0;
	virtual void DrawText(char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4)=0;


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

	Renderer3D(Frame*);
	virtual ~Renderer3D(){};
};


#endif //INTERFACES_H



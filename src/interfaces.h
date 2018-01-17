#ifndef INTERFACES_H
#define INTERFACES_H

#include "entities.h"

struct EditorEntity;

struct GuiRect;
struct GuiRootRect;
struct GuiString;
struct GuiButton;
struct GuiImage;
struct GuiButtonBool;
struct GuiScrollBar;
struct GuiScrollRect;
struct GuiPropertyString;
struct GuiPropertySlider;
struct GuiAnimationController;
struct GuiLabel;
struct GuiViewport;
struct GuiSceneViewer;
struct GuiEntityViewer;
struct GuiCompilerViewer;
struct GuiProjectViewer;
struct GuiScriptViewer;
struct GuiPropertyVec3;
struct GuiPropertyPtr;
struct GuiPropertyFloat;
struct GuiPropertyBool;
struct GuiPropertyAnimationController;

struct TabContainer;
struct EditorWindowContainer;
struct SplitterContainer;
struct EditorMainAppWindow;
struct ResourceNodeDir;
struct CompilerInterface;

//entity forward declaration 

struct Entity;
struct AnimationController;
struct Script;

#define ENGINE_EXTENSION ".__engine_"


#define MAX_TOUCH_INPUTS 10


struct InputInterface
{
};

struct TouchInput : InputInterface
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

struct KeyboardInput : InputInterface
{
	bool IsPressed(unsigned int iCharCode);
};

struct MouseInput : InputInterface
{
	vec2	pos;
	vec2	posold;

	bool left;
	bool right;
	bool middle;

	MouseInput();

	bool Left();
	bool Right();
	bool Middle();
};


struct InputManager
{
	static TouchInput touchInput;
	static MouseInput mouseInput;
	static KeyboardInput keyboardInput;
	/*static InputInterface voiceInput;
	static InputInterface joystickInput;*/
};



struct AppInterface : TStaticInstance<AppInterface>
{
	Timer					*timerMain;
	EditorMainAppWindow		*mainAppWindow;
	String					projectFolder;
	FilePath				exeFolder;
	String					applicationDataFolder;
	CompilerInterface*		compiler; 
	InputManager            inputManager;
	unsigned int            caretLastTime;

	AppInterface();

	virtual int Initialize()=0;	
	virtual void Deinitialize()=0;
	virtual void Run()=0;

	virtual void CreateNodes(String,ResourceNodeDir*)=0;
	virtual void ScanDir(String)=0;
};

struct Renderer2DInterface
{
	static const unsigned int COLOR_TAB_BACKGROUND=0x808080;
	static const unsigned int COLOR_TAB_SELECTED=0x0000FF;
	static const unsigned int COLOR_GUI_BACKGROUND = 0x707070;
	static const unsigned int COLOR_MAIN_BACKGROUND = 0x505050;
	static const unsigned int COLOR_TEXT=0xFFFFFF;
	static const unsigned int COLOR_TEXT_SELECTED=0x0000ff;
	static const unsigned int COLOR_TEXT_HOVERED=0x0000f1;

	unsigned int colorBackgroud;
	unsigned int colorText;

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
};

struct Renderer3DInterface : Renderer3DInterfaceBase
{
	std::list<GuiViewport*> viewports;
	Task* rendererTask;
	bool picking;
	std::vector<ShaderInterface*> shaders;
	TabContainer* tabContainer;

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
	virtual void draw(Script*)=0;

	virtual void draw(EntityComponent*)=0;
	virtual void draw(Entity*)=0;

	virtual void ChangeContext()=0;

	virtual void Render(GuiViewport*,bool)=0;
	virtual void Render()=0;

	Renderer3DInterface(TabContainer*);
	virtual ~Renderer3DInterface(){};

	ShaderInterface* unlit;
	ShaderInterface* unlit_color;
	ShaderInterface* unlit_texture;
	ShaderInterface* font;
	ShaderInterface* shaded_texture;
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



struct GuiRect : THierarchyVector<GuiRect>
{
	static const int TREEVIEW_ROW_HEIGHT=20;
	static const int TREEVIEW_ROW_ADVANCE=TREEVIEW_ROW_HEIGHT;
	static const int SCROLLBAR_WIDTH=20;
	static const int SCROLLBAR_TIP_HEIGHT=SCROLLBAR_WIDTH;
	static const int SCROLLBAR_AMOUNT=10;

	String name;

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
	virtual void OnKeyDown(TabContainer*,void* data=0);
	virtual void OnKeyUp(TabContainer*,void* data=0);
	virtual void OnMouseEnter(TabContainer*,void* data=0);
	virtual void OnMouseExit(TabContainer*,void* data=0);

	virtual void OnButtonPressed(TabContainer*,GuiButton*){}

	virtual void DrawBackground(TabContainer*);

	virtual void SelfRenderEnd(TabContainer*,bool&);

	virtual bool BeginSelfClip(TabContainer*);
	virtual void EndSelfClip(TabContainer*,bool&);

	virtual void SetClip(GuiScrollRect*);

	virtual GuiRect* GetRoot(); 
	GuiRootRect* GetRootRect(); 

	bool _contains(vec4& quad,vec2);

	void BroadcastToChilds(void (GuiRect::*func)(TabContainer*,void*),TabContainer*,void* data=0);
	void BroadcastToRoot(void (GuiRect::*func)(TabContainer*,void*),void* data=0);
	template<class C> void BroadcastTo(void (GuiRect::*func)(TabContainer* iTabContainer))
	{
		C* isaC=dynamic_cast<C*>(this);

		if(isaC)
			(this->*func)(iTabContainer);

		for(std::vector<GuiRect*>::iterator tRect=this->childs.begin();tRect!=this->childs.end();tRect++)
			((*tRect)->*func)(iTabContainer);
	}

	GuiString* Container(const char* iText);

	GuiRect* Rect(float ix=0, float iy=0, float iw=0,float ih=0,float apx=0, float apy=0, float arx=1,float ary=1);

	GuiString* Text(String str,float ix=0, float iy=0, float iw=0,float ih=0,vec2 _alignText=vec2(-1,-1));
	GuiString* Text(String str,vec2 _alignPos=vec2(-1,-1),vec2 _alignRect=vec2(-1,-1),vec2 _alignText=vec2(-1,-1));

	GuiPropertyString* Property(const char* iLeft,String& iRight);
	GuiPropertyVec3* Property(const char* iLeft,vec3& iRight);
	GuiPropertyPtr* Property(const char* iLeft,void* iRight);
	GuiPropertyFloat* Property(const char* iLeft,float& iRight);
	GuiPropertyBool* Property(const char* iLeft,bool& iRight);
	GuiPropertyAnimationController* Property(AnimationController&);
	GuiPropertySlider* Property(const char* iLeft,float& ref,float& imin,float& imax);

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

struct GuiRootRect : GuiRect , TPoolVector<GuiRootRect>
{
	TabContainer* tabContainer;

	void OnSize(TabContainer*);

	GuiRootRect(TabContainer* t):tabContainer(t){this->name="RootRect";this->Set(0,0,0,-1,0,0,0,0,0,0,1,1);}
};

struct GuiString : GuiRect
{
	vec2 alignText;
	std::string text;
	std::wstring wText;
	bool clipText;

	GuiString():alignText(-1,-1),clipText(true){this->name="String";}

	virtual void OnPaint(TabContainer*,void* data=0);
};

struct GuiButton : GuiString
{
	GuiButton();

	virtual void OnLMouseUp(TabContainer* tab,void* data=0);
	virtual void OnPaint(TabContainer* tab,void* data=0);
};

struct GuiButtonFunc : GuiButton
{
	GuiButtonFunc(void (*iFunc)(void*)=0,void* iParam=0);

	void (*func)(void*);
	void* param;

	virtual void OnLMouseUp(TabContainer* tab,void* data=0);
};

struct GuiButtonBool : GuiButton
{
	bool& referenceValue;
	int		updateMode;

	GuiButtonBool(bool& iBool):referenceValue(iBool),updateMode(-1){this->name="Button";}

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

	void SetRect(GuiRect*);
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

struct GuiSlider : GuiRect
{
	float& referenceValue;

	float& minimum;
	float& maximum;

	GuiSlider(float& iRef,float& iMin,float& iMax):referenceValue(iRef),minimum(iMin),maximum(iMax){this->name="GuiSlider";}

	virtual void OnPaint(TabContainer*,void* data=0);
	virtual void OnMouseMove(TabContainer*,void* data=0);
	virtual void OnSize(TabContainer*,void* data=0);
	void DrawSliderTip(TabContainer*,void* data=0);
};

struct GuiProperty : GuiRect
{
	String description;
};

struct GuiPropertyString : GuiProperty
{
	String& val;

	GuiPropertyString(String& iVal):val(iVal){this->name="PropertyString";this->Set(0,0,0,-1,0,0,0,20,0,0,1,-1);}

	virtual void OnPaint(TabContainer*,void* data=0);
};

struct GuiPropertyFloat : GuiProperty
{
	float& val;

	GuiPropertyFloat(float& iVal):val(iVal){this->name="GuiPropertyFloat";this->Set(0,0,0,-1,0,0,0,20,0,0,1,-1);}

	virtual void OnPaint(TabContainer*,void* data=0);
};

struct GuiPropertyVec3 : GuiProperty
{
	vec3& val;

	GuiPropertyVec3(vec3& iVal):val(iVal){this->name="GuiPropertyVec3";this->Set(0,0,0,-1,0,0,0,20,0,0,1,-1);}

	virtual void OnPaint(TabContainer*,void* data=0);
};

struct GuiPropertyPtr : GuiProperty
{
	void* val;

	GuiPropertyPtr(void* iVal):val(iVal){this->name="GuiPropertyPtr";this->Set(0,0,0,-1,0,0,0,20,0,0,1,-1);}

	virtual void OnPaint(TabContainer*,void* data=0);
};

struct GuiPropertyBool : GuiProperty
{
	bool& val;

	GuiPropertyBool(bool& iVal):val(iVal){this->name="GuiPropertyBool";}

	virtual void OnPaint(TabContainer*,void* data=0);
};

struct GuiPropertySlider : GuiProperty
{
	GuiSlider slider;

	GuiPropertySlider(float& iRefVal,float& iMin,float& iMax):slider(iRefVal,iMin,iMax){this->name="PropertySlider";this->Set(0,0,0,-1,0,0,0,25,0,0,1,-1);this->slider.Set(this,0,0,-1,0,0,0,0,0.5f,0,0.5f,1);}

	virtual void OnPaint(TabContainer*,void* data=0);
};

struct GuiAnimationController : GuiRect
{
	AnimationController& animationController;

	GuiAnimationController(AnimationController&);

	GuiButtonBool play;
	GuiButtonBool stop;

	GuiSlider slider;

	virtual void OnMouseMove(TabContainer*,void* data=0);

	void OnButtonPressed(TabContainer* tabContainer,GuiButtonBool*);
};

struct GuiPropertyAnimationController : GuiProperty
{
	GuiAnimationController guiAnimationController;

	GuiPropertyAnimationController(AnimationController&);
};

struct GuiViewport : GuiRect , TPoolVector<GuiViewport>
{
	EditorEntity* rootEntity;

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

	virtual void OnPaint(TabContainer*,void* data=0);
	virtual void OnSize(TabContainer*,void* data=0);
	virtual void OnMouseWheel(TabContainer*,void* data=0);
	virtual void OnMouseMove(TabContainer*,void* data=0);
	virtual void OnActivate(TabContainer*,void* data=0);
	virtual void OnDeactivate(TabContainer*,void* data=0);
	virtual void OnReparent(TabContainer*,void* data=0);
	virtual void OnLMouseUp(TabContainer*,void* data=0);
};

struct GuiSceneViewer : GuiScrollRect
{
	GuiSceneViewer();
	~GuiSceneViewer();

	EditorEntity* entityRoot;
	
	std::vector<EditorEntity*> selection;

	void OnPaint(TabContainer*,void* data=0);
	void OnLMouseDown(TabContainer*,void* data=0);
	void OnEntitiesChange(TabContainer*,void* data=0);
	void OnEntitySelected(TabContainer*,void* data=0);
	void OnRecreateTarget(TabContainer*,void* data=0);
	void OnRMouseUp(TabContainer*,void* data=0);
	void OnMouseWheel(TabContainer*,void* data=0);

	EditorEntity* GetHoveredRow(EditorEntity* node,vec2& mpos,vec2& pos,bool& oExpandos);
	void DrawNodes(TabContainer*,EditorEntity*,vec2&);
	int UpdateNodes(EditorEntity*);
	void UnselectNodes(EditorEntity*);
	void ExpandUntil(EditorEntity* iTarget);
};	



struct GuiEntityViewer : GuiScrollRect , TPoolVector<GuiEntityViewer>
{
	GuiEntityViewer();
	~GuiEntityViewer();

	EditorEntity* entity;

	TabContainer* tabContainer;

	void OnActivate(TabContainer*,void* data=0);

	virtual void OnEntitySelected(TabContainer*,void* data=0);
	virtual void OnPaint(TabContainer*,void* data=0);
	virtual void OnExpandos(TabContainer*,void* data=0);
	virtual void OnMouseWheel(TabContainer*,void* data=0);

	bool ProcessMouseInput(vec2&,vec2&,GuiRect* node);
	void DrawNodes(TabContainer*,GuiRect*,vec2&);
	int CalcNodesHeight(GuiRect*);
};


struct ResourceNode
{
	ResourceNode* parent;

	String fileName;

	bool selectedLeft;
	bool selectedRight;
	int level;
	bool isDir;

	ResourceNode();
	virtual ~ResourceNode();
};

struct ResourceNodeDir : ResourceNode
{
	bool expanded;

	std::list<ResourceNodeDir*> dirs;
	std::list<ResourceNode*>	files;

	ResourceNodeDir();
	~ResourceNodeDir();
};

struct GuiProjectViewer : GuiRect
{
	struct GuiProjectDirViewer : GuiScrollRect
	{
		ResourceNodeDir* rootResource;

		void DrawNodes(TabContainer*,ResourceNodeDir* node,vec2&,bool& terminated);
		ResourceNodeDir* GetHoveredRow(ResourceNodeDir* node,vec2& mpos,vec2& pos,bool& oExpandos);
		int CalcNodesHeight(ResourceNodeDir*);
		void UnselectNodes(ResourceNodeDir*);
		std::vector<ResourceNodeDir*> selectedDirs;

		void OnLMouseDown(TabContainer*,void* data=0);
		void OnPaint(TabContainer*,void* data=0);

	}left;

	struct GuiProjectFileViewer : GuiScrollRect
	{
		ResourceNodeDir* rootResource;
		std::vector<ResourceNodeDir*> selectedDirs;
		std::vector<ResourceNode*> selectedFiles;

		void DrawNodes(TabContainer*,ResourceNodeDir* node,vec2&);
		ResourceNode* GetHoveredRow(ResourceNodeDir* node,vec2& mpos,vec2& pos,bool& oExpandos);
		int CalcNodesHeight(ResourceNodeDir*);
		void UnselectNodes(ResourceNodeDir*);
		
		void OnLMouseDown(TabContainer*,void* data=0);
		void OnPaint(TabContainer*,void* data=0);
		void OnRMouseUp(TabContainer*,void* data=0);
	}right;

	struct GuiProjectDataViewer : GuiScrollRect
	{
		
	}viewer;


	ResourceNodeDir rootResource;

	bool splitterLeft;
	bool splitterRight;
	float hotspotDist;

	GuiProjectViewer();
	~GuiProjectViewer();


	void OnPaint(TabContainer*,void* data=0);
	void OnLMouseDown(TabContainer*,void* data=0);
	void OnLMouseUp(TabContainer*,void* data=0);
	void OnMouseMove(TabContainer*,void* data=0);
	void OnReparent(TabContainer*,void* data=0);
	void OnActivate(TabContainer*,void* data=0);
	void OnSize(TabContainer*,void* data=0);

	void DestroyNode(ResourceNode*);
};

struct WindowData
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

struct GuiImage
{
	int width,height;
	int bpp;

	GuiImage();
	~GuiImage();

	virtual void Release()=0;
	virtual bool Fill(Renderer2DInterface*,unsigned char* iData,float iWidth,float iHeight)=0;
};

struct GuiScriptViewer : GuiScrollRect , TPoolVector<GuiScriptViewer>
{
	Script* script;

	int			cursor;
	GuiString*	paper;

	GuiScriptViewer();

	void Open(Script*);
	bool Save();
	bool Compile();

	void OnKeyDown(TabContainer*,void* data=0);
	void OnKeyUp(TabContainer*,void* data=0);
	void OnLMouseDown(TabContainer*,void* data=0);
	void OnSize(TabContainer*,void* data=0);
};

struct GuiCompilerViewer : GuiScrollRect , TPoolVector<GuiCompilerViewer>
{
	GuiCompilerViewer();

	bool ParseCompilerOutputFile(TabContainer*,wchar_t*);
	void OnSize(TabContainer*,void* data=0);
};

struct DrawInstance
{
	int		 code;
	bool	 frame;
	GuiRect* rect;

	bool remove;

	DrawInstance(int a,bool b,GuiRect* c,bool iRemove):code(a),frame(b),rect(c),remove(true){}
};

struct TabContainer : TPoolVector<TabContainer>
{
	WindowData* windowData;

	EditorWindowContainer* editorWindowContainer;

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

	SplitterContainer* splitterContainer;

	Renderer2DInterface *renderer2D;
	Renderer3DInterface *renderer3D;

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

	Task*	 drawTask;

	float mousex,mousey;

	unsigned int lastFrameTime;

	ThreadInterface* thread;

	TabContainer(float x,float y,float w,float h);
	virtual ~TabContainer();

	operator TabContainer& (){return *this;}

	virtual void OnGuiPaint(void* data=0);
	virtual void OnGuiSize(void* data=0);
	virtual void OnWindowPosChanging(void* data=0);
	virtual void OnGuiLMouseDown(void* data=0);
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

	virtual bool BeginDraw()=0;
	virtual void EndDraw()=0;

	DrawInstance* SetDraw(int iCode=1,bool iFrame=true,GuiRect* iRect=0,bool iRemove=true);

	virtual int TrackGuiSceneViewerPopup(bool iSelected)=0;
	virtual int TrackTabMenuPopup()=0;
	virtual int TrackProjectFileViewerPopup(bool iSelected)=0;

	void SetSelection(GuiRect* iRect);

	virtual void SetCursor(int)=0;
};

struct SplitterContainer 
{
	TabContainer* currentTabContainer;

	TabContainer* floatingTabRef;
	TabContainer* floatingTab;
	TabContainer* floatingTabTarget;
	int  floatingTabRefTabIdx;
	int  floatingTabRefTabCount;
	int  floatingTabTargetAnchorPos;
	int	 floatingTabTargetAnchorTabIndex;

	const int   splitterSize;
	char*		splitterCursor;

	SplitterContainer();
	~SplitterContainer();

	virtual void CreateFloatingTab(TabContainer*)=0;
	virtual void DestroyFloatingTab()=0;
};


struct EditorWindowContainer
{
	std::vector<TabContainer*> tabContainers;

	WindowData* window;
	SplitterContainer* splitter;

	int resizeDiffHeight;
	int resizeDiffWidth;
	int resizeEnumType;
	int resizeCheckWidth;
	int resizeCheckHeight;

	EditorWindowContainer();

	virtual void OnSizing()=0;
	virtual void OnSize()=0;

	virtual TabContainer* CreateTabContainer(float x,float y,float w,float h)=0;
};


struct EditorMainAppWindow
{
	std::vector<EditorWindowContainer*> containers;

	virtual EditorWindowContainer* CreateContainer()=0;
};


struct CompilerInterface
{
	String compilerPath;
	String linkerPath;
	String includePath;
	String libPath;
	String runBefore;
	String runAfter;

	virtual bool Compile(Script*)=0;
	virtual bool Execute(String iPath,String iCmdLine)=0;
	virtual bool Load(Script*)=0;
	virtual bool Unload(Script*)=0;
};

struct EditorProperties
{
	GuiString properties;

	EditorProperties();

	virtual void OnPropertiesCreate(){};
	virtual void OnResourcesCreate(){};
	virtual void OnPropertiesUpdate(TabContainer*){};
};


template<class T> struct EditorObject : T , EditorProperties
{
};

struct EditorEntity : EditorObject<Entity> 
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

	void OnPropertiesUpdate(TabContainer*);
};
				

struct EditorAnimationController : EditorObject<AnimationController>
{
	float oldCursor;
	GuiPropertyAnimationController*  guiPropertyAnimationController;

	void OnPropertiesCreate();
	void OnPropertiesUpdate(TabContainer*);

};

struct EditorMesh : EditorObject<Mesh>
{
	void OnPropertiesCreate(); 
	void OnPropertiesUpdate(TabContainer*);
};
struct EditorRoot : EditorObject<Root>
{
	void OnPropertiesCreate(); 
	void OnPropertiesUpdate(TabContainer*);
};
struct EditorSkeleton : EditorObject<Skeleton>
{
	void OnPropertiesCreate(); 
	void OnPropertiesUpdate(TabContainer*);
};
struct EditorGizmo : EditorObject<Gizmo>
{
	void OnPropertiesCreate(); 
	void OnPropertiesUpdate(TabContainer*);
};
struct EditorAnimation : EditorObject<Animation>
{
	void OnPropertiesCreate(); 
	void OnPropertiesUpdate(TabContainer*);
};
struct EditorBone : EditorObject<Bone>
{
	void OnPropertiesCreate(); 
	void OnPropertiesUpdate(TabContainer*);
};
struct EditorLight : EditorObject<Light>
{
	void OnPropertiesCreate(); 
	void OnPropertiesUpdate(TabContainer*);
};
struct EditorScript : EditorObject<Script>
{
	void OnPropertiesCreate(); 
	void OnPropertiesUpdate(TabContainer*);
	void OnResourcesCreate();
};
struct EditorCamera : EditorObject<Camera>
{
	void OnPropertiesCreate(); 
	void OnPropertiesUpdate(TabContainer*);
};
struct EditorSkin : EditorObject<Skin>
{
	void OnPropertiesCreate(); 
	void OnPropertiesUpdate(TabContainer*);
};





#endif //INTERFACES_H



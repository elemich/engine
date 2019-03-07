#include "interfaces.h"

#include "imgpng.h"
#include "imgjpg.h"
#include "imgtga.h"

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////////globals///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////////globals///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

#define GLOBALGETTERFUNCASSINGLETON(funcName,dataType) \
	if(!funcName()) \
	funcName()=new dataType; \
	return funcName(); \

GLOBALGETTERFUNC(GlobalGuiProjectViewerInstance,GuiProjectViewer*);
GLOBALGETTERFUNC(GlobalGuiSceneViewerInstance,GuiSceneViewer*);
GLOBALGETTERFUNC(GlobalGuiCompilerViewerInstance,GuiCompilerViewer*);
GLOBALGETTERFUNC(GlobalGuiConsoleViewerInstance,GuiConsoleViewer*);
GLOBALGETTERFUNC(GlobalDefaultFontInstance,GuiFont*);
GLOBALGETTERFUNC(GlobalFontPoolInstance,std::vector<GuiFont*>);
GLOBALGETTERFUNC(GlobalIdeInstance,Ide*);
GLOBALGETTERFUNC(GlobalScriptViewers,std::list<GuiScriptViewer*>);
GLOBALGETTERFUNC(GlobalViewports,std::list<GuiViewport*>);
GLOBALGETTERFUNC(GlobalRootProjectDirectory,ResourceNodeDir*);
GLOBALGETTERFUNC(GlobalGuiEntityViewerInstance,std::list<GuiEntityViewer*>);




///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////SerializerHelpers///////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

namespace SerializerHelpers
{
	extern std::list<Skin*>							globalSkinsToBind;
	extern std::list< std::list<unsigned int> >		globalSkinsClusterBoneIdToBind;

	extern std::list<AnimationController*>			globalAnimationControllersToBind;
	extern std::list< std::list<unsigned int> >		globalAnimationControllersAnimationsIndicesToBind;

	extern void SetEntityId(Entity* iEntity,unsigned int& iId);
	extern Entity* GetRootEntity(Entity* iEntity);
	extern Entity* GetEntityById(Entity* iEntity,unsigned int iId);

	void			saveSceneEntityRecursively(Entity* iEntity,FILE* iFile);
	EditorEntity*	loadSceneEntityRecursively(EditorEntity* iEntity,FILE* iFile);

	void Save(Mesh*,FILE*);
	void Save(Skin*,FILE*);
	void Save(Animation*,FILE*);
	void Save(AnimationController*,FILE*);
	void Save(Skin*,FILE*);
	void Save(Line*,FILE*);
	void Save(Script*,FILE*);

	extern void Load(Mesh*,FILE*);
	extern void Load(Skin*,FILE*);
	extern void Load(Animation*,FILE*);
	extern void Load(AnimationController*,FILE*);
	extern void Load(Skin*,FILE*);
	extern void Load(Line*,FILE*);
	extern void Load(Script*,FILE*);

	void Load(EditorLine*,FILE*);

	extern void BindSkinLinks(Entity* iEntityParent);
	extern void BindAnimationLinks(Entity* iEntityParent);

	void WriteComponentCode(int,FILE*);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////ResourceNode/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////



ResourceNode::ResourceNode():
	parent(0),
	isDir(0)
{
}

ResourceNode::~ResourceNode()
{
	this->parent=0;
	this->fileName.clear();
	this->isDir=false;
}

ResourceNodeDir::ResourceNodeDir(){}

ResourceNodeDir::~ResourceNodeDir()
{
	for(std::list<ResourceNode*>::iterator tFile=this->files.begin();tFile!=this->files.end();tFile++)
		SAFEDELETE(*tFile);

	for(std::list<ResourceNodeDir*>::iterator tDir=this->dirs.begin();tDir!=this->dirs.end();tDir++)
		SAFEDELETE(*tDir);

	this->files.clear();
	this->dirs.clear();
}

String ResourceNode::BuildPath()
{
	if(!this->parent)
		return this->fileName;

	String tReturnPath=L"\\" + this->fileName;

	ResourceNode* t=this;

	while(t->parent)
	{
		tReturnPath.insert(0, !t->parent->parent ? t->parent->fileName : L"\\" + t->parent->fileName);
		t=t->parent;
	}
	 
	return tReturnPath;
}



ResourceNodeDir* ResourceNodeFindDirNode(const String& iFile,ResourceNodeDir* iNode,String &tmp)
{
	tmp+= iNode->parent ? L"\\"+iNode->fileName : iNode->fileName;

	if(tmp==iFile)
		return iNode;

	for(std::list<ResourceNodeDir*>::iterator iterDir=iNode->dirs.begin();iterDir!=iNode->dirs.end();iterDir++)
	{
		ResourceNodeDir* tResult=(*iterDir)->FindDirNode(iFile);

		if(tResult)
			return tResult;
	}

	return 0;
}

ResourceNodeDir* ResourceNodeDir::FindDirNode(String iFile)
{
	ResourceNodeDir* tRootNode=ResourceNodeDir::GetRootDirNode();

	if(tRootNode && tRootNode->fileName.size())
	{
		String tmp;
		return ResourceNodeFindDirNode(iFile,tRootNode,tmp);
	}

	return 0;
}

ResourceNode* ResourceNodeFindFileNode(const String& iFile,ResourceNodeDir* iNode,String &tmp)
{
	tmp+= iNode->parent ? L"\\"+iNode->fileName : iNode->fileName;

	if(tmp==iFile)
		return iNode;
	
	for(std::list<ResourceNode*>::iterator iterFile=iNode->files.begin();iterFile!=iNode->files.end();iterFile++)
	{
		if((tmp+L"\\"+(*iterFile)->fileName)==iFile)
			return *iterFile;
	}

	for(std::list<ResourceNodeDir*>::iterator iterDir=iNode->dirs.begin();iterDir!=iNode->dirs.end();iterDir++)
	{
		ResourceNode* tResult=ResourceNodeFindFileNode(iFile,*iterDir,tmp);

		if(tResult)
			return tResult;
	}

	return 0;
}

ResourceNode* ResourceNodeDir::FindFileNode(String iFile)
{
	ResourceNodeDir* tRootNode=ResourceNodeDir::GetRootDirNode();

	if(tRootNode && tRootNode->fileName.size())
	{
		String tmp;
		return ResourceNodeFindFileNode(iFile,tRootNode,tmp);
	}

	return 0;
}

ResourceNodeDir* ResourceNodeDir::GetRootDirNode()
{
	return GlobalRootProjectDirectory();
}

String gFindResource(String& iCurrentDirectory,String& iProjectDir,ResourceNodeDir* iResDir,String& iResourceName)
{
	//store current dir

	if(iResDir->parent)
	{
		iCurrentDirectory+=iResDir->fileName.c_str();
		iCurrentDirectory+=L"\\";
	}

	//if node contains files, process them, later process other dir nodes

	for(std::list<ResourceNode*>::iterator tResFile=iResDir->files.begin();tResFile!=iResDir->files.end();tResFile++)
	{
		String	tVirtualFileName=iCurrentDirectory + (*tResFile)->fileName;

		if(tVirtualFileName==iResourceName)
		{
			return iProjectDir + iCurrentDirectory + (*tResFile)->fileName;
		}
	}

	for(std::list<ResourceNodeDir*>::iterator tResNodeDir=iResDir->dirs.begin();tResNodeDir!=iResDir->dirs.end();tResNodeDir++)
	{
		String t=gFindResource(iCurrentDirectory,iProjectDir,*tResNodeDir,iResourceName);

		return t;
	}

	return String();
}

void* Resource::Load(FilePath iResourceName)
{
	String tRootTrailingSlashes(L"\\");

	FilePath tResourcePath=gFindResource(tRootTrailingSlashes,GlobalRootProjectDirectory()->fileName,GlobalRootProjectDirectory(),iResourceName);

	if(!tResourcePath.File().empty())
	{
		String tFileExtension=tResourcePath.Extension();

		if(tFileExtension==L"engineScene")
		{

		}
	}

	return 0;
}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////AppInterface////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////




Ide::Ide():
	timer(0),
	mainAppWindow(0),
	compiler(0),
	processId(0),
	processThreadId(0),
	stringEditor(0),
	popup(0)
{
	GlobalIdeInstance()=this;
}
Ide::~Ide(){}

Ide* Ide::Instance()
{
	return GlobalIdeInstance();
}

String Ide::GetSceneExtension()
{
	return L".engineScene";
}
String Ide::GetEntityExtension()
{
	return L".engineEntity";
}

Tab*	Ide::CreatePopup(TabContainer* iParent,float ix,float iy,float iw,float ih)
{
	this->DestroyPopup();

	wprintf(L"creating popup\n");

	this->popup=iParent->CreateModalTab(ix,iy,iw,ih);
	this->popup->hasFrame=false;

	return this->popup;
}
void	Ide::DestroyPopup()
{
	if(this->popup)
	{
		wprintf(L"destroying popup %p\n",this->popup);

		this->popup->rects.OnDeactivate(this->popup,Msg());

		this->popup->rects.Destroy();

		this->popup->Destroy();
		this->popup=0;
	}
}

Tab*	Ide::GetPopup()
{
	return this->popup;
}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////EditorWindowContainer///////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

TabContainer::TabContainer()
{
	resizeDiffHeight=0;
	resizeDiffWidth=0;
	resizeEnumType=-1;
	resizeCheckWidth=0;
	resizeCheckHeight=0;
}

TabContainer::~TabContainer(){}

void TabContainer::Broadcast(GuiRect::Funcs iFunc,const Msg& iMsg)
{
	for(std::vector<Tab*>::iterator tTab=this->tabs.begin();tTab!=this->tabs.end();tTab++)
		(*tTab)->Broadcast(iFunc,iMsg);
}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////Debugger//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
Debugger::Debugger():breaked(false),threadSuspendend(false),runningScript(0),runningScriptFunction(0),debuggerCode(0),lastBreakedAddress(0),sleep(1){}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////TouchInput//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

TouchInput::TouchInput()
{
	for(int i=0;i<10;i++)
	{
		pressed[i]=0;
		released[i]=0;
		position[i].make(0,0);
	}
}

bool TouchInput::IsPressed(int i){return pressed[i];}
bool TouchInput::IsReleased(int i){return released[i];}

void TouchInput::SetPressed(bool b,int i){pressed[i]=b;}
void TouchInput::SetReleased(bool b,int i){released[i]=b;}


vec2& TouchInput::GetPosition(int i){return position[i];}
void   TouchInput::SetPosition(vec2& pos,int i){position[i]=pos;}

MouseInput::MouseInput()
{}

bool MouseInput::Left(){return this->left;}
bool MouseInput::Right(){return this->right;}
bool MouseInput::Middle(){return this->middle;}

TouchInput InputManager::touchInput;
MouseInput InputManager::mouseInput;
KeyboardInput InputManager::keyboardInput;

/*
InputInterface InputManager::voiceInput;
InputInterface InputManager::joystickInput;*/


///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////TabContainer/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

void (GuiRect::*Tab::rectFuncs[GuiRect::MAXFUNCS])(GUIMSGDEF)=
{
	0,
	&GuiRect::OnRecreateTarget,
	&GuiRect::OnPaint,
	&GuiRect::OnEntitiesChange,
	&GuiRect::OnSize,
	&GuiRect::OnMouseDown,
	&GuiRect::OnMouseClick,
	&GuiRect::OnMouseUp,
	&GuiRect::OnMouseMove,
	&GuiRect::OnUpdate,
	&GuiRect::OnReparent,
	&GuiRect::OnSelected,
	&GuiRect::OnRender,
	&GuiRect::OnMouseWheel,
	&GuiRect::OnActivate,
	&GuiRect::OnDeactivate,
	&GuiRect::OnEntitySelected,
	&GuiRect::OnExpandos,
	&GuiRect::OnKeyDown,
	&GuiRect::OnKeyUp,
	&GuiRect::OnMouseEnter,
	&GuiRect::OnMouseExit,
	&GuiRect::OnEnterFocus,
	&GuiRect::OnExitFocus
};

unsigned char Tab::rawUpArrow[]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0xa7,0x0,0x0,0x0,0xa8,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x87,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x88,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6a,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x6b,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x4b,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x4c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x2b,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x2c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xb,0x0,0x0,0x0,0xfa,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfb,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x7,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0x7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
unsigned char Tab::rawRightArrow[]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0x7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0xb,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfa,0x0,0x0,0x0,0x2b,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x4b,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x6a,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x87,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xa7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xa8,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x88,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x6b,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x4c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfb,0x0,0x0,0x0,0x2c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0x7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
unsigned char Tab::rawLeftArrow[]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x07,0x0,0x0,0x0,0x0c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0c,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x2c,0x0,0x0,0x0,0xfb,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x4c,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6b,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x01,0x0,0x0,0x0,0x88,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xa8,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xa7,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x01,0x0,0x0,0x0,0x87,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6a,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x4b,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x2b,0x0,0x0,0x0,0xfa,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0b,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x07,0x0,0x0,0x0,0x0c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
unsigned char Tab::rawDownArrow[]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x7,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0x7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0xfb,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfa,0x0,0x0,0x0,0xb,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x2c,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x2b,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x4c,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x4b,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6b,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x6a,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x88,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x87,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0xa8,0x0,0x0,0x0,0xa7,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
unsigned char Tab::rawFolder[]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe6,0xc1,0x9b,0x12,0xe9,0xc4,0x9f,0xd8,0xe9,0xc4,0xa0,0xff,0xe9,0xc4,0xa0,0xff,0xe9,0xc4,0xa0,0xff,0xe9,0xc4,0xa0,0xff,0xe9,0xc4,0xa0,0xff,0xe6,0xc0,0x9a,0x5b,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe5,0xc1,0x9e,0x20,0xe5,0xc2,0x9e,0xef,0xe5,0xc2,0x9e,0xff,0xe5,0xc2,0x9e,0xff,0xe5,0xc2,0x9d,0xff,0xe5,0xc2,0x9d,0xff,0xe5,0xc2,0x9d,0xff,0xe4,0xc2,0x9c,0xa6,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe3,0xbe,0x98,0x21,0xdf,0xb9,0x94,0xee,0xdf,0xb9,0x93,0xff,0xdf,0xb9,0x93,0xff,0xde,0xb9,0x92,0xff,0xde,0xb9,0x93,0xff,0xde,0xb9,0x93,0xff,0xdd,0xb7,0x90,0xe4,0xda,0xb2,0x89,0xa2,0xda,0xb2,0x8a,0xa3,0xda,0xb2,0x8a,0xa3,0xda,0xb2,0x8a,0xa3,0xda,0xb2,0x8a,0x86,0xe6,0xc1,0x97,0xf,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xfb,0xda,0xaf,0xd2,0xf7,0xd6,0xab,0xfd,0xf7,0xd6,0xab,0xff,0xf7,0xd6,0xab,0xff,0xf7,0xd6,0xab,0xff,0xf6,0xd6,0xab,0xff,0xf6,0xd6,0xab,0xff,0xf6,0xd6,0xab,0xff,0xf6,0xd5,0xab,0xff,0xf6,0xd5,0xab,0xff,0xf6,0xd5,0xab,0xff,0xf6,0xd5,0xab,0xff,0xf6,0xd5,0xab,0xff,0xfa,0xda,0xaf,0xd7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc9,0x96,0x64,0xe7,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xc9,0x97,0x64,0xf1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
unsigned char Tab::rawFile[]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc8,0xc8,0xc8,0x4c,0xe0,0xe0,0xe0,0xcd,0xe1,0xe1,0xe1,0xc0,0xe1,0xe1,0xe1,0xbf,0xe1,0xe1,0xe1,0xbf,0xe1,0xe1,0xe1,0xbf,0xe1,0xe1,0xe1,0xc0,0xd6,0xd6,0xd6,0xc4,0xbd,0xbd,0xbd,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc5,0xc5,0xc5,0x70,0xec,0xec,0xec,0xff,0xed,0xed,0xed,0xff,0xed,0xed,0xed,0xff,0xed,0xed,0xed,0xff,0xed,0xed,0xed,0xff,0xee,0xee,0xee,0xff,0xeb,0xeb,0xeb,0xff,0xce,0xce,0xce,0xff,0xbd,0xbd,0xbd,0xa6,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xbe,0xbe,0xbe,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc5,0xc5,0xc5,0x6c,0xec,0xec,0xec,0xff,0xed,0xed,0xed,0xff,0xed,0xed,0xed,0xff,0xed,0xed,0xed,0xff,0xed,0xed,0xed,0xff,0xed,0xed,0xed,0xff,0xef,0xef,0xef,0xff,0xd3,0xd3,0xd3,0xfd,0xe1,0xe1,0xe1,0xff,0xde,0xde,0xde,0xae,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc8,0xc8,0xc8,0x6c,0xee,0xee,0xee,0xff,0xef,0xef,0xef,0xff,0xef,0xef,0xef,0xff,0xef,0xef,0xef,0xff,0xef,0xef,0xef,0xff,0xef,0xef,0xef,0xff,0xf0,0xf0,0xf0,0xff,0xdb,0xdb,0xdb,0xff,0xc0,0xc0,0xc0,0xfe,0xc9,0xc9,0xc9,0xff,0xb9,0xb9,0xb9,0xbd,0x0,0x0,0x0,0x0,0xff,0xff,0xff,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc9,0xc9,0xc9,0x6b,0xf0,0xf0,0xf0,0xff,0xf1,0xf1,0xf1,0xff,0xf1,0xf1,0xf1,0xff,0xf1,0xf1,0xf1,0xff,0xf1,0xf1,0xf1,0xff,0xf1,0xf1,0xf1,0xff,0xf1,0xf1,0xf1,0xff,0xe7,0xe7,0xe7,0xff,0xc3,0xc3,0xc3,0xff,0xb6,0xb6,0xb6,0xfc,0xc2,0xc2,0xc2,0xff,0xbd,0xbd,0xbd,0x95,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xcb,0xcb,0xcb,0x6b,0xf2,0xf2,0xf2,0xff,0xf3,0xf3,0xf3,0xff,0xf3,0xf3,0xf3,0xff,0xf3,0xf3,0xf3,0xff,0xf3,0xf3,0xf3,0xff,0xf3,0xf3,0xf3,0xff,0xf3,0xf3,0xf3,0xff,0xf2,0xf2,0xf2,0xff,0xf2,0xf2,0xf2,0xff,0xf4,0xf4,0xf4,0xff,0xf2,0xf2,0xf2,0xff,0xd5,0xd5,0xd5,0xc0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xcd,0xcd,0xcd,0x6b,0xf4,0xf4,0xf4,0xff,0xf5,0xf5,0xf5,0xff,0xf5,0xf5,0xf5,0xff,0xf5,0xf5,0xf5,0xff,0xf5,0xf5,0xf5,0xff,0xf5,0xf5,0xf5,0xff,0xf5,0xf5,0xf5,0xff,0xf5,0xf5,0xf5,0xff,0xf6,0xf6,0xf6,0xff,0xf6,0xf6,0xf6,0xff,0xf8,0xf8,0xf8,0xff,0xe4,0xe4,0xe4,0xbc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xcf,0xcf,0xcf,0x6b,0xf6,0xf6,0xf6,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf9,0xf9,0xf9,0xff,0xe5,0xe5,0xe5,0xbc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd0,0xd0,0xd0,0x6b,0xf8,0xf8,0xf8,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xfb,0xfb,0xfb,0xff,0xe8,0xe8,0xe8,0xbc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd2,0xd2,0xd2,0x6b,0xf9,0xf9,0xf9,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfc,0xfc,0xfc,0xff,0xe9,0xe9,0xe9,0xbb,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd3,0xd3,0xd3,0x6b,0xfb,0xfb,0xfb,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfe,0xfe,0xfe,0xff,0xeb,0xeb,0xeb,0xbb,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd4,0xd4,0xd4,0x6b,0xfc,0xfc,0xfc,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xff,0xff,0xff,0xff,0xec,0xec,0xec,0xbb,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd4,0xd4,0xd4,0x6b,0xfd,0xfd,0xfd,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xff,0xff,0xff,0xff,0xec,0xec,0xec,0xbb,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd5,0xd5,0xd5,0x6a,0xfd,0xfd,0xfd,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xff,0xff,0xff,0xff,0xed,0xed,0xed,0xbb,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xdb,0xdb,0xdb,0x70,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf1,0xf1,0xf1,0xc5,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xa4,0xa4,0xa4,0x41,0xbc,0xbc,0xbc,0xab,0xbb,0xbb,0xbb,0xa2,0xbb,0xbb,0xbb,0xa1,0xbb,0xbb,0xbb,0xa1,0xbb,0xbb,0xbb,0xa1,0xbb,0xbb,0xbb,0xa1,0xbb,0xbb,0xbb,0xa1,0xbb,0xbb,0xbb,0xa1,0xbb,0xbb,0xbb,0xa2,0xbb,0xbb,0xbb,0xa2,0xbd,0xbd,0xbd,0xaa,0xb4,0xb4,0xb4,0x73,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x4,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x4,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};





Tab::Tab(float x,float y,float w,float h):
	rects(this),
	windowData(0),
	container(0),
	selected(0),
	mouseDown(false),
	isRender(false),
	splitterContainer(0),
	renderer3D(0),
	renderer2D(0),
	recreateTarget(false),
	resizeTarget(true),
	resizing(false),
	lastFrameTime(0),
	iconUp(0),
	iconLeft(0),
	iconRight(0),
	iconDown(0),
	iconFolder(0),
	iconFile(0),
	thread(0),
	focused(0),
	pressed(0),
	hovered(0),
	isModal(false),
	hasFrame(true)
{}

Tab::~Tab()
{
	this->container->tabs.erase(std::find(container->tabs.begin(),container->tabs.end(),this));
}

GuiRect* Tab::GetSelected()
{
	return selected;
}


void Tab::DrawBlock(bool iBool)
{
	Thread* tProjectDirChangedThread=Ide::Instance()->projectDirChangedThread;

	if(iBool)
	{
		this->drawTask->Block(iBool);
		tProjectDirChangedThread->Block(true);
	}

	for(std::list<DrawInstance*>::iterator iterDrawInstance=this->drawInstances.begin();iterDrawInstance!=this->drawInstances.end();iterDrawInstance++)
	{
		if((*iterDrawInstance) && (*iterDrawInstance)->remove==true)
		{
			SAFEDELETE(*iterDrawInstance);
		}
	}

	this->SetHover(0);
	this->SetPressed(0);
	this->SetFocus(0);

	if(!iBool)
	{
		tProjectDirChangedThread->Block(false);
		this->drawTask->Block(iBool);
	}
}

struct DLLBUILD MsgPaintData
{
	vec4 clip;
	vec2 coord;
};

void Tab::Draw()
{
	if(this->drawTask->pause)
		DEBUG_BREAK();

	if(!this->drawInstances.empty())
	{
		Msg tDrawEvent(this);

		for(std::list<DrawInstance*>::iterator it=this->drawInstances.begin();it!=this->drawInstances.end();it)
		{
			DrawInstance*& tDrawInstance=*it;

			if(tDrawInstance && !tDrawInstance->skip)
			{
				if(this->BeginDraw())
				{
					if(!tDrawInstance->rect)
						this->OnGuiPaint();
					else
						this->BroadcastPaintTo(tDrawInstance->rect);

					this->EndDraw();
				}
			}

			if(!tDrawInstance || tDrawInstance->remove)
			{
				SAFEDELETE(tDrawInstance);
				it=this->drawInstances.erase(it);
			}
			else it++;
		}
	}

	Ide::Instance()->stringEditor->Draw(this);
}

DrawInstance* Tab::SetDraw(GuiRect* iRect,bool iRemove,bool iSkip)
{
	DrawInstance* newInstance=new DrawInstance(iRect,iRemove,iSkip);
	this->drawInstances.push_back(newInstance);
	return newInstance;
}

void Tab::Broadcast(GuiRect::Funcs iFunc,const Msg& iMsg)
{
	this->BroadcastTo(&this->rects,iFunc,iMsg);
}

void Tab::BroadcastTo(GuiRect* iRect,GuiRect::Funcs iFunc,const Msg& iMsg)
{
	if(iRect->IsActive())
	{
		switch(iFunc)
		{
			case GuiRect::ONHITTEST:
			{
				vec4 tClip=iRect->Edges();//iRect->EdgeClip();

				if(iRect->Contains(iMsg.mouse))
					iMsg.hit=iRect;

				iRect->Broadcast(iFunc,this,iMsg);

				/*if(iMsg.mouse.x>tClip.x && iMsg.mouse.x<tClip.z && iMsg.mouse.y>tClip.y && iMsg.mouse.y<tClip.w)
				{
					
				}*/
			}
			break;
			case GuiRect::ONPAINT:
			{
				unsigned int	tColor=iRect->BlendColor(iRect->Color());
				vec4			tClip=iRect->Edges();
				ClipData		tClipData;

				this->renderer2D->DrawRectangle(tClip,tColor,true);

				iRect->OnPaint(this,iMsg);

				/*if(iRect->GetFlag(GuiRect::FLAGS,GuiRect::CHILD))
					this->PushScissor(iRect->EdgeClip(),iRect->EdgeOffset());*/

				iRect->Broadcast(iFunc,this,iMsg);

				/*if(iRect->GetFlag(GuiRect::FLAGS,GuiRect::CHILD))
					this->PopScissor();*/

				GuiScrollRect* tScrollRect=dynamic_cast<GuiScrollRect*>(iRect);

				this->renderer2D->DrawRectangle(tClip.x+0.5f,tClip.y+0.5f,tClip.z-0.5f,tClip.w-0.5f,0xff0000,false);
			}
			break;
			case GuiRect::ONSIZE:
			{
				iRect->OnSize(this,iMsg);
				iRect->Broadcast(iFunc,this,iMsg);
			}
			break;
			case GuiRect::ONACTIVATE:
			{
				iRect->OnActivate(this,iMsg);
				iRect->Broadcast(iFunc,this,iMsg);
			}
			break;
			case GuiRect::ONDEACTIVATE:
			{
				iRect->OnDeactivate(this,iMsg);
				iRect->Broadcast(iFunc,this,iMsg);
			}
			break;
			default:
			{
				(iRect->*this->rectFuncs[iFunc])(this,iMsg);
				iRect->Broadcast(iFunc,this,iMsg);
			}
		}
	}
}

void Tab::BroadcastInputTo(GuiRect* iRect,GuiRect::Funcs iFunc,const Msg& iMsg)
{
	switch(iFunc)
	{
		case GuiRect::ONMOUSEENTER:
		case GuiRect::ONMOUSEEXIT:
		case GuiRect::ONMOUSEMOVE:
		case GuiRect::ONMOUSEUP:
		case GuiRect::ONMOUSEDOWN:
		case GuiRect::ONMOUSECLICK:
		{
			GuiRect*			tRect=iRect->GetParent();
			std::list<GuiRect*> tAncestors;

			while(tRect)tAncestors.push_front(tRect),tRect=tRect->GetParent();

			for(std::list<GuiRect*>::const_iterator i=tAncestors.begin();i!=tAncestors.end();i++)
			{
				GuiRect* tRect=*i;

				vec4 tClip=tRect->EdgeClip();

				if(tRect->Contains(iMsg.mouse))
					iMsg.hit=tRect;

				if(iMsg.mouse.x>tClip.x && iMsg.mouse.x<tClip.z && iMsg.mouse.y>tClip.y && iMsg.mouse.y<tClip.w)
				{
					vec2 tOffset=tRect->EdgeOffset();

					iMsg.mouse.x-=tOffset.x;
					iMsg.mouse.y-=tOffset.y;
				}
			}

			(iRect->*this->rectFuncs[iFunc])(this,iMsg);
		}
		break;
	}
}

void Tab::BroadcastPaintTo(GuiRect* iRect)
{
	GuiRect*			tRect=iRect->GetParent();
	std::list<GuiRect*> tAncestors;
	Msg					tMsg;

	tAncestors.push_back(iRect);

	while(tRect)tAncestors.push_front(tRect),tRect=tRect->GetParent();

	for(std::list<GuiRect*>::const_iterator i=tAncestors.begin();i!=tAncestors.end();i++)
	{
		bool tTargetParent=(*i==tAncestors.back());

		if(!tTargetParent && (*i)->GetFlag(GuiRect::FLAGS,GuiRect::CHILD))
			this->PushScissor((*i)->EdgeClip(),(*i)->EdgeOffset());
	}

	this->BroadcastTo(iRect,GuiRect::ONPAINT,tMsg);

	int tClips=this->clips.size();

	for(int i=0;i<tClips;i++)
		this->PopScissor();
};

void Tab::SetSelection(GuiRect* iRect)
{
	Msg iMsg;

	if(this->selected && this->selected!=iRect && this->selected->IsActive())
		this->BroadcastTo(this->selected,GuiRect::ONDEACTIVATE,iMsg);

	this->selected=iRect;

	if(this->selected)
	{
		if(!this->selected->IsActive())
			this->selected->Activate(true);

		this->Broadcast(GuiRect::ONSIZE,iMsg);
		this->BroadcastTo(this->selected,GuiRect::ONACTIVATE,iMsg);
	}

	this->SetDraw();
}

void Tab::OnGuiSize(void* data)
{
	this->Broadcast(GuiRect::ONSIZE,Msg());

	this->resizeTarget=true;
}

void Tab::OnWindowPosChanging(void* data)
{
	this->resizeTarget=true;

	this->Broadcast(GuiRect::ONSIZE,Msg());
}

void Tab::OnGuiMouseMove(void* data)
{
	splitterContainer->currentTabContainer=this;

	/*if(mouseDown && tmx!=mousex && tmy!=mousey)
		splitterContainer->CreateFloatingTab(this);

	mousex=tmx;
	mousey=tmy;*/

	//if(mousey>TabContainer::CONTAINER_HEIGHT)

	Msg	tEvent(this->mouse);

	GuiRect*			tOldHover=this->GetHover();
	Msg 				tNewHoverMsg(this->mouse);
	GuiRect* 			tNewHover=0;

	if(tOldHover && tOldHover->IsPressing())
	{
		this->BroadcastInputTo(tOldHover,GuiRect::ONMOUSEMOVE,tNewHoverMsg);
		return;
	}

	if(this->GetSelected())
		this->BroadcastTo(this->GetSelected(),GuiRect::ONHITTEST,tNewHoverMsg);

	tNewHover=tNewHoverMsg.hit;

	if(tOldHover!=tNewHover)
	{
		if(tOldHover)
			tOldHover->OnMouseExit(this,tNewHoverMsg);

		if(tNewHover)
			tNewHover->OnMouseEnter(this,tNewHoverMsg);
	}

	if(tNewHover)
		tNewHover->OnMouseMove(this,tNewHoverMsg);

	this->SetHover(tNewHover);
}

void Tab::OnGuiLMouseUp(void* data)
{
	mouseDown=false;

	if(this->GetHover())
	{
		this->BroadcastInputTo(this->GetHover(),GuiRect::ONMOUSEUP,Msg(this->mouse,1));

		if(this->GetHover()->GetFlag(GuiRect::FLAGS,GuiRect::DRAWPRESS))
			this->SetDraw(this->GetHover());
	}
}

void Tab::OnGuiMouseWheel(void* data)
{
	if(this->GetHover())
		this->BroadcastInputTo(this->GetHover(),GuiRect::ONMOUSEWHEEL,Msg(this->mouse,1));
}

void Tab::OnGuiLMouseDown(void* data)
{
	if(Ide::Instance()->GetPopup()!=this)
		Ide::Instance()->DestroyPopup();

	if(this->hasFrame && this->mouse.y<=BAR_HEIGHT)
	{
		float &x=this->mouse.x;
		float &y=this->mouse.y;

		GuiRect* tPreviousTabSelected=this->GetSelected();

		vec2 tDim(0,LABEL_LEFT_OFFSET);

		for(std::list<GuiRect*>::const_iterator i=this->rects.Childs().begin();i!=this->rects.Childs().end();i++)
		{
			vec2 tTextSize = this->renderer2D->MeasureText((*i)->GetName().c_str());

			tDim.x = tDim.y;
			tDim.y = tDim.x + tTextSize.x + LABEL_RIGHT_OFFSET;

			bool tMouseContained = (x>tDim.x && x< tDim.y) && (y>(BAR_HEIGHT-LABEL_HEIGHT) &&  y<BAR_HEIGHT);
			
			if(tMouseContained && (*i)!=tPreviousTabSelected)
			{
				mouseDown=true;

				this->SetSelection(*i);

				break;
			}
		}
	}
	else
	{
		if(this->GetHover())
		{
			this->BroadcastInputTo(this->GetHover(),GuiRect::ONMOUSEDOWN,Msg(this->mouse,1));

			if(this->GetHover()->GetFlag(GuiRect::FLAGS,GuiRect::DRAWPRESS))
				this->SetDraw(this->GetHover());
		}
	}
}

void Tab::OnGuiDLMouseDown(void* data)
{
	if(!this->hasFrame || this->mouse.y>BAR_HEIGHT)
	{
		if(this->GetHover())
			this->BroadcastInputTo(this->GetHover(),GuiRect::ONMOUSECLICK,Msg(this->mouse,1));
	}
}


void Tab::OnGuiRMouseUp(void* data)
{
	if(this->GetHover())
		this->BroadcastInputTo(this->GetHover(),GuiRect::ONMOUSEUP,Msg(this->mouse,3));
}

vec2 Tab::Size()
{
	return this->windowData->Size();
}


void Tab::DrawFrame()
{
	if(!this->hasFrame)
		return;

	vec2 iTabDim=this->Size();

	float tLabelLeft=0;
	float tLabelRight=5;

	this->renderer2D->Identity();

	vec2 tDim(0,LABEL_LEFT_OFFSET);

	//render label text
	for(std::list<GuiRect*>::const_iterator i=this->rects.Childs().begin();i!=this->rects.Childs().end();i++)
	{
		vec2 tTextSize = this->renderer2D->MeasureText((*i)->GetName().c_str());

		tDim.x = tDim.y;
		tDim.y = tDim.x + tTextSize.x + LABEL_RIGHT_OFFSET;

		if(this->selected==*i)
			this->renderer2D->DrawRectangle(tDim.x,(float)(BAR_HEIGHT-LABEL_HEIGHT),tDim.y,(float)((BAR_HEIGHT-LABEL_HEIGHT)+LABEL_HEIGHT),Tab::COLOR_LABEL);

		this->renderer2D->DrawText((*i)->GetName(),tDim.x,(float)BAR_HEIGHT-LABEL_HEIGHT,tDim.y,(float)(BAR_HEIGHT-LABEL_HEIGHT) + (float)LABEL_HEIGHT,vec2(0.5f,0.5f),vec2(0.5f,0.5f),GuiString::COLOR_TEXT);
	}
}

void Tab::PaintBackground()
{
	vec2 iTabDim=this->Size();

	this->renderer2D->DrawRectangle(0,0,iTabDim.x,iTabDim.y/*-Tab::CONTAINER_HEIGHT*/,Tab::COLOR_BACK);
	//this->renderer2D->DrawRectangle(0,0,iTabDim.x,iTabDim.y/*-Tab::CONTAINER_HEIGHT*/,0xff0000,false);

	this->DrawFrame();
}



void Tab::OnGuiPaint(void* data)
{
	this->PaintBackground();

	this->BroadcastTo(&this->rects,GuiRect::ONPAINT,Msg(this->rects.Edges(),vec2()));
}

void Tab::OnResizeContainer(void* data)
{

}

void Tab::OnGuiActivate(void* data)
{
	this->Broadcast(GuiRect::ONACTIVATE,data);
}
void Tab::OnGuiDeactivate(void* data)
{
	this->Broadcast(GuiRect::ONDEACTIVATE,data);
}

void Tab::OnGuiKeyDown(void* data)
{
	this->Broadcast(GuiRect::ONKEYDOWN,data);
}

void Tab::OnGuiKeyUp(void* data)
{
	this->Broadcast(GuiRect::ONKEYUP,data);
}

void Tab::OnGuiRecreateTarget(void* data)
{
	this->Broadcast(GuiRect::ONRECREATETARGET,data);
}

void Tab::SetFocus(GuiRect* iFocusedRect)
{
	/*bool tFocusHasChanged=(Tab::focused!=iFocusedRect);

	if(tFocusHasChanged && Tab::focused)
		this->SetDraw(Tab::hovered);*/

	Tab::focused=iFocusedRect;

	/*if(tFocusHasChanged && iFocusedRect)
		this->SetDraw(iHoveredRect);*/
}

void Tab::SetHover(GuiRect* iHoveredRect)
{
	bool tHoveringHasChanged=(Tab::hovered!=iHoveredRect);

	if(tHoveringHasChanged && Tab::hovered && Tab::hovered->GetFlag(GuiRect::FLAGS,GuiRect::DRAWHOVER))
		this->SetDraw(Tab::hovered);

	Tab::hovered=iHoveredRect;

	if(tHoveringHasChanged && iHoveredRect && iHoveredRect->GetFlag(GuiRect::FLAGS,GuiRect::DRAWHOVER))
		this->SetDraw(iHoveredRect);
}

void Tab::SetPressed(GuiRect* iPressedRect)
{
	Tab::pressed=iPressedRect;
}

GuiRect* Tab::GetFocus()
{
	return Tab::focused;
}

GuiRect* Tab::GetPressed()
{
	return Tab::pressed;
}

GuiRect* Tab::GetHover()
{
	return Tab::hovered;
}
 
void Tab::PushScissor(vec4 iClip,vec2 iCoord)
{
	this->renderer2D->PushScissor(iClip.x,iClip.y,iClip.z,iClip.w);
	this->renderer2D->Translate(iCoord.x,iCoord.y);

	ClipData tCd={iClip,iCoord};

	this->clips.push_back(tCd);
}
void Tab::PopScissor(ClipData* oClipData)
{
	this->renderer2D->Translate(0,0);
	this->renderer2D->PopScissor();
	if(oClipData)
		*oClipData=this->clips.back();
	this->clips.pop_back();
}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////SplitterContainer///////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
Splitter::Splitter():
splitterSize(4)
{
	floatingTabRef=0;
	floatingTab=0;
	floatingTabTarget=0;
	floatingSelectedTebGuiRect=0;
	floatingTabRefTabCount=-1;
	floatingTabTargetAnchorPos=-1;
	floatingTabTargetAnchorTabIndex=-1;

	splitterCursor=L"IDC_ARROW";
}
Splitter::~Splitter()
{
}

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////GuiCaret///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

StringEditor::Cursor::Cursor():cursor(0){}

StringEditor::StringEditor():
	string(0),
	tab(0),
	lastBlinkTime(0),
	blinking(false),
	enabled(false),
	blinkingRate(BLINKRATE)
{}

StringEditor::~StringEditor()
{
}

void StringEditor::Bind(GuiString* iString)
{
	/*this->string=iString;

	this->tab=iString ? this->string->GetRoot()->tab : 0;

	if(iString && !this->string->cursor)
	{
		this->string->cursor=new Cursor;
		this->string->cursor->cursor=iString->text->c_str();

		this->EditText(CARET_RECALC,0);
	}

	this->enabled=false;*/
}

void StringEditor::Enable(bool iEnable)
{
	this->enabled=iEnable;
}

GuiString* StringEditor::Binded()
{
	return this->string;
}

bool StringEditor::Enabled()
{
	return this->enabled;
}

bool StringEditor::EditText(unsigned int iCaretOp,void* iParam)
{
	/*float iFontHeight=this->string->font->GetHeight();

	bool tMustResize=false;

	switch(iCaretOp)
	{
	case CARET_RECALC:
		{
			const wchar_t*	pText=this->string->text->c_str();

			bool tCarriageReturn=false;
			int tCharWidth=0;

			this->string->cursor->caret=vec4(this->string->textEdges.x,this->string->textEdges.y,0,iFontHeight);

			while(*pText)
			{
				if(tCarriageReturn)
				{
					this->string->cursor->caret.x=0;
					this->string->cursor->caret.y+=iFontHeight;
					
					this->string->cursor->rowcol.x+=1;
					this->string->cursor->rowcol.y=0;

					tCarriageReturn=false;
				}

				tCharWidth=this->string->font->GetCharWidth(*pText);

				if(*pText=='\n' ||  *pText=='\r')
					tCarriageReturn=true;

				if(pText==this->string->cursor->cursor)
					break;

				this->string->cursor->caret.x+=tCharWidth;
				this->string->cursor->rowcol.y+=1;

				pText++;
			}

			break;
		}
		case CARET_CANCEL:
		{
			if(this->string->text->empty() || this->string->cursor->cursor==&this->string->text->back())
				return false;

			std::wstring::iterator sIt=this->string->text->begin()+this->string->cursor->rowcol.y;

			if(sIt!=this->string->text->end())
				this->string->text->erase(sIt);

			break;
		}
		case CARET_BACKSPACE:
		{
			if(this->string->cursor->cursor==this->string->text->c_str())
				return false;

			char tCharCode=*(--this->string->cursor->cursor);

			if(tCharCode=='\n' ||  tCharCode=='\r')
			{
				//find previous row length
				const wchar_t*	pText=this->string->cursor->cursor-1;
				unsigned int	tRowCharsWidth=0;
				unsigned int    tRowCharCount=0;

				while(*pText!='\n' &&  *pText!='\r')
				{
					tRowCharsWidth+=this->string->font->GetCharWidth(*pText);

					if(pText==this->string->text->c_str())
						break;

					pText--;
					tRowCharCount++;

				}

				this->string->cursor->caret.x=tRowCharsWidth;
				this->string->cursor->caret.y-=iFontHeight;
				this->string->cursor->rowcol.x--;
				this->string->cursor->rowcol.y=tRowCharCount;

				tMustResize=true;
			}
			else
			{
				this->string->cursor->caret.x-=this->string->font->GetCharWidth(tCharCode);
				this->string->cursor->rowcol.y--;
			}

			this->string->text->erase(this->string->cursor->cursor-this->string->text->c_str(),1);

			break;
		}
		case CARET_ADD:
		{
			char tCharcode=*(char*)iParam;

			tCharcode=='\r' ? tCharcode='\n' : 0;

			if(tCharcode=='\n' || tCharcode=='\r')
			{
				this->string->cursor->caret.x=0;
				this->string->cursor->caret.y+=iFontHeight;
				this->string->cursor->rowcol.x++;
				this->string->cursor->rowcol.y=0;
				tMustResize=true;
			}
			else
			{
				this->string->cursor->caret.x+=this->string->font->GetCharWidth(tCharcode);
				this->string->cursor->rowcol.y++;
			}

			size_t tPosition=this->string->cursor->cursor-this->string->text->c_str();

			this->string->text->insert(tPosition,1,tCharcode);
			this->string->cursor->cursor=this->string->text->c_str()+tPosition+1;

			break;
		}
		case CARET_ARROWLEFT:
		{
			if(this->string->cursor->cursor==this->string->text->c_str())
				return false;

			char tCharCode=*(--this->string->cursor->cursor);

			if(tCharCode=='\n' ||  tCharCode=='\r')
			{
				const wchar_t*	pText=this->string->cursor->cursor;
				unsigned int	tRowLenght=0;
				unsigned int    tRowCharCount=0;

				//find previous row lengthc

				while(true)
				{
					if(this->string->cursor->cursor!=pText && (*pText=='\n' || *pText=='\r'))
						break;

					tRowLenght+=this->string->font->GetCharWidth(*pText);

					if(pText==this->string->text->c_str())
						break;

					pText--;
					tRowCharCount++;
				}

				this->string->cursor->caret.x=tRowLenght;
				this->string->cursor->caret.y-=iFontHeight;
				this->string->cursor->rowcol.x--;
				this->string->cursor->rowcol.y=tRowCharCount;
			}
			else
			{
				this->string->cursor->caret.x-=this->string->font->GetCharWidth(tCharCode);
				this->string->cursor->rowcol.y--;
			}

			break;
		}
		case CARET_ARROWRIGHT:
		{
			char tCharCode=*this->string->cursor->cursor;

			if(tCharCode=='\0')
				break;

			if(tCharCode=='\n' || tCharCode=='\r')
			{
				this->string->cursor->caret.x=0;
				this->string->cursor->caret.y+=iFontHeight;
				this->string->cursor->rowcol.x++;
				this->string->cursor->rowcol.y=0;
			}
			else
			{
				this->string->cursor->caret.x+=this->string->font->GetCharWidth(tCharCode);
				this->string->cursor->rowcol.y++;
			}

			this->string->cursor->cursor++;

			break;
		}
		case CARET_ARROWUP:
		{
			unsigned int tRowCharWidth=0;
			unsigned int tColumn=0;

			//---find current rowhead

			const wchar_t* pText=this->string->cursor->cursor;

			//skip the tail of the current row if present
			if(*pText=='\r' || *pText=='\n')
				pText--;

			//find the last upper row or the head of the current row
			while( pText!=this->string->text->c_str() && *pText!='\r' &&  *pText!='\n' )
				pText--;

			//return if no previous row exists
			if(pText==this->string->text->c_str())
				return false;

			//go to the upper row pre-carriage char
			pText--;

			//find the last upper superior row or the head of the upper row
			while( pText!=this->string->text->c_str() && *pText!='\r' &&  *pText!='\n' )
				pText--;

			//go to the upper superior row pre-carriage char
			if(pText!=this->string->text->c_str())
				pText++;

			//finally found the upper matching position
			while( tColumn!=this->string->cursor->rowcol.y && *pText!='\0' && *pText!='\r' && *pText!='\n' )
			{
				tRowCharWidth+=this->string->font->GetCharWidth(*pText);

				pText++;
				tColumn++;
			}

			this->string->cursor->cursor=pText;
			this->string->cursor->caret.x=tRowCharWidth;
			this->string->cursor->caret.y-=iFontHeight;
			this->string->cursor->rowcol.y=tColumn;
			this->string->cursor->rowcol.x--;

			break;
		}
		case CARET_ARROWDOWN:
		{
			unsigned int tRowCharWidth=0;
			unsigned int tColumn=0;

			//find current rowtail

			const wchar_t* pText=this->string->cursor->cursor;

			while( *pText!='\0' && *pText!='\r' && *pText!='\n' )
			{
				this->string->cursor->cursor++;
				pText++;
			}

			if(*pText=='\0')
				return false; //no previous row exists

			pText++;
			this->string->cursor->cursor++;

			//finally found the lower matching position

			while( tColumn!=this->string->cursor->rowcol.y && *pText!='\0' && *pText!='\r' && *pText!='\n' )
			{
				tRowCharWidth+=this->string->font->GetCharWidth(*pText);

				pText++;
				this->string->cursor->cursor++;
				tColumn++;
			}

			/ *if(tColumn!=this->string->cursor->rowcol.y)//string is shorter of the lower matching position
				tRowCharWidth* /

			this->string->cursor->caret.x=tRowCharWidth;
			this->string->cursor->caret.y+=iFontHeight;
			this->string->cursor->rowcol.y=tColumn;
			this->string->cursor->rowcol.x++;

			break;
		}
	}

	if(tMustResize)
		this->string->OnSize(this->tab,Msg());

	this->caret=this->string->cursor->caret;
	this->caretHeight=this->string->font->GetHeight();

	//wprintf(L"cursor: %d,col: %d\n",this->string->cursor->cursor-this->string->text->c_str(),this->string->cursor->rowcol.y);*/
return 0;
}

void StringEditor::Draw(Tab* iCallerTab)
{
	if(this->tab && iCallerTab==this->tab && this->enabled && this->string && iCallerTab->GetFocus()==this->string)
	{
		vec2 p1(this->caret.x,this->caret.y);
		vec2 p2(this->caret.x,this->caret.y + this->caretHeight);

		//if caret pos changes, reset blinking to true;
		if(this->caret!=this->caretPast)
		{
			this->lastBlinkTime=this->blinkingRate;
			this->blinking=true;
		}

		if(Ide::Instance()->timer->GetCurrent()-this->lastBlinkTime > this->blinkingRate)
		{
			if(this->tab->BeginDraw())
			{
				Msg tMsg;

				if(this->blinking)
					this->tab->renderer2D->DrawLine(p1,p2,0x00000000,1);
				else
					this->tab->renderer2D->DrawLine(p1,p2,0xff0000,2);

				this->tab->EndDraw();
			}

			this->lastBlinkTime=Ide::Instance()->timer->GetCurrent();
			this->blinking=!this->blinking;
		}

		this->caretPast=this->caret;
	}
}


///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////Renderer2D///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


GuiFont* GuiFont::GetDefaultFont()
{
	return ::GlobalDefaultFontInstance();
}

void GuiFont::SetDefaultFont(GuiFont* iFont)
{
	::GlobalDefaultFontInstance()=iFont;
}

std::vector<GuiFont*>& GuiFont::GetFontPool()
{
	return ::GlobalFontPoolInstance();
}

GuiFont::GuiFont():tabSpaces(4){}

float GuiFont::GetHeight()const
{
	return this->height;
}

float GuiFont::GetCharWidth(wchar_t tCharacter)const
{
	return this->widths[tCharacter];
}

vec2 GuiFont::MeasureText(const wchar_t* iText)const
{
	float width=0,tWidth=0;
	float height=0;

	if(!iText)
		vec2(width,height);

	float fontHeight=height=this->height;

	wchar_t* t=(wchar_t*)iText;

	while(*t)
	{
		float tW;

		if(*t=='\n' || *t=='\r')
		{
			height+=fontHeight;
			tWidth>width?width=tWidth:0;
			tWidth=0;
		}
		else
		{
			tW=this->widths[*t];
			tWidth+=tW;
		}

		t++;
	}

	tWidth>width?width=tWidth:0;

	return vec2(width,height);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////Renderer2D///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////



Renderer2D::Renderer2D(Tab* iTabContainer):
	tab(iTabContainer),
	tabSpaces(4)
{
	
}

Renderer2D::~Renderer2D(){}



vec2 Renderer2D::MeasureText(const wchar_t* iText,const GuiFont* iFont)
{
	return iFont->MeasureText(iText);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////Renderer3DInterface///////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

Renderer3D::Renderer3D(Tab* iTabContainer):
	tab(iTabContainer)
{

}



Shader* Renderer3D::FindShader(const char* name,bool exact)
{
	return Shader::Find(name,exact);
}

void Renderer3D::SetMatrices(const float* view,const float* mdl)
{
	for(size_t i=0;i<this->shaders.size();i++)
	{
		this->shaders[i]->Use();

		if(view)
			this->shaders[i]->SetProjectionMatrix((float*)view);
		if(mdl)
			this->shaders[i]->SetModelviewMatrix((float*)mdl);
	}
}

bool Renderer3D::LoadTexture(String iFilename,Texture* iTexture)
{
    FILE *tFile=fopen(StringUtils::ToChar(iFilename).c_str(),"rb");
	bool tReturn=false;

	if(tFile)
	{
		short int bmp_signature=0x4d42;
		int jpg_signature1=0xe0ffd8ff;
		int jpg_signature2=0xdbffd8ff;
		int jpg_signature3=0xe1ffd8ff;
		int png_signature1=0x474e5089;
		int png_signature2=0x0a1a0a0d;

		int sign1;
		int sign2;

		fread(&sign1,4,1,tFile);
		fread(&sign2,4,1,tFile);

		if(bmp_signature==(short int)sign1)
		{
            fseek(tFile,0x12,SEEK_SET);
            fread(&iTexture->m_width,2,1,tFile);
            fseek(tFile,0x16,SEEK_SET);
            fread(&iTexture->m_height,2,1,tFile);
            fseek(tFile,0x1c,SEEK_SET);
            fread(&iTexture->m_bpp,2,1,tFile);

            rewind(tFile);

            fseek(tFile,0,SEEK_END);
            iTexture->m_bufsize=ftell(tFile)-54;
            rewind(tFile);
            fseek(tFile,54,SEEK_SET);

            iTexture->m_buf=new unsigned char[iTexture->m_bufsize];

            fread(iTexture->m_buf,1,iTexture->m_bufsize,tFile);

            fclose(tFile);

			return true;
		}
		else
		{
			fclose(tFile);

			std::string tCharFilename=StringUtils::ToChar(iFilename);

			if(jpg_signature1==sign1 || jpg_signature3==sign1 || jpg_signature2==sign2  || jpg_signature3==sign2 )
			{
				int ncomp;

				iTexture->m_buf=jpgd::decompress_jpeg_image_from_file(tCharFilename.c_str(),&iTexture->m_width,&iTexture->m_height,&ncomp,4);

				tReturn=iTexture->m_buf ? true : false;
			}
			else if(png_signature1==sign1 && png_signature2==sign2)
			{
				{
					std::vector<unsigned char> image;
					unsigned long w, h;
					std::vector<unsigned char> buffer;
					int error=-1;

					loadFile(buffer,tCharFilename);

					error = decodePNG(image, w, h, buffer.empty() ? 0 : &buffer[0], (unsigned long)buffer.size());

					if(!error)
					{
						iTexture->m_bufsize=image.size();
						iTexture->m_buf=new unsigned char[iTexture->m_bufsize];
						memcpy(iTexture->m_buf,&image[0],iTexture->m_bufsize);
						iTexture->m_width=w;
						iTexture->m_height=h;

						tReturn=true;
					}

					tReturn=false;
				}
			}
			else if(655360==sign1 && 0==sign2)
			{
				tReturn=LoadTGA(tCharFilename.c_str(),iTexture->m_buf,iTexture->m_bufsize,iTexture->m_width,iTexture->m_height,iTexture->m_bpp);
			}
		}
	}

	return tReturn;

}

Msg::Msg():data(0),button(0),key(0),hit(0),sender(0),senderFunc(GuiRect::MAXFUNCS){}
Msg::Msg(void* iData):data(iData),button(0),key(0),hit(0),sender(0),senderFunc(GuiRect::MAXFUNCS){}
Msg::Msg(const vec2& iMouse,int iButton):data(0),mouse(iMouse),button(iButton),key(0),hit(0),sender(0),senderFunc(GuiRect::MAXFUNCS){}
Msg::Msg(char iKey):data(0),button(0),key(iKey),hit(0),sender(0),senderFunc(GuiRect::MAXFUNCS){}
Msg::Msg(const vec4& iClip,const vec2& iCoord):data(0),button(0),key(0),hit(0),clip(iClip),coord(iCoord),sender(0),senderFunc(GuiRect::MAXFUNCS){}
Msg::Msg(GuiRect* iControl,unsigned int iControlFunc):data(0),button(0),key(0),hit(0),sender(iControl),senderFunc(iControlFunc){}


///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////GuiRect////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


#define GUIRECTINITLIST \
	parent(0), \
	userData(0), \
	next(0), \
	previous(0), \
	color(GuiRect::COLOR_BACK) \

GuiRect::GuiRect():
	GUIRECTINITLIST
{
	for(int i=0;i<MAXSWITCH;i++)
		this->SetFlags((GuiRect::Switch)i,0);

	this->SetFlag(FLAGS,CHILD,true);
}


GuiRect::~GuiRect()
{
	this->Destroy();
}

void GuiRect::Destroy()
{
	this->parent=this->previous=this->next=0;

	GuiRect* iRect=0;

	for(std::list<GuiRect*>::iterator i=this->childs.begin();i!=this->childs.end();)
	{
		iRect=*i;

		i=this->childs.erase(i);

		if(iRect->GetFlag(FLAGS,PARDELETE))
		{
			SAFEDELETE(iRect);
		}
		else
			iRect->Destroy();
	}
}


vec4 GuiRect::EdgeClip()
{
	return this->Edges();
}

vec2 GuiRect::MouseClip(const vec2& iMouse)
{
	return iMouse;
}

vec2 GuiRect::EdgeOffset()
{
	return vec2(0,0);
}

unsigned int blend(unsigned int in,unsigned int icol)
{
	unsigned int ret=0;
	unsigned char* a=(unsigned char*)&in;
	unsigned char* b=(unsigned char*)&icol;
	unsigned char* c=(unsigned char*)&ret;

	for(int i=0;i<4;i++)
	{
		unsigned char tMax=std::max<unsigned char>(a[i],b[i]);
		unsigned char tMin=std::min<unsigned char>(a[i],b[i]);
		c[i]=tMin+(tMax-tMin)/2.0f;
	}

	return ret;
}


bool GuiRect::Contains(const vec2& iPoint)
{
	vec4 tEdges=this->Edges();

	return (iPoint.x>tEdges.x && iPoint.x<=tEdges.z && iPoint.y>tEdges.y && iPoint.y<=tEdges.w);
}


void GuiRect::Broadcast(Funcs iFunc,GUIMSGDEF)
{
	for(std::list<GuiRect*>::const_iterator i=this->childs.begin();i!=this->childs.end();i++)
		iTab->BroadcastTo(*i,iFunc,iMsg);
}

unsigned int GuiRect::BlendColor(unsigned int iBaseColor,unsigned int iPressColor,unsigned int iHoverColor,unsigned int iCheckColor)
{
	unsigned int tColor=iBaseColor;

	bool a=this->GetFlag(FLAGS,DRAWCHECK);
	bool b=this->GetFlag(FLAGS,DRAWHOVER);
	bool c=this->GetFlag(FLAGS,DRAWPRESS);

	bool aa=this->IsChecked();
	bool bb=this->IsHovering();
	bool cc=this->IsPressing();

	if(a && aa)
		tColor=blend(tColor,iCheckColor);
	if(b && bb)
		tColor=blend(tColor,iHoverColor);
	if(c && cc)
		tColor=blend(tColor,iPressColor);

	return tColor;
}



void GuiRect::OnMouseDown(GUIMSGDEF)
{
	this->SetFlag(FLAGS,PRESSING,true);
}

void GuiRect::OnMouseClick(GUIMSGDEF){}

void GuiRect::OnMouseUp(GUIMSGDEF)
{
	this->SetFlag(FLAGS,PRESSING,false);
}

void GuiRect::OnMouseMove(GUIMSGDEF){}

void GuiRect::OnUpdate(GUIMSGDEF){}
void GuiRect::OnReparent(GUIMSGDEF){}
void GuiRect::OnSelected(GUIMSGDEF){}
void GuiRect::OnRender(GUIMSGDEF){}
void GuiRect::OnMouseWheel(GUIMSGDEF){}

void GuiRect::OnActivate(GUIMSGDEF){this->SetFlag(FLAGS,ACTIVE,true);}
void GuiRect::OnDeactivate(GUIMSGDEF){this->SetFlag(FLAGS,ACTIVE,false);}

void GuiRect::OnEntitySelected(GUIMSGDEF){}
void GuiRect::OnExpandos(GUIMSGDEF){}
void GuiRect::OnKeyDown(GUIMSGDEF){}
void GuiRect::OnKeyUp(GUIMSGDEF){}

void GuiRect::OnMouseEnter(GUIMSGDEF){this->SetFlag(FLAGS,HOVERING,true);}
void GuiRect::OnMouseExit(GUIMSGDEF){this->SetFlag(FLAGS,HOVERING,false);}

void GuiRect::OnEnterFocus(GUIMSGDEF){}
void GuiRect::OnExitFocus(GUIMSGDEF){}



bool	GuiRect::GetFlag(Switch iType,unsigned int iFlag)
{
	return (this->flags[iType] >> iFlag) & 1U; 
}
void	GuiRect::SetFlag(Switch iType,unsigned int iFlag,bool iVal)
{
	this->flags[iType] ^= (-iVal ^ this->flags[iType]) & (1UL << iFlag);
}
void	GuiRect::SetFlags(Switch iType,unsigned int iFlag){this->flags[iType]=iFlag;}
unsigned int	GuiRect::GetFlags(Switch iType){return this->flags[iType];}

GuiRect* GuiRect::Append(GuiRect* iRect,bool isChild)
{
	iRect->previous=this->childs.size() ? this->childs.back() : this;
	iRect->previous->next=iRect;
	iRect->next=0;

	iRect->parent=this;
	
	this->childs.push_back(iRect);

	iRect->SetFlag(FLAGS,CHILD,isChild);

	return iRect;
}

GuiRect* GuiRect::Prepend(GuiRect* iRect)
{
	iRect->previous=this;
	iRect->previous->next=this;
	iRect->next=this->childs.size() ? this->childs.front() : 0;

	if(iRect->next)
		iRect->next->previous=iRect;

	iRect->parent=this;

	this->childs.push_front(iRect);

	return iRect;
}

void GuiRect::Remove(GuiRect* iRect)
{
	this->childs.remove(iRect);
	iRect->parent=0;

	if(this->GetFlag(FLAGS,PARDELETE))
		SAFEDELETE(iRect);
}


GuiRect* GuiRect::GetParent(){return this->parent;}

void GuiRect::SetParent(GuiRect* iRect){this->parent=iRect;}

const std::list<GuiRect*>&  GuiRect::Childs(){return this->childs;}

GuiRect*	GuiRect::Next(){return this->next;}
GuiRect*	GuiRect::Previous(){return this->previous;}

GuiRoot* GuiRect::GetRoot()
{
	GuiRect* tParent=(GuiRect*)this->GetParent();

	while(tParent)
	{
		if(tParent->GetParent())
			tParent=(GuiRect*)tParent->GetParent();
		else
			return dynamic_cast<GuiRoot*>(tParent);
	}

	GuiRect* tThis=(GuiRect*)this;

	return dynamic_cast<GuiRoot*>(tThis);
}

vec4 GuiRect::Edges(){return vec4(this->left(),this->top(),this->right(),this->bottom());}

void GuiRect::Edges(const vec4& iEdges){this->edges=iEdges;}


float	GuiRect::left(){return this->edges.x;}
float	GuiRect::top(){return this->edges.y;}
float	GuiRect::right(){return this->edges.z;}
float	GuiRect::bottom(){return this->edges.w;}

void	GuiRect::left(float f){this->edges.x=f;}
void	GuiRect::top(float f){this->edges.y=f;}
void	GuiRect::right(float f){this->edges.z=f;}
void	GuiRect::bottom(float f){this->edges.w=f;}

const String& GuiRect::GetName(){return this->name;}

void GuiRect::SetName(String iName){this->name=iName;}

float	GuiRect::Width(){return this->right()-this->left();}
float	GuiRect::Height(){return this->bottom()-this->top();}

unsigned int	GuiRect::Color(){return this->color;}
void			GuiRect::Color(unsigned int iColor){this->color=iColor;}


bool	GuiRect::IsChecked(){return GetFlag(FLAGS,CHECKED);}
bool	GuiRect::IsHovering(){return GetFlag(FLAGS,HOVERING);}
bool	GuiRect::IsPressing(){return GetFlag(FLAGS,PRESSING);}
bool	GuiRect::IsActive(){return GetFlag(FLAGS,ACTIVE);}
void	GuiRect::Activate(bool iActivate){SetFlag(FLAGS,ACTIVE,iActivate);}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////GuiTreeview/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


GuiTreeView::Item::Item():expanded(true),selected(false),treeview(0)
{
	this->SetFlag(FLAGS,DRAWPRESS,true);
	this->SetFlag(FLAGS,DRAWCHECK,true);
	this->SetFlag(FLAGS,DRAWHOVER,true);
}


vec4 GuiTreeView::Item::EdgeExpandos()
{
	float	tExpandosLeft=this->EdgeLevel();

	return vec4(tExpandosLeft,GuiRect::Edges().y,tExpandosLeft+Tab::ICON_WH,GuiRect::Edges().w);
}

float GuiTreeView::Item::EdgeLevel()
{
	return this->Edges().x+this->level*Tab::ICON_WH;
}

bool GuiTreeView::Item::ExpandosContains(const vec2& iMouse)
{
	vec4 tExpandosEdges=this->EdgeExpandos();

	return iMouse.x>tExpandosEdges.x && iMouse.x<tExpandosEdges.z && iMouse.y>tExpandosEdges.y && iMouse.y<tExpandosEdges.w;
}
void GuiTreeView::Item::Append(Item& iItem)
{
	this->items.push_back(&iItem);
	iItem.SetParent(this);

	if(this->TreeView())
		this->TreeView()->RecalcItems();
}

GuiTreeView* GuiTreeView::Item::TreeView()
{
	return this->treeview;
}

bool GuiTreeView::Item::Expanded(){return this->expanded;}


void GuiTreeView::Item::Expand(bool iExpand)
{
	bool tHasChanged=this->expanded!=iExpand;

	this->expanded=iExpand;

	for(std::list<Item*>::const_iterator i=this->Items().begin();i!=this->Items().end();i++)
			(*i)->Activate(this->expanded);

	if(tHasChanged)
		this->TreeView()->RecalcItems();
}

bool GuiTreeView::Item::Selected(){return this->selected;}

const std::list<GuiTreeView::Item*>& GuiTreeView::Item::Items(){return this->items;}

void GuiTreeView::Item::Select(bool iSelect)
{
	this->selected!=iSelect;
}

void GuiTreeView::Item::OnMouseUp(GUIMSGDEF)
{
	GuiRect::OnMouseUp(GUIMSGCALL);

	if(iMsg.button==1)
	{
		float	tItemStart=this->EdgeLevel();

		if(this->Items().size() && iMsg.mouse.x>tItemStart && iMsg.mouse.x<tItemStart+Tab::ICON_WH)
		{
			this->Expand(!this->Expanded());
			iTab->SetDraw(this->TreeView());
		}
		else
		{
			this->Select(this->Selected());
			iTab->SetDraw(this);
		}
	}
}

void GuiTreeView::Item::OnPaint(GUIMSGDEF)
{
	GuiRect::OnPaint(GUIMSGCALL);

	vec4 tEdges=this->Edges();

	float tX=this->EdgeLevel();

	if(this->Items().size())
	{
		iTab->renderer2D->DrawBitmap(this->Expanded() ? iTab->iconDown : iTab->iconRight,tX,tEdges.y,tX+Tab::ICON_WH,tEdges.w);
		tX+=Tab::ICON_WH;
	}

	iTab->renderer2D->DrawText(this->GetName(),tX,tEdges.y,tEdges.z,tEdges.w,vec2(0,0.5f),vec2(0,0.5f));
}

void GuiTreeView::ConfigureContent(GuiTreeView::Item* iLabel,float& iTop,unsigned int iLevel,GuiRoot* iRoot)
{
	float	tTextLength=GuiFont::GetDefaultFont()->MeasureText(iLabel->GetName().c_str()).x;

	iLabel->Activate(true);

	iLabel->SetFlag(FLAGS,CHILD,false);

	iLabel->level=iLevel;
	iLabel->treeview=this;

	iLabel->top(iTop);
	iTop+=Tab::ICON_WH;
	iLabel->bottom(iTop);

	tTextLength+=(iLevel*Tab::ICON_WH)+(iLabel->Items().size() ? Tab::ICON_WH : 0);

	this->SetContent(tTextLength,iTop-this->top());

	if(iLabel->Items().size())
	{
		if(iLabel->expanded)
		{
			iLevel++;

			for(std::list<Item*>::const_iterator iterLabel=iLabel->Items().begin();iterLabel!=iLabel->Items().end();iterLabel++)
				this->ConfigureContent(*iterLabel,iTop,iLevel,iRoot);
		}
	}
}

GuiTreeView::GuiTreeView():
	items(0)
{
	this->SetFlag(FLAGS,HSCROLL,true);
	this->SetFlag(FLAGS,VSCROLL,true);
}

void GuiTreeView::Append(Item& iLabel)
{
	this->items=&iLabel;
	iLabel.SetParent(this);
	this->RecalcItems();
}


void GuiTreeView::RecalcItems()
{
	float			tTop=this->top();
	unsigned int	tLevel=0;

	this->ResetContent();

	if(this->items)
		this->ConfigureContent(this->items,tTop,tLevel,this->GetRoot());
}

void BroadcastGuiTreeViewItems(GuiTreeView::Item* iItem,GuiRect::Funcs iFunc,Tab* iTab,const Msg& iMsg)
{
	if(iItem->IsActive())
	{
		iTab->BroadcastTo(iItem,iFunc,iMsg);

		for(std::list<GuiTreeView::Item*>::const_iterator i=iItem->Items().begin();i!=iItem->Items().end();i++)
			BroadcastGuiTreeViewItems(*i,iFunc,iTab,iMsg);
	}
}

void GuiTreeView::Broadcast(Funcs iFunc,GUIMSGDEF)
{
	GuiRect::Broadcast(iFunc,GUIMSGCALL);

	switch(iFunc)
	{
		case ONHITTEST:
		{
			if(iMsg.hit==this)
			{
				GuiRect* hBar=this->HBar();
				GuiRect* vBar=this->VBar();

				iTab->BroadcastTo(hBar,iFunc,iMsg);
				iTab->BroadcastTo(vBar,iFunc,iMsg);

				if(this->items && iMsg.hit!=hBar && iMsg.hit!=vBar)
				{
					vec2 tMouse=iMsg.mouse;

					vec2 tOffset=this->EdgeOffset();

					iMsg.mouse.x-=tOffset.x;
					iMsg.mouse.y-=tOffset.y;

					BroadcastGuiTreeViewItems(this->items,iFunc,iTab,iMsg);

					iMsg.mouse.x=tMouse.x;
					iMsg.mouse.y=tMouse.y;
				}
			}
		}
		break;
		case ONPAINT:
		{
			iTab->PushScissor(this->EdgeClip(),this->EdgeOffset());

			if(this->items)
				BroadcastGuiTreeViewItems(this->items,iFunc,iTab,iMsg);

			iTab->PopScissor();

			iTab->BroadcastTo(this->HBar(),iFunc,iMsg);
			iTab->BroadcastTo(this->VBar(),iFunc,iMsg);
		}
		break;
		default:
		{
			if(this->items)
				BroadcastGuiTreeViewItems(this->items,iFunc,iTab,iMsg);

			iTab->BroadcastTo(this->HBar(),iFunc,iMsg);
			iTab->BroadcastTo(this->VBar(),iFunc,iMsg);
		}
	}
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////GuiPropertyTree/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

///////////////////////////////////////////////
//////GuiPropertyTree::Container::Property/////
///////////////////////////////////////////////

GuiPropertyTree::Container::Property::Property(String iDescription,GuiRect& iProperty,float iHeight):height(iHeight),property(&iProperty)
{
	this->SetFlag(FLAGS,DRAWHOVER,true);
	this->SetFlag(FLAGS,CHILD,false);

	this->SetName(iDescription);
	this->Append(&iProperty);

	this->property->SetFlag(FLAGS,CHILD,false);

	this->property->Activate(true);
}

float GuiPropertyTree::Container::Property::EdgeLevel()
{
	return this->Container()->PropertyTree()->Edges().x+this->Container()->level*Tab::ICON_WH + Tab::ICON_WH;
}

GuiPropertyTree::Container* GuiPropertyTree::Container::Property::Container()
{
	return this->container;
}

void GuiPropertyTree::Container::Property::OnSize(GUIMSGDEF)
{
	vec4 tEdges=this->Edges();

	tEdges.x=this->Container()->EdgeSplitter()+4;

	this->property->Edges(tEdges);
}

void GuiPropertyTree::Container::Property::OnPaint(GUIMSGDEF)
{
	GuiRect::OnPaint(GUIMSGCALL);

	vec4 tEdges=this->Edges();
	float tLeft=this->EdgeLevel();
	float tSplitterLeft=this->Container()->EdgeSplitter();

	//draw description
	iTab->renderer2D->DrawText(this->GetName(),tLeft,tEdges.y,tSplitterLeft,tEdges.w,vec2(0,0.5f),vec2(0,0.5f),0xffffff);
	//draw splitter
	iTab->renderer2D->DrawRectangle(tSplitterLeft,tEdges.y,tSplitterLeft+4,tEdges.w,0x202020);
}



void GuiPropertyTree::Container::Property::OnMouseDown(GUIMSGDEF)
{
	GuiRect::OnMouseDown(GUIMSGCALL);

	float tPropertyLeft=this->EdgeLevel();
	float tSplitterLeft=this->Container()->EdgeSplitter();

	if(iMsg.mouse.x>tSplitterLeft && iMsg.mouse.x<tSplitterLeft+4)
	{
		this->Container()->PropertyTree()->SplitterPressed(this->Container());
		iTab->SetCursor(1);
	}
}


void GuiPropertyTree::Container::Property::OnMouseMove(GUIMSGDEF)
{
	GuiPropertyTree::Container*			tContainer=this->container;
	GuiPropertyTree*&					tPropertyTree=tContainer->propertytree;
	
	float tPropertyLeft=this->EdgeLevel();
	float tSplitterLeft=this->Container()->EdgeSplitter();

	if(tPropertyTree->SplitterPressed() && tContainer==tPropertyTree->SplitterPressed())
	{
		iTab->SetCursor(1);

		tPropertyTree->SplitterPressed()->Splitter(iMsg.mouse.x - tPropertyLeft - 2);

		for(std::list<GuiRect*>::const_iterator i=tContainer->Items().begin();i!=tContainer->Items().end();i++)
		{
			iTab->BroadcastTo(*i,ONSIZE);
			iTab->SetDraw(*i);
		}
	}
	else
	{
		if(iMsg.mouse.x>tSplitterLeft && iMsg.mouse.x<tSplitterLeft+4)
			iTab->SetCursor(1);
	}
}

void GuiPropertyTree::Container::Property::OnMouseUp(GUIMSGDEF)
{
	GuiRect::OnMouseUp(GUIMSGCALL);

	if(this->container->propertytree->splitterPressed)
		this->container->propertytree->splitterPressed=0;
}

///////////////////////////////////////////////
//////GuiPropertyTree::Container::Container////
///////////////////////////////////////////////

GuiPropertyTree::Container::Container(String iDescription):expanded(true),propertytree(0),expandedHeight(0),splitter(100)
{
	this->SetFlag(FLAGS,DRAWHOVER,true);
	this->SetFlag(FLAGS,CHILD,false);

	this->SetName(iDescription);
}

GuiPropertyTree::Container::Container():expanded(true),propertytree(0),expandedHeight(0),splitter(100)
{
	this->SetFlag(FLAGS,DRAWHOVER,true);
	this->SetFlag(FLAGS,CHILD,false);
}

vec4 GuiPropertyTree::Container::EdgeExpandos()
{
	float	tExpandosLeft=this->EdgeLevel();

	return vec4(tExpandosLeft,this->top(),tExpandosLeft+Tab::ICON_WH,this->bottom());
}

float GuiPropertyTree::Container::EdgeLevel()
{
	return this->propertytree->left()+this->level*Tab::ICON_WH;
}

float GuiPropertyTree::Container::EdgeSplitter()
{
	return this->EdgeLevel()+Tab::ICON_WH+this->Splitter();
}

bool GuiPropertyTree::Container::ExpandosContains(const vec2& iMouse)
{
	vec4 tExpandosEdges=this->EdgeExpandos();

	return iMouse.x>tExpandosEdges.x && iMouse.x<tExpandosEdges.z && iMouse.y>tExpandosEdges.y && iMouse.y<tExpandosEdges.w;
}

bool GuiPropertyTree::Container::Expanded()
{
	return this->expanded;
}

void GuiPropertyTree::Container::Expand(bool iExpand)
{
	bool tHasChanged=this->expanded!=iExpand;

	this->expanded=iExpand;

	for(std::list<GuiRect*>::const_iterator i=this->Items().begin();i!=this->Items().end();i++)
		(*i)->Activate(this->expanded);

	if(tHasChanged)
		this->PropertyTree()->RecalcItems();
}

float GuiPropertyTree::Container::Splitter(){return this->splitter;}

void GuiPropertyTree::Container::Splitter(float iSplitter){this->splitter=iSplitter;}

const std::list<GuiRect*>& GuiPropertyTree::Container::Items(){return this->items;}

GuiPropertyTree::Container* GuiPropertyTree::Container::Append(Container& iContainer)
{
	this->items.push_back(&iContainer);
	iContainer.SetParent(this);
	if(this->PropertyTree())
		this->PropertyTree()->RecalcItems();
	return &iContainer;
}

GuiPropertyTree::Container::Property* GuiPropertyTree::Container::Append(Property& iProperty)
{
	this->items.push_back(&iProperty);
	iProperty.SetParent(this);
	if(this->PropertyTree())
		this->PropertyTree()->RecalcItems();
	return &iProperty;
}

GuiPropertyTree* GuiPropertyTree::Container::PropertyTree(){return this->propertytree;}


void GuiPropertyTree::Container::OnMouseUp(GUIMSGDEF)
{
	GuiRect::OnMouseUp(GUIMSGCALL);

	if(iMsg.button==1)
	{
		float tItemLeft=this->EdgeLevel();

		if(this->Items().size() && iMsg.mouse.x>tItemLeft && iMsg.mouse.x<tItemLeft+Tab::ICON_WH)
		{
			this->Expand(!this->Expanded());
			iTab->SetDraw(this->PropertyTree());
		}
	}
}

void GuiPropertyTree::Container::OnPaint(GUIMSGDEF)
{
	GuiRect::OnPaint(GUIMSGCALL);

	vec4 tEdges=this->Edges();

	float tLeadEdge=this->EdgeLevel();

	if(this->Items().size())
	{
		iTab->renderer2D->DrawBitmap(this->expanded ? iTab->iconDown : iTab->iconRight,tLeadEdge,tEdges.y,tLeadEdge+Tab::ICON_WH,this->bottom());
		tLeadEdge+=Tab::ICON_WH;
	}

	iTab->renderer2D->DrawText(this->GetName(),tLeadEdge,tEdges.y,tEdges.z,tEdges.w,vec2(0,0.5f),vec2(0,0.5f));
}

///////////////////////////////////////////////
//////////////////GuiPropertyTree//////////////
///////////////////////////////////////////////

void		 GuiPropertyTree::SplitterPressed(GuiPropertyTree::Container* iContainer)
{
	this->splitterPressed=iContainer;
}
GuiPropertyTree::Container*	 GuiPropertyTree::SplitterPressed()
{
	return this->splitterPressed;
}


void BroadcastGuiPropertyTreeItems(GuiRect* iItem,GuiRect::Funcs iFunc,Tab* iTab,const Msg& iMsg)
{
	if(iItem->IsActive())
	{
		iTab->BroadcastTo(iItem,iFunc,iMsg);

		GuiPropertyTree::Container::Property*	tProperty=dynamic_cast<GuiPropertyTree::Container::Property*>(iItem);
		GuiPropertyTree::Container*				tContainer=dynamic_cast<GuiPropertyTree::Container*>(iItem);

		if(tContainer)
		{
			for(std::list<GuiRect*>::const_iterator i=tContainer->Items().begin();i!=tContainer->Items().end();i++)
				BroadcastGuiPropertyTreeItems(*i,iFunc,iTab,iMsg);
		}
		else iTab->BroadcastTo(iItem,iFunc,iMsg);
	}
}

void GuiPropertyTree::Broadcast(Funcs iFunc,GUIMSGDEF)
{
	GuiRect::Broadcast(iFunc,GUIMSGCALL);

	switch(iFunc)
	{
	case ONHITTEST:
		{
			if(iMsg.hit==this)
			{
				GuiRect* hBar=this->HBar();
				GuiRect* vBar=this->VBar();

				iTab->BroadcastTo(hBar,iFunc,iMsg);
				iTab->BroadcastTo(vBar,iFunc,iMsg);

				if(this->items && iMsg.hit!=hBar && iMsg.hit!=vBar)
				{
					vec2 tMouse=iMsg.mouse;

					vec2 tOffset=this->EdgeOffset();

					iMsg.mouse.x-=tOffset.x;
					iMsg.mouse.y-=tOffset.y;

					BroadcastGuiPropertyTreeItems(this->items,iFunc,iTab,iMsg);

					iMsg.mouse.x=tMouse.x;
					iMsg.mouse.y=tMouse.y;
				}
			}
		}
		break;
	case ONPAINT:
		{
			iTab->PushScissor(this->EdgeClip(),this->EdgeOffset());

			if(this->items)
				BroadcastGuiPropertyTreeItems(this->items,iFunc,iTab,iMsg);

			iTab->PopScissor();

			iTab->BroadcastTo(this->HBar(),iFunc,iMsg);
			iTab->BroadcastTo(this->VBar(),iFunc,iMsg);
		}
		break;
	default:
		{
			if(this->items)
				BroadcastGuiPropertyTreeItems(this->items,iFunc,iTab,iMsg);

			iTab->BroadcastTo(this->HBar(),iFunc,iMsg);
			iTab->BroadcastTo(this->VBar(),iFunc,iMsg);
		}
	}
}

void GuiPropertyTree::ConfigureContent(GuiRect* iParent,GuiRect* iGui,float& iTop,unsigned int iLevel,GuiRoot* iRoot)
{
	Container* tContainer=dynamic_cast<Container*>(iGui);

	if(tContainer)
	{
		float	tTextLength=GuiFont::GetDefaultFont()->MeasureText(tContainer->GetName().c_str()).x;

		tContainer->Activate(true);

		tContainer->SetFlag(FLAGS,CHILD,false);

		tContainer->level=iLevel;
		tContainer->propertytree=this;
		tContainer->expandedHeight=0;

		tContainer->top(iTop);
		iTop+=Tab::ICON_WH;
		tContainer->bottom(iTop);

		tTextLength+=(iLevel*Tab::ICON_WH)+(tContainer->items.size() ? Tab::ICON_WH : 0);

		this->SetContent(tTextLength,iTop-this->top());

		if(tContainer->Items().size())
		{
			if(tContainer->expanded)
			{
				iLevel++;

				for(std::list<GuiRect*>::const_iterator iterLabel=tContainer->Items().begin();iterLabel!=tContainer->Items().end();iterLabel++)
					this->ConfigureContent(iGui,*iterLabel,iTop,iLevel,iRoot);
			}
		}
	}
	else
	{
		Container::Property*	tProperty=dynamic_cast<Container::Property*>(iGui);
		Container*				tContainer=dynamic_cast<Container*>(iParent);

		if(tProperty && tContainer)
		{
			tProperty->Activate(true);

			tProperty->SetFlag(FLAGS,CHILD,false);

			tProperty->container=tContainer;

			tProperty->top(iTop);
			iTop=tProperty->bottom();

			this->SetContent(this->GetContent().x,iTop-this->top());

			iRoot->GetTab()->BroadcastTo(tProperty,ONSIZE);
		}
		else
			DEBUG_BREAK();
	}
}

GuiPropertyTree::GuiPropertyTree():
	items(0),
	splitterPressed(0)
{
	this->SetFlag(FLAGS,HSCROLL,true);
	this->SetFlag(FLAGS,VSCROLL,true);
}

void GuiPropertyTree::InsertContainer(Container& iLabel)
{
	this->items=&iLabel;
	iLabel.SetParent(this);
	this->RecalcItems();
}

void GuiPropertyTree::RecalcItems()
{
	float			tTop=this->top();
	unsigned int	tLevel=0;

	this->ResetContent();

	if(this->items)
		this->ConfigureContent(0,this->items,tTop,tLevel,GetRoot());
}





///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////Compiler/////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

Compiler::Compiler()
{
	ideSrcPath=Ide::Instance()->pathExecutable.PathUp(5) + L"\\src";
	ideLibPath=Ide::Instance()->pathExecutable.Path();

	COMPILER msCompiler={L"ms",
						 L"vc2010",
						 L"cl.exe ",
						 L"linker.exe ",
						 L" /nologo /MDd /ZI /EHsc ",
						 L" /link /MANIFEST:NO /DLL /NOENTRY ",
						 L" /OUT:",
						 L"enginelibMS",
						 L".lib ",
						 L" /I",
						 L" ",
						 L"kernel32.lib"
						};

	COMPILER mingwCompiler={L"mingw",
							L"i686-w64-mingw32",
							L"c:\\sdk\\mingw32\\bin\\i686-w64-mingw32-g++.exe ",
							L"c:\\sdk\\mingw32\\bin\\i686-w64-mingw32-ld.exe ",
							L" -O0 -g -shared ",
							L"",
							L" -o ",
							L"enginelibMingW",
							L".dll ",
							L" -I"
						   };

	COMPILER llvmCompiler={L"llvm",
						   L"5.0.0 32bit",
						   L"c:\\sdk\\llvm32\\bin\\clang-cl.exe ",
						   L"c:\\sdk\\llvm32\\bin\\lld-link.exe ",
						   L" /nologo /MDd /ZI /EHsc ",
						   L" /link /MANIFEST:NO /DLL /NOENTRY ",
						   L" /OUT:",
						   L"enginelibLLVM",
						   L".lib ",
						   L" /I"
						  };

	compilers.push_back(msCompiler);
	compilers.push_back(mingwCompiler);
	compilers.push_back(llvmCompiler);
}


Compiler::~Compiler(){}


String Compiler::Compose(unsigned int iCompiler,EditorScript* iScript)
{
	Compiler*			icplr=Ide::Instance()->compiler;
	Compiler::COMPILER& cplr=icplr->compilers[iCompiler];

	String tOutputModule=iScript->module + L"\\" + iScript->file.Name() + L".dll ";
	String tScriptSource=Ide::Instance()->folderProject + L"\\" + iScript->file.File();
	String tIdeSourcePath=Ide::Instance()->compiler->ideSrcPath +  L" ";
	String tEngineLibrary=icplr->ideLibPath + L"\\" + cplr.engineLibraryName + cplr.engineLibraryExtension + L" ";


	String tCompilerExecutableString;

	if(iCompiler==Compiler::COMPILER_MS)
		tCompilerExecutableString=L"vcvars32.bat && ";

	tCompilerExecutableString+=cplr.compilerExecutable;

	String	tComposedOutput=tCompilerExecutableString +
		cplr.compilerFlags +
		cplr.includeHeadersPrefix +
		tIdeSourcePath +
		tScriptSource +
		cplr.linkerFlags +
		cplr.outputCommand +
		tOutputModule +
		tEngineLibrary +
		cplr.includeLibPrefix + 
		cplr.additionalLibs;

	return	tComposedOutput;
}

bool Compiler::Compile(EditorScript* iScript)
{
	if(iScript->runtime)
		return false;

	bool retVal;

	String tOriginalScriptSource=iScript->LoadScript();
	String tCompileScriptSource=tOriginalScriptSource;

	{
		tCompileScriptSource+=L"\n\nextern \"C\" __declspec(dllexport) EntityScript* Create(){return new " + iScript->Entity()->name + L"_;}";
		tCompileScriptSource+=L"\n\nextern \"C\" __declspec(dllexport) void Destroy(EntityScript* iDestroy){SAFEDELETE(iDestroy);}\n\n";
	}

	File tCompilerErrorOutput=Ide::Instance()->folderAppData + L"\\error.output";


	//delete error.output

	if(tCompilerErrorOutput.Exist())
	{
		if(!tCompilerErrorOutput.Delete())
			DEBUG_BREAK();
	}

	//create random directory for the module if not exist yet

	if(iScript->module.empty())
		iScript->module=Ide::Instance()->subsystem->RandomDir(Ide::Instance()->folderAppData,8);

	//append exports to source

	iScript->SaveScript(tCompileScriptSource);

	//compose the compiler command line

	unsigned int tCompiler=Compiler::COMPILER_MS;

	String tCommandLineMingW=this->Compose(tCompiler,iScript);

	//execute compilation

	bool tExecuteWithSuccess=Ide::Instance()->subsystem->Execute(iScript->module,tCommandLineMingW,tCompilerErrorOutput.path,true,true,true);

	if(!tExecuteWithSuccess)
		DEBUG_BREAK();

	//unroll exports

	iScript->SaveScript(tOriginalScriptSource);

	//convert compiler output to readable locale

	String tWideCharCompilationOutput=StringUtils::ReadCharFile(tCompilerErrorOutput.path,L"rb");

	//extract and parse breakpoint line addresses

	File tLineAddressesOutput=iScript->module + L"\\laddrss.output";

	String tScriptFileName=iScript->file.Name();

	String tSharedObjectFileName=tScriptFileName + L".dll";
	String tSharedObjectSourceName=tScriptFileName + L".cpp";

	String tParseLineAddressesCommandLine=L"objdump --dwarf=decodedline " + tSharedObjectFileName + L" | find \""+ tSharedObjectSourceName + L"\" | find /V \":\"";

	tExecuteWithSuccess=Ide::Instance()->subsystem->Execute(iScript->module,tParseLineAddressesCommandLine,tLineAddressesOutput.path,true,true,true);

	if(!tExecuteWithSuccess)
		DEBUG_BREAK();

	if(tLineAddressesOutput.Open(L"rb"))
	{
		EditorScript* tEditorScript=(EditorScript*)iScript;

		int tNumberOfLines=tLineAddressesOutput.CountOccurrences('\n');

		char c[500];
		unsigned int line;

		for(int i=0;i<tNumberOfLines;i++)
		{
			fscanf(tLineAddressesOutput.data,"%s",c);
			fscanf(tLineAddressesOutput.data,"%u",&line);
			fscanf(tLineAddressesOutput.data,"%s",&c);

			if(i>7)//skip source exports
			{
				Debugger::Breakpoint tLineAddress;

				sscanf(c,"%lx", &tLineAddress.address);
				tLineAddress.line=line;
				tLineAddress.script=iScript;


				Ide::Instance()->debugger->allAvailableBreakpoints.push_back(tLineAddress);
			}
		}

		tLineAddressesOutput.Close();

		if(!tLineAddressesOutput.Delete())
			DEBUG_BREAK();
	}

	//spawn a compilerViewer and show it if errors  @mic best to send message to the guicompilerviewer

	EditorScript* tEditorScript=(EditorScript*)iScript;

	MainContainer* tMainContainer=Ide::Instance()->mainAppWindow;

	bool noErrors=GuiCompilerViewer::Instance()->ParseCompilerOutputFile(tWideCharCompilationOutput);

	/*
	guiCompilerViewer->OnSize(tabContainer);
	guiCompilerViewer->OnActivate(tabContainer);

	if(false==noErrors)
		tabContainer->SetSelection(guiCompilerViewer);
	*/

	wprintf(L"%s on compiling %s\n",noErrors ? "OK" : "ERROR",iScript->file.c_str());

	return retVal;
}


bool Compiler::LoadScript(EditorScript* iScript)
{
	if(!iScript->module.size())
		return false;

	EntityScript*	(*tCreateModuleClassFunction)()=0;
	String          tModuleFile=iScript->module + L"\\" + iScript->file.Name() + L".dll";

	if(!iScript->handle)
	{
		iScript->handle=Ide::Instance()->subsystem->LoadLibrary(tModuleFile.c_str());

		if(!iScript->handle)
			return false;

	}

	tCreateModuleClassFunction=(EntityScript* (*)())Ide::Instance()->subsystem->GetProcAddress(iScript->handle,L"Create");

	if(tCreateModuleClassFunction)
	{
		iScript->runtime=tCreateModuleClassFunction();
		iScript->runtime->entity=iScript->Entity();

		Ide::Instance()->debugger->RunDebuggeeFunction(iScript,0);

		return true;
	}
	else
	{
		wprintf(L"error creating module %s\n",tModuleFile.c_str());

		SAFEDELETEARRAY(iScript->runtime);

		if(iScript->handle && Ide::Instance()->subsystem->FreeLibrary(iScript->handle))
			DEBUG_BREAK();
	}

	return false;
}

bool Compiler::UnloadScript(EditorScript* iScript)
{
	const bool tDestroyInTheDLL=false;

	if(iScript->runtime)
	{
		if(!iScript->handle)
			return false;

		//iScript->runtime->deinit();
		Ide::Instance()->debugger->RunDebuggeeFunction(iScript,2);

		if(!Ide::Instance()->subsystem->FreeLibrary(iScript->handle))
			return false;
		else
			iScript->handle=0;

		if(tDestroyInTheDLL)
		{
			void (*DestroyScript)(EntityScript*)=(void(*)(EntityScript*))Ide::Instance()->subsystem->GetProcAddress(iScript->handle,L"Destroy");

			std::vector<GuiViewport*> tGuiViewport;

			Ide::Instance()->mainAppWindow->GetTabRects<GuiViewport>(tGuiViewport);

			Tab* tabContainerRunningUpdater=(Tab*)tGuiViewport[0]->GetRoot()->GetTab();

			tabContainerRunningUpdater->drawTask->pause=true;

			while(tabContainerRunningUpdater->drawTask->executing);

			DestroyScript(iScript->runtime);
			iScript->runtime=0;

			tabContainerRunningUpdater->drawTask->pause=false;
		}
		else
			iScript->runtime=0;

		return true;
	}

	return true;
}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////GuiRootRect/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiRoot::GuiRoot(Tab* t):tab(t)
{
	this->SetName(L"RootRect");
	this->SetFlag(FLAGS,ACTIVE,true);
}

GuiRoot::~GuiRoot()
{
}

void GuiRoot::OnSize(GUIMSGDEF)
{
	vec2 tTabSize=this->GetTab()->Size();

	vec4 tGuiRootSize(0,this->GetTab()->hasFrame ? Tab::BAR_HEIGHT : 0,tTabSize.x,tTabSize.y);

	this->Edges(tGuiRootSize);

	if(this->GetTab()->GetSelected())
		this->GetTab()->GetSelected()->Edges(tGuiRootSize);
}

GuiRect* GuiRoot::Append(GuiRect* iGui)
{
	GuiRect::Append(iGui);
	this->tab->SetSelection(iGui);
	return iGui;
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////StringValue///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiStringProperty::GuiStringProperty(void* iValuePointer1,unsigned int iValueType,void* iValuePointer2,unsigned int iValueParameter1,unsigned int iValueParameter2):
	valuePointer1(iValuePointer1),
	valuePointer2(iValuePointer2),
	valueType(iValueType),
	valueParameter1(iValueParameter1),
	valueParameter2(iValueParameter2)
{
}
void GuiStringProperty::OnPaint(GUIMSGDEF)
{
	GuiRect::OnPaint(GUIMSGCALL);

	const unsigned int ctMaxTmpArraySize=256;

	String tValue;

	switch(this->valueType)
	{
	case GuiStringProperty::BOOL:
		{
			bool& tBool=*(bool*)this->valuePointer1;

			tValue=tBool ? L"True" : L"False";
		}
		break;
	case GuiStringProperty::STRING:
		{
			String& tString=*(String*)this->valuePointer1;

			tValue=tString;
		}
		break;
	case GuiStringProperty::BOOLPTR:
		{
			void* pRef=*(void**)this->valuePointer1;

			tValue=pRef ? L"True" : L"False";
		}
		break;
	case GuiStringProperty::INT:
		{
			int& tInt=*(int*)this->valuePointer1;

			wchar_t tCharInt[ctMaxTmpArraySize];
			swprintf(tCharInt,L"%d",tInt);

			tValue=tCharInt;
		}
		break;
	case GuiStringProperty::FLOAT:
		{
			float& tFloat=*(float*)this->valuePointer1;

			//https://stackoverflow.com/questions/16413609/printf-variable-number-of-decimals-in-float

			wchar_t tCharFloat[ctMaxTmpArraySize];
			swprintf(tCharFloat,L"%*.*f",this->valueParameter1,this->valueParameter2,tFloat);

			tValue=tCharFloat;
		}
		break;
	case GuiStringProperty::VEC2:
		{
			vec2& tVec2=*(vec2*)this->valuePointer1;

			wchar_t tCharVec2[ctMaxTmpArraySize];
			swprintf(tCharVec2,L"%*.*f , %*.*f",this->valueParameter1,this->valueParameter2,tVec2.x,this->valueParameter1,this->valueParameter2,tVec2.y);

			tValue=tCharVec2;
		}
		break;
	case GuiStringProperty::VEC3:
		{
			vec3& tVec3=*(vec3*)this->valuePointer1;

			wchar_t tCharVec3[ctMaxTmpArraySize];
			swprintf(tCharVec3,L"%*.*f , %*.*f , %*.*f",	this->valueParameter1,this->valueParameter2,tVec3.x,
				this->valueParameter1,this->valueParameter2,tVec3.y,
				this->valueParameter1,this->valueParameter2,tVec3.z);

			tValue=tCharVec3;
		}
		break;
	case GuiStringProperty::VEC4:
		{
			vec4& tVec4=*(vec4*)this->valuePointer1;

			wchar_t tCharVec4[ctMaxTmpArraySize];
			swprintf(tCharVec4,L"%*.*f , %*.*f , %*.*f , %*.*f",	this->valueParameter1,this->valueParameter2,tVec4.x,
				this->valueParameter1,this->valueParameter2,tVec4.y,
				this->valueParameter1,this->valueParameter2,tVec4.z,
				this->valueParameter1,this->valueParameter2,tVec4.w);

			tValue=tCharVec4;
		}
		break;
	case GuiStringProperty::PTR:
		{
			wchar_t tCharPointer[ctMaxTmpArraySize];
			swprintf(tCharPointer,L"0x%p",this->valuePointer1);

			tValue=tCharPointer;
		}
		break;
	case GuiStringProperty::MAT4POS:
		{
			mat4& tMat4=*(mat4*)this->valuePointer1;

			vec3 tPosition=tMat4.position();

			wchar_t tCharVec3[ctMaxTmpArraySize];
			swprintf(tCharVec3,L"%*.*f , %*.*f , %*.*f",	this->valueParameter1,this->valueParameter2,tPosition.x,
				this->valueParameter1,this->valueParameter2,tPosition.y,
				this->valueParameter1,this->valueParameter2,tPosition.z);

			tValue=tCharVec3;
		}
		break;
	case GuiStringProperty::ENTITYLISTSIZE:
		{
			std::list<Entity*>& tEntityVec=*(std::list<Entity*>*)this->valuePointer1;

			size_t tCount=tEntityVec.size();

			wchar_t tCharVecSize[10];
			swprintf(tCharVecSize,L"%d",tCount);

			tValue=tCharVecSize;
		}
		break;
	case GuiStringProperty::VEC3VECSIZE:
		{
			std::vector<vec3>& tVec3Vector=*(std::vector<vec3>*)this->valuePointer1;

			size_t tCount=tVec3Vector.size();

			wchar_t tCharVecSize[10];
			swprintf(tCharVecSize,L"%d",tCount);

			tValue=tCharVecSize;
		}
		break;
	case GuiStringProperty::VEC3LISTSIZE:
		{
			std::list<vec3>& tVec3List=*(std::list<vec3>*)this->valuePointer1;

			size_t tCount=tVec3List.size();

			wchar_t tCharVecSize[10];
			swprintf(tCharVecSize,L"%d",tCount);

			tValue=tCharVecSize;
		}
		break;
	case GuiStringProperty::ANIMATIONVECSIZE:
		{
			std::vector<Animation*>& tAnimationVec=*(std::vector<Animation*>*)this->valuePointer1;

			size_t tCount=tAnimationVec.size();

			wchar_t tCharVecSize[10];
			swprintf(tCharVecSize,L"%d",tCount);

			tValue=tCharVecSize;
		}
		break;
	case GuiStringProperty::ISBONECOMPONENT:
		{
			Entity* tEntity=(Entity*)this->valueParameter1;

			tValue=L"must implement in GuiStringValue::ISBONECOMPONENT";
		}
		break;
	case GuiStringProperty::FLOAT2MINUSFLOAT1:
		{
			float& a=*(float*)this->valuePointer1;
			float& b=*(float*)this->valuePointer2;

			wchar_t tCharFloatMinusOp[20];
			swprintf(tCharFloatMinusOp,L"%*.*g",this->valueParameter1,this->valueParameter2,b-a);

			tValue=tCharFloatMinusOp;
		}
		break;
	case GuiStringProperty::VEC32MINUSVEC31:
		{
			vec3& a=*(vec3*)this->valuePointer1;
			vec3& b=*(vec3*)this->valuePointer2;

			vec3 tVecResult=b-a;

			wchar_t tCharVec3[ctMaxTmpArraySize];
			swprintf(tCharVec3,L"%*.*g , %*.*g , %*.*g",	this->valueParameter1,this->valueParameter2,tVecResult.x,
				this->valueParameter1,this->valueParameter2,tVecResult.y,
				this->valueParameter1,this->valueParameter2,tVecResult.z);

			tValue=tCharVec3;
		}
		break;
	default:
		tValue=L"GuiStringValue Must Implement";
	}

	if(this->valueType)
	{
		vec4 tEdges=this->Edges();
		iTab->renderer2D->DrawText(tValue,tEdges.x,tEdges.y,tEdges.z,tEdges.w,vec2(0,0.5f),vec2(0,0.5f),0xffffff,GuiFont::GetDefaultFont());
	}
	
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////GuiString///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiString::GuiString():align(0.0f,0.5f),spot(0.0f,0.5f),color(GuiString::COLOR_TEXT),font((GuiFont*)GuiFont::GetDefaultFont()){}


const String&  GuiString::Text(){return this->text;}
void	GuiString::Text(const String& iText){this->text=iText;}

void  GuiString::Font(GuiFont* iFont){this->font=iFont;}
GuiFont*	GuiString::Font(){return this->font;}

void GuiString::Spot(float iSpotX,float iSpotY){this->spot.make(iSpotX,iSpotY);}
const vec2& GuiString::Spot(){return this->spot;}

void GuiString::Alignment(float iAlignX,float iAlignY){this->align.make(iAlignX,iAlignY);}
const vec2& GuiString::Alignment(){return this->align;}


void GuiString::OnPaint(GUIMSGDEF)
{
	GuiRect::OnPaint(GUIMSGCALL);

	vec4 tEdges=this->Edges();

	iTab->renderer2D->DrawText(this->text,tEdges.x,tEdges.y,tEdges.z,tEdges.w,this->spot,this->align,this->color,this->font);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////GuiListBox////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


namespace GuiListBoxHelpers
{
	void OnItemPressed(void*);
}

GuiListBox::GuiListBox(){}
GuiListBox::~GuiListBox(){}

GuiListBox::Item* GuiListBox::Append(Item* iItem)
{
	iItem->listbox=this;

	iItem->top(this->Childs().empty() ? this->top() : this->Childs().back()->bottom());

	iItem->Activate(true);

	GuiRect::Append(iItem);

	return iItem;
}

void GuiListBox::Remove(Item* iItem)
{
	for(std::list<GuiRect*>::const_iterator i=this->Childs().begin();i!=this->Childs().end();)
	{
		if(iItem==*i)
		{
			GuiRect* tFound=*i;
			GuiRect* tNext=(std::next(i) != this->Childs().end() ?  *std::next(i) : 0);

			if(tNext)
				tNext->top(tFound->top());

			this->GuiRect::Remove(iItem);

			return;
		}
		else i++;
	}

	

	
}

GuiListBox::Item::Item(void* iValuePointer1,unsigned int iValueType,void* iValuePointer2):GuiStringProperty(iValuePointer1,iValueType,iValuePointer2),height(Tab::ICON_WH),listbox(0){}


GuiListBox* GuiListBox::Item::ListBox(){return this->listbox;}

float GuiListBox::Item::left(){return this->ListBox()->left();}
float GuiListBox::Item::right(){return this->ListBox()->left()+this->ListBox()->Maxes().x;}
float GuiListBox::Item::bottom(){return this->top()+this->height;}

/*
void GuiListBox::AddItem(void* iValuePointer1,unsigned int iValueType,void* iValuePointer2,unsigned int iValueParameter1,unsigned int iValueParameter2)
{
	GuiButton* tButton=new GuiButton;
//	//tButton->GetAutoEdges()->ref.y=this->GetChilds().size() ? &this->GetChilds().back()->GetEdges().w : 0;
//	//tButton->GetAutoEdges()->fix.w=GuiRect::ROW_HEIGHT;
	//tButton->SetStringMode(iValuePointer1,iValueType,iValuePointer2,iValueParameter1,iValueParameter2);
//	tButton->GetColor(GuiRect::BACKGROUND)+=(this->GetChilds().size()%2)==0 ? 0x101010 : 0x151515;
//	tButton->GetColor(GuiRect::HOVERING)+=0x202020;
	tButton->func=GuiListBoxHelpers::OnItemPressed;
	tButton->param=tButton;
	tButton->SetUserData(this);
	tButton->Spot(0,0);
	tButton->Alignment(0,0);
	//tButton->margins.x=10;

	this->Append(tButton);
}*/

/*
void GuiListBox::AddItem(String iString)
{
	GuiButton* tButton=new GuiButton;
//	//tButton->GetAutoEdges()->ref.y=this->GetChilds().size() ? &this->GetChilds().back()->GetEdges().w : 0;
//	//tButton->GetAutoEdges()->fix.w=GuiRect::ROW_HEIGHT;
	//tButton->SetStringMode(iString,false);
//	tButton->GetColor(GuiRect::BACKGROUND)+=(this->GetChilds().size()%2)==0 ? 0x101010 : 0x151515;
//	tButton->GetColor(GuiRect::HOVERING)+=0x202020;
	tButton->func=GuiListBoxHelpers::OnItemPressed;
	tButton->param=tButton;
	tButton->SetUserData(this);
	tButton->Spot(0,0);
	tButton->Alignment(0,0);
	//tButton->margins.x=10;

	this->Append(tButton);
}*/


void GuiListBoxHelpers::OnItemPressed(void* iData)
{
	GuiButton*	tButton=(GuiButton*)iData;
	GuiListBox* tListBox=(GuiListBox*)tButton->GetUserData();
}

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////GuiComboBox////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiComboBox::GuiComboBox():selectedItem(0),popup(0)
{
	this->Append(&this->string);
	this->Append(&this->button);
}

GuiComboBox::~GuiComboBox()
{
}

void GuiComboBox::OnControlEvent(GUIMSGDEF)
{
	GuiRect::OnControlEvent(GUIMSGCALL);

	if(iMsg.sender==&this->button && iMsg.senderFunc==ONMOUSEUP)
	{

	}
}

/*
void pfdGuiComboBoxLabelsButtonAction(void* iData)
{
	GuiButton*		tButton=(GuiButton*)iData;
	GuiComboBox*	tComboBox=(GuiComboBox*)tButton->GetUserData();

	tComboBox->string->Text(tButton->Text());

	tComboBox->GetRoot()->GetTab()->SetDraw(tComboBox->string);

	Ide::Instance()->GetPopup()->windowData->Show(false);
}*/


void GuiComboBox::CretePopupList()
{
	if(!this->popup)
	{
		vec4 tEdges=this->Edges();

		vec2 tPopupSize(tEdges.z-tEdges.x,this->items.size()*GuiRect::ROW_HEIGHT);
		vec2 tTabPos=this->GetRoot()->GetTab()->windowData->Pos();

		tTabPos.x+=tEdges.x;
		tTabPos.y+=tEdges.w;

		this->popup=Ide::Instance()->mainAppWindow->mainContainer->CreateModalTab(tTabPos.x,tTabPos.y,tPopupSize.x,tPopupSize.y<300.0f?tPopupSize.y:300.0f);
		this->popup->hasFrame=false;
	}

	this->popup->rects.Append(&this->listbox);

	this->popup->OnGuiActivate();
	this->popup->OnGuiSize();

	this->popup->SetDraw();
}

void GuiComboBox::RecreateList()
{
	/*if(!this->list)
		this->list=new GuiRect;
	else
		this->list->Destroy();

	if(this->items.size())
	{
		this->string->Text(this->items[0]);

		for(size_t i=0;i<this->items.size();i++)
		{
			GuiButton* tButton=new GuiButton;
//			//tButton->GetAutoEdges()->ref.y=this->list->GetChilds().size() ? &this->list->GetChilds().back()->GetEdges().w : 0;
//			//tButton->GetAutoEdges()->fix.w=GuiRect::ROW_HEIGHT;
			tButton->Text(this->items[i]);
//			tButton->GetColor(GuiRect::BACKGROUND)+=(i%2)==0 ? 0x101010 : 0x151515;
//			tButton->GetColor(GuiRect::HOVERING)+=0x202020;
			tButton->SetUserData(this);
			tButton->Spot(0,0);
			tButton->Alignment(0,0);
			//tButton->margins.x=10;

			this->list->Append(tButton);
		}
	}*/
}



///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////GuiImage///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiImage::GuiImage():image(0){}
GuiImage::GuiImage(unsigned char* iRefData,float iWidth,float iHeight):image(new PictureRef(iRefData,iWidth,iHeight)){}
GuiImage::GuiImage(String iFilename):image(new PictureFile(iFilename)){}

GuiImage::~GuiImage()
{
	SAFEDELETE(this->image);
}

void GuiImage::OnPaint(GUIMSGDEF)
{
	GuiRect::OnPaint(GUIMSGCALL);

	if(this->image && this->image->handle)
		iTab->renderer2D->DrawBitmap(this->image,this->Edges().x,this->Edges().y,this->Edges().z,this->Edges().w);

	//this->BroadcastToChilds(&GuiRect::OnPaint,iMsg);
	//this->EndClip(GUIMSGCALL);
}

void GuiImage::OnActivate(GUIMSGDEF)
{
	GuiRect::OnActivate(GUIMSGCALL);

	if(this->image && this->image->handle)
	{
		bool tErrorLoading=iTab->renderer2D->LoadBitmap(this->image);

		if(tErrorLoading)
			wprintf(L"error loading image (rect %p ,image %p\n",this,this->image);
	}
}

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////GuiTextBox/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
GuiTextBox::GuiTextBox():cursor(0){}

GuiTextBox::~GuiTextBox(){SAFEDELETE(this->cursor);}


bool GuiTextBox::ParseKeyInput(GUIMSGDEF)
{
	StringEditor&	tStringEditor=*Ide::Instance()->stringEditor;

	bool			tRedraw=false;

	if(tStringEditor.Binded()==this && tStringEditor.Enabled())
	{
		unsigned int	tCaretOperation=tStringEditor.CARET_DONTCARE;
		void*			tCaretParameter=0;

		if(iMsg.key)
		{
			switch(iMsg.key)
			{
			case 0x08:/*VK_BACK*/tCaretOperation=tStringEditor.CARET_BACKSPACE; break;
			default:
				tCaretOperation=tStringEditor.CARET_ADD;
				tCaretParameter=(char*)iMsg.key;
			}
		}
		else
		{
			if(InputManager::keyboardInput.IsPressed(0x25/*VK_LEFT*/))
				tCaretOperation=tStringEditor.CARET_ARROWLEFT;
			if(InputManager::keyboardInput.IsPressed(0x27/*VK_RIGHT*/))
				tCaretOperation=tStringEditor.CARET_ARROWRIGHT;
			if(InputManager::keyboardInput.IsPressed(0x26/*VK_UP*/))
				tCaretOperation=tStringEditor.CARET_ARROWUP;
			if(InputManager::keyboardInput.IsPressed(0x28/*VK_DOWN*/))
				tCaretOperation=tStringEditor.CARET_ARROWDOWN;
			if(InputManager::keyboardInput.IsPressed(0x03/*VK_CANCEL*/))
				tCaretOperation=tStringEditor.CARET_CANCEL;
			if(InputManager::keyboardInput.IsPressed(0x2E/*VK_DELETE*/))
				tCaretOperation=tStringEditor.CARET_CANCEL;
			if(InputManager::keyboardInput.IsPressed(0x1B/*VK_ESCAPE*/)) 
			{
				tStringEditor.Enable(false);
				return false;
			}
		}

		tRedraw=true;

		bool tMustResize=tStringEditor.EditText(tCaretOperation,tCaretParameter);
	}

	return tRedraw;
}

void GuiTextBox::OnKeyDown(GUIMSGDEF)
{
	if(this==iTab->GetFocus())
	{
		StringEditor& tStringEditor=*Ide::Instance()->stringEditor;

		if(InputManager::keyboardInput.IsPressed(0x71/*VK_F2*/))
		{
			tStringEditor.Bind(this);
			tStringEditor.Enable(true);
		}
		else if(InputManager::keyboardInput.IsPressed(0x1B/*VK_ESCAPE*/))
		{
			tStringEditor.Bind(0);
			tStringEditor.Enable(false);

		}

		bool tRedraw=false;

		if(tStringEditor.Binded()==this && tStringEditor.Enabled())
			tRedraw=this->ParseKeyInput(GUIMSGCALL);

		if(tRedraw)
			iTab->SetDraw(this);
	}

	GuiRect::OnKeyDown(GUIMSGCALL);
}


void GuiTextBox::OnMouseDown(GUIMSGDEF)
{
	iTab->SetFocus(this);

	GuiRect::OnMouseDown(GUIMSGCALL);
}

void GuiTextBox::OnPaint(GUIMSGDEF)
{
}



///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////GuiButton//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiButton::GuiButton()
{
	this->SetFlag(FLAGS,DRAWHOVER,true);
	this->SetFlag(FLAGS,DRAWPRESS,true);

	this->Spot(0.5f,0.5f);
	this->Alignment(0.5f,0.5f);
}

void GuiButton::OnMouseUp(GUIMSGDEF)
{
	GuiString::OnMouseUp(GUIMSGCALL);

	this->GetParent()->OnControlEvent(iTab,Msg(this,ONMOUSEUP));
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////GuiCheckButton//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiCheckButton::GuiCheckButton()
{
	this->SetFlag(FLAGS,DRAWCHECK,true);
}

GuiCheckButton::~GuiCheckButton(){}

void GuiCheckButton::OnMouseUp(GUIMSGDEF)
{
	GuiString::OnMouseUp(GUIMSGCALL);

	this->GetParent()->OnControlEvent(iTab,Msg(this,ONMOUSEUP));
}



///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////GuiPath////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

namespace GuiPathHelpers
{
	void OnSelectDirectoryButtonPressed(void* iData);
}

GuiPath::GuiPath()
{
	this->Append(&this->button);
	this->Append(&this->path);
}

GuiPath::~GuiPath()
{

}

void GuiPathHelpers::OnSelectDirectoryButtonPressed(void* iData)
{
	GuiPath* tGuiPropertyPath=(GuiPath*)iData;

	Tab* iTab=tGuiPropertyPath->GetRoot()->GetTab();

	iTab->isModal ? iTab->windowData->Enable(false) : Ide::Instance()->mainAppWindow->mainContainer->windowData->Enable(false);
	iTab->SetDraw();

	String tDirectory=Ide::Instance()->subsystem->DirectoryChooser(L"",L"");

	if(tDirectory.size())
		tGuiPropertyPath->path.Text(tDirectory);

	iTab->isModal ? iTab->windowData->Enable(true) : Ide::Instance()->mainAppWindow->mainContainer->windowData->Enable(true);
	iTab->SetDraw();

	if(tGuiPropertyPath->func)
		tGuiPropertyPath->func(tGuiPropertyPath->param);
}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////GuiSlider///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiSlider::GuiSlider(float* iRef,float* iMin,float* iMax):referenceValue(iRef),minimum(iMin),maximum(iMax)
{
}

void GuiSlider::SetPosition(Tab* iTab,const vec2& iMouse)
{
	if(this->minimum && this->maximum && this->referenceValue)
	{
		vec4  tEdges=this->Edges();
		float tWidth=tEdges.z-tEdges.x;
		float tHeight=tEdges.w-tEdges.y;
		float tRunLeft=tEdges.x+10;
		float tRunRight=tEdges.z-10;
		float tTipUp=tEdges.y+tHeight/2.0f-5;
		float tTipBottom=tEdges.y+tHeight/2.0f+5;

		if(iMouse.x > tRunLeft && iMouse.x < tRunRight)
		{
			float f1=(iMouse.x-(tRunLeft))/(tWidth-20);
			float f2=*maximum-*minimum;
			float cursor=f1*f2;

			cursor = cursor<*minimum ? *minimum : (cursor>*maximum ? *maximum : cursor);

			if(*referenceValue!=cursor)
			{
				*referenceValue=cursor;
				iTab->SetDraw(this);
			}
		}
	}
}

float GuiSlider::Value(){return *this->referenceValue;}
float GuiSlider::Min(){return *this->minimum;}
float GuiSlider::Max(){return *this->maximum;}

void GuiSlider::OnPaint(GUIMSGDEF)
{
	GuiRect::OnPaint(GUIMSGCALL);

	if(this->minimum && this->maximum && this->referenceValue)
	{
		vec4 tEdges=this->Edges();
		float tWidth=tEdges.z-tEdges.x;
		float tHeight=tEdges.w-tEdges.y;

		iTab->renderer2D->DrawRectangle(tEdges.x+10,tEdges.y+tHeight/2.0f-2,tEdges.z-10,tEdges.y+tHeight/2.0f+2,0x000000);

		float tMinimum=*this->minimum;

		String smin(StringUtils::Float(*this->minimum));
		String smax(StringUtils::Float(*this->maximum));
		String value(StringUtils::Float(*this->referenceValue));

		iTab->renderer2D->DrawText(smin,tEdges.x+10,tEdges.y,tEdges.x+tWidth/2,tEdges.y+10,vec2(0,0.5f),vec2(0,0.5f),GuiString::COLOR_TEXT);
		iTab->renderer2D->DrawText(smax,tEdges.x+tWidth/2,tEdges.y,tEdges.z-10,tEdges.y+10,vec2(1,0.5f),vec2(1.0f,0.5f),GuiString::COLOR_TEXT);
		iTab->renderer2D->DrawText(value,tEdges.x,tEdges.y+20,tEdges.z,tEdges.w,vec2(0.5f,0.5f),vec2(0.5f,0.5f),GuiString::COLOR_TEXT);

		float tip=(tEdges.x+10) + ((*referenceValue)/(*maximum-*minimum))*(tWidth-20);

		iTab->renderer2D->DrawRectangle(tip-5,tEdges.y+tHeight/2.0f-5,tip+5,tEdges.y+tHeight/2.0f+5,GuiString::COLOR_TEXT);
	}
}

void GuiSlider::OnMouseMove(GUIMSGDEF)
{
	GuiRect::OnMouseMove(GUIMSGCALL);

	if(this->IsPressing())
		this->SetPosition(iTab,iMsg.mouse);
}

void GuiSlider::OnMouseDown(GUIMSGDEF)
{
	GuiRect::OnMouseDown(GUIMSGCALL);

	if(iMsg.button==1)
		this->SetPosition(iTab,iMsg.mouse);
}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////GuiAnimationController//////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiAnimationController::GuiAnimationController(AnimationController& iAnimationController):
	animationController(iAnimationController),
	slider(&animationController.cursor,&this->animationController.start,&this->animationController.end)
{
	this->play.Text(L"Play");
	this->stop.Text(L"Stop");

	this->slider.Activate(true);
	this->stop.Activate(true);
	this->play.Activate(true);

	this->Append(&this->slider);
	this->Append(&this->play);
	this->Append(&this->stop);
}

void GuiAnimationController::OnSize(GUIMSGDEF)
{
	GuiRect::OnSize(GUIMSGCALL);

	vec4 tEdges=this->Edges();
	float tHalfWidth=tEdges.x+(tEdges.z-tEdges.x)/2.0f;

	vec4 tSliderEdges=tEdges;
	vec4 tStopEdges=tEdges;
	vec4 tPlayEdges=tEdges;

	tStopEdges.y=tPlayEdges.y=tSliderEdges.w=tEdges.y+30;
	tStopEdges.x=tPlayEdges.z=tHalfWidth;
	tStopEdges.w=tPlayEdges.w=tEdges.w;

	this->slider.Edges(tSliderEdges);
	this->stop.Edges(tStopEdges);
	this->play.Edges(tPlayEdges);
}

void GuiAnimationController::OnControlEvent(GUIMSGDEF)
{
	GuiRect::OnControlEvent(GUIMSGCALL);

	if(iMsg.sender==&this->slider)
	{
		if(iMsg.senderFunc==ONMOUSEMOVE)
		{
			float value=this->slider.Value();

			if(value!=this->slider.Value() && this->slider.IsPressing())
			{
				this->animationController.SetFrame(this->slider.Value());
				iTab->SetDraw(this);
			}
		}
	}
	else if(iMsg.sender==&this->play)
	{
		if(iMsg.senderFunc==ONMOUSEUP)
		{
			this->animationController.Play();
		}
	}
	else if(iMsg.sender==&this->stop)
	{
		if(iMsg.senderFunc==ONMOUSEUP)
		{
			this->animationController.Stop();
		}
	}

}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////GuiViewport/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

void launchStopGuiViewportCallback(void* iData)
{
	GuiViewport* guiViewport=(GuiViewport*)iData;
}

#define GUIVIEWPORTINITLIST \
	renderBuffer(0), \
	needsPicking(0), \
	pickedEntity(0), \
	renderInstance(0), \
	renderFps(60), \
	renderBitmap(0)

#define GUIVIEWPORTINITFUNC \
	GlobalViewports().push_back(this); \
	this->playStopButton.Text(L"Run"); \
	this->playStopButton.Activate(true); \
	this->SetName(L"Viewport"); \
	//this->Append(&this->playStopButton); \

GuiViewport::GuiViewport():GUIVIEWPORTINITLIST{GUIVIEWPORTINITFUNC;}

GuiViewport::GuiViewport(vec3 pos,vec3 target,vec3 up,bool perspective):GUIVIEWPORTINITLIST
{
	GUIVIEWPORTINITFUNC;

	this->projection= !perspective ? this->projection : this->projection.perspective(90,16/9,1,1000);
	this->view.move(pos);
	this->view.lookat(target,up);
}

mat4& GuiViewport::Projection(){return this->projection;}
mat4& GuiViewport::View(){return this->view;}
mat4& GuiViewport::Model(){return this->model;}

EditorEntity* GuiViewport::GetHoveredEntity(){return 0;}
EditorEntity* GuiViewport::GetPickedEntity(){return this->pickedEntity;}

void GuiViewport::SetFps(unsigned int iFps){this->renderFps=iFps;}
unsigned int GuiViewport::GetFps(){return this->renderFps;}

std::list<GuiViewport*>& GuiViewport::Pool()
{
	return GlobalViewports();
}

void GuiViewport::OnPaint(GUIMSGDEF)
{
#if ENABLE_RENDERER

	if(Timer::GetInstance()->GetCurrent()-this->lastFrameTime>(1000.0f/this->renderFps))
	{
		this->lastFrameTime=Ide::Instance()->timer->GetCurrent();
		this->Render(iTab);
	}
	else
		this->DrawBuffer(iTab,this->Edges());
#endif
}

void GuiViewport::OnMouseWheel(GUIMSGDEF)
{
	GuiRect::OnMouseWheel(GUIMSGCALL);

	float factor=*(float*)iMsg.data;

	this->view*=mat4().translate(0,0,factor*10);
}

void GuiViewport::OnMouseUp(GUIMSGDEF)
{
	if(this->IsHovering() && this->pickedEntity)
		Ide::Instance()->mainAppWindow->mainContainer->Broadcast(GuiRect::Funcs::ONENTITYSELECTED,Msg(this->pickedEntity));

	GuiRect::OnMouseUp(GUIMSGCALL);
}

void GuiViewport::OnMouseMove(GUIMSGDEF)
{
	GuiRect::OnMouseMove(GUIMSGCALL);

	if(this->IsHovering())
	{
		if(InputManager::keyboardInput.IsPressed(0x01/*VK_LBUTTON*/))
		{
			float dX=(iMsg.mouse.x-this->mouseold.x);
			float dY=(iMsg.mouse.y-this->mouseold.y);

			if(InputManager::keyboardInput.IsPressed(0x011/*VK_CONTROL*/))
			{
				mat4 mview;
				vec3 vx,vy,vz;
				vec3 pos;
				mat4 rot;

				mview=this->view;

				mview.traspose();
				mview.inverse();

				mview.axes(vx,vy,vz);

				pos=this->model.position();

				this->model.move(vec3());

				if(dY)
					rot.rotate(dY,vx);
				this->model.rotate(dX,0,0,1);

				this->model*=rot;

				this->model.move(pos);
			}
			else
			{
				this->view*=mat4().translate(dX,dY,0);
			}
		}

		this->needsPicking=true;

		mouseold=iMsg.mouse;
	}
}


void GuiViewport::OnActivate(GUIMSGDEF)
{
	GuiRect::OnActivate(GUIMSGCALL);
#if ENABLE_RENDERER
	iTab->Create3DRenderer();
	iTab->OnGuiSize();

	if(!this->renderInstance)
		this->renderInstance=iTab->SetDraw(this,false);
#endif
}
void GuiViewport::OnDeactivate(GUIMSGDEF)
{
	GuiRect::OnDeactivate(GUIMSGCALL);
#if ENABLE_RENDERER
	if(this->renderInstance)
		this->renderInstance->remove=true;
#endif
}


GuiSceneViewport::GuiSceneViewport(vec3 pos,vec3 target,vec3 up,bool perspective):GuiViewport(pos,target,up,perspective)
{
	this->SetName(L"SceneView");
}

EditorEntity* GuiSceneViewport::Entity(){return GuiSceneViewer::Instance()->Entity();}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////GuiScrollBar////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiScrollBar::GuiScrollBar(GuiScrollRect* iRect,Type iScrollbarType):
	rect(iRect),
	type(iScrollbarType),
	scrollerPressed(-1),
	position(0)
{}

GuiScrollBar::~GuiScrollBar(){}

GuiScrollBar::Type GuiScrollBar::GetType(){return this->type;}

bool GuiScrollBar::Contains(const vec2& iPoint)
{
	vec4 tEdges=this->Edges();

	return (iPoint.x>tEdges.x && iPoint.x<=tEdges.z && iPoint.y>tEdges.y && iPoint.y<=tEdges.w);
}
vec4  GuiScrollBar::Edges()
{
	vec4 tEdges=this->rect->Edges();

	switch(this->type)
	{
		case HORIZONTAL: 
			tEdges.y=tEdges.w-GuiScrollBar::TICK;
			tEdges.z-=this->rect->VBar()->IsActive() ? GuiScrollBar::TICK :0;
		break;
		case VERTICAL: 
			tEdges.x=tEdges.z-GuiScrollBar::TICK;
			tEdges.w-=this->rect->HBar()->IsActive() ? GuiScrollBar::TICK : 0;
		break;
	}

	return tEdges;
}

void GuiScrollBar::SetPosition(float iPositionPercent)
{
	float oldScrollerPosition=this->position;

	vec4 tEdges=this->Edges();

	float tScrollerContainerLength=this->GetContainerLength(tEdges);
	float tScrollerLength=this->GetScrollerLength(tEdges);
	float tRatio=this->Ratio();

	if((iPositionPercent+tRatio)>1)
		this->position=(tScrollerContainerLength-tScrollerLength)/tScrollerContainerLength;
	else
		this->position = iPositionPercent < 0 ? 0 : iPositionPercent;
}

void GuiScrollBar::Scroll(Tab* tabContainer,float upOrDown)
{
	float rowHeightRatio=this->Ratio()/GuiScrollBar::TICK;

	float amount=this->position + (upOrDown<0 ? rowHeightRatio : -rowHeightRatio);

	this->SetPosition(amount);
}



float GuiScrollBar::GetContainerLength(const vec4& iEdges){return	(this->type==VERTICAL ? (iEdges.w-iEdges.y) : (iEdges.z-iEdges.x)) - (2.0f*TICK);}
float GuiScrollBar::GetScrollerBegin(const vec4& iEdges){return this->GetContainerBegin(iEdges)+this->position*this->GetContainerLength(iEdges);}
float GuiScrollBar::GetScrollerEnd(const vec4& iEdges){return this->GetScrollerBegin(iEdges)+this->Ratio()*this->GetContainerLength(iEdges);}
float GuiScrollBar::GetScrollerLength(const vec4& iEdges){return this->GetScrollerEnd(iEdges)-this->GetScrollerBegin(iEdges);}
float GuiScrollBar::GetContainerBegin(const vec4& iEdges){return	(this->type==VERTICAL ? iEdges.y : iEdges.x) + TICK;}
float GuiScrollBar::GetContainerEnd(const vec4& iEdges){return	(this->type==VERTICAL ? iEdges.w : iEdges.z) - TICK;}
float GuiScrollBar::GetPosition(){return this->position*(this->type==VERTICAL ? this->rect->GetContent().y : this->rect->GetContent().x);}

float GuiScrollBar::Ratio()
{
	vec4 tEdges=this->rect->EdgeClip();
	vec2 tDim(tEdges.z-tEdges.x,tEdges.w-tEdges.y);
	vec2 tMaxes=this->rect->Maxes();

	float tRatio = tDim[this->type]/tMaxes[this->type];

	return tRatio;
}


void GuiScrollBar::OnMouseUp(GUIMSGDEF)
{
	GuiRect::OnMouseUp(GUIMSGCALL);

	if(iMsg.button==1)
		this->scrollerPressed=-1;
}


void GuiScrollBar::OnMouseDown(GUIMSGDEF)
{
	GuiRect::OnMouseDown(GUIMSGCALL);

	if(iMsg.button==1)
	{
		vec4 tEdges=this->Edges();

		float& tTreshold=this->type==VERTICAL ? iMsg.mouse.y : iMsg.mouse.x;

		float tScrollerBegin=this->GetScrollerBegin(tEdges);
		float tScrollerEnd=this->GetScrollerEnd(tEdges);
		float tScrollerLength=this->GetScrollerLength(tEdges);
		float tContainerBegin=this->GetContainerBegin(tEdges);
		float tContainerEnd=this->GetContainerEnd(tEdges);
		float tContainerLength=this->GetContainerLength(tEdges);

		if(tTreshold<tContainerBegin)
		{
			this->Scroll(iTab,1);
		}
		else if(tTreshold<tContainerEnd)
		{
			this->scrollerPressed=0;

			if(tTreshold>=tScrollerBegin && tTreshold<=tScrollerEnd)
				this->scrollerPressed=((tTreshold-tScrollerBegin)/tScrollerLength)*this->Ratio();
			else
				SetPosition((tTreshold-tContainerBegin)/tContainerLength);
		}
		else
		{
			this->Scroll(iTab,-1);
		}

		iTab->SetDraw(this->rect);
	}
}

void GuiScrollBar::OnMouseMove(GUIMSGDEF)
{
	if(this->IsPressing())
	{
		float& tTreshold=this->type==VERTICAL ? iMsg.mouse.y : iMsg.mouse.x;

		vec4 tEdges=this->Edges();

		float tContainerBegin=this->GetContainerBegin(tEdges);
		float tContainerEnd=this->GetContainerEnd(tEdges);
		float tContainerLength=this->GetContainerLength(tEdges);

		if(tTreshold>tContainerBegin && tTreshold<tContainerEnd)
		{
			float mouseContainerTreshold=(tTreshold-tContainerBegin)/tContainerLength;

			this->SetPosition(mouseContainerTreshold-this->scrollerPressed);

			iTab->SetDraw(this->rect);
		}
	}
}

void GuiScrollBar::OnPaint(GUIMSGDEF)
{
	vec4 e=this->Edges();

	float tContainerBegin=this->GetContainerBegin(e);
	float tScrollerBegin=this->GetScrollerBegin(e);
	float tContainerEnd=this->GetContainerEnd(e);
	float tScrollerEnd=this->GetScrollerEnd(e);
	float tScrollerLength=this->GetScrollerLength(e);

	if(this->type==VERTICAL)
	{
		iTab->renderer2D->DrawRectangle(e.x,e.y,e.x+TICK,e.y+TICK,GuiScrollBar::COLOR_BACK);
		iTab->renderer2D->DrawRectangle(e.x,e.y,e.x+TICK,e.y+TICK,0x00000000,false);
		iTab->renderer2D->DrawRectangle(e.x,e.w-TICK,e.x+TICK,e.w,GuiScrollBar::COLOR_BACK);
		iTab->renderer2D->DrawRectangle(e.x,e.w-TICK,e.x+TICK,e.w,0x00000000,false);

		iTab->renderer2D->DrawBitmap(iTab->iconUp,e.x,e.y,e.x+TICK,e.y+TICK);
		iTab->renderer2D->DrawBitmap(iTab->iconDown,e.x,e.w-TICK,e.x+TICK,e.w);

		iTab->renderer2D->DrawRectangle(e.x,tContainerBegin,e.x+TICK,tContainerEnd,GuiScrollBar::COLOR_BACK);
		iTab->renderer2D->DrawRectangle(e.x,tContainerBegin,e.x+TICK,tContainerEnd,0x00000000,false);

		iTab->renderer2D->DrawRectangle(e.x,tScrollerBegin,e.x+TICK,tScrollerEnd,0x00000000);
	}
	else
	{
		iTab->renderer2D->DrawRectangle(e.x,e.y,e.x+TICK,e.y+TICK,GuiScrollBar::COLOR_BACK);
		iTab->renderer2D->DrawRectangle(e.x,e.y,e.x+TICK,e.y+TICK,0x00000000,false);
		iTab->renderer2D->DrawRectangle(e.z-TICK,e.y,e.z,e.w,GuiScrollBar::COLOR_BACK);
		iTab->renderer2D->DrawRectangle(e.z-TICK,e.y,e.z,e.w,0x00000000,false);

		iTab->renderer2D->DrawBitmap(iTab->iconLeft,e.x,e.y,e.x+TICK,e.y+TICK);
		iTab->renderer2D->DrawBitmap(iTab->iconRight,e.z-TICK,e.y,e.z,e.w);

		iTab->renderer2D->DrawRectangle(tContainerBegin,e.y,tContainerEnd,e.y+TICK,GuiScrollBar::COLOR_BACK);
		iTab->renderer2D->DrawRectangle(tContainerBegin,e.y,tContainerEnd,e.y+TICK,0x00000000,false);

		iTab->renderer2D->DrawRectangle(tScrollerBegin,e.y,tScrollerEnd,e.y+TICK,0x00000000);
	}
}

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////GuiScrollRect////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiScrollRect::GuiScrollRect():hscroll(this,GuiScrollBar::HORIZONTAL),vscroll(this,GuiScrollBar::VERTICAL)
{
	this->hscroll.SetFlag(FLAGS,CHILD,false);
	this->vscroll.SetFlag(FLAGS,CHILD,false);
}


vec4 GuiScrollRect::EdgeClip()
{
	float	tLeft=this->left();
	float	tTop=this->top();
	float	tRight=this->right();
	float	tBottom=this->bottom();

	tRight -= this->vscroll.IsActive() ? GuiScrollBar::TICK : 0;
	tBottom -= this->hscroll.IsActive() ? GuiScrollBar::TICK : 0;

	return vec4(tLeft,tTop,tRight,tBottom);
}

vec2 GuiScrollRect::EdgeOffset()
{
	return vec2(-this->hscroll.GetPosition(),-this->vscroll.GetPosition());
}

vec2 GuiScrollRect::MouseClip(const vec2& iMouse)
{
	return iMouse;
}

void GuiScrollRect::Broadcast(Funcs iFunc,GUIMSGDEF)
{
	GuiRect::Broadcast(iFunc,GUIMSGCALL);

	iTab->BroadcastTo(this->HBar(),iFunc,iMsg);
	iTab->BroadcastTo(this->VBar(),iFunc,iMsg);
}

GuiScrollBar* GuiScrollRect::HBar(){return &this->hscroll;}
GuiScrollBar* GuiScrollRect::VBar(){return &this->vscroll;}

void GuiScrollRect::RecalcScrollbars()
{
	float	tLeft=this->left();
	float	tTop=this->top();
	float	tRight=this->right();
	float	tBottom=this->bottom();

	this->hscroll.Activate(this->content.x>(tRight-tLeft));
	tBottom -= this->hscroll.IsActive() ? GuiScrollBar::TICK : 0;
	this->vscroll.Activate(this->content.y>(tBottom-tTop));
	tRight -= this->vscroll.IsActive() ? GuiScrollBar::TICK : 0;
	this->hscroll.Activate(this->content.x>(tRight-tLeft));

	this->maxes.x=std::max<float>(tRight-tLeft,this->content.x);
	this->maxes.y=std::max<float>(tBottom-tTop,this->content.y);

	this->hscroll.SetPosition(this->hscroll.position);
	this->vscroll.SetPosition(this->vscroll.position);
}

void GuiScrollRect::OnSize(GUIMSGDEF)
{
	GuiRect::OnSize(GUIMSGCALL);

	this->RecalcScrollbars();
}

void GuiScrollRect::SetContent(float iHor,float iVer)
{
	this->content.x=std::max<float>(this->content.x,iHor);
	this->content.y=std::max<float>(this->content.y,iVer);

	this->RecalcScrollbars();
}

void GuiScrollRect::ResetContent()
{
	this->content.make(0,0);

	this->RecalcScrollbars();
}

const vec2& GuiScrollRect::GetContent(){return this->content;}

const vec2& GuiScrollRect::Maxes(){return this->maxes;}

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////////GuiPanel/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiPanel::GuiPanel()
{
	this->SetName(L"GuiPanel");
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////GuiSceneViewer/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiSceneViewer::GuiSceneViewer():entity(0){this->SetName(L"Scene");}
GuiSceneViewer* GuiSceneViewer::Instance()
{
	GLOBALGETTERFUNCASSINGLETON(GlobalGuiSceneViewerInstance,GuiSceneViewer);
}

EditorEntity*	GuiSceneViewer::Entity(){return this->entity;}
void			GuiSceneViewer::Entity(EditorEntity* iEntity){this->entity=iEntity;} 

void GuiSceneViewer::OnEntitiesChange(GUIMSGDEF)
{
	EditorEntity* newEntity=(EditorEntity*)iMsg.data;

	if(newEntity)
	{
		if(!this->entity)
			this->Append(newEntity->sceneLabel);

		this->entity=newEntity;
		
		iTab->BroadcastTo(this,ONSIZE);
		iTab->SetDraw(this);
	}
}

GuiSceneViewer::SceneLabel::SceneLabel()
{
	this->SetFlag(FLAGS,DRAWCHECK,true);
}

void			GuiSceneViewer::SceneLabel::Entity(EditorEntity* iEntity){this->entity=iEntity;}
EditorEntity*	GuiSceneViewer::SceneLabel::Entity(){return this->entity;}

void GuiSceneViewer::SceneLabel::OnMouseUp(GUIMSGDEF)
{
	Item::OnMouseUp(GUIMSGCALL);

	switch(iMsg.button)
	{
	case 3:
		{
			int tMenuItem=iTab->TrackGuiSceneViewerPopup(this);	

			EditorPropertiesBase*	tCreatedEditorObject=0;

			switch(tMenuItem)
			{
			case 1:
				{
					EditorEntity* tNewEntity=0;

					tCreatedEditorObject=tNewEntity=new EditorEntity;

					tNewEntity->name=L"__00000000000000000000000000000000000000000000000000000000000000000000000000000000000";

					this->entity->Append(tNewEntity);

					tNewEntity->OnPropertiesCreate();
				}
				break;
			case 2:
				//GuiSceneLabelHelpers::hovered->DestroyChilds();
				break;
			case 3:tCreatedEditorObject=this->entity->CreateComponent<EditorLight>();break;
			case 4:tCreatedEditorObject=this->entity->CreateComponent<EditorMesh>();break;
			case 5:tCreatedEditorObject=this->entity->CreateComponent<EditorCamera>();break;
			case 14:tCreatedEditorObject=this->entity->CreateComponent<EditorScript>();break;
			}

			if(tCreatedEditorObject)
				tCreatedEditorObject->OnResourcesCreate();

			if(tMenuItem)
				iTab->SetDraw(this->TreeView());
		}
		break;
	case 1:
		{
			if(!this->ExpandosContains(iMsg.mouse))
			{
				this->SetFlag(FLAGS,CHECKED,!this->GetFlag(FLAGS,CHECKED));

				for(std::list<GuiEntityViewer*>::const_iterator it=GuiEntityViewer::Pool().begin();it!=GuiEntityViewer::Pool().end();it++)
					(*it)->OnEntitySelected((Tab*)(*it)->GetRoot()->GetTab(),Msg(this->entity));
			}
		}
		break;
	}
}

void GuiSceneViewer::Save(String iFilename)
{
}

void GuiSceneViewer::Load(String iFilename)
{

}
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////GuiEntityViewer/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiEntityViewer::GuiEntityViewer():entity(0)
{
	this->SetName(L"Entity");
	GlobalGuiEntityViewerInstance().push_back(this);
}

GuiEntityViewer::~GuiEntityViewer()
{
	GlobalGuiEntityViewerInstance().remove(this);
}

std::list<GuiEntityViewer*>& GuiEntityViewer::Pool()
{
	return GlobalGuiEntityViewerInstance();
}

void			GuiEntityViewer::Entity(EditorEntity* iEntity){this->entity=iEntity;}
EditorEntity*	GuiEntityViewer::Entity(){return this->entity;}

void GuiEntityViewer::OnEntitySelected(GUIMSGDEF)
{
	EditorEntity* tReceivedEntity=(EditorEntity*)iMsg.data;

	if(this->Entity()!=tReceivedEntity)
	{
		if(this->Entity())
			this->Remove(&this->Entity()->PropertyContainer());

		if(tReceivedEntity)
		{
			this->Entity(tReceivedEntity);
			this->InsertContainer(tReceivedEntity->PropertyContainer());
		}
	}

	iTab->SetDraw(this);
}

void GuiEntityViewer::OnExpandos(GUIMSGDEF)
{
	//this->OnSize(GUIMSGCALL);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////GuiConsoleViewer////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
GuiConsoleViewer::GuiConsoleViewer(){}
GuiConsoleViewer::~GuiConsoleViewer(){}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////GuiProjectViewer////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

const int SPLITTER_SIZE=4;

GuiProjectViewer::GuiProjectViewer():
	projectDirectory(GlobalRootProjectDirectory()),
	splitterLeftActive(false),
	splitterRightActive(false),
	splitterLeft(100),
	splitterRight(200)
{
	this->SetName(L"Project");

	this->Color(0x303030);

	projectDirectory=new ResourceNodeDir;
	this->projectDirectory->fileName=Ide::Instance()->folderProject;
	this->projectDirectory->isDir=true;
	this->projectDirectory->dirLabel.SetName(this->projectDirectory->fileName);
	this->projectDirectory->fileLabel.SetName(this->projectDirectory->fileName);

	this->dirViewer.Color(GuiRect::COLOR_BACK);
	this->fileViewer.Color(GuiRect::COLOR_BACK);
	this->resViewer.Color(GuiRect::COLOR_BACK);

	this->Append(&this->dirViewer);
	this->Append(&this->fileViewer);
	this->Append(&this->resViewer);

	this->dirViewer.Activate(true);
	this->fileViewer.Activate(true);
	this->resViewer.Activate(true);
}

GuiProjectViewer::~GuiProjectViewer()
{
}

GuiProjectViewer* GuiProjectViewer::Instance()
{
	GLOBALGETTERFUNCASSINGLETON(GlobalGuiProjectViewerInstance,GuiProjectViewer);
}

void GuiProjectViewer::OnActivate(GUIMSGDEF)
{
	GuiRect::OnActivate(GUIMSGCALL);

	Ide::Instance()->ScanDir(this->projectDirectory->fileName,this->projectDirectory);
	this->dirViewer.Append(this->projectDirectory->dirLabel);
	this->fileViewer.Append(this->projectDirectory->fileLabel);
}

void GuiProjectViewer::OnDeactivate(GUIMSGDEF){GuiRect::OnDeactivate(GUIMSGCALL);}

void GuiProjectViewer::RefreshAll()
{
}

void GuiProjectViewer::OnMouseMove(GUIMSGDEF)
{
	GuiRect::OnMouseMove(GUIMSGCALL);

	iTab->SetCursor(1);

	if(this->IsPressing())
	{
		this->OnSize(GUIMSGCALL);

		if(this->splitterLeftActive)
		{
			this->splitterLeft=iMsg.mouse.x;
			this->dirViewer.OnSize(GUIMSGCALL);
		}

		this->fileViewer.OnSize(GUIMSGCALL);

		if(this->splitterRightActive)
		{
			this->splitterRight=iMsg.mouse.x;
			this->resViewer.OnSize(GUIMSGCALL);
		}

		iTab->SetDraw(this);
	}
}


void GuiProjectViewer::OnMouseDown(GUIMSGDEF)
{
	GuiRect::OnMouseDown(GUIMSGCALL);

	iTab->SetCursor(1);

	if(iMsg.mouse.x>=this->splitterLeft && iMsg.mouse.x<=this->splitterLeft+SPLITTER_SIZE)
		this->splitterLeftActive=true;
	else if(iMsg.mouse.x>=this->splitterRight && iMsg.mouse.x<=this->splitterRight+SPLITTER_SIZE)
		this->splitterRightActive=true;
}

void GuiProjectViewer::OnMouseUp(GUIMSGDEF)
{
	GuiRect::OnMouseUp(GUIMSGCALL);

	if(this->splitterLeftActive || this->splitterRightActive)
	{
		this->splitterLeftActive=false;
		this->splitterRightActive=false;
	}
}


void GuiProjectViewer::OnReparent(GUIMSGDEF)
{
}

void GuiProjectViewer::OnSize(GUIMSGDEF)
{
	this->dirViewer.Edges(this->Edges());
	this->fileViewer.Edges(this->Edges());
	this->resViewer.Edges(this->Edges());

	this->dirViewer.right(splitterLeft);
	this->fileViewer.left(splitterLeft+4);
	this->fileViewer.right(splitterRight);
	this->resViewer.left(splitterRight+4);


	/*float tWidth=this->Width();
	float tLeftPercent=0;
	float tRightPercent=0;

	if(tWidth)
	{
		tLeftPercent=this->GetWidth()/this->splitterLeft;
		tRightPercent=this->GetWidth()/this->splitterRight;
	}

	if(tLeftPercent && tRightPercent)
	{
		this->splitterLeft=this->GetWidth()*(1.0f/tLeftPercent);
		this->splitterRight=this->GetWidth()*(1.0f/tRightPercent);
	}*/
}


void GuiProjectViewer::Delete(Tab* iTab,ResourceNode* iNode)
{

}


void GuiProjectViewer::findResources(std::vector<ResourceNode*>& oResultArray,ResourceNode* iResourceNode,String iExtension)
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

std::vector<ResourceNode*> GuiProjectViewer::findResources(String iExtension)
{
	std::vector<ResourceNode*> oResultArray;

	this->findResources(oResultArray,this->projectDirectory,iExtension);

	return oResultArray;
}

////////////////////////////////////////////
////////////////////////////////////////////
/////////GuiProjectViewer::DirViewer////////
////////////////////////////////////////////
////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
///////////////GuiProjectViewer::FileViewer/////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void launchStopScriptEditorCallback(void* iData)
{
	EditorScript* editorScript=(EditorScript*)iData;

	if(editorScript->runtime)
	{
		if(Ide::Instance()->compiler->UnloadScript(editorScript))
			editorScript->buttonLaunch->Text(L"Launch");
	}
	else
	{
		if(Ide::Instance()->compiler->LoadScript(editorScript))
			editorScript->buttonLaunch->Text(L"Stop");
	}

	editorScript->PropertyContainer().GetRoot()->GetTab()->SetDraw(editorScript->buttonLaunch);
}

void GuiProjectViewer::FileViewer::FileLabel::OnMouseUp(GUIMSGDEF)
{
	Item::OnMouseUp(GUIMSGCALL);

	if(iMsg.button==3)
	{
		int menuResult=iTab->TrackProjectFileViewerPopup(this->resource);

		switch(menuResult)
		{
		case 1:
			{
				iTab->concurrentInstances.push_back(std::function<void()>(std::bind(&GuiProjectViewer::Delete,GuiProjectViewer::Instance(),iTab,this->resource)));
			}
			break;
		case 3://load
			if(this->resource->fileName.PointedExtension() == Ide::Instance()->GetSceneExtension())
			{
				if(GuiSceneViewer::Instance())
				{
					Tab* tTab=GuiSceneViewer::Instance()->GetRoot()->GetTab();

					iTab->DrawBlock(true);
					tTab->DrawBlock(true);

					String tHoveredNodeFilename=this->resource->BuildPath();

					GuiSceneViewer::Instance()->Load(tHoveredNodeFilename.c_str());
					tTab->SetDraw(GuiSceneViewer::Instance());

					tTab->DrawBlock(false);
					iTab->DrawBlock(false);
				}
			}
			break;
		case 4://load
			if(this->resource->fileName.Extension() == L"cpp")
			{
				launchStopScriptEditorCallback(0);
			}
			break;
		}
	}

}

/////////////////////////////////////////
/////////////////////////////////////////
///////GuiProjectViewer::DataViewer//////
/////////////////////////////////////////
/////////////////////////////////////////

GuiProjectViewer::DataViewer::DataViewer()
{
}

//////////////////////////////////////
//////////////////////////////////////
///////////////GuiImage///////////////
//////////////////////////////////////
//////////////////////////////////////

Picture::Picture():
handle(0),
	width(0),
	height(0),
	bpp(0)
{}

Picture::~Picture()
{
	this->Release();
	this->width=-1;
	this->height=-1;
	this->bpp=-1;
}

PictureRef::PictureRef(unsigned char* iRefData,float iWidth,float iHeight):refData(iRefData)
{
	this->width=iWidth;
	this->height=iHeight;
}

PictureFile::PictureFile(String iFilename):fileName(iFilename){}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////DrawInstance/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


DrawInstance::DrawInstance(GuiRect* iRect,bool iRemove,bool iSkip):rect(iRect),remove(iRemove),skip(iSkip){}

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////GuiPaper///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

void GuiScriptViewer::DrawBreakpoints(Tab* tabContainer)
{
	float tFontHeight=GuiFont::GetDefaultFont()->GetHeight();

	std::vector<Debugger::Breakpoint>& breakpoints=Ide::Instance()->debugger->breakpointSet;

	for(size_t i=0;i<breakpoints.size();i++)
	{
		if(breakpoints[i].script==this->script)
		{
			unsigned int tLineInsertion=this->Edges().y + (breakpoints[i].line - 1) * tFontHeight;

			unsigned int tBreakColor = breakpoints[i].breaked ? 0xff0000 : 0xffff00 ;

			tabContainer->renderer2D->DrawRectangle(this->Edges().x + 1,tLineInsertion + 1,this->Edges().x/*+this->editor->margins.x -1*/,tLineInsertion + tFontHeight - 1,tBreakColor,true);
		}
	}
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////GuiScriptViewer/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
std::list<GuiScriptViewer*>& GuiScriptViewer::GetInstances()
{
	return GlobalScriptViewers();
}


GuiScriptViewer::GuiScriptViewer():
	script(0),
	lineNumbers(true)
{
	this->GetInstances().push_back(this);

	this->SetName(L"ScriptViewer");

	this->editor=new GuiTextBox;
	this->lines=new GuiString;

	this->editor->Font(GuiFont::GetFontPool()[3]);
//	this->GetAutoEdges()->off.make(0,0,0,0);

	this->lines->Alignment(0,0);
	this->lines->Spot(0,0);

	this->Append(this->lines);
	this->Append(this->editor);

//	this->lines->GetAutoEdges()->ref.make(0,0,&this->lines->GetEdges().x,0);
//	this->lines->GetAutoEdges()->off.x=0;

//	this->editor->GetAutoEdges()->ref.make(&this->lines->GetEdges().z,0,0,0);

}

GuiScriptViewer::~GuiScriptViewer()
{
	this->GetInstances().remove(this);
}

void GuiScriptViewer::Open(Script* iScript)
{
	this->script=(EditorScript*)iScript;

	this->editor->Text(this->script->LoadScript());

	this->script->scriptViewer=this;
}

bool GuiScriptViewer::Save()
{
	if(this->script)
		this->script->SaveScript(this->editor->Text());

	return false;
}


bool GuiScriptViewer::Compile()
{
	bool exited=false;
	bool compiled=false;
	bool runned=false;

	if(this->script)
	{
		exited=Ide::Instance()->compiler->UnloadScript(this->script);

		compiled=Ide::Instance()->compiler->Compile(this->script);

		runned=Ide::Instance()->compiler->LoadScript(this->script);
	}

	return exited && compiled && runned;
}



void GuiScriptViewer::OnKeyDown(GUIMSGDEF)
{
	if(this->script)
	{
		if(InputManager::keyboardInput.IsPressed(0x11/*VK_CONTROL*/) && !InputManager::keyboardInput.IsPressed(0x12/*VK_ALT*/))
		{
			if(InputManager::keyboardInput.IsPressed('S'))
				this->Save();
		}
	}

	GuiRect::OnKeyDown(GUIMSGCALL);
}

void GuiScriptViewer::OnKeyUp(GUIMSGDEF)
{
	GuiRect::OnKeyUp(GUIMSGCALL);
}

void GuiScriptViewer::OnLMouseDown(GUIMSGDEF)
{
	GuiRect::OnMouseDown(GUIMSGCALL);

	/*if(iMsg.mouse.x < this->editor->margins.x)
	{
		unsigned int tBreakOnLine=(iMsg.mouse.y-this->GetEdges().y)/this->editor->font->GetHeight() + 1;

		EditorScript* tEditorScript=(EditorScript*)this->script;

		std::vector<Debugger::Breakpoint>& tAvailableBreakpoints=Ide::Instance()->debugger->allAvailableBreakpoints;
		std::vector<Debugger::Breakpoint>& tBreakpoints=Ide::Instance()->debugger->breakpointSet;

		for(size_t i=0;i<tAvailableBreakpoints.size();i++)
		{
			if(tAvailableBreakpoints[i].script==this->script && tAvailableBreakpoints[i].line==tBreakOnLine)
			{
				std::vector<Debugger::Breakpoint>::iterator tFoundedBreakpointIterator=std::find(tBreakpoints.begin(),tBreakpoints.end(),tAvailableBreakpoints[i]);

				bool tAdd=tBreakpoints.end()==tFoundedBreakpointIterator;

				if(tAdd)
					tBreakpoints.push_back(tAvailableBreakpoints[i]);
				else
					tBreakpoints.erase(tFoundedBreakpointIterator);

				Ide::Instance()->debugger->SetBreakpoint(tAvailableBreakpoints[i],tAdd);

				iTab->SetDraw(this);

				break;
			}
		}
	}*/

	iTab->SetFocus(this->editor);
}

void GuiScriptViewer::OnDeactivate(GUIMSGDEF)
{
	iTab->SetFocus(0);

	GuiRect::OnDeactivate(GUIMSGCALL);
}

void GuiScriptViewer::OnMouseMove(GUIMSGDEF)
{
	GuiRect::OnMouseMove(GUIMSGCALL);

	if(this->IsHovering())
	{
		/*if(iMsg.mouse.x < this->editor->margins.x)
		{

		}
		else
		{

		}*/
	}
}

void GuiScriptViewer::OnSize(GUIMSGDEF)
{
	GuiRect::OnSize(GUIMSGCALL);

	this->lineCount=this->CountScriptLines()-1;
}


int GuiScriptViewer::CountScriptLines()
{
	const wchar_t* t=this->editor->Text().c_str();

	int tLinesCount=!(*t) ? 0 : 1;

	while(*t)
	{
		if(*t=='\n' || *t=='\r')
			tLinesCount++;

		t++;
	}

	return tLinesCount;
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////GuiCompilerViewer///////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiCompilerViewer* GuiCompilerViewer::Instance()
{
	GLOBALGETTERFUNCASSINGLETON(GlobalGuiCompilerViewerInstance,GuiCompilerViewer);
}

GuiCompilerViewer::GuiCompilerViewer(){this->SetName(L"CompilerViewer");}

const wchar_t* nthOccurrenceInLine(const wchar_t* iStr,char iChar,int iNth)
{
	const wchar_t* str=iStr;
	int occurr=0;

	char c=*str;

	while(c!='\0' || c!='\n')
	{
		if(c==iChar)
		{
			if(++occurr==iNth)
				return str;
		}

		c=*(++str);
	}

	return 0;
}


bool GuiCompilerViewer::ParseCompilerOutputFile(String iFileBuffer)
{
	bool bReturnValue=true;

	//this->RemoveChild(&this->vScrollbar);

	this->Destroy();

	GuiRect* messagesGlue=new GuiRect;
	this->Append(messagesGlue);

	const int MESSAHE_ROW_HEIGHT=20;

	const wchar_t* fileBuffer=iFileBuffer.c_str();

	if(fileBuffer)
	{
		const wchar_t* LineBegin=fileBuffer;
		const wchar_t* FileEnd=LineBegin;
		while(*FileEnd++);

		while(LineBegin!=FileEnd-1)
		{
			const wchar_t* LineEnd=wcschr(LineBegin,'\n');

			const wchar_t* compileError=wcsstr(LineBegin,L"error C");
			const wchar_t* linkError=wcsstr(LineBegin,L"error LNK");

			compileError!=0 && LineEnd < compileError ? compileError=0 : 0;
			linkError!=0 && LineEnd < linkError ? linkError=0 : 0;

			bool simpleMessage=!compileError && !linkError;

			GuiString* tCompilerMessageRow=new GuiString;

			{
				/*tCompilerMessageRow->fixed.w=MESSAHE_ROW_HEIGHT;
				tCompilerMessageRow->alignRect.y=-1;
				tCompilerMessageRow->alignText.make(-1,0.5f);*/
			}

			if(simpleMessage)
			{
				tCompilerMessageRow->Text(String(LineBegin,LineEnd-LineBegin));
			}
			else
			{
				bReturnValue=false;

				const wchar_t* FileEnd=nthOccurrenceInLine(LineBegin,':', compileError ? 2 : 1);//the first : should be the volume drive

				if(FileEnd)
					FileEnd+=2;

				const wchar_t* ErrorEnd=nthOccurrenceInLine(FileEnd,':',1);

				if(ErrorEnd)
					ErrorEnd+=2;

				tCompilerMessageRow->Text(String(FileEnd,ErrorEnd-FileEnd)+String(ErrorEnd,LineEnd-ErrorEnd));
//				tCompilerMessageRow->GetColor(GuiRect::BACKGROUND)=0xff0000;
			}

//			tCompilerMessageRow->GetAutoEdges()->ref.make(0,&messagesGlue->GetEdges().w);

			this->Append(tCompilerMessageRow);

			LineBegin=++LineEnd;
		}

		//this->AddChild(&this->vScrollbar);

		//this->contentHeight=messagesGlue->childs.size()*MESSAHE_ROW_HEIGHT;
	}

	return bReturnValue;
}


void GuiCompilerViewer::OnSize(GUIMSGDEF)
{
	/*this->vScrollbar.SetScrollerRatio(this->contentHeight,this->rect.w);

	if(this->childs.size()==2)
		//we don't need the GuiScrollRect::width cause the newly GuiRect::offset
		this->vScrollbar.IsVisible() ? this->childs[0]->offsets.z=-GuiScrollBar::SCROLLBAR_TICK : this->childs[0]->offsets.z=0;*/

	GuiRect::OnSize(GUIMSGCALL);

	/*this->rect.z=this->rect.z;*/
}


////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////Properties////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////EditorEntity////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

EditorEntity::EditorEntity():
	spName(&this->name,GuiStringProperty::STRING),
	spPtr(this,GuiStringProperty::PTR),
	spId(&this->id,GuiStringProperty::INT),
	spPosition(&this->world,GuiStringProperty::MAT4POS),
	spChilds((void*)&this->Childs(),GuiStringProperty::ENTITYLISTSIZE),
	spMin(this->bbox.a,GuiStringProperty::VEC3),
	spMax(this->bbox.b,GuiStringProperty::VEC3),
	spVolume(this->bbox.a,GuiStringProperty::VEC3,this->bbox.b),
	pName(L"Name",spName),
	pPtr(L"Ptr",spPtr),
	pId(L"Id",spId),
	pPosition(L"Position",spPosition),
	pChilds(L"Childs",spChilds),
	pMin(L"Min",spMin),
	pMax(L"Max",spMax),
	pVolume(L"Volume",spVolume),
	pcAABB(L"AABB")
{}

EditorEntity* EditorEntity::Parent(){return (EditorEntity*)this->Entity::Parent();}

const std::list<EditorEntity*>& EditorEntity::Childs(){return (std::list<EditorEntity*>&)this->Entity::Childs();}

void EditorEntity::SetName(String iName)
{
	this->Entity::name=iName;
	this->sceneLabel.SetName(iName);
}

EditorEntity*	EditorEntity::Append(EditorEntity* iEntity)
{
	this->Entity::Append(iEntity);
	this->sceneLabel.Append(iEntity->sceneLabel);
	return iEntity;
}
void	EditorEntity::Remove(EditorEntity* iEntity)
{
	this->Entity::Remove(iEntity);
	this->sceneLabel.Remove(&iEntity->sceneLabel);
}

EditorEntity::~EditorEntity()
{
}

void EditorEntity::DestroyChilds()
{
}

void EditorEntity::OnPropertiesCreate()
{
	this->sceneLabel.Entity(this);
	this->sceneLabel.SetName(this->name);
	this->PropertyContainer().SetName(L"Entity");

	this->PropertyContainer().Append(pName);
	this->PropertyContainer().Append(pPtr);
	this->PropertyContainer().Append(pId);
	this->PropertyContainer().Append(pPosition);
	this->PropertyContainer().Append(pChilds);

	this->PropertyContainer().Append(pcAABB);

	pcAABB.Append(pMin);
	pcAABB.Append(pMax);
	pcAABB.Append(pVolume);
}

void EditorEntity::OnPropertiesUpdate(Tab* tab)
{
	for(std::list<EntityComponent*>::const_iterator it=this->Components().begin();it!=this->Components().end();it++)
	{
		EditorPropertiesBase* componentProperties=dynamic_cast<EditorPropertiesBase*>(*it);

		if(componentProperties)
			componentProperties->OnPropertiesUpdate(tab);
	}
}

EditorMesh::EditorMesh():
	spControlpoints(&this->ncontrolpoints,GuiStringProperty::INT),
	spNormals(&this->nnormals,GuiStringProperty::INT),
	spPolygons(&this->npolygons,GuiStringProperty::INT),
	spTextCoord(&this->ntexcoord,GuiStringProperty::INT),
	spVertexIndices(&this->nvertexindices,GuiStringProperty::INT),
	pControlpoints(L"Controlpoints",spControlpoints),
	pNormals(L"Normals",spNormals),
	pPolygons(L"Polygons",spPolygons),
	pTextCoord(L"Texcoord",spTextCoord),
	pVertexIndices(L"Vertexindices",spVertexIndices)
{}

void EditorMesh::OnPropertiesCreate()
{
	this->PropertyContainer().SetName(L"Mesh");

	this->PropertyContainer().Append(pControlpoints);
	this->PropertyContainer().Append(pNormals);
	this->PropertyContainer().Append(pPolygons);
	this->PropertyContainer().Append(pTextCoord);
	this->PropertyContainer().Append(pVertexIndices);

	this->Entity()->PropertyContainer().Append(this->PropertyContainer());
}

void EditorMesh::OnPropertiesUpdate(Tab* tab)
{
}

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////EditorSkin/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

EditorSkin::EditorSkin():
	spClusters(&this->nclusters,GuiStringProperty::INT),
	spTextures(&this->ntextures,GuiStringProperty::INT),
	pClusters(L"Clusters",spClusters),
	pTextures(L"Textures",spTextures)
{}

void EditorSkin::OnPropertiesCreate()
{
	this->PropertyContainer().SetName(L"Skin");

	this->PropertyContainer().Append(pClusters);
	this->PropertyContainer().Append(pTextures);

	this->Entity()->PropertyContainer().Append(this->PropertyContainer());
}
void EditorSkin::OnPropertiesUpdate(Tab* tab)
{
}
void EditorRoot::OnPropertiesCreate()
{
	this->PropertyContainer().SetName(L"Root");
	this->Entity()->PropertyContainer().Append(this->PropertyContainer());
}
void EditorRoot::OnPropertiesUpdate(Tab* tab)
{
}
void EditorSkeleton::OnPropertiesCreate()
{
	this->PropertyContainer().SetName(L"Skeleton");
	this->Entity()->PropertyContainer().Append(this->PropertyContainer());
}
void EditorSkeleton::OnPropertiesUpdate(Tab* tab)
{
}
void EditorGizmo::OnPropertiesCreate()
{
	this->PropertyContainer().SetName(L"Sizmo");
	this->Entity()->PropertyContainer().Append(this->PropertyContainer());
}
void EditorGizmo::OnPropertiesUpdate(Tab* tab)
{
}



EditorAnimation::EditorAnimation():
	spIsBone(this,GuiStringProperty::ISBONECOMPONENT),
	spDuration(&this->start,GuiStringProperty::FLOAT2MINUSFLOAT1,&this->end),
	spBegin(&this->start,GuiStringProperty::FLOAT),
	spEnd(&this->end,GuiStringProperty::FLOAT),
	pIsBone(L"IsBone",spIsBone),
	pDuration(L"Duration",spDuration),
	pBegin(L"Begin",spBegin),
	pEnd(L"End",spEnd)
{}

void EditorAnimation::OnPropertiesCreate()
{
	this->PropertyContainer().SetName(L"Animation");

	this->PropertyContainer().Append(pIsBone);
	this->PropertyContainer().Append(pDuration);
	this->PropertyContainer().Append(pBegin);
	this->PropertyContainer().Append(pEnd);

	this->Entity()->PropertyContainer().Append(this->PropertyContainer());
}
void EditorAnimation::OnPropertiesUpdate(Tab* tab)
{
}

EditorAnimationController::EditorAnimationController():
	minSpeed(0),
	maxSpeed(2.0f),
	spNumNodes(&this->animations,GuiStringProperty::ANIMATIONVECSIZE),
	spVelocity(&this->speed,&this->minSpeed,&this->maxSpeed),
	spDuration(&this->start,GuiStringProperty::FLOAT2MINUSFLOAT1,&this->end),
	spBegin(&this->start,GuiStringProperty::FLOAT),
	spEnd(&this->end,GuiStringProperty::FLOAT),
	acpGuiAnimationController(*this),
	pNumNodes(L"NumNodes",spNumNodes),
	pVelocity(L"Velocity",spVelocity,30),
	pDuration(L"Duration",spDuration),
	pBegin(L"Begin",spBegin),
	pEnd(L"End",spEnd),
	pPlayer(L"Player",acpGuiAnimationController,50)
{}

EditorAnimationController::~EditorAnimationController(){}

void EditorAnimationController::OnPropertiesCreate()
{
	this->PropertyContainer().SetName(L"AnimationController");

	this->PropertyContainer().Append(pNumNodes);
	this->PropertyContainer().Append(pBegin);
	this->PropertyContainer().Append(pEnd);
	this->PropertyContainer().Append(pDuration);
	this->PropertyContainer().Append(pPlayer);
	this->PropertyContainer().Append(pVelocity);
	

	this->Entity()->PropertyContainer().Append(this->PropertyContainer());
}


void EditorAnimationController::OnPropertiesUpdate(Tab* tab)
{
	if(this->PropertyContainer().Expanded() && this->oldCursor!=this->cursor)
	{
		tab->SetDraw(&this->acpGuiAnimationController.slider);

		this->oldCursor=this->cursor;
	}
}

EditorLine::EditorLine():
	spNumSegments(&this->points,GuiStringProperty::VEC3LISTSIZE),
	pNumSegments(L"NumSegments",spNumSegments)
{}

void EditorLine::OnPropertiesCreate()
{
	this->PropertyContainer().SetName(L"Line");

	this->PropertyContainer().Append(pNumSegments);

	for(std::list<vec3>::iterator i=this->points.begin();i!=this->points.end();i++)
	{
		GuiListBox::Item* tPoint=new GuiListBox::Item(*i,GuiStringProperty::VEC3);

		this->pointItems.push_back(tPoint);
		this->pointListBox.Append(tPoint);
	}

	this->Entity()->PropertyContainer().Append(this->PropertyContainer());
}
void EditorLine::OnPropertiesUpdate(Tab* tab)
{
}

void EditorLine::DestroyPoints()
{
	this->pointListBox.Destroy();
}
void EditorLine::Append(vec3 iPoint)
{
	this->points.push_back(iPoint);
	vec3* tVec3=&this->points.back();

	GuiListBox::Item* tPoint=new GuiListBox::Item(tVec3,GuiStringProperty::VEC3);

	this->pointItems.push_back(tPoint);
	this->pointListBox.Append(tPoint);
}


void EditorBone::OnPropertiesCreate()
{
	this->PropertyContainer().SetName(L"Bone");
	this->Entity()->PropertyContainer().Append(this->PropertyContainer());
}
void EditorBone::OnPropertiesUpdate(Tab* tab)
{
}
void EditorLight::OnPropertiesCreate()
{
	this->PropertyContainer().SetName(L"Light");
	this->Entity()->PropertyContainer().Append(this->PropertyContainer());
}
void EditorLight::OnPropertiesUpdate(Tab* tab)
{
}

void editScriptEditorCallback(void* iData)
{
	EditorScript* editorScript=(EditorScript*)iData;

	Tab* tabContainer=Ide::Instance()->mainAppWindow->mainContainer->tabs[0];

	if(!tabContainer)
		DEBUG_BREAK();

	std::list<GuiScriptViewer*>& tGuiScriptViewerInstances=GuiScriptViewer::GetInstances();

	if(GuiScriptViewer::GetInstances().empty())
		tabContainer->rects.Append(new GuiScriptViewer);

	GuiScriptViewer* guiScriptViewer=GuiScriptViewer::GetInstances().front();

	if(guiScriptViewer)
	{
		guiScriptViewer->Open(editorScript);
		guiScriptViewer->OnSize(tabContainer,Msg());
		tabContainer->SetDraw();
	}
}

void compileScriptEditorCallback(void* iData)
{
	EditorScript* editorScript=(EditorScript*)iData;

    Ide::Instance()->compiler->Compile(editorScript);
}



EditorScript::EditorScript():scriptViewer(0)
{

};

void EditorScript::OnPropertiesCreate()
{
	this->PropertyContainer().SetName(L"Bone");
	this->Entity()->PropertyContainer().Append(this->PropertyContainer());

	/*GuiProperty<GuiString>* tFile=container->Property(L"File",new GuiString(this->Script::file));
	GuiProperty<GuiString>* tRunning=container->Property(L"Running",new GuiString(&this->Script::runtime,StringProperty::BOOLPTR));

	GuiButton* buttonEdit=new GuiButton;
	buttonEdit->SetName(L"EditorScript Edit Button");
	buttonEdit->func=editScriptEditorCallback;
	buttonEdit->param=this;
	buttonEdit->text=L"Edit";
//	buttonEdit->GetAutoEdges()->fix.make(0,0,0,20);
	container->Insert(buttonEdit);

	GuiButton* buttonCompile=new GuiButton;
	buttonCompile->func=compileScriptEditorCallback;
	buttonCompile->param=this;
	buttonCompile->text=L"Compile";
//	buttonCompile->GetAutoEdges()->fix.make(0,0,0,20);
	container->Insert(buttonCompile);

	this->buttonLaunch=new GuiButton;
	this->buttonLaunch->func=launchStopScriptEditorCallback;
	this->buttonLaunch->param=this;
	this->buttonLaunch->text=L"Launch";
//	this->buttonLaunch->GetAutoEdges()->fix.make(0,0,0,20);
	container->Insert(this->buttonLaunch);*/
}
void EditorScript::OnPropertiesUpdate(Tab* tab)
{
}

void EditorScript::SaveScript(const String& iString)
{
	StringUtils::WriteCharFile(this->file,iString,L"wb");
}

String EditorScript::LoadScript()
{
	return StringUtils::ReadCharFile(this->file,L"rb");
}

void EditorScript::OnResourcesCreate()
{
	if(this->file.empty())
		this->file=Ide::Instance()->folderProject + L"\\" + this->Entity()->name + L".cpp";
	
	if(!File::Exist(this->file))
	{
		if(!File::Create(this->file))
			DEBUG_BREAK();

		String tDefaultScript=L"#include \"entities.h\"\n\nstruct " + this->Entity()->name + L"_ : EntityScript\n{\n\t int counter;\n\tvoid init()\n\t{\n\t\tcounter=0;\n\tthis->entity->local.identity();\n\t\tprintf(\"inited\\n\");\n\t}\n\n\tvoid update()\n\t{\n\t\tthis->entity->local.translate(0.1f,0,0);\n\t//printf(\"counter: %d\\n\",counter);\n\tcounter++;\n\t}\n\n\tvoid deinit()\n\t{\n\t\tprintf(\"deinited\\n\");\n\t}\n\n};\n";

		this->SaveScript(tDefaultScript);
	}

	GuiProjectViewer::Instance()->RefreshAll();

	this->resourceNode=ResourceNodeDir::FindFileNode(this->file);

	if(!this->resourceNode)
		DEBUG_BREAK();
}

void EditorCamera::OnPropertiesCreate()
{
	this->PropertyContainer().SetName(L"Camera");
	this->Entity()->PropertyContainer().Append(this->PropertyContainer());
}
void EditorCamera::OnPropertiesUpdate(Tab* tab)
{
}




///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////FileSystem////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

bool FileExists(const char* iFile)
{
	FILE* tFile=fopen(iFile,"rb");

	if(tFile)
	{
		return true;
		fclose(tFile);
	}

	return false;
}


///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////PluginSystem//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


PluginSystem::Plugin::Plugin():loaded(false),name(L"Plugin"),handle(0),listBoxItem(this){}

void PluginSystem::Plugin::Load()
{
	this->loaded=true;
	this->listBoxItem.button.Color(0x00ff00);
}

void PluginSystem::Plugin::Unload()
{
	this->loaded=false;
	this->listBoxItem.button.Color(0xff0000);
}

PluginSystem::PluginSystem():viewer(this){this->viewer.SetName(L"Plugins");}
PluginSystem::~PluginSystem(){}	

PluginSystem::Plugin::PluginListBoxItem::PluginListBoxItem(Plugin* iPlugin):Item(&iPlugin->name,GuiStringProperty::STRING),plugin(iPlugin)
{
	this->button.Activate(true);
	this->Append(&this->button);
}

void PluginSystem::Plugin::PluginListBoxItem::OnSize(GUIMSGDEF)
{
	Item::OnSize(GUIMSGCALL);

	vec4 tEdges=this->Edges();

	this->button.Edges(vec4(tEdges.z-15,tEdges.w-15,tEdges.z-5,tEdges.w-5));
}

void PluginSystem::Plugin::PluginListBoxItem::OnControlEvent(GUIMSGDEF)
{
	Item::OnControlEvent(GUIMSGCALL);

	if(iMsg.sender==&this->button && iMsg.senderFunc==ONMOUSEUP)
		this->plugin->loaded ? this->plugin->Unload() : this->plugin->Load();
}

const std::list<PluginSystem::Plugin*>& PluginSystem::Plugins(){return this->plugins;}
PluginSystem::PluginViewer& PluginSystem::Viewer(){return this->viewer;}

PluginSystem::PluginViewer::PluginViewer(PluginSystem* iPluginSystem):pluginSystem(iPluginSystem){}

void PluginSystem::PluginViewer::OnSize(GUIMSGDEF)
{
	GuiRect::OnSize(GUIMSGCALL);

	vec4 tEdges=this->GetParent()->Edges();

	this->pluginList.Edges(vec4(tEdges.x,tEdges.y,tEdges.z,tEdges.w-30));
	this->exitButton.Edges(vec4(tEdges.z-110,tEdges.w-25,tEdges.z-10,tEdges.w-5));
}

void PluginSystem::PluginViewer::OnControlEvent(GUIMSGDEF)
{
	if(iMsg.sender==&this->exitButton && iMsg.senderFunc==ONMOUSEUP)
	{
		this->tab->Destroy();
		Ide::Instance()->mainAppWindow->mainContainer->windowData->Enable(true);
	}
}

void PluginSystem::PluginViewer::Show()
{
	this->pluginSystem->ScanPluginsDirectory();

	TabContainer* tContainer=Ide::Instance()->mainAppWindow->mainContainer;

	tContainer->windowData->Enable(false);

	vec2 tIdeFrameSize=tContainer->windowData->Size();
	vec2 tTabSize(500,300);
	vec2 tTabPos=tContainer->windowData->Pos();

	tTabPos.x+=tIdeFrameSize.x/2.0f-tTabSize.x/2.0f;
	tTabPos.y+=tIdeFrameSize.y/2.0f-tTabSize.y/2.0f;

	this->tab=tContainer->CreateModalTab(tTabPos.x,tTabPos.y,tTabSize.x,tTabSize.y);

	vec4 tTabEdges(tTabPos.x,tTabPos.y,tTabPos.x+tTabSize.x,tTabPos.y+tTabSize.y);

	this->tab->rects.Append(this);

	this->Color(0x303030);

	int even=0x101010;
	int odd=0x202020;

	int idx=0;

	for(std::list<Plugin*>::const_iterator i=this->pluginSystem->Plugins().begin();i!=this->pluginSystem->Plugins().end();i++,idx++)
	{
		Plugin* tPlugin=(*i);

		this->pluginList.Append(&tPlugin->listBoxItem);

		tPlugin->listBoxItem.Color(this->pluginList.Color() + (idx%2==0 ? even : odd));
		
		tPlugin->listBoxItem.SetFlag(GuiRect::FLAGS,GuiRect::DRAWHOVER,false);
		tPlugin->listBoxItem.SetFlag(GuiRect::FLAGS,GuiRect::DRAWPRESS,false);

		tPlugin->listBoxItem.button.Color(tPlugin->loaded ? 0x00ff00 : 0xff0000);
	}

	this->pluginList.Activate(true);
	this->exitButton.Activate(true);

	this->Append(&this->pluginList);
	this->Append(&this->exitButton);

	this->exitButton.Text(L"Exit");

	this->Activate(true);
}



///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////MainContainer/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


MainContainer::MainContainer():mainContainer(0){}
MainContainer::~MainContainer(){}

void MainContainer::OnMenuPressed(int iIdx)
{
	if(iIdx==MenuActionExit)
	{

	}
	else if(iIdx==MenuActionConfigurePlugin)
	{
		Ide::Instance()->pluginSystem->Viewer().Show();
	}
	else if(iIdx==MenuActionProgramInfo)
	{

	}

}


void MainContainer::Broadcast(GuiRect::Funcs iFunc,const Msg& iMsg)
{
	for(std::vector<TabContainer*>::iterator tContainer=this->containers.begin();tContainer!=this->containers.end();tContainer++)
		(*tContainer)->Broadcast(iFunc,iMsg);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////Subsystem//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

String Subsystem::RandomDir(String iWhere,int iSize,String iAlphabet)
{
	String tRandomWorkingDirectoryName;
	String tRandomWorkingDirectory;

	while(true)
	{
		tRandomWorkingDirectoryName=StringUtils::RandomString(iSize,iAlphabet);

		String tFullRandomDirName=iWhere + L"\\" + tRandomWorkingDirectoryName;

		if(!this->DirectoryExist(tRandomWorkingDirectory.c_str()))
			break;
	}

	tRandomWorkingDirectory=iWhere + L"\\" + tRandomWorkingDirectoryName;

	if(!this->CreateDirectory(tRandomWorkingDirectory.c_str()))
		DEBUG_BREAK();

	if(!this->DirectoryExist(tRandomWorkingDirectory.c_str()))
		DEBUG_BREAK();

	return tRandomWorkingDirectory;
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////SerializerHelpers impl//////////
///////////////////////////////////////////////
///////////////////////////////////////////////

void SerializerHelpers::Save(Mesh* tMesh,FILE* iFile)
{
	WriteComponentCode(Serializer::Mesh,iFile);

	fwrite(&tMesh->ncontrolpoints,sizeof(unsigned int),1,iFile);
	fwrite(&tMesh->nvertexindices,sizeof(unsigned int),1,iFile);
	fwrite(&tMesh->ntexcoord,sizeof(unsigned int),1,iFile);
	fwrite(&tMesh->nnormals,sizeof(unsigned int),1,iFile);
	fwrite(&tMesh->npolygons,sizeof(unsigned int),1,iFile);
	fwrite(&tMesh->isCCW,sizeof(bool),1,iFile);

	if(tMesh->ncontrolpoints)
		fwrite(tMesh->controlpoints,sizeof(float)*3*tMesh->ncontrolpoints,1,iFile);

	if(tMesh->nvertexindices)
		fwrite(tMesh->vertexindices,sizeof(unsigned int)*tMesh->nvertexindices,1,iFile);

	if(tMesh->ntexcoord)
		fwrite(tMesh->texcoord,sizeof(float)*2*tMesh->ntexcoord,1,iFile);

	if(tMesh->nnormals)
		fwrite(tMesh->normals,sizeof(float)*3*tMesh->nnormals,1,iFile);
}


void SerializerHelpers::Save(Skin* tSkin,FILE* iFile)
{
	WriteComponentCode(Serializer::Skin,iFile);

	fwrite(&tSkin->nclusters,sizeof(unsigned int),1,iFile);
	fwrite(&tSkin->ntextures,sizeof(unsigned int),1,iFile);

	for(int clusterIdx=0;clusterIdx<tSkin->nclusters;clusterIdx++)
	{
		Cluster* tCluster=&tSkin->clusters[clusterIdx];

		fwrite(&tCluster->bone->id,sizeof(unsigned int),1,iFile);
		fwrite(&tCluster->ninfluences,sizeof(unsigned int),1,iFile);
		fwrite(tCluster->offset,sizeof(float),16,iFile);

		for(int influenceIdx=0;influenceIdx<tCluster->ninfluences;influenceIdx++)
		{
			Influence* tInfluence=&tCluster->influences[influenceIdx];

			fwrite(&tInfluence->weight,sizeof(float),1,iFile);
			fwrite(&tInfluence->ncontrolpointindex,sizeof(unsigned int),1,iFile);
			fwrite(tInfluence->controlpointindex,sizeof(unsigned int)*tInfluence->ncontrolpointindex,1,iFile);
		}
	}

	SerializerHelpers::Save((Mesh*)tSkin,iFile);
}


void SerializerHelpers::Save(Animation* tAnimation,FILE* iFile)
{
	WriteComponentCode(Serializer::Animation,iFile);

	fwrite(&tAnimation->start,sizeof(float),1,iFile);
	fwrite(&tAnimation->end,sizeof(float),1,iFile);
	fwrite(&tAnimation->index,sizeof(int),1,iFile);

	unsigned int tAnimClipsSize=tAnimation->clips.size();

	fwrite(&tAnimClipsSize,sizeof(unsigned int),1,iFile);

	for(std::vector<AnimClip*>::iterator iterAnimClip=tAnimation->clips.begin();iterAnimClip!=tAnimation->clips.end();iterAnimClip++)
	{
		AnimClip* tAnimClip=*iterAnimClip;

		fwrite(&tAnimClip->start,sizeof(float),1,iFile);
		fwrite(&tAnimClip->end,sizeof(float),1,iFile);

		unsigned int tKeyCurveSize=tAnimClip->curves.size();

		fwrite(&tKeyCurveSize,sizeof(unsigned int),1,iFile);

		for(std::vector<KeyCurve*>::iterator iterKeyCurve=tAnimClip->curves.begin();iterKeyCurve!=tAnimClip->curves.end();iterKeyCurve++)
		{
			KeyCurve* tKeyCurve=*iterKeyCurve;

			fwrite(&tKeyCurve->channel,sizeof(unsigned int),1,iFile);
			fwrite(&tKeyCurve->start,sizeof(float),1,iFile);
			fwrite(&tKeyCurve->end,sizeof(float),1,iFile);

			unsigned int tKeyframeSize=tKeyCurve->frames.size();

			fwrite(&tKeyframeSize,sizeof(unsigned int),1,iFile);

			for(std::vector<Keyframe*>::iterator iterKeyFrame=tKeyCurve->frames.begin();iterKeyFrame!=tKeyCurve->frames.end();iterKeyFrame++)
			{
				Keyframe* tKeyframe=*iterKeyFrame;

				fwrite(&tKeyframe->time,sizeof(float),1,iFile);
				fwrite(&tKeyframe->value,sizeof(float),1,iFile);
			}
		}
	}
}


void SerializerHelpers::Save(AnimationController* tAnimationController,FILE* iFile)
{
	WriteComponentCode(Serializer::AnimationController,iFile);

	unsigned int tAnimationIdsSize=tAnimationController->animations.size();
	fwrite(&tAnimationIdsSize,sizeof(unsigned int),1,iFile);

	if(tAnimationIdsSize)
	{
		unsigned int *tAnimationIdsArray=new unsigned int[tAnimationIdsSize];

		for(int i=0;i<tAnimationIdsSize;i++)
		{
			wprintf(L"animation[%d] entity: %s\n",i,tAnimationController->animations[i]->Entity()->name.c_str());
			tAnimationIdsArray[i]=tAnimationController->animations[i]->Entity()->id;
		}

		fwrite(tAnimationIdsArray,sizeof(unsigned int)*tAnimationIdsSize,1,iFile);
		SAFEDELETEARRAY(tAnimationIdsArray);
	}

	fwrite(&tAnimationController->speed,sizeof(float),1,iFile);
	fwrite(&tAnimationController->cursor,sizeof(float),1,iFile);
	fwrite(&tAnimationController->play,sizeof(bool),1,iFile);
	fwrite(&tAnimationController->looped,sizeof(bool),1,iFile);
	fwrite(&tAnimationController->start,sizeof(float),1,iFile);
	fwrite(&tAnimationController->end,sizeof(float),1,iFile);
	fwrite(&tAnimationController->framesPerSecond,sizeof(unsigned int),1,iFile);
	fwrite(&tAnimationController->frameTime,sizeof(unsigned int),1,iFile);
}


void SerializerHelpers::Save(Line* tLine,FILE* iFile)
{
	WriteComponentCode(Serializer::Line,iFile);

	unsigned int tPointSize=tLine->points.size();

	fwrite(&tPointSize,sizeof(unsigned int),1,iFile);

	for(std::list<vec3>::iterator it=tLine->points.begin();it!=tLine->points.end();it++)
		fwrite(&(*it),sizeof(float)*3,1,iFile);
}

void SerializerHelpers::Load(EditorLine* tLine,FILE* iFile)
{
	unsigned int	tPointSize;
	vec3			tPoint;

	fread(&tPointSize,sizeof(unsigned int),1,iFile);

	for(int i=0;i<tPointSize;i++)
	{
		fread(&tPoint,sizeof(float)*3,1,iFile);
		tLine->Append(tPoint);
	}
}

void SerializerHelpers::Save(Script* tScript,FILE* iFile)
{
	WriteComponentCode(Serializer::Script,iFile);

	StringUtils::WriteWstring(iFile,tScript->file);
}


void SerializerHelpers::saveSceneEntityRecursively(Entity* iEntity,FILE* iFile)
{
	unsigned int childsSize=iEntity->Childs().size();
	unsigned int componentsSize=iEntity->Components().size();

	if(iEntity->id<0)
		DEBUG_BREAK();

	//iEntity->saved=true;

	fwrite(&childsSize,sizeof(unsigned int),1,iFile);
	fwrite(&iEntity->id,sizeof(unsigned int),1,iFile);

	fwrite(iEntity->local,sizeof(float),16,iFile);
	fwrite(iEntity->world,sizeof(float),16,iFile);

	StringUtils::WriteWstring(iFile,iEntity->name);

	fwrite(iEntity->bbox.a,sizeof(float),3,iFile);
	fwrite(iEntity->bbox.b,sizeof(float),3,iFile);

	fwrite(&componentsSize,sizeof(unsigned int),1,iFile);

	for(std::list<EntityComponent*>::const_iterator iteratorComponent=iEntity->Components().begin();iteratorComponent!=iEntity->Components().end();iteratorComponent++)
	{
		if((*iteratorComponent)->is<Script>()) SerializerHelpers::Save((Script*)(*iteratorComponent),iFile);
		else if((*iteratorComponent)->is<Line>()) SerializerHelpers::Save((Line*)(*iteratorComponent),iFile);
		else if((*iteratorComponent)->is<Animation>()) SerializerHelpers::Save((Animation*)(*iteratorComponent),iFile);
		else if((*iteratorComponent)->is<AnimationController>()) SerializerHelpers::Save((AnimationController*)(*iteratorComponent),iFile);
		else if((*iteratorComponent)->is<Skin>()) SerializerHelpers::Save((Skin*)(*iteratorComponent),iFile);
		else if((*iteratorComponent)->is<Mesh>()) SerializerHelpers::Save((Mesh*)(*iteratorComponent),iFile);
		else if((*iteratorComponent)->is<Bone>()) WriteComponentCode(Serializer::Bone,iFile);
		else
			DEBUG_BREAK();
	}

	if(childsSize)
	{
		for(std::list<Entity*>::const_iterator eIter=iEntity->Childs().begin();eIter!=iEntity->Childs().end();eIter++)
			saveSceneEntityRecursively(*eIter,iFile);
	}
}



EditorEntity* SerializerHelpers::loadSceneEntityRecursively(EditorEntity* iEditorEntityParent,FILE* iFile)
{
	unsigned int componentsSize;
	unsigned int childsSize;
	int componentCode;

	EditorEntity* tEditorEntity=new EditorEntity;

	iEditorEntityParent->Append(tEditorEntity);

	fread(&childsSize,sizeof(unsigned int),1,iFile);
	fread(&tEditorEntity->id,sizeof(unsigned int),1,iFile);

	fread(tEditorEntity->local,sizeof(float),16,iFile);
	fread(tEditorEntity->world,sizeof(float),16,iFile);

	StringUtils::ReadWstring(iFile,tEditorEntity->name);

	fread(tEditorEntity->bbox.a,sizeof(float),3,iFile);
	fread(tEditorEntity->bbox.b,sizeof(float),3,iFile);

	fread(&componentsSize,sizeof(unsigned int),1,iFile);

	for(int tComponentIndex=0;tComponentIndex<componentsSize;tComponentIndex++)
	{
		fread(&componentCode,sizeof(int),1,iFile);

		switch(componentCode)
		{
			case Serializer::Script: SerializerHelpers::Load(tEditorEntity->CreateComponent<EditorScript>(),iFile); break;
			case Serializer::Line: SerializerHelpers::Load(tEditorEntity->CreateComponent<EditorLine>(),iFile); break;
			case Serializer::Animation: SerializerHelpers::Load(tEditorEntity->CreateComponent<EditorAnimation>(),iFile); break;
			case Serializer::AnimationController: SerializerHelpers::Load(tEditorEntity->CreateComponent<EditorAnimationController>(),iFile); break; 
			case Serializer::Mesh: SerializerHelpers::Load(tEditorEntity->CreateComponent<EditorMesh>(),iFile); break;
			case Serializer::Skin: SerializerHelpers::Load(tEditorEntity->CreateComponent<EditorSkin>(),iFile); break;
			/*default:
				DEBUG_BREAK();*/
		}

		if(tEditorEntity->Components().back())
		{
			EditorPropertiesBase* tEditorObjectBase=dynamic_cast<EditorPropertiesBase*>(tEditorEntity->Components().back());
			
			if(tEditorObjectBase)
				tEditorObjectBase->OnResourcesCreate();
		}
	}

	if(childsSize)
	{
		for(int i=0;i<childsSize;i++)
			loadSceneEntityRecursively(tEditorEntity,iFile);
	}

	BindSkinLinks(tEditorEntity);
	BindAnimationLinks(tEditorEntity);

	tEditorEntity->OnResourcesCreate();
	tEditorEntity->OnPropertiesCreate();

	return tEditorEntity;
}

void SerializerHelpers::WriteComponentCode(int iCode,FILE* iFile)
{
	fwrite(&iCode,sizeof(int),1,iFile);
}
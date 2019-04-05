#include "interfaces.h"

#include "imgpng.h"
#include "imgjpg.h"
#include "imgtga.h"

/////////////////////globals///////////////////



GLOBALGETTERFUNC(GlobalGuiProjectViewerInstance,GuiProject*);
GLOBALGETTERFUNC(GlobalGuiSceneViewerInstance,GuiScene*);
GLOBALGETTERFUNC(GlobalGuiCompilerViewerInstance,GuiCompiler*);
GLOBALGETTERFUNC(GlobalGuiConsoleViewerInstance,GuiConsole*);
GLOBALGETTERFUNC(GlobalDefaultFontInstance,GuiFont*);
GLOBALGETTERFUNC(GlobalFontPoolInstance,std::vector<GuiFont*>);
GLOBALGETTERFUNC(GlobalMainFrameInstance,MainFrame*);
GLOBALGETTERFUNC(GlobalFrameInstance,std::list<Frame*>);
GLOBALGETTERFUNC(GlobalScriptViewers,std::list<GuiScript*>);
GLOBALGETTERFUNC(GlobalViewports,std::list<GuiViewport*>);
GLOBALGETTERFUNC(GlobalRootProjectDirectory,ResourceNodeDir*);
GLOBALGETTERFUNC(GlobalGuiEntityViewerInstance,std::list<GuiEntity*>);
GLOBALGETTERFUNC(GlobalGuiLoggerInstance,std::list<GuiLogger*>);
GLOBALGETTERFUNC(GlobalPluginSystemInstance,PluginSystem*);
GLOBALGETTERFUNC(GlobalDebuggerInstance,Debugger*);
GLOBALGETTERFUNC(GlobalTouchInputInstance,TouchInput*);
GLOBALGETTERFUNC(GlobalKeyboardInputInstance,Keyboard*);
GLOBALGETTERFUNC(GlobalMouseInputInstance,Mouse*);
GLOBALGETTERFUNC(GlobalSubsystemInstance,Subsystem*);
GLOBALGETTERFUNC(GlobalViewersInstance,std::list<GuiViewer*>);
GLOBALGETTERFUNC(GlobalStringEditorInstance,StringEditor*);
GLOBALGETTERFUNC(GlobalCaretInstance,GuiCaret*);

bool EdgesContainsPoint(const vec4& ivec,const vec2& ipoint)
{
	return (ipoint.x>ivec.x && ipoint.x<ivec.z && ipoint.y>ivec.y && ipoint.y<ivec.w);
}

bool CircleContainsPoint(float x,float y,float ray,const vec2& ipoint)
{
	return (ipoint.x>(x-ray) && ipoint.x<(x+ray) && ipoint.y>(y-ray) && ipoint.y<(y+ray));
}

unsigned int BlendColor(unsigned int in,unsigned int icol)
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

void SetFlag(unsigned int& iFlags,unsigned int iFlag,bool iValue)
{
	iFlags ^= (-iValue^iFlags) & (1UL << iFlag);
}

bool GetFlag(unsigned int& iFlags,unsigned int iFlag)
{
	return (iFlags >> iFlag) & 1U;
}

///////////////SerializerHelpers///////////////

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

///////////////ResourceNode/////////////////

ResourceNode::ResourceNode():
	parent(0),
	isDir(0),
	fileLabel(this)
{
}

ResourceNode::~ResourceNode()
{
	this->parent=0;
	this->fileName.clear();
	this->isDir=false;
}

ResourceNodeDir::ResourceNodeDir():dirLabel(this){}

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

///////////////////Ide////////////////

Ide::Ide():
	processId(0),
	processThreadId(0),
	popup(0)
{}

Ide::~Ide()
{
	
}

String Ide::GetSceneExtension()
{
	return L".engineScene";
}
String Ide::GetEntityExtension()
{
	return L".engineEntity";
}

Frame*	Ide::CreatePopup(Frame* iParent,float ix,float iy,float iw,float ih)
{
	this->DestroyPopup();

	wprintf(L"creating popup\n");

	/*this->popup=iParent->CreateModalViewer(ix,iy,iw,ih);
	this->popup->hasFrame=false;*/

	return this->popup;
}
void	Ide::DestroyPopup()
{
	if(this->popup)
	{
		wprintf(L"destroying popup %p\n",this->popup);

		//this->popup->viewers.SetState(GuiRectState::ACTIVE,false);

		this->popup->Destroy();
		this->popup=0;
	}
}

Frame*	Ide::GetPopup()
{
	return this->popup;
}

///////////////////Debugger//////////////////

Debugger::Debugger():breaked(false),threadSuspendend(false),runningScript(0),runningScriptFunction(0),debuggerCode(0),lastBreakedAddress(0),sleep(1){}

Debugger* Debugger::Instance()
{
	GLOBALGETTERFUNCASSINGLETON(GlobalDebuggerInstance,Debugger);
}

std::vector<Debugger::Breakpoint>& Debugger::GetAllBreakpoint()
{
	return this->allAvailableBreakpoints;
}

std::vector<Debugger::Breakpoint>& Debugger::GetBreakpointSet()
{
	return this->breakpointSet;
}
/*

Script*			Debugger::GetRunningScript()
{

}

unsigned char	Debugger::GetRunningScriptFunction()
{

}*/

///////////////////TouchInput//////////////////

TouchInput::TouchInput()
{
	for(int i=0;i<10;i++)
	{
		pressed[i]=0;
		released[i]=0;
		position[i].make(0,0);
	}
}

TouchInput* TouchInput::Instance()
{
	GLOBALGETTERFUNCASSINGLETON(GlobalTouchInputInstance,TouchInput);
}

bool TouchInput::IsPressed(int i){return pressed[i];}
bool TouchInput::IsReleased(int i){return released[i];}

void TouchInput::SetPressed(bool b,int i){pressed[i]=b;}
void TouchInput::SetReleased(bool b,int i){released[i]=b;}


vec2& TouchInput::GetPosition(int i){return position[i];}
void   TouchInput::SetPosition(vec2& pos,int i){position[i]=pos;}

///////////////////MouseInput//////////////////


Mouse::Mouse(){}

Mouse* Mouse::Instance()
{
	GLOBALGETTERFUNCASSINGLETON(GlobalMouseInputInstance,Mouse);
}

bool Mouse::Left(){return this->buttons[0];}
bool Mouse::Right(){return this->buttons[1];}
bool Mouse::Middle(){return this->buttons[2];}

///////////////////KeyboardInput//////////////////

Keyboard::Keyboard(){}

Keyboard* Keyboard::Instance()
{
	GLOBALGETTERFUNCASSINGLETON(GlobalKeyboardInputInstance,Keyboard);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////TabContainer/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

unsigned char Frame::rawUpArrow[]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0xa7,0x0,0x0,0x0,0xa8,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x87,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x88,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6a,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x6b,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x4b,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x4c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x2b,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x2c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xb,0x0,0x0,0x0,0xfa,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfb,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x7,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0x7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
unsigned char Frame::rawRightArrow[]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0x7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0xb,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfa,0x0,0x0,0x0,0x2b,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x4b,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x6a,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x87,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xa7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xa8,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x88,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x6b,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x4c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfb,0x0,0x0,0x0,0x2c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0x7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
unsigned char Frame::rawLeftArrow[]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x07,0x0,0x0,0x0,0x0c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0c,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x2c,0x0,0x0,0x0,0xfb,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x4c,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6b,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x01,0x0,0x0,0x0,0x88,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xa8,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xa7,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x01,0x0,0x0,0x0,0x87,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6a,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x4b,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x2b,0x0,0x0,0x0,0xfa,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0b,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x07,0x0,0x0,0x0,0x0c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
unsigned char Frame::rawDownArrow[]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x7,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0x7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0xfb,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfa,0x0,0x0,0x0,0xb,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x2c,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x2b,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x4c,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x4b,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6b,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x6a,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x88,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x87,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0xa8,0x0,0x0,0x0,0xa7,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
unsigned char Frame::rawFolder[]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe6,0xc1,0x9b,0x12,0xe9,0xc4,0x9f,0xd8,0xe9,0xc4,0xa0,0xff,0xe9,0xc4,0xa0,0xff,0xe9,0xc4,0xa0,0xff,0xe9,0xc4,0xa0,0xff,0xe9,0xc4,0xa0,0xff,0xe6,0xc0,0x9a,0x5b,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe5,0xc1,0x9e,0x20,0xe5,0xc2,0x9e,0xef,0xe5,0xc2,0x9e,0xff,0xe5,0xc2,0x9e,0xff,0xe5,0xc2,0x9d,0xff,0xe5,0xc2,0x9d,0xff,0xe5,0xc2,0x9d,0xff,0xe4,0xc2,0x9c,0xa6,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe3,0xbe,0x98,0x21,0xdf,0xb9,0x94,0xee,0xdf,0xb9,0x93,0xff,0xdf,0xb9,0x93,0xff,0xde,0xb9,0x92,0xff,0xde,0xb9,0x93,0xff,0xde,0xb9,0x93,0xff,0xdd,0xb7,0x90,0xe4,0xda,0xb2,0x89,0xa2,0xda,0xb2,0x8a,0xa3,0xda,0xb2,0x8a,0xa3,0xda,0xb2,0x8a,0xa3,0xda,0xb2,0x8a,0x86,0xe6,0xc1,0x97,0xf,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xfb,0xda,0xaf,0xd2,0xf7,0xd6,0xab,0xfd,0xf7,0xd6,0xab,0xff,0xf7,0xd6,0xab,0xff,0xf7,0xd6,0xab,0xff,0xf6,0xd6,0xab,0xff,0xf6,0xd6,0xab,0xff,0xf6,0xd6,0xab,0xff,0xf6,0xd5,0xab,0xff,0xf6,0xd5,0xab,0xff,0xf6,0xd5,0xab,0xff,0xf6,0xd5,0xab,0xff,0xf6,0xd5,0xab,0xff,0xfa,0xda,0xaf,0xd7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc9,0x96,0x64,0xe7,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xc9,0x97,0x64,0xf1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
unsigned char Frame::rawFile[]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc8,0xc8,0xc8,0x4c,0xe0,0xe0,0xe0,0xcd,0xe1,0xe1,0xe1,0xc0,0xe1,0xe1,0xe1,0xbf,0xe1,0xe1,0xe1,0xbf,0xe1,0xe1,0xe1,0xbf,0xe1,0xe1,0xe1,0xc0,0xd6,0xd6,0xd6,0xc4,0xbd,0xbd,0xbd,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc5,0xc5,0xc5,0x70,0xec,0xec,0xec,0xff,0xed,0xed,0xed,0xff,0xed,0xed,0xed,0xff,0xed,0xed,0xed,0xff,0xed,0xed,0xed,0xff,0xee,0xee,0xee,0xff,0xeb,0xeb,0xeb,0xff,0xce,0xce,0xce,0xff,0xbd,0xbd,0xbd,0xa6,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xbe,0xbe,0xbe,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc5,0xc5,0xc5,0x6c,0xec,0xec,0xec,0xff,0xed,0xed,0xed,0xff,0xed,0xed,0xed,0xff,0xed,0xed,0xed,0xff,0xed,0xed,0xed,0xff,0xed,0xed,0xed,0xff,0xef,0xef,0xef,0xff,0xd3,0xd3,0xd3,0xfd,0xe1,0xe1,0xe1,0xff,0xde,0xde,0xde,0xae,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc8,0xc8,0xc8,0x6c,0xee,0xee,0xee,0xff,0xef,0xef,0xef,0xff,0xef,0xef,0xef,0xff,0xef,0xef,0xef,0xff,0xef,0xef,0xef,0xff,0xef,0xef,0xef,0xff,0xf0,0xf0,0xf0,0xff,0xdb,0xdb,0xdb,0xff,0xc0,0xc0,0xc0,0xfe,0xc9,0xc9,0xc9,0xff,0xb9,0xb9,0xb9,0xbd,0x0,0x0,0x0,0x0,0xff,0xff,0xff,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc9,0xc9,0xc9,0x6b,0xf0,0xf0,0xf0,0xff,0xf1,0xf1,0xf1,0xff,0xf1,0xf1,0xf1,0xff,0xf1,0xf1,0xf1,0xff,0xf1,0xf1,0xf1,0xff,0xf1,0xf1,0xf1,0xff,0xf1,0xf1,0xf1,0xff,0xe7,0xe7,0xe7,0xff,0xc3,0xc3,0xc3,0xff,0xb6,0xb6,0xb6,0xfc,0xc2,0xc2,0xc2,0xff,0xbd,0xbd,0xbd,0x95,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xcb,0xcb,0xcb,0x6b,0xf2,0xf2,0xf2,0xff,0xf3,0xf3,0xf3,0xff,0xf3,0xf3,0xf3,0xff,0xf3,0xf3,0xf3,0xff,0xf3,0xf3,0xf3,0xff,0xf3,0xf3,0xf3,0xff,0xf3,0xf3,0xf3,0xff,0xf2,0xf2,0xf2,0xff,0xf2,0xf2,0xf2,0xff,0xf4,0xf4,0xf4,0xff,0xf2,0xf2,0xf2,0xff,0xd5,0xd5,0xd5,0xc0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xcd,0xcd,0xcd,0x6b,0xf4,0xf4,0xf4,0xff,0xf5,0xf5,0xf5,0xff,0xf5,0xf5,0xf5,0xff,0xf5,0xf5,0xf5,0xff,0xf5,0xf5,0xf5,0xff,0xf5,0xf5,0xf5,0xff,0xf5,0xf5,0xf5,0xff,0xf5,0xf5,0xf5,0xff,0xf6,0xf6,0xf6,0xff,0xf6,0xf6,0xf6,0xff,0xf8,0xf8,0xf8,0xff,0xe4,0xe4,0xe4,0xbc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xcf,0xcf,0xcf,0x6b,0xf6,0xf6,0xf6,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf9,0xf9,0xf9,0xff,0xe5,0xe5,0xe5,0xbc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd0,0xd0,0xd0,0x6b,0xf8,0xf8,0xf8,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xfb,0xfb,0xfb,0xff,0xe8,0xe8,0xe8,0xbc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd2,0xd2,0xd2,0x6b,0xf9,0xf9,0xf9,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfc,0xfc,0xfc,0xff,0xe9,0xe9,0xe9,0xbb,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd3,0xd3,0xd3,0x6b,0xfb,0xfb,0xfb,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfe,0xfe,0xfe,0xff,0xeb,0xeb,0xeb,0xbb,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd4,0xd4,0xd4,0x6b,0xfc,0xfc,0xfc,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xff,0xff,0xff,0xff,0xec,0xec,0xec,0xbb,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd4,0xd4,0xd4,0x6b,0xfd,0xfd,0xfd,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xff,0xff,0xff,0xff,0xec,0xec,0xec,0xbb,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd5,0xd5,0xd5,0x6a,0xfd,0xfd,0xfd,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xff,0xff,0xff,0xff,0xed,0xed,0xed,0xbb,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xdb,0xdb,0xdb,0x70,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf1,0xf1,0xf1,0xc5,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xa4,0xa4,0xa4,0x41,0xbc,0xbc,0xbc,0xab,0xbb,0xbb,0xbb,0xa2,0xbb,0xbb,0xbb,0xa1,0xbb,0xbb,0xbb,0xa1,0xbb,0xbb,0xbb,0xa1,0xbb,0xbb,0xbb,0xa1,0xbb,0xbb,0xbb,0xa1,0xbb,0xbb,0xbb,0xa1,0xbb,0xbb,0xbb,0xa2,0xbb,0xbb,0xbb,0xa2,0xbd,0xbd,0xbd,0xaa,0xb4,0xb4,0xb4,0x73,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x4,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x4,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};

Frame::Frame(float x,float y,float w,float h):
	mainframe(0),
	windowData(0),
	drag(false),
	wasdrawing(0),
	renderer3D(0),
	renderer2D(0),
	retarget(false),
	lastFrameTime(0),
	iconUp(0),
	iconLeft(0),
	iconRight(0),
	iconDown(0),
	iconFolder(0),
	iconFile(0),
	thread(0),
	focused(0),
	isModal(false),
	hasFrame(true),
	mousedata(vec2(),0,0,*Mouse::Instance()),
	dragcode(0),
	sizeboxcode(0)
{
	GlobalFrameInstance().push_back(this);

	hittest.hit=0;
	hittest.locked=false;
	hittest.dragrect[0]=0;
	hittest.dragrect[1]=0;
	hittest.dragrect[2]=0;
	hittest.dragrect[3]=0;
}

Frame::~Frame()
{
	GlobalFrameInstance().remove(this);
}




void Frame::DrawBlock(bool iBool)
{
	Thread* tProjectDirChangedThread=Ide::Instance()->projectDirChangedThread;

	if(iBool)
	{
		this->renderingTask->Block(iBool);
		tProjectDirChangedThread->Block(true);
	}

	for(std::list<DrawInstance*>::iterator iterDrawInstance=this->drawInstances.begin();iterDrawInstance!=this->drawInstances.end();iterDrawInstance++)
	{
		if((*iterDrawInstance) && (*iterDrawInstance)->remove==true)
		{
			SAFEDELETE(*iterDrawInstance);
		}
	}

	if(!iBool)
	{
		tProjectDirChangedThread->Block(false);
		this->renderingTask->Block(iBool);
	}
}

struct DLLBUILD MsgClipData
{
	vec4 clip;
	vec2 coord;
};

void Frame::Render()
{
#if RENDERER_ENABLED
	int tRenderCounter=0;

	for(std::list<GuiViewport*>::iterator i=GuiViewport::GetPool().begin();i!=GuiViewport::GetPool().end();i++)
#if RENDERER_THREADED
		tRenderCounter+=(*i)->Render(this);
#else
	{
		GuiViewport*			tViewport=*i;
		Frame*					tViewportFrame=tViewport->GetFrame();
		EditorEntity*			tEntity=(EditorEntity*)(*i)->GetEntity();

		if(tEntity)
		{
			tEntity->world=tViewport->GetModelMatrix();
			tViewportFrame->renderer3D->UpdateEntities(tEntity);

			for(std::list<GuiEntity*>::iterator it=GuiEntity::GetPool().begin();it!=GuiEntity::GetPool().end();it++)
			{
				EditorEntity*	tEntityPropertiesEntity=(*it)->GetEntity();
				Frame*			tEntityPropertiesFrame=(*it)->GetRoot()->GetFrame();

				if(tEntityPropertiesEntity && tEntityPropertiesFrame)
					tEntityPropertiesEntity->OnPropertiesUpdate(tEntityPropertiesFrame);
			}

			tViewportFrame->SetDraw(tViewport);
		}
	}
#endif
#endif

#if !CARET_OS
	GuiCaret::Instance()->CheckDraw(this);
#endif
}





void Frame::Draw()
{
	/*if(this->drawTask->pause)
		DEBUG_BREAK();*/

	if(!this->drawInstances.empty())
	{
		for(std::list<DrawInstance*>::iterator it=this->drawInstances.begin();it!=this->drawInstances.end();it)
		{
			DrawInstance*& tDrawInstance=*it;

			if(tDrawInstance && !tDrawInstance->skip)
			{
				if(this->BeginDraw(tDrawInstance->rect ? tDrawInstance->rect : (void*)this))
				{
					if(!tDrawInstance->rect)
						this->OnPaint();
					else
						this->BroadcastPaintTo(tDrawInstance->rect,&PaintData(tDrawInstance->param));

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
}

DrawInstance* Frame::SetDraw(GuiRect* iRect,bool iRemove,bool iSkip,void* iParam)
{
	DrawInstance* newInstance=new DrawInstance(iRect,iRemove,iSkip,iParam);
	this->drawInstances.push_back(newInstance);
	this->windowData->PostMessage(0,0,0);
	return newInstance;
}

void Frame::SetDraw(GuiViewport* iViewport)
{
	this->windowData->SendMessage(0,(int)iViewport,0);
}

void Frame::Message(GuiRect* iRect,GuiRectMessages iMsg,void* iData)
{
	if(iRect)iRect->Procedure(this,iMsg,iData);
}

void Frame::Broadcast(GuiRectMessages iMsg,void* iData)
{
	for(std::list<GuiViewer*>::iterator i=this->viewers.begin();i!=this->viewers.end();i++)
		this->BroadcastTo(*i,iMsg,iData);
}

template <typename T> void Frame::BroadcastToPool(GuiRectMessages iMsg,void* iData)
{
	for(typename std::list<T*>::iterator i=T::GetPool().begin();i!=T::GetPool().end();i++)
		this->BroadcastTo(*i,iMsg,iData);
}

template <typename T> void Frame::MessagePool(GuiRectMessages iMsg,void* iData)
{
	for(typename std::list<T*>::iterator i=T::GetPool().begin();i!=T::GetPool().end();i++)
		(*i)->Procedure(this,iMsg,iData);
}


void RollBroadcastOnItemChilds(Frame* iFrame,GuiRect* iRect,GuiRectMessages iMsg,MsgData* iMsgData)
{
	for(std::list<GuiRect*>::const_iterator i=iRect->Childs().begin();i!=iRect->Childs().end() && !iMsgData->skipall && !iMsgData->skipchilds;i++)
		iFrame->BroadcastTo(*i,iMsg,iMsgData);
}


void Frame::BroadcastTo(GuiRect* iRect,GuiRectMessages iMsg,void* iData)
{
	if(iRect && iRect->GetFlag(GuiRectFlag::ACTIVE))
	{
		MsgData* tMsgData=(MsgData*)iData;

		switch(iMsg)
		{
			case ONHITTEST:
			{
				HitTestData& thtd=*(HitTestData*)iData;

				vec2	tMouse=thtd.mouse;

				this->Message(iRect,iMsg,iData);
				RollBroadcastOnItemChilds(this,iRect,iMsg,tMsgData);
			}
			break;
			default:
			{
				this->Message(iRect,iMsg,iData);
				RollBroadcastOnItemChilds(this,iRect,iMsg,tMsgData);
			}
		break;
		}
	}
}

void Frame::BroadcastInputTo(GuiRect* iRect,GuiRectMessages iMsg,void* iData)
{
	/*switch(iMsg)
	{
		case GuiRectMessages::ONMOUSEENTER:
		case GuiRectMessages::ONMOUSEEXIT:
		case GuiRectMessages::ONMOUSEMOVE:
		case GuiRectMessages::ONMOUSEUP:
		case GuiRectMessages::ONMOUSEDOWN:
		case GuiRectMessages::ONMOUSECLICK:
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
		}
		break;
	}*/
}

void Frame::BroadcastPaintTo(GuiRect* iRect,void* iData)
{			
	PaintData&			tPd=*(PaintData*)iData;
	GuiRect*			tRect=iRect->Parent();
	std::list<GuiRect*> tAncestors;

	while(tRect)tAncestors.push_front(tRect),tRect=tRect->Parent();

	for(std::list<GuiRect*>::const_iterator i=tAncestors.begin();i!=tAncestors.end();i++)
		(*i)->Procedure(this,GuiRectMessages::ONPREPAINT,iData);

	this->renderer2D->PushScissor(tPd.clip.x,tPd.clip.y,tPd.clip.z,tPd.clip.w);
	this->renderer2D->Translate(tPd.offset.x,tPd.offset.y);

	this->BroadcastTo(iRect,GuiRectMessages::ONPAINT,iData);

	this->renderer2D->Translate(0,0);
	this->renderer2D->PopScissor();
};

void Frame::OnSize(float iWidth,float iHeight)
{
	if(this->viewers.size()==1)
	{
		this->viewers.front()->edges.make(0,0,iWidth,iHeight);
		this->Message(this->viewers.front(),GuiRectMessages::ONSIZE);
	}
	else
	{
		for(std::list<GuiViewer*>::iterator i=this->viewers.begin();i!=this->viewers.end();i++)
		{
			vec4	tSave=(*i)->edges;
			vec4&	tEdges=(*i)->edges;

			if(tEdges.x!=0)
				tEdges.x-=2,tEdges.x=iWidth*(1.0f/(this->previousSize.x/tEdges.x)),tEdges.x+=2;
			if(tEdges.y!=0)
				tEdges.y-=2,tEdges.y=iHeight*(1.0f/(this->previousSize.y/tEdges.y)),tEdges.y+=2;

			tEdges.z+=2,tEdges.z=iWidth*(1.0f/(this->previousSize.x/tEdges.z)),tEdges.z-=2;
			tEdges.w+=2,tEdges.w=iHeight*(1.0f/(this->previousSize.y/tEdges.w)),tEdges.w-=2;

			/*if((tEdges.z-tEdges.x)<50 || (tEdges.w-tEdges.y)<50)
			{
				tEdges=tSave;
				this->Message(*i,GuiRectMessages::ONSIZE,0);
				break;
			}*/
			
			this->Message(*i,GuiRectMessages::ONSIZE);
		}
	}

	if(iWidth<this->previousSize.x || iHeight<this->previousSize.y)
		this->SetDraw();

	this->previousSize.make(iWidth,iHeight);
}


void Frame::OnMouseMove(float iX,float iY)
{
	this->mouse.x=iX;
	this->mouse.y=iY;

	HitTestData& htd=hittest;

	mousedata.button=0;

	if(!this->drag)
	{
		memset(htd.dragrect,0,sizeof(GuiRect*)*4);

		GuiRect* hit=0;
		GuiRect* old=htd.hit;

		if(!htd.locked)
		{
			htd.mouse=this->mouse;
			htd.off.make(0,0);
			htd.hit=0;

			for(std::list<GuiViewer*>::iterator i=this->viewers.begin();i!=this->viewers.end() && !hittest.hit;i++)
				this->BroadcastTo(*i,GuiRectMessages::ONHITTEST,&hittest);

			mousedata.mouse=htd.mouse;
		}
		else
		{
			mousedata.mouse.x=this->mouse.x+htd.off.x;
			mousedata.mouse.y=this->mouse.y+htd.off.y;
		}

		hit=htd.hit;

		if(hit)
		{
			if(old!=hit)
			{
				this->Message(old,GuiRectMessages::ONMOUSEEXIT);
				this->Message(hit,GuiRectMessages::ONMOUSEENTER,&mousedata);
			}

			this->Message(hit,GuiRectMessages::ONMOUSEMOVE,&mousedata);
		}
		else
		{
			if(this->dragcode==1)
				this->SetCursor(5);
			else if(this->dragcode==2)
				this->SetCursor(1);
			else if(this->dragcode==3)
				this->SetCursor(2);
		}
	}
	else
	{
		if(this->dragcode==1)
		{

		}
		else if(this->dragcode==2)
		{
			for(std::list<GuiViewer*>::iterator i=this->draggables[0].begin();i!=this->draggables[0].end();i++)
				(*i)->edges.z=this->mouse.x,this->Message(*i,GuiRectMessages::ONSIZE);

			for(std::list<GuiViewer*>::iterator i=this->draggables[1].begin();i!=this->draggables[1].end();i++)
				(*i)->edges.x=this->mouse.x+4,this->Message(*i,GuiRectMessages::ONSIZE);
		}
		else if(this->dragcode==3)
		{
			for(std::list<GuiViewer*>::iterator i=this->draggables[0].begin();i!=this->draggables[0].end();i++)
				(*i)->edges.w=this->mouse.y,this->Message(*i,GuiRectMessages::ONSIZE);

			for(std::list<GuiViewer*>::iterator i=this->draggables[2].begin();i!=this->draggables[2].end();i++)
				(*i)->edges.y=this->mouse.y+4,this->Message(*i,GuiRectMessages::ONSIZE);
		}

		this->SetDraw();
	}
}

void Frame::OnMouseWheel(float iScrollingValue)
{
	mousedata.scroller=iScrollingValue;
	this->Message(hittest.hit,GuiRectMessages::ONMOUSEWHEEL,&mousedata);
}

void Frame::OnSizeboxDown(unsigned iSizeboxCode)
{
	this->Message(hittest.hit,GuiRectMessages::ONMOUSEEXIT);
}

void Frame::OnSizeboxUp()
{

}

void Frame::OnMouseDown(unsigned int iButton)
{
	if(this->hittest.hit==0 && iButton==1)
		this->drag=true;

	if(this->drag)
	{
		/*find common edge viewers*/

		if(this->dragcode==1)
		{

		}
		else if(this->dragcode==2 && hittest.dragrect[0] && hittest.dragrect[1])
		{
			for(std::list<GuiViewer*>::iterator i=this->viewers.begin();i!=this->viewers.end();i++)
			{
				if((*i)->edges.z==hittest.dragrect[0]->edges.z)
					this->draggables[0].push_back(*i);
				if((*i)->edges.x==hittest.dragrect[1]->edges.x)
					this->draggables[1].push_back(*i);
			}
		}
		else if(this->dragcode==3 && hittest.dragrect[0] && hittest.dragrect[2])
		{
			for(std::list<GuiViewer*>::iterator i=this->viewers.begin();i!=this->viewers.end();i++)
			{
				if((*i)->edges.w==hittest.dragrect[0]->edges.w)
					this->draggables[0].push_back(*i);
				if((*i)->edges.y==hittest.dragrect[2]->edges.y)
					this->draggables[2].push_back(*i);
			}
		}
	}

	if(hittest.hit)
	{
		mousedata.button=iButton;
		this->Message(hittest.hit,GuiRectMessages::ONMOUSEDOWN,&mousedata);
	}
}

void Frame::OnMouseLeave()
{

}

void Frame::OnMouseUp(unsigned int iButton)
{
	if(this->drag)
	{
		for(int i=0;i<4;i++)
			this->draggables[i].clear();

		this->dragcode=0;
	}

	if(iButton==1)
		this->drag=false;

	if(hittest.hit)
	{
		mousedata.button=iButton;
		this->Message(hittest.hit,GuiRectMessages::ONMOUSEUP,&mousedata);
	}
}

void Frame::OnMouseClick(unsigned int iButton)
{
	switch(iButton)
	{
		case 0:
			mousedata.button=iButton;
			this->Message(hittest.hit,GuiRectMessages::ONMOUSECLICK,&mousedata);
		break;
	}
}



vec2 Frame::Size()
{
	return this->windowData->Size();
}

std::list<GuiViewer*>& Frame::GetViewers()
{
	return this->viewers;
}

std::list<Frame*>& Frame::GetPool()
{
	return GlobalFrameInstance();
}

void Frame::RemoveViewer(GuiViewer* iViewer)
{
	this->viewers.remove(iViewer);
}
GuiViewer* Frame::AddViewer(GuiViewer* iViewer)
{
	iViewer->frame=this;
	this->viewers.push_back(iViewer);
	return iViewer;
}



void Frame::OnPaint()
{
	vec2 iTabDim=this->Size();
	this->renderer2D->DrawRectangle(0,0,iTabDim.x,iTabDim.y,Frame::COLOR_BACK);
	this->Broadcast(GuiRectMessages::ONPAINT,&PaintData());
}


void Frame::OnKeyDown(char iKey)
{
	KeyData tKeyData(iKey);
	this->Message(this->focused,GuiRectMessages::ONKEYDOWN,&tKeyData);
}

void Frame::OnKeyUp(char iKey)
{
	KeyData tKeyData(iKey);
	this->Message(this->focused,GuiRectMessages::ONKEYUP,&tKeyData);
}

void Frame::OnRecreateTarget()
{
	this->Broadcast(GuiRectMessages::ONRECREATETARGET);
}

void Frame::SetFocus(GuiRect* iFocusedRect)
{
	/*bool tFocusHasChanged=(Tab::focused!=iFocusedRect);

	if(tFocusHasChanged && Tab::focused)
		this->SetDraw(Tab::hovered);*/

	Frame::focused=iFocusedRect;

	/*if(tFocusHasChanged && iFocusedRect)
		this->SetDraw(iHoveredRect);*/
}

GuiRect* Frame::GetFocus()
{
	return Frame::focused;
}
 
void Frame::PushScissor(vec4 iClip,vec2 iCoord)
{
	this->renderer2D->PushScissor(iClip.x,iClip.y,iClip.z,iClip.w);
	this->renderer2D->Translate(iCoord.x,iCoord.y);

	ClipData tCd={iClip,iCoord};

	this->clips.push_back(tCd);
}
void Frame::PopScissor(ClipData* oClipData)
{
	this->renderer2D->Translate(0,0);
	this->renderer2D->PopScissor();
	if(oClipData)
		*oClipData=this->clips.back();
	this->clips.pop_back();
}

void Frame::Enable(bool iEnable)
{
	this->windowData->Enable(iEnable);
}

bool Frame::IsEnabled()
{
	return this->windowData->IsEnabled();
}

/////////////////////MainFrame//////////////////////



MainFrame::~MainFrame()
{
	this->Deintialize();
}

MainFrame* MainFrame::Instance()
{
	GLOBALGETTERFUNCASSINGLETON(GlobalMainFrameInstance,MainFrame);
}

bool MainFrame::IsInstanced()
{
	return GlobalMainFrameInstance() ? true : false;
}


void   MainFrame::DestroyFrame(Frame* iFrame)
{
	this->RemoveFrame(iFrame);
	SAFEDELETE(iFrame);
}

void MainFrame::OnMenuPressed(Frame*,int iIdx)
{
	if(iIdx==MenuActionExit)
	{

	}
	else if(iIdx==MenuActionConfigurePlugin)
	{
		PluginSystem::Instance()->ShowGui();
	}
	else if(iIdx==MenuActionProgramInfo)
	{

	}
}

Frame* MainFrame::GetFrame()
{
	return this->frame;
}

void	MainFrame::AddFrame(Frame* iFrame)
{
	this->frames.push_back(iFrame);
}
void	MainFrame::RemoveFrame(Frame* iFrame)
{
	this->frames.remove(iFrame);
}

void	MainFrame::Enable(bool iEnable)
{
	for(std::list<Frame*>::iterator i=this->frames.begin();i!=this->frames.end();i++)
		(*i)->windowData->Enable(iEnable);
}

//////////////////GuiCaret///////////////////


GuiCaret*	GuiCaret::Instance()
{
	GLOBALGETTERFUNCASSINGLETON(GlobalCaretInstance,GuiCaret);
}

void	GuiCaret::Draw(Frame* iFrame)
{
	GuiRect*			tRect=this->rect;

	PaintData			tPd;
	GuiRect*			pRect=tRect->Parent();
	std::list<GuiRect*> tAncestors;

	vec2 tPos=this->pos;
	vec2 tDim=this->dim;

	vec2 p1=tPos;
	vec2 p2(p1.x,p1.y+tDim.y);

	bool tBlink=this->blink;

	while(pRect)tAncestors.push_front(pRect),pRect=pRect->Parent();

	tAncestors.push_back(tRect);

	for(std::list<GuiRect*>::const_iterator i=tAncestors.begin();i!=tAncestors.end();i++)
		(*i)->Procedure(iFrame,GuiRectMessages::ONPREPAINT,&tPd);

	if(iFrame->BeginDraw(this))
	{
		iFrame->renderer2D->SetAntialiasing(false);
		iFrame->renderer2D->PushScissor(tPd.clip.x,tPd.clip.y,tPd.clip.z,tPd.clip.w);
		iFrame->renderer2D->Translate(tPd.offset.x,tPd.offset.y);

		if(!tBlink)
			iFrame->renderer2D->DrawLine(p1,p2,this->colorback,tDim.x);
		else
			iFrame->renderer2D->DrawLine(p1,p2,this->colorfront,tDim.x);

		iFrame->renderer2D->Translate(0,0);
		iFrame->renderer2D->PopScissor();
		iFrame->renderer2D->SetAntialiasing(true);

		iFrame->EndDraw();
	}
}

void	GuiCaret::CheckDraw(Frame* iFrame)
{
	if(iFrame && this->rect)
	{
		if(Timer::Instance()->GetCurrent()-this->lasttime > this->blinktime)
		{
			iFrame->windowData->SendMessage(0,0,(int)1);

			this->lasttime=Timer::Instance()->GetCurrent();
			this->blink=!this->blink;
		}
	}
}


const vec2& GuiCaret::Get()
{
	return this->pos;
}


void	GuiCaret::SetColors(unsigned int tBack,unsigned int tFront)
{
	this->colorback=tBack;
	this->colorfront=tFront;
}

//////////////////StringEditor///////////////////

StringEditor::Cursor::Cursor():p(0){}

StringEditor::StringEditor(){}

StringEditor::~StringEditor(){}

StringEditor* StringEditor::Instance()
{
	GLOBALGETTERFUNCASSINGLETON(GlobalStringEditorInstance,StringEditor);
}

void StringEditor::ParseKeyInput(String& iString,Cursor& iCursor,const KeyData& iData,GuiFont* iFont)
{
	bool			tRedraw=false;

	unsigned int	tCaretOperation=StringEditor::CARET_DONTCARE;
	void*			tCaretParameter=0;

	if(iData.key)
	{
		switch(iData.key)
		{
		case 0x08:/*VK_BACK*/tCaretOperation=StringEditor::CARET_BACKSPACE; break;
		default:
			tCaretOperation=StringEditor::CARET_ADD;
			tCaretParameter=(char*)iData.key;
		}
	}
	else
	{
		if(Keyboard::Instance()->IsPressed(0x25/*VK_LEFT*/))
			tCaretOperation=StringEditor::CARET_ARROWLEFT;
		if(Keyboard::Instance()->IsPressed(0x27/*VK_RIGHT*/))
			tCaretOperation=StringEditor::CARET_ARROWRIGHT;
		if(Keyboard::Instance()->IsPressed(0x26/*VK_UP*/))
			tCaretOperation=StringEditor::CARET_ARROWUP;
		if(Keyboard::Instance()->IsPressed(0x28/*VK_DOWN*/))
			tCaretOperation=StringEditor::CARET_ARROWDOWN;
		if(Keyboard::Instance()->IsPressed(0x03/*VK_CANCEL*/))
			tCaretOperation=StringEditor::CARET_CANCEL;
		if(Keyboard::Instance()->IsPressed(0x2E/*VK_DELETE*/))
			tCaretOperation=StringEditor::CARET_CANCEL;
		/*if(Keyboard::Instance()->IsPressed(0x1B/ *VK_ESCAPE* /)) 
		{
			StringEditor::Instance()->Enable(false);
			return false;
		}*/
	}

	bool tMustResize=StringEditor::Instance()->EditText(iString,iCursor,tCaretOperation,iFont,tCaretParameter);
}

bool StringEditor::EditText(String& iString,Cursor& iCursor,unsigned int iCaretOp,GuiFont* iFont,void* iParam)
{
	float iFontHeight=iFont->GetHeight();

	bool tMustResize=false;

	switch(iCaretOp)
	{
	case CARET_RECALC:
		{
			const wchar_t*	pText=iString.c_str();

			bool tCarriageReturn=false;
			int tCharWidth=0;

			iCursor.caret.make(0,0);

			while(*pText)
			{
				if(tCarriageReturn)
				{
					iCursor.caret.x=0;
					iCursor.caret.y+=iFontHeight;
					
					iCursor.rowcol.x+=1;
					iCursor.rowcol.y=0;

					tCarriageReturn=false;
				}

				tCharWidth=iFont->GetCharWidth(*pText);

				if(*pText=='\n' ||  *pText=='\r')
					tCarriageReturn=true;

				if(pText==iCursor.p)
					break;

				iCursor.caret.x+=tCharWidth;
				iCursor.rowcol.y+=1;

				pText++;
			}

			break;
		}
		case CARET_CANCEL:
		{
			if(iString.empty() || iCursor.p==&iString.back())
				return false;

			std::wstring::iterator sIt=iString.begin()+iCursor.rowcol.y;

			if(sIt!=iString.end())
				iString.erase(sIt);

			break;
		}
		case CARET_BACKSPACE:
		{
			if(iCursor.p==iString.c_str())
				return false;

			char tCharCode=*(--iCursor.p);

			if(tCharCode=='\n' ||  tCharCode=='\r')
			{
				//find previous row length
				const wchar_t*	pText=iCursor.p-1;
				unsigned int	tRowCharsWidth=0;
				unsigned int    tRowCharCount=0;

				while(*pText!='\n' &&  *pText!='\r')
				{
					tRowCharsWidth+=iFont->GetCharWidth(*pText);

					if(pText==iString.c_str())
						break;

					pText--;
					tRowCharCount++;

				}

				iCursor.caret.x=tRowCharsWidth;
				iCursor.caret.y-=iFontHeight;
				iCursor.rowcol.x--;
				iCursor.rowcol.y=tRowCharCount;

				tMustResize=true;
			}
			else
			{
				iCursor.caret.x-=iFont->GetCharWidth(tCharCode);
				iCursor.rowcol.y--;
			}

			iString.erase(iCursor.p-iString.c_str(),1);

			break;
		}
		case CARET_ADD:
		{
			char tCharcode=*(char*)iParam;

			tCharcode=='\r' ? tCharcode='\n' : 0;

			if(tCharcode=='\n' || tCharcode=='\r')
			{
				iCursor.caret.x=0;
				iCursor.caret.y+=iFontHeight;
				iCursor.rowcol.x++;
				iCursor.rowcol.y=0;
				tMustResize=true;
			}
			else
			{
				iCursor.caret.x+=iFont->GetCharWidth(tCharcode);
				iCursor.rowcol.y++;
			}

			size_t tPosition=iCursor.p-iString.c_str();

			iString.insert(tPosition,1,tCharcode);
			iCursor.p=iString.c_str()+tPosition+1;

			break;
		}
		case CARET_ARROWLEFT:
		{
			if(iCursor.p==iString.c_str())
				return false;

			char tCharCode=*(--iCursor.p);

			if(tCharCode=='\n' ||  tCharCode=='\r')
			{
				const wchar_t*	pText=iCursor.p;
				unsigned int	tRowLenght=0;
				unsigned int    tRowCharCount=0;

				//find previous row lengthc

				while(true)
				{
					if(iCursor.p!=pText && (*pText=='\n' || *pText=='\r'))
						break;

					tRowLenght+=iFont->GetCharWidth(*pText);

					if(pText==iString.c_str())
						break;

					pText--;
					tRowCharCount++;
				}

				iCursor.caret.x=tRowLenght;
				iCursor.caret.y-=iFontHeight;
				iCursor.rowcol.x--;
				iCursor.rowcol.y=tRowCharCount;
			}
			else
			{
				iCursor.caret.x-=iFont->GetCharWidth(tCharCode);
				iCursor.rowcol.y--;
			}

			break;
		}
		case CARET_ARROWRIGHT:
		{
			char tCharCode=*iCursor.p;

			if(tCharCode=='\0')
				break;

			if(tCharCode=='\n' || tCharCode=='\r')
			{
				iCursor.caret.x=0;
				iCursor.caret.y+=iFontHeight;
				iCursor.rowcol.x++;
				iCursor.rowcol.y=0;
			}
			else
			{
				iCursor.caret.x+=iFont->GetCharWidth(tCharCode);
				iCursor.rowcol.y++;
			}

			iCursor.p++;

			break;
		}
		case CARET_ARROWUP:
		{
			unsigned int tRowCharWidth=0;
			unsigned int tColumn=0;

			//---find current rowhead

			const wchar_t* pText=iCursor.p;

			//skip the tail of the current row if present
			if(*pText=='\r' || *pText=='\n')
				pText--;

			//find the last upper row or the head of the current row
			while( pText!=iString.c_str() && *pText!='\r' &&  *pText!='\n' )
				pText--;

			//return if no previous row exists
			if(pText==iString.c_str())
				return false;

			//go to the upper row pre-carriage char
			pText--;

			//find the last upper superior row or the head of the upper row
			while( pText!=iString.c_str() && *pText!='\r' &&  *pText!='\n' )
				pText--;

			//go to the upper superior row pre-carriage char
			if(pText!=iString.c_str())
				pText++;

			//finally found the upper matching position
			while( tColumn!=iCursor.rowcol.y && *pText!='\0' && *pText!='\r' && *pText!='\n' )
			{
				tRowCharWidth+=iFont->GetCharWidth(*pText);

				pText++;
				tColumn++;
			}

			iCursor.p=pText;
			iCursor.caret.x=tRowCharWidth;
			iCursor.caret.y-=iFontHeight;
			iCursor.rowcol.y=tColumn;
			iCursor.rowcol.x--;

			break;
		}
		case CARET_ARROWDOWN:
		{
			unsigned int tRowCharWidth=0;
			unsigned int tColumn=0;

			//find current rowtail

			const wchar_t* pText=iCursor.p;

			while( *pText!='\0' && *pText!='\r' && *pText!='\n' )
			{
				iCursor.p++;
				pText++;
			}

			if(*pText=='\0')
				return false; //no previous row exists

			pText++;
			iCursor.p++;

			//finally found the lower matching position

			while( tColumn!=iCursor.rowcol.y && *pText!='\0' && *pText!='\r' && *pText!='\n' )
			{
				tRowCharWidth+=iFont->GetCharWidth(*pText);

				pText++;
				iCursor.p++;
				tColumn++;
			}

			/*if(tColumn!=this->string->cursor->rowcol.y)//string is shorter of the lower matching position
				tRowCharWidth*/

			iCursor.caret.x=tRowCharWidth;
			iCursor.caret.y+=iFontHeight;
			iCursor.rowcol.y=tColumn;
			iCursor.rowcol.x++;

			break;
		}
	}

	//wprintf(L"cursor: %d,col: %d\n",iString.c_str(),iCursor.rowcol.y);
}

//////////////////Renderer2D///////////////////

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
		return vec2();

	float fontHeight=height=this->height;

	wchar_t* t=(wchar_t*)iText;

	while(*t)
	{
		float tW;

		if(*t=='\n' || *t=='\r')
		{
			height+=fontHeight;
			width=std::max<float>(width,tWidth);
			tWidth=0;
		}
		else
		{
			tW=this->widths[*t];
			tWidth+=tW;
		}

		t++;
	}

	width=std::max<float>(width,tWidth);

	return vec2(width,height);
}

vec4 GuiFont::MeasureText2(const wchar_t* iText)const
{
	float width=0,tWidth=0;
	float height=0;
	unsigned int rows=0;
	unsigned int cols=0;

	if(!iText)
		return vec4();

	float fontHeight=height=this->height;

	wchar_t* t=(wchar_t*)iText;

	unsigned int tCol=0;

	while(*t)
	{
		float tW=0;

		if(*t=='\n' || *t=='\r')
		{
			height+=fontHeight;
			width=std::max<float>(width,tWidth);
			tWidth=0;

			rows++;

			cols=std::max<unsigned int>(cols,tCol);
			tCol=0;
		}
		else
		{
			tW=this->widths[*t];
			tWidth+=tW;
			tCol++;
		}

		t++;
	}
		
	cols=std::max<unsigned int>(cols,tCol);
	width=std::max<float>(width,tWidth);

	return vec4(width,height,cols,rows);
};

//////////////////Renderer2D///////////////////

Renderer2D::Renderer2D(Frame* iFrame):
	frame(iFrame),
	tabSpaces(4)
{
	
}

Renderer2D::~Renderer2D(){}

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////Renderer3DInterface///////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

Renderer3D::Renderer3D(Frame* iFrame):
	frame(iFrame)
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

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////GuiRect////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

void GuiRect::SetFlag(GuiRectFlag iState,bool iValue){this->flags ^= (-iValue^ this->flags) & (1UL << iState);}
bool GuiRect::GetFlag(GuiRectFlag iState){return (this->flags >> iState) & 1U;}

void		GuiRect::SetColor(unsigned iColor){this->color=iColor;}
unsigned	GuiRect::GetColor(){return this->color;}

GuiViewer* GuiRect::GetRoot()
{
	return this->parent ? this->parent->GetRoot() : (GuiViewer*)this;
}

void GuiRect::Append(GuiRect* iChild,bool isChild)
{
	this->childs.push_back(iChild);
	iChild->parent=this;
	iChild->SetFlag(GuiRectFlag::CHILD,isChild);
	iChild->SetFlag(GuiRectFlag::PARENTCOLOR,this->GetFlag(GuiRectFlag::PARENTCOLOR));
	iChild->SetFlag(GuiRectFlag::SELFDRAW,this->GetFlag(SELFDRAW));
}

void GuiRect::Remove(GuiRect* iChild)
{
	this->childs.remove(iChild);
	iChild->parent=0;
}

void GuiRect::OnSize(Frame* iFrame,const vec4& iNewSize)
{
	this->edges=iNewSize;
}

void GuiRect::Draw(Frame* iFrame)
{
	if(this->GetFlag(GuiRectFlag::SELFDRAW))
		iFrame->SetDraw(this);
}

GuiRect::GuiRect(unsigned int iState,unsigned int iColor):parent(0),flags(iState),color(iColor){}

void GuiRect::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		case ONHITTEST:
		{
			HitTestData& thtd=*(HitTestData*)iData;

			if(EdgesContainsPoint(this->edges,thtd.mouse))
				thtd.hit=this;
		}
		break;
		case ONMOUSEWHEEL:
		{
			iFrame->Message(this->parent,ONMOUSEWHEEL,iData);
		}
		break;
		case ONCONTROLEVENT:
		{
			iFrame->Message(this->parent,ONCONTROLEVENT,iData);
		}
		break;
		case ONPAINT:
		{
			bool tParentColor=this->GetFlag(GuiRectFlag::PARENTCOLOR);
			unsigned int tColor=(tParentColor && this->parent) ? this->parent->color : this->color;
			iFrame->renderer2D->DrawRectangle(this->edges.x,this->edges.y,this->edges.z,this->edges.w,tColor);
		}
		break;
		case ONMOUSEDOWN:
		{
			MouseData& md=*(MouseData*)iData;

			if(md.button==1)
				iFrame->SetFocus(this);
		}
		break;
	}
}

////////////////////GuiViewer//////////////////

GuiViewer::GuiViewer():tab(0),frame(0),labelsend(0)
{
	GlobalViewersInstance().push_back(this);
}

GuiViewer::GuiViewer(float x,float y,float z,float w):tab(0),frame(0)
{
	GlobalViewersInstance().push_back(this);
	this->edges.make(x,y,z,w);
}

GuiViewer::~GuiViewer()
{
	GlobalViewersInstance().remove(this);
}

void GuiViewer::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		case ONHITTEST:
		{
			HitTestData& thtd=*(HitTestData*)iData;

			vec4 tSizeEdges=this->GetBorderEdges();

			if(EdgesContainsPoint(tSizeEdges,thtd.mouse))
			{
				GuiRect::Procedure(iFrame,iMsg,iData);

				if(thtd.hit!=this)
				{
					if		(thtd.mouse.x<this->edges.x && thtd.mouse.y<this->edges.y)//upperleft
						thtd.dragrect[3]=this,iFrame->dragcode=1;
					else if (thtd.mouse.x<this->edges.x && thtd.mouse.y>this->edges.w)//bottomleft
						thtd.dragrect[1]=this,iFrame->dragcode=1;
					else if (thtd.mouse.x>this->edges.z && thtd.mouse.y<this->edges.y)//upperright
						thtd.dragrect[2]=this,iFrame->dragcode=1;
					else if (thtd.mouse.x>this->edges.z && thtd.mouse.y>this->edges.w)//bottomright
						thtd.dragrect[0]=this,iFrame->dragcode=1;
					else if (thtd.mouse.x<this->edges.x)//left
						thtd.dragrect[1]=this,iFrame->dragcode=2;
					else if (thtd.mouse.x>this->edges.z)//right
						thtd.dragrect[0]=this,iFrame->dragcode=2;
					else if (thtd.mouse.y<this->edges.y)//top
						thtd.dragrect[2]=this,iFrame->dragcode=3;
					else if (thtd.mouse.y>this->edges.w)//bottom
						thtd.dragrect[0]=this,iFrame->dragcode=3;
					else
						iFrame->dragcode=0;

					thtd.hit=0;
				}
			}
			iFrame->BroadcastTo(this->tab,iMsg,iData);
		}
		break;
		case ONMOUSEENTER:
		case ONMOUSEEXIT:
			GuiRect::Procedure(iFrame,iMsg,iData);
		break;
		case ONSIZE:
		{
			GuiRect::Procedure(iFrame,iMsg,iData);

			vec4	tTabEdges=this->GetTabEdges();

			if(this->tab)
				this->tab->OnSize(iFrame,tTabEdges);

			iFrame->BroadcastTo(this->tab,iMsg,&tTabEdges);
		}
		break;
		case ONMOUSEUP:
		{
			GuiRect::Procedure(iFrame,iMsg,iData);

			MouseData& tmd=*(MouseData*)iData;

			switch(tmd.button)
			{
				case 1:
				{
					if(EdgesContainsPoint(this->GetLabelsArea(),tmd.mouse))
					{
						std::list<vec4>::iterator		i=this->rects.begin();
						std::list<GuiRect*>::iterator	r=this->tabs.begin();

						for(;i!=this->rects.end();i++,r++)
						{
							vec4 tLabelEdges=*i;

							tLabelEdges.x+=this->edges.x;
							tLabelEdges.z+=this->edges.x;
							tLabelEdges.y+=this->edges.y;
							tLabelEdges.w+=this->edges.y;

							if(EdgesContainsPoint(tLabelEdges,tmd.mouse) && this->GetTab()!=*r)
							{
								this->SetTab(*r);
								this->frame->SetDraw(this);
							}
						}
					}
				}
				break;
				case 3:
				{
					if(EdgesContainsPoint(this->GetLabelsArea(),tmd.mouse))
					{
						int tabNumberHasChanged=this->tabs.size();

						for(std::list<vec4>::iterator i=this->rects.begin();i!=this->rects.end();i++)
						{
							vec4 tLabelEdges=*i;

							tLabelEdges.x+=this->edges.x;
							tLabelEdges.z+=this->edges.x;
							tLabelEdges.y+=this->edges.y;
							tLabelEdges.w+=this->edges.y;

							if(EdgesContainsPoint(tLabelEdges,tmd.mouse))
							{
								int menuResult=this->frame->TrackTabMenuPopup();

								switch(menuResult)
								{
									case 1:/*if(this->tabs.size()>1){this->Destroy();}*/break;
									case 2:/*this->selected=this->rects.Append(new GuiViewport())*/break;
									case 3:this->AddTab(GuiScene::Instance(),L"Scene");break;
									case 4:this->AddTab(GuiEntity::Instance(),L"Properties");break;
									case 5:this->AddTab(GuiProject::Instance(),L"Project");break;
								}
							}
						}

						if(tabNumberHasChanged!=this->tabs.size())
							this->frame->SetDraw(this);
					}
				}
				break;
			}

			

			iFrame->BroadcastTo(this->tab,iMsg,iData);
		}
		break;
		case ONPAINT:
		{
			iFrame->renderer2D->DrawRectangle(this->edges.x,this->edges.y,this->edges.z,this->edges.w,GuiViewer::COLOR_BACK);

			//render label text
			std::list<String>::iterator labelIter=this->labels.begin();
			std::list<GuiRect*>::iterator tabIter=this->tabs.begin();
			std::list<vec4>::iterator rectIter=this->rects.begin();
			for(;labelIter!=this->labels.end();labelIter++,tabIter++,rectIter++)
			{
				vec4 tLabel=*rectIter;

				tLabel.x+=this->edges.x;
				tLabel.z+=this->edges.x;
				tLabel.y+=this->edges.y;
				tLabel.w+=this->edges.y;

				if(this->tab==*tabIter)
					this->frame->renderer2D->DrawRectangle(tLabel.x,tLabel.y,tLabel.z,tLabel.w,Frame::COLOR_LABEL);

				this->frame->renderer2D->DrawText(*labelIter,tLabel.x,tLabel.y,tLabel.z,tLabel.w,vec2(0.5f,0.5f),vec2(0.5f,0.5f),GuiString::COLOR_TEXT);
			}

			this->frame->renderer2D->DrawLine(vec2(this->edges.x,this->edges.y+BAR_HEIGHT-1),vec2(this->edges.z,this->edges.y+BAR_HEIGHT-1),Frame::COLOR_LABEL,2);

			iFrame->BroadcastTo(this->tab,iMsg,iData);
		}
		break;
		case ONPREPAINT:
		{
			PaintData& pd=*(PaintData*)iData;

			vec4 tTabEdges=this->GetTabEdges();

			pd.clip.x=std::max<float>(pd.clip.x,tTabEdges.x);
			pd.clip.y=std::max<float>(pd.clip.y,tTabEdges.y);
			pd.clip.z=std::min<float>(pd.clip.z,tTabEdges.z);
			pd.clip.w=std::min<float>(pd.clip.w,tTabEdges.w);
		}	
		break;
		default:
		{
			GuiRect::Procedure(iFrame,iMsg,iData);
			iFrame->BroadcastTo(this->tab,iMsg,iData);
		}
	}
}


GuiViewer*	GuiViewer::Instance()
{
	return new GuiViewer;
}

std::list<GuiViewer*>&	GuiViewer::GetPool()
{
	return GlobalViewersInstance();
}

const std::list<GuiRect*>&      GuiViewer::GetTabs()
{
	return this->tabs;
}

const std::list<String>&        GuiViewer::GetLabels()
{
	return this->labels;
}

GuiRect* GuiViewer::AddTab(GuiRect* iTab,String iLabel)
{
	iTab->parent=this;
	this->tabs.push_back(iTab);
	this->labels.push_back(iLabel);

	this->UpdateLabelRects();

	this->SetTab(iTab);
	return iTab;
}

void GuiViewer::RemoveTab(GuiRect* iTab)
{
	this->tabs.remove(iTab);
	this->UpdateLabelRects();
	iTab->parent=0;
}

void GuiViewer::SetTab(GuiRect* iTab)
{
	if(this->tab && this->tab!=iTab)
	{
		if(this->tab->GetFlag(GuiRectFlag::ACTIVE))
		{
			this->frame->Message(this->tab,GuiRectMessages::ONDEACTIVATE);
			this->tab->SetFlag(GuiRectFlag::ACTIVE,false);
		}
	}

	this->tab=iTab;

	if(iTab)
	{
		if(!iTab->GetFlag(GuiRectFlag::ACTIVE))
		{
			this->frame->Message(iTab,GuiRectMessages::ONACTIVATE);
			iTab->SetFlag(GuiRectFlag::ACTIVE,true);
		}
		this->frame->Message(this,GuiRectMessages::ONSIZE);
	}

	this->frame->SetDraw(this);
}

GuiRect* GuiViewer::GetTab()
{
	return tab;
}

vec4 GuiViewer::GetTabEdges()
{
	return vec4(this->edges.x,this->edges.y+BAR_HEIGHT,this->edges.z,this->edges.w);
}

vec4 GuiViewer::GetBorderEdges()
{
	return vec4(this->edges.x-4,this->edges.y-4,this->edges.z+4,this->edges.w+4);
}

vec4 GuiViewer::GetLabelsArea()
{
	float ty=this->edges.y + (BAR_HEIGHT-LABEL_HEIGHT);

	return vec4(this->edges.x,ty,this->labelsend,ty+LABEL_HEIGHT);
}

void GuiViewer::UpdateLabelRects()
{
	this->rects.clear();
	this->labelsend=0;

	vec4 tLabelRect;

	tLabelRect.x=0;
	tLabelRect.y=BAR_HEIGHT-LABEL_HEIGHT;
	tLabelRect.z=tLabelRect.x+LABEL_LEFT_OFFSET;
	tLabelRect.w=tLabelRect.y+LABEL_HEIGHT;

	for(std::list<String>::iterator i=this->labels.begin();i!=this->labels.end();i++)
	{
		vec2 tTextSize = GuiFont::GetDefaultFont()->MeasureText((*i).c_str());

		tLabelRect.x=tLabelRect.z;
		tLabelRect.z=tLabelRect.x + tTextSize.x + LABEL_RIGHT_OFFSET;

		this->rects.push_back(tLabelRect);
	}

	this->labelsend=tLabelRect.z;
}


Frame* GuiViewer::GetFrame()
{
	return this->frame;
}

////////////////////GuiListBoxNode////////////////

GuiListBoxNode::GuiListBoxNode():flags(0){}
GuiListBoxNode::GuiListBoxNode(const String& iLabel):flags(0),label(iLabel){}

GuiListBox::GuiListBox():hovered(0){}

void			GuiListBoxNode::SetLabel(const String& iString)
{
	this->label=iString;
}

const String&	GuiListBoxNode::GetLabel()
{
	return this->label;
}

float GuiListBoxNode::GetWidth()
{
	return GuiFont::GetDefaultFont()->MeasureText(this->label.c_str()).x;
}

float GuiListBoxNode::GetHeight()
{
	return Frame::ICON_WH;
}

void  GuiListBoxNode::OnPaint(GuiListBox*,Frame* iFrame,const vec4& iEdges)
{
	iFrame->renderer2D->DrawText(this->label,iEdges.x,iEdges.y,iEdges.z,iEdges.w,vec2(0,0.5f),vec2(0,0.5f),0xffffffff);
}

void  GuiListBoxNode::OnMouseDown(GuiListBox*,Frame*,const vec4& iEdges,const MouseData&)
{

}
void  GuiListBoxNode::OnMouseUp(GuiListBox*,Frame*,const vec4& iEdges,const MouseData&)
{

}
void  GuiListBoxNode::OnMouseMove(GuiListBox*,Frame*,const vec4& iEdges,const MouseData&)
{

}

////////////////////GuiListBox////////////////

void GuiListBox::InsertItem(GuiListBoxNode& iItem)
{
	this->items.push_back(&iItem);
	this->CalculateLayout();
}

void GuiListBox::InsertItems(const std::list<GuiListBoxNode*>& iItems)
{
	this->items=iItems;
	this->CalculateLayout();
}

void GuiListBox::RemoveItem(GuiListBoxNode& iItem)	
{
	this->items.remove(&iItem);
	if(this->hovered==&iItem)
		this->hovered=0;
	this->CalculateLayout();
}

void GuiListBox::RemoveAll()	
{
	this->items.clear();
	this->hovered=0;
	this->CalculateLayout();
}

void GuiListBox::SetFlag(GuiListBoxNode* iItem,GuiListBoxNode::Flags iFlag,bool iValue)
{
	::SetFlag(iItem->flags,iFlag,iValue);
}

bool GuiListBox::GetFlag(GuiListBoxNode* iItem,GuiListBoxNode::Flags iFlag)
{
	return ::GetFlag(iItem->flags,iFlag);
}

GuiListBoxNode* GuiListBox::GetHoveredNode()
{
	return this->hovered;
}

void GuiListBox::SetHoverHighlight(Frame* iFrame,bool iValue)
{
	this->SetFlag(this->hovered,GuiListBoxNode::HIGHLIGHTED,iValue);
	iFrame->SetDraw(this,true,false,this->hovered);
}

GuiListBox::GuiListBoxData GuiListBox::GetListBoxData(void* iData)
{
	vec4	tContentEdges=this->GetContentEdges();
	vec4	tFirstItemEdges(tContentEdges.x,tContentEdges.y,tContentEdges.z,tContentEdges.y);

	GuiListBoxData tTreeViewData={tContentEdges,tFirstItemEdges,0,0,iData,false};

	return tTreeViewData;
}

void GuiListBox::SetLabelEdges(GuiListBoxNode* iItem,GuiListBoxData& iTvdata)
{
	iTvdata.labeledges.y=iTvdata.labeledges.w;
	iTvdata.labeledges.w=iTvdata.labeledges.y+iItem->GetHeight();
}

void GuiListBox::CalculateLayout()	
{
	this->ResetContent();

	vec4		tClippingEdges=this->GetClippingEdges();
	float		tTop=this->edges.y;

	for(std::list<GuiListBoxNode*>::iterator i=this->items.begin();i!=this->items.end();i++)
	{
		float tWidth=(*i)->GetWidth();
		tTop+=(*i)->GetHeight();
		
		this->SetContent(tWidth,tTop-this->edges.y);
	}

	this->CalculateScrollbarsLayout();
}

void GuiListBox::ItemProcedure(GuiListBoxNode* iItem,Frame* iFrame,GuiRectMessages iMsg,GuiListBoxData& iLbid)
{
	switch(iMsg)
	{
		case ONHITTEST:
		{
			HitTestData& thtd=*(HitTestData*)iLbid.data;

			if(EdgesContainsPoint(iLbid.labeledges,thtd.mouse))
			{
				iLbid.hit=iItem;
				iLbid.skip=true;
			}
		}
		break;
		case ONPAINT:
		{
			bool tHighligh=this->GetFlag(iItem,GuiListBoxNode::HIGHLIGHTED);
			bool tSelected=this->GetFlag(iItem,GuiListBoxNode::SELECTED);

			unsigned int tColor=(iLbid.idx%2) ? 0x555555 : 0x606060;

			if(tSelected)
				tColor=BlendColor(0x0000ff,tColor);
			if(tHighligh)
				tColor=BlendColor(0x00ffff,tColor);

			//base rect
			iFrame->renderer2D->DrawRectangle(iLbid.labeledges.x,iLbid.labeledges.y,iLbid.labeledges.z,iLbid.labeledges.w,tColor);

			iItem->OnPaint(this,iFrame,iLbid.labeledges);
		}
		break;
	}
}

void GuiListBox::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		case ONHITTEST:
		{
			HitTestData& thtd=*(HitTestData*)iData;

			GuiScrollRect::Procedure(iFrame,iMsg,iData);

			if(thtd.hit==this)
			{
				if(this->scrollerhit==0 && this->scrollerpressed==0)
				{
					vec4			tContentEdges=this->GetContentEdges();
					GuiListBoxData	tLbid=GetListBoxData(&thtd);

					for(std::list<GuiListBoxNode*>::iterator i=this->items.begin();i!=this->items.end() && !tLbid.skip;i++)
					{
						this->SetLabelEdges(*i,tLbid);
						this->ItemProcedure(*i,iFrame,iMsg,tLbid);
						tLbid.idx++;
					}

					//draw previous hitted node
					if(this->hovered)
						this->SetHoverHighlight(iFrame,false);

					if(tLbid.hit)
					{
						//draw current hitted node
						this->hovered=tLbid.hit;
						this->hoverededges=tLbid.labeledges;
						this->SetHoverHighlight(iFrame,true);
					}
					else
						this->hovered=0;
				}
				else if(this->hovered)
				{
					this->SetHoverHighlight(iFrame,false);
					this->hovered=0;
				}
			}
		}
		break;
		case ONMOUSEUP:
		{
			GuiScrollRect::Procedure(iFrame,iMsg,iData);

			if(this->hovered)
				this->hovered->OnMouseUp(this,iFrame,this->hoverededges,*(MouseData*)iData);
		}
		break;
		case ONMOUSEDOWN:
		{
			GuiScrollRect::Procedure(iFrame,iMsg,iData);

			if(this->hovered)
				this->hovered->OnMouseDown(this,iFrame,this->hoverededges,*(MouseData*)iData);
		}
		break;
		case ONMOUSEMOVE:
		{
			GuiScrollRect::Procedure(iFrame,iMsg,iData);

			if(this->hovered)
				this->hovered->OnMouseMove(this,iFrame,this->hoverededges,*(MouseData*)iData);
		}
		break;
		case ONMOUSEEXIT:
		{
			//draw previous hitted node
			if(this->hovered)
			{
				this->SetHoverHighlight(iFrame,false);
				this->hovered=0;
			}
		}
		break;
		case ONPAINT:
		{
			PaintData& pd=*(PaintData*)iData;

			if(pd.data==0)GuiRect::Procedure(iFrame,iMsg,iData);

			vec4 tClippingEdges=this->GetClippingEdges();
			vec4 tContentEdges=this->GetContentEdges();

			float tOffsetx=this->GetScrollbarPosition(0);
			float tOffsety=this->GetScrollbarPosition(1);

			iFrame->renderer2D->PushScissor(tClippingEdges.x,tClippingEdges.y,tClippingEdges.z,tClippingEdges.w);
			iFrame->renderer2D->Translate(-tOffsetx,-tOffsety);

			GuiListBoxData tLbid=this->GetListBoxData(pd.data);

			for(std::list<GuiListBoxNode*>::iterator i=this->items.begin();i!=this->items.end() && !tLbid.skip;i++)
			{
				this->SetLabelEdges(*i,tLbid);

				if(pd.data==0 || pd.data==*i)
					this->ItemProcedure(*i,iFrame,iMsg,tLbid);

				tLbid.idx++;
			}

			iFrame->renderer2D->Translate(0,0);
			iFrame->renderer2D->PopScissor();

			if(pd.data==0)GuiScrollRect::Procedure(iFrame,iMsg,iData);
		}
		break;
	default:
		GuiScrollRect::Procedure(iFrame,iMsg,iData);
	}
}

////////////////GuiTreeViewNode////////////////

GuiTreeViewNode::GuiTreeViewNode()
{
	this->parent=0;
	this->flags=0;
	::SetFlag(this->flags,GuiTreeViewNode::EXPANDED,true);
}

void GuiTreeViewNode::Reset()
{
	this->parent=0;
	this->childs.clear();
	this->flags=0;
}

void			GuiTreeViewNode::SetLabel(const String& iLabel){this->label=iLabel;}
const String&	GuiTreeViewNode::GetLabel(){return this->label;}

void GuiTreeViewNode::Insert(GuiTreeViewNode& iChild)
{
	this->childs.push_back(&iChild);
	iChild.parent=this;
}

void GuiTreeViewNode::Remove(GuiTreeViewNode& iItem)
{
	this->childs.remove(&iItem);
	iItem.parent=0;
}

void  GuiTreeViewNode::OnPaint(Frame* iFrame,const vec4& iEdges,const float& tExpandosEnd)
{
	iFrame->renderer2D->DrawText(this->label,tExpandosEnd,iEdges.y,iEdges.z,iEdges.w,vec2(0,0.5f),vec2(0,0.5f),0xffffffff);
}
float GuiTreeViewNode::GetWidth(const float& tExpandosEnd)
{
	return tExpandosEnd + GuiFont::GetDefaultFont()->MeasureText(this->label.c_str()).x;
}
float GuiTreeViewNode::GetHeight()
{
	return Frame::ICON_WH;
}
void  GuiTreeViewNode::OnExpandos(Frame*,const bool&){}
void  GuiTreeViewNode::OnMouseDown(Frame*,const MouseData&){}
void  GuiTreeViewNode::OnMouseUp(Frame*,const MouseData&){}

////////////////GuiTreeView////////////////

GuiTreeView::GuiTreeView():root(0),hovered(0){}

void GuiTreeView::InsertRoot(GuiTreeViewNode& iRoot)
{
	this->root=&iRoot;
	this->SetFlag(this->root,GuiTreeViewNode::EXPANDED,true);
	this->CalculateLayout();
}

void GuiTreeView::RemoveRoot()	
{
	this->root=0;
	this->CalculateLayout();
}


void GuiTreeView::SetFlag(GuiTreeViewNode* iItem,GuiTreeViewNode::Flags iFlag,bool iValue)
{
	::SetFlag(iItem->flags,iFlag,iValue);
}

bool GuiTreeView::GetFlag(GuiTreeViewNode* iItem,GuiTreeViewNode::Flags iFlag)
{
	return ::GetFlag(iItem->flags,iFlag);
}

void GuiTreeView::SetLabelEdges(GuiTreeViewNode* iItem,GuiTreeViewData& iTvdata)
{
	iTvdata.labeledges.y=iTvdata.labeledges.w;
	iTvdata.labeledges.w=iTvdata.labeledges.y+iItem->GetHeight();
}

GuiTreeView::GuiTreeViewData GuiTreeView::GetTreeViewData(void* iData)
{
	vec4	tContentEdges=this->GetContentEdges();
	vec4	tFirstItemEdges(tContentEdges.x,tContentEdges.y,tContentEdges.z,tContentEdges.y);

	GuiTreeViewData tTreeViewData={0,tContentEdges,tFirstItemEdges,0,0,iData,false};

	return tTreeViewData;
}

void GuiTreeView::CalculateLayout()	
{
	Frame* tFrame=this->GetRoot()->GetFrame();
	this->ResetContent();
	GuiTreeViewData tTreeViewData=this->GetTreeViewData();
	this->ItemLayout(tFrame,this->root,tTreeViewData);
	this->CalculateScrollbarsLayout();
}

GuiTreeViewNode* GuiTreeView::GetTreeViewHoveredNode()
{
	return this->hovered;
}

GuiTreeViewNode* GuiTreeView::GetTreeViewRootNode()
{
	return this->root;
}

void GuiTreeView::ItemLayout(Frame* iFrame,GuiTreeViewNode* iItem,GuiTreeViewData& iTvdata)
{
	if(iItem)
	{
		float	tWidth=iItem->GetWidth(iTvdata.level*20+(iItem->childs.size() ? 20 : 0));

		this->SetLabelEdges(iItem,iTvdata);
		
		this->SetContent(tWidth,iTvdata.labeledges.w-this->edges.y);

		if(this->GetFlag(iItem,GuiTreeViewNode::EXPANDED) && iItem->childs.size())
		{
			iTvdata.level++;

			for(std::list<GuiTreeViewNode*>::iterator i=iItem->childs.begin();i!=iItem->childs.end();i++)
				this->ItemLayout(iFrame,*i,iTvdata);
		}
	}
}

void GuiTreeView::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		case ONHITTEST:
		{
			HitTestData& thtd=*(HitTestData*)iData;

			GuiScrollRect::Procedure(iFrame,iMsg,iData);

			if(thtd.hit==this)
			{
				if(this->scrollerhit==0 && this->scrollerpressed==0)
				{
					GuiTreeViewData tTreeViewData=this->GetTreeViewData(iData);
					this->ItemRoll(this->root,iFrame,GuiRectMessages::ONHITTEST,tTreeViewData);

					if(this->hovered)
					{
						this->SetFlag(this->hovered,GuiTreeViewNode::HIGHLIGHTED,false);
						iFrame->SetDraw(this,true,false,this->hovered);
					}

					if(tTreeViewData.hit)
					{
						//draw current hitted node
						this->hovered=tTreeViewData.hit;
						this->hoveredg=tTreeViewData.labeledges;
						this->hoverlvl=tTreeViewData.level;

						this->SetFlag(this->hovered,GuiTreeViewNode::HIGHLIGHTED,true);
						iFrame->SetDraw(this,true,false,this->hovered);
					}
					else
						this->hovered=0;
				}
				else if(this->hovered)
				{
					this->SetFlag(this->hovered,GuiTreeViewNode::HIGHLIGHTED,false);
					iFrame->SetDraw(this,true,false,this->hovered);
					this->hovered=0;
				}
			}
		}
		break;
		case ONMOUSEUP:
		{
			GuiScrollRect::Procedure(iFrame,iMsg,iData);

			if(this->hovered)
			{
				MouseData& tmd=*(MouseData*)iData;

				float tBegin=this->hoveredg.x+this->hoverlvl*Frame::ICON_WH;
				float tEnd=tBegin+Frame::ICON_WH;

				vec4 tExpandosEdges(tBegin,this->hoveredg.y,tEnd,this->hoveredg.w);

				if(EdgesContainsPoint(tExpandosEdges,tmd.mouse))
				{
					bool tExpanded=this->GetFlag(this->hovered,GuiTreeViewNode::EXPANDED);
					this->SetFlag(this->hovered,GuiTreeViewNode::EXPANDED,!tExpanded);
					this->CalculateLayout();
					iFrame->SetDraw(this);
				}
				else
					this->hovered->OnMouseUp(iFrame,tmd);
			}
		}
		break;
		case ONMOUSEEXIT:
		{
			//draw previous hitted node
			if(this->hovered)
			{
				this->SetFlag(this->hovered,GuiTreeViewNode::HIGHLIGHTED,false);
				iFrame->SetDraw(this,true,false,this->hovered);
			}
		}
		break;
		case ONPAINT:
		{
			PaintData& pd=*(PaintData*)iData;

			GuiTreeViewData tTreeViewData=this->GetTreeViewData(pd.data);

			vec4	tClippingEdges=this->GetClippingEdges();
			vec4	tContentEdges=this->GetContentEdges();

			float tOffsetx=this->GetScrollbarPosition(0);
			float tOffsety=this->GetScrollbarPosition(1);

			if(pd.data==0)
			{
				GuiRect::Procedure(iFrame,iMsg,iData);

				float tContentBottom=(this->edges.y+this->GetContent().y)<=this->edges.w ? (this->edges.y+this->GetContent().y)-tOffsety : this->edges.w;

				iFrame->renderer2D->DrawRectangle(this->edges.x,this->edges.y,tClippingEdges.z,tContentBottom,Frame::COLOR_BACK);
			}

			iFrame->renderer2D->PushScissor(tClippingEdges.x,tClippingEdges.y,tClippingEdges.z,tClippingEdges.w);
			iFrame->renderer2D->Translate(-tOffsetx,-tOffsety);

			this->ItemRoll(this->root,iFrame,GuiRectMessages::ONPAINT,tTreeViewData);

			iFrame->renderer2D->Translate(0,0);
			iFrame->renderer2D->PopScissor();
			
			if(pd.data==0)GuiScrollRect::Procedure(iFrame,iMsg,pd.data);
		}
		break;
		default:
			GuiScrollRect::Procedure(iFrame,iMsg,iData);
	}
}

void GuiTreeView::ItemRoll(GuiTreeViewNode* iItem,Frame* iFrame,GuiRectMessages iMsg,GuiTreeViewData& iTvdata)
{
	if(iItem)
	{
		this->SetLabelEdges(iItem,iTvdata);

		switch(iMsg)
		{
		case ONPAINT:
			{
				if(!iTvdata.data || iTvdata.data==iItem)
				{
					this->ItemProcedure(iItem,iFrame,iMsg,iTvdata);

					if(iTvdata.data==iItem)
						iTvdata.skip=true;
				}
			}
			break;
		default:
			this->ItemProcedure(iItem,iFrame,iMsg,iTvdata);
		}

		if(iTvdata.skip)
			return;

		iTvdata.idx++;

		if(this->GetFlag(iItem,GuiTreeViewNode::EXPANDED) && iItem->childs.size())
		{
			iTvdata.level++;

			for(std::list<GuiTreeViewNode*>::iterator i=iItem->childs.begin();i!=iItem->childs.end() && !iTvdata.skip;i++)
				this->ItemRoll(*i,iFrame,iMsg,iTvdata);

			if(!iTvdata.skip)
				iTvdata.level--;
		}
	}
}

void GuiTreeView::ItemProcedure(GuiTreeViewNode* iItem,Frame* iFrame,GuiRectMessages iMsg,GuiTreeViewData& iTvdata)
{
	if(iItem)
	{
		switch(iMsg)
		{
			case ONHITTEST:
			{
				HitTestData& thtd=*(HitTestData*)iTvdata.data;

				vec4 tFullRow(iTvdata.contentedges.x,iTvdata.labeledges.y,iTvdata.contentedges.z,iTvdata.labeledges.w);

				if(EdgesContainsPoint(tFullRow,thtd.mouse))
				{
					iTvdata.hit=iItem;
					iTvdata.skip=true;
				}
			}
			break;
			case ONMOUSEUP:
			{
				iItem->OnMouseUp(iFrame,*(MouseData*)iTvdata.data);
			}
			break;
			case ONPAINT:
			{
				bool tHighligh=this->GetFlag(iItem,GuiTreeViewNode::HIGHLIGHTED);
				bool tSelected=this->GetFlag(iItem,GuiTreeViewNode::SELECTED);

				float tContentBegin=iTvdata.labeledges.x+iTvdata.level*20;

				unsigned int tColor=(iTvdata.idx%2) ? 0x555555 : 0x606060;

				if(tSelected)
					tColor=BlendColor(0x0000ff,tColor);
				if(tHighligh)
					tColor=BlendColor(0x00ffff,tColor);

				iItem->SetBackgroundColor(tColor);

				//base rect
				iFrame->renderer2D->DrawRectangle(iTvdata.labeledges.x,iTvdata.labeledges.y,iTvdata.labeledges.z,iTvdata.labeledges.w,tColor);
				//expandos
				if(iItem->childs.size())
				{
					iFrame->renderer2D->DrawBitmap(this->GetFlag(iItem,GuiTreeViewNode::EXPANDED) ? iFrame->iconDown : iFrame->iconRight,tContentBegin,iTvdata.labeledges.y,tContentBegin+20,iTvdata.labeledges.w);
					tContentBegin+=20;
				}

				iItem->OnPaint(iFrame,iTvdata.labeledges,tContentBegin);
			}
			break;
		}

	}
}

//////////////////GuiPropertyTree/////////////////////

void GuiPropertyTree::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		case GuiRectMessages::ONACTIVATE:
		{
			GuiTreeView::Procedure(iFrame,iMsg,iData);

			GuiTreeViewData tTreeViewData=this->GetTreeViewData();
			this->ItemRoll(this->GetTreeViewRootNode(),iFrame,iMsg,tTreeViewData);
		}
		break;
		case GuiRectMessages::ONMOUSEDOWN:
		{
			GuiTreeView::Procedure(iFrame,iMsg,iData);

			MouseData& md=*(MouseData*)iData;

			if(this->GetTreeViewHoveredNode() && md.mouse.x>this->splitter && md.mouse.x<(this->splitter+4))
				this->splitterpressed=true;

			this->splitterpressed ? iFrame->SetCursor(1) : iFrame->SetCursor(0);
		}
		break;
		case GuiRectMessages::ONMOUSEUP:
		{
			GuiTreeView::Procedure(iFrame,iMsg,iData);

			if(this->splitterpressed)
				this->splitterpressed=false;

			this->splitterpressed ? iFrame->SetCursor(1) : iFrame->SetCursor(0);
		}
		break;
		case GuiRectMessages::ONMOUSEMOVE:
		{
			GuiTreeView::Procedure(iFrame,iMsg,iData);

			MouseData& md=*(MouseData*)iData;

			bool tSplitterHovered = this->splitterpressed || (md.mouse.x>this->splitter && md.mouse.x<(this->splitter+4));

			if(tSplitterHovered)
				iFrame->SetCursor(1);

			if(this->splitterpressed && md.raw.Left()) 
			{
				vec4 tContentEdges=this->GetContentEdges();

				if(md.mouse.x>(tContentEdges.x+50) && md.mouse.x<(tContentEdges.z-50))
				{
					iFrame->SetCursor(1);

					this->splitter=md.mouse.x;

					this->Procedure(iFrame,ONSIZE);
					iFrame->SetDraw(this);
				}
			}
		}
		break;
		case GuiRectMessages::ONSIZE:
		{
			GuiTreeView::Procedure(iFrame,iMsg,iData);

			GuiTreeViewData tTreeViewData=this->GetTreeViewData(iData);
			this->ItemRoll(this->GetTreeViewRootNode(),iFrame,iMsg,tTreeViewData);
		}
		break;
	default:
		GuiTreeView::Procedure(iFrame,iMsg,iData);
	}
}

void GuiPropertyTree::CalculateLayout()	
{
	Frame* tFrame=this->GetRoot()->GetFrame();
	this->ResetContent();
	this->splitter=0;
	GuiTreeViewData tTreeViewData=this->GetTreeViewData();
	this->ItemLayout(tFrame,this->GetTreeViewRootNode(),tTreeViewData);
	this->splitter=this->GetContent().x + 10;
	this->SetContent(this->splitter+154,0);
	this->Procedure(tFrame,ONACTIVATE);
	this->Procedure(tFrame,ONSIZE);
}

void GuiPropertyTree::SetLabelEdges(GuiTreeViewNode* iItem,GuiTreeViewData& iTvdata)
{
	GuiPropertyTreeItem* tPropertyTreeItem=dynamic_cast<GuiPropertyTreeItem*>(iItem);

	GuiTreeView::SetLabelEdges(iItem,iTvdata);
	iTvdata.labeledges.z=tPropertyTreeItem ? this->splitter : iTvdata.contentedges.z;
}

void GuiPropertyTree::ItemProcedure(GuiTreeViewNode* iItem,Frame* iFrame,GuiRectMessages iMsg,GuiTreeViewData& iGtvd)
{
	GuiPropertyTreeItem* tTreeItem=dynamic_cast<GuiPropertyTreeItem*>(iItem);

	switch(iMsg)
	{
		case ONACTIVATE:
		{
			GuiTreeView::ItemProcedure(iItem,iFrame,iMsg,iGtvd);

			if(tTreeItem)
				tTreeItem->GetProperty()->parent=this;
		}
		break;
		case ONSIZE:
		{
			GuiTreeView::ItemProcedure(iItem,iFrame,iMsg,iGtvd);

			if(tTreeItem)
			{
				tTreeItem->GetProperty()->edges.make(this->splitter+4,iGtvd.labeledges.y,iGtvd.contentedges.z,iGtvd.labeledges.w);
				iFrame->BroadcastTo(tTreeItem->GetProperty(),ONSIZE);
			}
		}
		break;
		case ONPAINT:
			GuiTreeView::ItemProcedure(iItem,iFrame,iMsg,iGtvd);
			if(tTreeItem)
				iFrame->BroadcastTo(tTreeItem->GetProperty(),iMsg);
		break;
		default:
			GuiTreeView::ItemProcedure(iItem,iFrame,iMsg,iGtvd);
			if(tTreeItem)
				iFrame->BroadcastTo(tTreeItem->GetProperty(),iMsg,iGtvd.data);
	}
}


//////////////////Compiler/////////////////////

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
	Compiler::COMPILER& cplr=this->compilers[iCompiler];

	String tOutputModule=iScript->module + L"\\" + iScript->scriptpath.Name() + L".dll ";
	String tScriptSource=Ide::Instance()->folderProject + L"\\" + iScript->scriptpath.File();
	String tIdeSourcePath=this->ideSrcPath +  L" ";
	String tEngineLibrary=this->ideLibPath + L"\\" + cplr.engineLibraryName + cplr.engineLibraryExtension + L" ";


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

const String& Compiler::GetSourcesPath()
{
	return this->ideSrcPath;
}

const String& Compiler::GetLibrariesPath()
{
	return this->ideLibPath;
}

bool Compiler::Compile(EditorScript* iEditorScript)
{
	if(iEditorScript->runtime)
		return false;

	bool retVal;

	iEditorScript->SaveScript();

	String tOriginalScriptSource=iEditorScript->script;
	String tCompileScriptSource=tOriginalScriptSource;

	{
		tCompileScriptSource+=L"\n\nextern \"C\" __declspec(dllexport) EntityScript* Create(){return new " + iEditorScript->Entity()->name + L"_;}";
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

	if(iEditorScript->module.empty())
		iEditorScript->module=Subsystem::RandomDir(Ide::Instance()->folderAppData,8);

	//append exports to source

	iEditorScript->SaveScript();

	//compose the compiler command line

	unsigned int tCompiler=Compiler::COMPILER_MS;

	String tCommandLineMingW=this->Compose(tCompiler,iEditorScript);

	//execute compilation

	bool tExecuteWithSuccess=Subsystem::Execute(iEditorScript->module,tCommandLineMingW,tCompilerErrorOutput.path,true,true,true);

	if(!tExecuteWithSuccess)
		DEBUG_BREAK();

	//unroll exports

	iEditorScript->SaveScript();

	//convert compiler output to readable locale

	String tWideCharCompilationOutput=StringUtils::ReadCharFile(tCompilerErrorOutput.path,L"rb");

	//extract and parse breakpoint line addresses

	File tLineAddressesOutput=iEditorScript->module + L"\\laddrss.output";

	String tScriptFileName=iEditorScript->scriptpath.Name();

	String tSharedObjectFileName=tScriptFileName + L".dll";
	String tSharedObjectSourceName=tScriptFileName + L".cpp";

	String tParseLineAddressesCommandLine=L"objdump --dwarf=decodedline " + tSharedObjectFileName + L" | find \""+ tSharedObjectSourceName + L"\" | find /V \":\"";

	tExecuteWithSuccess=Subsystem::Execute(iEditorScript->module,tParseLineAddressesCommandLine,tLineAddressesOutput.path,true,true,true);

	if(!tExecuteWithSuccess)
		DEBUG_BREAK();

	if(tLineAddressesOutput.Open(L"rb"))
	{
		EditorScript* tEditorScript=(EditorScript*)iEditorScript;

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
				tLineAddress.script=iEditorScript;


				Debugger::Instance()->GetAllBreakpoint().push_back(tLineAddress);
			}
		}

		tLineAddressesOutput.Close();

		if(!tLineAddressesOutput.Delete())
			DEBUG_BREAK();
	}

	//spawn a compilerViewer and show it if errors  @mic best to send message to the guicompilerviewer

	EditorScript* tEditorScript=(EditorScript*)iEditorScript;

	bool noErrors=GuiCompiler::Instance()->ParseCompilerOutputFile(tWideCharCompilationOutput);

	/*
	guiCompilerViewer->OnSize(tabContainer);
	guiCompilerViewer->OnActivate(tabContainer);

	if(false==noErrors)
		tabContainer->SetSelection(guiCompilerViewer);
	*/

	GuiLogger::Log(StringUtils::Format(L"%s on compiling %s\n",noErrors ? "OK" : "ERROR",iEditorScript->scriptpath.c_str()));

	return retVal;
}


bool Compiler::LoadScript(EditorScript* iScript)
{
	if(!iScript->module.size())
		return false;

	EntityScript*	(*tCreateModuleClassFunction)()=0;
	String          tModuleFile=iScript->module + L"\\" + iScript->scriptpath.Name() + L".dll";

	if(!iScript->handle)
	{
		iScript->handle=Subsystem::LoadLib(tModuleFile.c_str());

		if(!iScript->handle)
			return false;

	}

	tCreateModuleClassFunction=(EntityScript* (*)())Subsystem::GetProcAddress(iScript->handle,L"Create");

	if(tCreateModuleClassFunction)
	{
		iScript->runtime=tCreateModuleClassFunction();
		iScript->runtime->entity=iScript->Entity();

		Debugger::Instance()->RunDebuggeeFunction(iScript,0);

		return true;
	}
	else
	{
		GuiLogger::Log(StringUtils::Format(L"error creating module %s\n",tModuleFile.c_str()));

		SAFEDELETEARRAY(iScript->runtime);

		if(iScript->handle && Subsystem::FreeLib(iScript->handle))
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
		Debugger::Instance()->RunDebuggeeFunction(iScript,2);

		if(!Subsystem::FreeLib(iScript->handle))
			return false;
		else
			iScript->handle=0;

		if(tDestroyInTheDLL)
		{
			void (*DestroyScript)(EntityScript*)=(void(*)(EntityScript*))Subsystem::GetProcAddress(iScript->handle,L"Destroy");

			std::vector<GuiViewport*> tGuiViewport;

			//Ide::Instance()->mainAppWindow->CreateTabRects<GuiViewport>(tGuiViewport);

			Frame* tabContainerRunningUpdater=(Frame*)tGuiViewport[0]->GetRoot()->GetFrame();

			tabContainerRunningUpdater->renderingTask->pause=true;

			while(tabContainerRunningUpdater->renderingTask->executing);

			DestroyScript(iScript->runtime);
			iScript->runtime=0;

			tabContainerRunningUpdater->renderingTask->pause=false;
		}
		else
			iScript->runtime=0;

		return true;
	}

	return true;
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
void GuiStringProperty::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	const unsigned int ctMaxTmpArraySize=256;

	switch(iMsg)
	{
	case ONPAINT:
		{
			switch(this->valueType)
			{
			case GuiStringProperty::BOOL:
				{
					this->text=(*(bool*)this->valuePointer1 ? L"True" : L"False");
				}
				break;
			case GuiStringProperty::STRING:
				{
					this->text=*(String*)this->valuePointer1;
				}
				break;
			case GuiStringProperty::BOOLPTR:
				{
					this->text=(*(void**)this->valuePointer1 ? L"True" : L"False");
				}
				break;
			case GuiStringProperty::INT:
				{
					StringUtils::Format(this->text,L"%d",*(int*)this->valuePointer1);
				}
				break;
			case GuiStringProperty::FLOAT:
				{
					//https://stackoverflow.com/questions/16413609/printf-variable-number-of-decimals-in-float

					StringUtils::Format(this->text,L"%*.*f",this->valueParameter1,this->valueParameter2,*(float*)this->valuePointer1);
				}
				break;
			case GuiStringProperty::VEC2:
				{
					vec2& tVec2=*(vec2*)this->valuePointer1;
					StringUtils::Format(this->text,L"%*.*f , %*.*f",this->valueParameter1,this->valueParameter2,tVec2.x,this->valueParameter1,this->valueParameter2,tVec2.y);
				}
				break;
			case GuiStringProperty::VEC3:
				{
					vec3& tVec3=*(vec3*)this->valuePointer1;

					StringUtils::Format(this->text,L"%*.*f , %*.*f , %*.*f",	this->valueParameter1,this->valueParameter2,tVec3.x,
						this->valueParameter1,this->valueParameter2,tVec3.y,
						this->valueParameter1,this->valueParameter2,tVec3.z);
				}
				break;
			case GuiStringProperty::VEC4:
				{
					vec4& tVec4=*(vec4*)this->valuePointer1;

					StringUtils::Format(this->text,L"%*.*f , %*.*f , %*.*f , %*.*f",	this->valueParameter1,this->valueParameter2,tVec4.x,
						this->valueParameter1,this->valueParameter2,tVec4.y,
						this->valueParameter1,this->valueParameter2,tVec4.z,
						this->valueParameter1,this->valueParameter2,tVec4.w);
				}
				break;
			case GuiStringProperty::PTR:
				{
					StringUtils::Format(this->text,L"0x%p",this->valuePointer1);
				}
				break;
			case GuiStringProperty::MAT4POS:
				{
					mat4& tMat4=*(mat4*)this->valuePointer1;

					vec3 tPosition=tMat4.position();

					StringUtils::Format(this->text,L"%*.*f , %*.*f , %*.*f",	this->valueParameter1,this->valueParameter2,tPosition.x,
						this->valueParameter1,this->valueParameter2,tPosition.y,
						this->valueParameter1,this->valueParameter2,tPosition.z);
				}
				break;
			case GuiStringProperty::ENTITYLISTSIZE:
				{
					std::list<Entity*>& tEntityVec=*(std::list<Entity*>*)this->valuePointer1;
					size_t tCount=tEntityVec.size();
					StringUtils::Format(this->text,L"%d",tCount);
				}
				break;
			case GuiStringProperty::VEC3VECSIZE:
				{
					std::vector<vec3>& tVec3Vector=*(std::vector<vec3>*)this->valuePointer1;
					size_t tCount=tVec3Vector.size();
					StringUtils::Format(this->text,L"%d",tCount);
				}
				break;
			case GuiStringProperty::VEC3LISTSIZE:
				{
					std::list<vec3>& tVec3List=*(std::list<vec3>*)this->valuePointer1;
					size_t tCount=tVec3List.size();
					StringUtils::Format(this->text,L"%d",tCount);
				}
				break;
			case GuiStringProperty::ANIMATIONVECSIZE:
				{
					std::vector<Animation*>& tAnimationVec=*(std::vector<Animation*>*)this->valuePointer1;
					size_t tCount=tAnimationVec.size();
					StringUtils::Format(this->text,L"%d",tCount);
				}
				break;
			case GuiStringProperty::ISBONECOMPONENT:
				{
					Entity* tEntity=(Entity*)this->valueParameter1;

					this->text=L"must implement in GuiStringValue::ISBONECOMPONENT";
				}
				break;
			case GuiStringProperty::FLOAT2MINUSFLOAT1:
				{
					float& a=*(float*)this->valuePointer1;
					float& b=*(float*)this->valuePointer2;
					StringUtils::Format(this->text,L"%*.*g",this->valueParameter1,this->valueParameter2,b-a);
				}
				break;
			case GuiStringProperty::VEC32MINUSVEC31:
				{
					vec3& a=*(vec3*)this->valuePointer1;
					vec3& b=*(vec3*)this->valuePointer2;

					vec3 tVecResult=b-a;

					StringUtils::Format(this->text,L"%*.*g , %*.*g , %*.*g",	this->valueParameter1,this->valueParameter2,tVecResult.x,
						this->valueParameter1,this->valueParameter2,tVecResult.y,
						this->valueParameter1,this->valueParameter2,tVecResult.z);
				}
				break;
			default:
				this->text=L"GuiStringValue Must Implement";
			}

			GuiString::Procedure(iFrame,iMsg,iData);
		}
		break;
		default:
			GuiString::Procedure(iFrame,iMsg,iData);
	}
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////GuiString///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiString::GuiString():align(0.0f,0.5f),spot(0.0f,0.5f),textcolor(GuiString::COLOR_TEXT),font((GuiFont*)GuiFont::GetDefaultFont())
{
	this->cursor.p=this->text.c_str();
}

void GuiString::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		case ONPAINT:
			GuiRect::Procedure(iFrame,iMsg,iData);
			iFrame->renderer2D->DrawText(this->text,edges.x,edges.y,edges.z,edges.w,this->spot,this->align,this->textcolor,this->font);
		break;
		/*case ONMOUSEDOWN:
		{
			GuiRect::Procedure(iFrame,iMsg,iData);

			MouseData& md=*(MouseData*)iData;

			if(md.button==1)
			{
				StringEditor::Instance()->Bind(iFrame,this);
				StringEditor::Instance()->Enable(true);
			}
		}
		break;*/
		default:
			GuiRect::Procedure(iFrame,iMsg,iData);
	}
}

const String&  GuiString::GetText(){return this->text;}
void	GuiString::SetText(const String& iText){this->text=iText;}

void  GuiString::SetFont(GuiFont* iFont){this->font=iFont;}
GuiFont*	GuiString::GetFont(){return this->font;}

void GuiString::SetSpot(float iSpotX,float iSpotY){this->spot.make(iSpotX,iSpotY);}
const vec2& GuiString::GetSpot(){return this->spot;}

void GuiString::SetAlignment(float iAlignX,float iAlignY){this->align.make(iAlignX,iAlignY);}
const vec2& GuiString::GetAlignment(){return this->align;}

unsigned int	GuiString::GetTextColor(){return this->textcolor;}
void			GuiString::SetTextColor(unsigned int iTextColor){this->textcolor=iTextColor;}

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////GuiListBox////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

/*

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
*/

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


/*
void GuiListBoxHelpers::OnItemPressed(void* iData)
{
	GuiButton*	tButton=(GuiButton*)iData;
	GuiListBox* tListBox=(GuiListBox*)tButton->GetUserData();
}*/

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////GuiComboBox////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
/*

GuiComboBox::GuiComboBox():selectedItem(0),popup(0)
{
	this->Append(&this->string);
	this->Append(&this->button);
}

GuiComboBox::~GuiComboBox()
{
}

void GuiComboBox::OnControlEvent(Tab* iTab,const MsgData& iData)
{
	GuiRect::OnControlEvent(iTab,iData);

	if(iMsg.sender==&this->button && iMsg.senderFunc==ONMOUSEUP)
	{

	}
}

/ *
void pfdGuiComboBoxLabelsButtonAction(void* iData)
{
	GuiButton*		tButton=(GuiButton*)iData;
	GuiComboBox*	tComboBox=(GuiComboBox*)tButton->GetUserData();

	tComboBox->string->Text(tButton->Text());

	tComboBox->GetRoot()->CreateTab()->SetDraw(tComboBox->string);

	Ide::Instance()->GetPopup()->windowData->Show(false);
}* /


void GuiComboBox::CretePopupList()
{
	if(!this->popup)
	{
		vec4 tEdges=this->Edges();

		vec2 tPopupSize(tEdges.z-tEdges.x,this->items.size()*GuiRect::ROW_HEIGHT);
		vec2 tTabPos=this->GetRoot()->CreateTab()->windowData->Pos();

		tTabPos.x+=tEdges.x;
		tTabPos.y+=tEdges.w;

		this->popup=Ide::Instance()->mainAppWindow->mainContainer->CreateModalTab(tTabPos.x,tTabPos.y,tPopupSize.x,tPopupSize.y<300.0f?tPopupSize.y:300.0f);
		this->popup->hasFrame=false;
	}

	this->popup->viewers.Append(&this->listbox);

	this->popup->OnGuiActivate();
	this->popup->OnGuiSize();

	this->popup->SetDraw();
}

void GuiComboBox::RecreateList()
{
	/ *if(!this->list)
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
	}* /
}

*/


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

void GuiImage::OnPaint(Frame* iFrame,const PaintData& iData)
{
	/*if(this->image && this->image->handle)
		iFrame->renderer2D->DrawBitmap(this->image,edges.x,edges.y,edges.z,edges.w);*/

	//this->BroadcastToChilds(&GuiRect::OnPaint,iMsg);
	//this->EndClip(iTab,iData);
}

/*
void GuiImage::OnActivate(Tab* iTab,const MsgData& iData)
{
	GuiRect::OnActivate(iTab,iData);

	if(this->image && this->image->handle)
	{
		bool tErrorLoading=iFrame->renderer2D->LoadBitmap(this->image);

		if(tErrorLoading)
			wprintf(L"error loading image (rect %p ,image %p\n",this,this->image);
	}
}*/

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////GuiTextBox/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
GuiTextBox::GuiTextBox():cursor(0){}

GuiTextBox::~GuiTextBox(){SAFEDELETE(this->cursor);}


bool GuiTextBox::ParseKeyInput(Frame* iFrame,const KeyData& iData)
{
	bool			tRedraw=false;

	/*if(StringEditor::Instance()->Binded()==this && StringEditor::Instance()->Enabled())
	{
		unsigned int	tCaretOperation=StringEditor::CARET_DONTCARE;
		void*			tCaretParameter=0;

		if(iData.key)
		{
			switch(iData.key)
			{
			case 0x08:/ *VK_BACK* /tCaretOperation=StringEditor::CARET_BACKSPACE; break;
			default:
				tCaretOperation=StringEditor::CARET_ADD;
				tCaretParameter=(char*)iData.key;
			}
		}
		else
		{
			if(Keyboard::Instance()->IsPressed(0x25/ *VK_LEFT* /))
				tCaretOperation=StringEditor::CARET_ARROWLEFT;
			if(Keyboard::Instance()->IsPressed(0x27/ *VK_RIGHT* /))
				tCaretOperation=StringEditor::CARET_ARROWRIGHT;
			if(Keyboard::Instance()->IsPressed(0x26/ *VK_UP* /))
				tCaretOperation=StringEditor::CARET_ARROWUP;
			if(Keyboard::Instance()->IsPressed(0x28/ *VK_DOWN* /))
				tCaretOperation=StringEditor::CARET_ARROWDOWN;
			if(Keyboard::Instance()->IsPressed(0x03/ *VK_CANCEL* /))
				tCaretOperation=StringEditor::CARET_CANCEL;
			if(Keyboard::Instance()->IsPressed(0x2E/ *VK_DELETE* /))
				tCaretOperation=StringEditor::CARET_CANCEL;
			if(Keyboard::Instance()->IsPressed(0x1B/ *VK_ESCAPE* /)) 
			{
				StringEditor::Instance()->Enable(false);
				return false;
			}
		}

		tRedraw=true;

		bool tMustResize=StringEditor::Instance()->EditText(tCaretOperation,tCaretParameter);
	}*/

	return tRedraw;
}

void GuiTextBox::OnKeyDown(Frame* iFrame,const KeyData& iData)
{
	if(this==iFrame->GetFocus())
	{
		/*if(Keyboard::Instance()->IsPressed(0x71/ *VK_F2* /))
		{
			StringEditor::Instance()->Bind(iFrame,this);
			StringEditor::Instance()->Enable(true);
		}
		else if(Keyboard::Instance()->IsPressed(0x1B/ *VK_ESCAPE* /))
		{
			StringEditor::Instance()->Enable(false);
		}

		bool tRedraw=false;

		if(StringEditor::Instance()->Binded()==this && StringEditor::Instance()->Enabled())
			tRedraw=this->ParseKeyInput(iFrame,iData);

		if(tRedraw)
			iFrame->SetDraw(this);*/
	}
}


/*
void GuiTextBox::OnMouseDown(Tab* iTab,const MsgData& iData)
{
	iTab->SetFocus(this);
}*/

void GuiTextBox::OnPaint(Frame* iFrame,const PaintData& iData)
{
}



///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////GuiButton//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiButton::GuiButton()
{
	this->buttonflags=0;
	this->SetSpot(0.5f,0.5f);
	this->SetAlignment(0.5f,0.5f);
}

void GuiButton::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		case ONMOUSEENTER:
		{
			GuiString::Procedure(iFrame,iMsg,iData);
			::SetFlag(this->buttonflags,GuiRectFlag::HIGHLIGHTED,true);
			iFrame->SetDraw(this);
		}
		break;
		case ONMOUSEEXIT:
		{
			GuiString::Procedure(iFrame,iMsg,iData);
			::SetFlag(this->buttonflags,GuiRectFlag::HIGHLIGHTED,false);
			iFrame->SetDraw(this);
		}
		break;
		case ONMOUSEDOWN:
		{
			GuiString::Procedure(iFrame,iMsg,iData);

			MouseData* md=(MouseData*)iData;

			if(md && md->button==1)
			{
				::SetFlag(this->buttonflags,GuiRectFlag::PRESSED,true);
				iFrame->SetDraw(this);
			}
		}
		break;
		case ONMOUSEUP:
		{
			GuiString::Procedure(iFrame,iMsg,iData);

			MouseData* md=(MouseData*)iData;

			if(md && md->button==1)
			{
				bool tContains=EdgesContainsPoint(this->edges,md->mouse);

				::SetFlag(this->buttonflags,GuiRectFlag::PRESSED,false);

				if(!tContains)
					::SetFlag(this->buttonflags,GuiRectFlag::HIGHLIGHTED,false);

				iFrame->SetDraw(this);

				if(tContains)
				{
					ControlData cd(this,GuiRectMessages::ONMOUSEUP,iData);
					iFrame->Message(this->parent,GuiRectMessages::ONCONTROLEVENT,&cd);
				}
			}
		}
		break;
		case ONPAINT:
		{
			unsigned int tColor=this->GetColor();

			if(::GetFlag(this->buttonflags,GuiRectFlag::PRESSED))
				tColor=BlendColor(0x608050,tColor);
			if(::GetFlag(this->buttonflags,GuiRectFlag::HIGHLIGHTED))
				tColor=BlendColor(0x805060,tColor);

			vec4 tEdges(edges.x+0.5f,edges.y+0.5f,edges.z-0.5f,edges.w-0.5f);

			iFrame->renderer2D->DrawRectangle(tEdges.x,tEdges.y,tEdges.z,tEdges.w,tColor);
			iFrame->renderer2D->DrawText(this->text,tEdges.x,tEdges.y,tEdges.z,tEdges.w,this->spot,this->align,this->textcolor,this->font);

			tColor=BlendColor(0x704020,tColor);

			iFrame->renderer2D->DrawRectangle(tEdges.x,tEdges.y,tEdges.z,tEdges.w,tColor,1);
		}
		break;
		default:
			GuiString::Procedure(iFrame,iMsg,iData);
	}
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////GuiCheckButton//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiCheckButton::GuiCheckButton()
{
	//this->SetFlag(FLAGS,DRAWCHECK,true);
}

GuiCheckButton::~GuiCheckButton(){}

void GuiCheckButton::OnMouseUp(Frame* iFrame,const MsgData& iData)
{
	//GuiString::OnMouseUp(iTab,iData);

	//this->GetParent()->OnControlEvent(iTab,Msg(this,ONMOUSEUP));
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
	/*this->Append(&this->button);
	this->Append(&this->path);*/
}

GuiPath::~GuiPath()
{

}

void GuiPathHelpers::OnSelectDirectoryButtonPressed(void* iData)
{
	GuiPath* tGuiPropertyPath=(GuiPath*)iData;

	Frame* iFrame=tGuiPropertyPath->GetRoot()->GetFrame();

	iFrame->isModal ? iFrame->windowData->Enable(false) : MainFrame::Instance()->GetFrame()->Enable(false);
	iFrame->SetDraw();

	String tDirectory=Subsystem::DirectoryChooser(L"",L"");

	if(tDirectory.size())
		tGuiPropertyPath->path.SetText(tDirectory);

	iFrame->isModal ? iFrame->windowData->Enable(true) : MainFrame::Instance()->GetFrame()->Enable(true);
	iFrame->SetDraw();

	if(tGuiPropertyPath->func)
		tGuiPropertyPath->func(tGuiPropertyPath->param);
}

///////////////////GuiSlider///////////////////

GuiSlider::GuiSlider(float* iRef,float* iMin,float* iMax):referenceValue(iRef),minimum(iMin),maximum(iMax){}

void GuiSlider::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		case ONMOUSEMOVE:
		case ONMOUSEDOWN:
		{
			GuiRect::Procedure(iFrame,iMsg,iData);

			MouseData* md=(MouseData*)iData;

			if(md && md->raw.Left())
				this->SetPosition(iFrame,md->mouse);
		}
		break;
		case ONPAINT:
		{
			GuiRect::Procedure(iFrame,iMsg,iData);

			if(this->minimum && this->maximum && this->referenceValue)
			{
				vec4 tEdges=this->edges;
				float tWidth=tEdges.z-tEdges.x;
				float tHeight=tEdges.w-tEdges.y;

				iFrame->renderer2D->DrawRectangle(tEdges.x+10,tEdges.y+tHeight/2.0f-2,tEdges.z-10,tEdges.y+tHeight/2.0f+2,0x000000);

				float tMinimum=*this->minimum;

				String smin(StringUtils::Float(*this->minimum));
				String smax(StringUtils::Float(*this->maximum));
				String value(StringUtils::Float(*this->referenceValue));

				iFrame->renderer2D->DrawText(smin,tEdges.x+10,tEdges.y,tEdges.x+tWidth/2,tEdges.y+10,vec2(0,0.5f),vec2(0,0.5f),GuiString::COLOR_TEXT);
				iFrame->renderer2D->DrawText(smax,tEdges.x+tWidth/2,tEdges.y,tEdges.z-10,tEdges.y+10,vec2(1,0.5f),vec2(1.0f,0.5f),GuiString::COLOR_TEXT);
				iFrame->renderer2D->DrawText(value,tEdges.x,tEdges.y+20,tEdges.z,tEdges.w,vec2(0.5f,0.5f),vec2(0.5f,0.5f),GuiString::COLOR_TEXT);

				float tip=(tEdges.x+10) + ((*referenceValue)/(*maximum-*minimum))*(tWidth-20);

				iFrame->renderer2D->DrawRectangle(tip-5,tEdges.y+tHeight/2.0f-5,tip+5,tEdges.y+tHeight/2.0f+5,GuiString::COLOR_TEXT);
			}
		}
		break;
		default:
			GuiRect::Procedure(iFrame,iMsg,iData);
	}
}

void GuiSlider::SetPosition(Frame* iFrame,const vec2& iMouse)
{
	if(this->minimum && this->maximum && this->referenceValue)
	{
		const vec4&  tEdges=this->edges;
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
				iFrame->SetDraw(this);
			}
		}
	}
}

float GuiSlider::Value(){return *this->referenceValue;}
float GuiSlider::Min(){return *this->minimum;}
float GuiSlider::Max(){return *this->maximum;}


///////////GuiAnimationController//////////////


GuiAnimationController::GuiAnimationController(AnimationController& iAnimationController):
	animationController(iAnimationController),
	slider(&animationController.cursor,&this->animationController.start,&this->animationController.end)
{
	this->play.SetText(L"Play");
	this->stop.SetText(L"Stop");

	this->Append(&this->slider);
	this->Append(&this->play);
	this->Append(&this->stop);
}

void GuiAnimationController::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		case ONSIZE:
		{
			GuiRect::Procedure(iFrame,iMsg,iData);

			float tHalfWidth=edges.x+(edges.z-edges.x)/2.0f;

			vec4 tSliderEdges=edges;
			vec4 tStopEdges=edges;
			vec4 tPlayEdges=edges;

			tStopEdges.y=tPlayEdges.y=tSliderEdges.w=edges.y+30;
			tStopEdges.x=tPlayEdges.z=tHalfWidth;
			tStopEdges.w=tPlayEdges.w=edges.w;

			this->slider.edges=tSliderEdges;
			this->stop.edges=tStopEdges;
			this->play.edges=tPlayEdges;
		}
		break;
		case ONCONTROLEVENT:
		{
			GuiRect::Procedure(iFrame,iMsg,iData);

			ControlData* cd=(ControlData*)iData;

			if(cd->control==&this->slider)
			{
				if(cd->msg==ONMOUSEMOVE)
				{
					float value=this->slider.Value();

					if(value!=this->slider.Value())
					{
						this->animationController.SetFrame(this->slider.Value());
						iFrame->SetDraw(this);
					}
				}
			}
			else if(cd->control==&this->play)
			{
				if(cd->msg==ONMOUSEUP)
				{
					this->animationController.Play();
				}
			}
			else if(cd->control==&this->stop)
			{
				if(cd->msg==ONMOUSEUP)
				{
					this->animationController.Stop();
				}
			}
		}		
		break;											
		default:
			GuiRect::Procedure(iFrame,iMsg,iData);
	}
}

///////////////////GuiViewport/////////////////

void launchStopGuiViewportCallback(void* iData)
{
	GuiViewport* guiViewport=(GuiViewport*)iData;
}

GuiViewport::GuiViewport():
	frame(0),
	buffer(0),
	needsPicking(0),
	pickedEntity(0),
	renderInstance(0),
	renderFps(60),
	bitmap(0),
	editorentity(0)
{
	GlobalViewports().push_back(this);

	this->flags=0;

	this->projection=this->projection.perspective(90,16/9,1,1000);
	this->view.move(vec3(100,100,100));
	this->view.lookat(vec3(0,0,0));
}

void GuiViewport::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		case ONPAINT:
		{
			#if RENDERER_ENABLED
				#if !RENDERER_THREADED
					this->Render(iFrame);
				#endif
				this->DrawBuffer(iFrame);
			#endif
		}
		break;
		case ONMOUSEMOVE:
		{
			GuiRect::Procedure(iFrame,iMsg,iData);

			MouseData& tmd=*(MouseData*)iData;

			if(Keyboard::Instance()->IsPressed(0x01/*VK_LBUTTON*/))
			{
				float dX=(tmd.mouse.x-this->mouseold.x);
				float dY=(tmd.mouse.y-this->mouseold.y);

				if(Keyboard::Instance()->IsPressed(0x011/*VK_CONTROL*/))
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

				iFrame->SetDraw(this);
			}
			this->needsPicking=true;

			this->mouseold=tmd.mouse;
		}
		break;
		case ONMOUSEUP:
		{
			/*if(this->IsHovering() && this->pickedEntity)
			Ide::Instance()->mainAppWindow->mainContainer->Broadcast(GuiRect::Funcs::ONENTITYSELECTED,Msg(this->pickedEntity));

			GuiRect::OnMouseUp(iTab,iData);*/
		}
		break;
		case ONMOUSEWHEEL:
		{
			MouseData& md=*(MouseData*)iData;

			if(md.scroller)
			{
				this->view*=mat4().translate(0,0,md.scroller*10);
				iFrame->SetDraw(this);
			}
		}
		break;
		case ONSIZE:
		{
			this->ResizeBuffers(iFrame);
		}
		break;
		case ONACTIVATE:
		{
			this->frame=iFrame;
		}
		break;
		default:
			GuiRect::Procedure(iFrame,iMsg,iData);
	}
}

EditorEntity*		GuiViewport::GetEntity(){return this->editorentity;}
void				GuiViewport::SetEntity(EditorEntity* iEditorEntity){this->editorentity=iEditorEntity;}

mat4& GuiViewport::GetProjectionMatrix(){return this->projection;}
mat4& GuiViewport::GetViewMatrix(){return this->view;}
mat4& GuiViewport::GetModelMatrix(){return this->model;}

EditorEntity* GuiViewport::GetHoveredEntity(){return 0;}
EditorEntity* GuiViewport::GetPickedEntity(){return this->pickedEntity;}

unsigned int	GuiViewport::GetLastFrameTime(){return this->lastFrameTime;}
void GuiViewport::SetRenderingRate(unsigned int iFps){this->renderFps=iFps;}
unsigned int GuiViewport::GetRenderingRate(){return this->renderFps;}

std::list<GuiViewport*>& GuiViewport::GetPool()
{
	return GlobalViewports();
}

GuiViewport* GuiViewport::Instance()
{
	return new GuiViewport();
}

//////////////////GuiScrollRect////////////////

GuiScrollRect::GuiScrollRect()
{
	this->clength[0]=0;
	this->clength[1]=0;
	this->ratios[0]=0;
	this->ratios[1]=0;
	this->active[0]=0;
	this->active[1]=0;
	this->positions[0]=0;
	this->positions[1]=0;
	this->content[0]=0;
	this->content[1]=0;
	this->maxes[0]=0;
	this->maxes[1]=0;
	this->scrollerhit=0;
	this->scrollerpressed=0;
	this->scroller=0;
}

void GuiScrollRect::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		case GuiRectMessages::ONHITTEST:
		{
			HitTestData& thtd=*(HitTestData*)iData;

			GuiRect::Procedure(iFrame,iMsg,iData);

			if(thtd.hit==this)
			{
				if(this->active[SCROLLH] && EdgesContainsPoint(this->sedges[SCROLLH][0],thtd.mouse))
				{
					if(EdgesContainsPoint(this->sedges[SCROLLH][1],thtd.mouse))this->scrollerhit=1;
					else if(EdgesContainsPoint(this->sedges[SCROLLH][2],thtd.mouse))this->scrollerhit=2;
					else if(EdgesContainsPoint(this->sedges[SCROLLH][3],thtd.mouse))this->scrollerhit=3;
				}
				else if(this->active[SCROLLV] && EdgesContainsPoint(this->sedges[SCROLLV][0],thtd.mouse))
				{
					if(EdgesContainsPoint(this->sedges[SCROLLV][1],thtd.mouse))this->scrollerhit=4;
					else if(EdgesContainsPoint(this->sedges[SCROLLV][2],thtd.mouse))this->scrollerhit=5;
					else if(EdgesContainsPoint(this->sedges[SCROLLV][3],thtd.mouse))this->scrollerhit=6;
				}
				else
				{
					this->scrollerhit=0;

					float offx=this->positions[SCROLLH]*this->content[0];
					float offy=this->positions[SCROLLV]*this->content[1];

					thtd.off.x+=offx;
					thtd.off.y+=offy;

					thtd.mouse.x+=offx;
					thtd.mouse.y+=offy;
				}
			}
		}
		break;
		case GuiRectMessages::ONPREPAINT:
		{
			PaintData& pd=*(PaintData*)iData;
			
			vec4 tClippingEdges=this->GetClippingEdges();

			pd.clip.x=std::max<float>(pd.clip.x,tClippingEdges.x);
			pd.clip.y=std::max<float>(pd.clip.y,tClippingEdges.y);
			pd.clip.z=std::min<float>(pd.clip.z,tClippingEdges.z);
			pd.clip.w=std::min<float>(pd.clip.w,tClippingEdges.w);
			
			pd.offset.x-=this->GetScrollbarPosition(0);
			pd.offset.y-=this->GetScrollbarPosition(1);
		}
		break;
		case ONMOUSEWHEEL:
		{
			MouseData& md=*(MouseData*)iData;

			if(this->IsScrollbarVisible(1) && !md.button && md.scroller)
			{
				float tScrollRatio=this->GetScrollerRatio(1)/TICKNESS;
				float tScrollAmount=this->positions[1] + (md.scroller < 0 ? tScrollRatio : -tScrollRatio);

				this->SetScrollbarPosition(1,tScrollAmount);

				/*//update hittest ??
				iFrame->hittest.off.y-=tScrollAmount;
				iFrame->hittest.mouse.y-=tScrollAmount;
				this->Procedure(iFrame,ONHITTEST,&iFrame->hittest);
				*/

				iFrame->SetDraw(this);
			}
		}
		break;
		case GuiRectMessages::ONMOUSEDOWN:
		{
			GuiRect::Procedure(iFrame,iMsg,iData);

			MouseData& md=*(MouseData*)iData;

			if(md.raw.Left())
			{
				this->scrollerpressed=this->scrollerhit;

				if(this->scrollerpressed>0)
				{
					const unsigned int tBarType=(this->scrollerpressed>=1 && this->scrollerpressed<=3) ? 0 : 1;

					//tip a
					if(this->scrollerpressed==1 || this->scrollerpressed==4)
					{
						float tRatio=this->ratios[tBarType];
						float tAmount=this->positions[tBarType] - tRatio/TICKNESS;

						this->SetScrollbarPosition(tBarType,tAmount);
					}
					else if(this->scrollerpressed==2 || this->scrollerpressed==5)
					{
						this->scroller=0;

						float tMouseValue=tBarType ? md.mouse.y : md.mouse.x;
						vec4  tScroller=this->GetScrollerEdges(tBarType);
					
						float tContainerBegin=tBarType ? this->sedges[tBarType][2].y : this->sedges[tBarType][2].x;
						float tContainerLength=this->clength[tBarType];
						float tScrollerBegin=tBarType ? tScroller.y : tScroller.x;
						float tScrollerEnd=tBarType ? tScroller.w : tScroller.z;
						float tScrollerLength=tScrollerEnd-tScrollerBegin;

						if(tMouseValue>=tScrollerBegin && tMouseValue<=tScrollerEnd)
							this->scroller=((tMouseValue-tScrollerBegin)/tScrollerLength)*this->ratios[tBarType];
						else
							this->SetScrollbarPosition(tBarType,(tMouseValue-tContainerBegin)/tContainerLength);
					}
					else if(this->scrollerpressed==3 || this->scrollerpressed==6)
					{
						float tRatio=this->ratios[tBarType];
						float tAmount=this->positions[tBarType] + tRatio/TICKNESS;

						this->SetScrollbarPosition(tBarType,tAmount);
					}

					iFrame->SetDraw(this);
				}
			}
		}
		break;
		case GuiRectMessages::ONMOUSEUP:
		{
			GuiRect::Procedure(iFrame,iMsg,iData);

			MouseData& md=*(MouseData*)iData;

			if(md.button==1)
			{
				this->scrollerpressed=0;
				this->scroller=0;
			}
		}
		break;
		case GuiRectMessages::ONMOUSEMOVE:
		{
			GuiRect::Procedure(iFrame,iMsg,iData);

			MouseData& md=*(MouseData*)iData;

			if(this->scroller>0)
			{
				const unsigned int tBarType=(this->scrollerpressed>=1 && this->scrollerpressed<=3) ? 0 : 1;
				float tMouseValue=tBarType ? md.mouse.y : md.mouse.x;

				float tContainerBegin=tBarType ? this->sedges[tBarType][2].y : this->sedges[tBarType][2].x;
				float tContainerEnd=tBarType ? this->sedges[tBarType][2].w : this->sedges[tBarType][2].z;
				float tContainerLength=this->clength[tBarType];

				if(tMouseValue>tContainerBegin && tMouseValue<tContainerEnd)
				{
					float tMouseTreshold=(tMouseValue-tContainerBegin)/tContainerLength;

					this->SetScrollbarPosition(tBarType,tMouseTreshold-this->scroller);

					iFrame->SetDraw(this);
				}
			}			
		}
		break;
		case GuiRectMessages::ONSIZE:
		{
			this->CalculateScrollbarsLayout();
		}
		break;
		case GuiRectMessages::ONPAINT:
		{
			if(this->active[SCROLLH])
			{
				const vec4& e=this->sedges[SCROLLH][0];
				const vec4& a=this->sedges[SCROLLH][1];
				const vec4& c=this->sedges[SCROLLH][2];
				const vec4& b=this->sedges[SCROLLH][3];
				vec4  s=this->GetScrollerEdges(SCROLLH);

				//tip a
				iFrame->renderer2D->DrawRectangle(a.x,a.y,a.z,a.w,0x00000000,1);
				iFrame->renderer2D->DrawBitmap(iFrame->iconLeft,a.x,a.y,a.z,a.w);
				//body
				iFrame->renderer2D->DrawRectangle(c.x,c.y,c.z,c.w,0x00000000,1);
				//scroller
				iFrame->renderer2D->DrawRectangle(s.x,s.y,s.z,s.w,0x00000000);
				//tip b
				iFrame->renderer2D->DrawRectangle(b.x,b.y,b.z,b.w,0x00000000,1);
				iFrame->renderer2D->DrawBitmap(iFrame->iconRight,b.x,b.y,b.z,b.w);
			}

			if(this->active[SCROLLV])
			{
				const vec4& e=this->sedges[SCROLLV][0];
				const vec4& a=this->sedges[SCROLLV][1];
				const vec4& c=this->sedges[SCROLLV][2];
				const vec4& b=this->sedges[SCROLLV][3];
				vec4  s=this->GetScrollerEdges(SCROLLV);

				//tip a
				iFrame->renderer2D->DrawRectangle(a.x,a.y,a.z,a.w,0x00000000,1);
				iFrame->renderer2D->DrawBitmap(iFrame->iconUp,a.x,a.y,a.z,a.w);
				//body
				iFrame->renderer2D->DrawRectangle(c.x,c.y,c.z,c.w,0x00000000,1);
				//scroller
				iFrame->renderer2D->DrawRectangle(s.x,s.y,s.z,s.w,0x00000000);
				//tip b
				iFrame->renderer2D->DrawRectangle(b.x,b.y,b.z,b.w,0x00000000,1);
				iFrame->renderer2D->DrawBitmap(iFrame->iconDown,b.x,b.y,b.z,b.w);
			}
		}
		break;
		default:
			GuiRect::Procedure(iFrame,iMsg,iData);
	}
}

float GuiScrollRect::GetScrollerRatio(unsigned int iType)
{
	return this->ratios[iType];
};

void GuiScrollRect::SetScrollbarPosition(unsigned int iType,float iNewPositionPercent)
{
	float tPosition=iNewPositionPercent+this->ratios[iType];

	vec4 s=this->GetScrollerEdges(iType);

	float slen=iType ? s.w-s.y : s.z-s.x;
	const float& clen=this->clength[iType];
	
	if(tPosition>1)
		this->positions[iType]=(clen-slen)/clen;
	else			 
		this->positions[iType] = iNewPositionPercent < 0 ? 0 : iNewPositionPercent;
};

float GuiScrollRect::GetScrollbarPosition(unsigned int iType)
{
	return this->positions[iType]*this->content[iType];
}

bool GuiScrollRect::IsScrollbarVisible(unsigned int iType)
{
	return this->active[iType];
}

vec4 GuiScrollRect::GetScrollerEdges(unsigned int iType)
{
	const vec4&		e=this->sedges[iType][0];
	const vec4&		c=this->sedges[iType][2];
	const float&	p=this->positions[iType];
	const float&	r=this->ratios[iType];
	const float&	clen=this->clength[iType];

	float	sbeg;

	vec4 scroller;

	switch(iType)
	{
		case SCROLLH:
		{
			sbeg=c.x+p*clen;
			scroller.make(sbeg,e.y,sbeg+r*clen,e.w);
		}
		break;
		case SCROLLV:
		{
			sbeg=c.y+p*clen;
			scroller.make(e.x,sbeg,e.z,sbeg+r*clen);
		}
		break;
		default:
			DEBUG_BREAK();
	}

	return scroller;
}

vec4 GuiScrollRect::GetClippingEdges()
{
	float right=this->edges.z - (this->active[1] ? TICKNESS : 0);
	float bottom=this->edges.w - (this->active[0] ? TICKNESS : 0);

	return vec4(this->edges.x,this->edges.y,right,bottom);
}

vec4 GuiScrollRect::GetContentEdges()
{
	return vec4(this->edges.x,this->edges.y,this->edges.x+this->maxes[0],this->edges.y+this->maxes[1]);
}

bool GuiScrollRect::IsScrollbarHitted()
{
	return this->scrollerhit;
}

void GuiScrollRect::CalculateScrollbarsLayout()
{
	float tRight=this->edges.z;
	float tBottom=this->edges.w;

	bool& hActive=this->active[SCROLLH];
	bool& vActive=this->active[SCROLLV];

	hActive=this->content[0]>(tRight-this->edges.x);
	tBottom -= hActive ? TICKNESS : 0;
	vActive=this->content[1]>(tBottom-this->edges.y);
	tRight -= vActive ? TICKNESS : 0;
	hActive=this->content[0]>(tRight-this->edges.x);

	this->maxes[0]=std::max<float>(tRight-this->edges.x,this->content[0]);
	this->maxes[1]=std::max<float>(tBottom-this->edges.y,this->content[1]);

	if(hActive)
	{
		const vec4& e=this->sedges[SCROLLH][0];

		this->sedges[SCROLLH][0].make(edges.x,edges.w-TICKNESS,edges.z - (vActive ? TICKNESS : 0),edges.w);
		this->sedges[SCROLLH][1].make(e.x,e.y,e.x+TICKNESS,e.w);
		this->sedges[SCROLLH][3].make(e.z-TICKNESS,e.y,e.z,e.w);
		this->sedges[SCROLLH][2].make(this->sedges[SCROLLH][1].z,e.y,this->sedges[SCROLLH][3].x,e.w);
		this->clength[SCROLLH]=(this->sedges[SCROLLH][2].z-this->sedges[SCROLLH][2].x);
		this->ratios[SCROLLH]=(this->sedges[SCROLLH][0].z-this->sedges[SCROLLH][0].x)/this->maxes[SCROLLH];
	}
	else
		 this->sedges[SCROLLH][0].make(0,0,0,0);

	if(vActive)
	{
		const vec4& e=this->sedges[SCROLLV][0];

		this->sedges[SCROLLV][0].make(edges.z-TICKNESS,edges.y,edges.z,edges.w - (hActive ? TICKNESS : 0));
		this->sedges[SCROLLV][1].make(e.x,e.y,e.z,e.y+TICKNESS);
		this->sedges[SCROLLV][3].make(e.x,e.w-TICKNESS,e.z,e.w);
		this->sedges[SCROLLV][2].make(e.x,this->sedges[SCROLLV][1].w,e.z,this->sedges[SCROLLV][3].y);
		this->clength[SCROLLV]=(this->sedges[SCROLLV][2].w-this->sedges[SCROLLV][2].y);
		this->ratios[SCROLLV]=(this->sedges[SCROLLV][0].w-this->sedges[SCROLLV][0].y)/this->maxes[SCROLLV];
	}
	else
		 this->sedges[SCROLLV][0].make(0,0,0,0);
	
	this->SetScrollbarPosition(0,this->positions[0]);
	this->SetScrollbarPosition(1,this->positions[1]);
}

void GuiScrollRect::SetContent(float iHor,float iVer)
{
	this->content[0]=std::max<float>(this->content[0],iHor);
	this->content[1]=std::max<float>(this->content[1],iVer);

	this->CalculateScrollbarsLayout();
}

void GuiScrollRect::ResetContent()
{
	this->content[0]=0;
	this->content[1]=0;

	this->CalculateScrollbarsLayout();
}

const vec2& GuiScrollRect::GetContent(){return this->content;}

//////////////////////GuiPanel/////////////////

GuiPanel::GuiPanel()
{
	//this->SetName(L"GuiPanel");
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////GuiSceneViewer/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiScene::GuiScene():sceneRootEntity(0){}
GuiScene* GuiScene::Instance()
{
	GLOBALGETTERFUNCASSINGLETON(GlobalGuiSceneViewerInstance,GuiScene);
}

EditorEntity*		GuiScene::GetSceneRootEntity(){return this->sceneRootEntity;}
void				GuiScene::SetSceneRootEntity(EditorEntity* iEntity)
{
	this->sceneRootEntity=iEntity;
	this->InsertRoot(iEntity->GetSceneLabel());
} 

void GuiScene::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		case ONENTITIESCHANGE:
		{
			if(iData)
			{
				EditorEntity* tEntity=(EditorEntity*)iData; 

				if(!this->sceneRootEntity)
				{
					this->sceneRootEntity=tEntity;
					this->InsertRoot(this->sceneRootEntity->GetSceneLabel());
				}				

				iFrame->SetDraw(this);
			}
		}
		break;
		case ONMOUSEUP:
		{
			GuiTreeView::Procedure(iFrame,iMsg,iData);

			MouseData& md=*(MouseData*)iData;

			if(md.button==3 && !this->IsScrollbarHitted())
			{
				int tMenuItem=iFrame->TrackGuiSceneViewerPopup(this->GetTreeViewHoveredNode());	

				SceneEntityLabel*		tSceneLabel=dynamic_cast<SceneEntityLabel*>(this->GetTreeViewHoveredNode());
				EditorPropertiesBase*	tCreatedEditorObject=0;

				switch(tMenuItem)
				{
					case 1:
					{
						EditorEntity* tEditorEntity=new EditorEntity;

						tEditorEntity->SetName(StringUtils::Format(L"Entity_%i",EditorEntity::GetInstancedEntitiesNumber()));

						tEditorEntity->OnPropertiesCreate();

						if(tSceneLabel)
						{
							tSceneLabel->GetValue()->Append(tEditorEntity);
							this->CalculateLayout();
						}
						else if(this->GetSceneRootEntity())
						{
							this->GetSceneRootEntity()->Append(tEditorEntity);
							this->CalculateLayout();
						}
						else
							this->SetSceneRootEntity(tEditorEntity);
					}
					break;
					case 2:
						//GuiSceneLabelHelpers::hovered->DestroyChilds();
					break;

					case 3:tCreatedEditorObject=tSceneLabel->GetValue()->CreateComponent<EditorLight>();break;
					case 4:tCreatedEditorObject=tSceneLabel->GetValue()->CreateComponent<EditorMesh>();break;
					case 5:tCreatedEditorObject=tSceneLabel->GetValue()->CreateComponent<EditorCamera>();break;
					case 14:tCreatedEditorObject=tSceneLabel->GetValue()->CreateComponent<EditorScript>();break;
				}

				if(tCreatedEditorObject)
					tCreatedEditorObject->OnResourcesCreate();

				if(tSceneLabel)
				{
					for(std::list<GuiEntity*>::iterator i=GuiEntity::GetPool().begin();i!=GuiEntity::GetPool().end();i++)
					{
						if((*i)->GetEntity()==tSceneLabel->GetValue())
						{
							(*i)->CalculateLayout();
							iFrame->SetDraw(*i);
						}
					}
				}

				if(tMenuItem)
					iFrame->SetDraw(GuiScene::Instance());
			}
			break;
		}
		break;
		default:
			GuiTreeView::Procedure(iFrame,iMsg,iData);
	}
}

/*
EditorEntity* EditorEntity::SceneEntityLabel::GetValue()
{
	return this->GuiTreeViewItem<EditorEntity*>::GetValue();
}*/

void SceneEntityLabel::OnMouseUp(Frame* iFrame,const MouseData& iMd)
{
	GuiTreeViewItem::OnMouseUp(iFrame,iMd);

	switch(iMd.button)
	{
		case 1:
		{
			iFrame->MessagePool<GuiEntity>(GuiRectMessages::ONENTITYSELECTED,this->GetValue());
		}
		break;
	}
}

void GuiScene::Save(String iFilename)
{
}

void GuiScene::Load(String iFilename)
{

}
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////GuiEntityViewer/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiEntity::GuiEntity():entity(0)
{
	GlobalGuiEntityViewerInstance().push_back(this);
}

GuiEntity::~GuiEntity()
{
	GlobalGuiEntityViewerInstance().remove(this);
}

GuiEntity* GuiEntity::Instance()
{
	return new GuiEntity;
}

std::list<GuiEntity*>& GuiEntity::GetPool()
{
	return GlobalGuiEntityViewerInstance();
}

void GuiEntity::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		case ONENTITYSELECTED:
		{
			EditorEntity* tReceivedEntity=(EditorEntity*)iData;

			if(this->GetEntity()!=tReceivedEntity)
			{
				if(tReceivedEntity)
				{
					if(!this->GetEntity())
					{
						this->SetEntity(tReceivedEntity);
						this->InsertRoot(tReceivedEntity->TreeViewContainer());
					}
					else
						this->GetEntity()->Append(tReceivedEntity);
				}
			}

			iFrame->SetDraw(this);
		}
		break;
		default:
			GuiPropertyTree::Procedure(iFrame,iMsg,iData);
	}
}

void			GuiEntity::SetEntity(EditorEntity* iEntity){this->entity=iEntity;}
EditorEntity*	GuiEntity::GetEntity(){return this->entity;}

///////////////GuiLogger////////////////

GuiLogger::GuiLoggerItem::GuiLoggerItem(const String& iString):GuiListBoxNode(iString)
{
	if(this->label.size() && this->label.back()==L'\n')
		this->label.pop_back();

	vec2 tTextDim=GuiFont::GetDefaultFont()->MeasureText(this->label.c_str());
	this->height=tTextDim.y*SEZAUREA;
}

float GuiLogger::GuiLoggerItem::GetHeight(){return this->height;}

GuiLogger::GuiLogger()
{
	GlobalGuiLoggerInstance().push_back(this);
	this->flags=0;
}
GuiLogger::~GuiLogger()
{
	GlobalGuiLoggerInstance().remove(this);

	this->Clear();
}

GuiLogger*	GuiLogger::Instance()
{
	return new GuiLogger;
}
std::list<GuiLogger*>&	GuiLogger::GetPool(){return GlobalGuiLoggerInstance();}

void GuiLogger::AppendLog(const String& iString)
{
	GuiLoggerItem* tLog=new GuiLoggerItem(iString);

	this->logs.push_back(tLog);
	this->InsertItem(*tLog);
}

void GuiLogger::Clear()
{
	for(std::list<GuiLoggerItem*>::iterator i=this->logs.begin();i!=this->logs.end();i++)
		SAFEDELETE(*i);
}

void GuiLogger::Log(const String& iString,GuiLogger* iLogger)
{
	GuiLogger* tLogger=iLogger ? iLogger : GuiLogger::GetPool().size() ? GuiLogger::GetPool().front() : 0;

	if(tLogger)
		tLogger->AppendLog(iString);
}

///////////////GuiConsoleViewer////////////////

GuiConsole::GuiConsole(){}
GuiConsole::~GuiConsole(){}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////GuiProjectViewer////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

const int SPLITTER_SIZE=4;

void GuiProject::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		case ONMOUSEDOWN:
		{
			GuiRect::Procedure(iFrame,iMsg,iData);

			MouseData& md=*(MouseData*)iData;

			if(md.mouse.y>this->edges.y)
				iFrame->SetCursor(1);

			if(md.mouse.x>=this->splitterLeft && md.mouse.x<=this->splitterLeft+SPLITTER_SIZE)
				this->splitterLeftActive=true;
			else if(md.mouse.x>=this->splitterRight && md.mouse.x<=this->splitterRight+SPLITTER_SIZE)
				this->splitterRightActive=true;
		}
		break;
		case ONMOUSEUP:
		{
			GuiRect::Procedure(iFrame,iMsg,iData);

			this->splitterLeftActive=false;
			this->splitterRightActive=false;
		}
		break;
		case ONMOUSEMOVE:
		{
			GuiRect::Procedure(iFrame,iMsg,iData);

			MouseData& md=*(MouseData*)iData;

			if(md.mouse.y>this->edges.y)
				iFrame->SetCursor(1);

			if(md.raw.Left())
			{
				if(this->splitterLeftActive)
					this->splitterLeft=md.mouse.x;
				else if(this->splitterRightActive)
					this->splitterRight=md.mouse.x;

				this->OnSize(iFrame,this->edges);
				iFrame->BroadcastTo(this,ONSIZE);

				iFrame->SetDraw(this);
			}
		}
		break;
		case ONACTIVATE:
		{
			this->Scan();
		}
		break;
		case ONDEACTIVATE:
		{
			this->Reset();
		}
		break;
		case ONPAINT:
			iFrame->renderer2D->DrawRectangle(this->edges.x,this->edges.y,this->edges.z,this->edges.w,Frame::COLOR_BACK);
		break;
		default:
			GuiRect::Procedure(iFrame,iMsg,iData);
	}
}

GuiProject::GuiProject():
	projectDirectory(GlobalRootProjectDirectory()),
	splitterLeftActive(false),
	splitterRightActive(false),
	splitterLeft(100),
	splitterRight(200),
	leftmousepressing(0)
{
	this->flags=0;

	projectDirectory=new ResourceNodeDir;
	this->projectDirectory->fileName=Ide::Instance()->folderProject;
	this->projectDirectory->isDir=true;
	this->projectDirectory->dirLabel.SetLabel(this->projectDirectory->fileName);
	this->projectDirectory->fileLabel.SetLabel(this->projectDirectory->fileName);

	this->Append(&this->dirView);
	this->Append(&this->fileView);
	this->Append(&this->resView);
}

GuiProject::~GuiProject()
{
}

GuiProject* GuiProject::Instance()
{
	GLOBALGETTERFUNCASSINGLETON(GlobalGuiProjectViewerInstance,GuiProject);
}

void GuiProject::OnSize(Frame* iFrame,const vec4& iEdges)
{
	float tOldWidth=this->edges.z-this->edges.x;
	float tNewWidth=iEdges.z-iEdges.x;
	float tLeftPercent=0;
	float tRightPercent=0;

	if(tOldWidth)
	{
		tLeftPercent=tOldWidth/this->splitterLeft;
		tRightPercent=tOldWidth/this->splitterRight;
	}

	if(tLeftPercent && tRightPercent)
	{
		this->splitterLeft=tNewWidth*(1.0f/tLeftPercent);
		this->splitterRight=tNewWidth*(1.0f/tRightPercent);
	}

	GuiRect::OnSize(iFrame,iEdges);

	this->dirView.edges=this->edges;
	this->fileView.edges=this->edges;
	this->resView.edges=this->edges;

	this->dirView.edges.z=this->splitterLeft;
	this->fileView.edges.x=this->splitterLeft+4;
	this->fileView.edges.z=this->splitterRight;
	this->resView.edges.x=this->splitterRight+4;
}

void GuiProject::Scan()
{
	Ide::Instance()->ScanDir(this->projectDirectory->fileName,this->projectDirectory);
	this->dirView.InsertRoot(this->projectDirectory->dirLabel);
	this->fileView.InsertItems(this->projectDirectory->fileLabels);
}

void GuiProject::Reset()
{
	for(std::list<ResourceNode*>::iterator tFile=this->projectDirectory->files.begin();tFile!=this->projectDirectory->files.end();tFile++)
		SAFEDELETE(*tFile);

	for(std::list<ResourceNodeDir*>::iterator tDir=this->projectDirectory->dirs.begin();tDir!=this->projectDirectory->dirs.end();tDir++)
		SAFEDELETE(*tDir);

	this->projectDirectory->files.clear();
	this->projectDirectory->dirs.clear();
	this->projectDirectory->fileLabels.clear();
	this->projectDirectory->dirLabel.Reset();

	this->dirView.RemoveRoot();
	this->fileView.RemoveAll();
}

void GuiProject::findResources(std::vector<ResourceNode*>& oResultArray,ResourceNode* iResourceNode,String iExtension)
{
	/*if(iResourceNode->isDir)
	{
		ResourceNodeDir* tResourceNodeDir=(ResourceNodeDir*)iResourceNode;

		for(std::list<ResourceNode*>::iterator nCh=tResourceNodeDir->files.begin();nCh!=tResourceNodeDir->files.end();nCh++)
			this->findResources(oResultArray,*nCh,iExtension);

		for(std::list<ResourceNodeDir*>::iterator nCh=tResourceNodeDir->dirs.begin();nCh!=tResourceNodeDir->dirs.end();nCh++)
			this->findResources(oResultArray,*nCh,iExtension);
	}
	else if(iResourceNode->fileName.PointedExtension()==iExtension)
		oResultArray.push_back(iResourceNode);*/
}

std::vector<ResourceNode*> GuiProject::findResources(String iExtension)
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
	/*EditorScript* editorScript=(EditorScript*)iData;

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

	editorScript->PropertyContainer().GetRoot()->CreateTab()->SetDraw(editorScript->buttonLaunch);*/
}
/*

void GuiProjectViewer::FileViewer::FileLabel::OnMouseUp(Tab* iTab,const MsgData& iData)
{
	Item::OnMouseUp(iTab,iData);

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
					Tab* tTab=GuiSceneViewer::Instance()->GetRoot()->CreateTab();

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
*/

/////////////////////////////////////////
/////////////////////////////////////////
///////GuiProjectViewer::DataViewer//////
/////////////////////////////////////////
/////////////////////////////////////////

GuiProject::DataView::DataView()
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

///////////////DrawInstance/////////////////

DrawInstance::DrawInstance(GuiRect* iRect,bool iRemove,bool iSkip,void* iParam):rect(iRect),remove(iRemove),skip(iSkip),param(iParam){}
DrawInstance::~DrawInstance(){}

///////////////GuiScript/////////////////

GuiScript*	GuiScript::Instance()
{
	return new GuiScript;
}

std::list<GuiScript*>&	GuiScript::GetPool()
{
	return GlobalScriptViewers();
}

GuiScript::GuiScript():
	font(GuiFont::GetDefaultFont()),
	editorscript(0),
	numrows(0),
	numcols(0),
	verticaltoolbar(0),
	colorbackbreak(GuiRect::COLOR_BACK),
	colorbacklines(GuiRect::COLOR_BACK),
	colorbackeditor(0xffffff),
	colorfrontbreak(0xff0000),
	colorfrontlines(0xffffff),
	colorfronteditor(0x000000)
{
	GlobalScriptViewers().push_back(this);
	this->font=GuiFont::GetFontPool()[2];
}

GuiScript::~GuiScript()
{
	GlobalScriptViewers().remove(this);
}

void GuiScript::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		case ONMOUSEDOWN:
		{
			GuiScrollRect::Procedure(iFrame,iMsg,iData);

			MouseData& md=*(MouseData*)iData;

			if(md.mouse.x < GuiScript::BREAKPOINT_COLUMN_WIDTH)
			{
				unsigned int tBreakOnLine=(md.mouse.y-this->edges.y)/GuiFont::GetDefaultFont()->GetHeight() + 1;

				std::vector<Debugger::Breakpoint>& tAvailableBreakpoints=Debugger::Instance()->GetAllBreakpoint();
				std::vector<Debugger::Breakpoint>& tBreakpoints=Debugger::Instance()->GetBreakpointSet();

				for(size_t i=0;i<tAvailableBreakpoints.size();i++)
				{
					if(tAvailableBreakpoints[i].script==this->editorscript && tAvailableBreakpoints[i].line==tBreakOnLine)
					{
						std::vector<Debugger::Breakpoint>::iterator tFoundedBreakpointIterator=std::find(tBreakpoints.begin(),tBreakpoints.end(),tAvailableBreakpoints[i]);

						bool tAdd=tBreakpoints.end()==tFoundedBreakpointIterator;

						if(tAdd)
							tBreakpoints.push_back(tAvailableBreakpoints[i]);
						else
							tBreakpoints.erase(tFoundedBreakpointIterator);

						Debugger::Instance()->SetBreakpoint(tAvailableBreakpoints[i],tAdd);

						iFrame->SetDraw(this);

						break;
					}
				}
			}

			GuiCaret::Instance()->Show(this,this->colorbackeditor,0x000000);
		}
		break;
		case ONKEYDOWN:
		{
			if(Keyboard::Instance()->IsPressed(0x11 /*VK_CONTROL*/) && !Keyboard::Instance()->IsPressed(0x12 /*VK_ALT*/))
			{
				if(Keyboard::Instance()->IsPressed('S'))
				this->Save();
			}
			else
			{
				KeyData& kd=*(KeyData*)iData;

				StringEditor::Instance()->ParseKeyInput(this->string,this->cursor,kd,this->font);

				GuiCaret::Instance()->SetPos(this->edges.x+this->verticaltoolbar+this->cursor.caret.x,this->edges.y+this->cursor.caret.y);
				GuiCaret::Instance()->SetDim(1,this->font->GetHeight());

			}
		}
		break;
		case ONPAINT:
		{
			vec4	tClippingEdges=this->GetClippingEdges();
			vec4	tContentEdges=this->GetContentEdges();

			//////draw fixed backgrounds//////
			float tBreakpointColRight=this->edges.x+GuiScript::BREAKPOINT_COLUMN_WIDTH;
			float tLineNumbersColLeft=tBreakpointColRight;
			float tLineNumbersColRight=this->edges.x+this->verticaltoolbar;
			float tTextEditorEdgeLeft=tLineNumbersColRight;

			//rect background
			iFrame->renderer2D->DrawRectangle(this->edges.x,this->edges.y,this->edges.z,this->edges.w,this->color);
			//breakpoints column background
			iFrame->renderer2D->DrawRectangle(this->edges.x,this->edges.y,tBreakpointColRight,tClippingEdges.w,this->colorbackbreak);
			//line numbers column background
			iFrame->renderer2D->DrawRectangle(tLineNumbersColLeft,this->edges.y,tLineNumbersColRight,tClippingEdges.w,this->colorbacklines);
			//text background
			iFrame->renderer2D->DrawRectangle(tTextEditorEdgeLeft,this->edges.y,tClippingEdges.z,tClippingEdges.w,this->colorbackeditor);

			//////draw clipped vertical toolbar//////
			iFrame->renderer2D->PushScissor(this->edges.x,this->edges.y,tLineNumbersColRight,tClippingEdges.w);

			this->DrawLineNumbers(iFrame);
			this->DrawBreakpoints(iFrame);

			iFrame->renderer2D->PopScissor();

			//////draw text//////
			float tOffsetx=this->GetScrollbarPosition(0);
			float tOffsety=this->GetScrollbarPosition(1);

			iFrame->renderer2D->PushScissor(tLineNumbersColRight,tClippingEdges.y,tClippingEdges.z,tClippingEdges.w);
			iFrame->renderer2D->Translate(-tOffsetx,-tOffsety);

			//text
			iFrame->renderer2D->DrawText(this->string.c_str(),this->edges.x+this->verticaltoolbar,this->edges.y,this->edges.z,this->edges.w,this->colorfronteditor,this->font);

			iFrame->renderer2D->Translate(0,0);
			iFrame->renderer2D->PopScissor();

			GuiScrollRect::Procedure(iFrame,iMsg,iData);
		}
		break;
		default:
			GuiScrollRect::Procedure(iFrame,iMsg,iData);
	}
}

void GuiScript::CalculateLayout()
{
	this->ResetContent();
	vec4 tContent=this->font->MeasureText2(this->string.c_str());

	this->numrows=tContent.w;
	this->numcols=tContent.z;
	
	this->verticaltoolbar=GuiScript::BREAKPOINT_COLUMN_WIDTH;
	this->verticaltoolbar+=this->font->MeasureText(StringUtils::Int(this->numrows).c_str()).x;
	this->verticaltoolbar+=GuiScript::LINENUMBERS_MARGINS*2.0f;

	this->SetContent(this->verticaltoolbar+tContent.x,tContent.y);
	this->CalculateScrollbarsLayout();
}

void GuiScript::SetEditorScript(EditorScript* iEditorScript)
{
	this->editorscript=iEditorScript;
	this->editorscript->scriptViewer=this;
	this->editorscript->LoadScript();
	this->string=this->editorscript->script;
	this->cursor.p=this->string.c_str();
	this->CalculateLayout();
}

EditorScript* GuiScript::GetEditorScript()
{
	return this->editorscript;
}

void GuiScript::Save()
{
	this->editorscript->script=this->string;
	this->editorscript->SaveScript();
}


bool GuiScript::Compile()
{
	bool exited=false;
	bool compiled=false;
	bool runned=false;

	if(this->editorscript)
	{
		exited=GuiCompiler::Instance()->UnloadScript(this->editorscript);

		compiled=GuiCompiler::Instance()->Compile(this->editorscript);

		runned=GuiCompiler::Instance()->LoadScript(this->editorscript);
	}

	return exited && compiled && runned;
}



void GuiScript::OnKeyUp(Frame* iFrame,const MsgData& iData)
{
	//GuiRect::OnKeyUp(iTab,iData);
}


void GuiScript::OnMouseMove(Frame* iFrame,const MsgData& iData)
{
	/*GuiRect::OnMouseMove(iTab,iData);

	if(this->IsHovering())
	{
		/ *if(iMsg.mouse.x < this->editor->margins.x)
		{

		}
		else
		{

		}* /
	}*/
}


void GuiScript::DrawLineNumbers(Frame* iFrame)
{
	float tTextHeight=this->font->GetHeight();
	float tRowX=this->edges.x+GuiScript::BREAKPOINT_COLUMN_WIDTH+GuiScript::LINENUMBERS_MARGINS;
	float tRowY=this->edges.y;
	float tRowZ=tRowX+this->verticaltoolbar-GuiScript::LINENUMBERS_MARGINS;

	for(int i=0;i<this->numrows;i++)
	{
		iFrame->renderer2D->DrawText(StringUtils::Int(i+1),tRowX,tRowY,tRowZ,tRowY + tTextHeight,this->colorfrontlines,this->font);
		tRowY+=tTextHeight;
	}
}

void GuiScript::DrawBreakpoints(Frame* iFrame)
{
	float tFontHeight=GuiFont::GetDefaultFont()->GetHeight();

	std::vector<Debugger::Breakpoint>& breakpoints=Debugger::Instance()->GetBreakpointSet();

	float tBreakpointRay=GuiScript::BREAKPOINT_COLUMN_WIDTH/3.0f;

	for(size_t i=0;i<breakpoints.size();i++)
	{
		if(breakpoints[i].script==this->editorscript)
		{
			unsigned int tLineInsertion=this->edges.y + (breakpoints[i].line - 1) * tFontHeight;

			unsigned int tBreakColor = breakpoints[i].breaked ? 0xff0000 : 0xffff00 ;

			iFrame->renderer2D->DrawCircle(this->edges.x + tBreakpointRay,tLineInsertion + 10,this->edges.x,tBreakpointRay,tBreakColor);
		}
	}
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////GuiCompilerViewer///////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiCompiler* GuiCompiler::Instance()
{
	GLOBALGETTERFUNCASSINGLETON(GlobalGuiCompilerViewerInstance,GuiCompiler);
}

GuiCompiler::GuiCompiler()
{
	GlobalGuiCompilerViewerInstance()=this;
}

GuiCompiler::~GuiCompiler()
{
}

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


bool GuiCompiler::ParseCompilerOutputFile(String iFileBuffer)
{
	bool bReturnValue=true;

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

			if(simpleMessage)
			{
				this->Log(String(LineBegin,LineEnd-LineBegin));
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

				this->Log(String(FileEnd,ErrorEnd-FileEnd)+String(ErrorEnd,LineEnd-ErrorEnd));
//				tCompilerMessageRow->GetColor(GuiRect::BACKGROUND)=0xff0000;
			}

			LineBegin=++LineEnd;
		}
	}

	return bReturnValue;
	return false;
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
	sceneLabel(this),
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
	pcAABB(L"AABB",&this->bbox)
{
	this->TreeViewNode().SetLabel(L"Entity");
	this->instancedEntitiesNumber++;
}

unsigned int EditorEntity::instancedEntitiesNumber=0;

EditorEntity::~EditorEntity()
{
	this->instancedEntitiesNumber--;
}

EditorEntity* EditorEntity::Parent(){return (EditorEntity*)this->Entity::Parent();}

const std::list<EditorEntity*>& EditorEntity::Childs(){return (std::list<EditorEntity*>&)this->Entity::Childs();}

void EditorEntity::SetName(String iName)
{
	this->Entity::name=iName;
	this->sceneLabel.SetLabel(this->name);
}

SceneEntityLabel&	EditorEntity::GetSceneLabel()
{
	return this->sceneLabel;
}

unsigned int	EditorEntity::GetInstancedEntitiesNumber()
{
	return EditorEntity::instancedEntitiesNumber;
}

EditorEntity*	EditorEntity::Append(EditorEntity* iEntity)
{
	this->Entity::Append(iEntity);
	this->sceneLabel.Insert(iEntity->sceneLabel);
	return iEntity;
}
void	EditorEntity::Remove(EditorEntity* iEntity)
{
	this->Entity::Remove(iEntity);
	this->sceneLabel.Remove(iEntity->sceneLabel);
}

void EditorEntity::DestroyChilds(){}

void EditorEntity::OnPropertiesCreate()
{
	this->TreeViewContainer().Insert(pName);
	this->TreeViewContainer().Insert(pPtr);
	this->TreeViewContainer().Insert(pId);
	this->TreeViewContainer().Insert(pPosition);
	this->TreeViewContainer().Insert(pChilds);

	this->TreeViewContainer().Insert(pcAABB);

	pcAABB.Insert(pMin);
	pcAABB.Insert(pMax);
	pcAABB.Insert(pVolume);
}

void EditorEntity::OnPropertiesUpdate(Frame* tab)
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
{
	this->TreeViewContainer().SetLabel(L"Mesh");
}

void EditorMesh::OnPropertiesCreate()
{
	this->TreeViewContainer().Insert(pControlpoints);
	this->TreeViewContainer().Insert(pNormals);
	this->TreeViewContainer().Insert(pPolygons);
	this->TreeViewContainer().Insert(pTextCoord);
	this->TreeViewContainer().Insert(pVertexIndices);
}

void EditorMesh::OnPropertiesUpdate(Frame* tab)
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
{
	this->TreeViewContainer().SetLabel(L"Skin");
}

void EditorSkin::OnPropertiesCreate()
{
	this->TreeViewContainer().Insert(pClusters);
	this->TreeViewContainer().Insert(pTextures);
}
void EditorSkin::OnPropertiesUpdate(Frame* tab)
{
}
void EditorRoot::OnPropertiesCreate()
{
	/*this->PropertyContainer().SetName(L"Root");
	this->Entity()->PropertyContainer().Append(this->PropertyContainer());*/
}
void EditorRoot::OnPropertiesUpdate(Frame* tab)
{
}
void EditorSkeleton::OnPropertiesCreate()
{
	/*this->PropertyContainer().SetName(L"Skeleton");
	this->Entity()->PropertyContainer().Append(this->PropertyContainer());*/
}
void EditorSkeleton::OnPropertiesUpdate(Frame* tab)
{
}
void EditorGizmo::OnPropertiesCreate()
{
	/*this->PropertyContainer().SetName(L"Sizmo");
	this->Entity()->PropertyContainer().Append(this->PropertyContainer());*/
}
void EditorGizmo::OnPropertiesUpdate(Frame* tab)
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
{
	this->TreeViewContainer().SetLabel(L"Animation");
}

void EditorAnimation::OnPropertiesCreate()
{
	this->TreeViewContainer().Insert(pIsBone);
	this->TreeViewContainer().Insert(pDuration);
	this->TreeViewContainer().Insert(pBegin);
	this->TreeViewContainer().Insert(pEnd);
}
void EditorAnimation::OnPropertiesUpdate(Frame* tab)
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
{
	this->TreeViewContainer().SetLabel(L"AnimationController");
}

EditorAnimationController::~EditorAnimationController(){}

void EditorAnimationController::OnPropertiesCreate()
{
	this->TreeViewContainer().Insert(pNumNodes);
	this->TreeViewContainer().Insert(pBegin);
	this->TreeViewContainer().Insert(pEnd);
	this->TreeViewContainer().Insert(pDuration);
	this->TreeViewContainer().Insert(pPlayer);
	this->TreeViewContainer().Insert(pVelocity);
}


void EditorAnimationController::OnPropertiesUpdate(Frame* tab)
{
	/*if(this->TreeViewContainer().Expanded() && this->oldCursor!=this->cursor)
	{
		tab->SetDraw(&this->acpGuiAnimationController.slider);

		this->oldCursor=this->cursor;
	}*/
}

EditorLine::EditorLine():
	spNumSegments(&this->points,GuiStringProperty::VEC3LISTSIZE),
	pNumSegments(L"NumSegments",spNumSegments)
{
	this->TreeViewContainer().SetLabel(L"Line");
}

void EditorLine::OnPropertiesCreate()
{
	this->TreeViewContainer().Insert(pNumSegments);

	/*for(std::list<vec3>::iterator i=this->points.begin();i!=this->points.end();i++)
	{
		GuiListBox::Item* tPoint=new GuiListBox::Item(*i,GuiStringProperty::VEC3);

		this->pointItems.push_back(tPoint);
		this->pointListBox.Append(tPoint);
	}*/
}
void EditorLine::OnPropertiesUpdate(Frame* tab)
{
}

void EditorLine::DestroyPoints()
{
	//this->pointListBox.Destroy();
}
void EditorLine::Append(vec3 iPoint)
{
	/*this->points.push_back(iPoint);
	vec3* tVec3=&this->points.back();

	GuiListBox::Item* tPoint=new GuiListBox::Item(tVec3,GuiStringProperty::VEC3);

	this->pointItems.push_back(tPoint);
	this->pointListBox.Append(tPoint);*/
}


void EditorBone::OnPropertiesCreate()
{
	this->TreeViewContainer().SetLabel(L"Bone");
}
void EditorBone::OnPropertiesUpdate(Frame* tab)
{
}
void EditorLight::OnPropertiesCreate()
{
	/*this->PropertyContainer().SetName(L"Light");
	this->Entity()->PropertyContainer().Append(this->PropertyContainer());*/
}
void EditorLight::OnPropertiesUpdate(Frame* tab)
{
}


///////////////////EditorScript////////////////

EditorScript::EditorScriptButtons::EditorScriptButtons(EditorScript* iEditorScript):editorscript(iEditorScript)
{
	this->buttonCompile.SetText(L"Compile");
	this->buttonEdit.SetText(L"Edit");
	this->buttonLaunch.SetText(L"Launch");

	this->Append(&this->buttonCompile);
	this->Append(&this->buttonEdit);
	this->Append(&this->buttonLaunch);
}

void EditorScript::EditorScriptButtons::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		case ONSIZE:
		{
			float ty=this->edges.y;

			this->buttonCompile.edges=this->edges;
			this->buttonEdit.edges=this->edges;
			this->buttonLaunch.edges=this->edges;

			this->buttonCompile.edges.y=ty;
			this->buttonCompile.edges.w=ty+20;
			this->buttonEdit.edges.y=ty+20;
			this->buttonEdit.edges.w=ty+40;
			this->buttonLaunch.edges.y=ty+40;
			this->buttonLaunch.edges.w=ty+60;
		}
		break;
		case ONCONTROLEVENT:
		{
			ControlData* cd=(ControlData*)iData;

			if(cd && cd->msg==ONMOUSEUP)
			{
				if(cd->control==&this->buttonEdit)
				{
					GuiScript* tBindedGuiScript=0;

					for(std::list<GuiScript*>::iterator i=GuiScript::GetPool().begin();i!=GuiScript::GetPool().end();i++)
					{
						if((*i)->GetEditorScript()==this->editorscript)
							tBindedGuiScript=*i;
					}

					if(tBindedGuiScript)
						tBindedGuiScript->GetRoot()->SetTab(tBindedGuiScript);
					else
					{
						GuiScript* tGuiScript=GuiScript::Instance();
						tGuiScript->SetEditorScript(this->editorscript);

						iFrame->GetViewers().front()->AddTab(tGuiScript,L"Script");
					}
				}
				else if(cd->control==&this->buttonCompile)
				{
					GuiCompiler::Instance()->Compile(this->editorscript);
				}
				else if(cd->control==&this->buttonLaunch)
				{

				}
			}
		}
		break;
		default:
			GuiRect::Procedure(iFrame,iMsg,iData);
	}
}
void EditorScript::EditorScriptButtons::SetColor(unsigned int iColor)
{
	GuiRect::SetColor(iColor);

	this->buttonCompile.SetColor(iColor);
	this->buttonEdit.SetColor(iColor);
	this->buttonLaunch.SetColor(iColor);
}

EditorScript::EditorScript():
	scriptViewer(0),
	spButtons(this),
	spFilePath(&this->Script::scriptpath,GuiStringProperty::STRING),
	spIsRunning(&this->Script::runtime,GuiStringProperty::BOOLPTR),
	pFilePath(L"Path",spFilePath),
	pIsRunning(L"Running",spIsRunning),
	pActions(L"Actions",spButtons,60)
{

};

void EditorScript::OnPropertiesCreate()
{
	this->TreeViewNode().SetLabel(L"Script");

	this->TreeViewContainer().Insert(pFilePath);
	this->TreeViewContainer().Insert(pIsRunning);
	this->TreeViewContainer().Insert(pActions);
}
void EditorScript::OnPropertiesUpdate(Frame* tab)
{
}

void EditorScript::SaveScript()
{
	StringUtils::WriteCharFile(this->scriptpath,this->script,L"wb");
}

void EditorScript::LoadScript()
{
	this->script=StringUtils::ReadCharFile(this->scriptpath,L"rb");
}

void EditorScript::OnResourcesCreate()
{
	if(this->scriptpath.empty())
		this->scriptpath=Ide::Instance()->folderProject + L"\\" + this->Entity()->name + L".cpp";
	
	if(!File::Exist(this->scriptpath))
	{
		if(!File::Create(this->scriptpath))
			DEBUG_BREAK();

		this->script=L"#include \"entities.h\"\n\nstruct " + this->Entity()->name + L"_ : EntityScript\n{\n\t int counter;\n\tvoid init()\n\t{\n\t\tcounter=0;\n\tthis->entity->local.identity();\n\t\tprintf(\"inited\\n\");\n\t}\n\n\tvoid update()\n\t{\n\t\tthis->entity->local.translate(0.1f,0,0);\n\t//printf(\"counter: %d\\n\",counter);\n\tcounter++;\n\t}\n\n\tvoid deinit()\n\t{\n\t\tprintf(\"deinited\\n\");\n\t}\n\n};\n";

		this->SaveScript();
	}

	GuiProject::Instance()->Reset();
	GuiProject::Instance()->Scan();

	this->resourceNode=ResourceNodeDir::FindFileNode(this->scriptpath);

	if(!this->resourceNode)
		DEBUG_BREAK();
}

void EditorCamera::OnPropertiesCreate()
{
	this->TreeViewNode().SetLabel(L"Camera");
}
void EditorCamera::OnPropertiesUpdate(Frame* tab)
{
}

/////////////////FileSystem////////////////////

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

/////////////////PluginSystem/////////////////

PluginSystem::Plugin::Plugin():loaded(false),name(L"Plugin"),handle(0),listBoxItem(this){}

void PluginSystem::Plugin::Load()
{
	this->loaded=true;
}

void PluginSystem::Plugin::Unload()
{
	this->loaded=false;
}

PluginSystem::Plugin::PluginListBoxItem::PluginListBoxItem(Plugin* iPlugin):GuiListBoxItem(iPlugin){}

void PluginSystem::Plugin::PluginListBoxItem::OnPaint(GuiListBox*,Frame* iFrame,const vec4& iEdges)
{
	iFrame->renderer2D->DrawCircle(iEdges.x+15,iEdges.y+10,7.5f,this->GetValue()->loaded ? 0x00ff00 : 0xff0000);
	iFrame->renderer2D->DrawText(this->GetLabel(),iEdges.x+30,iEdges.y,iEdges.z,iEdges.w,vec2(0,0.5f),vec2(0,0.5f));
}
void PluginSystem::Plugin::PluginListBoxItem::OnMouseDown(GuiListBox*,Frame* iFrame,const vec4& iEdges,const MouseData& iMd)
{
	
}
void PluginSystem::Plugin::PluginListBoxItem::OnMouseUp(GuiListBox* iListBox,Frame* iFrame,const vec4& iEdges,const MouseData& iMd)
{
	if(iMd.button==1 && CircleContainsPoint(iEdges.x+15,iEdges.y+10,7.5f,iMd.mouse))
	{
		this->GetValue()->loaded ? this->GetValue()->Unload() : this->GetValue()->Load();
		iFrame->SetDraw(iListBox,true,false,this);
	}
}
void PluginSystem::Plugin::PluginListBoxItem::OnMouseMove(GuiListBox*,Frame* iFrame,const vec4& iEdges,const MouseData& iMd)
{

}

PluginSystem::PluginSystem():pluginsTab(this){}
PluginSystem::~PluginSystem(){}	

PluginSystem* PluginSystem::Instance()
{
	GLOBALGETTERFUNCASSINGLETON(GlobalPluginSystemInstance,PluginSystem);
}

PluginSystem::GuiPluginTab::GuiPluginTab(PluginSystem* iPluginSystem):pluginSystem(iPluginSystem)
{
	this->exitButton.SetText(L"Exit");
	this->Append(&this->exitButton);
}

void PluginSystem::GuiPluginTab::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		case ONCONTROLEVENT:
		{
			ControlData* cd=(ControlData*)iData;

			if(cd && cd->control==&this->exitButton && cd->msg==ONMOUSEUP)
			{
				MainFrame::Instance()->DestroyFrame(this->GetRoot()->GetFrame());
				MainFrame::Instance()->Enable(true);
			}
		}
		break;
		default:
			GuiListBox::Procedure(iFrame,iMsg,iData);
	}
}

void PluginSystem::GuiPluginTab::OnSize(Frame* iFrame,const vec4& iEdges)
{
	GuiRect::OnSize(iFrame,iEdges);

	this->exitButton.edges.make(iEdges.z-110,iEdges.w-25,iEdges.z-10,iEdges.w-5);
}



void PluginSystem::ShowGui()
{
	this->ScanPluginsDirectory();

	MainFrame*	tMainFrame=MainFrame::Instance();
	Frame*		tFrame=0;
	GuiViewer*	tViewer=0;

	tMainFrame->Enable(false);

	vec2 tIdeFrameSize=tMainFrame->GetFrame()->windowData->Size();
	vec2 tTabSize(500,300);
	vec2 tTabPos=tMainFrame->GetFrame()->windowData->Pos();

	tTabPos.x+=tIdeFrameSize.x/2.0f-tTabSize.x/2.0f;
	tTabPos.y+=tIdeFrameSize.y/2.0f-tTabSize.y/2.0f;

	tFrame=tMainFrame->CreateFrame(tTabPos.x,tTabPos.y,tTabSize.x,tTabSize.y,0,true);
	tViewer=tFrame->GetViewers().front();
	
	tViewer->edges.make(tTabPos.x,tTabPos.y,tTabPos.x+tTabSize.x,tTabPos.y+tTabSize.y);

	for(std::list<Plugin*>::const_iterator i=this->plugins.begin();i!=this->plugins.end();i++)
	{
		Plugin* tPlugin=(*i);

		this->pluginsTab.InsertItem(tPlugin->listBoxItem);
	}

	tViewer->AddTab(&this->pluginsTab,L"Plugins");
	
	tFrame->OnSize(tFrame->Size().x,tFrame->Size().y);
}

////////////////////Subsystem//////////////////

String Subsystem::RandomDir(String iWhere,int iSize,String iAlphabet)
{
	String tRandomWorkingDirectoryName;
	String tRandomWorkingDirectory;

	while(true)
	{
		tRandomWorkingDirectoryName=StringUtils::RandomString(iSize,iAlphabet);

		String tFullRandomDirName=iWhere + L"\\" + tRandomWorkingDirectoryName;

		if(!Subsystem::DirectoryExist(tRandomWorkingDirectory.c_str()))
			break;
	}

	tRandomWorkingDirectory=iWhere + L"\\" + tRandomWorkingDirectoryName;

	if(!Subsystem::CreateDir(tRandomWorkingDirectory.c_str()))
		DEBUG_BREAK();

	if(!Subsystem::DirectoryExist(tRandomWorkingDirectory.c_str()))
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

	StringUtils::WriteWstring(iFile,tScript->scriptpath);
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
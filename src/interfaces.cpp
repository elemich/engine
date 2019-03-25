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

GLOBALGETTERFUNC(GlobalGuiProjectViewerInstance,GuiProject*);
GLOBALGETTERFUNC(GlobalGuiSceneViewerInstance,GuiScene*);
GLOBALGETTERFUNC(GlobalGuiCompilerViewerInstance,std::list<GuiCompiler*>);
GLOBALGETTERFUNC(GlobalGuiConsoleViewerInstance,GuiConsole*);
GLOBALGETTERFUNC(GlobalDefaultFontInstance,GuiFont*);
GLOBALGETTERFUNC(GlobalFontPoolInstance,std::vector<GuiFont*>);
GLOBALGETTERFUNC(GlobalIdeInstance,Ide*);
GLOBALGETTERFUNC(GlobalScriptViewers,std::list<GuiScript*>);
GLOBALGETTERFUNC(GlobalViewports,std::list<GuiViewport*>);
GLOBALGETTERFUNC(GlobalRootProjectDirectory,ResourceNodeDir*);
GLOBALGETTERFUNC(GlobalGuiEntityViewerInstance,std::list<GuiEntity*>);

bool Contains(const vec4& ivec,const vec2& ipoint)
{
	return (ipoint.x>ivec.x && ipoint.x<ivec.z && ipoint.y>ivec.y && ipoint.y<ivec.w);
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

void SetFlag(unsigned int& iFlags,unsigned int iFlag,bool& iValue)
{
	iFlags ^= (-iValue^iFlags) & (1UL << iFlag);
}

bool GetFlag(unsigned int& iFlags,unsigned int iFlag)
{
	return (iFlags >> iFlag) & 1U;
}


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


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////AppInterface////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////




Ide::Ide():
	timer(0),
	mainframe(0),
	compiler(0),
	processId(0),
	processThreadId(0),
	stringeditor(0),
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

MainFrame*	Ide::GetMainFrame()
{
	return this->mainframe;
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

bool MouseInput::Left(){return this->buttons[0];}
bool MouseInput::Right(){return this->buttons[1];}
bool MouseInput::Middle(){return this->buttons[2];}

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
	wasrender(false),
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
	pressed(0),
	hovered(0),
	isModal(false),
	hasFrame(true),
	mousedata(vec2(),0,0,Ide::Instance()->inputManager.mouseInput),
	dragcode(0),
	sizeboxcode(0)
{
	hittest.hit=0;
	hittest.locked=false;
	hittest.dragrect[0]=0;
	hittest.dragrect[1]=0;
	hittest.dragrect[2]=0;
	hittest.dragrect[3]=0;
}

Frame::~Frame()
{
}




void Frame::DrawBlock(bool iBool)
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

struct DLLBUILD MsgClipData
{
	vec4 clip;
	vec2 coord;
};

void Frame::Render()
{
	for(std::list<GuiViewport*>::iterator i=GuiViewport::GetPool().begin();i!=GuiViewport::GetPool().end();i++)
	{
		(*i)->Render(this);
	}
}

void Frame::Draw()
{
	/*if(this->drawTask->pause)
		DEBUG_BREAK();*/

	if(!this->drawInstances.empty())
	{
		PaintData tDrawEvent;

		for(std::list<DrawInstance*>::iterator it=this->drawInstances.begin();it!=this->drawInstances.end();it)
		{
			DrawInstance*& tDrawInstance=*it;

			if(tDrawInstance && !tDrawInstance->skip)
			{
				if(this->BeginDraw())
				{
					if(!tDrawInstance->rect)
						this->OnPaint();
					else
						this->BroadcastPaintTo(tDrawInstance->rect,tDrawInstance->param);

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

	Ide::Instance()->stringeditor->Draw(this);
}

DrawInstance* Frame::SetDraw(GuiRect* iRect,bool iRemove,bool iSkip,void* iParam)
{
	DrawInstance* newInstance=new DrawInstance(iRect,iRemove,iSkip,iParam);
	this->drawInstances.push_back(newInstance);
	return newInstance;
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

void Frame::BroadcastTo(GuiRect* iRect,GuiRectMessages iMsg,void* iData)
{
	if(iRect && iRect->GetFlag(GuiRectFlag::ACTIVE))
	{
		switch(iMsg)
		{
			case ONHITTEST:
			{
				HitTestData& thtd=*(HitTestData*)iData;

				vec2	tMouse=thtd.mouse;

				this->Message(iRect,iMsg,iData);

				for(std::list<GuiRect*>::const_iterator i=iRect->Childs().begin();i!=iRect->Childs().end();i++)
					this->BroadcastTo(*i,iMsg,iData);

				//thtd.mouse=tMouse;
			}
			break;
			default:
			{
				this->Message(iRect,iMsg,iData);
				for(std::list<GuiRect*>::const_iterator i=iRect->Childs().begin();i!=iRect->Childs().end();i++)
					this->BroadcastTo(*i,iMsg,iData);
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
	this->BroadcastTo(iRect,GuiRectMessages::ONPAINT,iData);

	/*GuiRect*			tRect=iRect->GetParent();
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

	this->BroadcastTo(iRect,GuiRectMessages::ONPAINT,tMsg);

	int tClips=this->clips.size();

	for(int i=0;i<tClips;i++)
		this->PopScissor();*/
};

void Frame::OnSize(float iWidth,float iHeight)
{
	if(this->viewers.size()==1)
	{
		this->viewers.front()->edges.make(0,0,iWidth,iHeight);
		this->Message(this->viewers.front(),GuiRectMessages::ONSIZE,0);
	}
	else
	{
		for(std::list<GuiViewer*>::iterator i=this->viewers.begin();i!=this->viewers.end();i++)
		{
			vec4& tEdges=(*i)->edges;

			if(tEdges.x!=0)
				tEdges.x=iWidth*(1.0f/(this->previousSize.x/tEdges.x));
			if(tEdges.y!=0)
				tEdges.y=iHeight*(1.0f/(this->previousSize.y/tEdges.y));

			tEdges.z=iWidth*(1.0f/(this->previousSize.x/tEdges.z));
			tEdges.w=iHeight*(1.0f/(this->previousSize.y/tEdges.w));

			this->Message(*i,GuiRectMessages::ONSIZE,0);
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

			this->Broadcast(GuiRectMessages::ONHITTEST,&htd);

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
				this->Message(old,GuiRectMessages::ONMOUSEEXIT,0);
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
				(*i)->edges.z=this->mouse.x,this->Message(*i,GuiRectMessages::ONSIZE,0);

			for(std::list<GuiViewer*>::iterator i=this->draggables[1].begin();i!=this->draggables[1].end();i++)
				(*i)->edges.x=this->mouse.x+4,this->Message(*i,GuiRectMessages::ONSIZE,0);
		}
		else if(this->dragcode==3)
		{
			for(std::list<GuiViewer*>::iterator i=this->draggables[0].begin();i!=this->draggables[0].end();i++)
				(*i)->edges.w=this->mouse.y,this->Message(*i,GuiRectMessages::ONSIZE,0);

			for(std::list<GuiViewer*>::iterator i=this->draggables[2].begin();i!=this->draggables[2].end();i++)
				(*i)->edges.y=this->mouse.y+4,this->Message(*i,GuiRectMessages::ONSIZE,0);
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

void Frame::OnMouseUp(unsigned int iButton)
{
	if(this->drag)
	{
		for(int i=0;i<4;i++)
			this->draggables[i].clear();

		this->dragcode=0;
	}

	if(iButton==1)
		this->hittest.locked=this->drag=false;

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

	this->renderer2D->DrawRectangle(0,0,iTabDim.x,iTabDim.y/*-Tab::CONTAINER_HEIGHT*/,0x202020);

	this->Broadcast(GuiRectMessages::ONPAINT,0);
}


void Frame::OnKeyDown(char iKey)
{
	KeyData tKeyData={iKey};
	this->Broadcast(GuiRectMessages::ONKEYDOWN,&tKeyData);
}

void Frame::OnKeyUp(char iKey)
{
	KeyData tKeyData={iKey};
	this->Broadcast(GuiRectMessages::ONKEYUP,&tKeyData);
}

void Frame::OnRecreateTarget()
{
	this->Broadcast(GuiRectMessages::ONRECREATETARGET,0);
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

void Frame::SetHover(GuiRect* iHoveredRect)
{
	/*bool tHoveringHasChanged=(Tab::hovered!=iHoveredRect);

	if(tHoveringHasChanged && Tab::hovered)
		this->SetDraw(Tab::hovered);

	Tab::hovered=iHoveredRect;

	if(tHoveringHasChanged && iHoveredRect)
		this->SetDraw(iHoveredRect);*/
}

void Frame::SetPressed(GuiRect* iPressedRect)
{
	Frame::pressed=iPressedRect;
}

GuiRect* Frame::GetFocus()
{
	return Frame::focused;
}

GuiRect* Frame::GetPressed()
{
	return Frame::pressed;
}

GuiRect* Frame::GetHover()
{
	return hittest.hit;
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

void StringEditor::Draw(Frame* iCallerTab)
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



Renderer2D::Renderer2D(Frame* iFrame):
	frame(iFrame),
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

GuiViewer* GuiRect::GetRoot()
{
	return this->parent ? this->parent->GetRoot() : (GuiViewer*)this;
}

void GuiRect::Append(GuiRect* iChild,bool isChild)
{
	this->childs.push_back(iChild);
	iChild->parent=this;
	this->SetFlag(GuiRectFlag::CHILD,isChild);
}

void GuiRect::Remove(GuiRect* iChild)
{
	this->childs.remove(iChild);
	iChild->parent=0;
}

GuiRect::GuiRect(unsigned int iState):parent(0),flags(iState){}

void GuiRect::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		case ONHITTEST:
		{
			HitTestData& thtd=*(HitTestData*)iData;

			if(Contains(this->edges,thtd.mouse))
				thtd.hit=this;
		}
		break;
		case ONMOUSEDOWN:
		{
			MouseData& tmd=*(MouseData*)iData;

			if(tmd.button==1)
				iFrame->hittest.locked=true;
		}
		break;
		case ONPAINT:
		{
			iFrame->renderer2D->DrawRectangle(this->edges.x,this->edges.y,this->edges.z,this->edges.w,0x505050);
		}
		break;
	}
}

////////////////////GuiViewer//////////////////

GuiViewer::GuiViewer():tab(0),frame(0)
{
}

GuiViewer::GuiViewer(float x,float y,float z,float w):tab(0),frame(0)
{
	this->edges.make(x,y,z,w);
}

void GuiViewer::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		case ONHITTEST:
		{
			HitTestData& thtd=*(HitTestData*)iData;

			vec4 tSizeEdges=this->GetSizeEdges();

			if(Contains(tSizeEdges,thtd.mouse))
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
		case ONSIZE:
		{
			GuiRect::Procedure(iFrame,iMsg,iData);

			vec4	tTabEdges=this->GetTabEdges();
			iFrame->BroadcastTo(this->tab,iMsg,&tTabEdges);
		}
		break;
		case ONMOUSEUP:
		{
			MouseData& tmd=*(MouseData*)iData;

			vec4 tBar(this->edges.x,this->edges.y,this->edges.z,this->edges.y+BAR_HEIGHT);

			if(tmd.button==2 && Contains(tBar,tmd.mouse))
			{
				int tabNumberHasChanged=this->tabs.size();

				vec4 tLabel;

				tLabel.x=this->edges.x;
				tLabel.y=this->edges.y + (BAR_HEIGHT-LABEL_HEIGHT);
				tLabel.z=tLabel.x+LABEL_LEFT_OFFSET;
				tLabel.w=tLabel.y+LABEL_HEIGHT;

				for(std::list<String>::iterator i=this->labels.begin();i!=this->labels.end();i++)
				{
					vec2 tTextSize = this->frame->renderer2D->MeasureText((*i).c_str());

					tLabel.x=tLabel.z;
					tLabel.z=tLabel.x + tTextSize.x + LABEL_RIGHT_OFFSET;

					if(tmd.mouse.x>tLabel.x && tmd.mouse.x<tLabel.z && tmd.mouse.y>tLabel.y &&  tmd.mouse.y<tLabel.w)
					{
						int menuResult=this->frame->TrackTabMenuPopup();

						switch(menuResult)
						{
						case 1:
							if(this->tabs.size()>1)
							{
								//this->Destroy();
							}
							break;
						case 2:
							//this->selected=this->rects.Append(new GuiViewport());
							break;
						case 3:
							this->AddTab(GuiScene::Instance(),L"");
							break;
						case 4:
							this->AddTab(GuiEntity::Instance(),L"");
							break;
						case 5:
							this->AddTab(GuiProject::Instance(),L"");
							break;
						case 6:
							/*this->tabs.ScriptViewer();
							this->selected=this->tabs.childs.size()-1;*/
							break;
						}

						break;
					}
				}

				if(tabNumberHasChanged!=this->tabs.size())
					this->frame->SetDraw();
			}

			iFrame->BroadcastTo(this->tab,iMsg,iData);
		}
		break;
		case ONPAINT:
		{
			iFrame->renderer2D->DrawRectangle(this->edges.x,this->edges.y,this->edges.z,this->edges.w,0x707070);

			vec4 tLabel;

			tLabel.x=this->edges.x;
			tLabel.y=this->edges.y + (BAR_HEIGHT-LABEL_HEIGHT);
			tLabel.z=tLabel.x+LABEL_LEFT_OFFSET;
			tLabel.w=tLabel.y+LABEL_HEIGHT;

			//render label text
			std::list<String>::iterator labelIter=this->labels.begin();
			std::list<GuiRect*>::iterator tabIter=this->tabs.begin();
			for(;labelIter!=this->labels.end();labelIter++,tabIter++)
			{
				vec2 tTextSize = this->frame->renderer2D->MeasureText((*labelIter).c_str());

				tLabel.x=tLabel.z;
				tLabel.z=tLabel.x + tTextSize.x + LABEL_RIGHT_OFFSET;

				if(this->tab==*tabIter)
					this->frame->renderer2D->DrawRectangle(tLabel.x,tLabel.y,tLabel.z,tLabel.w,Frame::COLOR_LABEL);

				this->frame->renderer2D->DrawText(*labelIter,tLabel.x,tLabel.y,tLabel.z,tLabel.w,vec2(0.5f,0.5f),vec2(0.5f,0.5f),GuiString::COLOR_TEXT);
			}

			iFrame->BroadcastTo(this->tab,iMsg,iData);
		}
		break;
		default:
		{
			GuiRect::Procedure(iFrame,iMsg,iData);
			iFrame->BroadcastTo(this->tab,iMsg,iData);
		}
	}
}

GuiRect* GuiViewer::AddTab(GuiRect* iTab,String iLabel)
{
	iTab->parent=this;
	this->tabs.push_back(iTab);
	this->labels.push_back(iLabel);
	this->SetTab(iTab);
	return iTab;
}

void GuiViewer::RemoveTab(GuiRect* iTab)
{
	this->tabs.remove(iTab);
	iTab->parent=0;
}

void GuiViewer::SetTab(GuiRect* iTab)
{
	if(this->tab && this->tab!=iTab)
	{
		if(this->tab->GetFlag(GuiRectFlag::ACTIVE))
		{
			this->frame->Message(this->tab,GuiRectMessages::ONDEACTIVATE,0);
			this->tab->SetFlag(GuiRectFlag::ACTIVE,false);
		}
	}

	this->tab=iTab;

	if(iTab)
	{
		if(!iTab->GetFlag(GuiRectFlag::ACTIVE))
		{
			this->frame->Message(iTab,GuiRectMessages::ONACTIVATE,0);
			iTab->SetFlag(GuiRectFlag::ACTIVE,true);
		}
		this->frame->Message(this,GuiRectMessages::ONSIZE,0);
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

vec4 GuiViewer::GetSizeEdges()
{
	return vec4(this->edges.x-4,this->edges.y-4,this->edges.z+4,this->edges.w+4);
}

Frame* GuiViewer::GetFrame()
{
	return this->frame;
}

////////////////////GuiListBoxNode////////////////

GuiListBoxNode::GuiListBoxNode():flags(0){}

GuiListBox::GuiListBox():hovered(0){}

void			GuiListBoxNode::SetLabel(const String& iString)
{
	this->label=iString;
}

const String&	GuiListBoxNode::SetLabel()
{
	return this->label;
}

void  GuiListBoxNode::OnPaint(Frame* iFrame,const vec4& iEdges)
{
	iFrame->renderer2D->DrawText(this->label,iEdges.x,iEdges.y,iEdges.z,iEdges.w,vec2(0,0.5f),vec2(0,0.5f),0xffffffff);
}
float GuiListBoxNode::OnCalcWidth()
{
	return GuiFont::GetDefaultFont()->MeasureText(this->label.c_str()).x;
}
float GuiListBoxNode::OnCalcHeight()
{
	return Frame::ICON_WH;
}
void  GuiListBoxNode::OnMouseDown(Frame*,const MouseData&){}
void  GuiListBoxNode::OnMouseUp(Frame*,const MouseData&){}

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

void GuiListBox::CalculateLayout()	
{
	this->ResetContent();

	vec4		tClippingEdges=this->GetClippingEdges();
	float		tTop=this->edges.y;

	for(std::list<GuiListBoxNode*>::iterator i=this->items.begin();i!=this->items.end();i++)
	{
		float tWidth=(*i)->OnCalcWidth();
		tTop+=(*i)->OnCalcHeight();
		
		this->SetContent(tWidth,tTop-this->edges.y);
	}

	this->CalculateScrollbarsLayout();
}

void GuiListBox::ItemProcedure(GuiListBoxNode* iItem,Frame* iFrame,GuiRectMessages iMsg,ListBoxItemData& iLbid)
{
	switch(iMsg)
	{
		case ONHITTEST:
		{
			HitTestData& thtd=*(HitTestData*)iLbid.data;

			vec4 tItemEdges(iLbid.edges.x,iLbid.top,iLbid.edges.z,iLbid.top+iItem->OnCalcHeight());

			if(Contains(tItemEdges,thtd.mouse))
			{
				//draw previous hitted node
				if(this->hovered)
				{
					this->SetFlag(this->hovered,GuiListBoxNode::HIGHLIGHTED,false);
					iFrame->SetDraw(this,true,false,this->hovered);
				}

				//draw current hitted node
				this->hovered=iItem;
				this->SetFlag(this->hovered,GuiListBoxNode::HIGHLIGHTED,true);
				iFrame->SetDraw(this,true,false,this->hovered);
				
				iLbid.skip=true;
			}
		}
		break;
		case ONPAINT:
		{
			bool tHighligh=this->GetFlag(iItem,GuiListBoxNode::HIGHLIGHTED);
			bool tSelected=this->GetFlag(iItem,GuiListBoxNode::SELECTED);

			float tBottom=iLbid.top+iItem->OnCalcHeight();

			unsigned int tColor=(iLbid.idx%2) ? 0x555555 : 0x606060;

			if(tSelected)
				tColor=BlendColor(0x0000ff,tColor);
			if(tHighligh)
				tColor=BlendColor(0x00ffff,tColor);

			//base rect
			iFrame->renderer2D->DrawRectangle(iLbid.edges.x,iLbid.top,iLbid.edges.z,tBottom,tColor);

			vec4 tEdges(iLbid.edges.x,iLbid.top,iLbid.edges.z,tBottom);

			iItem->OnPaint(iFrame,tEdges);
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
					ListBoxItemData tLbid={tContentEdges,tContentEdges.y,0,&thtd,false};

					for(std::list<GuiListBoxNode*>::iterator i=this->items.begin();i!=this->items.end() && !tLbid.skip;i++)
					{
						this->ItemProcedure(*i,iFrame,iMsg,tLbid);
						tLbid.idx++;
						tLbid.top+=(*i)->OnCalcHeight();
					}
				}
				else if(this->hovered)
				{
					this->SetFlag(this->hovered,GuiListBoxNode::HIGHLIGHTED,false);
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
				this->hovered->OnMouseUp(iFrame,*(MouseData*)iData);
		}
		break;
		case ONMOUSEEXIT:
		{
			//draw previous hitted node
			if(this->hovered)
			{
				this->SetFlag(this->hovered,GuiListBoxNode::HIGHLIGHTED,false);
				iFrame->SetDraw(this,true,false,this->hovered);
				this->hovered=0;
			}
		}
		break;
		case ONPAINT:
		{
			if(iData==0)GuiRect::Procedure(iFrame,iMsg,iData);

			vec4 tClippingEdges=this->GetClippingEdges();
			vec4 tContentEdges=this->GetContentEdges();

			float tOffsetx=this->GetScrollbarPosition(0);
			float tOffsety=this->GetScrollbarPosition(1);

			iFrame->renderer2D->PushScissor(tClippingEdges.x,tClippingEdges.y,tClippingEdges.z,tClippingEdges.w);
			iFrame->renderer2D->Translate(-tOffsetx,-tOffsety);

			float tTop=tContentEdges.y;

			ListBoxItemData tLbid={tContentEdges,tContentEdges.y,0,iData,false};

			for(std::list<GuiListBoxNode*>::iterator i=this->items.begin();i!=this->items.end() && !tLbid.skip;i++)
			{
				if(iData==0 || iData==*i)
					this->ItemProcedure(*i,iFrame,iMsg,tLbid);

				tLbid.idx++;
				tLbid.top+=(*i)->OnCalcHeight();
			}

			iFrame->renderer2D->Translate(0,0);
			iFrame->renderer2D->PopScissor();

			if(iData==0)GuiScrollRect::Procedure(iFrame,iMsg,iData);
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
float GuiTreeViewNode::OnCalcWidth(const float& tExpandosEnd)
{
	return tExpandosEnd + GuiFont::GetDefaultFont()->MeasureText(this->label.c_str()).x;
}
float GuiTreeViewNode::OnCalcHeight()
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

void GuiTreeView::CalculateLayout()	
{
	this->ResetContent();
	vec4			tContentEdges=this->GetContentEdges();
	GuiTreeViewData tTreeViewData={0,tContentEdges,tContentEdges.y,0,0,false};
	this->ItemLayout(this->root,tTreeViewData);
	this->CalculateScrollbarsLayout();
}

void GuiTreeView::ItemLayout(GuiTreeViewNode* iItem,GuiTreeViewData& iTvdata)
{
	if(iItem)
	{
		float	tWidth=iItem->OnCalcWidth(iTvdata.level*20+(iItem->childs.size() ? 20 : 0));
		iTvdata.top+=iItem->OnCalcHeight();

		this->SetContent(tWidth,iTvdata.top-this->edges.y);

		if(this->GetFlag(iItem,GuiTreeViewNode::EXPANDED) && iItem->childs.size())
		{
			iTvdata.level++;

			for(std::list<GuiTreeViewNode*>::iterator i=iItem->childs.begin();i!=iItem->childs.end();i++)
				this->ItemLayout(*i,iTvdata);
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
					vec4			tContentEdges=this->GetContentEdges();
					GuiTreeViewData tTvid={0,tContentEdges,tContentEdges.y,0,iData,false};

					this->ItemRoll(this->root,iFrame,GuiRectMessages::ONHITTEST,tTvid);

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

				if(Contains(tExpandosEdges,tmd.mouse))
				{
					bool tExpanded=this->GetFlag(this->hovered,GuiTreeViewNode::EXPANDED);
					this->SetFlag(this->hovered,GuiTreeViewNode::EXPANDED,!tExpanded);
					this->CalculateLayout();
					iFrame->SetDraw(this);
				}
				else
					this->hovered->OnMouseUp(iFrame,*(MouseData*)iData);
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
			if(iData==0)GuiRect::Procedure(iFrame,iMsg,iData);

			vec4	tClippingEdges=this->GetClippingEdges();
			vec4	tContentEdges=this->GetContentEdges();

			GuiTreeViewData tTreeViewData={0,tContentEdges,tContentEdges.y,0,iData,false};

			float tOffsetx=this->GetScrollbarPosition(0);
			float tOffsety=this->GetScrollbarPosition(1);

			iFrame->renderer2D->PushScissor(tClippingEdges.x,tClippingEdges.y,tClippingEdges.z,tClippingEdges.w);
			iFrame->renderer2D->Translate(-tOffsetx,-tOffsety);
			
			this->ItemRoll(this->root,iFrame,GuiRectMessages::ONPAINT,tTreeViewData);

			iFrame->renderer2D->Translate(0,0);
			iFrame->renderer2D->PopScissor();
			
			if(iData==0)GuiScrollRect::Procedure(iFrame,iMsg,iData);
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

		iTvdata.top+=iItem->OnCalcHeight();
		iTvdata.idx++;

		if(this->GetFlag(iItem,GuiTreeViewNode::EXPANDED) && iItem->childs.size())
		{
			iTvdata.level++;

			for(std::list<GuiTreeViewNode*>::iterator i=iItem->childs.begin();i!=iItem->childs.end() && !iTvdata.skip;i++)
				this->ItemRoll(*i,iFrame,iMsg,iTvdata);
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

				vec4 tEdges(iTvdata.edges.x,iTvdata.top,iTvdata.edges.z,iTvdata.top+iItem->OnCalcHeight());

				if(Contains(tEdges,thtd.mouse))
				{
					//draw previous hitted node
					if(this->hovered)
					{
						this->SetFlag(this->hovered,GuiTreeViewNode::HIGHLIGHTED,false);
						iFrame->SetDraw(this,true,false,this->hovered);
					}

					//draw current hitted node
					this->hovered=(GuiTreeViewNode*)iItem;
					this->hoveredg=tEdges;
					this->hoverlvl=iTvdata.level;

					this->SetFlag(this->hovered,GuiTreeViewNode::HIGHLIGHTED,true);
					iFrame->SetDraw(this,true,false,this->hovered);

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

				float tContentBegin=iTvdata.edges.x+iTvdata.level*20;
				float tBottom=iTvdata.top+iItem->OnCalcHeight();

				unsigned int tColor=(iTvdata.idx%2) ? 0x555555 : 0x606060;

				if(tSelected)
					tColor=BlendColor(0x0000ff,tColor);
				if(tHighligh)
					tColor=BlendColor(0x00ffff,tColor);

				//base rect
				iFrame->renderer2D->DrawRectangle(iTvdata.edges.x,iTvdata.top,iTvdata.edges.z,tBottom,tColor);
				//expandos
				if(iItem->childs.size())
				{
					iFrame->renderer2D->DrawBitmap(this->GetFlag(iItem,GuiTreeViewNode::EXPANDED) ? iFrame->iconDown : iFrame->iconRight,tContentBegin,iTvdata.top,tContentBegin+20,tBottom);
					tContentBegin+=20;
				}

				vec4 tEdges(iTvdata.edges.x,iTvdata.top,iTvdata.edges.z,tBottom);

				iItem->OnPaint(iFrame,tEdges,tContentBegin);
			}
			break;
		}

	}
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
	/*if(iScript->runtime)
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

	/ *
	guiCompilerViewer->OnSize(tabContainer);
	guiCompilerViewer->OnActivate(tabContainer);

	if(false==noErrors)
		tabContainer->SetSelection(guiCompilerViewer);
	* /

	wprintf(L"%s on compiling %s\n",noErrors ? "OK" : "ERROR",iScript->file.c_str());

	return retVal;*/
return 0;
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

			//Ide::Instance()->mainAppWindow->CreateTabRects<GuiViewport>(tGuiViewport);

			Frame* tabContainerRunningUpdater=(Frame*)tGuiViewport[0]->GetRoot()->GetFrame();

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
void GuiStringProperty::OnPaint(Frame* iFrame,const PaintData& iData)
{
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
		//iFrame->renderer2D->DrawText(tValue,edges.x,edges.y,edges.z,edges.w,vec2(0,0.5f),vec2(0,0.5f),0xffffff,GuiFont::GetDefaultFont());
	}
	
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////GuiString///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiString::GuiString():align(0.0f,0.5f),spot(0.0f,0.5f),color(GuiString::COLOR_TEXT),font((GuiFont*)GuiFont::GetDefaultFont()){}

void GuiString::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		case ONPAINT:
			GuiRect::Procedure(iFrame,iMsg,iData);
			iFrame->renderer2D->DrawText(this->text,edges.x,edges.y,edges.z,edges.w,this->spot,this->align,this->color,this->font);
		break;
		default:
			GuiRect::Procedure(iFrame,iMsg,iData);
	}
}

const String&  GuiString::Text(){return this->text;}
void	GuiString::Text(const String& iText){this->text=iText;}

void  GuiString::Font(GuiFont* iFont){this->font=iFont;}
GuiFont*	GuiString::Font(){return this->font;}

void GuiString::Spot(float iSpotX,float iSpotY){this->spot.make(iSpotX,iSpotY);}
const vec2& GuiString::Spot(){return this->spot;}

void GuiString::Alignment(float iAlignX,float iAlignY){this->align.make(iAlignX,iAlignY);}
const vec2& GuiString::Alignment(){return this->align;}


void GuiString::OnPaint(Frame* iFrame,const PaintData& iData)
{
	
}

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
	StringEditor&	tStringEditor=*Ide::Instance()->stringeditor;

	bool			tRedraw=false;

	if(tStringEditor.Binded()==this && tStringEditor.Enabled())
	{
		unsigned int	tCaretOperation=tStringEditor.CARET_DONTCARE;
		void*			tCaretParameter=0;

		if(iData.key)
		{
			switch(iData.key)
			{
			case 0x08:/*VK_BACK*/tCaretOperation=tStringEditor.CARET_BACKSPACE; break;
			default:
				tCaretOperation=tStringEditor.CARET_ADD;
				tCaretParameter=(char*)iData.key;
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

void GuiTextBox::OnKeyDown(Frame* iFrame,const KeyData& iData)
{
	if(this==iFrame->GetFocus())
	{
		StringEditor& tStringEditor=*Ide::Instance()->stringeditor;

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
			tRedraw=this->ParseKeyInput(iFrame,iData);

		if(tRedraw)
			iFrame->SetDraw(this);
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
	this->Spot(0.5f,0.5f);
	this->Alignment(0.5f,0.5f);
}

void GuiButton::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		/*case ONMOUSEENTER:
		{
			GuiString::Procedure(iFrame,iMsg,iData);
			iFrame->renderer2D->DrawRectangle(edges.x,edges.y,edges.z,edges.w,this->color,this->font);
		}
		break;
		case ONMOUSEEXIT:
		{
			GuiString::Procedure(iFrame,iMsg,iData);
			iFrame->renderer2D->DrawText(this->text,edges.x,edges.y,edges.z,edges.w,this->spot,this->align,this->color,this->font);
		}
		break;*/
		case ONMOUSEUP:
		{
			GuiString::Procedure(iFrame,iMsg,iData);
			ControlData cd={this,GuiRectMessages::ONMOUSEUP,0};
			iFrame->Message(this->parent,GuiRectMessages::ONCONTROLEVENT,&cd);
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

	iFrame->isModal ? iFrame->windowData->Enable(false) : Ide::Instance()->mainframe->frame->windowData->Enable(false);
	iFrame->SetDraw();

	String tDirectory=Ide::Instance()->subsystem->DirectoryChooser(L"",L"");

	if(tDirectory.size())
		tGuiPropertyPath->path.Text(tDirectory);

	iFrame->isModal ? iFrame->windowData->Enable(true) : Ide::Instance()->mainframe->frame->windowData->Enable(true);
	iFrame->SetDraw();

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

void GuiSlider::OnPaint(Frame* iFrame,const PaintData& iData)
{
	//GuiRect::OnPaint(iTab,iData);

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

void GuiSlider::OnMouseMove(Frame* iFrame,const MsgData& iData)
{
	//GuiRect::OnMouseMove(iTab,iData);

	/*if(this->IsPressing())
		this->SetPosition(iTab,iMsg.mouse);*/
}

void GuiSlider::OnMouseDown(Frame* iFrame,const MsgData& iData)
{
	//GuiRect::OnMouseDown(iTab,iData);

	/*if(iMsg.button==1)
		this->SetPosition(iTab,iMsg.mouse);*/
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
	/*this->play.Text(L"Play");
	this->stop.Text(L"Stop");

	this->slider.Activate(true);
	this->stop.Activate(true);
	this->play.Activate(true);

	this->Append(&this->slider);
	this->Append(&this->play);
	this->Append(&this->stop);*/
}

void GuiAnimationController::OnSize(Frame* iFrame,const MsgData& iData)
{
	/*GuiRect::OnSize(iTab,iData);

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
	this->play.Edges(tPlayEdges);*/
}

void GuiAnimationController::OnControlEvent(Frame* iFrame,const MsgData& iData)
{
	/*GuiRect::OnControlEvent(iTab,iData);

	if(iMsg.sender==&this->slider)
	{
		if(iMsg.senderFunc==ONMOUSEMOVE)
		{
			float value=this->slider.Value();

			if(value!=this->slider.Value() && this->slider.IsPressing())
			{
				this->animationController.SetFrame(this->slider.Value());
				iFrame->SetDraw(this);
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
	}*/

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

GuiViewport::GuiViewport():
	renderBuffer(0),
	needsPicking(0),
	pickedEntity(0),
	renderInstance(0),
	renderFps(60),
	renderBitmap(0)
{
	GlobalViewports().push_back(this);

	this->projection=this->projection.perspective(90,16/9,1,1000);
	this->view.move(vec3(100,100,100));
	this->view.lookat(vec3(0,0,0));
}

void GuiViewport::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		case ONMOUSEMOVE:
		{
			MouseData& tmd=*(MouseData*)iData;

			if(InputManager::keyboardInput.IsPressed(0x01/*VK_LBUTTON*/))
			{
				float dX=(tmd.mouse.x-this->mouseold.x);
				float dY=(tmd.mouse.y-this->mouseold.y);

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

			this->mouseold=tmd.mouse;

			iFrame->SetDraw(this);
		}
		break;
		case ONPAINT:
		{
			#if ENABLE_RENDERER
			/*if(Timer::GetInstance()->GetCurrent()-tViewport->lastFrameTime>(1000.0f/tViewport->renderFps))
			{
				tViewport->lastFrameTime=Ide::Instance()->timer->GetCurrent();
				tViewport->Render(iFrame);
			}
			else*/
				this->DrawBuffer(iFrame,this->edges);
			#endif
		}
		break;
		default:
			GuiRect::Procedure(iFrame,iMsg,iData);
	}
}

mat4& GuiViewport::Projection(){return this->projection;}
mat4& GuiViewport::View(){return this->view;}
mat4& GuiViewport::Model(){return this->model;}

EditorEntity* GuiViewport::GetHoveredEntity(){return 0;}
EditorEntity* GuiViewport::GetPickedEntity(){return this->pickedEntity;}

void GuiViewport::SetFps(unsigned int iFps){this->renderFps=iFps;}
unsigned int GuiViewport::GetFps(){return this->renderFps;}

std::list<GuiViewport*>& GuiViewport::GetPool()
{
	return GlobalViewports();
}

GuiViewport* GuiViewport::Instance()
{
	return new GuiViewport();
}

/*
void GuiViewport::OnPaint(Frame*)
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
}*/

void GuiViewport::OnSize(Frame*,const vec4& iEdges)
{
	this->edges=iEdges;
}

void GuiViewport::OnMouseWheel(Frame* iFrame,const MsgData& iData)
{
	/*GuiRect::OnMouseWheel(iTab,iData);

	float factor=*(float*)iMsg.data;

	this->view*=mat4().translate(0,0,factor*10);*/
}

void GuiViewport::OnMouseUp(Frame* iFrame,const MsgData& iData)
{
	/*if(this->IsHovering() && this->pickedEntity)
		Ide::Instance()->mainAppWindow->mainContainer->Broadcast(GuiRect::Funcs::ONENTITYSELECTED,Msg(this->pickedEntity));

	GuiRect::OnMouseUp(iTab,iData);*/
}

void GuiViewport::OnMouseMove(Frame* iFrame,const MsgData& iData)
{
	/*GuiRect::OnMouseMove(iTab,iData);

	if(this->IsHovering())
	{
		if(InputManager::keyboardInput.IsPressed(0x01/ *VK_LBUTTON* /))
		{
			float dX=(iMsg.mouse.x-this->mouseold.x);
			float dY=(iMsg.mouse.y-this->mouseold.y);

			if(InputManager::keyboardInput.IsPressed(0x011/ *VK_CONTROL* /))
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
	}*/
}


void GuiViewport::OnActivate(Frame* iFrame)
{
#if ENABLE_RENDERER
		//this->renderInstance=iFrame->SetDraw(this,false);
#endif
}
void GuiViewport::OnDeactivate(Frame* iFrame)
{
#if ENABLE_RENDERER
#endif
}



///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////GuiScrollRect////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

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
				if(this->active[SCROLLH] && Contains(this->sedges[SCROLLH][0],thtd.mouse))
				{
					if(Contains(this->sedges[SCROLLH][1],thtd.mouse))this->scrollerhit=1;
					else if(Contains(this->sedges[SCROLLH][2],thtd.mouse))this->scrollerhit=2;
					else if(Contains(this->sedges[SCROLLH][3],thtd.mouse))this->scrollerhit=3;
				}
				else if(this->active[SCROLLV] && Contains(this->sedges[SCROLLV][0],thtd.mouse))
				{
					if(Contains(this->sedges[SCROLLV][1],thtd.mouse))this->scrollerhit=4;
					else if(Contains(this->sedges[SCROLLV][2],thtd.mouse))this->scrollerhit=5;
					else if(Contains(this->sedges[SCROLLV][3],thtd.mouse))this->scrollerhit=6;
				}
				else
				{
					this->scrollerhit=0;

					thtd.off.x+=this->positions[SCROLLH]*this->content[0];
					thtd.off.y+=this->positions[SCROLLV]*this->content[1];

					thtd.mouse.x+=thtd.off.x;
					thtd.mouse.y+=thtd.off.y;
				}
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
						float tAmount=this->positions[tBarType] - tRatio/SCROLLTIP;

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
						float tAmount=this->positions[tBarType] + tRatio/SCROLLTIP;

						this->SetScrollbarPosition(tBarType,tAmount);
					}

					iFrame->SetDraw(this);
				}
			}
		}
		break;
		case GuiRectMessages::ONMOUSEUP:
		{
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
	return 0;
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
	float right=this->edges.z - (this->active[1] ? SCROLLTIP : 0);
	float bottom=this->edges.w - (this->active[0] ? SCROLLTIP : 0);

	return vec4(this->edges.x,this->edges.y,right,bottom);
}

vec4 GuiScrollRect::GetContentEdges()
{
	return vec4(this->edges.x,this->edges.y,this->edges.x+this->maxes[0],this->edges.y+this->maxes[1]);
}

void GuiScrollRect::CalculateScrollbarsLayout()
{
	float tRight=this->edges.z;
	float tBottom=this->edges.w;

	bool& hActive=this->active[SCROLLH];
	bool& vActive=this->active[SCROLLV];

	hActive=this->content[0]>(tRight-this->edges.x);
	tBottom -= hActive ? SCROLLTIP : 0;
	vActive=this->content[1]>(tBottom-this->edges.y);
	tRight -= vActive ? SCROLLTIP : 0;
	hActive=this->content[0]>(tRight-this->edges.x);

	this->maxes[0]=std::max<float>(tRight-this->edges.x,this->content[0]);
	this->maxes[1]=std::max<float>(tBottom-this->edges.y,this->content[1]);

	if(hActive)
	{
		const vec4& e=this->sedges[SCROLLH][0];

		this->sedges[SCROLLH][0].make(edges.x,edges.w-SCROLLTIP,edges.z - (vActive ? SCROLLTIP : 0),edges.w);
		this->sedges[SCROLLH][1].make(e.x,e.y,e.x+SCROLLTIP,e.w);
		this->sedges[SCROLLH][3].make(e.z-SCROLLTIP,e.y,e.z,e.w);
		this->sedges[SCROLLH][2].make(this->sedges[SCROLLH][1].z,e.y,this->sedges[SCROLLH][3].x,e.w);
		this->clength[SCROLLH]=(this->sedges[SCROLLH][2].z-this->sedges[SCROLLH][2].x);
		this->ratios[SCROLLH]=(this->sedges[SCROLLH][0].z-this->sedges[SCROLLH][0].x)/this->maxes[SCROLLH];
	}
	else
		 this->sedges[SCROLLH][0].make(0,0,0,0);

	if(vActive)
	{
		const vec4& e=this->sedges[SCROLLV][0];

		this->sedges[SCROLLV][0].make(edges.z-SCROLLTIP,edges.y,edges.z,edges.w - (hActive ? SCROLLTIP : 0));
		this->sedges[SCROLLV][1].make(e.x,e.y,e.z,e.y+SCROLLTIP);
		this->sedges[SCROLLV][3].make(e.x,e.w-SCROLLTIP,e.z,e.w);
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

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////////GuiPanel/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiPanel::GuiPanel()
{
	//this->SetName(L"GuiPanel");
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////GuiSceneViewer/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiScene::GuiScene():entity(0){}
GuiScene* GuiScene::Instance()
{
	GLOBALGETTERFUNCASSINGLETON(GlobalGuiSceneViewerInstance,GuiScene);
}

EditorEntity*	GuiScene::Entity(){return this->entity;}
void			GuiScene::Entity(EditorEntity* iEntity){this->entity=iEntity;} 

void GuiScene::OnEntitiesChange(Frame* iFrame,const MsgData& iData)
{
	/*EditorEntity* newEntity=(EditorEntity*)iMsg.data;

	if(newEntity)
	{
		if(!this->entity)
			this->Append(newEntity->sceneLabel);

		this->entity=newEntity;
		
		iFrame->BroadcastTo(this,ONSIZE);
		iFrame->SetDraw(this);
	}*/
}

/*
GuiSceneViewer::SceneLabel::SceneLabel()
{
	this->SetFlag(FLAGS,DRAWCHECK,true);
}

void			GuiSceneViewer::SceneLabel::Entity(EditorEntity* iEntity){this->entity=iEntity;}
EditorEntity*	GuiSceneViewer::SceneLabel::Entity(){return this->entity;}

void GuiSceneViewer::SceneLabel::OnMouseUp(Tab* iTab,const MsgData& iData)
{
	Item::OnMouseUp(iTab,iData);

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
				iFrame->SetDraw(this->TreeView());
		}
		break;
	case 1:
		{
			if(!this->ExpandosContains(iMsg.mouse))
			{
				this->SetFlag(FLAGS,CHECKED,!this->GetFlag(FLAGS,CHECKED));

				for(std::list<GuiEntityViewer*>::const_iterator it=GuiEntityViewer::Pool().begin();it!=GuiEntityViewer::Pool().end();it++)
					(*it)->OnEntitySelected((Tab*)(*it)->GetRoot()->CreateTab(),Msg(this->entity));
			}
		}
		break;
	}
}*/

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

GuiEntity::GuiEntity():entity(0){}

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

void			GuiEntity::Entity(EditorEntity* iEntity){this->entity=iEntity;}
EditorEntity*	GuiEntity::Entity(){return this->entity;}

void GuiEntity::OnEntitySelected(Frame* iFrame,const MsgData& iData)
{
	/*EditorEntity* tReceivedEntity=(EditorEntity*)iMsg.data;

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

	iFrame->SetDraw(this);*/
}

void GuiEntity::OnExpandos(Frame* iFrame,const MsgData& iData)
{
	//this->OnSize(iTab,iData);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////GuiConsoleViewer////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
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
			this->splitterLeftActive=false;
			this->splitterRightActive=false;
		}
		break;
		case ONMOUSEMOVE:
		{
			MouseData& md=*(MouseData*)iData;

			if(md.mouse.y>this->edges.y)
				iFrame->SetCursor(1);

			if(md.button==1)
			{
				if(this->splitterLeftActive)
					this->splitterLeft=md.mouse.x;
				else if(this->splitterRightActive)
					this->splitterRight=md.mouse.x;

				iFrame->BroadcastTo(this,GuiRectMessages::ONSIZE,0);

				iFrame->SetDraw(this);
			}
		}
		break;
		case ONSIZE:
		{
			GuiRect::Procedure(iFrame,iMsg,iData);

			const vec4* iSize=(const vec4*)iData;

			float tOldWidth=this->edges.z-this->edges.x;
			float tNewWidth=iSize ? (iSize->z-iSize->x) : tOldWidth;
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

			if(iSize)
			this->edges=*iSize;

			this->dirView.edges=this->edges;
			this->fileView.edges=this->edges;
			this->resView.edges=this->edges;

			this->dirView.edges.z=this->splitterLeft;
			this->fileView.edges.x=this->splitterLeft+4;
			this->fileView.edges.z=this->splitterRight;
			this->resView.edges.x=this->splitterRight+4;
		}
		break;
		case ONACTIVATE:
			Ide::Instance()->ScanDir(this->projectDirectory->fileName,this->projectDirectory);
			this->dirView.InsertRoot(this->projectDirectory->dirLabel);
			this->fileView.InsertItems(this->projectDirectory->fileLabels);
		break;
		case ONPAINT:
			iFrame->renderer2D->DrawRectangle(this->edges.x,this->edges.y,this->edges.z,this->edges.w,0x202020);
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

void GuiProject::RefreshAll()
{
}

void GuiProject::OnMouseMove(Frame* iFrame,const MsgData& iData)
{
	/*GuiRect::OnMouseMove(iTab,iData);

	iTab->SetCursor(1);

	if(this->IsPressing())
	{
		this->OnSize(iTab,iData);

		if(this->splitterLeftActive)
		{
			this->splitterLeft=iMsg.mouse.x;
			this->dirViewer.OnSize(iTab,iData);
		}

		this->fileViewer.OnSize(iTab,iData);

		if(this->splitterRightActive)
		{
			this->splitterRight=iMsg.mouse.x;
			this->resViewer.OnSize(iTab,iData);
		}

		iFrame->SetDraw(this);
	}*/
}


void GuiProject::OnMouseDown(Frame* iFrame,const MsgData& iData)
{
	/*GuiRect::OnMouseDown(iTab,iData);

	iTab->SetCursor(1);

	if(iMsg.mouse.x>=this->splitterLeft && iMsg.mouse.x<=this->splitterLeft+SPLITTER_SIZE)
		this->splitterLeftActive=true;
	else if(iMsg.mouse.x>=this->splitterRight && iMsg.mouse.x<=this->splitterRight+SPLITTER_SIZE)
		this->splitterRightActive=true;*/
}

void GuiProject::OnMouseUp(Frame* iFrame,const MsgData& iData)
{
	/*GuiRect::OnMouseUp(iTab,iData);

	if(this->splitterLeftActive || this->splitterRightActive)
	{
		this->splitterLeftActive=false;
		this->splitterRightActive=false;
	}*/
}


void GuiProject::OnReparent(Frame* iFrame,const MsgData& iData)
{
}


void GuiProject::Delete(Frame* iFrame,ResourceNode* iNode)
{

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


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////DrawInstance/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


DrawInstance::DrawInstance(GuiRect* iRect,bool iRemove,bool iSkip,void* iParam):rect(iRect),remove(iRemove),skip(iSkip),param(iParam){}
DrawInstance::~DrawInstance(){}

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////GuiPaper///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

void GuiScript::DrawBreakpoints(Frame* tabContainer)
{
	/*float tFontHeight=GuiFont::GetDefaultFont()->GetHeight();

	std::vector<Debugger::Breakpoint>& breakpoints=Ide::Instance()->debugger->breakpointSet;

	for(size_t i=0;i<breakpoints.size();i++)
	{
		if(breakpoints[i].script==this->script)
		{
			unsigned int tLineInsertion=this->Edges().y + (breakpoints[i].line - 1) * tFontHeight;

			unsigned int tBreakColor = breakpoints[i].breaked ? 0xff0000 : 0xffff00 ;

			tabContainer->renderer2D->DrawRectangle(this->Edges().x + 1,tLineInsertion + 1,this->Edges().x/ *+this->editor->margins.x -1* /,tLineInsertion + tFontHeight - 1,tBreakColor,true);
		}
	}*/
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////GuiScriptViewer/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
std::list<GuiScript*>& GuiScript::GetInstances()
{
	return GlobalScriptViewers();
}


GuiScript::GuiScript():
	script(0),
	lineNumbers(true)
{
	/*this->GetInstances().push_back(this);

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
*/

}

GuiScript::~GuiScript()
{
	this->GetInstances().remove(this);
}

void GuiScript::Open(Script* iScript)
{
	this->script=(EditorScript*)iScript;

	this->editor->Text(this->script->LoadScript());

	this->script->scriptViewer=this;
}

bool GuiScript::Save()
{
	if(this->script)
		this->script->SaveScript(this->editor->Text());

	return false;
}


bool GuiScript::Compile()
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



void GuiScript::OnKeyDown(Frame* iFrame,const MsgData& iData)
{
	/*if(this->script)
	{
		if(InputManager::keyboardInput.IsPressed(0x11/ *VK_CONTROL* /) && !InputManager::keyboardInput.IsPressed(0x12/ *VK_ALT* /))
		{
			if(InputManager::keyboardInput.IsPressed('S'))
				this->Save();
		}
	}

	GuiRect::OnKeyDown(iTab,iData);*/
}

void GuiScript::OnKeyUp(Frame* iFrame,const MsgData& iData)
{
	//GuiRect::OnKeyUp(iTab,iData);
}

void GuiScript::OnLMouseDown(Frame* iFrame,const MsgData& iData)
{
	//GuiRect::OnMouseDown(iTab,iData);

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

				iFrame->SetDraw(this);

				break;
			}
		}
	}*/

	iFrame->SetFocus(this->editor);
}

void GuiScript::OnDeactivate(Frame* iFrame,const MsgData& iData)
{
	iFrame->SetFocus(0);

	//GuiRect::OnDeactivate(iTab,iData);
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

void GuiScript::OnSize(Frame* iFrame,const MsgData& iData)
{
	/*GuiRect::OnSize(iTab,iData);

	this->lineCount=this->CountScriptLines()-1;*/
}


int GuiScript::CountScriptLines()
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

std::list<GuiCompiler*>& GuiCompiler::InnerInstances()
{
	return GlobalGuiCompilerViewerInstance();
}

GuiCompiler::GuiCompiler()
{
	GlobalGuiCompilerViewerInstance().push_back(this);
}

GuiCompiler::~GuiCompiler()
{
	GlobalGuiCompilerViewerInstance().remove(this);
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
	/*bool bReturnValue=true;

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
				/ *tCompilerMessageRow->fixed.w=MESSAHE_ROW_HEIGHT;
				tCompilerMessageRow->alignRect.y=-1;
				tCompilerMessageRow->alignText.make(-1,0.5f);* /
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

	return bReturnValue;*/
	return false;
}


void GuiCompiler::OnSize(Frame* iFrame,const MsgData& iData)
{
	/*this->vScrollbar.SetScrollerRatio(this->contentHeight,this->rect.w);

	if(this->childs.size()==2)
		//we don't need the GuiScrollRect::width cause the newly GuiRect::offset
		this->vScrollbar.IsVisible() ? this->childs[0]->offsets.z=-GuiScrollBar::SCROLLBAR_TICK : this->childs[0]->offsets.z=0;*/

	//GuiRect::OnSize(iTab,iData);

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
	sceneLabel(this)
	/*spName(&this->name,GuiStringProperty::STRING),
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
	pcAABB(L"AABB")*/
{}

EditorEntity* EditorEntity::Parent(){return (EditorEntity*)this->Entity::Parent();}

const std::list<EditorEntity*>& EditorEntity::Childs(){return (std::list<EditorEntity*>&)this->Entity::Childs();}

void EditorEntity::SetName(String iName)
{
	this->Entity::name=iName;
	this->sceneLabel.SetLabel(this->name);
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

EditorEntity::~EditorEntity()
{
}

void EditorEntity::DestroyChilds()
{
}

void EditorEntity::OnPropertiesCreate()
{
	/*this->sceneLabel.Entity(this);
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
	pcAABB.Append(pVolume);*/
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

EditorMesh::EditorMesh()/*:
	spControlpoints(&this->ncontrolpoints,GuiStringProperty::INT),
	spNormals(&this->nnormals,GuiStringProperty::INT),
	spPolygons(&this->npolygons,GuiStringProperty::INT),
	spTextCoord(&this->ntexcoord,GuiStringProperty::INT),
	spVertexIndices(&this->nvertexindices,GuiStringProperty::INT),
	pControlpoints(L"Controlpoints",spControlpoints),
	pNormals(L"Normals",spNormals),
	pPolygons(L"Polygons",spPolygons),
	pTextCoord(L"Texcoord",spTextCoord),
	pVertexIndices(L"Vertexindices",spVertexIndices)*/
{}

void EditorMesh::OnPropertiesCreate()
{
	/*this->PropertyContainer().SetName(L"Mesh");

	this->PropertyContainer().Append(pControlpoints);
	this->PropertyContainer().Append(pNormals);
	this->PropertyContainer().Append(pPolygons);
	this->PropertyContainer().Append(pTextCoord);
	this->PropertyContainer().Append(pVertexIndices);

	this->Entity()->PropertyContainer().Append(this->PropertyContainer());*/
}

void EditorMesh::OnPropertiesUpdate(Frame* tab)
{
}

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////EditorSkin/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

EditorSkin::EditorSkin()/*:
	spClusters(&this->nclusters,GuiStringProperty::INT),
	spTextures(&this->ntextures,GuiStringProperty::INT),
	pClusters(L"Clusters",spClusters),
	pTextures(L"Textures",spTextures)*/
{}

void EditorSkin::OnPropertiesCreate()
{
	/*this->PropertyContainer().SetName(L"Skin");

	this->PropertyContainer().Append(pClusters);
	this->PropertyContainer().Append(pTextures);

	this->Entity()->PropertyContainer().Append(this->PropertyContainer());*/
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



EditorAnimation::EditorAnimation()/*:
	spIsBone(this,GuiStringProperty::ISBONECOMPONENT),
	spDuration(&this->start,GuiStringProperty::FLOAT2MINUSFLOAT1,&this->end),
	spBegin(&this->start,GuiStringProperty::FLOAT),
	spEnd(&this->end,GuiStringProperty::FLOAT),
	pIsBone(L"IsBone",spIsBone),
	pDuration(L"Duration",spDuration),
	pBegin(L"Begin",spBegin),
	pEnd(L"End",spEnd)*/
{}

void EditorAnimation::OnPropertiesCreate()
{
	/*this->PropertyContainer().SetName(L"Animation");

	this->PropertyContainer().Append(pIsBone);
	this->PropertyContainer().Append(pDuration);
	this->PropertyContainer().Append(pBegin);
	this->PropertyContainer().Append(pEnd);

	this->Entity()->PropertyContainer().Append(this->PropertyContainer());*/
}
void EditorAnimation::OnPropertiesUpdate(Frame* tab)
{
}

EditorAnimationController::EditorAnimationController()/*:
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
	pPlayer(L"Player",acpGuiAnimationController,50)*/
{}

EditorAnimationController::~EditorAnimationController(){}

void EditorAnimationController::OnPropertiesCreate()
{
	/*this->PropertyContainer().SetName(L"AnimationController");

	this->PropertyContainer().Append(pNumNodes);
	this->PropertyContainer().Append(pBegin);
	this->PropertyContainer().Append(pEnd);
	this->PropertyContainer().Append(pDuration);
	this->PropertyContainer().Append(pPlayer);
	this->PropertyContainer().Append(pVelocity);
	

	this->Entity()->PropertyContainer().Append(this->PropertyContainer());*/
}


void EditorAnimationController::OnPropertiesUpdate(Frame* tab)
{
	/*if(this->PropertyContainer().Expanded() && this->oldCursor!=this->cursor)
	{
		tab->SetDraw(&this->acpGuiAnimationController.slider);

		this->oldCursor=this->cursor;
	}*/
}

EditorLine::EditorLine()/*:
	spNumSegments(&this->points,GuiStringProperty::VEC3LISTSIZE),
	pNumSegments(L"NumSegments",spNumSegments)*/
{}

void EditorLine::OnPropertiesCreate()
{
	/*this->PropertyContainer().SetName(L"Line");

	this->PropertyContainer().Append(pNumSegments);

	for(std::list<vec3>::iterator i=this->points.begin();i!=this->points.end();i++)
	{
		GuiListBox::Item* tPoint=new GuiListBox::Item(*i,GuiStringProperty::VEC3);

		this->pointItems.push_back(tPoint);
		this->pointListBox.Append(tPoint);
	}

	this->Entity()->PropertyContainer().Append(this->PropertyContainer());*/
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
	/*this->PropertyContainer().SetName(L"Bone");
	this->Entity()->PropertyContainer().Append(this->PropertyContainer());*/
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

void editScriptEditorCallback(void* iData)
{
	/*EditorScript* editorScript=(EditorScript*)iData;

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
	}*/
}

void compileScriptEditorCallback(void* iData)
{
	/*EditorScript* editorScript=(EditorScript*)iData;

    Ide::Instance()->compiler->Compile(editorScript);*/
}



EditorScript::EditorScript():scriptViewer(0)
{

};

void EditorScript::OnPropertiesCreate()
{
	/*this->PropertyContainer().SetName(L"Bone");
	this->Entity()->PropertyContainer().Append(this->PropertyContainer());*/

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
void EditorScript::OnPropertiesUpdate(Frame* tab)
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
	/*if(this->file.empty())
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
		DEBUG_BREAK();*/
}

void EditorCamera::OnPropertiesCreate()
{
	/*this->PropertyContainer().SetName(L"Camera");
	this->Entity()->PropertyContainer().Append(this->PropertyContainer());*/
}
void EditorCamera::OnPropertiesUpdate(Frame* tab)
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
	//this->listBoxItem.button.Color(0x00ff00);
}

void PluginSystem::Plugin::Unload()
{
	this->loaded=false;
	//this->listBoxItem.button.Color(0xff0000);
}

PluginSystem::PluginSystem():pluginsTab(this){}
PluginSystem::~PluginSystem(){}	

PluginSystem::GuiPluginTab::GuiPluginTab(PluginSystem* iPluginSystem):pluginSystem(iPluginSystem)
{
	this->exitButton.Text(L"Exit");
	this->Append(&this->exitButton);
}

void PluginSystem::GuiPluginTab::Procedure(Frame* iFrame,GuiRectMessages iMsg,void* iData)
{
	switch(iMsg)
	{
		case ONSIZE:
		{
			GuiListBox::Procedure(iFrame,iMsg,iData);

			const vec4* tSize=(const vec4*)iData;

			if(tSize)
			{
				this->edges=*tSize;
				this->exitButton.edges.make(this->edges.z-110,this->edges.w-25,this->edges.z-10,this->edges.w-5);
			}
		}
		break;
		case ONCONTROLEVENT:
		{
			ControlData* cd=(ControlData*)iData;

			if(cd && cd->control==&this->exitButton && cd->msg==ONMOUSEUP)
			{
				Ide::Instance()->GetMainFrame()->DestroyFrame(this->GetRoot()->GetFrame());
				Ide::Instance()->GetMainFrame()->Enable(true);
			}
		}
		break;
		default:
			GuiListBox::Procedure(iFrame,iMsg,iData);
	}
}

void PluginSystem::GuiPluginTab::OnControlEvent(Frame* iFrame,const MsgData& iData)
{
	
}

void PluginSystem::ShowGui()
{
	this->ScanPluginsDirectory();

	MainFrame*	tMainFrame=Ide::Instance()->GetMainFrame();
	Frame*		tFrame=0;

	tMainFrame->Enable(false);

	vec2 tIdeFrameSize=tMainFrame->GetFrame()->windowData->Size();
	vec2 tTabSize(500,300);
	vec2 tTabPos=tMainFrame->GetFrame()->windowData->Pos();

	tTabPos.x+=tIdeFrameSize.x/2.0f-tTabSize.x/2.0f;
	tTabPos.y+=tIdeFrameSize.y/2.0f-tTabSize.y/2.0f;

	tFrame=tMainFrame->CreateFrame(tTabPos.x,tTabPos.y,tTabSize.x,tTabSize.y,0,true);

	this->viewer.edges.make(tTabPos.x,tTabPos.y,tTabPos.x+tTabSize.x,tTabPos.y+tTabSize.y);

	tFrame->AddViewer(&this->viewer);

	for(std::list<Plugin*>::const_iterator i=this->plugins.begin();i!=this->plugins.end();i++)
	{
		Plugin* tPlugin=(*i);

		this->pluginsTab.InsertItem(tPlugin->listBoxItem);
	}

	this->viewer.AddTab(&this->pluginsTab,L"Plugins");
	
	tFrame->OnSize(tFrame->Size().x,tFrame->Size().y);
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
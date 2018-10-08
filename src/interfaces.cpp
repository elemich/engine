#include "interfaces.h"

#include "imgpng.h"
#include "imgjpg.h"
#include "imgtga.h"

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////////globals///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

Ide*										globalIdeInstance=0;
GuiFont*									globalDefaultFont=0;
std::vector<GuiFont*>						globalFontPool;
GuiSceneViewer*								globalGuiSceneViewer=0;
GuiProjectViewer*							globalGuiProjectViewer=0;
GuiCompilerViewer*							globalGuiCompilerViewer=0;
GuiConsoleViewer*							globalGuiConsoleViewer=0;
std::vector<GuiScriptViewer*>				globalScriptViewers;
std::list<GuiViewport*>						globalViewports;

GLOBALGETTERFUNC(GlobalGuiProjectViewerInstance,globalGuiProjectViewer,GuiProjectViewer*&);
GLOBALGETTERFUNC(GlobalGuiSceneViewerInstance,globalGuiSceneViewer,GuiSceneViewer*&);
GLOBALGETTERFUNC(GlobalGuiCompilerViewerInstance,globalGuiCompilerViewer,GuiCompilerViewer*&);
GLOBALGETTERFUNC(GlobalGuiConsoleViewerInstance,globalGuiConsoleViewer,GuiConsoleViewer*&);
GLOBALGETTERFUNC(GlobalDefaultFontInstance,globalDefaultFont,GuiFont*);
GLOBALGETTERFUNC(GlobalFontPoolInstance,globalFontPool,std::vector<GuiFont*>&);
GLOBALGETTERFUNC(GlobalIdeInstance,globalIdeInstance,Ide*&);
GLOBALGETTERFUNC(GlobalScriptViewers,globalScriptViewers,std::vector<GuiScriptViewer*>&);
GLOBALGETTERFUNC(GlobalViewports,globalViewports,std::list<GuiViewport*>&);

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////SerialiserHelpers/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
/*

namespace SerializerHelper
{
	void LoadSave(Serializer::Mode iMode,int iSerializerCode,void*& iObject,FILE* iStream)
	{
		switch(iSerializerCode)
		{
		case Serializer::Root:
		{
			switch(iMode)
			{
				case Serializer::Mode::MODE_LOAD:
				{

				}
				break:
				case Serializer::Mode::MODE_SAVE:
				{

				}
				break:
				default: 
					DEBUG_BREAK();
			}
		}
		break;
		case Serializer::Skeleton:
		{
			switch(iMode)
			{
			case Serializer::Mode::MODE_LOAD:
				{

				}
				break:
			case Serializer::Mode::MODE_SAVE:
				{

				}
				break:
			default: 
				DEBUG_BREAK();
			}
		}
		break;
		case Serializer::Animation:
		{
			switch(iMode)
			{
			case Serializer::Mode::MODE_LOAD:
				{

				}
				break:
			case Serializer::Mode::MODE_SAVE:
				{

				}
				break:
			default: 
				DEBUG_BREAK();
			}
		}
		break;
		case Serializer::Gizmo:
		{
			switch(iMode)
			{
			case Serializer::Mode::MODE_LOAD:
				{

				}
				break:
			case Serializer::Mode::MODE_SAVE:
				{

				}
				break:
			default: 
				DEBUG_BREAK();
			}
		}
		break;
		case Serializer::AnimationController:
		{
			switch(iMode)
			{
				case Serializer::Mode::MODE_LOAD:
				{

				}
				break:
				case Serializer::Mode::MODE_SAVE:
				{

				}
				break:
				default: 
					DEBUG_BREAK();
			}
		}
		break;
		case Serializer::Bone:
		{

		}
		break;
		case Serializer::Light:
		{

		}
		break;
		case Serializer::Mesh:
		{

		}
		break;
		case Serializer::Script:
		{

		}
		break;
		case Serializer::Camera:
		{

		}
		break;
		case Serializer::Project:
		{

		}
		break;
		case Serializer::Scene:
		{

		}
		break;
		case Serializer::Entity:
		{

		}
		break;
		case Serializer::Material:
		{

		}
		break;
		case Serializer::Texture:
		{

		}
		break;
		case Serializer::Line:
		{

		}
		break;
		case Serializer::Unknown:
		{
			DEBUG_BREAK();
		}
		break;
		}
	}
}
*/

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////ResourceNode/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

ResourceNodeDir* rootProjectDirectory=0;

ResourceNode::ResourceNode():
	parent(0),
	level(0),
	isDir(0),
	selectedLeft(0),
	selectedRight(0)
{
}

ResourceNode::~ResourceNode()
{
	this->parent=0;
	this->fileName.clear();
	this->selectedLeft=false;
	this->selectedRight=false;
	this->level=0;
	this->isDir=false;
}

ResourceNodeDir::ResourceNodeDir():
expanded(0)
{
}

String ResourceNodeDir::BuildPath()
{
	if(!this->parent)
		return this->fileName;

	String tReturnPath=L"\\" + this->fileName;

	ResourceNodeDir* t=this;

	while(t->parent)
	{
		tReturnPath.insert(0,L"\\" + t->parent->fileName);
		t=(ResourceNodeDir*)t->parent;
	}
	 
	return tReturnPath;
}

ResourceNodeDir::~ResourceNodeDir()
{
	this->directoryViewerRow.SetParent(0);

	for(std::list<ResourceNode*>::iterator tFile=this->files.begin();tFile!=this->files.end();tFile++)
		SAFEDELETE(*tFile);

	for(std::list<ResourceNodeDir*>::iterator tDir=this->dirs.begin();tDir!=this->dirs.end();tDir++)
		SAFEDELETE(*tDir);

	this->files.clear();
	this->dirs.clear();
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

	FilePath tResourcePath=gFindResource(tRootTrailingSlashes,rootProjectDirectory->fileName,rootProjectDirectory,iResourceName);

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
	globalIdeInstance=this->Instance();
}

Ide::~Ide(){}

String Ide::GetSceneExtension()
{
	return L".engineScene";
}
String Ide::GetEntityExtension()
{
	return L".engineEntity";
}

Ide* Ide::GetInstance()
{
	return GlobalIdeInstance();
}

bool Ide::IsInstanced()
{
	return GlobalIdeInstance() ? true : false;
}

Tab*	Ide::CreatePopup(Container* iParent,float ix,float iy,float iw,float ih)
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

		GuiEvent tDeactivate(this->popup);
		this->popup->rects.OnDeactivate(tDeactivate);

		for(int i=0;i<this->popup->rects.childs.size();i++)
			this->popup->rects.childs[i]->SetParent(0);

		this->popup->Destroy();
		this->popup=0;
	}
}

Tab*	Ide::GetPopup()
{
	return this->popup;
}


GuiProjectViewer*	Ide::GetProjectViewer()
{
	return GuiProjectViewer::GetInstance();
}
GuiSceneViewer*		Ide::GetSceneViewer()
{
	return GuiSceneViewer::GetInstance();
}
GuiCompilerViewer*	Ide::GetCompilerViewer()
{
	return GuiCompilerViewer::GetInstance();
}
GuiConsoleViewer*	Ide::GetConsoleViewer()
{
	return GuiConsoleViewer::GetInstance();
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////EditorWindowContainer///////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

Container::Container()
{
	resizeDiffHeight=0;
	resizeDiffWidth=0;
	resizeEnumType=-1;
	resizeCheckWidth=0;
	resizeCheckHeight=0;
}

Container::~Container(){}

void Container::BroadcastToTabs(void (Tab::*func)(void*),void* data)
{
	for(std::vector<Tab*>::iterator tTab=this->tabs.begin();tTab!=this->tabs.end();tTab++)
		((*tTab)->*func)(data);
}

void Container::BroadcastToSelectedTabRects(void (GuiRect::*func)(const GuiEvent&),void* iData)
{
	for(std::vector<Tab*>::iterator tTab=this->tabs.begin();tTab!=this->tabs.end();tTab++)
		(*tTab)->BroadcastToSelected(func,iData);
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

unsigned char Tab::rawUpArrow[]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0xa7,0x0,0x0,0x0,0xa8,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x87,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x88,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6a,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x6b,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x4b,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x4c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x2b,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x2c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xb,0x0,0x0,0x0,0xfa,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfb,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x7,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0x7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
unsigned char Tab::rawRightArrow[]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0x7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0xb,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfa,0x0,0x0,0x0,0x2b,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x4b,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x6a,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x87,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xa7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xa8,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x88,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x6b,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x4c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfb,0x0,0x0,0x0,0x2c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0x7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
unsigned char Tab::rawLeftArrow[]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x07,0x0,0x0,0x0,0x0c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0c,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x2c,0x0,0x0,0x0,0xfb,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x4c,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6b,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x01,0x0,0x0,0x0,0x88,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xa8,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xa7,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x01,0x0,0x0,0x0,0x87,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6a,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x4b,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x2b,0x0,0x0,0x0,0xfa,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0b,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x07,0x0,0x0,0x0,0x0c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
unsigned char Tab::rawDownArrow[]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xef,0x0,0x0,0x0,0xe7,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x7,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xdc,0x0,0x0,0x0,0x7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc,0x0,0x0,0x0,0xfb,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfa,0x0,0x0,0x0,0xb,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x2c,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xfe,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x2b,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x4c,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x4b,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6b,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x6a,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x88,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x87,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0xa8,0x0,0x0,0x0,0xa7,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
unsigned char Tab::rawFolder[]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe6,0xc1,0x9b,0x12,0xe9,0xc4,0x9f,0xd8,0xe9,0xc4,0xa0,0xff,0xe9,0xc4,0xa0,0xff,0xe9,0xc4,0xa0,0xff,0xe9,0xc4,0xa0,0xff,0xe9,0xc4,0xa0,0xff,0xe6,0xc0,0x9a,0x5b,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe5,0xc1,0x9e,0x20,0xe5,0xc2,0x9e,0xef,0xe5,0xc2,0x9e,0xff,0xe5,0xc2,0x9e,0xff,0xe5,0xc2,0x9d,0xff,0xe5,0xc2,0x9d,0xff,0xe5,0xc2,0x9d,0xff,0xe4,0xc2,0x9c,0xa6,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe3,0xbe,0x98,0x21,0xdf,0xb9,0x94,0xee,0xdf,0xb9,0x93,0xff,0xdf,0xb9,0x93,0xff,0xde,0xb9,0x92,0xff,0xde,0xb9,0x93,0xff,0xde,0xb9,0x93,0xff,0xdd,0xb7,0x90,0xe4,0xda,0xb2,0x89,0xa2,0xda,0xb2,0x8a,0xa3,0xda,0xb2,0x8a,0xa3,0xda,0xb2,0x8a,0xa3,0xda,0xb2,0x8a,0x86,0xe6,0xc1,0x97,0xf,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xfb,0xda,0xaf,0xd2,0xf7,0xd6,0xab,0xfd,0xf7,0xd6,0xab,0xff,0xf7,0xd6,0xab,0xff,0xf7,0xd6,0xab,0xff,0xf6,0xd6,0xab,0xff,0xf6,0xd6,0xab,0xff,0xf6,0xd6,0xab,0xff,0xf6,0xd5,0xab,0xff,0xf6,0xd5,0xab,0xff,0xf6,0xd5,0xab,0xff,0xf6,0xd5,0xab,0xff,0xf6,0xd5,0xab,0xff,0xfa,0xda,0xaf,0xd7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0xf7,0xd5,0xaa,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0xf3,0xcf,0xa3,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0xee,0xc8,0x9b,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0xea,0xc2,0x95,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0xe5,0xbb,0x8e,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0xe1,0xb6,0x87,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0xdc,0xb0,0x7f,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0xd8,0xa9,0x78,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0xd3,0xa3,0x71,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc9,0x96,0x64,0xe7,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xca,0x97,0x65,0xff,0xc9,0x97,0x64,0xf1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
unsigned char Tab::rawFile[]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc8,0xc8,0xc8,0x4c,0xe0,0xe0,0xe0,0xcd,0xe1,0xe1,0xe1,0xc0,0xe1,0xe1,0xe1,0xbf,0xe1,0xe1,0xe1,0xbf,0xe1,0xe1,0xe1,0xbf,0xe1,0xe1,0xe1,0xc0,0xd6,0xd6,0xd6,0xc4,0xbd,0xbd,0xbd,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc5,0xc5,0xc5,0x70,0xec,0xec,0xec,0xff,0xed,0xed,0xed,0xff,0xed,0xed,0xed,0xff,0xed,0xed,0xed,0xff,0xed,0xed,0xed,0xff,0xee,0xee,0xee,0xff,0xeb,0xeb,0xeb,0xff,0xce,0xce,0xce,0xff,0xbd,0xbd,0xbd,0xa6,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xbe,0xbe,0xbe,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc5,0xc5,0xc5,0x6c,0xec,0xec,0xec,0xff,0xed,0xed,0xed,0xff,0xed,0xed,0xed,0xff,0xed,0xed,0xed,0xff,0xed,0xed,0xed,0xff,0xed,0xed,0xed,0xff,0xef,0xef,0xef,0xff,0xd3,0xd3,0xd3,0xfd,0xe1,0xe1,0xe1,0xff,0xde,0xde,0xde,0xae,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc8,0xc8,0xc8,0x6c,0xee,0xee,0xee,0xff,0xef,0xef,0xef,0xff,0xef,0xef,0xef,0xff,0xef,0xef,0xef,0xff,0xef,0xef,0xef,0xff,0xef,0xef,0xef,0xff,0xf0,0xf0,0xf0,0xff,0xdb,0xdb,0xdb,0xff,0xc0,0xc0,0xc0,0xfe,0xc9,0xc9,0xc9,0xff,0xb9,0xb9,0xb9,0xbd,0x0,0x0,0x0,0x0,0xff,0xff,0xff,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc9,0xc9,0xc9,0x6b,0xf0,0xf0,0xf0,0xff,0xf1,0xf1,0xf1,0xff,0xf1,0xf1,0xf1,0xff,0xf1,0xf1,0xf1,0xff,0xf1,0xf1,0xf1,0xff,0xf1,0xf1,0xf1,0xff,0xf1,0xf1,0xf1,0xff,0xe7,0xe7,0xe7,0xff,0xc3,0xc3,0xc3,0xff,0xb6,0xb6,0xb6,0xfc,0xc2,0xc2,0xc2,0xff,0xbd,0xbd,0xbd,0x95,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xcb,0xcb,0xcb,0x6b,0xf2,0xf2,0xf2,0xff,0xf3,0xf3,0xf3,0xff,0xf3,0xf3,0xf3,0xff,0xf3,0xf3,0xf3,0xff,0xf3,0xf3,0xf3,0xff,0xf3,0xf3,0xf3,0xff,0xf3,0xf3,0xf3,0xff,0xf2,0xf2,0xf2,0xff,0xf2,0xf2,0xf2,0xff,0xf4,0xf4,0xf4,0xff,0xf2,0xf2,0xf2,0xff,0xd5,0xd5,0xd5,0xc0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xcd,0xcd,0xcd,0x6b,0xf4,0xf4,0xf4,0xff,0xf5,0xf5,0xf5,0xff,0xf5,0xf5,0xf5,0xff,0xf5,0xf5,0xf5,0xff,0xf5,0xf5,0xf5,0xff,0xf5,0xf5,0xf5,0xff,0xf5,0xf5,0xf5,0xff,0xf5,0xf5,0xf5,0xff,0xf6,0xf6,0xf6,0xff,0xf6,0xf6,0xf6,0xff,0xf8,0xf8,0xf8,0xff,0xe4,0xe4,0xe4,0xbc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xcf,0xcf,0xcf,0x6b,0xf6,0xf6,0xf6,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf7,0xf7,0xf7,0xff,0xf9,0xf9,0xf9,0xff,0xe5,0xe5,0xe5,0xbc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd0,0xd0,0xd0,0x6b,0xf8,0xf8,0xf8,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xf9,0xf9,0xf9,0xff,0xfb,0xfb,0xfb,0xff,0xe8,0xe8,0xe8,0xbc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd2,0xd2,0xd2,0x6b,0xf9,0xf9,0xf9,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfa,0xfa,0xfa,0xff,0xfc,0xfc,0xfc,0xff,0xe9,0xe9,0xe9,0xbb,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd3,0xd3,0xd3,0x6b,0xfb,0xfb,0xfb,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfc,0xfc,0xfc,0xff,0xfe,0xfe,0xfe,0xff,0xeb,0xeb,0xeb,0xbb,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd4,0xd4,0xd4,0x6b,0xfc,0xfc,0xfc,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xfd,0xfd,0xfd,0xff,0xff,0xff,0xff,0xff,0xec,0xec,0xec,0xbb,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd4,0xd4,0xd4,0x6b,0xfd,0xfd,0xfd,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xff,0xff,0xff,0xff,0xec,0xec,0xec,0xbb,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xd5,0xd5,0xd5,0x6a,0xfd,0xfd,0xfd,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xfe,0xfe,0xfe,0xff,0xff,0xff,0xff,0xff,0xed,0xed,0xed,0xbb,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xdb,0xdb,0xdb,0x70,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf1,0xf1,0xf1,0xc5,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xa4,0xa4,0xa4,0x41,0xbc,0xbc,0xbc,0xab,0xbb,0xbb,0xbb,0xa2,0xbb,0xbb,0xbb,0xa1,0xbb,0xbb,0xbb,0xa1,0xbb,0xbb,0xbb,0xa1,0xbb,0xbb,0xbb,0xa1,0xbb,0xbb,0xbb,0xa1,0xbb,0xbb,0xbb,0xa1,0xbb,0xbb,0xbb,0xa2,0xbb,0xbb,0xbb,0xa2,0xbd,0xbd,0xbd,0xaa,0xb4,0xb4,0xb4,0x73,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x4,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x4,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};





Tab::Tab(float x,float y,float w,float h):
	rects(this),
	rectsLayered(this),
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

	this->rectsLayered.name=L"RECTLAYERED";
}

GuiRect* Tab::GetSelected()
{
	return selected<rects.childs.size() ? rects.childs[selected] : 0;
}

void Tab::Draw()
{
	if(this->drawTask->pause)
		DEBUG_BREAK();

	if(!this->drawInstances.empty())
	{
		for(std::list<DrawInstance*>::iterator it=this->drawInstances.begin();it!=this->drawInstances.end();it)
		{
			DrawInstance*& tDrawInstance=*it;

			if(!tDrawInstance)
				DEBUG_BREAK();

			if(!tDrawInstance->skip && (tDrawInstance->code || tDrawInstance->frame))
			{
				if(this->BeginDraw())
				{
					/*if(tDrawInstance->frame)
						this->DrawFrame();*/

					switch(tDrawInstance->code)
					{
					case 1:this->OnGuiPaint();break;
					case 2:tDrawInstance->rect->OnPaint(GuiEvent(this,0,0));break;
					}

					/*static unsigned int a=0;
					printf("draw %d %0xp\n",a++,tDrawInstance->rect);*/

					this->EndDraw();
				}
			}

			if(tDrawInstance->remove)
			{
				SAFEDELETE(tDrawInstance);
				it=this->drawInstances.erase(it);
			}
			else it++;
		}
	}

	Ide::GetInstance()->stringEditor->Draw(this);
}

DrawInstance* Tab::SetDraw(int iNoneAllRect,bool iFrame,GuiRect* iRect,String iName,bool iRemove,bool iSkip)
{
	DrawInstance* newInstance=new DrawInstance(iNoneAllRect,iFrame,iRect,iName,iRemove,iSkip);
	this->drawInstances.push_back(newInstance);
	return newInstance;
}

void Tab::BroadcastToSelected(void (GuiRect::*iFunc)(const GuiEvent&),void* iData)
{
	GuiRect* selectedTab=this->GetSelected();

	if(selectedTab)
		(selectedTab->*iFunc)(GuiEvent(this,selectedTab,0,iFunc,iData));

	this->rectsLayered.BroadcastToChilds(iFunc,GuiEvent(this,&this->rectsLayered,0,iFunc,iData));
}

void Tab::BroadcastToAll(void (GuiRect::*iFunc)(const GuiEvent&),void* iData)
{
	(this->rects.*iFunc)(GuiEvent(this,&this->rects,0,iFunc,iData));
	this->rectsLayered.BroadcastToChilds(iFunc,GuiEvent(this,&this->rectsLayered,0,iFunc,iData));
}






void Tab::SetSelection(GuiRect* iRect)
{
	this->BroadcastToSelected(&GuiRect::OnDeactivate);

	for(size_t i=0;i<this->rects.childs.size();i++)
	{
		if(iRect==this->rects.childs[i])
		{
			this->selected=i;
			break;
		}
	}

	this->BroadcastToSelected(&GuiRect::OnActivate);
	this->BroadcastToSelected(&GuiRect::OnSize);

	this->SetDraw();
}


void Tab::OnGuiSize(void* data)
{
	GuiEvent tEvent(this,0,0,0,data);

	this->rects.OnSize(tEvent);
	this->rectsLayered.BroadcastToChilds(&GuiRect::OnSize,tEvent);

	this->BroadcastToSelected(&GuiRect::OnSize,data);
	
	this->resizeTarget=true;
}

void Tab::OnWindowPosChanging(void* data)
{
	this->resizeTarget=true;

	GuiEvent tEvent(this,0,0,0,data);

	this->rects.OnSize(tEvent);
	this->rectsLayered.BroadcastToChilds(&GuiRect::OnSize,tEvent);

	this->BroadcastToSelected(&GuiRect::OnSize,data);
}

void Tab::OnGuiMouseMove(void* data)
{
	splitterContainer->currentTabContainer=this;

	/*if(mouseDown && tmx!=mousex && tmy!=mousey)
		splitterContainer->CreateFloatingTab(this);

	mousex=tmx;
	mousey=tmy;*/

	//if(mousey>TabContainer::CONTAINER_HEIGHT)

	GuiRect* tOldHover=this->GetHover();
	
	this->BroadcastToSelected(&GuiRect::OnMouseMove,this->mouse);

	GuiRect* tNewHover=this->GetHover();

	if(tOldHover!=tNewHover)
	{
		if(tOldHover && tOldHover->active)
			this->SetDraw(2,0,tOldHover);
		if(tNewHover && tNewHover->active)
			this->SetDraw(2,0,tNewHover);
	}
}

void Tab::OnGuiLMouseUp(void* data)
{
	mouseDown=false;

	this->BroadcastToSelected(&GuiRect::OnLMouseUp,this->mouse);
}

void Tab::OnGuiMouseWheel(void* data)
{

}

void Tab::OnGuiLMouseDown(void* data)
{
	if(Ide::GetInstance()->GetPopup()!=this)
		Ide::GetInstance()->DestroyPopup();

	if(this->hasFrame && this->mouse.y<=BAR_HEIGHT)
	{
		float &x=this->mouse.x;
		float &y=this->mouse.y;

		int tPreviousTabSelected=selected;

		vec2 tDim(0,LABEL_LEFT_OFFSET);

		for(int i=0;i<(int)rects.childs.size();i++)
		{
			vec2 tTextSize = this->renderer2D->MeasureText(this->rects.childs[i]->name.c_str());

			tDim.x = tDim.y;
			tDim.y = tDim.x + tTextSize.x + LABEL_RIGHT_OFFSET;

			bool tMouseContained = (x>tDim.x && x< tDim.y) && (y>(BAR_HEIGHT-LABEL_HEIGHT) &&  y<BAR_HEIGHT);

			if(tMouseContained)
			{
				mouseDown=true;

				this->SetSelection(rects.childs[i]);

				break;
			}
		}
	}
	else
	{
		this->BroadcastToSelected(&GuiRect::OnLMouseDown,this->mouse);
	}
}

void Tab::OnGuiDLMouseDown(void* data)
{
	if(!this->hasFrame || this->mouse.y>BAR_HEIGHT)
		this->BroadcastToSelected(&GuiRect::OnDLMouseDown,this->mouse);
}


void Tab::OnGuiUpdate(void* data)
{
	this->BroadcastToSelected(&GuiRect::OnUpdate,data);
}

void Tab::OnGuiRMouseUp(void* data)
{
	this->BroadcastToSelected(&GuiRect::OnRMouseUp,data);
}

void Tab::OnGuiRender(void* data)
{
	this->BroadcastToSelected(&GuiRect::OnRender,data);
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
	for(size_t i=0;i<rects.childs.size();i++)
	{
		vec2 tTextSize = this->renderer2D->MeasureText(this->rects.childs[i]->name.c_str());

		tDim.x = tDim.y;
		tDim.y = tDim.x + tTextSize.x + LABEL_RIGHT_OFFSET;

		if(this->selected==i)
			this->renderer2D->DrawRectangle(tDim.x,(float)(BAR_HEIGHT-LABEL_HEIGHT),tDim.y,(float)((BAR_HEIGHT-LABEL_HEIGHT)+LABEL_HEIGHT),Tab::COLOR_LABEL);

		this->renderer2D->DrawText(rects.childs[i]->name,tDim.x,(float)BAR_HEIGHT-LABEL_HEIGHT,tDim.y,(float)(BAR_HEIGHT-LABEL_HEIGHT) + (float)LABEL_HEIGHT,vec2(0.5f,0.5f),vec2(0.5f,0.5f),GuiString::COLOR_TEXT);
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

	this->BroadcastToSelected(&GuiRect::OnPaint,data);
}

void Tab::OnResizeContainer(void* data)
{

}

void Tab::OnEntitiesChange(void* data)
{
	this->BroadcastToSelected(&GuiRect::OnEntitiesChange,data);
}
void Tab::OnGuiActivate(void* data)
{
	this->BroadcastToSelected(&GuiRect::OnActivate,data);
}
void Tab::OnGuiDeactivate(void* data)
{
	this->BroadcastToSelected(&GuiRect::OnDeactivate,data);
}
void Tab::OnGuiEntitySelected(void* data)
{
	this->BroadcastToSelected(&GuiRect::OnEntitySelected,data);
}

void Tab::OnGuiKeyDown(void* data)
{
	this->BroadcastToSelected(&GuiRect::OnKeyDown,data);
}

void Tab::OnGuiKeyUp(void* data)
{
	this->BroadcastToSelected(&GuiRect::OnKeyUp,data);
}

void Tab::OnGuiRecreateTarget(void* data)
{
	this->BroadcastToSelected(&GuiRect::OnRecreateTarget,data);
}

void Tab::SetFocus(GuiRect* iFocusedRect)
{
	if(Tab::focused)
		Tab::focused->OnExitFocus(GuiEvent(Tab::focused->GetRootRect()->tab,Tab::focused,0,&GuiRect::OnExitFocus,Tab::focused));

	Tab::focused=iFocusedRect;

	if(Tab::focused)
		Tab::focused->OnEnterFocus(GuiEvent(Tab::focused->GetRootRect()->tab,Tab::focused,0,&GuiRect::OnEnterFocus,Tab::focused));
}

void Tab::SetHover(GuiRect* iHoveredRect)
{
	if(iHoveredRect==Tab::hovered)
		return;

	if(Tab::hovered)
		Tab::hovered->hovering=false;

	Tab::hovered=iHoveredRect;
}

void Tab::SetPressed(GuiRect* iPressedRect)
{
	if(Tab::pressed)
		Tab::pressed->pressing=false;

	Tab::pressed=iPressedRect;

	if(Tab::pressed)
		Tab::pressed->pressing=true;
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
	floatingTabRefTabIdx=-1;
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
	blinkingRate(BLINKRATE),
	recalcBackground(false)
{}

void StringEditor::Bind(GuiString* iString)
{
	this->string=iString;

	this->tab=iString ? this->string->GetRootRect()->tab : 0;

	if(iString && !this->string->cursor)
	{
		this->string->cursor=new Cursor;
		this->string->cursor->cursor=iString->text->c_str();

		this->EditText(CARET_RECALC,0);
	}

	this->enabled=false;
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
	float iFontHeight=this->string->font->GetHeight();

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

				this->string->cursor->caret.z=tCharWidth;

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

			/*if(tColumn!=this->string->cursor->rowcol.y)//string is shorter of the lower matching position
				tRowCharWidth*/

			this->string->cursor->caret.x=tRowCharWidth;
			this->string->cursor->caret.y+=iFontHeight;
			this->string->cursor->rowcol.y=tColumn;
			this->string->cursor->rowcol.x++;

			break;
		}
	}

	if(tMustResize)
		this->string->OnSize(GuiEvent(this->tab,0,this->string,&GuiRect::OnSize,0));

	this->recalcBackground=true;

	this->tab->renderer2D->SetCaretPos(this->string->cursor->caret.x,this->string->cursor->caret.y);

	//wprintf(L"cursor: %d,col: %d\n",this->string->cursor->cursor-this->string->text->c_str(),this->string->cursor->rowcol.y);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////Renderer2D///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


const GuiFont* GuiFont::GetDefaultFont()
{
	return ::GlobalDefaultFontInstance();
}

const std::vector<GuiFont*>& GuiFont::GetFontPool()
{
	return ::GlobalFontPoolInstance();
}

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

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////GuiMsg/////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiEvent::GuiEvent(Tab* iTab,GuiRect* iSender,GuiRect* iTarget,void (GuiRect::*iFunction)(const GuiEvent&),void* iData):
	tab(iTab),
	sender(iSender),
	target(iTarget),
	func(iFunction),
	data(iData)
{}

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////GuiRect////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiRect::GuiRect(GuiRect* iParent,float ix, float iy, float iw,float ih,vec2 _alignPos,vec2 _alignRect):
	colorBackground(GuiRect::COLOR_BACK),
	colorHovering(GuiRect::COLOR_BACK),
	colorPressing(GuiRect::COLOR_BACK),
	colorChecked(GuiRect::COLOR_BACK),
	pictureBackGround(0),
	pictureHovering(0),
	picturePressing(0),
	pictureChecked(0),
	pressing(false),
	hovering(false),
	checked(false),
	active(false),
	scalars(1,1,1,1),
	minimums(0,0),
	userData(0),
	clip(0)
{
	this->SetEdges();
	this->SetParent(iParent);
	this->fixed.make(ix,iy,iw,ih);
}

GuiRect::~GuiRect()
{
	SAFEDELETE(this->pictureBackGround);
	SAFEDELETE(this->pictureChecked);
	SAFEDELETE(this->pictureHovering);
	SAFEDELETE(this->picturePressing);

	this->DestroyChilds();
}

void GuiRect::SetParent(GuiRect* iParent)
{
	GuiRect* oldParent=this->parent;
	this->parent=iParent;

	if(oldParent)
		oldParent->childs.erase(std::find(oldParent->childs.begin(),oldParent->childs.end(),this));

	if(this->parent)
	{
		this->parent->childs.push_back(this);

		GuiScrollRect* tScrollRectParent=dynamic_cast<GuiScrollRect*>(iParent);

		GuiScrollRect* tScrollRectClip=tScrollRectParent ? tScrollRectParent : (iParent->clip ? iParent->clip : 0);

		if(tScrollRectClip)
			this->SetClip(tScrollRectClip);

		if(this->parent->active && !this->active)
		{
			GuiRootRect* tRootRect=this->parent->GetRootRect();
			Tab* tTab=tRootRect ? tRootRect->tab : 0;
			GuiEvent tEvent(tTab);

			if(tTab)
				this->OnActivate(tEvent);
		}
	}
	else
	{
		this->SetClip(0);
	}

	//this->active=iParent ? iParent->active : 0;
}

void GuiRect::OnSizePre()
{
	Edges &re=this->refedges;
	vec4  &o=this->offsets;
	vec4  &s=this->scalars;
	vec4  &e=this->edges;
	vec4  &r=this->rect;
	vec4  &f=this->fixed;
	vec2  &m=this->minimums;

	float tLeft=0;
	float tTop=0;
	float tRight=0;
	float tBottom=0;

	if(this->parent)
	{
		vec4  &pe=this->parent->edges;

		tLeft= (re.left ? *re.left : (f.x ? f.x : pe.x));
		tTop= (re.top ? *re.top : (f.y ? f.y : pe.y));
		tRight= (re.right ? *re.right : (f.z ? tLeft+f.z : pe.z));
		tBottom= (re.bottom ? *re.bottom : (f.w ? tTop+f.w : pe.w));
	}
	else
	{
		tLeft= (re.left ? *re.left : (f.x ? f.x : 0));
		tTop= (re.top ? *re.top : (f.y ? f.y : 0));
		tRight= (re.right ? *re.right : (f.z ? tLeft+f.z : 0));
		tBottom= (re.bottom ? *re.bottom : (f.w ? tTop+f.w : 0));
	}

	tLeft += (s.x!=1 ? (tRight-tLeft)*s.x : 0);
	tTop += (s.y!=1 ? (tBottom-tTop)*s.y : 0);

	tRight = (s.z!=1 ? tLeft + (tRight-tLeft)*s.z : tRight);
	tBottom = (s.w!=1 ? tTop + (tBottom-tTop)*s.w : tBottom);

	tLeft += o.x;
	tTop += o.y;
	tRight += o.z;
	tBottom += o.w;

	float tWidth=tRight-tLeft;
	float tHeight=tBottom-tTop;

	tWidth=(tWidth<m.x) ? m.x : tWidth;
	tHeight=(tHeight<m.y) ? m.y : tHeight;

	e.make(tLeft,tTop,tLeft+tWidth,tTop+tHeight);
	r.make(tLeft,tTop,tWidth,tHeight);

	this->content.make(e.z,e.w);
}

void GuiRect::OnSizePost()
{
	if(this->parent)
	{
		if(this->edges.z>this->parent->edges.z)
			this->parent->content.x=this->edges.z;

		if(this->edges.w>this->parent->edges.w)
			this->parent->content.y=this->edges.w;
	}
}


void GuiRect::SetClip(GuiScrollRect* iScrollingRect)
{
	this->clip=iScrollingRect;

	for(std::vector<GuiRect*>::iterator tRect=this->childs.begin();tRect!=this->childs.end();tRect++)
		(*tRect)->SetClip(iScrollingRect);
}

void GuiRect::BeginClip(Tab* iTab)
{
	this->clip 
		? 
		this->clip->BeginClip(iTab) 
		: 
		iTab->renderer2D->PushScissor(this->edges.x,this->edges.y,this->edges.z,this->edges.w);
}

void GuiRect::EndClip(Tab* iTab)
{
	this->clip ? this->clip->EndClip(iTab) : iTab->renderer2D->PopScissor();
}

void GuiRect::SetColors(unsigned int iBack,unsigned int iHover,unsigned int iPress,unsigned int iCheck,bool iPropagate)
{
	this->colorBackground=iBack;
	this->colorHovering=iHover;
	this->colorPressing=iPress;
	this->colorChecked=iCheck;

	if(iPropagate)
	{
		for(std::vector<GuiRect*>::iterator it=this->childs.begin();it!=this->childs.end();it++)
			(*it)->SetColors(iBack,iHover,iPress,iCheck,iPropagate);
	}
}

void GuiRect::SetAllColors(unsigned int iColor,bool iPropagate)
{
	this->colorBackground=iColor;
	this->colorHovering=iColor;
	this->colorPressing=iColor;
	this->colorChecked=iColor;

	if(iPropagate)
	{
		for(std::vector<GuiRect*>::iterator it=this->childs.begin();it!=this->childs.end();it++)
			(*it)->SetColors(iColor,iPropagate);
	}
}

void GuiRect::SetEdges(float* iLeft,float* iTop,float* iRight,float* iBottom)
{
	this->refedges.left=iLeft;
	this->refedges.top=iTop;
	this->refedges.right=iRight;
	this->refedges.bottom=iBottom;
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

unsigned int GuiRect::GetColor()
{
	unsigned int tColor=this->colorBackground;

	if(this->checked)
		tColor=blend(tColor,this->colorChecked);
	if(this->hovering)
		tColor=blend(tColor,this->colorHovering);
	if(this->pressing)
		tColor=blend(tColor,this->colorPressing);

	return tColor;
}

bool GuiRect::Contains(vec4& quad,vec2 point)
{
	return (point.x>quad.x && point.x<=quad.z && point.y>quad.y && point.y<=quad.w);
}

void GuiRect::BroadcastToChilds(void (GuiRect::*func)(const GuiEvent&),const GuiEvent& iMsg)
{
	for(std::vector<GuiRect*>::iterator tRect=this->childs.begin();tRect!=this->childs.end();tRect++)
		((*tRect)->*func)(iMsg);
}


void GuiRect::BroadcastToRoot(void (GuiRect::*func)(const GuiEvent&),const GuiEvent& iMsg)
{
	GuiRootRect* root=dynamic_cast<GuiRootRect*>(this->GetRoot());

	if(root)
		(root->*func)(iMsg);
}



void GuiRect::OnEntitiesChange(const GuiEvent& iMsg)
{
	this->BroadcastToChilds(&GuiRect::OnEntitiesChange,iMsg);
}

void GuiRect::DrawBackground(Tab* iTab)
{
	if(this->pictureBackGround || this->pictureChecked || this->pictureHovering || this->picturePressing)
	{
		if(this->pictureBackGround)
			iTab->renderer2D->DrawBitmap(this->pictureBackGround,this->edges.x,this->edges.y,this->edges.z,this->edges.w);
		if(this->pictureChecked)
			iTab->renderer2D->DrawBitmap(this->pictureChecked,this->edges.x,this->edges.y,this->edges.z,this->edges.w);
		if(this->pictureHovering)
			iTab->renderer2D->DrawBitmap(this->pictureHovering,this->edges.x,this->edges.y,this->edges.z,this->edges.w);
		if(this->picturePressing)
			iTab->renderer2D->DrawBitmap(this->picturePressing,this->edges.x,this->edges.y,this->edges.z,this->edges.w);
	}
	else
	{
		iTab->renderer2D->DrawRectangle(this->edges.x,this->edges.y,this->edges.z,this->edges.w,this->GetColor(),true);
	}
}


void GuiRect::OnPaint(const GuiEvent& iMsg)
{
	this->BeginClip(iMsg.tab);
	this->DrawBackground(iMsg.tab);
	this->BroadcastToChilds(&GuiRect::OnPaint,iMsg);
	this->EndClip(iMsg.tab);
}


void GuiRect::OnSize(const GuiEvent& iMsg)
{
	this->OnSizePre();
	this->BroadcastToChilds(&GuiRect::OnSize,iMsg);
	this->OnSizePost();
}

void GuiRect::OnLMouseDown(const GuiEvent& iMsg)
{
	bool tWasPressing=this->pressing;

	this->pressing=this->hovering;

	if(this->hovering)
		iMsg.tab->SetFocus(this);

	this->BroadcastToChilds(&GuiRect::OnLMouseDown,iMsg);

	if(tWasPressing!=this->pressing)
		iMsg.tab->SetDraw(2,0,this);
}

void GuiRect::OnDLMouseDown(const GuiEvent& iMsg)
{
	this->BroadcastToChilds(&GuiRect::OnDLMouseDown,iMsg);
}

void GuiRect::OnLMouseUp(const GuiEvent& iMsg)
{
	bool tWasPressing=this->pressing;
	bool tWasChecked=this->checked;

	if(this->hovering && this->pressing)
	{	
		this->checked=!this->checked;
		this->pressing=false;
	}

	this->BroadcastToChilds(&GuiRect::OnLMouseUp,iMsg);

	if(tWasPressing!=this->pressing || tWasChecked!=this->checked)
		iMsg.tab->SetDraw(2,0,this);
}

void GuiRect::OnRMouseUp(const GuiEvent& iMsg)
{
	this->BroadcastToChilds(&GuiRect::OnRMouseUp,iMsg);
}

void GuiRect::OnMouseMove(const GuiEvent& iMsg)
{
	vec2 mpos=*(vec2*)iMsg.data;

	bool tWasHovering=this->hovering;
	this->hovering=this->Contains(this->edges,mpos);

	if(this->hovering)
		iMsg.tab->SetHover(this);

	this->BroadcastToChilds(&GuiRect::OnMouseMove,iMsg);
}

void GuiRect::OnUpdate(const GuiEvent& iMsg)
{
	this->BroadcastToChilds(&GuiRect::OnUpdate,iMsg);
}

void GuiRect::OnReparent(const GuiEvent& iMsg)
{
	this->BroadcastToChilds(&GuiRect::OnReparent,iMsg);
}

void GuiRect::OnSelected(const GuiEvent& iMsg)
{
	this->BroadcastToChilds(&GuiRect::OnSelected,iMsg);
}

void GuiRect::OnRender(const GuiEvent& iMsg)
{
	this->BroadcastToChilds(&GuiRect::OnRender,iMsg);
}

void GuiRect::OnMouseWheel(const GuiEvent& iMsg)
{
	this->BroadcastToChilds(&GuiRect::OnMouseWheel,iMsg);
}

void GuiRect::OnActivate(const GuiEvent& iMsg)
{
	this->active=true;

	if(this->pictureBackGround)
		iMsg.tab->renderer2D->LoadBitmap(this->pictureBackGround);
	if(this->pictureChecked)
		iMsg.tab->renderer2D->LoadBitmap(this->pictureChecked);
	if(this->pictureHovering)
		iMsg.tab->renderer2D->LoadBitmap(this->pictureHovering);
	if(this->picturePressing)
		iMsg.tab->renderer2D->LoadBitmap(this->picturePressing);

	GuiRect::BroadcastToChilds(&GuiRect::OnActivate,iMsg);
}
void GuiRect::OnDeactivate(const GuiEvent& iMsg)
{
	this->active=false;
	GuiRect::BroadcastToChilds(&GuiRect::OnDeactivate,iMsg);
}
void GuiRect::OnEntitySelected(const GuiEvent& iMsg)
{
	this->BroadcastToChilds(&GuiRect::OnEntitySelected,iMsg);
}

void GuiRect::OnExpandos(const GuiEvent& iMsg)
{
	if(this->parent)
		this->parent->OnExpandos(iMsg);
}

void GuiRect::OnKeyDown(const GuiEvent& iMsg)
{
	this->BroadcastToChilds(&GuiRect::OnKeyDown,iMsg);
}

void GuiRect::OnKeyUp(const GuiEvent& iMsg)
{
	this->BroadcastToChilds(&GuiRect::OnKeyDown,iMsg);
}

void GuiRect::OnMouseEnter(const GuiEvent& iMsg)
{
	this->BroadcastToChilds(&GuiRect::OnMouseEnter,iMsg);
}
void GuiRect::OnMouseExit(const GuiEvent& iMsg)
{
	this->BroadcastToChilds(&GuiRect::OnMouseExit,iMsg);
}

void GuiRect::OnEnterFocus(const GuiEvent& iMsg)
{
	this->BroadcastToChilds(&GuiRect::OnEnterFocus,iMsg);
}

void GuiRect::OnExitFocus(const GuiEvent& iMsg)
{
	this->BroadcastToChilds(&GuiRect::OnExitFocus,iMsg);
}

GuiRect* GuiRect::GetRoot()
{
	return this->parent ? this->parent->GetRoot() : this;
}

GuiRootRect* GuiRect::GetRootRect()
{
	return (GuiRootRect*)this->GetRoot();
}

GuiString* GuiRect::Text(String str)
{
	GuiString* label=new GuiString;
	label->parent=this;
	label->text=str.c_str();
	this->childs.push_back(label);
	return label;
}

void GuiRect::Insert(GuiRect* iProperty)
{
	iProperty->SetEdges(0,this->childs.size() ? &this->childs.back()->edges.w : 0,0,0); 

	iProperty->SetParent(this);
}
	

GuiContainer* GuiRect::Container(String iText)
{
	GuiContainer* tContainer=new GuiContainer;

	tContainer->text=iText;
	tContainer->textAlign.make(0,0.5f);

	this->Insert(tContainer);

	return tContainer;
}

template<typename T> GuiContainerRow<T>* GuiRect::ContainerRow(T iData)
{
	 GuiContainerRow<T>* tContainerRow=new GuiContainerRow<T>;

	 tContainerRow->rowData=iData;

	this->Insert(tContainerRow);

	return tContainerRow;
}

void GuiRect::DestroyChilds()
{
	for(std::vector<GuiRect*>::iterator it=this->childs.begin();it!=this->childs.end();it++)
		SAFEDELETE(*it);

	this->childs.clear();
}

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////Compiler/////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

Compiler::Compiler()
{
	ideSrcPath=Ide::GetInstance()->pathExecutable.PathUp(5) + L"\\src";
	ideLibPath=Ide::GetInstance()->pathExecutable.Path();

	COMPILER msCompiler={L"ms",
						 L"vc2010",
						 L"cl.exe ",
						 L"linker.exe ",
						 L" /nologo /MDd /ZI /EHsc ",
						 L" /link /MANIFEST:NO /DLL /NOENTRY ",
						 L" /OUT:",
						 L"enginelibMS",
						 L".lib ",
						 L" /I"
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


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////EditorEntity////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

EditorEntity::EditorEntity():
	selected(false),
	expanded(false),
	level(0)
{
	this->entityViewerContainer.name=L"EntityRootProperties";
	this->entityViewerContainer.SetStringMode(this->Entity::name,true);
	this->entityViewerContainer.userData=this;
	this->entityViewerContainer.state=true;

	this->sceneViewerRow.SetStringMode(this->Entity::name,true);
	this->sceneViewerRow.userData=this;
}

void EditorEntity::SetParent(Entity* iParent)
{
	Entity::SetParent(iParent);

	if(iParent)
		((EditorEntity*)iParent)->sceneViewerRow.Insert(&this->sceneViewerRow);

	this->SetLevel(this);
}

void EditorEntity::SetLevel(EditorEntity* iEntity)
{
	EditorEntity* eeParent=(EditorEntity*)iEntity->parent;

	iEntity->level=eeParent ? eeParent->level+1 : iEntity->level;
	iEntity->expanded=!iEntity->level ? true : false;

	for(std::list<Entity*>::iterator i=iEntity->childs.begin();i!=iEntity->childs.end();i++)
		((EditorEntity*)*i)->SetLevel((EditorEntity*)*i);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////GuiRootRect/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiRootRect::GuiRootRect(Tab* t):tab(t)
{
	this->name=L"RootRect";
	//this->Set(0,0,0,-1,0,0,0,0,0,0,1,1);

}

GuiRootRect::~GuiRootRect()
{
}

void GuiRootRect::OnSize(const GuiEvent& iData)
{
	vec2 tTabSize=this->tab->Size();
	vec2 tTabTopBottom;

	if(this->tab->hasFrame)
	{
		tTabTopBottom.x=Tab::BAR_HEIGHT;
		tTabTopBottom.y=tTabSize.y-Tab::BAR_HEIGHT;
	}
	else
	{
		tTabTopBottom.x=0;
		tTabTopBottom.y=tTabSize.y;
	}

	this->rect.make(0.0f,tTabTopBottom.x,tTabSize.x,tTabTopBottom.y);
	this->edges.x=0;
	this->edges.y=tTabTopBottom.x;
	this->edges.z=tTabSize.x;
	this->edges.w=tTabSize.y;
}

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////GuiContainer//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiContainer::GuiContainer():state(false)
{
	this->name=L"GuiContainer";
	this->fixed.make(0,0,0,20);
}

void GuiContainer::OnSizePre(Tab* iTab)
{
	this->margins.x=this->childs.empty() ? 0 : ROW_ADVANCE;

	GuiString::OnSizePre(iTab);
}

void GuiContainer::OnSizePost(Tab* iTab)
{
	if(this->state && !this->childs.empty())
	{
		GuiRect* tLastChild=this->childs.back();

		this->rect.w=tLastChild->edges.w-this->edges.y;
		this->edges.w=tLastChild->edges.w;
		
		
	}

	GuiString::OnSizePost();
}

void GuiContainer::Insert(GuiRect* iProperty)
{
	iProperty->SetEdges(0,this->childs.size() ? &this->childs.back()->edges.w : 0,0,0); 

	float tYOffset=iProperty->refedges.top ? 0 : ROW_HEIGHT;

	iProperty->offsets.make(ROW_ADVANCE,tYOffset,0,tYOffset);

	iProperty->SetParent(this);
}

void GuiContainer::BroadcastToChilds(void (GuiRect::*iFunction)(const GuiEvent&),const GuiEvent& iMsg)
{
	if(this->state)
		GuiString::BroadcastToChilds(iFunction,iMsg);
}

void GuiContainer::OnSize(const GuiEvent& iMsg)
{
	this->OnSizePre(iMsg.tab);
	this->BroadcastToChilds(&GuiRect::OnSize,iMsg);
	this->OnSizePost(iMsg.tab);
}

void GuiContainer::OnPaint(const GuiEvent& iMsg)
{
	this->BeginClip(iMsg.tab);
	this->DrawBackground(iMsg.tab);

	if(this->childs.size())
		iMsg.tab->renderer2D->DrawBitmap(
												this->state ?
												iMsg.tab->iconDown :
												iMsg.tab->iconRight,
												this->rect.x,
												this->rect.y,
												this->rect.x+Tab::ICON_WH,
												this->rect.y+Tab::ICON_WH
											);

	this->DrawTheText(iMsg.tab);
	this->BroadcastToChilds(&GuiRect::OnPaint,iMsg);
	this->EndClip(iMsg.tab);
}


void GuiContainer::OnLMouseDown(const GuiEvent& iMsg)
{
	GuiString::OnLMouseDown(iMsg);

	vec2 tMousePosition=*(vec2*)iMsg.data;

	bool tContainerButtonPressed=(	
								tMousePosition.x > this->rect.x && 
								tMousePosition.x < this->rect.x+Tab::ICON_WH &&
								tMousePosition.y > this->rect.y && 
								tMousePosition.y <this->rect.y+Tab::ICON_WH
							);

	if(tContainerButtonPressed)
	{
		this->state=!this->state;

		this->checked=false;
		this->pressing=false;

		GuiRect* tRoot=this;

		while(tRoot)
		{
			if(!dynamic_cast<GuiContainer*>(tRoot->parent))
				break;

			tRoot=tRoot->parent;
		}

		tRoot->OnSize(iMsg);
		this->OnExpandos(iMsg);
		iMsg.tab->SetDraw(2,0,tRoot->parent ? tRoot->parent : tRoot);
	}
}
///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////GuiContainerRow//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

template<typename T> GuiContainerRow<T>::GuiContainerRow():rowData(0)
{
	this->colorHovering=0x8f8f8f;
	this->colorHovering=0x8f8f8f;
	this->colorChecked=0x9f9fff;
}

template<typename T> void GuiContainerRow<T>::UnselectAll(GuiRect* iRect)
{
	iRect->checked=false;
	iRect->pressing=false;

	for(std::vector<GuiRect*>::iterator tRect=iRect->childs.begin();tRect!=iRect->childs.end();tRect++)
		this->UnselectAll(*tRect);
}

template<typename T> void GuiContainerRow<T>::OnLMouseUp(const GuiEvent& iMsg)
{
	bool tWasPressing=this->pressing;

	GuiRect::OnLMouseUp(iMsg);

	if(tWasPressing)
	{	
		GuiRect* tRoot=this;

		bool tCancelOldSelection=!InputManager::keyboardInput.IsPressed(/*VK_CONTROL*/0x11);

		if(tCancelOldSelection)
		{
			while(tRoot)
			{
				GuiContainerRow* tRectParentRow=dynamic_cast<GuiContainerRow*>(tRoot->parent);

				if(!tRectParentRow)
					break; 

				tRoot=tRoot->parent;
			}

			this->UnselectAll(tRoot);

			this->checked=true;
		}
		
		iMsg.tab->SetDraw(2,0,tRoot);
	}
}



template<typename T> void GuiContainerRow<T>::DrawBackground(const GuiEvent& iMsg)
{
	vec4 tRowRectangle=this->GetRowRectangle();
	unsigned int tColor=this->GetColor();

	iMsg.tab->renderer2D->DrawRectangle(tRowRectangle.x,tRowRectangle.y,tRowRectangle.z,tRowRectangle.w,tColor,true);
}

template<typename T> vec4 GuiContainerRow<T>::GetRowRectangle()
{
	return vec4(this->clip->edges.x,this->edges.y,this->clip->edges.z,this->edges.y+this->fixed.w);
}

template<typename T> void GuiContainerRow<T>::OnMouseMove(const GuiEvent& iMsg)
{
	vec2 mpos=*(vec2*)iMsg.data;

	vec4 tRowRectangle=GetRowRectangle();

	bool tWasHovering=this->hovering;
	this->hovering=this->Contains(tRowRectangle,mpos);

	if(this->hovering)
		iMsg.tab->SetHover(this);

	this->BroadcastToChilds(&GuiRect::OnMouseMove,iMsg);
}

template<typename T> void GuiContainerRow<T>::OnSizePre(Tab* iTab)
{
	GuiContainer::OnSizePre(iTab);
}

template<typename T> void GuiContainerRow<T>::OnSize(const GuiEvent& iMsg)
{
	this->OnSizePre(iMsg.tab);
	this->BroadcastToChilds(&GuiRect::OnSize,iMsg);
	this->OnSizePost(iMsg.tab);
}

template<typename T> void GuiContainerRow<T>::OnPaint(const GuiEvent& iMsg)
{
	this->BeginClip(iMsg.tab);

	this->DrawBackground(iMsg);

	vec2 icons(this->rect.x,this->rect.y);

	if(this->childs.size())
	{
		iMsg.tab->renderer2D->DrawBitmap(
			this->state ?
			iMsg.tab->iconDown :
		iMsg.tab->iconRight,
			icons.x,
			icons.y,
			icons.x+Tab::ICON_WH,
			icons.y+Tab::ICON_WH
			);

		icons.x+=ROW_ADVANCE;
	}

	this->DrawTheText(iMsg.tab);
	this->BroadcastToChilds(&GuiRect::OnPaint,iMsg);
	this->EndClip(iMsg.tab);
}

template<> void GuiContainerRow<ResourceNode*>::OnSizePre(Tab* iTab)
{
	this->margins.x=ROW_ADVANCE;

	GuiString::OnSizePre(iTab);
}

template<> void GuiContainerRow<ResourceNodeDir*>::OnSizePre(Tab* iTab)
{
	this->margins.x=ROW_ADVANCE + (this->childs.empty() ? 0 : ROW_ADVANCE);

	GuiString::OnSizePre(iTab);
}

template<> void GuiContainerRow<ResourceNode*>::OnPaint(const GuiEvent& iMsg)
{
	this->BeginClip(iMsg.tab);

	this->DrawBackground(iMsg);

	vec2 icons(this->rect.x,this->rect.y);

	if(this->rowData)
	iMsg.tab->renderer2D->DrawBitmap(
									this->rowData->isDir ?
									iMsg.tab->iconFolder :
									iMsg.tab->iconFile,
									icons.x,
									icons.y,
									icons.x+Tab::ICON_WH,
									icons.y+Tab::ICON_WH
									);

	this->DrawTheText(iMsg.tab);
	this->BroadcastToChilds(&GuiRect::OnPaint,iMsg);
	this->EndClip(iMsg.tab);
}

template<> void GuiContainerRow<ResourceNodeDir*>::OnPaint(const GuiEvent& iMsg)
{
	this->BeginClip(iMsg.tab);

	this->DrawBackground(iMsg);

	vec2 icons(this->rect.x,this->rect.y);

	if(this->childs.size())
	{
		iMsg.tab->renderer2D->DrawBitmap(
			this->state ?
			iMsg.tab->iconDown :
		iMsg.tab->iconRight,
			icons.x,
			icons.y,
			icons.x+Tab::ICON_WH,
			icons.y+Tab::ICON_WH
			);

		icons.x+=ROW_ADVANCE;
	}


	if(this->rowData)
		iMsg.tab->renderer2D->DrawBitmap(
		this->rowData->isDir ?
		iMsg.tab->iconFolder :
	iMsg.tab->iconFile,
		icons.x,
		icons.y,
		icons.x+Tab::ICON_WH,
		icons.y+Tab::ICON_WH
		);

	this->DrawTheText(iMsg.tab);
	this->BroadcastToChilds(&GuiRect::OnPaint,iMsg);
	this->EndClip(iMsg.tab);
}


///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////GuiContainerRawFile//////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

/*
GuiContainerRowFile::GuiContainerRowFile()
{
	this->textOffsets.x=ROW_ADVANCE*2.0f;
}

void GuiContainerRowFile::OnPaint(const GuiMsg& iMsg)
{
	this->BeginClip(iMsg.tab);
	this->DrawBackground(iMsg.tab);

	if(this->childs.size())
		iMsg.tab->renderer2D->DrawBitmap(
		this->state ?
		iMsg.tab->iconDown :
	iMsg.tab->iconRight,
		this->rect.x,
		this->rect.y,
		this->rect.x+Tab::CONTAINER_ICON_WH,
		this->rect.y+Tab::CONTAINER_ICON_WH
		);

	if(this->filename.size())
		iMsg.tab->renderer2D->DrawBitmap(
		this->state ?
		iMsg.tab->iconDown :
	iMsg.tab->iconRight,
		this->rect.x,
		this->rect.y,
		this->rect.x+Tab::CONTAINER_ICON_WH,
		this->rect.y+Tab::CONTAINER_ICON_WH
		);

	this->DrawTheText(iMsg.tab);
	this->BroadcastToChilds(&GuiRect::OnPaint,iMsg);
	this->EndClip(iMsg.tab);
}
*/

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////StringValue///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

StringValue::StringValue(GuiString& iGuiString,void* iValuePointer1,unsigned int iValueType,void* iValuePointer2,unsigned int iValueParameter1,unsigned int iValueParameter2):
	guiString(iGuiString),
	valuePointer1(iValuePointer1),
	valuePointer2(iValuePointer2),
	valueType(iValueType),
	valueParameter1(iValueParameter1),
	valueParameter2(iValueParameter2)
{
}

StringValue::~StringValue(){}

void StringValue::RefreshReference(Tab* iTabContainer)
{
	const unsigned int ctMaxTmpArraySize=256;

	switch(this->valueType)
	{
	case StringValue::BOOL:
		{
			bool& tBool=*(bool*)this->valuePointer1;

			this->guiString.text=tBool ? L"True" : L"False";
		}
		break;
	case StringValue::BOOLPTR:
		{
			void* pRef=*(void**)this->valuePointer1;

			this->guiString.text=pRef ? L"True" : L"False";
		}
		break;
	case StringValue::INT:
		{
			int& tInt=*(int*)this->valuePointer1;

			wchar_t tCharInt[ctMaxTmpArraySize];
			swprintf(tCharInt,L"%d",tInt);

			this->guiString.text=tCharInt;
		}
		break;
	case StringValue::FLOAT:
		{
			float& tFloat=*(float*)this->valuePointer1;

			//https://stackoverflow.com/questions/16413609/printf-variable-number-of-decimals-in-float

			wchar_t tCharFloat[ctMaxTmpArraySize];
			swprintf(tCharFloat,L"%*.*f",this->valueParameter1,this->valueParameter2,tFloat);

			this->guiString.text=tCharFloat;
		}
		break;
	case StringValue::VEC2:
		{
			vec2& tVec2=*(vec2*)this->valuePointer1;

			wchar_t tCharVec2[ctMaxTmpArraySize];
			swprintf(tCharVec2,L"%*.*f , %*.*f",this->valueParameter1,this->valueParameter2,tVec2.x,this->valueParameter1,this->valueParameter2,tVec2.y);

			this->guiString.text=tCharVec2;
		}
		break;
	case StringValue::VEC3:
		{
			vec3& tVec3=*(vec3*)this->valuePointer1;

			wchar_t tCharVec3[ctMaxTmpArraySize];
			swprintf(tCharVec3,L"%*.*f , %*.*f , %*.*f",	this->valueParameter1,this->valueParameter2,tVec3.x,
				this->valueParameter1,this->valueParameter2,tVec3.y,
				this->valueParameter1,this->valueParameter2,tVec3.z);

			this->guiString.text=tCharVec3;
		}
		break;
	case StringValue::VEC4:
		{
			vec4& tVec4=*(vec4*)this->valuePointer1;

			wchar_t tCharVec4[ctMaxTmpArraySize];
			swprintf(tCharVec4,L"%*.*f , %*.*f , %*.*f , %*.*f",	this->valueParameter1,this->valueParameter2,tVec4.x,
				this->valueParameter1,this->valueParameter2,tVec4.y,
				this->valueParameter1,this->valueParameter2,tVec4.z,
				this->valueParameter1,this->valueParameter2,tVec4.w);

			this->guiString.text=tCharVec4;
		}
		break;
	case StringValue::PTR:
		{
			wchar_t tCharPointer[ctMaxTmpArraySize];
			swprintf(tCharPointer,L"0x%p",this->valuePointer1);

			this->guiString.text=tCharPointer;
		}
		break;
	case StringValue::MAT4POS:
		{
			mat4& tMat4=*(mat4*)this->valuePointer1;

			vec3 tPosition=tMat4.position();

			wchar_t tCharVec3[ctMaxTmpArraySize];
			swprintf(tCharVec3,L"%*.*f , %*.*f , %*.*f",	this->valueParameter1,this->valueParameter2,tPosition.x,
				this->valueParameter1,this->valueParameter2,tPosition.y,
				this->valueParameter1,this->valueParameter2,tPosition.z);

			this->guiString.text=tCharVec3;
		}
		break;
	case StringValue::ENTITYLISTSIZE:
		{
			std::list<Entity*>& tEntityVec=*(std::list<Entity*>*)this->valuePointer1;

			size_t tCount=tEntityVec.size();

			wchar_t tCharVecSize[10];
			swprintf(tCharVecSize,L"%d",tCount);

			this->guiString.text=tCharVecSize;
		}
		break;
	case StringValue::VEC3VECSIZE:
		{
			std::vector<vec3>& tVec3Vector=*(std::vector<vec3>*)this->valuePointer1;

			size_t tCount=tVec3Vector.size();

			wchar_t tCharVecSize[10];
			swprintf(tCharVecSize,L"%d",tCount);

			this->guiString.text=tCharVecSize;
		}
		break;
	case StringValue::VEC3LISTSIZE:
		{
			std::list<vec3>& tVec3List=*(std::list<vec3>*)this->valuePointer1;

			size_t tCount=tVec3List.size();

			wchar_t tCharVecSize[10];
			swprintf(tCharVecSize,L"%d",tCount);

			this->guiString.text=tCharVecSize;
		}
		break;
	case StringValue::ANIMATIONVECSIZE:
		{
			std::vector<Animation*>& tAnimationVec=*(std::vector<Animation*>*)this->valuePointer1;

			size_t tCount=tAnimationVec.size();

			wchar_t tCharVecSize[10];
			swprintf(tCharVecSize,L"%d",tCount);

			this->guiString.text=tCharVecSize;
		}
		break;
	case StringValue::ISBONECOMPONENT:
		{
			Entity* tEntity=(Entity*)this->valueParameter1;

			this->guiString.text=L"must implement in GuiStringValue::ISBONECOMPONENT";
		}
		break;
	case StringValue::FLOAT2MINUSFLOAT1:
		{
			float& a=*(float*)this->valuePointer1;
			float& b=*(float*)this->valuePointer2;

			wchar_t tCharFloatMinusOp[20];
			swprintf(tCharFloatMinusOp,L"%*.*g",this->valueParameter1,this->valueParameter2,b-a);

			this->guiString.text=tCharFloatMinusOp;
		}
		break;
	case StringValue::VEC32MINUSVEC31:
		{
			vec3& a=*(vec3*)this->valuePointer1;
			vec3& b=*(vec3*)this->valuePointer2;

			vec3 tVecResult=b-a;

			wchar_t tCharVec3[ctMaxTmpArraySize];
			swprintf(tCharVec3,L"%*.*g , %*.*g , %*.*g",	this->valueParameter1,this->valueParameter2,tVecResult.x,
				this->valueParameter1,this->valueParameter2,tVecResult.y,
				this->valueParameter1,this->valueParameter2,tVecResult.z);

			this->guiString.text=tCharVec3;
		}
		break;
	default:
		this->guiString.text=L"GuiStringValue Must Implement";
	}
}
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////GuiString///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiString::GuiStringBase::GuiStringBase():_______text(new String),________dtor(true){};
GuiString::GuiStringBase::~GuiStringBase()
{
	if(________dtor)
	{
		SAFEDELETE(this->_______text);
		________dtor=false;
	}
}
String& GuiString::GuiStringBase::Text()
{
	return *this->_______text;
}
void GuiString::GuiStringBase::SetStringMode(String& iString,bool isReference)
{
	if(this->________dtor)
		SAFEDELETE(this->_______text);

	if(isReference)
	{
		this->________dtor=false;
		this->_______text=&iString;
	}
	else
	{
		this->_______text=new String;
		this->________dtor=true;
		this->_______text->assign(iString);
	};
}
GuiString::GuiStringBase::operator String&()
{
	return *this->_______text;
}
String& GuiString::GuiStringBase::operator=(const String& iString)
{
	return *this->_______text=iString;
}
String* GuiString::GuiStringBase::operator->()
{
	return this->_______text;
}

GuiString::GuiString():
	textAlign(0.0f,0.5f),
	textSpot(0.0f,0.5f),
	canEdit(false),
	adaptRect(false),
	cursor(0),
	textColor(GuiString::COLOR_TEXT),
	font((GuiFont*)GuiFont::GetDefaultFont()),
	valueData(0)
{
	this->fixed.make(0,0,0,20);
	this->name=L"GuiString";
}

GuiString::GuiString(String iString):
	textAlign(0.0f,0.5f),
	textSpot(0.0f,0.5f),
	canEdit(false),
	adaptRect(false),
	cursor(0),
	textColor(GuiString::COLOR_TEXT),
	font((GuiFont*)GuiFont::GetDefaultFont()),
	valueData(0)
{
	this->fixed.make(0,0,0,20);
	this->name=L"GuiString";
	this->text=iString;
}

GuiString::GuiString(String* iString):
	textAlign(0.0f,0.5f),
	textSpot(0.0f,0.5f),
	canEdit(false),
	adaptRect(false),
	cursor(0),
	textColor(GuiString::COLOR_TEXT),
	font((GuiFont*)GuiFont::GetDefaultFont()),
	valueData(0)
{
	this->fixed.make(0,0,0,20);
	this->name=L"GuiString";
	this->SetStringMode(*iString,true);
}

GuiString::GuiString(void* iValuePointer1,unsigned int iValueType,void* iValuePointer2,unsigned int iValueParameter1,unsigned int iValueParameter2):
	textAlign(0.0f,0.5f),
	textSpot(0.0f,0.5f),
	canEdit(false),
	adaptRect(false),
	cursor(0),
	textColor(GuiString::COLOR_TEXT),
	font((GuiFont*)GuiFont::GetDefaultFont()),
	valueData(new StringValue(*this,iValuePointer1,iValueType,iValuePointer2,iValueParameter1,iValueParameter2))
{
	this->fixed.make(0,0,0,20);
	this->name=L"GuiStringValue";
}

GuiString::~GuiString()
{
	SAFEDELETE(this->cursor);
	SAFEDELETE(this->valueData);
}

void GuiString::SetStringMode(String& iString,bool isReference)
{
	SAFEDELETE(this->valueData);

	this->text.SetStringMode(iString,isReference);
}

void GuiString::SetStringMode(void* iValuePointer1,unsigned int iValueType,void* iValuePointer2,unsigned int iValueParameter1,unsigned int iValueParameter2)
{
	SAFEDELETE(this->valueData);

	this->valueData=new StringValue(*this,iValuePointer1,iValueType,iValuePointer2,iValueParameter1,iValueParameter2);

	this->text.SetStringMode(String(L""),false);
}

void GuiString::OnSizePre(Tab* iTab)
{
	GuiRect::OnSizePre();

	float tWidth=this->edges.z-this->edges.x;
	float tHeight=this->edges.w-this->edges.y;

	wchar_t* tText=(wchar_t*)this->text->c_str();

	this->textRect=iTab->renderer2D->MeasureText(this->text->c_str());

	float tLeft=this->edges.x + (tWidth*this->textAlign.x) + this->margins.x - (this->textRect.x * this->textSpot.x);
	float tTop=this->edges.y + (tHeight*this->textAlign.y) + this->margins.y - (this->textRect.y * this->textSpot.y);

	tLeft<0 ? tLeft=0 : 0;
	tTop<0 ? tTop=0 : 0;

	this->textEdges.make(
		tLeft,
		tTop,
		tLeft + this->textRect.x,
		tTop + this->textRect.y
		);

	if(this->adaptRect)
	{
		if(this->textRect.x>this->rect.z)
		{
			this->edges.z=this->edges.x+this->textRect.x;
			this->rect.z=this->textRect.x;
		}
		if(this->textRect.y>this->rect.w)
		{
			this->edges.w=this->edges.y+this->textRect.y;
			this->rect.w=this->textRect.y;
		}
	}
}


void GuiString::OnSize(const GuiEvent& iMsg)
{
	if(this->valueData)
		this->valueData->RefreshReference(iMsg.tab);

	this->OnSizePre(iMsg.tab);

	this->BroadcastToChilds(&GuiRect::OnSize,iMsg);
	this->OnSizePost();
}

void GuiString::DrawTheText(Tab* iTab)
{
	iTab->renderer2D->DrawText(
											this->text,
											this->textEdges.x,
											this->textEdges.y,
											this->textEdges.z,
											this->textEdges.w,
											this->textSpot,
											this->textAlign,
											this->textColor,
											this->font
										);
}

void GuiString::OnPaint(const GuiEvent& iMsg)
{
	this->BeginClip(iMsg.tab);
	this->DrawBackground(iMsg.tab);
	this->DrawTheText(iMsg.tab);
	this->BroadcastToChilds(&GuiRect::OnPaint,iMsg);
	this->EndClip(iMsg.tab);
}

bool GuiString::ParseKeyInput(const GuiEvent& iMsg)
{
	StringEditor&	tStringEditor=*Ide::GetInstance()->stringEditor;

	bool			tRedraw=false;

	if(tStringEditor.Binded()==this && tStringEditor.Enabled())
	{
		unsigned int	tCaretOperation=tStringEditor.CARET_DONTCARE;
		void*			tCaretParameter=0;
		char			tCharcode;

		if(iMsg.data)
		{
			tCharcode=*(int*)iMsg.data;

			switch(tCharcode)
			{
				case 0x08:/*VK_BACK*/tCaretOperation=tStringEditor.CARET_BACKSPACE; break;
				default:
					tCaretOperation=tStringEditor.CARET_ADD;
					tCaretParameter=&tCharcode;
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

void GuiString::OnKeyDown(const GuiEvent& iMsg)
{
	if(this->canEdit && this==iMsg.tab->GetFocus())
	{
		StringEditor& tStringEditor=*Ide::GetInstance()->stringEditor;

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
			tRedraw=this->ParseKeyInput(iMsg);

		if(tRedraw)
			iMsg.tab->SetDraw(2,0,this);
	}

	GuiRect::OnKeyDown(iMsg);
}


void GuiString::OnLMouseDown(const GuiEvent& iMsg)
{
	if(this->hovering)
		iMsg.tab->SetFocus(this);

	GuiRect::OnLMouseDown(iMsg);
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

GuiListBox::GuiListBox():selection(0)
{
	this->name=L"GuiListBox";

	this->fixed.w=100;
}

void GuiListBox::DestroyItems()
{
	this->DestroyChilds();
}

void GuiListBox::AddItem(void* iValuePointer1,unsigned int iValueType,void* iValuePointer2,unsigned int iValueParameter1,unsigned int iValueParameter2)
{
	GuiButton* tButton=new GuiButton;
	tButton->refedges.top=this->childs.size() ? &this->childs.back()->edges.w : 0;
	tButton->fixed.w=GuiRect::ROW_HEIGHT;
	tButton->SetStringMode(iValuePointer1,iValueType,iValuePointer2,iValueParameter1,iValueParameter2);
	tButton->colorBackground+=(this->childs.size()%2)==0 ? 0x101010 : 0x151515;
	tButton->colorHovering+=0x202020;
	tButton->func=GuiListBoxHelpers::OnItemPressed;
	tButton->param=tButton;
	tButton->userData=this;
	tButton->textSpot.x=0.0f;
	tButton->textAlign.x=0.0f;
	tButton->margins.x=10;

	tButton->SetParent(this);
}

void GuiListBox::AddItem(String iString)
{
	GuiButton* tButton=new GuiButton;
	tButton->refedges.top=this->childs.size() ? &this->childs.back()->edges.w : 0;
	tButton->fixed.w=GuiRect::ROW_HEIGHT;
	tButton->SetStringMode(iString,false);
	tButton->colorBackground+=(this->childs.size()%2)==0 ? 0x101010 : 0x151515;
	tButton->colorHovering+=0x202020;
	tButton->func=GuiListBoxHelpers::OnItemPressed;
	tButton->param=tButton;
	tButton->userData=this;
	tButton->textSpot.x=0.0f;
	tButton->textAlign.x=0.0f;
	tButton->margins.x=10;

	tButton->SetParent(this);
}


GuiListBox::~GuiListBox()
{
}

void GuiListBoxHelpers::OnItemPressed(void* iData)
{
	GuiButton*	tButton=(GuiButton*)iData;
	GuiListBox* tListBox=(GuiListBox*)tButton->userData;
	tListBox->selection=tButton;
}

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////GuiComboBox////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

void pfdGuiComboBoxLabelsButtonAction(void* iData)
{
	GuiButton*		tButton=(GuiButton*)iData;
	GuiComboBox*	tComboBox=(GuiComboBox*)tButton->userData;

	tComboBox->string->text=tButton->text;

	tComboBox->GetRootRect()->tab->SetDraw(2,0,tComboBox->string);

	Ide::GetInstance()->GetPopup()->windowData->Show(false);
}


void pfdGuiComboBoxDownButtonAction(void* iData)
{
	GuiComboBox* tComboBox=(GuiComboBox*)iData;

	if(tComboBox->popupPointer)
	{
		tComboBox->popupPointer=Ide::GetInstance()->GetPopup();
	}
	else
	{
		Tab*		tTab=tComboBox->GetRootRect()->tab;
		Container*	tContainer=tTab->container;

		vec2		tPopupSize(tComboBox->string->rect.z+tComboBox->button->rect.z,tComboBox->items.size()*GuiRect::ROW_HEIGHT);

		vec2 tTabPos=tTab->windowData->Pos();

		tTabPos.x+=tComboBox->string->edges.x;
		tTabPos.y+=tComboBox->string->edges.w;

		tComboBox->popupPointer=Ide::GetInstance()->CreatePopup(tContainer,tTabPos.x,tTabPos.y,tPopupSize.x,tPopupSize.y<300.0f?tPopupSize.y:300.0f);

		if(!tComboBox->list)
			tComboBox->RecreateList();

		tComboBox->list->SetParent(&tComboBox->popupPointer->rects);

		tComboBox->popupPointer->OnGuiActivate();
		tComboBox->popupPointer->OnGuiSize();

		tComboBox->popupPointer->SetDraw(2,0,tComboBox);
	}
}

void GuiComboBox::RecreateList()
{
	if(!this->list)
		this->list=new GuiScrollRect;
	else
		this->list->DestroyChilds();

	if(this->items.size())
	{
		this->string->text=this->items[0];

		for(size_t i=0;i<this->items.size();i++)
		{
			GuiButton* tButton=new GuiButton;
			tButton->refedges.top=this->list->childs.size() ? &this->list->childs.back()->edges.w : 0;
			tButton->fixed.w=GuiRect::ROW_HEIGHT;
			tButton->text=this->items[i];
			tButton->colorBackground+=(i%2)==0 ? 0x101010 : 0x151515;
			tButton->colorHovering+=0x202020;
			tButton->func=pfdGuiComboBoxLabelsButtonAction;
			tButton->param=tButton;
			tButton->userData=this;
			tButton->textSpot.x=0.0f;
			tButton->textAlign.x=0.0f;
			tButton->margins.x=10;

			tButton->SetParent(this->list);
		}
	}
}

GuiComboBox::GuiComboBox():selectedItem(0),list(0),popupPointer(0)
{
	this->name=L"GuiComboBox";

	this->fixed.w=GuiRect::ROW_HEIGHT;

	this->string=new GuiString;
	this->string->offsets.z=-Tab::ICON_WH;
	this->string->offsets.y=2;
	this->string->offsets.w=-2;
	this->string->SetParent(this);

	this->string->colorBackground=0xffffff;
	this->string->colorHovering=0xffffff;
	this->string->colorChecked=0xffffff;
	this->string->colorPressing=0xffffff;
	
	this->button=new GuiButton;
	this->button->refedges.left=&this->string->edges.z;
	this->button->pictureBackGround=new PictureRef(Tab::rawDownArrow,Tab::ICON_WH,Tab::ICON_WH);
	this->button->func=pfdGuiComboBoxDownButtonAction;
	this->button->param=this;
	this->button->SetParent(this);
}

GuiComboBox::~GuiComboBox()
{
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

void GuiImage::OnPaint(const GuiEvent& iMsg)
{
	this->BeginClip(iMsg.tab);
	this->DrawBackground(iMsg.tab);

	if(this->image && this->image->handle)
		iMsg.tab->renderer2D->DrawBitmap(this->image,this->edges.x,this->edges.y,this->edges.z,this->edges.w);

	this->BroadcastToChilds(&GuiRect::OnPaint,iMsg);
	this->EndClip(iMsg.tab);
}

void GuiImage::OnActivate(const GuiEvent& iMsg)
{
	GuiRect::OnActivate(iMsg);

	if(this->image && this->image->handle)
	{
		bool tErrorLoading=iMsg.tab->renderer2D->LoadBitmap(this->image);

		if(tErrorLoading)
			wprintf(L"error loading image (rect %p ,image %p\n",this,this->image);
	}
}

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////GuiTextBox/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
GuiTextBox::GuiTextBox()
{
	this->name=L"GuiTextBox";

	this->offsets.make(0,2,0,-2);
	this->SetAllColors(0xffffff,false);


	this->canEdit=true;
	this->textSpot.make(0,0);
	this->textAlign.make(0,0);
	this->adaptRect=true;
}

GuiTextBox::~GuiTextBox()
{

}

void GuiTextBox::OnKeyDown(const GuiEvent& iMsg)
{
	if(this==iMsg.tab->GetFocus())
	{
		StringEditor& tStringEditor=*Ide::GetInstance()->stringEditor;

		if(tStringEditor.Binded()==this && tStringEditor.Enabled())
		{
			bool tRedraw=this->ParseKeyInput(iMsg);

			if(tRedraw)
				iMsg.tab->SetDraw(2,0,this);
		}
	}

	GuiRect::OnKeyDown(iMsg);
}

void GuiTextBox::OnLMouseDown(const GuiEvent& iMsg)
{
	StringEditor& tStringEditor=*Ide::GetInstance()->stringEditor;

	if(this->hovering)
	{
		iMsg.tab->SetFocus(this);

		if(tStringEditor.Binded()!=this)
		{
			tStringEditor.Bind(this);
			tStringEditor.Enable(true);
		}

	}
	else
	{
		if(tStringEditor.Binded()==this)
			tStringEditor.Bind(0);
	}

	GuiRect::OnLMouseDown(iMsg);
}

void GuiTextBox::SetColors(unsigned int iBack,unsigned int iHover,unsigned int iPress,unsigned int iCheck,bool iPropagate)
{
	if(!iPropagate)
	{
		this->colorBackground=iBack;
		this->colorHovering=iHover;
		this->colorPressing=iPress;
		this->colorChecked=iCheck;
	}

	if(iPropagate)
	{
		for(std::vector<GuiRect*>::iterator it=this->childs.begin();it!=this->childs.end();it++)
			(*it)->SetColors(iBack,iHover,iPress,iCheck,iPropagate);
	}
}

void GuiTextBox::SetAllColors(unsigned int iColor,bool iPropagate)
{
	if(!iPropagate)
	{
		this->colorBackground=iColor;
		this->colorHovering=iColor;
		this->colorPressing=iColor;
		this->colorChecked=iColor;
	}

	if(iPropagate)
	{
		for(std::vector<GuiRect*>::iterator it=this->childs.begin();it!=this->childs.end();it++)
			(*it)->SetColors(iColor,iPropagate);
	}
}

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////GuiButton//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiButton::GuiButton():func(0),param(0),value(0),mode(-1)
{
	this->name=L"GuiButton";

	this->fixed.w=20;

	this->textAlign.make(0.5f,0.5f);
	this->textSpot.make(0.5f,0.5f);

	this->colorHovering=GuiRect::COLOR_BACK+30;
	this->colorPressing=GuiRect::COLOR_BACK+90;
}
void GuiButton::OnLMouseUp(const GuiEvent& iMsg)
{
	GuiString::OnLMouseUp(iMsg);

	if(this->hovering)
	{
		if(this->func)
			this->func(this->param);

		if(this->value)
		{
			switch(this->mode)
			{
			case -1:	*value=!*value; break;
			case 0:		*value=false;  break;
			case 1:		*value=true;	  break;
			}
		}

		if(this->parent)
		{
			GuiEvent tMsg(iMsg.tab,this,0,&GuiRect::OnButtonPressed,this);
			this->parent->OnButtonPressed(tMsg);
		}	
	}
}

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////GuiScrollRect//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiScrollRect::GuiScrollRect():
clipped(false),
hScrollbar(GuiScrollBar::SCROLLBAR_HORIZONTAL),
vScrollbar(GuiScrollBar::SCROLLBAR_VERTICAL)
{
	this->vScrollbar.guiRect=this;
	this->vScrollbar.SetEdges(&this->edges.z,&this->edges.y,0,&this->edges.w);

	this->hScrollbar.guiRect=this;
	this->hScrollbar.SetEdges(&this->edges.x,&this->edges.w,&this->edges.z,0);
}

GuiScrollRect::~GuiScrollRect()
{
}

vec2 GuiScrollRect::GetClippedMouse(vec2& iMpos)
{
	vec2 tMpos(iMpos);

	tMpos.x=tMpos.x+this->hScrollbar.scrollerPosition*(this->content.x-this->edges.x);
	tMpos.y=tMpos.y+this->vScrollbar.scrollerPosition*(this->content.y-this->edges.y);

	return tMpos;
}

void GuiScrollRect::OnLMouseDown(const GuiEvent& iMsg)
{
	vec2 tMpos=this->GetClippedMouse(*(vec2*)iMsg.data);

	this->GuiRect::OnLMouseDown(GuiEvent(iMsg.tab,this,0,0,tMpos));

	this->vScrollbar.OnLMouseDown(iMsg);
	this->hScrollbar.OnLMouseDown(iMsg);
}

void GuiScrollRect::OnMouseMove(const GuiEvent& iMsg)
{
	vec2 tMpos=this->GetClippedMouse(*(vec2*)iMsg.data);

	this->GuiRect::OnMouseMove(GuiEvent(iMsg.tab,this,0,0,tMpos));

	this->vScrollbar.OnMouseMove(iMsg);
	this->hScrollbar.OnMouseMove(iMsg);
}

void GuiScrollRect::OnMouseWheel(const GuiEvent& iMsg)
{
	if(this->vScrollbar.IsVisible() && this->Contains(this->edges,iMsg.tab->mouse))
		this->vScrollbar.Scroll(iMsg.tab,*(float*)iMsg.data);

	this->BroadcastToChilds(&GuiRect::OnMouseWheel,iMsg);
}

void GuiScrollRect::OnExpandos(const GuiEvent& iMsg)
{
	this->OnSize(iMsg);
	iMsg.tab->SetDraw(2,0,this);

	GuiRect::OnExpandos(iMsg);
}

void GuiScrollRect::BroadcastToChilds(void (GuiRect::*iFunction)(const GuiEvent&),const GuiEvent& iMsg)
{
	GuiRect::BroadcastToChilds(iFunction,iMsg);

	(this->vScrollbar.*iFunction)(iMsg);
	(this->hScrollbar.*iFunction)(iMsg);
}

void GuiScrollRect::OnSize(const GuiEvent& iMsg)
{
	bool tVSWasVisible=this->vScrollbar.IsVisible();
	bool tHSWasVisible=this->hScrollbar.IsVisible();

	GuiRect::OnSize(iMsg);

	float vContent=this->content.y-this->edges.y;
	float hContent=this->content.x-this->edges.x;

	this->vScrollbar.SetScrollerRatio(vContent,this->rect.w);
	this->hScrollbar.SetScrollerRatio(hContent,this->rect.z);

	if(this->vScrollbar.IsVisible())
	{
		if(this->vScrollbar.IsVisible())
		{
			this->rect.z-=GuiScrollBar::SCROLLBAR_TICK;
			this->edges.z-=GuiScrollBar::SCROLLBAR_TICK;
		}
	}

	if(this->hScrollbar.IsVisible())
	{
		if(this->hScrollbar.IsVisible())
		{
			this->rect.w-=GuiScrollBar::SCROLLBAR_TICK;
			this->edges.w-=GuiScrollBar::SCROLLBAR_TICK;
		}
	}

	this->BroadcastToChilds(&GuiRect::OnSize,iMsg);
}

void GuiScrollRect::SetClip(GuiScrollRect* iScrollingRect)
{
	this->clip=iScrollingRect;

	for(std::vector<GuiRect*>::iterator tRect=this->childs.begin();tRect!=this->childs.end();tRect++)
		(*tRect)->SetClip(this);
}

void GuiScrollRect::BeginClip(Tab* tabContainer)
{
	if(!this->clipped)
	{
		vec4& tClipEdges=this->edges;
		tabContainer->renderer2D->PushScissor(tClipEdges.x,tClipEdges.y,tClipEdges.z,tClipEdges.w);
		tabContainer->renderer2D->Translate(0,-this->vScrollbar.scrollerPosition*(this->content.y-this->edges.y));
		this->clipped=true;
	}
}

void GuiScrollRect::EndClip(Tab* iTab)
{
	iTab->renderer2D->DrawRectangle(this->rect.x+1,this->rect.y+1,this->rect.x+this->rect.z-1,this->rect.y+this->rect.w-1,0xffffff,false);

	if(this->clipped)
	{
		iTab->renderer2D->PopScissor();
		iTab->renderer2D->Identity();
		this->clipped=false;
	}
}

void GuiScrollRect::OnPaint(const GuiEvent& iMsg)
{
	this->BeginClip(iMsg.tab);
	this->GuiRect::DrawBackground(iMsg.tab);
	this->GuiRect::BroadcastToChilds(&GuiRect::OnPaint,iMsg);
	this->EndClip(iMsg.tab);
	this->vScrollbar.OnPaint(iMsg);
	this->hScrollbar.OnPaint(iMsg);
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

GuiPath::GuiPath():func(0),param(0)
{
	this->name=L"GuiPropertyTextBox";

	this->fixed.make(0,0,0,ROW_HEIGHT);

	this->path=new GuiTextBox;

	this->path->offsets.z=-Tab::ICON_WH;

	this->path->SetParent(this);

	this->button=new GuiButton;
	this->button->refedges.left=&this->path->edges.z;
	this->button->func=GuiPathHelpers::OnSelectDirectoryButtonPressed;
	this->button->param=this;

	this->button->pictureBackGround=new PictureRef(Tab::rawFolder,Tab::ICON_WH,Tab::ICON_WH);

	this->button->SetParent(this);
}

GuiPath::~GuiPath()
{

}

void GuiPathHelpers::OnSelectDirectoryButtonPressed(void* iData)
{
	GuiPath* tGuiPropertyPath=(GuiPath*)iData;

	Tab* iTab=tGuiPropertyPath->GetRootRect()->tab;

	iTab->isModal ? iTab->windowData->Enable(false) : Ide::GetInstance()->mainAppWindow->mainContainer->windowData->Enable(false);
	iTab->SetDraw();

	String tDirectory=Ide::GetInstance()->subsystem->DirectoryChooser(L"",L"");

	if(tDirectory.size())
		tGuiPropertyPath->path->text=tDirectory;

	iTab->isModal ? iTab->windowData->Enable(true) : Ide::GetInstance()->mainAppWindow->mainContainer->windowData->Enable(true);
	iTab->SetDraw();

	if(tGuiPropertyPath->func)
		tGuiPropertyPath->func(tGuiPropertyPath->param);
}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////GuiSlider///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiSlider::GuiSlider():referenceValue(0),minimum(0),maximum(0)
{
	this->name=L"GuiSlider";
	this->fixed.make(0,0,0,30);
}

GuiSlider::GuiSlider(float* iRef,float* iMin,float* iMax):referenceValue(iRef),minimum(iMin),maximum(iMax)
{
	this->name=L"GuiSlider";
	this->fixed.make(0,0,0,30);
}

void GuiSlider::DrawSliderTip(Tab* tabContainer,void* data)
{
	
}


void GuiSlider::OnPaint(const GuiEvent& iMsg)
{
	this->BeginClip(iMsg.tab);
	this->DrawBackground(iMsg.tab);

	if(this->minimum && this->maximum && this->referenceValue)
	{
		iMsg.tab->renderer2D->DrawRectangle(this->edges.x+10,this->rect.y+this->rect.w/2.0f-2,this->edges.z-10,this->rect.y+this->rect.w/2.0f+2,0x000000);

		float tMinimum=*this->minimum;

		String smin(StringUtils::Float(*this->minimum));
		String smax(StringUtils::Float(*this->maximum));
		String value(StringUtils::Float(*this->referenceValue));

		iMsg.tab->renderer2D->DrawText(smin,this->edges.x+10,this->edges.y,this->edges.x+this->rect.z/2,this->edges.y+10,vec2(0,0.5f),vec2(0,0.5f),GuiString::COLOR_TEXT);
		iMsg.tab->renderer2D->DrawText(smax,this->edges.x+this->rect.z/2,this->edges.y,this->edges.z-10,this->edges.y+10,vec2(1,0.5f),vec2(1.0f,0.5f),GuiString::COLOR_TEXT);
		iMsg.tab->renderer2D->DrawText(value,this->edges.x,this->edges.y+20,this->edges.z,this->edges.w,vec2(0.5f,0.5f),vec2(0.5f,0.5f),GuiString::COLOR_TEXT);

		float tip=(this->rect.x+10) + ((*referenceValue)/(*maximum-*minimum))*(this->rect.z-20);

		iMsg.tab->renderer2D->DrawRectangle(tip-5,this->rect.y+this->rect.w/2.0f-5,tip+5,rect.y+this->rect.w/2.0f+5,GuiString::COLOR_TEXT);
	}

	this->BroadcastToChilds(&GuiRect::OnPaint,iMsg);
	this->EndClip(iMsg.tab);
}

void GuiSlider::OnMouseMove(const GuiEvent& iMsg)
{
	GuiRect::OnMouseMove(iMsg);

	if(this->pressing && this->minimum && this->maximum && this->referenceValue)
	{
		vec2& mpos=*(vec2*)iMsg.data;

		if(mpos.x > this->rect.x && mpos.x < this->rect.x+this->rect.z)
		{
			float f1=(mpos.x-(this->rect.x+10))/(this->rect.z-20);
			float f2=*maximum-*minimum;
			float cursor=f1*f2;

			cursor = cursor<*minimum ? *minimum : (cursor>*maximum ? *maximum : cursor);

			if(*referenceValue!=cursor)
			{
				*referenceValue=cursor;
				iMsg.tab->SetDraw(2,0,this);
			}
		}
	}
}



///////////////////////////////////////////////
///////////////////////////////////////////////
///////////GuiAnimationController//////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiAnimationController::GuiAnimationController(AnimationController& iAnimationController)
	:animationController(iAnimationController)
{
	this->name=L"GuiAnimationController";

	this->play=new GuiButton;
	this->stop=new GuiButton;

	this->play->value=&this->animationController.play;
	this->stop->value=&this->animationController.play;

	this->slider=new GuiSlider(&animationController.cursor,&this->animationController.start,&this->animationController.end);

	this->fixed.make(0,0,0,50);

	this->slider->SetParent(this);

	this->play->SetParent(this);
	this->play->SetEdges(0,&this->slider->edges.w);
	this->play->scalars.make(1,1,0.5,1);
	this->play->offsets.make(10,2.5f,-5,-2.5f);
	this->play->colorBackground+=0x151515;
	this->play->text=L"Play";

	this->stop->SetParent(this);
	this->stop->SetEdges(&this->play->edges.z,&this->slider->edges.w);
	this->stop->scalars.make(1,1,1,1);
	this->stop->offsets.make(10,2.5f,-10,-2.5f);
	this->stop->colorBackground+=0x151515;
	this->stop->text=L"Stop";

	this->play->mode=1;
	this->stop->mode=0;//set 0 onlyif 1
}

void GuiAnimationController::OnMouseMove(const GuiEvent& iMsg)
{
	float value=*this->slider->referenceValue;

	GuiRect::OnMouseMove(iMsg);

	if(value!=*this->slider->referenceValue && this->slider->pressing)
	{
		this->animationController.SetFrame(*this->slider->referenceValue);
		iMsg.tab->SetDraw(2,0,this);
	}
}

void GuiAnimationController::OnButtonPressed(const GuiEvent& iMsg)
{
	if(iMsg.data==&this->play)
		this->animationController.Play();

	if(iMsg.data==&this->stop)
		this->animationController.Stop();
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

#define INITLIST \
	renderBuffer(0), \
	rootEntity(0), \
	needsPicking(0), \
	pickedEntity(0), \
	playStopButton(0), \
	renderDrawInstance(0), \
	renderFps(60), \
	renderBitmap(0)

void GuiViewportInitFunc(GuiViewport* iGuiViewport)
{
	GlobalViewports().push_back(iGuiViewport);

	iGuiViewport->name=L"ViewportViewer";
	
	iGuiViewport->playStopButton=new GuiButton;
	
	iGuiViewport->playStopButton->func=launchStopGuiViewportCallback;
	iGuiViewport->playStopButton->param=iGuiViewport;
	iGuiViewport->playStopButton->text=L"Run";
	iGuiViewport->playStopButton->SetEdges(0,&iGuiViewport->edges.y);
	iGuiViewport->playStopButton->colorBackground=MainContainer::COLOR_BACK;
	
	iGuiViewport->playStopButton->SetParent(iGuiViewport);
}

std::list<GuiViewport*>& GuiViewport::GetPool()
{
	return GlobalViewports();
}

GuiViewport::GuiViewport():INITLIST
{
	GuiViewportInitFunc(this);
}

GuiViewport::GuiViewport(vec3 pos,vec3 target,vec3 up,bool perspective):INITLIST
{
	GuiViewportInitFunc(this);

	this->projection= !perspective ? this->projection : this->projection.perspective(90,16/9,1,1000);
	this->view.move(pos);
	this->view.lookat(target,up);
}


void GuiViewport::OnSize(const GuiEvent& iMsg)
{
	GuiRect::OnSize(iMsg);
}

void GuiViewport::OnPaint(const GuiEvent& iMsg)
{
	this->BeginClip(iMsg.tab);

#if ENABLE_RENDERER

	if(Timer::GetInstance()->GetCurrent()-this->lastFrameTime>(1000.0f/this->renderFps))
	{
		this->lastFrameTime=Ide::GetInstance()->timer->GetCurrent();
		this->Render(iMsg.tab);
	}
	else
		this->DrawBuffer(iMsg.tab,this->rect);
#endif

	this->BroadcastToChilds(&GuiRect::OnPaint,iMsg);
	this->EndClip(iMsg.tab);
}

void GuiViewport::OnMouseWheel(const GuiEvent& iMsg)
{
	GuiRect::OnMouseWheel(iMsg);

	float factor=*(float*)iMsg.data;

	this->view*=mat4().translate(0,0,factor*10);
}

void GuiViewport::OnLMouseUp(const GuiEvent& iMsg)
{
	if(this->hovering/* && this->pickedEntity*/)
		Ide::GetInstance()->mainAppWindow->mainContainer->BroadcastToTabs(&Tab::OnGuiEntitySelected,(void*)this->pickedEntity);

	GuiRect::OnLMouseUp(iMsg);
}

void GuiViewport::OnMouseMove(const GuiEvent& iMsg)
{
	GuiRect::OnMouseMove(iMsg);

	if(this->hovering)
	{
		vec2 &mpos=*(vec2*)iMsg.data;

		if(InputManager::keyboardInput.IsPressed(0x01/*VK_LBUTTON*/))
		{
			float dX=(mpos.x-this->mouseold.x);
			float dY=(mpos.y-this->mouseold.y);

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

		mouseold=mpos;
	}
}


void GuiViewport::OnActivate(const GuiEvent& iMsg)
{
	GuiRect::OnActivate(iMsg);
#if ENABLE_RENDERER
	iMsg.tab->Create3DRenderer();
	iMsg.tab->OnGuiSize();
	this->renderDrawInstance=iMsg.tab->SetDraw(2,0,this,L"",false);
#endif
}
void GuiViewport::OnDeactivate(const GuiEvent& iMsg)
{
	GuiRect::OnDeactivate(iMsg);
#if ENABLE_RENDERER
	this->renderDrawInstance->remove=true;
#endif
}

void GuiViewport::OnReparent(const GuiEvent& iMsg)
{
	GuiRect::OnReparent(iMsg);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////GuiScrollBar////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiScrollBar::GuiScrollBar(unsigned int iScrollbarType):
	scrollbarType(iScrollbarType),
	guiRect(0),
	scrollerPressed(-1),
	scrollerPosition(0),
	scrollerRatio(1)
{
	this->fixed.make(0,0,this->scrollbarType==SCROLLBAR_VERTICAL ? GuiScrollBar::SCROLLBAR_TICK : 0,this->scrollbarType==SCROLLBAR_HORIZONTAL ? GuiScrollBar::SCROLLBAR_TICK : 0);
	this->name=L"ScrollBar";
}
GuiScrollBar::~GuiScrollBar()
{

}

void GuiScrollBar::SetType(unsigned int iScrollbarType)
{
	this->scrollbarType=iScrollbarType;

	this->fixed.make(0,0,this->scrollbarType==SCROLLBAR_VERTICAL ? GuiScrollBar::SCROLLBAR_TICK : 0,this->scrollbarType==SCROLLBAR_HORIZONTAL ? GuiScrollBar::SCROLLBAR_TICK : 0);
}


void GuiScrollBar::SetScrollerRatio(float contentLength,float containerLength)
{
	float oldScrollerRatio=this->scrollerRatio;

	this->scrollerRatio = (contentLength<containerLength) ? 1.0f : containerLength/contentLength;

	if(oldScrollerRatio!=this->scrollerRatio)
		SetScrollerPosition(this->scrollerPosition);
}

void GuiScrollBar::SetScrollerPosition(float positionPercent)
{
	float oldScrollerPosition=this->scrollerPosition;

	float scrollerContainerLength=this->GetContainerLength();
	float scrollerLength=this->GetScrollerLength();

	if(positionPercent+scrollerRatio>1)
		this->scrollerPosition=(scrollerContainerLength-scrollerLength)/scrollerContainerLength;
	else
		this->scrollerPosition = positionPercent < 0 ? 0 : positionPercent;
}

void GuiScrollBar::Scroll(Tab* tabContainer,float upOrDown)
{
	float rowHeightRatio=this->scrollerRatio/GuiSceneViewer::ROW_HEIGHT;

	float amount=this->scrollerPosition + (upOrDown<0 ? rowHeightRatio : -rowHeightRatio);

	this->SetScrollerPosition(amount);

	tabContainer->SetDraw(2,0,this->guiRect);
}

void GuiScrollBar::SetRect(GuiRect* iRect)
{

}

bool GuiScrollBar::IsVisible()
{
	return this->scrollerRatio<1.0f;
}

float GuiScrollBar::GetContainerLength()
{
	return	this->scrollbarType==SCROLLBAR_VERTICAL ? 
			this->rect.w-2.0f*SCROLLBAR_TIP_SIZE :
			this->rect.z-2.0f*SCROLLBAR_TIP_SIZE;
}
float GuiScrollBar::GetScrollerBegin()
{
	return this->GetContainerBegin()+this->scrollerPosition*this->GetContainerLength();
}
float GuiScrollBar::GetScrollerEnd()
{
	return this->GetScrollerBegin()+this->scrollerRatio*this->GetContainerLength();
}
float GuiScrollBar::GetScrollerLength()
{
	return this->GetScrollerEnd()-this->GetScrollerBegin();
}
float GuiScrollBar::GetContainerBegin()
{
	return	this->scrollbarType==SCROLLBAR_VERTICAL ? 
			this->rect.y+SCROLLBAR_TIP_SIZE :
			this->rect.x+SCROLLBAR_TIP_SIZE;
}
float GuiScrollBar::GetContainerEnd()
{
	return	this->scrollbarType==SCROLLBAR_VERTICAL ? 
			this->edges.w-SCROLLBAR_TIP_SIZE :
			this->edges.z-SCROLLBAR_TIP_SIZE;
}

void GuiScrollBar::OnLMouseUp(const GuiEvent& iMsg)
{
	GuiRect::OnLMouseUp(iMsg);

	this->scrollerPressed=-1;
}

void GuiScrollBar::OnLMouseDown(const GuiEvent& iMsg)
{
	if(!this->IsVisible())
		return;

	GuiRect::OnLMouseDown(iMsg);

	if(!this->hovering || scrollerRatio==1.0f)
		return;

	vec2& mpos=*(vec2*)iMsg.data;

	float& tTreshold=this->scrollbarType==SCROLLBAR_VERTICAL ? mpos.y : mpos.x;

	if(tTreshold<this->GetContainerBegin())
	{
		this->Scroll(iMsg.tab,1);
	}
	else if(tTreshold<this->GetContainerEnd())
	{
		this->scrollerPressed=0;

		if(tTreshold>=this->GetScrollerBegin() && tTreshold<=this->GetScrollerEnd())
			this->scrollerPressed=((tTreshold-this->GetScrollerBegin())/this->GetScrollerLength())*this->scrollerRatio;
		else
			SetScrollerPosition((tTreshold-this->GetContainerBegin())/this->GetContainerLength());
	}
	else
	{
		this->Scroll(iMsg.tab,-1);
	}

	iMsg.tab->SetDraw(2,0,this);
	iMsg.tab->SetDraw(2,0,this->guiRect);
}

void GuiScrollBar::OnMouseMove(const GuiEvent& iMsg)
{
	if(!this->IsVisible())
		return;

	GuiRect::OnMouseMove(iMsg);

	if(this->hovering && this->scrollerPressed>=0)
	{
		vec2& mpos=*(vec2*)iMsg.data;

		float& tTreshold=this->scrollbarType==SCROLLBAR_VERTICAL ? mpos.y : mpos.x;

		if(tTreshold>this->GetContainerBegin() && tTreshold<this->GetContainerEnd())
		{
			float mouseContainerTreshold=(tTreshold-this->GetContainerBegin())/this->GetContainerLength();

			this->SetScrollerPosition(mouseContainerTreshold-this->scrollerPressed);

			iMsg.tab->SetDraw(2,0,this);
			iMsg.tab->SetDraw(2,0,this->guiRect);
		}
	}
	else
		this->BroadcastToChilds(&GuiRect::OnMouseMove,iMsg);
}

void GuiScrollBar::OnPaint(const GuiEvent& iMsg)
{
	if(this->scrollerRatio==1.0f)
		return;

	this->BeginClip(iMsg.tab);

	if(this->scrollbarType==SCROLLBAR_VERTICAL)
	{
		iMsg.tab->renderer2D->DrawRectangle(this->rect.x,this->rect.y,this->rect.x+SCROLLBAR_TICK,this->rect.y+SCROLLBAR_TIP_SIZE,GuiRect::COLOR_BACK);
		iMsg.tab->renderer2D->DrawRectangle(this->rect.x,this->edges.w-SCROLLBAR_TIP_SIZE,this->rect.x+SCROLLBAR_TICK,this->edges.w,GuiRect::COLOR_BACK);

		iMsg.tab->renderer2D->DrawBitmap(iMsg.tab->iconUp,this->rect.x,this->rect.y,this->rect.x+SCROLLBAR_TICK,this->rect.y+SCROLLBAR_TIP_SIZE);
		iMsg.tab->renderer2D->DrawBitmap(iMsg.tab->iconDown,this->rect.x,this->edges.w-SCROLLBAR_TIP_SIZE,this->rect.x+SCROLLBAR_TICK,this->edges.w);

		iMsg.tab->renderer2D->DrawRectangle(this->rect.x,this->GetContainerBegin(),this->rect.x+SCROLLBAR_TICK,this->GetContainerEnd(),GuiRect::COLOR_BACK);

		iMsg.tab->renderer2D->DrawRectangle(this->rect.x,this->GetScrollerBegin(),this->rect.x+SCROLLBAR_TICK,this->GetScrollerEnd(),0x00000000);
	}
	else
	{
		iMsg.tab->renderer2D->DrawRectangle(this->rect.x,this->rect.y,this->rect.x+SCROLLBAR_TICK,this->rect.y+SCROLLBAR_TIP_SIZE,GuiRect::COLOR_BACK);
		iMsg.tab->renderer2D->DrawRectangle(this->edges.z-SCROLLBAR_TIP_SIZE,this->rect.y,this->edges.z,this->edges.w,GuiRect::COLOR_BACK);

		iMsg.tab->renderer2D->DrawBitmap(iMsg.tab->iconLeft,this->rect.x,this->rect.y,this->rect.x+SCROLLBAR_TICK,this->rect.y+SCROLLBAR_TIP_SIZE);
		iMsg.tab->renderer2D->DrawBitmap(iMsg.tab->iconRight,this->edges.z-SCROLLBAR_TIP_SIZE,this->rect.y,this->edges.z,this->rect.y+this->rect.w);

		iMsg.tab->renderer2D->DrawRectangle(this->GetContainerBegin(),this->rect.y,this->GetContainerEnd(),this->rect.y+SCROLLBAR_TICK,GuiRect::COLOR_BACK);

		iMsg.tab->renderer2D->DrawRectangle(this->GetScrollerBegin(),this->rect.y,this->GetScrollerEnd(),this->rect.y+SCROLLBAR_TICK,0x00000000);
	}

	this->BroadcastToChilds(&GuiRect::OnPaint,iMsg);
	this->EndClip(iMsg.tab);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////////GuiPanel/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiPanel::GuiPanel()
{
	this->name=L"GuiPanel";
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////GuiSceneViewer/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////




GuiSceneViewer* GuiSceneViewer::GetInstance()
{
	if(!::GlobalGuiSceneViewerInstance())
		::GlobalGuiSceneViewerInstance()=new GuiSceneViewer;

	return ::GlobalGuiSceneViewerInstance();
}

bool GuiSceneViewer::IsInstanced()
{
	return ::GlobalGuiSceneViewerInstance() ? true : false; 
}

GuiSceneViewer::GuiSceneViewer():entityRoot((EditorEntity*&)scene.entityRoot)
{
	this->entityRoot=new EditorEntity;
	this->entityRoot->name=L"SceneRootEntity";
	this->entityRoot->expanded=true;

	this->entityRoot->sceneViewerRow.SetParent(this);
	this->entityRoot->sceneViewerRow.state=true;
	this->entityRoot->sceneViewerRow.offsets.make(-20,-20,-20,-20);

	this->name=L"SceneViewer";
	this->scene.name=L"Default Scene";
}

GuiSceneViewer::GuiSceneViewer(GuiSceneViewer const& gsv):entityRoot((EditorEntity*&)gsv.scene.entityRoot){}


GuiSceneViewer::~GuiSceneViewer()
{
	wprintf(L"destroying treeview %p\n",this);
}

void GuiSceneViewer::OnRMouseUp(const GuiEvent& iMsg)
{
	GuiRect::OnRMouseUp(iMsg);

	GuiContainer*	tSelectedRect=dynamic_cast<GuiContainer*>(iMsg.tab->GetFocus());

	EditorEntity* tEditorEntity=tSelectedRect ? (EditorEntity*)tSelectedRect->userData : 0;

	int tChosedMenuIndex=iMsg.tab->TrackGuiSceneViewerPopup(tSelectedRect ? true : false);

	switch(tChosedMenuIndex)
	{
	case 1:
		{
			EditorEntity* tNewEntity=new EditorEntity;

			tNewEntity->name=L"EntityPippo";

			tNewEntity->bbox.a.make(-1,-1,-1);
			tNewEntity->bbox.b.make(1,1,1);

			tNewEntity->OnPropertiesCreate();

			tNewEntity->sceneViewerRow.canEdit=true;

			tNewEntity->SetParent(tEditorEntity ? tEditorEntity : this->entityRoot);
		}
		break;
	case 2:tEditorEntity->parent->childs.erase(std::find(tEditorEntity->parent->childs.begin(),tEditorEntity->parent->childs.end(),tEditorEntity));break;
	case 3:tEditorEntity->CreateComponent<EditorLight>();break;
	case 4:tEditorEntity->CreateComponent<EditorMesh>();break;
	case 5:tEditorEntity->CreateComponent<EditorCamera>();break;
	case 14:tEditorEntity->CreateComponent<EditorScript>();break;
	}

	this->OnSize(iMsg);
	iMsg.tab->SetDraw(2,0,this);
}

void GuiSceneViewer::OnEntitiesChange(const GuiEvent& iMsg)
{
	EditorEntity* entity=(EditorEntity*)iMsg.data;

	if(entity)
	{
		entity->SetParent(this->entityRoot);

		if(this->active)
		{
			this->OnSize(iMsg);
			iMsg.tab->SetDraw(2,0,this);
		}
	}

	GuiRect::OnEntitiesChange(iMsg);
}

void GuiSceneViewer::OnEntitySelected(const GuiEvent& iMsg)
{
	GuiRect::OnEntitySelected(iMsg);

	EditorEntity* entity=(EditorEntity*)iMsg.data;

	/*if(entity)
	{
		if(this->selection.end()==std::find(this->selection.begin(),this->selection.end(),entity))
		{
			this->selection.clear();
			this->selection.push_back(entity);

			this->OnSize(tabContainer);

			entity->selected=true;

			entity->CreateComponent<EditorGizmo>();

			tabContainer->SetDraw(2,0,this);
		}
	}*/
}

void GuiSceneViewer::OnLMouseDown(const GuiEvent& iMsg)
{
	bool tWasHovering=this->Contains(this->edges,(vec2&)*(vec2*)iMsg.data);

	GuiScrollRect::OnLMouseDown(iMsg);

	if(tWasHovering)
	{
		GuiContainer*	tSelectedRect=dynamic_cast<GuiContainer*>(iMsg.tab->GetFocus());

		if(tSelectedRect==&this->entityRoot->sceneViewerRow)
			return;

		EditorEntity* tEditorEntity=tSelectedRect ? (EditorEntity*)tSelectedRect->userData : 0;

		if(!InputManager::keyboardInput.IsPressed(0x11/*VK_CONTROL*/))
			this->selection.clear();

		Ide::GetInstance()->mainAppWindow->mainContainer->BroadcastToTabs(&Tab::OnGuiEntitySelected,tEditorEntity);
		
		iMsg.tab->SetDraw(2,0,this);
	}
}

void GuiSceneViewer::OnKeyDown(const GuiEvent& iMsg)
{
	if(this==iMsg.tab->GetFocus())
	{
		if(InputManager::keyboardInput.IsPressed(0x11/*VK_CONTROL*/) && !InputManager::keyboardInput.IsPressed(0x12/*VK_ALT*/))
		{
			if(InputManager::keyboardInput.IsPressed('S'))
			{
				String tSaveFile=Ide::GetInstance()->folderProject + L"\\" + this->scene.name + Ide::GetInstance()->GetSceneExtension();
				this->Save(tSaveFile.c_str());
			}
		}
	}

	GuiScrollRect::OnKeyDown(iMsg);
}

void saveEntityRecursively(Entity* iEntity,FILE* iFile)
{
	int childsSize=iEntity->childs.size();
	int componentsSize=iEntity->components.size();
	int nameCount=iEntity->name.size();

	fwrite(&childsSize,sizeof(int),1,iFile);//4

	fwrite(iEntity->local,sizeof(float),16,iFile);//64
	fwrite(iEntity->world,sizeof(float),16,iFile);//64

	StringUtils::WriteWstring(iFile,iEntity->name);

	fwrite(iEntity->bbox.a,sizeof(float),3,iFile);//12
	fwrite(iEntity->bbox.b,sizeof(float),3,iFile);//12

	fwrite(&componentsSize,sizeof(int),1,iFile);//4

	for(std::list<EntityComponent*>::iterator iteratorComponent=iEntity->components.begin();iteratorComponent!=iEntity->components.end();iteratorComponent++)
	{
		if((*iteratorComponent)->is<Script>())
		{
			Script* tScript=(Script*)(*iteratorComponent);

			fwrite(&Serializer::Script,sizeof(int),1,iFile);//1

			StringUtils::WriteWstring(iFile,tScript->file);
		}
		else if((*iteratorComponent)->is<Line>())
		{
			Line* tLine=(Line*)(*iteratorComponent);

			fwrite(&Serializer::Line,sizeof(int),1,iFile);//1

			unsigned int tPointSize=tLine->points.size();

			fwrite(&tPointSize,sizeof(unsigned int),1,iFile);//1

			for(std::list<vec3>::iterator it=tLine->points.begin();it!=tLine->points.end();it++)
				fwrite(&(*it),sizeof(float)*3,1,iFile);
		}
		else if((*iteratorComponent)->is<Animation>())
		{
			Animation* tAnimation=(Animation*)(*iteratorComponent);

			fwrite(&Serializer::Animation,sizeof(int),1,iFile);//1

			//generate AnimationController ID if not exist

			if(tAnimation->animationControllerId<0)
			{
				if(tAnimation->animationController)
					tAnimation->animationController->SetId(EditorAnimationController::GetFreeId());
				else
					DEBUG_BREAK();
			}

			fwrite(&tAnimation->animationControllerId,sizeof(unsigned int),1,iFile);//1
			fwrite(&tAnimation->start,sizeof(float),1,iFile);//1
			fwrite(&tAnimation->end,sizeof(float),1,iFile);//1
			fwrite(&tAnimation->index,sizeof(int),1,iFile);//1

			unsigned int tAnimClipsSize=tAnimation->clips.size();

			fwrite(&tAnimClipsSize,sizeof(unsigned int),1,iFile);//1

			for(std::vector<AnimClip*>::iterator iterAnimClip=tAnimation->clips.begin();iterAnimClip!=tAnimation->clips.end();iterAnimClip++)
			{
				AnimClip* tAnimClip=*iterAnimClip;

				fwrite(&tAnimClip->start,sizeof(float),1,iFile);//1
				fwrite(&tAnimClip->end,sizeof(float),1,iFile);//1

				unsigned int tKeyCurveSize=tAnimClip->curves.size();

				fwrite(&tKeyCurveSize,sizeof(unsigned int),1,iFile);//1

				for(std::vector<KeyCurve*>::iterator iterKeyCurve=tAnimClip->curves.begin();iterKeyCurve!=tAnimClip->curves.end();iterKeyCurve++)
				{
					KeyCurve* tKeyCurve=*iterKeyCurve;

					fwrite(&tKeyCurve->channel,sizeof(unsigned int),1,iFile);//1
					fwrite(&tKeyCurve->start,sizeof(float),1,iFile);//1
					fwrite(&tKeyCurve->end,sizeof(float),1,iFile);//1

					unsigned int tKeyframeSize=tKeyCurve->frames.size();

					fwrite(&tKeyframeSize,sizeof(unsigned int),1,iFile);//1

					for(std::vector<Keyframe*>::iterator iterKeyFrame=tKeyCurve->frames.begin();iterKeyFrame!=tKeyCurve->frames.end();iterKeyFrame++)
					{
						Keyframe* tKeyframe=*iterKeyFrame;

						fwrite(&tKeyframe->time,sizeof(float),1,iFile);//1
						fwrite(&tKeyframe->value,sizeof(float),1,iFile);//1
					}
				}
			}
		}
		else if((*iteratorComponent)->is<AnimationController>())
		{
			AnimationController* tAnimationController=(AnimationController*)(*iteratorComponent);

			fwrite(&Serializer::AnimationController,sizeof(int),1,iFile);//1
			
			//generate AnimationController ID if not exist

			if(tAnimationController->id<0)
				tAnimationController->SetId(EditorAnimationController::GetFreeId());

			fwrite(&tAnimationController->id,sizeof(unsigned int),1,iFile);//1
			fwrite(&tAnimationController->speed,sizeof(float),1,iFile);//1
			fwrite(&tAnimationController->cursor,sizeof(float),1,iFile);//1
			fwrite(&tAnimationController->play,sizeof(bool),1,iFile);//1
			fwrite(&tAnimationController->looped,sizeof(bool),1,iFile);//1
			fwrite(&tAnimationController->start,sizeof(float),1,iFile);//1
			fwrite(&tAnimationController->end,sizeof(float),1,iFile);//1
			fwrite(&tAnimationController->framesPerSecond,sizeof(unsigned int),1,iFile);//1
			fwrite(&tAnimationController->frameTime,sizeof(unsigned int),1,iFile);//1
		}
		else if((*iteratorComponent)->is<Mesh>())
		{
			Mesh* tMesh=(Mesh*)(*iteratorComponent);

			fwrite(&Serializer::Mesh,sizeof(int),1,iFile);

			fwrite(&tMesh->ncontrolpoints,sizeof(unsigned int),1,iFile);
			fwrite(&tMesh->nvertexindices,sizeof(unsigned int),1,iFile);
			fwrite(&tMesh->ntexcoord,sizeof(unsigned int),1,iFile);
			fwrite(&tMesh->nnormals,sizeof(unsigned int),1,iFile);
			fwrite(&tMesh->npolygons,sizeof(unsigned int),1,iFile);
			fwrite(&tMesh->isCCW,sizeof(bool),1,iFile);

			for(int i=0;i<tMesh->ncontrolpoints;i++)
				fwrite(&tMesh->controlpoints[i],sizeof(float)*3,1,iFile);
			
			if(tMesh->nvertexindices)
				fwrite(&tMesh->vertexindices,sizeof(unsigned int)*tMesh->nvertexindices,1,iFile);

			for(int i=0;i<tMesh->ntexcoord;i++)
				fwrite(&tMesh->texcoord[i],sizeof(float)*2,1,iFile);

			for(int i=0;i<tMesh->nnormals;i++)
				fwrite(&tMesh->normals[i],sizeof(float)*3,1,iFile);
		}
		else if((*iteratorComponent)->is<Skin>())
		{
			Skin* tSkin=(Skin*)(*iteratorComponent);

			fwrite(&Serializer::Skin,sizeof(int),1,iFile);

			fwrite(&tSkin->nclusters,sizeof(unsigned int),1,iFile);
			fwrite(&tSkin->ntextures,sizeof(unsigned int),1,iFile);

			for(int clusterIdx=0;clusterIdx<tSkin->nclusters;clusterIdx++)
			{
				Cluster* tCluster=&tSkin->clusters[clusterIdx];

				fwrite(&tCluster->ninfluences,sizeof(unsigned int),1,iFile);
				fwrite(tCluster->offset,sizeof(float),16,iFile);

				for(int influenceIdx=0;influenceIdx<tCluster->ninfluences;influenceIdx++)
				{
					Influence* tInfluence=&tCluster->influences[influenceIdx];

					fwrite(&tInfluence->weight,sizeof(float),1,iFile);
					fwrite(&tInfluence->ncontrolpointindex,sizeof(unsigned int),1,iFile);
					fwrite(&tInfluence->controlpointindex,sizeof(unsigned int)*tInfluence->ncontrolpointindex,1,iFile);
				}
			}
		}
		else if((*iteratorComponent)->is<Bone>())
		{
			fwrite(&Serializer::Bone,sizeof(int),1,iFile);
		}
	}

	if(childsSize)
	{
		for(std::list<Entity*>::iterator eIter=iEntity->childs.begin();eIter!=iEntity->childs.end();eIter++)
			saveEntityRecursively(*eIter,iFile);
	}
}

EditorEntity* loadEntityRecursively(EditorEntity* iEditorEntityParent,FILE* iFile)
{
	int nameCount;
	int componentsSize;
	int childsSize;
	int componentCode;

	EditorEntity* tEditorEntity=new EditorEntity;

	tEditorEntity->SetParent(iEditorEntityParent);

	fread(&childsSize,sizeof(int),1,iFile);//4

	fread(tEditorEntity->local,sizeof(float),16,iFile);//64
	fread(tEditorEntity->world,sizeof(float),16,iFile);//64

	StringUtils::ReadWstring(iFile,tEditorEntity->name);

	fread(tEditorEntity->bbox.a,sizeof(float),3,iFile);//12
	fread(tEditorEntity->bbox.b,sizeof(float),3,iFile);//12

	tEditorEntity->OnResourcesCreate();
	tEditorEntity->OnPropertiesCreate();

	fread(&componentsSize,sizeof(int),1,iFile);//4

	for(int tComponentIndex=0;tComponentIndex<componentsSize;tComponentIndex++)
	{
		fread(&componentCode,sizeof(int),1,iFile);//1

		switch(componentCode)
		{
			case Serializer::Script:
			{
				EditorScript* tScript=tEditorEntity->CreateComponent<EditorScript>();
				StringUtils::ReadWstring(iFile,tScript->file);
			}
			break;
			case Serializer::Line:
			{
				EditorLine* tLine=tEditorEntity->CreateComponent<EditorLine>();

				unsigned int tPointSize;

				fread(&tPointSize,sizeof(unsigned int),1,iFile);//1

				for(int i=0;i<tPointSize;i++)
				{
					vec3 tPoint;
					fread(&tPoint,sizeof(float)*3,1,iFile);//1
					tLine->AddPoint(tPoint);
				}
			}
			break;
			case Serializer::Animation:
			{
				EditorAnimation* tAnimation=tEditorEntity->CreateComponent<EditorAnimation>();

				fread(&tAnimation->animationControllerId,sizeof(unsigned int),1,iFile);//1

				if(tAnimation->animationControllerId<0)
					DEBUG_BREAK();

				fread(&tAnimation->start,sizeof(float),1,iFile);//1
				fread(&tAnimation->end,sizeof(float),1,iFile);//1
				fread(&tAnimation->index,sizeof(int),1,iFile);//1

				unsigned int tAnimClipsSize;

				fread(&tAnimClipsSize,sizeof(unsigned int),1,iFile);//1

				for(int iterAnimClip=0;iterAnimClip<tAnimClipsSize;iterAnimClip++)
				{
					AnimClip* tAnimClip=new AnimClip;

					tAnimation->clips.push_back(tAnimClip);

					fread(&tAnimClip->start,sizeof(float),1,iFile);//1
					fread(&tAnimClip->end,sizeof(float),1,iFile);//1

					unsigned int tKeyCurveSize;

					fread(&tKeyCurveSize,sizeof(unsigned int),1,iFile);//1

					for(int iterKeyCurve=0;iterKeyCurve<tKeyCurveSize;iterKeyCurve++)
					{
						KeyCurve* tKeyCurve=new KeyCurve;

						tAnimClip->curves.push_back(tKeyCurve);

						fread(&tKeyCurve->channel,sizeof(unsigned int),1,iFile);//1
						fread(&tKeyCurve->start,sizeof(float),1,iFile);//1
						fread(&tKeyCurve->end,sizeof(float),1,iFile);//1

						unsigned int tKeyframeSize;

						fread(&tKeyframeSize,sizeof(unsigned int),1,iFile);//1

						for(int iterKeyFrame=0;iterKeyFrame<tKeyframeSize;iterKeyFrame++)
						{
							Keyframe* tKeyframe=new Keyframe;

							tKeyCurve->frames.push_back(tKeyframe);

							fread(&tKeyframe->time,sizeof(float),1,iFile);//1
							fread(&tKeyframe->value,sizeof(float),1,iFile);//1
						}
					}
				}

				EditorAnimationController* tEditorAnimationController=(EditorAnimationController*)EditorAnimationController::GetById(tAnimation->animationControllerId);

				if(!tEditorAnimationController)
					DEBUG_BREAK();
				else
					tEditorAnimationController->AddAnimation(tAnimation);
			}
			break;
			case Serializer::AnimationController:
			{
				EditorAnimationController* tAnimationController=tEditorEntity->CreateComponent<EditorAnimationController>();

				unsigned int tAnimControllerId;

				fread(&tAnimControllerId,sizeof(unsigned int),1,iFile);//1

				if(tAnimationController->id<0)
					DEBUG_BREAK();
				else
					tAnimationController->SetId(tAnimControllerId);

				fread(&tAnimationController->speed,sizeof(float),1,iFile);//1
				fread(&tAnimationController->cursor,sizeof(float),1,iFile);//1
				fread(&tAnimationController->play,sizeof(bool),1,iFile);//1
				fread(&tAnimationController->looped,sizeof(bool),1,iFile);//1
				fread(&tAnimationController->start,sizeof(float),1,iFile);//1
				fread(&tAnimationController->end,sizeof(float),1,iFile);//1
				fread(&tAnimationController->framesPerSecond,sizeof(unsigned int),1,iFile);//1
				fread(&tAnimationController->frameTime,sizeof(unsigned int),1,iFile);//1
			}
			break; 
			case Serializer::Mesh:
			{
				EditorMesh* tMesh=tEditorEntity->CreateComponent<EditorMesh>();

				fread(&tMesh->ncontrolpoints,sizeof(unsigned int),1,iFile);
				fread(&tMesh->nvertexindices,sizeof(unsigned int),1,iFile);
				fread(&tMesh->ntexcoord,sizeof(unsigned int),1,iFile);
				fread(&tMesh->nnormals,sizeof(unsigned int),1,iFile);
				fread(&tMesh->npolygons,sizeof(unsigned int),1,iFile);
				fread(&tMesh->isCCW,sizeof(bool),1,iFile);

				tMesh->controlpoints=new float[tMesh->ncontrolpoints][3];
				
				if(tMesh->nvertexindices)
					tMesh->vertexindices=new unsigned int[tMesh->nvertexindices];
				
				tMesh->texcoord=new float[tMesh->ntexcoord][2];
				tMesh->normals=new float[tMesh->nnormals][3];

				for(int i=0;i<tMesh->ncontrolpoints;i++)
					fread(&tMesh->controlpoints[i],sizeof(float)*3,1,iFile);

				if(tMesh->nvertexindices)
					fread(&tMesh->vertexindices,sizeof(unsigned int)*tMesh->nvertexindices,1,iFile);

				for(int i=0;i<tMesh->ntexcoord;i++)
					fread(&tMesh->texcoord[i],sizeof(float)*2,1,iFile);

				for(int i=0;i<tMesh->nnormals;i++)
					fread(&tMesh->normals[i],sizeof(float)*3,1,iFile);
			}
			break;
		}
	}

	if(childsSize)
	{
		for(int i=0;i<childsSize;i++)
			loadEntityRecursively(tEditorEntity,iFile);
	}

	return tEditorEntity;
}


/*fseek(iFile,sizeof(unsigned char),SEEK_SET);

				unsigned int tMaterialSize;

				fread(&tMaterialSize,sizeof(unsigned char),1,iFile);
				

				for(int i=0;i<tMaterialSize;i++)
				{
					Material* tMaterial=new Material;

					tMesh->materials.push_back(tMaterial);

					fread(&tMaterial->femissive,sizeof(float),1,iFile);
					fread(&tMaterial->emissive,sizeof(float)*3,1,iFile);

					fread(&tMaterial->fambient,sizeof(float),1,iFile);
					fread(&tMaterial->ambient,sizeof(float)*3,1,iFile);

					fread(&tMaterial->fdiffuse,sizeof(float),1,iFile);
					fread(&tMaterial->diffuse,sizeof(float)*3,1,iFile);

					fread(&tMaterial->normalmap,sizeof(float)*3,1,iFile);

					fread(&tMaterial->fbump,sizeof(float),1,iFile);
					fread(&tMaterial->bump,sizeof(float)*3,1,iFile);

					fread(&tMaterial->ftransparent,sizeof(float),1,iFile);
					fread(&tMaterial->transparent,sizeof(float)*3,1,iFile);

					fread(&tMaterial->fdisplacement,sizeof(float),1,iFile);
					fread(&tMaterial->displacement,sizeof(float)*3,1,iFile);

					fread(&tMaterial->fspecular,sizeof(float),1,iFile);
					fread(&tMaterial->specular,sizeof(float)*3,1,iFile);

					fread(&tMaterial->fshininess,sizeof(float),1,iFile);

					fread(&tMaterial->freflection,sizeof(float),1,iFile);
					fread(&tMaterial->reflection,sizeof(float)*3,1,iFile);

					for(int t=0;t<tMaterial->textures.size();t++)
					{
						//save the texture

					}
				}*/

void GuiSceneViewer::Save(String iFilename)
{
	File tScriptFile(iFilename);

	if(tScriptFile.Open(L"wb"))
	{
		saveEntityRecursively(this->entityRoot,tScriptFile);
		tScriptFile.Close();
	}
}

void GuiSceneViewer::Load(String iFilename)
{
	File tScriptFile(iFilename);

	if(tScriptFile.Open(L"rb"))
	{
		wprintf(L"begin load project\n");

		for(std::list<Entity*>::iterator i=this->entityRoot->childs.begin();i!=this->entityRoot->childs.end();)
			i=this->entityRoot->childs.erase(i);

		EditorEntity* tEntity=loadEntityRecursively(0,tScriptFile);

		std::list<Entity*> tChildsCopy=tEntity->childs;

		for(std::list<Entity*>::iterator i=tChildsCopy.begin();i!=tChildsCopy.end();i++)
			(*i)->SetParent(this->entityRoot);

		SAFEDELETE(tEntity);

		if(this->active)
		{
			Tab* tab=this->GetRootRect()->tab;

			this->OnSize(GuiEvent(tab,this,0,&GuiRect::OnSize,0));
			tab->SetDraw(2,0,this);
		}

		tScriptFile.Close();

		wprintf(L"end load project\n");
	}
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////GuiEntityViewer/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiEntityViewer::GuiEntityViewer():
entity(0),tabContainer(0)
{
	this->name=L"EntityViewer";
};

GuiEntityViewer::~GuiEntityViewer()
{
	wprintf(L"destroying properties %p\n",this);
}

void GuiEntityViewer::OnEntitySelected(const GuiEvent& iMsg)
{
	EditorEntity* iEntity=(EditorEntity*)iMsg.data;

	GuiEvent tMsg(iMsg.tab,this);

	if(this->entity!=iEntity)
	{
		if(this->entity)
		{
			this->BroadcastToChilds(&GuiRect::OnDeactivate,tMsg);
			this->entity->entityViewerContainer.SetParent(0);
			this->entity=0;
		}

		if(iEntity)
		{
			this->entity=iEntity;
			this->entity->entityViewerContainer.SetParent(this);

			this->vScrollbar.SetScrollerPosition(0);
			this->hScrollbar.SetScrollerPosition(0);

			this->OnSize(GuiEvent(iMsg.tab,this));
			this->entity->entityViewerContainer.OnActivate(tMsg);
		}
		else
		{
			this->OnSize(tMsg);
		}
		
		iMsg.tab->SetDraw(2,0,this);
	}
	else
	{
		this->OnSize(tMsg);

		iMsg.tab->SetDraw(2,0,this);
	}

	GuiRect::OnEntitySelected(iMsg);
}

void GuiEntityViewer::OnExpandos(const GuiEvent& iMsg)
{
	this->OnSize(iMsg);
}

void GuiEntityViewer::OnActivate(const GuiEvent& iMsg)
{
	tabContainer=iMsg.tab;

	GuiRect::OnActivate(iMsg);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////GuiConsoleViewer////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
GuiConsoleViewer::GuiConsoleViewer(){}
GuiConsoleViewer::~GuiConsoleViewer(){}

GuiConsoleViewer* GuiConsoleViewer::GetInstance()
{
	if(!::GlobalGuiConsoleViewerInstance())
		::GlobalGuiConsoleViewerInstance()=new GuiConsoleViewer;

	return ::GlobalGuiConsoleViewerInstance();
}

bool GuiConsoleViewer::IsInstanced()
{
	return ::GlobalGuiConsoleViewerInstance() ? true : false; 
}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////GuiProjectViewer////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////



extern ResourceNodeDir* rootProjectDirectory;

GuiProjectViewer* GuiProjectViewer::GetInstance()
{
	if(!::GlobalGuiProjectViewerInstance())
		::GlobalGuiProjectViewerInstance()=new GuiProjectViewer;

	return ::GlobalGuiProjectViewerInstance();
}

bool GuiProjectViewer::IsInstanced()
{
	return ::GlobalGuiProjectViewerInstance() ? true : false; 
}

GuiProjectViewer::GuiProjectViewer(GuiProjectViewer const&):projectDirectory(rootProjectDirectory){}

GuiProjectViewer::GuiProjectViewer():
	projectDirectory(rootProjectDirectory),
	splitterLeftActive(false),
	splitterRightActive(false),
	splitterLeft(0),
	splitterRight(0)
{
	projectDirectory=new ResourceNodeDir;

	this->name=L"ProjectViewer";

	this->dirViewer.projectViewer=this;
	this->fileViewer.projectViewer=this;
	this->resViewer.projectViewer=this;

	const float tHalfSplit=2;

	this->dirViewer.SetParent(this);
	this->dirViewer.SetEdges(0,0,&this->splitterLeft);
	this->dirViewer.offsets.make(0,0,-tHalfSplit,0);

	this->fileViewer.SetParent(this);
	this->fileViewer.SetEdges(&this->splitterLeft,0,&this->splitterRight);
	this->fileViewer.offsets.make(tHalfSplit,0,-tHalfSplit,0);

	this->resViewer.SetParent(this);
	this->resViewer.SetEdges(&this->splitterRight);
	this->resViewer.offsets.make(tHalfSplit,0,0,0);

	this->projectDirectory->fileName=Ide::GetInstance()->folderProject;
	this->projectDirectory->expanded=true;
	this->projectDirectory->isDir=true;

	dirViewer.projectDirectory=this->projectDirectory;
	fileViewer.currentDirectory=this->projectDirectory;

	dirViewer.selectedDirs.push_back(this->projectDirectory);

	this->projectDirectory->directoryViewerRow.SetStringMode(this->projectDirectory->fileName,true);
	this->projectDirectory->directoryViewerRow.offsets.make(-20,0,0,0);
	this->projectDirectory->directoryViewerRow.state=true;
	this->projectDirectory->directoryViewerRow.rowData=this->projectDirectory;

	projectDirectory->directoryViewerRow.SetParent(&this->dirViewer);
}

GuiProjectViewer::~GuiProjectViewer()
{
	wprintf(L"destroying resources %p\n",this);
}



void GuiProjectViewer::OnActivate(const GuiEvent& iMsg)
{
	if(!this->active)
	{
		Ide::GetInstance()->ScanDir(this->projectDirectory->fileName,this->projectDirectory);

		this->fileViewer.SetDirectory(this->projectDirectory);

		this->OnSize(GuiEvent(iMsg.tab,this));
	}

	GuiRect::OnActivate(iMsg);
}

void GuiProjectViewer::OnDeactivate(const GuiEvent& iMsg)
{
	if(this->active)
	{
		for(std::list<ResourceNode*>::iterator tFile=this->projectDirectory->files.begin();tFile!=this->projectDirectory->files.end();tFile++)
			SAFEDELETE(*tFile);

		for(std::list<ResourceNodeDir*>::iterator tDir=this->projectDirectory->dirs.begin();tDir!=this->projectDirectory->dirs.end();tDir++)
			SAFEDELETE(*tDir);

		this->projectDirectory->files.clear();
		this->projectDirectory->dirs.clear();
	}

	GuiRect::OnDeactivate(iMsg);
}


void GuiProjectViewer::OnLMouseDown(const GuiEvent& iMsg)
{
	GuiRect::OnLMouseDown(iMsg);

	if(this->hovering)
	{
		vec2& mpos=*(vec2*)iMsg.data;

		if(mpos.x>this->dirViewer.edges.z && mpos.x<this->fileViewer.edges.x)
		{
			this->splitterLeftActive=true;
		}
		else if(mpos.x>this->fileViewer.edges.z && mpos.x<this->resViewer.edges.x)
		{
			this->splitterRightActive=true;
		}
	}
}

void GuiProjectViewer::OnLMouseUp(const GuiEvent& iMsg)
{
	GuiRect::OnLMouseUp(iMsg);

	if(this->pressing || this->splitterLeftActive || this->splitterRightActive)
	{
		this->splitterLeftActive=false;
		this->splitterRightActive=false;
	}
}

void GuiProjectViewer::OnMouseMove(const GuiEvent& iMsg)
{
	if(this->hovering || this->splitterLeftActive || this->splitterRightActive)
	{
		iMsg.tab->SetCursor(1);

		if(this->pressing)
		{
			vec2& mpos=*(vec2*)iMsg.data;

			if(this->splitterLeftActive)
			{
				this->splitterLeft=mpos.x;

				this->dirViewer.OnSize(iMsg);
				this->fileViewer.OnSize(iMsg);
			}
			if(this->splitterRightActive)
			{
				this->splitterRight=mpos.x;

				this->fileViewer.OnSize(iMsg);
				this->resViewer.OnSize(iMsg);
			}

			iMsg.tab->SetDraw(1,0,0);
		}
	}

	GuiRect::OnMouseMove(iMsg);
}

void GuiProjectViewer::OnReparent(const GuiEvent& iMsg)
{
	GuiRect::OnReparent(iMsg);

	iMsg.tab->OnGuiSize();
	iMsg.tab->OnGuiRecreateTarget();
}


void GuiProjectViewer::OnPaint(const GuiEvent& iMsg)
{
	this->BeginClip(iMsg.tab);
	iMsg.tab->renderer2D->DrawRectangle(this->rect,Tab::COLOR_BACK,true);

	this->BroadcastToChilds(&GuiRect::OnPaint,iMsg);
	this->EndClip(iMsg.tab);
}

void GuiProjectViewer::OnSize(const GuiEvent& iMsg)
{
	GuiRect::OnSize(iMsg);

	float tPanelsSizeWidth=this->rect.z/3.0f - 2*4;

	this->splitterLeft=tPanelsSizeWidth;
	this->splitterRight=tPanelsSizeWidth*2;

	this->dirViewer.OnSize(iMsg);
	this->fileViewer.OnSize(iMsg);
	this->resViewer.OnSize(iMsg);
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

////////////////////////////////////////////
////////////////////////////////////////////
/////////GuiProjectViewer::DirViewer////////
////////////////////////////////////////////
////////////////////////////////////////////

void GuiProjectViewer::DirViewer::OnLMouseDown(const GuiEvent& iMsg)
{
	GuiScrollRect::OnLMouseDown(iMsg);

	if(this->Contains(this->edges,*(vec2*)iMsg.data))
	{
		GuiContainerRow<ResourceNodeDir*>* tDirectoryRowPressed=dynamic_cast< GuiContainerRow<ResourceNodeDir*>* >(iMsg.tab->GetFocus());

		if(tDirectoryRowPressed)
		{
			this->projectViewer->fileViewer.SetDirectory(tDirectoryRowPressed->rowData);

			this->projectViewer->fileViewer.OnSize(iMsg);
			iMsg.tab->SetDraw(2,0,&this->projectViewer->fileViewer);
		}
	}
}




/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
///////////////GuiProjectViewer::FileViewer/////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void GuiProjectViewer::FileViewer::SetDirectory(ResourceNodeDir* iResourceNodeDir)
{
	this->currentDirectory=iResourceNodeDir;

	this->DestroyChilds();

	if(this->currentDirectory)
	{
		GuiContainerRow<ResourceNode*>* tCurrentDirectory=this->ContainerRow<ResourceNode*>(this->currentDirectory);

		tCurrentDirectory->offsets.make(-20,-20,0,0);
		tCurrentDirectory->state=true;

		GuiContainerRow<ResourceNode*>* tRow=0;

		for(std::list<ResourceNodeDir*>::iterator it=this->currentDirectory->dirs.begin();it!=this->currentDirectory->dirs.end();it++)
		{
			tRow=tCurrentDirectory->ContainerRow<ResourceNode*>(*it);
			tRow->SetStringMode(tRow->rowData->fileName,true);
		}
		for(std::list<ResourceNode*>::iterator it=this->currentDirectory->files.begin();it!=this->currentDirectory->files.end();it++)
		{
			tRow=tCurrentDirectory->ContainerRow<ResourceNode*>(*it);
			tRow->SetStringMode(tRow->rowData->fileName,true);
		}
	}
}

void GuiProjectViewer::FileViewer::OnLMouseDown(const GuiEvent& iMsg)
{
	GuiScrollRect::OnLMouseDown(iMsg);
}
void GuiProjectViewer::FileViewer::OnRMouseUp(const GuiEvent& iMsg)
{
	GuiScrollRect::OnRMouseUp(iMsg);

	if(this->Contains(this->edges,*(vec2*)iMsg.data))
	{
		GuiContainerRow<ResourceNode*>* tHovered=dynamic_cast< GuiContainerRow<ResourceNode*>* >(iMsg.tab->GetHover());

		if(tHovered)
		{
			int menuResult=iMsg.tab->TrackProjectFileViewerPopup(tHovered->rowData);

			switch(menuResult)
			{
			case 1:
				if(tHovered->rowData && tHovered->rowData->parent)
				{
					ResourceNodeDir* parentDirectory=(ResourceNodeDir*)tHovered->rowData->parent;

					String tFileNameBase=tHovered->rowData->parent->fileName + L"\\" + tHovered->rowData->fileName;
					String tFileNameBaseExtended=tFileNameBase + Ide::GetInstance()->GetEntityExtension();

					//first remove from the list

					if(tHovered->rowData->isDir)
					{
						parentDirectory->dirs.remove((ResourceNodeDir*)tHovered->rowData);

						Ide::GetInstance()->subsystem->Execute(parentDirectory->fileName.c_str(),L"rd /S /Q " + tHovered->rowData->fileName);
					}
					else
					{
						parentDirectory->files.remove(tHovered->rowData);

						File::Delete(tFileNameBase.c_str());
						File::Delete(tFileNameBaseExtended.c_str());
					}

					SAFEDELETE(tHovered->rowData);
				}
				break;
			case 3://load
				if(tHovered->rowData->fileName.PointedExtension() == Ide::GetInstance()->GetSceneExtension())
				{
					(*GuiViewport::GetPool().begin())->renderDrawInstance->skip=true;
					std::vector<GuiSceneViewer*> tGuiSceneViewers;

					Ide::GetInstance()->mainAppWindow->GetTabRects<GuiSceneViewer>(tGuiSceneViewers);

					if(tGuiSceneViewers.size())
					{
						String tHoveredNodeFilename=tHovered->rowData->parent->fileName + L"\\" + tHovered->rowData->fileName;

						tGuiSceneViewers[0]->Load(tHoveredNodeFilename.c_str());
						tGuiSceneViewers[0]->GetRootRect()->tab->SetDraw(2,0,tGuiSceneViewers[0]);
					}

					(*GuiViewport::GetPool().begin())->renderDrawInstance->skip=false;
				}
				break;
			}

			this->OnSize(iMsg);
			iMsg.tab->SetDraw(2,0,this);
		}
	}
}



void GuiProjectViewer::FileViewer::OnDLMouseDown(const GuiEvent& iMsg)
{
	GuiScrollRect::OnDLMouseDown(iMsg);
	/*if(this==iMsg.tab->GetFocus())
	{
		vec2& mpos=*(vec2*)iMsg.data;

		vec2 tDrawCanvas(0,0);

		bool tHoveredResourceNodeExpandedPressed=false;
		ResourceNode* tHoveredResourceNode=this->GetHoveredRow(this->rootResource,mpos,tDrawCanvas,tHoveredResourceNodeExpandedPressed);

		if(tHoveredResourceNode)
		{
			if(!tHoveredResourceNode->isDir)
			{
				String tExtension=tHoveredResourceNode->fileName.Extension();
				String tFilename=tHoveredResourceNode->parent->fileName + L"\\" + tHoveredResourceNode->fileName;

				if(tExtension == &Ide::GetInstance()->GetSceneExtension()[1])
				{
					Thread* renderThread=GuiViewport::GetPool()[0]->GetRootRect()->tabContainer->threadRender;
					Task* drawTask=GuiViewport::GetPool()[0]->GetRootRect()->tabContainer->taskDraw;

					drawTask->Block(true);

					GuiSceneViewer* guiSceneViewer=iMsg.tab->parentWindowContainer->SpawnViewer<GuiSceneViewer>();
					guiSceneViewer->Load(tFilename);

					drawTask->Block(false);

					//guiSceneViewer->GetRootRect()->tabContainer->SetDraw(2,0,guiSceneViewer);
				}
				else if(tExtension==L"cpp")
				{
					/ *GuiScriptViewer* guiScriptViewer=tabContainer->parentWindowContainer->SpawnViewer<GuiScriptViewer>();
					guiScriptViewer->Open(tFilename);
					guiScriptViewer->GetRootRect()->tabContainer->SetDraw(2,0,guiScriptViewer);* /
				}
			}
			else
			{
				ResourceNodeDir* tResourceNodeDir=(ResourceNodeDir*)tHoveredResourceNode;

				DirViewer& tDirViewer=this->projectViewer->dirViewer;

				tDirViewer.UnselectNodes(tDirViewer.rootResource);

				if(!tResourceNodeDir->selectedLeft)
					tResourceNodeDir->selectedLeft=true;

				tDirViewer.CalcNodesHeight(tDirViewer.rootResource);
				tDirViewer.OnSize(iMsg);

				this->rootResource=tResourceNodeDir;

				this->CalcNodesHeight(this->rootResource);
				this->OnSize(iMsg);

				iMsg.tab->SetDraw(2,0,&tDirViewer);
				iMsg.tab->SetDraw(2,0,this);
			}
		}
	}

	GuiRect::OnDLMouseDown(iMsg);*/
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////DrawInstance/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


DrawInstance::DrawInstance(int iNoneAllRect,bool iFrame,GuiRect* iRect,String iName,bool iRemove,bool iSkip):code(iNoneAllRect),frame(iFrame),rect(iRect),name(iName),remove(iRemove),skip(iSkip){}

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////GuiPaper///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

void GuiScriptViewer::DrawBreakpoints(Tab* tabContainer)
{
	float tFontHeight=GuiFont::GetDefaultFont()->GetHeight();

	std::vector<Debugger::Breakpoint>& breakpoints=Ide::GetInstance()->debugger->breakpointSet;

	for(size_t i=0;i<breakpoints.size();i++)
	{
		if(breakpoints[i].script==this->script)
		{
			unsigned int tLineInsertion=this->rect.y + (breakpoints[i].line - 1) * tFontHeight;

			unsigned int tBreakColor = breakpoints[i].breaked ? 0xff0000 : 0xffff00 ;

			tabContainer->renderer2D->DrawRectangle(this->rect.x + 1,tLineInsertion + 1,this->rect.x+this->editor.margins.x -1,tLineInsertion + tFontHeight - 1,tBreakColor,true);
		}
	}
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////GuiScriptViewer/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
std::vector<GuiScriptViewer*>& GuiScriptViewer::GetInstances()
{
	return GlobalScriptViewers();
}


GuiScriptViewer::GuiScriptViewer():
	script(0),
	lineNumbers(true)
{
	this->GetInstances().push_back(this);

	this->name=L"ScriptViewer";

	this->editor.font=GuiFont::GetFontPool()[3];

	this->lines.textAlign.make(0,0);
	this->lines.textSpot.make(0,0);

	this->lines.SetParent(this);
	this->editor.SetParent(this);

	this->lines.SetEdges(0,0,&this->lines.edges.x,0);
	this->lines.offsets.x=0;

	this->editor.SetEdges(&this->lines.edges.z,0,0,0);

}

GuiScriptViewer::~GuiScriptViewer()
{
	this->GetInstances().erase(std::find(this->GetInstances().begin(),this->GetInstances().end(),this));
}

void GuiScriptViewer::Open(Script* iScript)
{
	this->script=(EditorScript*)iScript;

	this->editor.text=StringUtils::ReadCharFile(iScript->file);

	this->script->scriptViewer=this;
}

bool GuiScriptViewer::Save()
{
	if(this->script)
	{
		File tScriptFile(this->script->file);

		if(tScriptFile.Open(L"wb"))
		{
			tScriptFile.Write((void*)this->editor.text->c_str(),sizeof(wchar_t),this->editor.text->size());

			tScriptFile.Close();

			return true;
		}
		else
			DEBUG_BREAK();
	}

	return false;
}


bool GuiScriptViewer::Compile()
{
	bool exited=false;
	bool compiled=false;
	bool runned=false;

	if(this->script)
	{
		exited=Ide::GetInstance()->compiler->UnloadScript(this->script);

		compiled=Ide::GetInstance()->compiler->Compile(this->script);

		runned=Ide::GetInstance()->compiler->LoadScript(this->script);
	}

	return exited && compiled && runned;
}



void GuiScriptViewer::OnKeyDown(const GuiEvent& iMsg)
{
	if(this->script)
	{
		if(InputManager::keyboardInput.IsPressed(0x11/*VK_CONTROL*/) && !InputManager::keyboardInput.IsPressed(0x12/*VK_ALT*/))
		{
			if(InputManager::keyboardInput.IsPressed('S'))
				this->Save();
		}
	}

	GuiRect::OnKeyDown(iMsg);
}

void GuiScriptViewer::OnKeyUp(const GuiEvent& iMsg)
{
	GuiRect::OnKeyUp(iMsg);
}

void GuiScriptViewer::OnLMouseDown(const GuiEvent& iMsg)
{
	GuiRect::OnLMouseDown(iMsg);

	vec2 tMpos=*(vec2*)iMsg.data;

	if(tMpos.x < this->editor.margins.x)
	{
		unsigned int tBreakOnLine=(tMpos.y-this->rect.y)/this->editor.font->GetHeight() + 1;

		EditorScript* tEditorScript=(EditorScript*)this->script;

		std::vector<Debugger::Breakpoint>& tAvailableBreakpoints=Ide::GetInstance()->debugger->allAvailableBreakpoints;
		std::vector<Debugger::Breakpoint>& tBreakpoints=Ide::GetInstance()->debugger->breakpointSet;

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

				Ide::GetInstance()->debugger->SetBreakpoint(tAvailableBreakpoints[i],tAdd);

				iMsg.tab->SetDraw(2,false,this);

				break;
			}
		}
	}

	iMsg.tab->SetFocus(&this->editor);
}

void GuiScriptViewer::OnDeactivate(const GuiEvent& iMsg)
{
	iMsg.tab->SetFocus(0);

	GuiRect::OnDeactivate(iMsg);
}

void GuiScriptViewer::OnMouseMove(const GuiEvent& iMsg)
{
	GuiRect::OnMouseMove(iMsg);

	if(this->hovering)
	{
		vec2 tMpos=*(vec2*)iMsg.data;

		if(tMpos.x < this->editor.margins.x)
		{

		}
		else
		{

		}
	}
}

void GuiScriptViewer::OnSize(const GuiEvent& iMsg)
{
	GuiScrollRect::OnSize(iMsg);

	this->lineCount=this->CountScriptLines()-1;
}


int GuiScriptViewer::CountScriptLines()
{
	const wchar_t* t=this->editor.text->c_str();

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

GuiCompilerViewer* GuiCompilerViewer::GetInstance()
{
	if(!::GlobalGuiCompilerViewerInstance())
		::GlobalGuiCompilerViewerInstance()=new GuiCompilerViewer;

	return ::GlobalGuiCompilerViewerInstance();
}

bool GuiCompilerViewer::IsInstanced()
{
	return ::GlobalGuiCompilerViewerInstance() ? true : false; 
}

GuiCompilerViewer::GuiCompilerViewer(){this->name=L"CompilerViewer";}
GuiCompilerViewer::~GuiCompilerViewer(){}

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

	this->vScrollbar.SetParent(0);

	this->DestroyChilds();

	GuiRect* messagesGlue=new GuiRect;
	messagesGlue->SetParent(this);

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
				tCompilerMessageRow->text=String(LineBegin,LineEnd-LineBegin);
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

				tCompilerMessageRow->text=String(FileEnd,ErrorEnd-FileEnd)+String(ErrorEnd,LineEnd-ErrorEnd);
				tCompilerMessageRow->colorBackground=0xff0000;
			}

			tCompilerMessageRow->SetEdges(0,&messagesGlue->edges.w);

			tCompilerMessageRow->SetParent(this);

			LineBegin=++LineEnd;
		}

		this->vScrollbar.SetParent(this);

		//this->contentHeight=messagesGlue->childs.size()*MESSAHE_ROW_HEIGHT;
	}

	return bReturnValue;
}


void GuiCompilerViewer::OnSize(const GuiEvent& iMsg)
{
	/*this->vScrollbar.SetScrollerRatio(this->contentHeight,this->rect.w);

	if(this->childs.size()==2)
		//we don't need the GuiScrollRect::width cause the newly GuiRect::offset
		this->vScrollbar.IsVisible() ? this->childs[0]->offsets.z=-GuiScrollBar::SCROLLBAR_TICK : this->childs[0]->offsets.z=0;*/

	GuiRect::OnSize(iMsg);

	/*this->rect.z=this->rect.z;*/
}


///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////Properties////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////



void EditorEntity::OnPropertiesCreate()
{
	//fill entityViewer properties

	std::vector<GuiRect*> tRootLevel(2);

	tRootLevel[0]=this->container=this->entityViewerContainer.Container(L"Entity");

	GuiProperty<GuiString>* tNameLabel;

	tNameLabel=tRootLevel[0]->Property(L"Name",new GuiString(&this->name));
	tNameLabel->property->canEdit=true;

	tRootLevel[0]->Property(L"Ptr",new GuiString(this,StringValue::PTR));
	tRootLevel[0]->Property(L"Position",new GuiString(&this->world,StringValue::MAT4POS));
	tRootLevel[0]->Property(L"Childs",new GuiString(&this->Entity::childs,StringValue::ENTITYLISTSIZE));
	
	tRootLevel[1]=tRootLevel[0]->Container(L"AABB");

	tRootLevel[1]->Property(L"Min",new GuiString(this->bbox.a,StringValue::VEC3));
	tRootLevel[1]->Property(L"Max",new GuiString(this->bbox.b,StringValue::VEC3));
	tRootLevel[1]->Property(L"Volume",new GuiString(this->bbox.a,StringValue::VEC3,this->bbox.b));
}

void EditorEntity::OnPropertiesUpdate(Tab* tab)
{
	for(std::list<EntityComponent*>::iterator it=this->components.begin();it!=this->components.end();it++)
	{
		EditorObjectBase* componentProperties=dynamic_cast<EditorObjectBase*>(*it);

		if(componentProperties)
			componentProperties->OnPropertiesUpdate(tab);
	}
}

void EditorMesh::OnPropertiesCreate()
{
	EditorEntity* tEditorEntity=(EditorEntity*)this->entity;

	this->container=tEditorEntity->entityViewerContainer.Container(L"Mesh");

	this->container->Property(L"Controlpoints",new GuiString(&this->ncontrolpoints,StringValue::INT));
	this->container->Property(L"Normals",new GuiString(&this->nnormals,StringValue::INT));
	this->container->Property(L"Polygons",new GuiString(&this->npolygons,StringValue::INT));
	this->container->Property(L"Texcoord",new GuiString(&this->ntexcoord,StringValue::INT));
	this->container->Property(L"Vertexindices",new GuiString(&this->nvertexindices,StringValue::INT));
}

void EditorMesh::OnPropertiesUpdate(Tab* tab)
{
}

void EditorSkin::OnPropertiesCreate()
{
	EditorEntity* tEditorEntity=(EditorEntity*)this->entity;

	this->container=tEditorEntity->entityViewerContainer.Container(L"Skin");

	this->container->Property(L"Clusters",new GuiString(&this->nclusters,StringValue::INT));
	this->container->Property(L"Textures",new GuiString(&this->ntextures,StringValue::INT));
}
void EditorSkin::OnPropertiesUpdate(Tab* tab)
{
}
void EditorRoot::OnPropertiesCreate()
{
	EditorEntity* tEditorEntity=(EditorEntity*)this->entity;

	this->container=tEditorEntity->entityViewerContainer.Container(L"Root");
}
void EditorRoot::OnPropertiesUpdate(Tab* tab)
{
}
void EditorSkeleton::OnPropertiesCreate()
{
	EditorEntity* tEditorEntity=(EditorEntity*)this->entity;

	this->container=tEditorEntity->entityViewerContainer.Container(L"Skeleton");
}
void EditorSkeleton::OnPropertiesUpdate(Tab* tab)
{
}
void EditorGizmo::OnPropertiesCreate()
{
	EditorEntity* tEditorEntity=(EditorEntity*)this->entity;

	this->container=tEditorEntity->entityViewerContainer.Container(L"Gizmo");
}
void EditorGizmo::OnPropertiesUpdate(Tab* tab)
{
}
void EditorAnimation::OnPropertiesCreate()
{
	EditorEntity* tEditorEntity=(EditorEntity*)this->entity;

	this->container=tEditorEntity->entityViewerContainer.Container(L"Animation");

	this->container->Property(L"IsBone",new GuiString(this,StringValue::ISBONECOMPONENT));
	this->container->Property(L"Duration",new GuiString(&this->start,StringValue::FLOAT2MINUSFLOAT1,&this->end));
	this->container->Property(L"Begin",new GuiString(&this->start,StringValue::FLOAT));
	this->container->Property(L"End",new GuiString(&this->end,StringValue::FLOAT));
}
void EditorAnimation::OnPropertiesUpdate(Tab* tab)
{
}

EditorAnimationController::EditorAnimationController():minSpeed(0),maxSpeed(2.0f){}
EditorAnimationController::~EditorAnimationController(){}

void EditorAnimationController::OnPropertiesCreate()
{
	EditorEntity* tEditorEntity=(EditorEntity*)this->entity;

	this->container=tEditorEntity->entityViewerContainer.Container(L"AnimationController");

	this->container->name=L"EditorAnimationControllerProperties";
	this->container->text=L"AnimationController";
	this->container->Property(L"Number of nodes",new GuiString(&this->animations,StringValue::ANIMATIONVECSIZE));
	this->container->Property(L"Velocity",new GuiSlider(&this->speed,&this->minSpeed,&this->maxSpeed));
	this->container->Property(L"Duration",new GuiString(&this->start,StringValue::FLOAT2MINUSFLOAT1,&this->end));
	this->container->Property(L"Begin",new GuiString(&this->start,StringValue::FLOAT));
	this->container->Property(L"End",new GuiString(&this->end,StringValue::FLOAT));
	guiAnimationController=new GuiAnimationController(*this);
	this->container->Insert(guiAnimationController);
}

void EditorAnimationController::OnPropertiesUpdate(Tab* tab)
{
	if(this->oldCursor!=this->cursor)
		tab->SetDraw(2,0,guiAnimationController->slider);

	this->oldCursor=this->cursor;
}

void EditorLine::OnPropertiesCreate()
{
	EditorEntity* tEditorEntity=(EditorEntity*)this->entity;

	this->container=tEditorEntity->entityViewerContainer.Container(L"Line");
	this->container->Property(L"Number of Segments",new GuiString(&this->points,StringValue::VEC3LISTSIZE));

	this->pointListBox=new GuiListBox;

	for(std::list<vec3>::iterator i=this->points.begin();i!=this->points.end();i++)
		this->pointListBox->AddItem(*i,StringValue::VEC3);

	this->container->Insert(this->pointListBox);
}
void EditorLine::OnPropertiesUpdate(Tab* tab)
{
}

void EditorLine::DestroyPoints()
{
	this->pointListBox->DestroyItems();
	this->points.clear();
}
void EditorLine::AddPoint(vec3 iPoint)
{
	this->points.push_back(iPoint);
	vec3* tVec3=&this->points.back();
	this->pointListBox->AddItem(tVec3,StringValue::VEC3);
}


void EditorBone::OnPropertiesCreate()
{
	EditorEntity* tEditorEntity=(EditorEntity*)this->entity;

	this->container=tEditorEntity->entityViewerContainer.Container(L"Bone");
}
void EditorBone::OnPropertiesUpdate(Tab* tab)
{
}
void EditorLight::OnPropertiesCreate()
{
	EditorEntity* tEditorEntity=(EditorEntity*)this->entity;

	this->container=tEditorEntity->entityViewerContainer.Container(L"Light");
}
void EditorLight::OnPropertiesUpdate(Tab* tab)
{
}

void editScriptEditorCallback(void* iData)
{
	EditorScript* editorScript=(EditorScript*)iData;

	Tab* tabContainer=Ide::GetInstance()->mainAppWindow->mainContainer->tabs[0];

	if(!tabContainer)
		DEBUG_BREAK();

	std::vector<GuiScriptViewer*>& tGuiScriptViewerInstances=GuiScriptViewer::GetInstances();

	if(tGuiScriptViewerInstances.empty())
		tabContainer->rects.childs.push_back(new GuiScriptViewer);

	GuiScriptViewer* guiScriptViewer= tGuiScriptViewerInstances[0];

	if(guiScriptViewer)
	{
		guiScriptViewer->Open(editorScript);
		guiScriptViewer->OnSize(GuiEvent(tabContainer));
		tabContainer->SetDraw(0,1);
	}
}

void compileScriptEditorCallback(void* iData)
{
	EditorScript* editorScript=(EditorScript*)iData;

    Ide::GetInstance()->compiler->Compile(editorScript);
}

void launchStopScriptEditorCallback(void* iData)
{
	EditorScript* editorScript=(EditorScript*)iData;

	if(editorScript->runtime)
	{
		if(Ide::GetInstance()->compiler->UnloadScript(editorScript))
			editorScript->buttonLaunch->text=L"Launch";
	}
	else
	{
		if(Ide::GetInstance()->compiler->LoadScript(editorScript))
			editorScript->buttonLaunch->text=L"Stop";
	}

	editorScript->container->GetRootRect()->tab->SetDraw(2,0,editorScript->buttonLaunch);
}

EditorScript::EditorScript():scriptViewer(0)
{

};

void EditorScript::OnPropertiesCreate()
{
	EditorEntity* tEditorEntity=(EditorEntity*)this->entity;

	this->container=tEditorEntity->entityViewerContainer.Container(L"Script");

	GuiProperty<GuiString>* tFile=container->Property(L"File",new GuiString(this->Script::file));
	GuiProperty<GuiString>* tRunning=container->Property(L"Running",new GuiString(&this->Script::runtime,StringValue::BOOLPTR));

	GuiButton* buttonEdit=new GuiButton;
	buttonEdit->name=L"EditorScript Edit Button";
	buttonEdit->func=editScriptEditorCallback;
	buttonEdit->param=this;
	buttonEdit->text=L"Edit";
	buttonEdit->fixed.make(0,0,0,20);
	container->Insert(buttonEdit);

	GuiButton* buttonCompile=new GuiButton;
	buttonCompile->func=compileScriptEditorCallback;
	buttonCompile->param=this;
	buttonCompile->text=L"Compile";
	buttonCompile->fixed.make(0,0,0,20);
	container->Insert(buttonCompile);

	this->buttonLaunch=new GuiButton;
	this->buttonLaunch->func=launchStopScriptEditorCallback;
	this->buttonLaunch->param=this;
	this->buttonLaunch->text=L"Launch";
	this->buttonLaunch->fixed.make(0,0,0,20);
	container->Insert(this->buttonLaunch);
}
void EditorScript::OnPropertiesUpdate(Tab* tab)
{
}

void EditorScript::OnResourcesCreate()
{
	String tFileNamePath=Ide::GetInstance()->folderProject + L"\\" + this->entity->name;
	this->file=tFileNamePath + L".cpp";

	if(!File::Exist(this->file))
	{
		if(!File::Create(this->file))
			DEBUG_BREAK();

		StringUtils::WriteCharFile(
									this->file,
									L"#include \"entities.h\"\n\nstruct " + this->entity->name + L"_ : EntityScript\n{\n\t int counter;\n\tvoid init()\n\t{\n\t\tcounter=0;\n\tthis->entity->local.identity();\n\t\tprintf(\"inited\\n\");\n\t}\n\n\tvoid update()\n\t{\n\t\tthis->entity->local.translate(0.1f,0,0);\n\t//printf(\"counter: %d\\n\",counter);\n\tcounter++;\n\t}\n\n\tvoid deinit()\n\t{\n\t\tprintf(\"deinited\\n\");\n\t}\n\n};\n",
									L"wb"
								   );
	}
}

void EditorCamera::OnPropertiesCreate()
{
	EditorEntity* tEditorEntity=(EditorEntity*)this->entity;

	this->container=tEditorEntity->entityViewerContainer.Container(L"Camera");
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

PluginSystem::Plugin::Plugin():loaded(false),name(L"Plugin"){}
PluginSystem::PluginSystem():modalTab(0){}
PluginSystem::~PluginSystem(){}

namespace PluginSystemUtils
{
	void ptfCloseConfigurationPanel(void* tData)
	{
		PluginSystem* tPluginSystem=(PluginSystem*)tData;

		tPluginSystem->modalTab->Destroy();
		Ide::GetInstance()->mainAppWindow->mainContainer->windowData->Enable(true);
	}

	void ptfLoadUnloadConfigurationPanelRowButton(void* tData)
	{
		GuiButton* tButtonFunc=(GuiButton*)tData;

		PluginSystem::Plugin* tPLugin=(PluginSystem::Plugin*)tButtonFunc->userData;

		tPLugin->loaded ? tPLugin->Unload() : tPLugin->Load();

		tButtonFunc->colorBackground=tButtonFunc->colorChecked = tPLugin->loaded==false ? 0xff0000 : 0x00ff00;
	}
}

void PluginSystem::ShowConfigurationPanel()
{
	Container* tContainer=Ide::GetInstance()->mainAppWindow->mainContainer;

	tContainer->windowData->Enable(false);

	vec2 tIdeFrameSize=tContainer->windowData->Size();
	vec2 tTabSize(500,300);
	vec2 tTabPos=tContainer->windowData->Pos();

	tTabPos.x+=tIdeFrameSize.x/2.0f-tTabSize.x/2.0f;
	tTabPos.y+=tIdeFrameSize.y/2.0f-tTabSize.y/2.0f;

	this->modalTab=tContainer->CreateModalTab(tTabPos.x,tTabPos.y,tTabSize.x,tTabSize.y);

	GuiPanel* tPanel=this->modalTab->CreateViewer<GuiPanel>();

	tPanel->offsets.w=-30;

	tPanel->name=L"Plugins";

	GuiRect* tLast=0;

	int even=0x101010;
	int odd=0x151515;
	
	for(size_t i=0;i<this->plugins.size();i++)
	{
		Plugin* tPlugin=this->plugins[i];

		GuiString* tString=tPanel->Text(L"");

		unsigned int tColor=GuiRect::COLOR_BACK + ((i%2)==0 ? even : odd);

		tString->SetColors(tColor,tColor,tColor,tColor);
		tString->margins.x=10;
		tString->SetStringMode(this->plugins[i]->name,true);

		//set on/off button

		GuiButton* tLoadButton=new GuiButton;
		tLoadButton->colorBackground = tPlugin->loaded ? 0x00ff00 : 0xff0000;
		tLoadButton->colorChecked = tPlugin->loaded ? 0x00ff00 : 0xff0000;
		tLoadButton->func=PluginSystemUtils::ptfLoadUnloadConfigurationPanelRowButton;
		tLoadButton->param=tLoadButton;
		tLoadButton->userData=this->plugins[i];

		tLoadButton->SetEdges(&tString->edges.z,0,&tString->edges.z,0);
		tLoadButton->offsets.make(-15,5,-5,-5);

		tLoadButton->SetParent(tString);

		//insert plugin row

		if(tLast && tPanel->childs.size())
			tString->SetEdges(0,&tLast->edges.w,0,0);

		tLast=tString;
	}

	this->exitButton=new GuiButton;
	this->exitButton->text=L"Exit";
	this->exitButton->SetEdges(&tPanel->edges.z,&tPanel->edges.w,&tPanel->edges.z,&tPanel->edges.w);
	this->exitButton->offsets.make(-35,5,-5,25);

	this->exitButton->colorBackground=0x888888;
	this->exitButton->colorHovering=0x989898;
	this->exitButton->colorPressing=0xa8a8a8;

	this->exitButton->func=PluginSystemUtils::ptfCloseConfigurationPanel;
	this->exitButton->param=this;

	this->exitButton->SetParent(&this->modalTab->rectsLayered);

	this->modalTab->OnGuiActivate();
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
		Ide::GetInstance()->pluginSystem->ShowConfigurationPanel();
	}
	else if(iIdx==MenuActionProgramInfo)
	{

	}

}


void MainContainer::BroadcastToTabs(void (Tab::*func)(void*),void* data)
{
	for(std::vector<Container*>::iterator tContainer=this->containers.begin();tContainer!=this->containers.end();tContainer++)
		(*tContainer)->BroadcastToTabs(func,data);
}

void MainContainer::BroadcastToSelectedTabRects(void (GuiRect::*func)(const GuiEvent&),void* iData)
{
	for(std::vector<Container*>::iterator tContainer=this->containers.begin();tContainer!=this->containers.end();tContainer++)
		(*tContainer)->BroadcastToSelectedTabRects(func,iData);
}


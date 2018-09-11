#include "win32.h"

struct DLLBUILD AndroidPlugin : PluginSystem::Plugin
{
	int MenuActionBuild;

	Tab* configurationPanel;

	String Apkname;
	String Keyname;

	String AndroidSdkDir;
	String AndroidPlatform;
	String AndroidBuildTool;
	String AndroidDebugBridge;

	String AndroidOutputDirectory;
	String AndroidProjectDirectory;
	String AndroidProjectJniDirectory;
	String AndroidProjectAssetDirectory;
	String AndroidProjectResDirectory;

	String AndroidProjectLibsDirectory;

	GuiButtonFunc* exitButton;
	GuiButtonFunc* buildButton;

	AndroidPlugin();

	void Load();
	void Unload();
	void OnMenuPressed(int);

	void ShowConfigurationPanel();
};

struct DLLBUILD CompilerAndroid : CompilerWin32
{
	bool Compile();
};


extern "C" DLLBUILD PluginSystem::Plugin* GetPlugin(PluginSystem*);
extern "C" DLLBUILD void DestroyPlugin();

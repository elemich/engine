#include "interfaces.h"

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

	GuiButton* exitButton;
	GuiButton* buildButton;

	GuiProperty<GuiPath>* sdkDirProperty;
	GuiProperty<GuiComboBox>* sdkPlatform;
	GuiProperty<GuiComboBox>* sdkBuildtool;

	AndroidPlugin();

	void Load();
	void Unload();
	void OnMenuPressed(int);

	void ShowConfigurationPanel();
};


extern "C" DLLBUILD PluginSystem::Plugin* GetPlugin(PluginSystem*);
extern "C" DLLBUILD void DestroyPlugin();

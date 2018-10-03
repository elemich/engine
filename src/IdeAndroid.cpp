#include "IdeAndroid.h"


AndroidPlugin* globalAllocatedAndroidPlugin=0;

namespace AndroidPluginGlobalDefaults
{
	String defaultApkName=L"Engine";
	String defaultApkKeyName=L"EngineKey";

	String defaultSdkPlatformDirectory=L"platforms";
	String defaultSdkBuildtoolsDirectory=L"build-tools";
}

namespace AndroidPluginGlobalFunctions
{
	GuiProperty<GuiComboBox>* globalAndroidPluginCreateComboBoxProperty(GuiPanel* iPanel,String iLabel,String* iRef,int iColor);
	GuiProperty<GuiPath>*	 globalAndroidPluginCreatePathProperty(GuiPanel* iPanel,String iLabel,String* iRef,int iColor);
	void				 globalAndroidPluginCreateTextBoxProperty(GuiPanel* iPanel,String iLabel,String* iRef,int iColor);
	void				 globalAndroidPluginFindPlatformDirectories(void* iData);

	void globalPackResources(String& iCurrentDirectory,ResourceNodeDir* iResDir,File& iPackFile,File& iTableFile,String& iAndroidTargetDirectory,std::vector<String>& iCollectSources);

	void globalCompileButton(void* tData);
	void globalExitButton(void* tData);

	void globalCompileAndroidApk(AndroidPlugin*);
}


AndroidPlugin::AndroidPlugin():exitButton(0),buildButton(0),sdkDirProperty(0),sdkPlatform(0),sdkBuildtool(0)
{
	this->name=L"AndroidBuilder";

	this->Apkname=L"Engine";
	this->Keyname=L"EngineKey";
}

void AndroidPlugin::Load()
{
	if(!this->loaded)
	{
		this->MenuActionBuild=this->Menu(L"Build\\Android",false);

		this->loaded=true;
	}
}

void AndroidPlugin::Unload()
{
	if(this->loaded)
	{
	}
}

void AndroidPlugin::OnMenuPressed(int iIdx)
{
	if(iIdx==this->MenuActionBuild)
		this->ShowConfigurationPanel();
}





void AndroidPlugin::ShowConfigurationPanel()
{
	Container* tContainer=Ide::GetInstance()->mainAppWindow->mainContainer;

	tContainer->windowData->Enable(false);

	vec2 tIdeFrameSize=tContainer->windowData->Size();
	vec2 tTabSize(500,300);
	vec2 tTabPos=tContainer->windowData->Pos();

	tTabPos.x+=tIdeFrameSize.x/2.0f-tTabSize.x/2.0f;
	tTabPos.y+=tIdeFrameSize.y/2.0f-tTabSize.y/2.0f;

	this->configurationPanel=tContainer->CreateModalTab(tTabPos.x,tTabPos.y,tTabSize.x,tTabSize.y);

	GuiPanel* tPanel=this->configurationPanel->CreateViewer<GuiPanel>();
	tPanel->offsets.w=-30;

	tPanel->name=L"Android Builder";

	int even=0x101010;
	int odd=0x151515;

	AndroidPluginGlobalFunctions::globalAndroidPluginCreateTextBoxProperty(tPanel,L"Apk Name",&this->Apkname,even);															
	AndroidPluginGlobalFunctions::globalAndroidPluginCreateTextBoxProperty(tPanel,L"Key Name",&this->Keyname,odd);
	this->sdkDirProperty=AndroidPluginGlobalFunctions::globalAndroidPluginCreatePathProperty(tPanel,L"SdkDir",&this->AndroidSdkDir,even);
	this->sdkPlatform=AndroidPluginGlobalFunctions::globalAndroidPluginCreateComboBoxProperty(tPanel,L"Platform",&this->AndroidPlatform,odd);
	this->sdkBuildtool=AndroidPluginGlobalFunctions::globalAndroidPluginCreateComboBoxProperty(tPanel,L"Build Tools",&this->AndroidBuildTool,even);
	AndroidPluginGlobalFunctions::globalAndroidPluginCreatePathProperty(tPanel,L"ADB",&this->AndroidDebugBridge,odd);
	AndroidPluginGlobalFunctions::globalAndroidPluginCreatePathProperty(tPanel,L"Output Dir",&this->AndroidOutputDirectory,even);

	sdkDirProperty->property->func=AndroidPluginGlobalFunctions::globalAndroidPluginFindPlatformDirectories;
	sdkDirProperty->property->param=this;

	//buttons

	this->exitButton=new GuiButton;
	this->exitButton->text=L"Exit";
	this->exitButton->SetEdges(&tPanel->edges.z,&tPanel->edges.w,&tPanel->edges.z,&tPanel->edges.w);
	this->exitButton->offsets.make(-35,5,-5,25);

	this->exitButton->colorBackground=0x888888;
	this->exitButton->colorHovering=0x989898;
	this->exitButton->colorPressing=0xa8a8a8;

	this->exitButton->func=AndroidPluginGlobalFunctions::globalExitButton;

	this->exitButton->SetParent(&this->configurationPanel->rectsLayered);

	//button build

	this->buildButton=new GuiButton;
	this->buildButton->text=L"Build";
	this->buildButton->SetEdges(&tPanel->edges.z,&tPanel->edges.w,&tPanel->edges.z,&tPanel->edges.w);
	this->buildButton->offsets.make(-75,5,-40,25);

	this->buildButton->colorBackground=0x888888;
	this->buildButton->colorHovering=0x989898;
	this->buildButton->colorPressing=0xa8a8a8;

	this->buildButton->func=AndroidPluginGlobalFunctions::globalCompileButton;

	this->buildButton->SetParent(&this->configurationPanel->rectsLayered);

	this->configurationPanel->OnGuiActivate();
}


PluginSystem::Plugin* GetPlugin(PluginSystem*)
{
	globalAllocatedAndroidPlugin=new AndroidPlugin;
	return globalAllocatedAndroidPlugin;
}
void DestroyPlugin()
{
	SAFEDELETE(globalAllocatedAndroidPlugin);
}





void AndroidPluginGlobalFunctions::globalCompileAndroidApk(AndroidPlugin* iAndroidPlugin)
{
	String& tApkname=iAndroidPlugin->Apkname;
	String& tKeyname=iAndroidPlugin->Keyname;

	String& tAndroidSdkDir=iAndroidPlugin->AndroidSdkDir;
	String& tAndroidDebugBridge=iAndroidPlugin->AndroidDebugBridge;

	String& tAndroidOutputDirectory=iAndroidPlugin->AndroidOutputDirectory;

	String  tAndroidPlatform=tAndroidSdkDir + L"\\" + AndroidPluginGlobalDefaults::defaultSdkPlatformDirectory + L"\\" + iAndroidPlugin->AndroidPlatform;
	String  tAndroidBuildTool=tAndroidSdkDir + L"\\" + AndroidPluginGlobalDefaults::defaultSdkBuildtoolsDirectory + L"\\" + iAndroidPlugin->AndroidBuildTool;

	String  tAndroidProjectDirectory=tAndroidOutputDirectory + L"\\project";
	String  tAndroidProjectJniDirectory=tAndroidProjectDirectory + L"\\jni";
	String  tAndroidProjectAssetDirectory=tAndroidProjectDirectory + L"\\assets";
	String  tAndroidProjectResDirectory=tAndroidProjectDirectory + L"\\res";

	String tAndroidProjectLibsDirectory=tAndroidProjectDirectory + L"\\libs\\armeabi-v7a";

	std::vector<String>		        tSourcePaths;
	std::vector<String>				tSourceFilesContent;
	int								tPackFileSize=0;
	int								tTableFileSize=0;
	String							tResourceDirectory=L"\\";

	{
		Subsystem* tSubsystem=Ide::GetInstance()->subsystem;

		//create output directory

		tSubsystem->CreateDirectory(tAndroidOutputDirectory);

		//create android project directory

		tSubsystem->CreateDirectory(tAndroidProjectDirectory);

		//create android asset directory

		tSubsystem->CreateDirectory(tAndroidProjectAssetDirectory);

		//create android jni project directory

		tSubsystem->CreateDirectory(tAndroidProjectJniDirectory);

		//create android res project directory

		tSubsystem->CreateDirectory(tAndroidProjectResDirectory);
		tSubsystem->CreateDirectory(tAndroidProjectResDirectory + L"\\values");
	}

	//pack data and table

	File tPackFile(tAndroidProjectAssetDirectory + L"\\pack.pck");
	File tTableFile(tAndroidProjectAssetDirectory + L"\\ptbl.txt");

	tPackFile.Open(L"wb");
	tTableFile.Open(L"wb");

	if(tPackFile.IsOpen() && tTableFile.IsOpen())
	{
		//pack non-sources resources

		std::vector<GuiProjectViewer*> tGuiProjectViewer;

		Ide::GetInstance()->mainAppWindow->GetTabRects<GuiProjectViewer>(tGuiProjectViewer);

		AndroidPluginGlobalFunctions::globalPackResources(tResourceDirectory,tGuiProjectViewer[0]->projectDirectory,tPackFile,tTableFile,tAndroidProjectDirectory,tSourcePaths);

		tPackFile.Close();
		tTableFile.Close();
	}

	//build sources

	String tAndroidMk=L"LOCAL_PATH := $(call my-dir)\n\n";

	for(std::vector<String>::iterator si=tSourcePaths.begin();si!=tSourcePaths.end();si++)
	{
		tAndroidMk+=L"include $(CLEAR_VARS)\n"
			L"DSTDIR := " + tAndroidProjectDirectory + L"\n"
			L"LOCAL_C_INCLUDES := " + Ide::GetInstance()->compiler->ideSrcPath + L"\n"
			//"LOCAL_STATIC_LIBRARIES := -lEngine\n"
			L"LOCAL_MODULE := " + ((FilePath)(*si)).Name() + L"\n"
			L"LOCAL_SRC_FILES := " + (*si) + L"\n"
			L"LOCAL_CPPFLAGS := -std=gnu++0x -Wall -fPIE -fpic\n"
			L"LOCAL_LDLIBS := -L" + Ide::GetInstance()->compiler->ideLibPath + L" -lEngine -llog\n"
			L"include $(BUILD_SHARED_LIBRARY)\n\n";
	}

	StringUtils::WriteCharFile(tAndroidProjectJniDirectory + L"\\Android.mk",tAndroidMk);

	StringUtils::WriteCharFile(tAndroidProjectJniDirectory + L"\\Application.mk",L"APP_STL:=c++_static\nAPP_ABI:=armeabi-v7a\nAPP_CPPFLAGS := -frtti\nAPP_OPTIM := debug\n");
	StringUtils::WriteCharFile(tAndroidProjectDirectory + L"\\project.properties",L"target=android-23\n");
	StringUtils::WriteCharFile(tAndroidProjectDirectory + L"\\local.properties",L"sdk.dir=C:\\Sdk\\android\\android-sdk\n");

	String tStringsXml= L"<resources>\n"
		L"\t<string name=\"Engine_activity\">Engine</string>\n"
		L"</resources>\n";

	StringUtils::WriteCharFile(tAndroidProjectResDirectory + L"\\values\\strings.xml",tStringsXml);

	String tBuildXml=	L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		L"<project name=\"EngineActivity\" default=\"help\">\n\n"
		L"\t<property file=\"local.properties\" />\n"
		L"\t<property file=\"ant.properties\" />\n\n"
		L"\t<property environment=\"env\" />\n"
		L"\t<condition property=\"sdk.dir\" value=\"${env.ANDROID_HOME}\">\n"
		L"\t\t<isset property=\"env.ANDROID_HOME\" />\n"
		L"\t</condition>\n\n"
		L"\t<loadproperties srcFile=\"project.properties\" />\n\n"
		L"\t<import file=\"custom_rules.xml\" optional=\"true\" />\n\n"
		L"\t<import file=\"${sdk.dir}/tools/ant/build.xml\" />\n\n"
		L"</project>\n";

	StringUtils::WriteCharFile(tAndroidProjectDirectory + L"\\build.xml",tBuildXml);

	String tManifestXml=    L"<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\"\n"
		L"\tpackage=\"com.android.Engine\">\n\n"
		L"\t<uses-permission android:name=\"android.permission.WRITE_EXTERNAL_STORAGE\"/>\n"
		L"\t<uses-permission android:name=\"android.permission.READ_EXTERNAL_STORAGE\"/>\n\n"
		L"\t<application\n"
		L"\t\t\tandroid:label=\"@string/Engine_activity\"\n"
		L"\t\t\tandroid:debuggable=\"true\">\n\n"
		L"\t\t<activity android:name=\".EngineActivity\"\n"
		L"\t\t\tandroid:theme=\"@android:style/Theme.NoTitleBar.Fullscreen\"\n"
		L"\t\t\tandroid:launchMode=\"singleTask\"\n"
		L"\t\t\tandroid:configChanges=\"orientation|keyboardHidden\">\n\n"
		L"\t\t\t<intent-filter>\n"
		L"\t\t\t\t<action android:name=\"android.intent.action.MAIN\" />\n"
		L"\t\t\t\t<category android:name=\"android.intent.category.LAUNCHER\" />\n"
		L"\t\t\t</intent-filter>\n\n"
		L"\t\t</activity>\n"
		L"\t</application>\n\n"
		L"\t<uses-feature android:glEsVersion=\"0x00020000\"/>\n"
		L"\t<uses-sdk android:minSdkVersion=\"15\"/>\n\n"
		L"</manifest>\n";

	StringUtils::WriteCharFile(tAndroidProjectDirectory + L"\\AndroidManifest.xml",tManifestXml);

	//backup original source and append exports

	for(std::vector<String>::iterator si=tSourcePaths.begin();si!=tSourcePaths.end();si++)
	{
		//backup
		tSourceFilesContent.push_back(StringUtils::ReadCharFile(*si));

		//append
		StringUtils::WriteCharFile(
			*si,
			L"\n\nextern \"C\" __attribute__ ((dllexport)) EntityScript* Create(){return new " + ((FilePath)*si).Name() + L"_;}",
			L"ab"
			);

		StringUtils::WriteCharFile(
			*si,
			L"\n\nextern \"C\" __attribute__ ((dllexport)) void Destroy(EntityScript* iDestroy){SAFEDELETE(iDestroy);}",
			L"ab"
			);
	}

	//build the native code

	Ide::GetInstance()->subsystem->Execute(tAndroidProjectDirectory,L"C:\\Sdk\\android\\android-ndk-r16b\\ndk-build",tAndroidProjectDirectory + L"\\ndk-build-log.txt",true,true,true);

	//unroll exports (rewrite original script file)

	int tSourceFilesContentIdx=0;
	for(std::vector<String>::iterator si=tSourcePaths.begin();si!=tSourcePaths.end();si++,tSourceFilesContentIdx++)
		StringUtils::WriteCharFile(*si,tSourceFilesContent[tSourceFilesContentIdx],L"wb");

	//write asset

	tPackFileSize=tPackFile.Size();
	tTableFileSize=tTableFile.Size();

	File tAssetFile(tAndroidProjectAssetDirectory + L"\\asset.mp3");

	tAssetFile.Open(L"wb");
	tPackFile.Open(L"rb");
	tTableFile.Open(L"rb");

	if(tAssetFile.IsOpen() && tPackFile.IsOpen() && tTableFile.IsOpen())
	{
		fwrite(&tTableFileSize,sizeof(int),1,tAssetFile.data);
		fwrite(&tPackFileSize,sizeof(int),1,tAssetFile.data);

		unsigned char* tTableData=new unsigned char[tTableFileSize];
		fread(tTableData,tTableFileSize,1,tTableFile.data);
		fwrite(tTableData,tTableFileSize,1,tAssetFile.data);
		SAFEDELETEARRAY(tTableData);

		unsigned char* tPackData=new unsigned char[tPackFileSize];
		fread(tPackData,tPackFileSize,1,tPackFile.data);
		fwrite(tPackData,tPackFileSize,1,tAssetFile.data);
		SAFEDELETEARRAY(tPackData);

		tAssetFile.Close();
		tPackFile.Close();
		tTableFile.Close();
	}

	//File::Delete(tPackFile.path.c_str());
	//File::Delete(tTableFile.path.c_str());

	//paths in the apk must have the / separator otherwise LoadLibrary will not find the native so lib
	//aapt add will register all file path, so call from current dir to avoid full path recording

	String tBuildApk=   L"@echo off\n"
		L"set PLATFORM=" + tAndroidPlatform + L"\n"
		L"set BUILDTOOL=" + tAndroidBuildTool + L"\n"
		L"set LIBDIR=" + Ide::GetInstance()->compiler->ideLibPath + L"\n"
		L"set PROJDIR=" + tAndroidProjectDirectory + L"\n"
		L"set ADB=" + tAndroidDebugBridge + L"\n"
		L"set KEYSTORE=" + tKeyname + L"\n"
		L"SET APP_name=L" + tApkname + L"\n"
		L"set PREVDIR=%cd%\n\n"

		L"SET ANDROID_AAPT_ADD=%BUILDTOOL%\\aapt.exe add\n"
		L"SET ANDROID_AAPT_PACK=%BUILDTOOL%\\aapt.exe package -v -f -I %PLATFORM%\\android.jar\n\n"

		L"if exist \"%PROJDIR%\\lib\" rmdir \"%PROJDIR%\\lib\" /S /Q\n"
		L"mkdir \"%PROJDIR%\\lib\"\n"
		L"mkdir \"%PROJDIR%\\lib\\armeabi-v7a\"\n\n"

		L"copy \"%LIBDIR%\\classes.dex\" \"%PROJDIR%\"\n"
		L"copy \"%LIBDIR%\\libengine.so\" \"%PROJDIR%\\lib\\armeabi-v7a\"\n\n"

		L"cd \"%PROJDIR%\"\n"
		L"copy /Y libs\\armeabi-v7a\\*.so lib\\armeabi-v7a\n"
		L"call %ANDROID_AAPT_PACK% -M AndroidManifest.xml -A assets -S  res -F  %APP_NAME%.apk\n"
		L"call %ANDROID_AAPT_ADD% %APP_NAME%.apk classes.dex\n"
		L"for %%f in (lib\\armeabi-v7a\\*.so) do (\n"
		L"	call %ANDROID_AAPT_ADD% %APP_NAME%.apk lib/armeabi-v7a/%%~nxf\n"
		L")\n\n"

		L"if not exist \"%KEYSTORE%\" (\n"
		L"	call keytool -genkey -noprompt -alias emmegi -dname \"CN=emmegi.com, OU=emmegi, O=mg, L=SA, S=NA, C=IT\" -keystore %KEYSTORE% -storepass password -keypass password -validity 10000\n\n"

		L"   if not exist \"%KEYSTORE%\" (\n"
		L"		echo key generation error\n"
		L"   ) else (\n"
		L"		echo key generated\n"
		L"   )\n"
		L") else (\n"
		L"	echo key already exists\n"
		L")\n\n"

		L"echo signing key\n"
		L"call jarsigner -keystore %KEYSTORE% -signedjar %APP_NAME%.apk %APP_NAME%.apk emmegi -storepass password -keypass password\n\n"

		L"call %ADB% install -r %APP_NAME%.apk\n";

	StringUtils::WriteCharFile(tAndroidProjectDirectory + L"\\buildapk.bat",tBuildApk);

	Ide::GetInstance()->subsystem->Execute(tAndroidProjectDirectory,L"buildapk",L"apk-build-log.txt",true,true,true);

}





GuiProperty<GuiComboBox>* AndroidPluginGlobalFunctions::globalAndroidPluginCreateComboBoxProperty(GuiPanel* iPanel,String iLabel,String* iRef,int iColor)
{
	GuiProperty<GuiComboBox>* pProp=iPanel->Property<GuiComboBox>(iLabel);

	pProp->SetAllColors(GuiRect::COLOR_BACK+iColor);

	pProp->description->SetAllColors(GuiRect::COLOR_BACK+iColor);
	pProp->description->margins.x=10;

	pProp->property->string->SetStringMode(*iRef,true);
	pProp->property->string->textColor=0x000000;

	return pProp;
}

GuiProperty<GuiPath>* AndroidPluginGlobalFunctions::globalAndroidPluginCreatePathProperty(GuiPanel* iPanel,String iLabel,String* iRef,int iColor)
{
	GuiProperty<GuiPath>* pProp=iPanel->Property<GuiPath>(iLabel);

	pProp->SetAllColors(GuiRect::COLOR_BACK+iColor);

	pProp->description->SetAllColors(GuiRect::COLOR_BACK+iColor);
	pProp->description->margins.x=10;

	pProp->property->path->SetStringMode(*iRef,true);
	pProp->property->path->textColor=0x000000;

	return pProp;
}

void AndroidPluginGlobalFunctions::globalAndroidPluginCreateTextBoxProperty(GuiPanel* iPanel,String iLabel,String* iRef,int iColor)
{
	GuiProperty<GuiTextBox>* pProp=iPanel->Property<GuiTextBox>(iLabel);

	pProp->SetAllColors(GuiRect::COLOR_BACK+iColor);

	pProp->description->SetAllColors(GuiRect::COLOR_BACK+iColor);
	pProp->description->margins.x=10;

	pProp->property->SetStringMode(*iRef,true);
	pProp->property->offsets.make(0,2,0,-2);
	pProp->property->textColor=0x000000;
}

void AndroidPluginGlobalFunctions::globalAndroidPluginFindPlatformDirectories(void* iData)
{
	AndroidPlugin* tAndroidPlugin=(AndroidPlugin*)iData;

	String tSdkDirectory=tAndroidPlugin->sdkDirProperty->property->path->text;

	std::vector<String> tPlatformsDirs;
	std::vector<String> tBuildtoolDirs;

	tPlatformsDirs=Ide::GetInstance()->subsystem->ListDirectories(tSdkDirectory + L"\\" + AndroidPluginGlobalDefaults::defaultSdkPlatformDirectory);
	tBuildtoolDirs=Ide::GetInstance()->subsystem->ListDirectories(tSdkDirectory + L"\\" + AndroidPluginGlobalDefaults::defaultSdkBuildtoolsDirectory);

	tAndroidPlugin->sdkPlatform->property->items=tPlatformsDirs;
	tAndroidPlugin->sdkBuildtool->property->items=tBuildtoolDirs;

	tAndroidPlugin->sdkPlatform->property->RecreateList();
	tAndroidPlugin->sdkBuildtool->property->RecreateList();
}


void AndroidPluginGlobalFunctions::globalPackResources(String& iCurrentDirectory,ResourceNodeDir* iResDir,File& iPackFile,File& iTableFile,String& iAndroidTargetDirectory,std::vector<String>& iCollectSources)
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
		File	tDataFile(Ide::GetInstance()->folderProject + iCurrentDirectory + (*tResFile)->fileName);
		int		tDataFileStart;
		size_t	tDataFileSize;
		String	tFinalFileName;

		if(tDataFile.path.Extension()==L"cpp")
		{
			//add sources to vector, later we'll build
			iCollectSources.push_back(tDataFile.path);
		}
		else
		{
			tDataFileSize=tDataFile.Size();

			if(tDataFile.Open())
			{
				tDataFileStart=ftell(iPackFile.data);

				if(tDataFileSize)
				{
					unsigned char* tDataFileData=new unsigned char[tDataFileSize];
					fread(tDataFileData,tDataFileSize,1,tDataFile.data);
					fwrite(tDataFileData,tDataFileSize,1,iPackFile.data);
					SAFEDELETEARRAY(tDataFileData);
				}

				tDataFile.Close();
			}
			else
				DEBUG_BREAK();

			tFinalFileName=iCurrentDirectory + tDataFile.path.File();

			fwprintf(iTableFile.data,L"%s %d %d\n",tFinalFileName.c_str(),tDataFileStart,tDataFileSize);
		}
	}

	for(std::list<ResourceNodeDir*>::iterator tResNodeDir=iResDir->dirs.begin();tResNodeDir!=iResDir->dirs.end();tResNodeDir++)
	{
		AndroidPluginGlobalFunctions::globalPackResources(iCurrentDirectory,*tResNodeDir,iPackFile,iTableFile,iAndroidTargetDirectory,iCollectSources);
		iCurrentDirectory=FilePath(iCurrentDirectory).PathUp(2) + L"\\";
	}
}

void AndroidPluginGlobalFunctions::globalCompileButton(void* tData)
{
	AndroidPluginGlobalFunctions::globalCompileAndroidApk(globalAllocatedAndroidPlugin);
}

void AndroidPluginGlobalFunctions::globalExitButton(void* tData)
{
	globalAllocatedAndroidPlugin->configurationPanel->Destroy();
	Ide::GetInstance()->mainAppWindow->mainContainer->windowData->Enable(true);
}
#include "IdeAndroid.h"


AndroidPlugin* allocatedAndroidPlugin=0;

AndroidPlugin::AndroidPlugin()
{
	this->name=L"AndroidBuilder";

	Apkname=L"pippo";
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

void ptfCompileAndroidBuild(void* tData)
{
	GuiButtonFunc* tButtonFunc=(GuiButtonFunc*)tData;

	AndroidPlugin* tAndroidPlugin=(AndroidPlugin*)tButtonFunc->userData;

	CompilerAndroid tCompilerAndroid;
	tCompilerAndroid.Compile();
}

void ptfExitBuild(void* tData)
{
	GuiButtonFunc* tButtonFunc=(GuiButtonFunc*)tData;

	AndroidPlugin* tAndroidPlugin=(AndroidPlugin*)tButtonFunc->userData;

	CompilerAndroid tCompilerAndroid;
	tCompilerAndroid.Compile();
}

void AndroidPlugin::ShowConfigurationPanel()
{
	this->configurationPanel=Ide::GetInstance()->mainAppWindow->containers[0]->CreateModalTabContainer(500,300);

	GuiPanel* tPanel=this->configurationPanel->rects.Panel();

	tPanel->name=L"Android Builder";

	//strings property
	GuiPropertyString* pApkName=new GuiPropertyString(L"Apk Name",&this->Apkname,GuiPropertyString::STRING);
						pApkName->SetParent(tPanel);																	
	GuiPropertyString* pKeyname=new GuiPropertyString(L"Key Name",&this->Keyname,GuiPropertyString::STRING);
						pKeyname->SetEdges(0,&tPanel->childs.back()->edges.w,0,0);
						pKeyname->SetParent(tPanel);
	GuiPropertyString* pAndroidSdkDir=new GuiPropertyString(L"SdkDir",&this->AndroidSdkDir,GuiPropertyString::STRING);
	pAndroidSdkDir->SetEdges(0,&tPanel->childs.back()->edges.w,0,0);
						pAndroidSdkDir->SetParent(tPanel);
	GuiPropertyString* pAndroidPlatform=new GuiPropertyString(L"Platform",&this->AndroidPlatform,GuiPropertyString::STRING);
						pAndroidPlatform->SetEdges(0,&tPanel->childs.back()->edges.w,0,0);
						pAndroidPlatform->SetParent(tPanel);
	GuiPropertyString* pAndroidBuildTool=new GuiPropertyString(L"Build Tool",&this->AndroidBuildTool,GuiPropertyString::STRING);
						pAndroidBuildTool->SetEdges(0,&tPanel->childs.back()->edges.w,0,0);
						pAndroidBuildTool->SetParent(tPanel);
	GuiPropertyString* pAndroidDebugBridge=new GuiPropertyString(L"ADB",&this->AndroidDebugBridge,GuiPropertyString::STRING);
	pAndroidDebugBridge->SetEdges(0,&tPanel->childs.back()->edges.w,0,0);
	pAndroidDebugBridge->SetParent(tPanel);
	GuiPropertyString* pAndroidOutputDirectory=new GuiPropertyString(L"Output Dir",&this->AndroidOutputDirectory,GuiPropertyString::STRING);
						pAndroidOutputDirectory->SetEdges(0,&tPanel->childs.back()->edges.w,0,0);
						pAndroidOutputDirectory->SetParent(tPanel);
	GuiPropertyString* pAndroidProjectDirectory=new GuiPropertyString(L"Project Dir",&this->AndroidProjectDirectory,GuiPropertyString::STRING);
	pAndroidProjectDirectory->SetEdges(0,&tPanel->childs.back()->edges.w,0,0);
	pAndroidProjectDirectory->SetParent(tPanel);
	GuiPropertyString* pAndroidProjectJniDirectory=new GuiPropertyString(L"Jni Dir",&this->AndroidProjectJniDirectory,GuiPropertyString::STRING);
	pAndroidProjectJniDirectory->SetEdges(0,&tPanel->childs.back()->edges.w,0,0);
	pAndroidProjectJniDirectory->SetParent(tPanel);
	GuiPropertyString* pAndroidProjectAssetDirectory=new GuiPropertyString(L"Asset Dir",&this->AndroidProjectAssetDirectory,GuiPropertyString::STRING);
	pAndroidProjectAssetDirectory->SetEdges(0,&tPanel->childs.back()->edges.w,0,0);
	pAndroidProjectAssetDirectory->SetParent(tPanel);
	GuiPropertyString* pAndroidProjectResDirectory=new GuiPropertyString(L"Res Directory",&this->AndroidProjectResDirectory,GuiPropertyString::STRING);
						pAndroidProjectResDirectory->SetEdges(0,&tPanel->childs.back()->edges.w,0,0);
						pAndroidProjectResDirectory->SetParent(tPanel);
	GuiPropertyString* pAndroidProjectLibsDirectory=new GuiPropertyString(L"Libs Dir",&this->AndroidProjectLibsDirectory,GuiPropertyString::STRING);
						pAndroidProjectLibsDirectory->SetEdges(0,&tPanel->childs.back()->edges.w,0,0);
						pAndroidProjectLibsDirectory->SetParent(tPanel);

	//buttons

	this->exitButton=new GuiButtonFunc();
	this->exitButton->text=L"Exit";
	this->exitButton->SetEdges(&tPanel->edges.z,&tPanel->edges.w,&tPanel->edges.z,&tPanel->edges.w);
	this->exitButton->offsets.make(-35,-25,-5,-5);

	this->exitButton->colorBackground=0x888888;
	this->exitButton->colorHovering=0x989898;
	this->exitButton->colorPressing=0xa8a8a8;

	this->exitButton->func=ptfExitBuild;
	this->exitButton->param=this;

	this->exitButton->SetParent(tPanel);

	//button build


	this->buildButton=new GuiButtonFunc();
	this->buildButton->text=L"Build";
	this->buildButton->SetEdges(&tPanel->edges.z,&tPanel->edges.w,&tPanel->edges.z,&tPanel->edges.w);
	this->buildButton->offsets.make(-35,-50,-5,-30);

	this->buildButton->colorBackground=0x888888;
	this->buildButton->colorHovering=0x989898;
	this->buildButton->colorPressing=0xa8a8a8;

	this->buildButton->func=ptfCompileAndroidBuild;
	this->buildButton->param=this;

	this->buildButton->SetParent(tPanel);
}


PluginSystem::Plugin* GetPlugin(PluginSystem*)
{
	allocatedAndroidPlugin=new AndroidPlugin;
	return allocatedAndroidPlugin;
}
void DestroyPlugin()
{
	SAFEDELETE(allocatedAndroidPlugin);
}


void gPackNonScriptResourceDir(String& iCurrentDirectory,ResourceNodeDir* iResDir,File& iPackFile,File& iTableFile,String& iAndroidTargetDirectory,std::vector<String>& iCollectSources)
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
		gPackNonScriptResourceDir(iCurrentDirectory,*tResNodeDir,iPackFile,iTableFile,iAndroidTargetDirectory,iCollectSources);
}


bool CompilerAndroid::Compile()
{
	String tApkname=L"Engine";
	String tKeyname=L"EngineKey";

	String tAndroidSdkDir=L"C:\\Sdk\\android\\android-sdk";
	String tAndroidPlatform=L"platforms\\android-27";
	String tAndroidBuildTool=L"build-tools\\27.0.3";
	String tAndroidDebugBridge=L"c:\\adb\\adb.exe";

	String tAndroidOutputDirectory=Ide::GetInstance()->folderProject.PathUp(1) + L"\\android";
	String tAndroidProjectDirectory=tAndroidOutputDirectory + L"\\project";
	String tAndroidProjectJniDirectory=tAndroidProjectDirectory + L"\\jni";
	String tAndroidProjectAssetDirectory=tAndroidProjectDirectory + L"\\assets";
	String tAndroidProjectResDirectory=tAndroidProjectDirectory + L"\\res";

	String tAndroidProjectLibsDirectory=tAndroidProjectDirectory + L"\\libs\\armeabi-v7a";

	std::vector<String>		        tSourcePaths;
	std::vector<String>				tSourceFilesContent;
	int								tPackFileSize=0;
	int								tTableFileSize=0;
	String							tResourceDirectory=L"\\";

	{
		SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES),0,false};

		//create output directory

		::CreateDirectory(tAndroidOutputDirectory.c_str(),&sa);

		//create android project directory

		::CreateDirectory(tAndroidProjectDirectory.c_str(),&sa);

		//create android asset directory

		::CreateDirectory(tAndroidProjectAssetDirectory.c_str(),&sa);

		//create android jni project directory

		::CreateDirectory(tAndroidProjectJniDirectory.c_str(),&sa);

		//create android res project directory

		::CreateDirectory(tAndroidProjectResDirectory.c_str(),&sa);
		::CreateDirectory((tAndroidProjectResDirectory + L"\\values").c_str(),&sa);
	}

	//pack data and table

	File tPackFile(tAndroidProjectAssetDirectory + L"\\pack.pck");
	File tTableFile(tAndroidProjectAssetDirectory + L"\\ptbl.txt");

	tPackFile.Open(L"wb");
	tTableFile.Open(L"wb");

	if(tPackFile.IsOpen() && tTableFile.IsOpen())
	{
		//pack non-sources resources

		gPackNonScriptResourceDir(tResourceDirectory,GuiProjectViewer::GetPool()[0]->projectDirectory,tPackFile,tTableFile,tAndroidProjectDirectory,tSourcePaths);

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
		L"set PLATFORM=" + tAndroidSdkDir + L"\\" + tAndroidPlatform + L"\n"
		L"set BUILDTOOL=" + tAndroidSdkDir + L"\\" + tAndroidBuildTool + L"\n"
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


	return true;
}
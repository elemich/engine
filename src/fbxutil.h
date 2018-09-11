#ifndef __FBXUTIL__HEADER__
#define __FBXUTIL__HEADER__

#include "interfaces.h"

struct DLLBUILD Fbx : PluginSystem::Plugin
{
	int MenuFbx;
	int MenuActionImport;
	int MenuActionExport;

	Fbx();

	void Load();
	void Unload();
	void OnMenuPressed(int);

	EditorEntity* Import(char* iFileName);
};

extern "C" DLLBUILD PluginSystem::Plugin* GetPlugin(PluginSystem*);
extern "C" DLLBUILD void DestroyPlugin();

#endif __FBXUTIL__HEADER__
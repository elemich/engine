#include "win32.h"

#ifdef _M_X64 //_WIN64 bug in vs2010 la flag seems off but is false
hjhb
char* cl_arguments="/MDd /I\"c:\\sdk\\Autodesk\\FBX\\FBX SDK\\2014.1\\include\" /Ic:\\sdk\\openGL /I\"C:\\Sdk\\Windows\\v7.1\\Include\"  /I\"C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\VC\\include\" \"C:\\USERS\\MICHELE\\DOCUMENTS\\VISUAL STUDIO 2010\\PROJECTS\\ENGINE\\ENGINE\\EC.CPP\" /link /DEBUG /NOENTRY /LIBPATH:\"C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\VC\\lib\\amd64\" /LIBPATH:\"C:\\Sdk\\Windows\\v7.1\\Lib\\x64\" /LIBPATH:\"C:\\sdk\\Autodesk\\FBX\\FBX SDK\\2014.1\\lib\\vs2010\\x64\\debug\" opengl32.lib d3d11.lib comctl32.lib Shlwapi.lib \"libfbxsdk-md.lib\" d2d1.lib dwrite.lib windowscodecs.lib dxgi.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib anim.obj bone.obj buildpath.obj Direct2DGui.obj EC.obj entity.obj fbxutil.obj fbxutil_fbx.obj inputmanager.obj interfaces.obj jpgdecoder.obj light.obj main.obj material.obj matrixstack.obj mesh.obj picopng.obj primitives.obj shadermanager.obj skin.obj texture.obj tga_reader.obj win32containers.obj win32interfaces.obj win32msg.obj win32openglrenderer.obj win32openglshader.obj win32procedures.obj win32splitter.obj /DLL ec.obj /OUT:ec.dll";
#else
char* cl_arguments="/MDd /I\"c:\\sdk\\Autodesk\\FBX\\FBX SDK\\2014.1\\include\" /Ic:\\sdk\\openGL /I\"C:\\Sdk\\Windows\\v7.1\\Include\"  /I\"C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\VC\\include\" \"C:\\USERS\\MICHELE\\DOCUMENTS\\VISUAL STUDIO 2010\\PROJECTS\\ENGINE\\ENGINE\\EC.CPP\" /link /DEBUG /NOENTRY /LIBPATH:\"C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\VC\\lib\" /LIBPATH:\"C:\\Sdk\\Windows\\v7.1\\Lib\" /LIBPATH:\"C:\\sdk\\Autodesk\\FBX\\FBX SDK\\2014.1\\lib\\vs2010\\x86\\debug\" opengl32.lib d3d11.lib comctl32.lib Shlwapi.lib \"libfbxsdk-md.lib\" d2d1.lib dwrite.lib windowscodecs.lib dxgi.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib anim.obj bone.obj buildpath.obj Direct2DGui.obj EC.obj entity.obj fbxutil.obj fbxutil_fbx.obj inputmanager.obj interfaces.obj jpgdecoder.obj light.obj main.obj material.obj matrixstack.obj mesh.obj picopng.obj primitives.obj shadermanager.obj skin.obj texture.obj tga_reader.obj win32containers.obj win32interfaces.obj win32msg.obj win32openglrenderer.obj win32openglshader.obj win32procedures.obj win32splitter.obj /DLL ec.obj /OUT:ec.dll";
#endif
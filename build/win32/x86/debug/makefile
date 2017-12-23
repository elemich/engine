INCPATH=/Ic:\sdk\Opengl "/Ic:\sdk\Autodesk\FBX\FBX SDK\2014.1\include"
LIBPATH="/LIBPATH:c:\sdk\Autodesk\FBX\FBX SDK\2014.1\lib\vs2010\x86\debug" "/LIBPATH:C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\lib" 

CLOPT=/MDd /ZI /W0 /nologo /EHsc $(INCPATH) -c
LKOPT=/nologo /DEBUG /MANIFEST:NO /MACHINE:x86 /SUBSYSTEM:CONSOLE $(LIBPATH) 

WIN32LIBD=user32.lib opengl32.lib kernel32.lib D3D11.lib d2d1.lib Dwrite.lib Ole32.lib Windowscodecs.lib Comdlg32.lib Gdi32.lib Shlwapi.lib Winmm.lib Comctl32.lib Shell32.lib
OPENGLLIB=libfbxsdk-md.lib advapi32.lib

LKDLL=/DLL /NOENTRY $(LKOPT)  

primitives.obj: $(DIRSRC)\primitives.cpp $(DIRSRC)\primitives.h
 @cl.exe $(CLOPT) $(DIRSRC)\primitives.cpp
 
entities.obj: $(DIRSRC)\entities.cpp $(DIRSRC)\entities.h primitives.obj
 @cl.exe $(CLOPT) $(DIRSRC)\entities.cpp
 
interfaces.obj: $(DIRSRC)\interfaces.cpp $(DIRSRC)\interfaces.h entities.obj primitives.obj
 @cl.exe $(CLOPT) $(DIRSRC)\interfaces.cpp
 
win32.obj: $(DIRSRC)\win32.cpp $(DIRSRC)\win32.h primitives.obj entities.obj interfaces.obj
 @cl.exe $(CLOPT) $(DIRSRC)\win32.cpp
 
fbxutil.obj: $(DIRSRC)\fbxutil.cpp $(DIRSRC)\fbxutil.h interfaces.obj
 @cl.exe $(CLOPT) $(DIRSRC)\fbxutil.cpp
 
imgjpg.obj: $(DIRSRC)\imgjpg.cpp $(DIRSRC)\imgjpg.h
 @cl.exe $(CLOPT) $(DIRSRC)\imgjpg.cpp
 
imgpng.obj: $(DIRSRC)\imgpng.cpp $(DIRSRC)\imgpng.h
 @cl.exe $(CLOPT) $(DIRSRC)\imgpng.cpp
 
imgtga.obj: $(DIRSRC)\imgtga.cpp $(DIRSRC)\imgtga.h
 @cl.exe $(CLOPT) $(DIRSRC)\imgtga.cpp
 
engine.lib: primitives.obj entities.obj interfaces.obj imgpng.obj imgtga.obj imgjpg.obj win32.obj fbxutil.obj
 @echo engine.lib
 @lib.exe /nologo /OUT:engine.lib /NAME:engine.dll primitives.obj entities.obj interfaces.obj imgpng.obj imgtga.obj imgjpg.obj win32.obj fbxutil.obj
 
engine.dll: engine.lib
 @echo engine.dll
 @link.exe $(LKOPT) /DLL /OUT:engine.dll *.obj engine.lib libfbxsdk-md.lib $(WIN32LIBD) $(OPENGLLIB)
 
engine.exe: engine.dll *.obj
 @echo engine.exe
 @link.exe $(LKOPT) /OUT:engine.exe engine.lib *.obj $(WIN32LIBD) $(OPENGLLIB)
 
all: primitives.obj entities.obj interfaces.obj imgjpg.obj imgpng.obj imgtga.obj win32.obj fbxutil.obj engine.lib engine.dll engine.exe
 
clean:
 @echo cleaning $(DIRDST)...
 @attrib +r makefile
 @del /Q *.* 0>NUL 1>NUL 2>NUL
 @attrib -r makefile
 @pause
 

 
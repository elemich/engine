INCPATH=/Ic:\sdk\OpenGL "/Ic:\sdk\Autodesk\FBX\FBX SDK\2014.1\include"
LIBPATH="/LIBPATH:c:\sdk\Autodesk\FBX\FBX SDK\2014.1\lib\vs2010\x86\debug" "/LIBPATH:C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\lib" 

CLOPT=/MDd /ZI /W0 /nologo /EHsc $(INCPATH)
LKOPT=/nologo /DEBUG /MANIFEST:NO /MACHINE:x86 /SUBSYSTEM:CONSOLE $(LIBPATH) 

WIN32LIBD=user32.lib opengl32.lib kernel32.lib D3D11.lib d2d1.lib Dwrite.lib Ole32.lib Windowscodecs.lib Comdlg32.lib Gdi32.lib Shlwapi.lib Winmm.lib Comctl32.lib Shell32.lib
OPENGLLIB=libfbxsdk-md.lib advapi32.lib

LKDLL=/DLL /NOENTRY $(LKOPT)  

engineWin32.dll: $(DIRSRC)\primitives.cpp $(DIRSRC)\primitives.h $(DIRSRC)\entities.cpp $(DIRSRC)\entities.h $(DIRSRC)\imgjpg.cpp $(DIRSRC)\imgjpg.h $(DIRSRC)\imgpng.cpp $(DIRSRC)\imgpng.h $(DIRSRC)\imgtga.cpp $(DIRSRC)\imgtga.h $(DIRSRC)\interfaces.cpp $(DIRSRC)\interfaces.h $(DIRSRC)\win32.cpp $(DIRSRC)\win32.h $(DIRSRC)\fbxutil.cpp $(DIRSRC)\fbxutil.h
 @echo -engineWin32.dll-
 @cl.exe $(CLOPT) /DDLLBUILD $(DIRSRC)\primitives.cpp $(DIRSRC)\entities.cpp $(DIRSRC)\imgjpg.cpp $(DIRSRC)\imgpng.cpp $(DIRSRC)\imgtga.cpp $(DIRSRC)\interfaces.cpp $(DIRSRC)\win32.cpp $(DIRSRC)\fbxutil.cpp /link $(LKOPT) /DLL /OUT:engineWin32.dll $(WIN32LIBD) $(OPENGLLIB)
 
engine.exe: engineWin32.dll 
 @echo -engine.exe-
 @link $(LKOPT) /OUT:engine.exe engineWin32.lib *.obj $(WIN32LIBD) $(OPENGLLIB)

clean:
 @echo cleaning $(DIRDST)...
 @attrib +r makefile
 @del /Q *.* 0>NUL 1>NUL 2>NUL
 @attrib -r makefile
 @pause
 

 
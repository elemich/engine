
engine.exe: engine.lib main.obj
 link /DEBUG /MANIFEST:NO /OUT:engine.exe engine.lib main.obj

engine.lib: primitives.obj entities.obj jpgdecoder.obj picopng.obj tga_reader.obj interfaces.obj win32.obj
 lib /OUT:engine.lib primitives.obj entities.obj jpgdecoder.obj picopng.obj tga_reader.obj interfaces.obj win32.obj user32.lib opengl32.lib gdi32.lib winmm.lib ole32.lib Dwrite.lib Comdlg32.lib D2d1.lib Shell32.lib shlwapi.lib Comctl32.lib Windowscodecs.lib D3D11.lib kernel32.lib
 
main.obj: main.cpp
 cl.exe /MDd /Ic:\sdk\Opengl /EHsc -c main.cpp

primitives.obj: primitives.cpp primitives.h
 cl.exe /MDd /ZI /EHsc -c primitives.cpp
 
jpgdecoder.obj: jpgdecoder.cpp jpgdecoder.h
 cl.exe /MDd /ZI /EHsc -c jpgdecoder.cpp
 
picopng.obj: picopng.cpp picopng.h
 cl.exe /MDd /ZI /EHsc -c picopng.cpp
 
tga_reader.obj: tga_reader.cpp tga_reader.h
 cl.exe /MDd /ZI /EHsc -c tga_reader.cpp

entities.obj: entities.cpp entities.h
 cl.exe /MDd /ZI /EHsc -c entities.cpp
 
interfaces.obj: interfaces.cpp interfaces.h
 cl.exe /MDd /ZI /EHsc -c interfaces.cpp
  
win32.obj: win32.cpp win32.h
 cl.exe /MDd /ZI /Ic:\sdk\Opengl /EHsc -c win32.cpp
 
fbximporter: fbxutil.cpp fbxutil.h engine.lib
 cl.exe /MDd /Zi /EHsc /I"c:\sdk\Autodesk\FBX\FBX SDK\2014.1\include" fbxutil.cpp /link /DLL /DEBUG /MANIFEST:NO /LIBPATH:"c:\sdk\Autodesk\FBX\FBX SDK\2014.1\lib\vs2010\x86\debug" /OUT:fbximporter.dll libfbxsdk-md.lib libfbxsdk.lib Advapi32.lib mincore.lib engine.lib
 
clean: 
 vcvarsall.bat
 del *.exe 2>NUL
 del *.obj 2>NUL
 del *.ilk 2>NUL
 del *.pdb 2>NUL
 del *.exp 2>NUL
 del *.lib 2>NUL
 del *.idb 2>NUL
 del *.dll 2>NUL
 del *.manifest 2>NUL
 
 

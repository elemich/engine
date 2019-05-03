SDKDIR=c:\sdk

MINGWDIR=$(SDKDIR)\mingw
MINGW32DIR=$(SDKDIR)\mingw32
MINGW64DIR=$(SDKDIR)\mingw64

MINGWPREFIX=mingw32
MINGW32PREFIX=i686-w64-mingw32
MINGW64PREFIX=x86_64-w64-mingw32

COMPILER=$(MINGWPREFIX)-g++.exe
COMPILER32=$(MINGW32PREFIX)-g++.exe
COMPILER64=$(MINGW64PREFIX)-g++.exe

COMPILERDIR=$(MINGW32DIR)\bin

CPPCOMPILER=$(COMPILERDIR)\$(COMPILER32)
LINKER=$(COMPILERDIR)\$(COMPILER32)
ARCHIVER=$(COMPILERDIR)\ar.exe

OPENGLSDK=$(SDKDIR)\OpenGL
FBXSDK="$(SDKDIR)\Autodesk\FBX\FBX SDK\2014.1\include"
MINGWINCLUDES=$(MINGW32DIR)\$(MINGW32PREFIX)\include

INCPATH=-I$(OPENGLSDK) -I$(FBXSDK) -I$(MINGWINCLUDES)
LIBPATH=-L$(MINGW32DIR)\$(MINGW32PREFIX)\lib -L"$(SDKDIR)\Autodesk\FBX\FBX SDK\2014.1\lib\vs2010\x64\release"

CLOPT= -fpermissive -w -O0 -g -DCREATEDLL -DEDITORBUILD $(INCPATH) -c

STATICCPPDRIVER= -static -lstdc++

WIN32LIBD= $(STATICCPPDRIVER) -luser32 -lopengl32 -lkernel32 -lD3D11 -ld2d1 -lDwrite -lOle32 -lWindowscodecs -lComdlg32 -lGdi32 -lShlwapi -lWinmm -lComctl32 -lShell32 -lDbghelp
FBXLIB=-llibfbxsdk-md -ladvapi32

primitives.o: $(DIRSRC)\primitives.cpp $(DIRSRC)\primitives.h
	$(info primitives.o)
	$(CPPCOMPILER) $(CLOPT) $(DIRSRC)\primitives.cpp

entities.o: primitives.o $(DIRSRC)\entities.cpp $(DIRSRC)\entities.h primitives.o
	$(info entities.o)
	$(CPPCOMPILER) $(CLOPT) $(DIRSRC)\entities.cpp

imgjpg.o: entities.o $(DIRSRC)\imgjpg.cpp $(DIRSRC)\imgjpg.h
	$(info imgjpg.o)
	$(CPPCOMPILER) $(CLOPT) $(DIRSRC)\imgjpg.cpp

imgpng.o: entities.o $(DIRSRC)\imgpng.cpp $(DIRSRC)\imgpng.h
	$(info imgpng.o)
	$(CPPCOMPILER) $(CLOPT) $(DIRSRC)\imgpng.cpp

imgtga.o: entities.o $(DIRSRC)\imgtga.cpp $(DIRSRC)\imgtga.h
	$(info imgtga.o)
	$(CPPCOMPILER) $(CLOPT) $(DIRSRC)\imgtga.cpp

interfaces.o: imgjpg.o imgpng.o imgtga.o $(DIRSRC)\interfaces.cpp $(DIRSRC)\interfaces.h
	$(info interfaces.o)
	$(CPPCOMPILER) $(CLOPT) $(DIRSRC)\interfaces.cpp

win32.o: interfaces.o $(DIRSRC)\win32.cpp $(DIRSRC)\win32.h
	$(info win32.o)
	$(CPPCOMPILER) $(CLOPT) $(DIRSRC)\win32.cpp 

fbxutil.o: win32.o $(DIRSRC)\fbxutil.cpp $(DIRSRC)\fbxutil.h
	$(info fbxutil.o)
	$(CPPCOMPILER) $(CLOPT) $(DIRSRC)\fbxutil.cpp

enginelibMingW.dll: primitives.o entities.o imgjpg.o imgpng.o imgtga.o interfaces.o win32.o
	$(info enginelibMingW.dll)
	$(CPPCOMPILER) $(LIBPATH) -fpermissive -g -o enginelibMingW.dll -shared primitives.o entities.o imgjpg.o imgpng.o imgtga.o interfaces.o win32.o -Wl,--out-implib,engineMingW.a $(WIN32LIBD) 

engineMingW.exe: enginelibMingW.dll $(DIRSRC)\main.cpp
	$(info engineMingW.exe)
	$(CPPCOMPILER)  -fpermissive -g -w -O0 $(INCPATH) $(LIBPATH) -o engineMingW.exe enginelibMingW.dll $(DIRSRC)\main.cpp $(STATICCPPDRIVER) 


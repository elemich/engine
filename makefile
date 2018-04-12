DIRPROJ=$(MAKEDIR)
DIRSRC=$(DIRPROJ)\src
DIRDST=$(DIRPROJ)\build\win32\x86\debug

DELTMP=debug

x86d:
 @echo project dir: $(DIRPROJ)
 @cd $(DIRDST)
 @echo --windows nmake--
 @nmake.exe /NOLOGO /S engine.exe DIRPROJ=$(DIRPROJ) DIRSRC=$(DIRSRC) DIRDST=$(DIRDST)
 @echo --mingw nmake--
 @mingw32-make -s -f $(MAKEDIR)\makefile-mingw x86d CURDIR=$(MAKEDIR)
 @echo --ndk-build nmake--
 @ndk-build -w APP_PROJECT_PATH=$(DIRSRC)\targets\android
 
x86r:
 cd build\win32\x86\release
 nmake
 
x64d:
 cd build\win32\x64\debug
 nmake
 
x64r:
 cd build\win32\x64\release
 nmake
 
all: x86d
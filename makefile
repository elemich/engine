DIRPROJ=$(MAKEDIR)
DIRSRC=$(DIRPROJ)\src
DIRDST=$(DIRPROJ)\build\win32\x86\debug

DELTMP=debug

x86d:
 @echo project dir: $(DIRPROJ)
 @cd $(DIRDST)
 @nmake.exe /NOLOGO /S all DIRPROJ=$(DIRPROJ) DIRSRC=$(DIRSRC) DIRDST=$(DIRDST)
 @mingw32-make -s -f $(MAKEDIR)\makefile-mingw x86d CURDIR=$(MAKEDIR)
 
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
DIRPROJ=c:\projects\engine
DIRSRC=$(DIRPROJ)\src
DIRDST=$(DIRPROJ)\build\win32\x86\debug

DELTMP=debug

x86d:
 @cd $(DIRDST)
 @nmake.exe /NOLOGO /S all DIRPROJ=$(DIRPROJ) DIRSRC=$(DIRSRC) DIRDST=$(DIRDST)
 
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
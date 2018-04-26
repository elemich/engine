DIRPROJ=$(MAKEDIR)
DIRSRC=$(DIRPROJ)\src
DIRDST=$(DIRPROJ)\build\win32\x86\debug

MSDIR=$(DIRDST)\MS
MINGWDIR=$(DIRDST)\MINGW
LLVMDIR=$(DIRDST)\LLVM

x86d:
 @echo project dir: $(DIRPROJ)
 
# @echo --ms build--
# 
# @if not exist $(MSDIR) md $(MSDIR)
# @cd $(MSDIR)
# @nmake.exe -f $(DIRDST)\makefile-ms /NOLOGO /S engineMS.exe DIRPROJ=$(DIRPROJ) DIRSRC=$(DIRSRC) DIRDST=$(MSDIR)
 
# @echo --llvm build--
# 
# @if not exist $(LLVMDIR) md $(LLVMDIR)
# @cd $(LLVMDIR)
# @nmake.exe -f $(DIRDST)\makefile-llvm /NOLOGO /S engineLLVM.exe DIRPROJ=$(DIRPROJ) DIRSRC=$(DIRSRC) DIRDST=$(LLVMDIR)
 
 @echo --mingw build--
 
 @if not exist $(MINGWDIR) md $(MINGWDIR)
 @cd $(MINGWDIR)
 @mingw32-make -s -f $(DIRDST)\makefile-mingw engineMingW.exe -C $(MINGWDIR) DIRSRC=$(DIRSRC) DIRDST=$(MINGWDIR)
 
# @echo --ndk-build build--
# @ndk-build -s -w APP_PROJECT_PATH=$(DIRSRC)\targets\android DIRDST=$(DIRDST)
 
cleanx86d:
 @echo cleaning x86d dir: $(DIRDST)
 @cd $(DIRDST)
 @attrib +r makefile*
 @del /Q *.* /A:-R
 @attrib -r makefile*
 
x86r:
 cd build\win32\x86\release
 nmake
 
x64d:
 cd build\win32\x64\debug
 nmake
 
x64r:
 cd build\win32\x64\release
 nmake
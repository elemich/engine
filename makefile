DIRPROJ=$(MAKEDIR)
DIRSRC=$(DIRPROJ)\src
DIRDST=$(DIRPROJ)\build\win32\x86\debug
ANDROIDSRC=$(DIRSRC)\targets\android\src\com\android\engine

x86d:
 @echo project dir: $(DIRPROJ)
 @echo build dir: $(DIRDST)

 @echo --ms build--
 
 @cd $(DIRDST)
 @nmake.exe -f $(DIRDST)\makefile_ms /NOLOGO /S engineMS.exe DIRSRC=$(DIRSRC) DIRDST=$(DIRDST)
      
 @echo --mingw build--
 @c:\sdk\mingw32\bin\mingw32-make -s -f $(DIRDST)\makefile_mingw engineMingW.exe -C $(DIRDST) DIRSRC=$(DIRSRC) DIRDST=$(DIRDST)
 
# @echo --LLVM build--
# @nmake.exe -f $(DIRDST)\makefile_llvm /NOLOGO /S engineLLVM.exe DIRSRC=$(DIRSRC) DIRDST=$(DIRDST)
  
 @echo --ndk-build build--        
 
 if not exist $(DIRSRC)\targets\android\obj mkdir $(DIRSRC)\targets\android\obj
 
 @c:\sdk\android\android-ndk-r16b_x86_64\ndk-build NDK_DEBUG=1 APP_PROJECT_PATH=$(DIRSRC)\targets\android DIRSRC=$(DIRSRC)\targets\android DIRDST=$(DIRDST)
 @javac -verbose -g -d $(DIRSRC)\targets\android\obj  $(ANDROIDSRC)\EngineActivity.java -sourcepath $(ANDROIDSRC)\*.java -classpath C:\Sdk\android\android-sdk\platforms\android-27\android.jar
 @C:\Sdk\android\android-sdk\build-tools\27.0.3\dx --verbose --dex --output=$(DIRDST)\classes.dex $(DIRSRC)\targets\android\obj
 
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
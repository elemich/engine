@echo off

echo paths in the apk must have the / separator otherwise LoadLibrary will not find the native so lib
echo aapt add will register all file path, so call from current dir to avoid full path recording

set PLATFORM=C:\Sdk\android\android-sdk\platforms\android-27
set BUILDTOOL=C:\Sdk\android\android-sdk\build-tools\27.0.3
set LIBDIR=C:\Projects\Engine\build\win32\x86\debug
set PROJDIR=C:\Users\Michele\Desktop\EngineProjects\android\project
set ADB=c:\adb\adb.exe
set KEYSTORE=EngineKey
set PREVDIR=%cd%

SET APP_NAME=Engine
SET ANDROID_REV=android-27
SET ANDROID_AAPT_ADD=%BUILDTOOL%\aapt.exe add
SET ANDROID_AAPT_PACK=%BUILDTOOL%\aapt.exe package -v -f -I %PLATFORM%\android.jar

rmdir %PROJDIR%\lib /S /Q
mkdir %PROJDIR%\lib
mkdir %PROJDIR%\lib\armeabi-v7a

copy %LIBDIR%\classes.dex %PROJDIR%
copy %LIBDIR%\libengine.so %PROJDIR%\lib\armeabi-v7a

call %ANDROID_AAPT_PACK% -M %PROJDIR%\AndroidManifest.xml -A %PROJDIR%\assets -S %PROJDIR%\res -F %PROJDIR%\%APP_NAME%.apk

cd %PROJDIR%
call %ANDROID_AAPT_ADD% %PROJDIR%/%APP_NAME%.apk classes.dex
call %ANDROID_AAPT_ADD% %PROJDIR%/%APP_NAME%.apk lib/armeabi-v7a/libengine.so
cd %PREVDIR%

if not exist "%PROJDIR%/%KEYSTORE%" (
	call keytool -genkey -noprompt -alias emmegi -dname "CN=emmegi.com, OU=emmegi, O=mg, L=SA, S=NA, C=IT" -keystore %PROJDIR%\%KEYSTORE% -storepass password -keypass password -validity 10000 
	
	if not exist "%PROJDIR%/%KEYSTORE%" (
		echo key generation error
	) else (
		echo key generated
	)
) else (
	echo key already exists
)

echo signing key
call jarsigner -keystore %PROJDIR%\%KEYSTORE% -signedjar %PROJDIR%\%APP_NAME%.apk %PROJDIR%\%APP_NAME%.apk emmegi -storepass password -keypass password

call %ADB% install -r %PROJDIR%\%APP_NAME%.apk




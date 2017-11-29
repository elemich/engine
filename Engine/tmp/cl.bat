@echo off
if "%_M_X64%"=="" ( call "c:\Program Files (x86)\Microsoft visual Studio 10.0\VC\bin\vcvars32.bat" 
"c:\Program Files (x86)\Microsoft visual Studio 10.0\VC\bin\cl.exe" %* > cl.txt ) else ( call "c:\Program Files (x86)\Microsoft visual Studio 10.0\VC\bin\amd64\vcvars64.bat" 
"c:\Program Files (x86)\Microsoft visual Studio 10.0\VC\bin\amd64\cl.exe" %* > cl.txt ) 

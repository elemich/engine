#!/bin/bash
called_path=${0%/*}
stripped=${called_path#[^/]*}
real_path=`pwd`$stripped

DIRPROJ=$real_path
echo project dir: $DIRPROJ
DIRSRC=$DIRPROJ/src
DIRDST=$DIRPROJ/build/win32/x86/debug
DIRSDK=/media/michele/Homedata/michele/sdk
ANDROIDSRC=$DIRSRC/targets/android/src/com/android/engine

echo building with mingw
echo project dir: $DIRPROJ
echo build dir: $DIRDST
echo sdk dir: $DIRSDK
 
make -s -f $DIRDST/makefile_linux all -C $DIRDST DIRSRC=$DIRSRC DIRDST=$DIRDST DIRSDK=$DIRSDK

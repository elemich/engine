LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

TARGET_ARCH_ABI := armeabi-v7a

MY_SOURCE_PATH:=$(LOCAL_PATH)/../../..

LOCAL_MODULE := Engine
LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv2 -latomic

LOCAL_SRC_FILES  = $(MY_SOURCE_PATH)/primitives.cpp \
				   $(MY_SOURCE_PATH)/entities.cpp \
				   $(MY_SOURCE_PATH)/EngineAndroid.cpp

include $(BUILD_SHARED_LIBRARY)

all: copyAllFiles

copyAllFiles:
	@copy $(DIRSRC)\libs\$(APP_ABI) $(DIRDST)
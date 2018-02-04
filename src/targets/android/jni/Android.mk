LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

TARGET_ARCH_ABI := armeabi-v7a

MY_SOURCE_PATH:=$(LOCAL_PATH)/../../..

$(warning $(LOCAL_PATH))
$(warning $(MY_SOURCE_PATH))

LOCAL_MODULE := Engine
LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv2 -latomic

LOCAL_SRC_FILES  = $(MY_SOURCE_PATH)/primitives.cpp \
				   $(MY_SOURCE_PATH)/entities.cpp \
				   $(MY_SOURCE_PATH)/imgjpg.cpp \
				   $(MY_SOURCE_PATH)/imgpng.cpp \
				   $(MY_SOURCE_PATH)/imgtga.cpp \
				   $(MY_SOURCE_PATH)/targets/android/android.cpp

include $(BUILD_SHARED_LIBRARY)
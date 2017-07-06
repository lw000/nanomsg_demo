LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := lib_hn_crosslog_static

LOCAL_MODULE_FILENAME := libhncrosslog

LOCAL_SRC_FILES := ../log4z.cpp \
					../FastLog.cpp \
					../log4z_android_ios_ext.cpp \

LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. \
          
LOCAL_LDLIBS    := -lm -llog          
LOCAL_EXPORT_LDLIBS := -llog \
                        -lz \
                        -landroid
                        
#include $(BUILD_STATIC_LIBRARY)

include $(BUILD_SHARED_LIBRARY)
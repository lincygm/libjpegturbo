LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libturbojpeg
LOCAL_SRC_FILES := libturbojpeg.so
include $(PREBUILT_SHARED_LIBRARY) 

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_MODULE := myjpeg
LOCAL_SRC_FILES := jpeggm.c
LOCAL_SHARED_LIBRARIES := libturbojpeg

LOCAL_LDLIBS := -llog -lz -ldl  
include $(BUILD_SHARED_LIBRARY)
FORCE_GYP := $(shell make -C ../../ GypAndroid.mk)
MY_LOCAL_PATH := $(call my-dir)
LOCAL_PATH := $(MY_LOCAL_PATH) 
include $(CLEAR_VARS)
LOCAL_MODULE := libv8_libbase
LOCAL_SRC_FILES := ../../../third_parties/lib/arm/v8/libv8_libbase.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_PATH := $(MY_LOCAL_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE := libv8_base
LOCAL_SRC_FILES := ../../../third_parties/lib/arm/v8/libv8_base.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_PATH := $(MY_LOCAL_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE := libv8_libplatform
LOCAL_SRC_FILES := ../../../third_parties/lib/arm/v8/libv8_libplatform.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_PATH := $(MY_LOCAL_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE := libv8_nosnapshot
LOCAL_SRC_FILES := ../../../third_parties/lib/arm/v8/libv8_nosnapshot.a
include $(PREBUILT_STATIC_LIBRARY)

include ../../GypAndroid.mk

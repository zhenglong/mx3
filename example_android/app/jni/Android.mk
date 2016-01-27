FORCE_GYP := $(shell make -C ../../ GypAndroid.mk)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := libv8
LOCAL_SRC_FILES := ../../../third_parties/lib/arm/v8/libv8.so
include $(PREBUILT_SHARED_LIBRARY)

include ../../GypAndroid.mk

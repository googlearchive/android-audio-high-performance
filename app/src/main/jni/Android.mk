LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := generate_tone
LOCAL_SRC_FILES := tone.c

include $(BUILD_SHARED_LIBRARY)

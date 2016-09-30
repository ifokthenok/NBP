LOCAL_PATH := $(call my-dir)

# module name should match apk name to be installed
include $(CLEAR_VARS)
LOCAL_MODULE := HomeDemo
LOCAL_SRC_FILES := HomeScreenDemo_55.apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_PATH := $(TARGET_OUT_APPS_PRIVILEGED)
LOCAL_MODULE_SUFFIX := .apk
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)


# module name should match apk name to be installed
include $(CLEAR_VARS)
LOCAL_MODULE := StoreDemo
LOCAL_SRC_FILES := StoreClientDemo_52.apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := .apk
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)

# module name should match apk name to be installed
include $(CLEAR_VARS)
LOCAL_MODULE := SyncDemo
LOCAL_SRC_FILES := SyncClientDemo_41.apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := .apk
LOCAL_CERTIFICATE := PRESIGNED
include $(BUILD_PREBUILT)



LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES += frameworks/base/services/camera/libcameraservice

LOCAL_SRC_FILES:= \
    NativeWindowFactory.cpp \
    BootCamera.cpp  \
	bootcamera_main.cpp
	
#LOCAL_SRC_FILES += ../../native/android/native_window.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
    libui \
    libgui  \
    libhardware \
    libcamera_client
	
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := bootcamera


include $(BUILD_EXECUTABLE)

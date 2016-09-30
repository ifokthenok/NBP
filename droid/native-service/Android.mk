LOCAL_PATH:= $(call my-dir)

###############################################
# Build libhelloworld.so
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    IHelloWorldService.cpp \
    BnHelloWorldService.cpp \
    BpHelloWorldService.cpp \
    HelloWorldService.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder
	
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:= libhelloworld

include $(BUILD_SHARED_LIBRARY)


###############################################
# Build libhelloworldservice
include $(CLEAR_VARS)

LOCAL_SRC_FILES := main_helloworldservice.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder   \
	libhelloworld

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := helloworldservice

include $(BUILD_EXECUTABLE)


###############################################
# Build helloworldclient

include $(CLEAR_VARS)

LOCAL_SRC_FILES := main_helloworldclient.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder   \
	libhelloworld
	
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := helloworldclient

include $(BUILD_EXECUTABLE)

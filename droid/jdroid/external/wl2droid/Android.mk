# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# the purpose of this sample is to demonstrate how one can
# generate two distinct shared libraries and have them both
# uploaded in
#

LOCAL_PATH:= $(call my-dir)

topdir=external
ifeq ($(HAVE_RT),y)
RT=-lrt
endif

# scanner, which compile protocol from xml file
#
include $(CLEAR_VARS)

wayland_scanner = wayland-scanner
LOCAL_MODULE    := $(wayland_scanner)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES :=				\
	src/wayland-util.c 			\
	src/scanner.c
LOCAL_STATIC_LIBRARIES := libexpat
LOCAL_C_INCLUDES := $(topdir)/expat/lib

include $(BUILD_HOST_EXECUTABLE)

$(LOCAL_PATH)/src/wayland-version.h:$(LOCAL_PATH)/src/wayland-version.h.in
	sed "s/@WAYLAND_VERSION_MAJOR@/$(wayland_major)/" $< > $@
	sed -i "s/@WAYLAND_VERSION_MINOR@/$(wayland_minor)/" $@ 
	sed -i "s/@WAYLAND_VERSION_MICRO@/$(wayland_micro)/" $@
	sed -i "s/@WAYLAND_VERSION@/$(wayland_version)/" $@

$(LOCAL_PATH)/protocol/%-server-protocol.h:$(LOCAL_PATH)/protocol/%.xml
	$(wayland_scanner) server-header < $< > $@

$(LOCAL_PATH)/protocol/%-client-protocol.h:$(LOCAL_PATH)/protocol/%.xml
	$(wayland_scanner) client-header < $< > $@

$(LOCAL_PATH)/protocol/%-protocol.c:$(LOCAL_PATH)/protocol/%.xml
	$(wayland_scanner) code < $< > $@	

# first build our libffi.so
#
MY_LOCAL_PATH := $(LOCAL_PATH)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libffi/lib/libffi.so
include $(BUILD_MULTI_PREBUILT)
LOCAL_PATH := $(MY_LOCAL_PATH)

# liffi.so.6 -> libffi.so
#
MY_SYMLINK := $(TARGET_OUT)/lib/libffi.so.6
MY_SOURCE := $(TARGET_OUT)/lib/libffi.so
$(MY_SYMLINK): $(MY_SOURCE) $(LOCAL_INSTALLED_MODULE) 
	@echo "Symlink: $@ -> $(MY_SOURCE)"
	$(hide) mkdir -p $(dir $@)
	$(hide) ln -sf libffi.so $@
	
ALL_DEFAULT_INSTALLED_MODULES += $(MY_SYMLINK)
# We need this so that the installed files could be picked up based on the
# local module name
ALL_MODULES.$(LOCAL_MODULE).INSTALLED := \
    $(ALL_MODULES.$(LOCAL_MODULE).INSTALLED) $(MY_SYMLINK)


# util lib, which will be built statically
#
include $(CLEAR_VARS)

LOCAL_MODULE    := libwayland-util
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES +=	\
	$(LOCAL_PATH)/libffi/lib/libffi-3.1/include	\
	$(LOCAL_PATH)/src

LOCAL_SRC_FILES := \
	src/connection.c			\
	src/wayland-util.c			\
	src/wayland-util.h			\
	src/wayland-os.c			\
	src/wayland-os.h			\
	src/wayland-private.h

LOCAL_SHARED_LIBRARIES := libffi
include $(BUILD_STATIC_LIBRARY)

# server lib, which will depend on and include the util
#
include $(CLEAR_VARS)

LOCAL_MODULE    := libwayland_server
LOCAL_MODULE_TAGS := optional
#LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES := \
	src/wayland-server.c				\
	src/wayland-shm.c					\
	src/event-loop.c 					\
	protocol/wayland-server-protocol.h	\
	protocol/wayland-protocol.c	

LOCAL_CFLAGS := -I$(LOCAL_PATH)
LOCAL_LDLIBS    += $(RT) -lm
#LOCAL_LDFLAGS = \
#	-Wl,-version-info,$(wayland_major):$(wayland_minor):$(wayland_micro)
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/libffi/lib/libffi-3.1/include	\
	$(LOCAL_PATH)/src 				\
	$(LOCAL_PATH)/protocol
LOCAL_STATIC_LIBRARIES := libwayland-util
LOCAL_SHARED_LIBRARIES := libffi

include $(BUILD_SHARED_LIBRARY)

# client lib, which will depend on and include the util
#
include $(CLEAR_VARS)

LOCAL_MODULE    := libwayland_client
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES :=		\
	src/wayland-client.c 				\
	protocol/wayland-client-protocol.h	\
	protocol/wayland-protocol.c 		

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/libffi/lib/libffi-3.1/include	\
	$(LOCAL_PATH)/src 					\
	$(LOCAL_PATH)/protocol

LOCAL_LDLIBS    += $(RT) -lm
#LOCAL_LDFLAGS = \
#	-Wl,-version-info,$(wayland_major):$(wayland_minor):$(wayland_micro)
LOCAL_STATIC_LIBRARIES := libwayland-util
LOCAL_SHARED_LIBRARIES := libffi 

include $(BUILD_SHARED_LIBRARY)

# cursor lib, which will depend on and include the util
#
include $(CLEAR_VARS)

LOCAL_MODULE    := libwayland_cursor
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES :=		\
	cursor/wayland-cursor.c				\
	cursor/os-compatibility.c			\
	cursor/os-compatibility.h			\
	cursor/cursor-data.h				\
	cursor/xcursor.c					\
	cursor/xcursor.h 					\
	src/wayland-version.h 				\
	src/wayland-client.c 				\
	protocol/wayland-client-protocol.h	\
	protocol/wayland-protocol.c	


LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/src \
	$(LOCAL_PATH)/protocol

LOCAL_CFLAGS := -I$(LOCAL_PATH)
LOCAL_LDLIBS    += $(RT) -lm
#LOCAL_LDFLAGS = \
#	-Wl,-version-info,$(wayland_major):$(wayland_minor):$(wayland_micro)
LOCAL_STATIC_LIBRARIES := libwayland-util
LOCAL_SHARED_LIBRARIES := libffi 

include $(BUILD_SHARED_LIBRARY)

# cursor lib, which will depend on and include the util
#
include $(CLEAR_VARS)

LOCAL_MODULE    := wayland_test
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES :=		\
	src/connection.c			\
	src/wayland-util.c			\
	src/wayland-util.h			\
	src/wayland-os.c			\
	src/wayland-os.h			\
	src/wayland-private.h 		\
	tests/test-runner.c			\
	tests/test-runner.h			\
	tests/test-helpers.c 		\
	tests/array-test.c 			\
	tests/client-test.c 		\
	tests/display-test.c 		\
	tests/connection-test.c 	\
	tests/event-loop-test.c 	\
	tests/fixed-test.c 			\
	tests/list-test.c 			\
	tests/map-test.c 			\
	tests/sanity-test.c 		\
	tests/socket-test.c 		\
	tests/queue-test.c 			\
	tests/signal-test.c 		\
	tests/resources-test.c 		\
	tests/message-test.c 		\
	tests/os-wrappers-test.c

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/libffi/lib/libffi-3.1/include	\
	$(LOCAL_PATH)/src 				\
	$(LOCAL_PATH)/protocol

LOCAL_CFLAGS := -I$(LOCAL_PATH) -UNDEBUG
LOCAL_LDLIBS    += $(RT) -lm
LOCAL_SHARED_LIBRARIES := libdl libwayland_client libwayland_server libffi
#LOCAL_LDFLAGS:= -export-dynamic
LOCAL_STATIC_LIBRARIES := libwayland-util

include $(BUILD_EXECUTABLE)


# hello world demo from https://github.com/hdante/hello_wayland
#
#include $(MY_LOCAL_PATH)/hello_wayland/Android.mk


# wl-test domo used for wayland testing
#
include $(MY_LOCAL_PATH)/droid_wayland/Android.mk


# oip sample from Conti
#
include $(MY_LOCAL_PATH)/oip_wayland/Android.mk


# gralloc.default.so and wl2droid
#
include $(MY_LOCAL_PATH)/gralloc_wayland/Android.mk


# hwcomposer.default.so
#
include $(MY_LOCAL_PATH)/hwcomposer_wayland/Android.mk


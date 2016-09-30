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


LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := wayland_hello
LOCAL_MODULE_TAGS := eng

LOCAL_SRC_FILES :=	\
	hello_wayland.c	\
	helpers.c

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../src 				\
	$(LOCAL_PATH)/../protocol			\
	$(LOCAL_PATH)

LOCAL_CFLAGS := -std=c11 -fvisibility=hidden
LOCAL_LDLIBS  += $(RT)
LOCAL_SHARED_LIBRARIES := libwayland_client

include $(BUILD_EXECUTABLE)



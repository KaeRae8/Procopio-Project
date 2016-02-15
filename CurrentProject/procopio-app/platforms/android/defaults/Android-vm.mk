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
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_LDLIBS := -L${SYSROOT}/usr/lib -lz -lGLESv1_CM

NATIVE_LIBS := ../../../libraries/native

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../libraries/native/plasmacore
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../libraries/native/slag
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../libraries/native/zlib-1.2.3/contrib/minizip
LOCAL_CFLAGS += -D SLAG_VM -D SLAG_USE_LONGJMP
LOCAL_CFLAGS += -fno-strict-aliasing
# no strict aliasing is essential for the Slag VM!

LOCAL_MODULE    := $(JNI_PROJECT_ID)

LOCAL_SRC_FILES := \
	android_core.cpp \
	custom.cpp \
  $(NATIVE_LIBS)/plasmacore/plasmacore.cpp \
  $(NATIVE_LIBS)/plasmacore/gl_core.cpp \
  $(NATIVE_LIBS)/slag/slag_vm.cpp \
  $(NATIVE_LIBS)/slag/slag_stdlib.cpp \
  $(NATIVE_LIBS)/slag/slag_mm.cpp \
  $(NATIVE_LIBS)/slag/slag_runtime.cpp \
  $(NATIVE_LIBS)/plasmacore/font_system_17.cpp \
  $(NATIVE_LIBS)/zlib-1.2.3/contrib/minizip/unzip.c \
  $(NATIVE_LIBS)/zlib-1.2.3/contrib/minizip/ioapi.c

include $(BUILD_SHARED_LIBRARY)


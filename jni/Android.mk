LOCAL_PATH := $(call my-dir)
 
include $(CLEAR_VARS)

OPENCV_LIB_TYPE:=STATIC
OPENCV_INSTALL_MODULES:=on
OPENCV_CAMERA_MODULES:=off

include local.env.mk
include $(OPENCV_CONFIG)

# Here we give our module name and source file(s)
LOCAL_MODULE    := tagrecognizer-jni
LOCAL_SRC_FILES := cv-image.cpp cv-squares.cpp cv-tagRecognizer.cpp cv-tag.cpp
APP_OPTIM := release
LOCAL_LDLIBS +=  -llog -ldl

include $(BUILD_SHARED_LIBRARY)
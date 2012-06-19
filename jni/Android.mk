LOCAL_PATH := $(call my-dir)
 
include $(CLEAR_VARS)

OPENCV_LIB_TYPE:=STATIC
OPENCV_INSTALL_MODULES:=on
OPENCV_CAMERA_MODULES:=off

include local.env.mk
include $(OPENCV_CONFIG)

# Here we give our module name and source file(s)
LOCAL_MODULE    := opencv
LOCAL_SRC_FILES := opencv-image.cpp opencv-squares.cpp opencv-algorithm.cpp

LOCAL_LDLIBS +=  -llog -ldl

include $(BUILD_SHARED_LIBRARY)
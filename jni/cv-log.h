/**
 * @file cv-log.h
 * @brief Definition of macros that allows to access to the log mechanism in Android from C++.
 * @author Antonio Manuel Gutierrez Martinez
 * @version 1.0
 */
#ifndef LOG_H
#define LOG_H

#include <android/log.h>

static std::string ANDROID_PATH = "/mnt/sdcard/Pictures/TagRecognizerApp/";
#define ANDROID_LOG_VERBOSE ANDROID_LOG_DEBUG
#define LOG_TAG "CVJNI"
#define LOGV(...) __android_log_print(ANDROID_LOG_SILENT, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#endif //LOG_H
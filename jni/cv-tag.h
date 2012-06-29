#ifndef TAG_H
#define TAG_H

#include <jni.h>
#include <cv.h>
#include <cxcore.h>
#include <opencv2/highgui/highgui.hpp>
#include "cv-log.h"

#ifdef __cplusplus
extern "C" {
#endif

std::vector<std::string> decodeTags (const std::vector<std::vector<cv::Mat> >& subsquares ) ;

std::string decodeTag ( std::vector<cv::Mat>& subsquares ) ;  

#ifdef __cplusplus
}
#endif

#endif //TAG_H
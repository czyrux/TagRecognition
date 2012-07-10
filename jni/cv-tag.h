#ifndef TAG_H
#define TAG_H

#include <jni.h>
#include <cv.h>
#include <cxcore.h>
#include <opencv2/highgui/highgui.hpp>
#include "cv-log.h"
#include "cv-constants.h"

#define COLS 4
#define ROWS 2

#define RED_VALUE 1
#define BLUE_VALUE 2
#define GREEN_VALUE 3
#define DEFAULT_VALUE 0

#ifdef __cplusplus
extern "C" {
#endif

std::vector<std::string> decodeTags (const std::vector<std::vector<cv::Mat> >& subsquares ) ;

std::string decodeTag (const std::vector<cv::Mat>& subsquares , int index ) ;  

void orientedTag ( std::vector<std::vector<int> > &v );

#ifdef __cplusplus
}
#endif

#endif //TAG_H
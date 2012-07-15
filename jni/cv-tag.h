#ifndef TAG_H
#define TAG_H

#include <jni.h>
#include <cv.h>
#include <cxcore.h>
#include <opencv2/highgui/highgui.hpp>
#include "cv-log.h"
#include "cv-image.h"
#include "cv-squares.h"

#define COLS 4
#define ROWS 2

#define RED_VALUE 1
#define BLUE_VALUE 2
#define GREEN_VALUE 3
#define DEFAULT_VALUE 0

#define DEBUG_TAG true

#ifdef __cplusplus
extern "C" {
#endif

struct Tag {
	int x;
	int y;
	std::string code;
};

std::vector<Tag> findTags ( const Image_data* data , bool oriented=false); 

std::vector<std::string> decodeTags (const std::vector<std::vector<cv::Mat> >& subsquares , bool oriented=false ) ;

std::string decodeTag (const std::vector<cv::Mat>& subsquares , int index , bool oriented=false ) ;  

inline bool validTag (std::string tag );

void orientedTag ( std::vector<std::vector<int> > &v );

void adjustRGBBoundaries(std::string readed ,std::string original );

#ifdef __cplusplus
}
#endif

#endif //TAG_H
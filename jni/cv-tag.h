#ifndef TAG_H
#define TAG_H

#include <jni.h>
#include <cv.h>
#include <cxcore.h>
#include <opencv2/highgui/highgui.hpp>
#include "cv-log.h"
#include "cv-image.h"
#include "cv-rect.h"
#include "cv-conf.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Defined value to red color in tag */
#define RED_VALUE 1
/** Defined value to blue color in tag */
#define BLUE_VALUE 2
/** Defined value to green color in tag */
#define GREEN_VALUE 3
/** Defined a unknown color in tag  */
#define DEFAULT_VALUE 0

/**
 * Data structure that represent a tag data
 * Fields:
 * - x. X coordinate in the image
 * - y. Y coordinate in the image
 * - code. String that identify the tag 
 */
struct Tag {
	float x;
	float y;
	std::string code;
};

/**
 * Find tags in the image passed as argument.
 * @param data pointer to Image_data structure. The fields rImage, bImage and gImage must be
 * filled.
 * @param oriented. It considered the tag already oriented if it is set to true. Used as part 
 * of calibrating procces. Default value false.
 * @return Array of Tag struct.
 */
std::vector<Tag> findTags ( const Image_data* data , bool oriented=false); 

/**
 * Digitalize the tags passed as argument in subsquares. Each tag is a set of 3 black and white images.
 * 1st is red channel, 2nd blue channel and 3rd green channel of the same image. Could be a 4 image, in color,
 * as part of debug proccess.
 * @param subsquares Array of Mats arrays.
 * @param oriented. It considered the tag already oriented if it is set to true. Used as part 
 * of calibrating procces. Default value false.
 * @return Array of strings with the digitalizes codes of each tag.
 */
std::vector<std::string> decodeTags (const std::vector<std::vector<cv::Mat> >& subsquares , bool oriented=false ) ;

/**
 * Digitalize the tag passed as argument in subsquares. The tag is a set of 3 black and white images.
 * 1st is red channel, 2nd blue channel and 3rd green channel of the same image. Could be a 4 image, in color,
 * as part of debug proccess.
 * @param subsquares Array of Mats arrays.
 * @param oriented. It considered the tag already oriented if it is set to true. Used as part 
 * of calibrating procces. Default value false.
 * @return String with the digitalize code of tag.
 */
std::string decodeTag (const std::vector<cv::Mat>& subsquares , int index , bool oriented=false ) ;  

/**
 * Check if one code tag as the correct format.
 * @param tag code to evaluate
 * @return result of evaluation
 */
inline bool validTag (const std::string &tag );

/**
 * Set the correct alignment to a tag represented as a 2D matrix of integers. The changes are made over the same
 * matrix.
 * @param v 2DMatrix of integers that represent a digitalized tag.
 */
void orientedTag ( std::vector<std::vector<int> > &v );

/**
 * Check the errors between the read code tag and the template tag. Modify the values of RED_THRESHOLD,
 * BLUE_THRESHOLD and GREEN_THRESHOLD in function of the errors found.
 * @param readed. Code tag readed
 * @param _template. Template tag to compare
 */
void adjustRGBThreshold(const std::string &readed ,const std::string &_template );

#ifdef __cplusplus
}
#endif

#endif //TAG_H
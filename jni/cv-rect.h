#ifndef RECT_H
#define RECT_H

#include <jni.h>
#include <cv.h>
#include <cxcore.h>
#include <opencv2/highgui/highgui.hpp>
#include "cv-log.h"
#include "cv-image.h"
#include "cv-conf.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************/

/**
 * Data structure used to hold information about the location
 * of rectangles inside of a image.
 * The fields are:
 * - points array of 2Dpoints
 * - bounding_box minimum rotated rectangle that envolves the points
 * - wrapper_box minimum box that involves the points
 */
struct Rect {
    cv::RotatedRect bounding_box;
    cv::Rect wrapper_box;
    std::vector<cv::Point> points;
};

/**************************************/

/**
 * Extract the data from the points of rectangle and return the
 * Rect structure asociated to the points.
 * @param p Array of points thats make up the square. 
 */
Rect extractRectData (const std::vector<cv::Point> &p);

/**
 * Find squares inside the image. Use the OpenCV's function cv::findContours
 * @param image Image of gray scale where to look for squares.
 * @return array of rectangles found.
 */
std::vector<Rect> findRect( const cv::Mat& image );

/**
 * helper function:
 * Finds a cosine of angle between vectors from pt0->pt1 and from pt0->pt2
 */
double angle( cv::Point pt1, cv::Point pt2, cv::Point pt0 );

/**
 * Draws all the squares in the image.
 * @param image RGB image where draw the squares.
 * @param squares Squares to draw.
 */
void drawRect( cv::Mat& image, const std::vector<Rect>& rect);

/**
 * Create a array of subsquares (Mat) from each square. Each array have 4 images
 * (R,G,B,RGB).
 * @param src Image_data structure with originals R,G,B and RGB images.
 * @param rect Array with the information of the location of rectangles.
 * @return Array of arrays of Mat (subimages)
 */
std::vector<std::vector<cv::Mat> > cutRect( const Image_data* src, 
												const std::vector<Rect>& rect);

/**
 * Remove border from image. Use the proportion of WIDTH_BORDER and HEIGHT_BORDER.
 * Used in the process of cutRect.
 */
void removeBorder ( cv::Mat & img ); 

/**
 * Remove rectangles that are inside others.
 */
void filterRect ( std::vector<Rect>& squares);

/**
 * helper function:
 * Check if the squares b is inside the square a.
 */
inline bool inside (const Rect &a , const Rect &b );

#ifdef __cplusplus
}
#endif

#endif //RECT_H
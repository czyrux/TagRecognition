#ifndef SQUARE_H
#define SQUARE_H

#include <jni.h>
#include <cv.h>
#include <cxcore.h>
#include <opencv2/highgui/highgui.hpp>
#include "cv-log.h"
#include "cv-image.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LIMIT_ROTATION 1 //degrees
#define MIN_RECT_SIZE 20 //size in pixels
#define WIDTH_BORDER 1/8. //1/9.5
#define HEIGHT_BORDER 1/4.5

#define DEBUG_SQUARES true

/**************************************/

/**
 * Data structure used to hold information about the location
 * of a square/rectangle inside of a image.
 */
struct Square {
    cv::RotatedRect rect;//bounding_box
    cv::Rect frame;//wrapper_box
    std::vector<cv::Point> points;
};

/**************************************/

/**
 * Extract the data from the points of square and return the
 * Square structure asociated to the points.
 * @param p Array of points thats make up the square. 
 */
Square extractSquareData (const std::vector<cv::Point> &p);

/**
 * Find squares inside the image. Use the OpenCV's function cv::findContours
 * @param image Image of gray scale where to look for squares.
 * @return array of squares found.
 */
std::vector<Square> findSquares( const cv::Mat& image );

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
void drawSquares( cv::Mat& image, const std::vector<Square>& squares);

/**
 * Create a array of subsquares (Mat) from each square. Each array have 4 images
 * (R,G,B,RGB).
 * @param src Image_data structure with originals R,G,B and RGB images.
 * @param squares Array with the information of the location of squares.
 * @return Array of arrays of Mat (subimages)
 */
std::vector<std::vector<cv::Mat> > cutSquares( const Image_data* src, 
												const std::vector<Square>& squares);

/**
 * Remove border from squares. Use the proportion of WIDTH_BORDER and HEIGHT_BORDER.
 * Used in the process of cutSquares.
 */
void removeBorder ( cv::Mat & img ); 

/**
 * Remove squares that are inside others.
 */
void filterSquares ( std::vector<Square>& squares);

/**
 * helper function:
 * Check if the squares b is inside the square a.
 */
inline bool inside (const Square &a , const Square &b );

#ifdef __cplusplus
}
#endif

#endif //SQUARE_H
#ifndef SQUARE_H
#define SQUARE_H

#include <jni.h>
#include <cv.h>
#include <cxcore.h>
#include <opencv2/highgui/highgui.hpp>
#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LIMIT_ROTATION 5 //degrees
#define MIN_RECT_SIZE 20

/**************************************/

struct Square {
    cv::RotatedRect rect;
    cv::Rect frame;
    std::vector<cv::Point> points;
};

/**************************************/

//Extract the data from the points of square
Square extractSquareData (const std::vector<cv::Point> &p);

// returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
void findSquares( const cv::Mat& image, std::vector<Square>& squares);

// the function draws all the squares in the image
void drawSquares( cv::Mat& image, const std::vector<Square>& squares);

//Extract the squares from the images and save the result in subsquares
void cutSquares( const cv::Mat& image, const std::vector<Square>& squares , std::vector<cv::Mat>& subsquares);

//Remove double squares over the same tag. Take the square more inside
void filterSquares ( std::vector<Square>& squares);

#ifdef __cplusplus
}
#endif

#endif //SQUARE_H
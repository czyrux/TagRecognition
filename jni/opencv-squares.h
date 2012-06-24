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

using namespace cv;

/**************************************/

struct Square {
    RotatedRect rect;
    Rect frame;
    vector<Point> points;
};

/**************************************/

//
Square extractSquareData (const std::vector<Point> &p);

// returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
void findSquares( const Mat& image, std::vector<Square>& squares);

// the function draws all the squares in the image
void drawSquares( Mat& image, const std::vector<Square>& squares);

//Extract the squares from the images and save the result in subsquares
void cutSquares( const Mat& image, const std::vector<Square>& squares , std::vector<Mat>& subsquares);

//
void filterSquares ( std::vector<Square>& squares);

//
void rotateSquares( const Mat& image , const std::vector<Square>& squares, std::vector<Mat>& subsquares);

#ifdef __cplusplus
}
#endif

#endif //SQUARE_H
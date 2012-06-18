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
using namespace std;

/**************************************/

struct Square {
    Point vertex;
    int w;
    int h;
    vector<Point> points;
};

/**************************************/

//
Square extractSquareData (vector<Point> &p);

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
double angle( Point pt1, Point pt2, Point pt0 );

// returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
void findSquares( const Mat& image, vector<Square>& squares);//vector<vector<Point> >& squares, vector<SquareData>& datasquares );

// the function draws all the squares in the image
void drawSquares( Mat& image, const vector<Square>& squares);//const vector<vector<Point> >& squares , const vector<SquareData>& datasquares);

//
void cutSquares(Mat& image, vector<Square>& squares);

// the function draws all the squares in the image
void filterSquares ( vector<Square>& squares);

#ifdef __cplusplus
}
#endif

#endif //SQUARE_H
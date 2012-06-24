#include "opencv-squares.h"

/**************************************/

Square extractSquareData (const std::vector<Point> &p) 
{
    Square sq;

    //Take the real dimensions of square
    sq.rect = cv::minAreaRect(cv::Mat(p));

    //Calculate the box which involve the square
    sq.frame = cv::boundingRect(cv::Mat(p));//sq.rect.boundingRect();

    //Save the points
    sq.points = p;

    return sq;
}

/**************************************/

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
double angle( Point pt1, Point pt2, Point pt0 )
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

/**************************************/

// returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
void findSquares( const Mat& image, std::vector<Square>& squares )
{
    squares.clear();
    Mat gray;
    vector<vector<Point> > contours;

    cvtColor( image, gray, CV_BGR2GRAY );

    /*
    string file = "/mnt/sdcard/Pictures/MyCameraApp/blackWhite.jpeg";
    imwrite(file,gray);*/
    
    
    // find contours and store them all as a list
    findContours(gray, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

    vector<Point> approx;
            
    // test each contour
    for( size_t i = 0; i < contours.size(); i++ )
    {
        // approximate contour with accuracy proportional
        // to the contour perimeter
        approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);
                
        // square contours should have 4 vertices after approximation
        // relatively large area (to filter out noisy contours)
        // and be convex.
        // Note: absolute value of an area is used because
        // area may be positive or negative - in accordance with the
        // contour orientation
        if( approx.size() == 4 && fabs(contourArea(Mat(approx))) > 20 &&
            isContourConvex(Mat(approx)) )
       {
            double maxCosine = 0;

            for( int j = 2; j < 5; j++ )
            {
                // find the maximum cosine of the angle between joint edges
                double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
                maxCosine = MAX(maxCosine, cosine);
            }

            // if cosines of all angles are small
            // (all angles are ~90 degree) then write quandrange
            // vertices to resultant sequence
            if( maxCosine < 0.3 ) {
                squares.push_back(extractSquareData(approx));
            }
         }
    }
}

/**************************************/

// the function draws all the squares in the image
void drawSquares( Mat& image, const std::vector<Square>& squares )
{
    Point po;
    for( size_t i = 0; i < squares.size(); i++ )
    {
        const Point* p = &squares[i].points[0];
        int n = (int)squares[i].points.size();
        polylines(image, &p, &n, 1, true, Scalar(0,0,255),1,CV_AA);//, 3, CV_AA);
        //draw frame top up left
        po.x = squares[i].frame.x;
        po.y = squares[i].frame.y;
        circle(image, po, 3, Scalar(0,255,255),2,CV_AA);
    }
}

/**************************************/

void cutSquares(const Mat& image, const std::vector<Square>& sq , std::vector<Mat>& subsquares)
{    
    subsquares.clear();
    for ( int i=0 ; i < sq.size() ; i++ ) {
        Mat subimg(image,sq[i].frame);
        subsquares.push_back(subimg);

        //Write image
        std::stringstream os1;
        os1 << "_" << i ;
        string file = "/mnt/sdcard/Pictures/MyCameraApp/squares" + os1.str() + ".jpeg";
        imwrite(file,subimg);
    }
}

/**************************************/

//check if the squares b is inside the square a
inline bool inside (const Square &a , const Square &b ) {
    return (a.frame & b.frame) == b.frame; //if the intersection between both is b
}

// the function draws all the squares in the image
void filterSquares ( std::vector<Square>& squares)
{
    vector<Square> sol;
    sol.clear();
    int n = squares.size();
    int *check = new int[n];
    for ( int i=0; i<n; i++ ) {
        check[i] = 0;
    }

    //check wich one's are inside others
    for (int i=0 ; i<n ; i++ ) {
        if ( check[i] != 1 && squares[i].frame.x != 1 && squares[i].frame.y != 1) {
            for ( int j=0 ; j<n ; j++ ) {
                if (j != i && check[i] != 1 && inside(squares[i],squares[j]) ) {
                    check[j] = 1; //it is inside
                    break;
                }
            }
        }
    }

    //take the greaters
    for ( int i=0 ; i<n ; i++ )
        if (check[i] == 0 )
            sol.push_back(squares[i]);

    //take the solution already filter
    squares = sol;

    delete check;
}

/**************************************/

void rotateSquares( const Mat& src , const std::vector<Square>& squares, std::vector<Mat>& subsquares) 
{
    for ( int i = 0 ; i<squares.size() ; i++ ) 
    {
        // matrices we'll use
        cv::Mat rotated , rot_mat , cropped;

        // get angle and size from the bounding box
        double angle = squares[i].rect.angle;
        cv::Size box_size = squares[i].rect.size;

        //adjust the angle from "http://felix.abecassis.me/2011/10/opencv-rotation-deskewing/"
        if (angle < -45.) {
            angle += 90.;
            std::swap(box_size.width, box_size.height);
        }

        //Rotation
        rot_mat = cv::getRotationMatrix2D(squares[i].rect.center, angle, 1);
        cv::warpAffine(src, rotated, rot_mat, src.size(), cv::INTER_CUBIC); // apply the geometric transformation
        
        //Cropped image
        cv::getRectSubPix(rotated, box_size, squares[i].rect.center, cropped);
        
        //log
        /*
        std::stringstream os;
        os << "Square : " << i << " --->alpha:"<< box.angle;
        LOGI(os.str().c_str());
        */
        //Write image
        std::stringstream os1;
        os1 << i ;
        string file = "/mnt/sdcard/Pictures/MyCameraApp/squares_rotate_" + os1.str() + ".jpeg";
        imwrite(file,cropped);
    }
    
}

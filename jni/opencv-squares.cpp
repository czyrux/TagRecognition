#include "opencv-squares.h"

/**************************************/

Square extractSquareData (const std::vector<cv::Point> &p) 
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
double angle( cv::Point pt1, cv::Point pt2, cv::Point pt0 )
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
void findSquares( const cv::Mat& image, std::vector<Square>& squares )
{
    squares.clear();
    cv::Mat gray;
    std::vector<std::vector<cv::Point> > contours;

    cv::cvtColor( image, gray, CV_BGR2GRAY );
    
    // find contours and store them all as a list
    cv::findContours(gray, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

    std::vector<cv::Point> approx;
            
    // test each contour
    for( size_t i = 0; i < contours.size(); i++ )
    {
        // approximate contour with accuracy proportional
        // to the contour perimeter
        cv::approxPolyDP(cv::Mat(contours[i]), approx, arcLength(cv::Mat(contours[i]), true)*0.02, true);
                
        // square contours should have 4 vertices after approximation
        // relatively large area (to filter out noisy contours)
        // and be convex.
        // Note: absolute value of an area is used because
        // area may be positive or negative - in accordance with the
        // contour orientation
        if( approx.size() == 4 && fabs(cv::contourArea(cv::Mat(approx))) > MIN_RECT_SIZE &&
            cv::isContourConvex(cv::Mat(approx)) )
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
void drawSquares( cv::Mat& image, const std::vector<Square>& squares )
{
    cv::Point po;
    for( size_t i = 0; i < squares.size(); i++ )
    {
        const cv::Point* p = &squares[i].points[0];
        int n = (int)squares[i].points.size();
        cv::polylines(image, &p, &n, 1, true, cv::Scalar(0,0,255),1,CV_AA);//, 3, CV_AA);
        //draw frame top up left
        po.x = squares[i].frame.x;
        po.y = squares[i].frame.y;
        cv::circle(image, po, 3, cv::Scalar(0,255,255),2,CV_AA);
    }
}

/**************************************/

void cutSquares(const cv::Mat& src, const std::vector<Square>& sq , std::vector<cv::Mat>& subsquares)
{    
    subsquares.clear();
    for ( int i=0 ; i < sq.size() ; i++ ) 
    {
         // get angle and size from the bounding box
        double angle = sq[i].rect.angle;
        cv::Size box_size = sq[i].rect.size;
        //adjust the angle from "http://felix.abecassis.me/2011/10/opencv-rotation-deskewing/"
        if (angle < -45.) {
            angle += 90.;
            std::swap(box_size.width, box_size.height);
        }

        //log
        std::stringstream os;
        os << "Square : " << i << " --->alpha:"<< angle;

        //rotation
        if ( std::abs(angle) > LIMIT_ROTATION ) {
            // matrices we'll use
            cv::Mat rotated , rot_mat , cropped;

            //Rotation
            rot_mat = cv::getRotationMatrix2D(sq[i].rect.center, angle, 1);
            cv::warpAffine(src, rotated, rot_mat, src.size(), cv::INTER_CUBIC); // apply the geometric transformation
        
            //Cropped image
            cv::getRectSubPix(rotated, box_size, sq[i].rect.center, cropped);
            subsquares.push_back(cropped);
            os << "_rotated";
        }
        else //just cut it
        { 
            cv::Mat subimg(src,sq[i].frame);
            subsquares.push_back(subimg);
        }     
        
        //log
        LOGI(os.str().c_str());
        
        //Write image
        std::stringstream os1;
        os1 << "_" << i ;
        std::string file = "/mnt/sdcard/Pictures/MyCameraApp/squares" + os1.str() + ".jpeg";
        cv::imwrite(file,subsquares[i]);
    }
}

/**************************************/

//check if the squares b is inside the square a
inline bool inside (const Square &a , const Square &b ) {
    return (a.frame & b.frame) == b.frame; //if the intersection between both is b
}

// the function draws all the squares in the image
void filterSquares ( std::vector<Square>& squares )
{
    std::vector<Square> sol;
    sol.clear();
    int n = squares.size();
    int *check = new int[n];
    for ( int i=0; i<n; i++ ) {
        check[i] = 0;
    }

    //check which one's are inside others
    for (int i=0 ; i<n ; i++ ) {
        if ( check[i] != 1 ) {
            for ( int j=0 ; j<n ; j++ ) {
                if (j != i && check[j] != 1 && inside(squares[j],squares[i]) ) {
                    check[j] = 1; //it is inside
                    break;
                }
            }
        }
    }

    //eliminate the squares which has another inside
    for ( int i=0 ; i<n ; i++ )
        if (check[i] == 0 )
            sol.push_back(squares[i]);

    //take the solution already filter
    squares = sol;

    delete check;
}

/**************************************/

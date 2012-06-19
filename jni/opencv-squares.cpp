#include "opencv-squares.h"

/**************************************/

Square extractSquareData (std::vector<Point> &p) {
    //stringstream os;
    int xmin = p[0].x, ymin = p[0].y, xmax = p[0].x , ymax = p[0].y ;
    for (int i=1 ; i<p.size() ; i++) {
        if (p[i].x < xmin ) xmin = p[i].x;
        if (p[i].x > xmax ) xmax = p[i].x;
        if (p[i].y < ymin ) ymin = p[i].y;
        if (p[i].y > ymax ) ymax = p[i].y;
    }

    Square result;
    result.vertex.x = xmin;
    result.vertex.y = ymin;
    result.w = xmax-xmin;
    result.h = ymax-ymin;
    result.points = p;

    //os << "Square en: min(" << result.vertex.x << "," << result.vertex.y << ") width:" << result.w << ", heigh" << result.h << "";
    //LOGI(os.str().c_str());

    return result;
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

    string file = "/mnt/sdcard/Pictures/MyCameraApp/blackWhite.jpeg";
    imwrite(file,gray);
    
    
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
        if( approx.size() == 4 && fabs(contourArea(Mat(approx))) > 10 &&
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
                //squares.push_back(approx);
                //datasquares.push_back(extractSquareData(approx));
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
        //draw vertex up left
        po.x = squares[i].vertex.x;
        po.y = squares[i].vertex.y;
        circle(image, po, 3, Scalar(0,255,255),2,CV_AA);
        /*po.x = squares[i].w + squares[i].vertex.x;
        po.y = squares[i].h + squares[i].vertex.y;
        circle(image, po, 5, Scalar(255,0,255),2,CV_AA);*/
    }
}

/**************************************/

void cutSquares(Mat& image, std::vector<Square>& sq , std::vector<Mat>& subsquares)
{    
    subsquares.clear();
    for ( int i=0 ; i < sq.size() ; i++ ) {
        //stringstream os;
        //os << "sub:" << i << " ";
        //os << sq[i].vertex.x<< " "<< sq[i].vertex.y<< " "<< sq[i].w+sq[i].vertex.x<< " "<<sq[i].h+sq[i].vertex.y;
        //LOGI(os.str().c_str());
        Mat subimg(image,cvRect(sq[i].vertex.x, sq[i].vertex.y, sq[i].w, sq[i].h)); //AQUI EL CAMBIO
        subsquares.push_back(subimg);
        //stringstream os1;
        //os1 << "_" << i ;
        //string file = "/mnt/sdcard/Pictures/MyCameraApp/squares" + os1.str() + ".jpeg";
        //imwrite(file,subimg);
    }
}

/**************************************/

//check if the squares b is inside the square a
bool inside (const Square &a , const Square &b ) {
    return ( (a.vertex.x < b.vertex.x && a.vertex.y < b.vertex.y) &&
            ((a.vertex.x+a.w) > b.vertex.x && (a.vertex.y+a.h) > b.vertex.y) &&
            (a.w > b.w && a.h > b.h) );
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
        if ( check[i] != 1 && squares[i].vertex.x != 1 && squares[i].vertex.y != 1) {
            for ( int j=0 ; j<n ; j++ ) {
                /*stringstream os;
                os << "check " << i << " (" << squares[i].vertex.x << "," << squares[i].vertex.y 
                   << ") con " << j << " (" << squares[j].vertex.x << "," << squares[j].vertex.y << ")" ;
                */
                if (j != i && check[i] != 1 && inside(squares[i],squares[j]) ) {
                    check[j] = 1; //it is inside
                    break;
                    //os << " dentro";
                }
                //LOGI(os.str().c_str());
            }
        }
    }

    //take the greaters
    for ( int i=0 ; i<n ; i++ )
        if (check[i] == 0 )
            sol.push_back(squares[i]);

    //take the solution already filter
    squares = sol;
}

/**************************************/

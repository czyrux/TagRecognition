#include "cv-rect.h"

/**************************************/

//Extract the data from the points of rectangle and return the
//Rect structure asociated to the points.
Rect extractRectData (const std::vector<cv::Point> &p) 
{
    Rect sq;
    //Take the real dimensions of rectangle
    sq.bounding_box = cv::minAreaRect(cv::Mat(p));
    //Calculate the box which involve the rectangle
    sq.wrapper_box = cv::boundingRect(cv::Mat(p));
    //Store points
    sq.points = p;

    return sq;
}

/**************************************/

// finds a cosine of angle between vectors from pt0->pt1 and from pt0->pt2
double angle( cv::Point pt1, cv::Point pt2, cv::Point pt0 )
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

/**************************************/

//find rectangles on image
std::vector<Rect> findRect( const cv::Mat& image )
{
    std::vector<Rect> rect;
    rect.clear();
    cv::Mat gray = image.clone();
    std::vector<std::vector<cv::Point> > contours;

    //SLOWER
    //erote the image to fill holes 
    /*int erosion_size = 1;
    cv::Mat element = getStructuringElement( cv::MORPH_RECT,
                                       cv::Size( 2*erosion_size + 1, 2*erosion_size+1 ),
                                       cv::Point( -1, -1 ) );
    cv::erode(gray, gray, element);
    */
    /*
    cv::erode(gray, gray, cv::Mat(), cv::Point(-1,-1),1); //standard call
    //cv::dilate(gray, gray, cv::Mat(), cv::Point(-1,-1),2); //standard call
    std::string file = "/mnt/sdcard/Pictures/MyCameraApp/red_trans.jpeg";
    cv::imwrite(file,gray);
    */
    

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

            for( int j = 2; j < 5; j++ ) {
                // find the maximum cosine of the angle between joint edges
                double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
                maxCosine = MAX(maxCosine, cosine);
            }

            // if cosines of all angles are small
            // (all angles are ~90 degree) then store quandrange
            // vertices to resultant sequence
            if( maxCosine < 0.3 ) {
                rect.push_back(extractRectData(approx));
            }
         }
    }

    return rect;
}

/**************************************/

// draws all the rectangles in the image
void drawRect( cv::Mat& image, const std::vector<Rect>& rect )
{
    cv::Point po;
    for( size_t i = 0; i < rect.size(); i++ )
    {
        const cv::Point* p = &rect[i].points[0];
        int n = (int)rect[i].points.size();
        cv::polylines(image, &p, &n, 1, true, cv::Scalar(0,255,255),2,CV_AA);
        //draw wrapper_box top up left
        po.x = rect[i].wrapper_box.x;
        po.y = rect[i].wrapper_box.y;
        cv::circle(image, po, 3, cv::Scalar(0,255,255),2,CV_AA);
    }
}

/**************************************/

//Remove border from rectangles
void removeBorder ( cv::Mat & img ) 
{
    int w , h;
    if (img.cols > img.rows ) {
        w = img.cols * WIDTH_BORDER;
        h = img.rows * HEIGHT_BORDER;
    }else {
        w = img.cols * HEIGHT_BORDER;
        h = img.rows * WIDTH_BORDER;
    }

    img = img( cv::Rect(w, h, (img.cols-w*2) , (img.rows-h*2)) );
}

/**************************************/

//Create a array of subrectangles (Mat) from each rectangle.
std::vector<std::vector<cv::Mat> > cutRect(const Image_data* src, const std::vector<Rect>& sq )
{    
    //Array to hold the solution
    std::vector<std::vector<cv::Mat> > subrect;
    subrect.clear();

    //Matrix's to extract the subrectangles
    cv::Mat rImage (src->rImage,false) , gImage (src->gImage,false) , bImage (src->bImage,false)
            , img (src->src,false);

    std::string file;
    //log
    if (DEBUG_RECT) {
        file = ANDROID_PATH + "cut_img.jpeg";
        cv::imwrite(file,img);
    }
    

    //erode images
    //cv::erode(gImage, gImage, cv::Mat(), cv::Point(-1,-1),1); //standard call
    //cv::erode(bImage, bImage, cv::Mat(), cv::Point(-1,-1),1); //standard call

    //Cut and deskew squares
    for ( int i=0 ; i < sq.size() ; i++ ) 
    {
        //Submatrix to fill
        cv::Mat subimg_red, subimg_green , subimg_blue , subimg;

        //auxiliar vector to store the array of Mat
        std::vector<cv::Mat> aux;
        aux.clear();

        // get angle and size from the bounding box
        double angle = sq[i].bounding_box.angle;
        cv::Size box_size = sq[i].bounding_box.size;
        //adjust the angle from "http://felix.abecassis.me/2011/10/opencv-rotation-deskewing/"
        if (angle < -45.) {
            angle += 90.;
            std::swap(box_size.width, box_size.height); // for cropping
        }

        //rotation
        if ( std::abs(angle) > LIMIT_ROTATION ) {
            
            // matrices we'll use
            cv::Mat rotatedR , rotatedG , rotatedB , rot_mat , rotated;
            
            //Rotation Matrix. Same rotation for all
            rot_mat = cv::getRotationMatrix2D(sq[i].bounding_box.center, angle, 1);
            
            //Apply transformation
            cv::warpAffine(rImage, rotatedR, rot_mat, rImage.size(), cv::INTER_LINEAR); // apply the geometric transformation
            cv::warpAffine(gImage, rotatedG, rot_mat, gImage.size(), cv::INTER_LINEAR);
            cv::warpAffine(bImage, rotatedB, rot_mat, bImage.size(), cv::INTER_LINEAR);//INTER_CUBIC
            cv::warpAffine(img   , rotated, rot_mat, img.size(), cv::INTER_LINEAR);
            
            //log
            if (DEBUG_RECT) {
                file = ANDROID_PATH + "cut_subimg_rot.jpeg";
                cv::imwrite(file,rotated);
            }

            //Cropped image
            cv::getRectSubPix(rotatedR, box_size, sq[i].bounding_box.center, subimg_red);
            cv::getRectSubPix(rotatedG, box_size, sq[i].bounding_box.center, subimg_green);
            cv::getRectSubPix(rotatedB, box_size, sq[i].bounding_box.center, subimg_blue);
            cv::getRectSubPix(rotated, box_size , sq[i].bounding_box.center, subimg);
            
            //log
            if (DEBUG_RECT) {
                file = ANDROID_PATH + "cut_subimg_crop.jpeg";
                cv::imwrite(file,subimg);
            }
            
        }
        else {//just cut it
            subimg_red   = rImage(sq[i].wrapper_box);
            subimg_green = gImage(sq[i].wrapper_box); 
            subimg_blue  = bImage(sq[i].wrapper_box);
            subimg       = img(sq[i].wrapper_box);
        }     
        
        //remove border of tag
        removeBorder(subimg_red);
        removeBorder(subimg_green);
        removeBorder(subimg_blue);
        removeBorder(subimg);

        //log
        if (DEBUG_RECT) {
            file = ANDROID_PATH + "cut_subimg_borderRemoved.jpeg";
            cv::imwrite(file,subimg);
        }

        //add subimages
        aux.push_back(subimg_red);//R
        aux.push_back(subimg_green);//G
        aux.push_back(subimg_blue);//B
        aux.push_back(subimg);//SRC

        subrect.push_back(aux);       
    }

    return subrect;
}

/**************************************/

//check if the rectangle b is inside the rectangle a
inline bool inside (const Rect &a , const Rect &b ) {
    return (a.wrapper_box & b.wrapper_box) == b.wrapper_box; //if the intersection between both is b
}

/**************************************/

//remove rectangles that are inside others
void filterRect ( std::vector<Rect>& rect )
{
    std::vector<Rect> sol;
    sol.clear();
    int n = rect.size();
    int *check = new int[n];
    for ( int i=0; i<n; i++ ) {
        check[i] = 0;
    }

    //check which one's are inside others
    for (int i=0 ; i<n ; i++ ) {
        if ( check[i] != 1 && rect[i].wrapper_box.x != 1 && rect[i].wrapper_box.y != 1) {
            for ( int j=0 ; j<n ; j++ ) {
                if (j != i && check[j] != 1 && inside(rect[i],rect[j]) ) {
                    check[j] = 1; //it is inside
                    break;
                }
            }
        }
    }

    //eliminate the squares which has another inside
    for ( int i=0 ; i<n ; i++ )
        if (check[i] == 0 && rect[i].wrapper_box.x != 1 && rect[i].wrapper_box.y != 1 )
            sol.push_back(rect[i]);

    //take the solution already filter
    rect = sol;

    delete check;
}

/**************************************/

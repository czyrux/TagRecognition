#include <jni.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "opencv2/nonfree/features2d.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <android/log.h>
#include <cv.h>
#include "opencv-image.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace std;
using namespace cv;



IplImage* pImage = NULL;

/**************************************/

/*
JNIEXPORT void JNICALL Java_de_unidue_tagrecognition_OpenCV_extractSURFFeature(
		JNIEnv* env, jobject thiz)
{
	IplImage *pWorkImage=cvCreateImage(cvGetSize(pImage),IPL_DEPTH_8U,1);
	cvCvtColor(pImage,pWorkImage,CV_BGR2GRAY);
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq *imageKeypoints = 0, *imageDescriptors = 0;
	CvSURFParams params = cvSURFParams(2000, 0);
	LOGI("after");
	cvExtractSURF( pWorkImage, 0, &imageKeypoints, &imageDescriptors, storage, params );
	LOGI("before");
	// show features
	for( int i = 0; i < imageKeypoints->total; i++ )
	{
		CvSURFPoint* r = (CvSURFPoint*)cvGetSeqElem( imageKeypoints, i );
		CvPoint center;
		int radius;
		center.x = cvRound(r->pt.x);
		center.y = cvRound(r->pt.y);
		radius = cvRound(r->size*1.2/9.*2);
		cvCircle( pImage, center, radius, CV_RGB(255,0,0), 1, CV_AA, 0 );
	}
	cvReleaseImage(&pWorkImage);
	cvRelease((void **)&imageKeypoints);
	cvReleaseMemStorage(&storage);
	LOGI("extract done");
}
*/
/**************************************/

JNIEXPORT jboolean JNICALL Java_de_unidue_tagrecognition_OpenCV_setSourceImage(
		JNIEnv* env, jobject thiz, jintArray photo_data, jint width,
		jint height) 
{
	if (pImage != NULL) {
		cvReleaseImage(&pImage);
		pImage = NULL;
	}
	pImage = getIplImageFromIntArray(env, photo_data, width, height);
	if (pImage == NULL) {
		return 0;
	}
	LOGI("Load Image Done.");
	return 1;
}

/**************************************/

JNIEXPORT jbooleanArray JNICALL Java_de_unidue_tagrecognition_OpenCV_getSourceImage(
		JNIEnv* env, jobject thiz)
{
	if (pImage == NULL) {
		LOGE("No source image.");
		return 0;
	}
	cvFlip(pImage);
	int width = pImage->width;
	int height = pImage->height;
	int rowStep = pImage->widthStep;
	int headerSize = 54;
	int imageSize = rowStep * height;
	int fileSize = headerSize + imageSize;
	unsigned char* image = new unsigned char[fileSize];
	struct bmpfile_header* fileHeader = (struct bmpfile_header*) (image);
	fileHeader->magic[0] = 'B';
	fileHeader->magic[1] = 'M';
	fileHeader->filesz = fileSize;
	fileHeader->creator1 = 0;
	fileHeader->creator2 = 0;
	fileHeader->bmp_offset = 54;
	struct bmp_dib_v3_header_t* imageHeader =
			(struct bmp_dib_v3_header_t*) (image + 14);
	imageHeader->header_sz = 40;
	imageHeader->width = width;
	imageHeader->height = height;
	imageHeader->nplanes = 1;
	imageHeader->bitspp = 24;
	imageHeader->compress_type = 0;
	imageHeader->bmp_bytesz = imageSize;
	imageHeader->hres = 0;
	imageHeader->vres = 0;
	imageHeader->ncolors = 0;
	imageHeader->nimpcolors = 0;
	memcpy(image + 54, pImage->imageData, imageSize);
	jbooleanArray bytes = env->NewBooleanArray(fileSize);
	if (bytes == 0) {
		LOGE("Error in creating the image.");
		delete[] image;
		return 0;
	}
	env->SetBooleanArrayRegion(bytes, 0, fileSize, (jboolean*) image);
	delete[] image;
	return bytes;
}

/**************************************/

JNIEXPORT jboolean JNICALL Java_de_unidue_tagrecognition_OpenCV_releaseSourceImage(
		JNIEnv* env, jobject thiz ) 
{
	if (pImage != NULL) {
		cvReleaseImage(&pImage);
		pImage = NULL;
	}
}

/**************************************/

JNIEXPORT void JNICALL Java_de_unidue_tagrecognition_OpenCV_extract(
		JNIEnv* env, jobject thiz) 
{
	LOGI("extract enter SIFT\n");
 	Mat img (pImage,true);
 	Mat grey;
 	cvtColor(img, grey, CV_BGR2GRAY);
  	vector<KeyPoint> keypoints;

  	// Detect the keypoints
  	
  	//SurfFeatureDetector detector(400.);
  	//FeatureDetector *detector = new SiftFeatureDetector(400, 500);
  	//FeatureDetector *detector = new SurfFeatureDetector(400.);
  	FeatureDetector *detector = new FastFeatureDetector(50);
  	detector->detect(grey,keypoints);
  	//Mat outputImage;
  	Scalar keypointColor = Scalar(0, 0, 255);     // color keypoints.
  	drawKeypoints(img, keypoints, img, keypointColor, DrawMatchesFlags::DEFAULT);
  	
  	//IplImage *dest = &outputImage.operator IplImage();
  	pImage = cvCloneImage(&img.operator IplImage());
  	LOGI("extract done\n");
}

JNIEXPORT jbooleanArray JNICALL Java_de_unidue_tagrecognition_OpenCV_extractFAST(
		JNIEnv* env, jobject thiz , jintArray photo_data, jint width,
		jint height) 
{
	LOGI("extractFAST enter\n");
	IplImage *image_data = getIplImageFromIntArray(env, photo_data, width, height);
	if (image_data == NULL) {
		return 0;
	}
 	Mat img (image_data,true);
 	Mat grey;
 	cvtColor(img, grey, CV_BGR2GRAY);
  	vector<KeyPoint> keypoints;

  	// Detect the keypoints
  	FeatureDetector *detector = new FastFeatureDetector(50);
  	//FeatureDetector *detector = new SiftFeatureDetector(400, 500);
  	//FeatureDetector *detector = new SurfFeatureDetector(400.);
  	detector->detect(grey,keypoints);
  	Scalar keypointColor = Scalar(0, 0, 255);     // color keypoints.
  	drawKeypoints(img /*original*/, keypoints, img/*result*/, keypointColor, DrawMatchesFlags::DEFAULT);
  	
  	cvReleaseImage(&image_data);
  	//image_data = &img.operator IplImage();
  	LOGI("extract done\n");
  	
  	return getBmpImage(env,&img.operator IplImage());
}


/**************************************/

struct SquareData {
    int x;
    int y;
    int width;
    int height;
};

SquareData extractSquareData (vector<Point> &p) {
    stringstream os;
    int xmin = p[0].x, ymin = p[0].y, xmax = p[0].x , ymax = p[0].y ;
    for (int i=1 ; i<p.size() ; i++) {
        if (p[i].x < xmin ) xmin = p[i].x;
        if (p[i].x > xmax ) xmax = p[i].x;
        if (p[i].y < ymin ) ymin = p[i].y;
        if (p[i].y > ymax ) ymax = p[i].y;
    }

    SquareData result;
    result.x = xmin;
    result.y = ymin;
    result.width = xmax - xmin;
    result.height = ymax - ymin; 

    os << "Square en: (" << result.x << "," << result.y << ") con : w=" << result.width << ", h:" << result.height;
    os << " max(" << xmax << "," << ymax << ") ";
    LOGI(os.str().c_str());

    return result;
}

int thresh = 50, N = 5;//11;

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

// returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
void findSquares( const Mat& image, vector<vector<Point> >& squares, vector<SquareData>& datasquares,
    double threshold1, double threshold2 , int apertureSize , bool scale = false )
{
    squares.clear();
    datasquares.clear();
    Mat gray;
    vector<vector<Point> > contours;
    if (scale) {
        /* Canny reducing original image
        Mat pyr, timg, gray0(image.size(), CV_8U);
    
        // down-scale and upscale the image to filter out the noise
        pyrDown(image, pyr, Size(image.cols/2, image.rows/2));
        pyrUp(pyr, timg, image.size());
        
        int ch[] = {1, 0};
        mixChannels(&timg, 1, &gray0, 1, ch, 1);

        // apply Canny. Take the upper threshold from slider
        // and set the lower to 0 (which forces edges merging)
        //Canny(gray0, gray, 0, thresh, 5);
        //Canny(gray0, gray, 500, 200, 3);
        Canny(gray0, gray, threshold1, threshold2, apertureSize);
        */

        //Canny over original image
        Canny(image, gray, threshold1, threshold2, apertureSize);
        // dilate canny output to remove potential
        // holes between edge segments
        dilate(gray, gray, Mat(), Point(-1,-1),2);//TOCAR POR AKI
        stringstream os;
        os << threshold1 << "_" << threshold2 << "_" << apertureSize << "_" << scale ;
        string file = "/mnt/sdcard/Pictures/MyCameraApp/canny" + os.str() + ".jpeg";
        imwrite(file,gray);

    }else {
        cvtColor( image, gray, CV_BGR2GRAY );
        stringstream os;
        os << "squares_before" ;
        string file = "/mnt/sdcard/Pictures/MyCameraApp/canny" + os.str() + ".jpeg";
        imwrite(file,gray);
    }
    
    
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
            	squares.push_back(approx);
                datasquares.push_back(extractSquareData(approx));
            }
         }
    }

    // find squares in every color plane of the image
    /*
    for( int c = 0; c < 3; c++ )
    {
        int ch[] = {c, 0};
        mixChannels(&timg, 1, &gray0, 1, ch, 1);

        // try several threshold levels
        for( int l = 0; l < N; l++ )
        {
            // hack: use Canny instead of zero threshold level.
            // Canny helps to catch squares with gradient shading
            if( l == 0 )
            {
                // apply Canny. Take the upper threshold from slider
                // and set the lower to 0 (which forces edges merging)
                Canny(gray0, gray, 0, thresh, 5);
                // dilate canny output to remove potential
                // holes between edge segments
                dilate(gray, gray, Mat(), Point(-1,-1));
            }
            else
            {
                // apply threshold if l!=0:
                //     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
                gray = gray0 >= (l+1)*255/N;
            }

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

				if( approx.size() == 4 &&
                    fabs(contourArea(Mat(approx))) > 50 &&
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
                    if( maxCosine < 0.3 )
                        squares.push_back(approx);
                }
            }
        }
    }
    */
}

// the function draws all the squares in the image
void drawSquares( Mat& image, const vector<vector<Point> >& squares )
{
    for( size_t i = 0; i < squares.size(); i++ )
    {
        const Point* p = &squares[i][0];
        int n = (int)squares[i].size();
        polylines(image, &p, &n, 1, true, Scalar(0,0,255),1,CV_AA);//, 3, CV_AA);
    }
}



// the function draws all the squares in the image
void filterSquares( vector<vector<Point> >& squares , vector<SquareData>& d /*datasquares*/)
{
    vector<vector<Point> > sol ;
    sol.clear();
    int inc;
    for ( int i=0 ; i<squares.size() ; i++ ) {
        inc = 0; //if nothing happen, there is not additional increase
        sol.push_back(squares[i]);
        //looking for subsquares
        for ( int j=i+1; j<squares.size() ; j++)
            if ( d[i].x > d[j].x && (d[i].x-d[i].width) < d[j].x 
                && d[i].y > d[j].y  && (d[i].y-d[i].height) < d[j].y ) {
                    inc++;
            } else
                break;
        i += inc; //jump the squares inside me
    }

    //take the solution already filter
    squares = sol;
}



JNIEXPORT jbooleanArray JNICALL Java_de_unidue_tagrecognition_OpenCV_square(
		JNIEnv* env, jobject thiz , jintArray photo_data, jint width,
		jint height) 
{
	LOGI("square enter\n");
	IplImage *image_data = getIplImageFromIntArray(env, photo_data, width, height);
	if (image_data == NULL) {
		return 0;
	}
 	Mat img (image_data,false); //NOT COPY OF IMAGE
 	vector<vector<Point> > squares;
    vector<SquareData> datasquares;
  
    //find squares on image
    //findSquares(img, squares, 0., 50., 5,true); //compresion de imagen
    findSquares(img, squares, datasquares , 0., 200., 3,false); //actually don't use the param
    stringstream os;
    os << "Square found before: " << squares.size() ;
    

    //filter squares inside other squares
    filterSquares(squares,datasquares);
    os << " later: " << squares.size() ;
    LOGI(os.str().c_str());
    //draw them
    drawSquares(img, squares);

    //cvReleaseImage(&image_data);
    LOGI("square done\n");
  	
    return getBmpImage(env,&img.operator IplImage());
}

/**************************************/

//hough transform
void hough( const Mat& src , int maxLineGap) {

    Mat dst, color_dst;

    /*
    void Canny(const Mat& image, Mat& edges, double threshold1, double threshold2, int apertureSize=3, bool L2gradient=false)
        Finds edges in an image using Canny algorithm.

        Parameters: 
        * image – Single-channel 8-bit input image
        * edges – The output edge map. It will have the same size and the same type as image
        * threshold1 – The first threshold for the hysteresis procedure
        * threshold2 – The second threshold for the hysteresis procedure
        * apertureSize – Aperture size for the Sobel() operator
        * L2gradient – Indicates, whether the more accurate norm should be 
        used to compute the image gradient magnitude ( L2gradient=true ), 
        or a faster default   norm   is enough ( L2gradient=false )
    */
    Canny( src, dst, 50, 200, 3 );
    cvtColor( dst, color_dst, CV_GRAY2BGR );
    /*
    void HoughLinesP(Mat& image, vector<Vec4i>& lines, double rho, double theta, int threshold, double minLineLength=0, double maxLineGap=0)
        Finds lines segments in a binary image using probabilistic Hough transform.
        Parameters: 
        * image – The 8-bit, single-channel, binary source image. The image may be modified by the function
        * lines – The output vector of lines. Each line is represented by a 4-element vector   , where   and  are the ending points of each line segment detected.
        * rho – Distance resolution of the accumulator in pixels
        * theta – Angle resolution of the accumulator in radians
        * threshold – The accumulator threshold parameter. Only those lines are returned that get enough votes (  )
        * minLineLength – The minimum line length. Line segments shorter than that will be rejected
        * maxLineGap – The maximum allowed gap between points on the same line to link them.
    */
    vector<Vec4i> lines;
    HoughLinesP( dst, lines, 1, CV_PI/180, 30/*80*/, 10, maxLineGap );
    for( size_t i = 0; i < lines.size(); i++ )
    {
        line( color_dst, Point(lines[i][0], lines[i][1]),
            Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 2, 8 );
    }

    
    stringstream os;
    os << maxLineGap;
    string file = "/mnt/sdcard/Pictures/MyCameraApp/hough" + os.str() + ".jpeg";

    imwrite(file,color_dst);
}

#ifdef __cplusplus
}
#endif

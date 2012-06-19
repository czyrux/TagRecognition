#include <jni.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "opencv2/nonfree/features2d.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <cv.h>

#include "opencv-image.h"
#include "opencv-squares.h"
#include "log.h"

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

/*
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
/*
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
/*
JNIEXPORT jboolean JNICALL Java_de_unidue_tagrecognition_OpenCV_releaseSourceImage(
		JNIEnv* env, jobject thiz ) 
{
	if (pImage != NULL) {
		cvReleaseImage(&pImage);
		pImage = NULL;
	}
}

/**************************************/
/*
JNIEXPORT void JNICALL Java_de_unidue_tagrecognition_OpenCV_extract(
		JNIEnv* env, jobject thiz) 
{
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
    drawKeypoints(img_original, keypoints, img_result, keypointColor, DrawMatchesFlags::DEFAULT);
  	drawKeypoints(img, keypoints, img, keypointColor, DrawMatchesFlags::DEFAULT);
  	
  	cvReleaseImage(&image_data);
  	//image_data = &img.operator IplImage();
  	LOGI("extract done\n");
  	
  	return getBmpImage(env,&img.operator IplImage());
}


/**************************************/


JNIEXPORT jbooleanArray JNICALL Java_de_unidue_tagrecognition_OpenCV_square(
		JNIEnv* env, jobject thiz , jintArray photo_data, jint width,
		jint height) 
{
    //load the image
    Image_data* data = getIplImageFromIntArray(env, photo_data, width, height);;
    IplImage *image_data = data->red;
	if (image_data == NULL) {
        LOGE("Image data couldn't be loaded.");
		return 0;
	}
    
    Mat img1 (data->src,false) , img2 (data->red,false) , img3 (data->blue,false) , img4 (data->green,false); //NOT COPY OF IMAGE
    string file1 = "/mnt/sdcard/Pictures/MyCameraApp/filter_src.jpeg";
    imwrite(file1,img1);
    file1 = "/mnt/sdcard/Pictures/MyCameraApp/filter_red.jpeg";
    imwrite(file1,img2);
    file1 = "/mnt/sdcard/Pictures/MyCameraApp/filter_green.jpeg";
    imwrite(file1,img4);
    file1 = "/mnt/sdcard/Pictures/MyCameraApp/filter_blue.jpeg";
    imwrite(file1,img3);
    

    //Convert IplImage to Mat
 	Mat img (image_data,false); //NOT COPY OF IMAGE
 	vector<Square> squares;
  
    //find squares on image
    findSquares(img, squares);
    stringstream os;
    os << "Square found before: " << squares.size() ;
    
    //draw all squares founded
    Mat cop = img.clone();
    drawSquares(cop, squares);
    string file = "/mnt/sdcard/Pictures/MyCameraApp/all_squares.jpeg";
    imwrite(file,cop);
    
    //remove squares inside others ones
    filterSquares(squares);

    os << " later: " << squares.size() ;
    LOGI(os.str().c_str());

    //draw them
    drawSquares(img, squares);

    //cut squares
    vector<Mat> subsquares;
    cutSquares(img,squares,subsquares);

    //cvReleaseImage(&image_data);
    cvReleaseImage(&data->src);
    cvReleaseImage(&data->green);
    cvReleaseImage(&data->blue);
    delete data;
  	
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

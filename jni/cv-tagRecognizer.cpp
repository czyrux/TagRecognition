#include <jni.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "opencv2/nonfree/features2d.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <cv.h>

#include "cv-image.h"
#include "cv-squares.h"
#include "cv-tag.h"
#include "cv-log.h"
#include "cv-constants.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************/

/*
 * error reporting
 */
void envDump(JNIEnv *env) {
  if (env->ExceptionCheck()) {
    env->ExceptionDescribe();
    env->ExceptionClear();
  }
}

/**************************************/
/*
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
*/

/**************************************/

JNIEXPORT void JNICALL Java_de_unidue_tagrecognition_JniWrapper_nativeSetup (JNIEnv* env, jobject thiz) {

    //Taking reference to the class
    jclass callbackClass = env->GetObjectClass(thiz);
    if (!callbackClass) {
        LOGE("setup//failed to discover class");
        envDump(env);
        return;
    }

    //Making global reference
    //callbackClass = (jclass) env->NewGlobalRef(tempClass);
    //env->DeleteLocalRef(tempClass);

    //Getting id's from variables
    rId = env->GetFieldID(callbackClass, "_RED_BOUNDARY", "I");
    if (!rId) {
        LOGE("setup//RED_BOUNDARY find failure");
        envDump(env);
        return;
    }
    gId = env->GetFieldID(callbackClass, "_GREEN_BOUNDARY", "I");
    if (!rId) {
        LOGE("setup//GREEN_BOUNDARY find failure");
        envDump(env);
        return;
    }
    bId = env->GetFieldID(callbackClass, "_BLUE_BOUNDARY", "I");
    if (!rId) {
        LOGE("setup//BLUE_BOUNDARY find failure");
        envDump(env);
        return;
    }

    //Get values from Java
    RED_BOUNDARY = (jint) env->GetObjectField(thiz, rId);
    GREEN_BOUNDARY = (jint) env->GetObjectField(thiz, gId);
    BLUE_BOUNDARY = (jint) env->GetObjectField(thiz, bId);

    //Update the values to java
    RED_BOUNDARY = 0;
    BLUE_BOUNDARY = -20;
    GREEN_BOUNDARY = -60;
    env->SetIntField(thiz, rId, RED_BOUNDARY);
    env->SetIntField(thiz, gId, GREEN_BOUNDARY);
    env->SetIntField(thiz, bId, BLUE_BOUNDARY);
}

/**************************************/


JNIEXPORT jbooleanArray JNICALL Java_de_unidue_tagrecognition_JniWrapper_tagRecognizer(
		JNIEnv* env, jobject thiz , jintArray photo_data, jint width,
		jint height) 
{
    //load the image
    Image_data* data = getIplImageFromIntArray(env, photo_data, width, height);
    if (data == NULL) {
        LOGE("Image data couldn't be loaded.");
        return 0;
    }

    cv::Mat img1 (data->src,false) , img2 (data->rImage,false) , img3 (data->bImage,false) , img4 (data->gImage,false); //NOT COPY OF IMAGE
    std::string file1 = PATH + "filter_src.jpeg";
    //cv::imwrite(file1,img1);
    file1 = PATH + "filter_red.jpeg";
    cv::imwrite(file1,img2);
    file1 = PATH + "filter_green.jpeg";
    cv::imwrite(file1,img4);
    file1 =  PATH + "filter_blue.jpeg";
    cv::imwrite(file1,img3);
    

    //Convert IplImage to Mat
    cv::Mat img (data->rImage,false); //NOT COPY OF IMAGE
    cv::Mat img_org (data->src,false); //NOT COPY OF IMAGE
    
    //find squares on image
    std::vector<Square> squares;
    findSquares(img, squares);

    //log
    std::stringstream os;
    os << "Square found before: " << squares.size() ;
    
    //remove squares inside others ones
    filterSquares(squares);

    //log
    os << " later: " << squares.size() ;
    LOGI(os.str().c_str());

    //cut squares
    std::vector<std::vector<cv::Mat> > subsquares;
    cutSquares(data,squares,subsquares);

    //recognize tag's in squares
    decodeTags(subsquares);

    //draw them
    drawSquares(img_org, squares);
    std::string file = PATH + "src_squares.jpeg";
    cv::imwrite(file,img_org);
    

    //release memory
    if (data->src)  cvReleaseImage(&data->src);
    if (data->rImage)  cvReleaseImage(&data->rImage);
    if (data->gImage)cvReleaseImage(&data->gImage);
    if (data->bImage) cvReleaseImage(&data->bImage);
    delete data;
  	
    //return the image with the squares
    //return getBmpImage(env,&img_org.operator IplImage());
    return 0;
}

/**************************************/

JNIEXPORT jboolean JNICALL Java_de_unidue_tagrecognition_JniWrapper_calibrate(
        JNIEnv* env, jobject thiz , jintArray photo_data, jint width,
        jint height) 
{
    //load the image
    Image_data* data = getIplImageFromIntArray(env, photo_data, width, height);
    if (data == NULL) {
        LOGE("Image data couldn't be loaded.");
        return 0;
    }

    cv::Mat img1 (data->src,false) , img2 (data->rImage,false) , img3 (data->bImage,false) , img4 (data->gImage,false); //NOT COPY OF IMAGE
    std::string file1 = "/mnt/sdcard/Pictures/MyCameraApp/filter_src.jpeg";
    //cv::imwrite(file1,img1);
    file1 = "/mnt/sdcard/Pictures/MyCameraApp/filter_red.jpeg";
    cv::imwrite(file1,img2);
    file1 = "/mnt/sdcard/Pictures/MyCameraApp/filter_green.jpeg";
    cv::imwrite(file1,img4);
    file1 = "/mnt/sdcard/Pictures/MyCameraApp/filter_blue.jpeg";
    cv::imwrite(file1,img3);
    

    //Convert IplImage to Mat
    cv::Mat img (data->rImage,false); //NOT COPY OF IMAGE
    cv::Mat img_org (data->src,false); //NOT COPY OF IMAGE
    
    //find squares on image
    std::vector<Square> squares;
    findSquares(img, squares);

    //log
    std::stringstream os;
    os << "Square found before: " << squares.size() ;
    
    //remove squares inside others ones
    filterSquares(squares);

    //log
    os << " later: " << squares.size() ;
    LOGI(os.str().c_str());

    //cut squares
    std::vector<std::vector<cv::Mat> > subsquares;
    cutSquares(data,squares,subsquares);

    //recognize tag's in squares
    decodeTags(subsquares);

    //draw them
    drawSquares(img_org, squares);
    std::string file = "/mnt/sdcard/Pictures/MyCameraApp/src_squares.jpeg";
    cv::imwrite(file,img_org);
    

    //release memory
    if (data->src)  cvReleaseImage(&data->src);
    if (data->rImage)  cvReleaseImage(&data->rImage);
    if (data->gImage)cvReleaseImage(&data->gImage);
    if (data->bImage) cvReleaseImage(&data->bImage);
    delete data;
    
    //return the image with the squares
    //return getBmpImage(env,&img_org.operator IplImage());
    return 0;
}

/**************************************/

#ifdef __cplusplus
}
#endif

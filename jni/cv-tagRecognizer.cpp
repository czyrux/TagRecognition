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
//#include "cv-constants.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************/

const std::string TEMPLATE_TAG = "12321113";

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


/**************************************/
/*
JNIEXPORT void JNICALL Java_de_unidue_tagrecognition_JniWrapper_nativeSetup (JNIEnv* env, jobject thiz) 
{

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
*/
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

    //looking for tags in the image
    std::vector<std::string> tags = findTags(data);

    //LOG
    for ( int i=0 ; i<tags.size() ; i++ ) {
        std::stringstream os;
        os << tags[i];
        if ( tags[i] == "12321113" )
            os << "----> CHECKED";
        LOGI(os.str().c_str());
    }

    //release memory
    if (data->src)  cvReleaseImage(&data->src);
    if (data->rImage)  cvReleaseImage(&data->rImage);
    if (data->gImage)cvReleaseImage(&data->gImage);
    if (data->bImage) cvReleaseImage(&data->bImage);
    delete data;

    return 0;
}

/**************************************/

JNIEXPORT jboolean JNICALL Java_de_unidue_tagrecognition_JniWrapper_calibrate(
        JNIEnv* env, jobject thiz , jintArray photo_data, jint width,
        jint height) 
{
    //Initialize values of boundaries
    RED_BOUNDARY = BLUE_BOUNDARY = GREEN_BOUNDARY = 50;

    bool exit_ = false;
    bool calibrated = false;
    int i = 0;

    while (!exit_) {
        //log
        std::stringstream os;

        //load the image
        Image_data* data = getIplImageFromIntArray(env, photo_data, width, height);
        if (data == NULL) {
            LOGE("Image data couldn't be loaded.");
            return 0;
        }

        //looking for tags in the image
        std::vector<std::string> tags = findTags(data);

        //check if found something
        if ( !tags.empty() ) {
            //log
            os << "FOUND_";
            //get the first one. Suppose to be only one
            std::string tag = tags.front();
            LOGI(tag.c_str());
            //checked if it's correct
            if (tag == TEMPLATE_TAG) {
                calibrated = true;
                exit_ = true;
            }
            else {
                //adjust values of boundaries
                adjustRGBBoundaries(tag,TEMPLATE_TAG);
            }
            
        }else {
            //log
            os<<"NOT_FOUND_";
            RED_BOUNDARY -= 5; //adjust to find red border
        }

        //log
        os << "itr:" << i << " R: " << RED_BOUNDARY << " G: " << GREEN_BOUNDARY << " B: " << BLUE_BOUNDARY;
        LOGI(os.str().c_str());
        i++;
        //condition of exit
        if (RED_BOUNDARY < - 100 || BLUE_BOUNDARY < -100 || GREEN_BOUNDARY < -100 || i >= 10 ) {
            exit_ = true;
        } 

        //release memory
        if (data->src)  cvReleaseImage(&data->src);
        if (data->rImage)  cvReleaseImage(&data->rImage);
        if (data->gImage)cvReleaseImage(&data->gImage);
        if (data->bImage) cvReleaseImage(&data->bImage);
        delete data;
    }

    /*
    //Update the values founded to java
    env->SetIntField(thiz, rId, RED_BOUNDARY);
    env->SetIntField(thiz, gId, GREEN_BOUNDARY);
    env->SetIntField(thiz, bId, BLUE_BOUNDARY);
    */
    return calibrated;
}

/**************************************/

#ifdef __cplusplus
}
#endif

#include <jni.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "opencv2/nonfree/features2d.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <cv.h>

#include "cv-image.h"
#include "cv-rect.h"
#include "cv-tag.h"
#include "cv-log.h"
#include "cv-conf.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************/

static std::string TEMPLATE_TAG = "12321113";

/**************************************/

JNIEXPORT void JNICALL Java_de_unidue_tagrecognition_NDKWrapper_setup (JNIEnv* env, jobject thiz,
    jint rows , jint cols , jfloat tag_width, jfloat tag_height , jfloat tag_border , jstring templateT , jboolean debug) 
{
    //variables from conf.h
    COLS = cols;
    ROWS = rows;
    WIDTH_BORDER = tag_border/tag_width;
    HEIGHT_BORDER = tag_border/tag_height;

    //TEMPLATE_TAG = templateT;
    const char *s = env->GetStringUTFChars(templateT,NULL);
    TEMPLATE_TAG=s;
    env->ReleaseStringUTFChars(templateT,s);
    DEBUG = debug;
}

/**************************************/


JNIEXPORT jstring JNICALL Java_de_unidue_tagrecognition_NDKWrapper_tagRecognizer(
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
    std::vector<Tag> tags = findTags(data);

    //Build string to return to java
    std::stringstream stream;
    for ( int i=0 ; i<tags.size() ; i++ ) {
        stream << tags[i].x << "/" << tags[i].y << "/" << tags[i].code;
        if ( i< tags.size()-1)
            stream << "&";
    }

    //release memory
    if (data->src)  cvReleaseImage(&data->src);
    if (data->rImage)  cvReleaseImage(&data->rImage);
    if (data->gImage)cvReleaseImage(&data->gImage);
    if (data->bImage) cvReleaseImage(&data->bImage);
    delete data;


    return env->NewStringUTF((char*)stream.str().c_str());
}

/**************************************/

JNIEXPORT jboolean JNICALL Java_de_unidue_tagrecognition_NDKWrapper_calibrate(
        JNIEnv* env, jobject thiz , jintArray photo_data, jint width,
        jint height) 
{
    //Backups of old values
    int backUpR = RED_THRESHOLD, backUpB = BLUE_THRESHOLD, backUpG = GREEN_THRESHOLD;

    //Initialize values of threshold
    RED_THRESHOLD = BLUE_THRESHOLD = GREEN_THRESHOLD = 0;

    if (DEBUG) {
        LOGI("Calibrating proces...");
        LOGI(TEMPLATE_TAG.c_str());
    }

    bool exit_ = false;
    bool calibrated = false;
    int i=0;
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
        std::vector<Tag> tags = findTags(data,true);//true means that the tags are already oriented

        //check if found something
        if ( !tags.empty() ) {
            //get the first one. Suppose to be only one
            std::string tag = tags.front().code;
            LOGI(tag.c_str());
            //checked if it's correct
            if (tag == TEMPLATE_TAG) {
                calibrated = true;
                exit_ = true;
            }
            else {
                //adjust values of boundaries
                adjustRGBThreshold(tag,TEMPLATE_TAG);
            }
            
        }else {
            RED_THRESHOLD -= 10; //adjust to find red border
            BLUE_THRESHOLD -= 5; //or to find the blue square
        }

        //condition of exit
        if (RED_THRESHOLD <= - 100 || BLUE_THRESHOLD <= -100 || GREEN_THRESHOLD <= -100 ) {
            exit_ = true;
            //Restore original values
            RED_THRESHOLD = backUpR;
            BLUE_THRESHOLD= backUpB;
            GREEN_THRESHOLD = backUpG;
        } 

        //log
        os << "itr: " << i << " | R: " << RED_THRESHOLD << " | G: " << GREEN_THRESHOLD << " | B: " << BLUE_THRESHOLD << " |";
        i++;
        LOGI(os.str().c_str());
        
        //release memory
        if (data->src)  cvReleaseImage(&data->src);
        if (data->rImage)  cvReleaseImage(&data->rImage);
        if (data->gImage)cvReleaseImage(&data->gImage);
        if (data->bImage) cvReleaseImage(&data->bImage);
        delete data;
    }

    return calibrated;
}

/**************************************/

#ifdef __cplusplus
}
#endif

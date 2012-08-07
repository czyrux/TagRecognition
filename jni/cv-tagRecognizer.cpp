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
JNIEXPORT void JNICALL Java_de_unidue_tagrecognition_NDKWrapper_variablesSetup (JNIEnv* env, jobject thiz) 
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
    env->SetIntField(thiz, rId, RED_BOUNDARY);
    env->SetIntField(thiz, gId, GREEN_BOUNDARY);
    env->SetIntField(thiz, bId, BLUE_BOUNDARY);
    
}
*/
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

    //LOG
    for ( int i=0 ; i<tags.size() ; i++ ) {
        std::stringstream os;
        os << "x: " << tags[i].x << " y:" << tags[i].y << " code:" << tags[i].code;
        if ( tags[i].code == "12321113" )
            os << "----> CHECKED";
        LOGI(os.str().c_str());
    }

    //Build string to return to java
    std::stringstream stream;
    for ( int i=0 ; i<tags.size() ; i++ ) {
        stream << tags[i].x << "/" << tags[i].y << "/" << tags[i].code;
        if ( i< tags.size()-1)
            stream << "&";
    }

    //LOGI(stream.str().c_str());

    //release memory
    if (data->src)  cvReleaseImage(&data->src);
    if (data->rImage)  cvReleaseImage(&data->rImage);
    if (data->gImage)cvReleaseImage(&data->gImage);
    if (data->bImage) cvReleaseImage(&data->bImage);
    delete data;


    //return NewStringUTF(env, (const jchar*)stream.str().c_str());
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

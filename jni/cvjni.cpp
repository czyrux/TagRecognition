/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <string.h>
#include <jni.h>
#include <android/log.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "opencv2/nonfree/features2d.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <cv.h>
#include <cxcore.h>
#include <vector>
#include "bmpfmt.h"
#define ANDROID_LOG_VERBOSE ANDROID_LOG_DEBUG
#define LOG_TAG "CVJNI"
#define LOGV(...) __android_log_print(ANDROID_LOG_SILENT, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#ifdef __cplusplus
extern "C" {
#endif

using namespace cv;
/**************************************/
	
IplImage* pImage = NULL;
IplImage* loadPixels(int* pixels, int width, int height);
IplImage* getIplImageFromIntArray(JNIEnv* env, jintArray array_data,
		jint width, jint height);

/**************************************/

JNIEXPORT void JNICALL Java_de_unidue_tagrecognition_OpenCV_extractSURFFeature(
		JNIEnv* env, jobject thiz) 
{
	//LOGI("extract enter");
	IplImage *pWorkImage=cvCreateImage(cvGetSize(pImage),IPL_DEPTH_8U,1);
	//LOGI("extract create work");
	cvCvtColor(pImage,pWorkImage,CV_BGR2GRAY);
	//LOGI("extract color");
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq *imageKeypoints = 0, *imageDescriptors = 0;
	CvSURFParams params = cvSURFParams(2000, 0);
	//LOGI("before");
	//cvExtractSURF( pWorkImage, 0, &imageKeypoints, &imageDescriptors, storage, params );
	try
	{
		cvExtractSURF( pWorkImage, 0, &imageKeypoints, NULL, storage, params );
	}
	catch( cv::Exception& e )
	{
	    const char* err_msg = e.what();
	    //LOGI(err_msg);
	}
	//LOGI("after");
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
	//LOGI("extract done");
}

/**************************************/

JNIEXPORT jboolean JNICALL Java_de_unidue_tagrecognition_OpenCV_setSourceImage(
		JNIEnv* env, jobject thiz, jintArray photo_data, jint width,
		jint height) {
	if (pImage != NULL) {
		cvReleaseImage(&pImage);
		pImage = NULL;
	}
	pImage = getIplImageFromIntArray(env, photo_data, width, height);
	if (pImage == NULL) {
		return 0;
	}
	//LOGI("Load Image Done.");
	return 1;
}

/**************************************/

JNIEXPORT jbooleanArray JNICALL Java_de_unidue_tagrecognition_OpenCV_getSourceImage(
		JNIEnv* env, jobject thiz) 
{
	//LOGI("En get.");
	if (pImage == NULL) {
		//LOGE("No source image.");
		return 0;
	}
	cvFlip(pImage);
	//LOGI("flip.");
	int width = pImage->width;
	int height = pImage->height;
	int rowStep = pImage->widthStep;
	int headerSize = 54;
	int imageSize = rowStep * height;
	int fileSize = headerSize + imageSize;
	unsigned char* image = new unsigned char[fileSize];
	struct bmpfile_header* fileHeader = (struct bmpfile_header*) (image);
	//LOGI("fileheader.");
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
	//LOGI("memcpy.");
	jbooleanArray bytes = env->NewBooleanArray(fileSize);
	//LOGI("env- new.");
	if (bytes == 0) {
		//LOGE("Error in creating the image.");
		delete[] image;
		return 0;
	}
	env->SetBooleanArrayRegion(bytes, 0, fileSize, (jboolean*) image);
	//LOGI("env- set.");
	delete[] image;
	return bytes;
}

/**************************************/

IplImage* loadPixels(int* pixels, int width, int height) {
	int x, y;
	IplImage *img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	unsigned char* base = (unsigned char*) (img->imageData);
	unsigned char* ptr;
	for (y = 0; y < height; y++) {
		ptr = base + y * img->widthStep;
		for (x = 0; x < width; x++) {
			// blue
			ptr[3 * x] = pixels[x + y * width] & 0xFF;
			// green
			ptr[3 * x + 1] = pixels[x + y * width] >> 8 & 0xFF;
			// blue
			ptr[3 * x + 2] = pixels[x + y * width] >> 16 & 0xFF;
		}
	}
	return img;
}

/**************************************/

IplImage* getIplImageFromIntArray(JNIEnv* env, jintArray array_data,
		jint width, jint height) 
{
	int *pixels = env->GetIntArrayElements(array_data, 0);
	if (pixels == 0) {
		//LOGE("Error getting int array of pixels.");
		return 0;
	}
	IplImage *image = loadPixels(pixels, width, height);
	env->ReleaseIntArrayElements(array_data, pixels, 0);
	if (image == 0) {
		//LOGE("Error loading pixel array.");
		return 0;
	}
	return image;
}

/**************************************/

JNIEXPORT void JNICALL Java_de_unidue_tagrecognition_OpenCV_extract(
		JNIEnv* env, jobject thiz) 
{
	LOGI("extract enter SIFT\n");
 	cv::Mat m (pImage,true);
 	Mat grey;
 	cvtColor(m, grey, CV_BGR2GRAY);
  	vector<KeyPoint> keypoints;

  	// Detect the keypoints
  	
  	//SurfFeatureDetector detector(400.);
  	//FeatureDetector *detector = new SiftFeatureDetector(400, 500);
  	//FeatureDetector *detector = new SurfFeatureDetector(400.);
  	FeatureDetector *detector = new FastFeatureDetector(50);
  	detector->detect(grey,keypoints);
  	Mat outputImage;
  	Scalar keypointColor = Scalar(0, 0, 255);     // color keypoints.
  	drawKeypoints(m, keypoints, outputImage, keypointColor, DrawMatchesFlags::DEFAULT);
  	LOGI("draw done");
  	imwrite( "/mnt/sdcard/Pictures/MyCameraApp/test.jpg", outputImage );
  	
  	LOGI("extract done\n");
}

#ifdef __cplusplus
}
#endif


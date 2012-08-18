/**
 * @file cv-image.h
 * @brief Implementation of operations to read and write images from Java to C++ and viceversa via
 * JNI. It also split the image read in three channels of colour.
 * @author Antonio Manuel Gutierrez Martinez
 * @version 1.0
 */

#ifndef IMAGE_H
#define IMAGE_H

#include <jni.h>
#include <stdint.h>
#include <cv.h>
#include <cxcore.h>
#include "cv-log.h"
#include "cv-conf.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Threshold value used to split the red channel on an image. */
extern int RED_THRESHOLD;
/** Threshold value used to split the blue channel on an image. */
extern int BLUE_THRESHOLD;
/** Threshold value used to split the green channel on an image. */
extern int GREEN_THRESHOLD;

/**
 * Header of bmp file
 */
struct bmpfile_header {
  unsigned char magic[2];
  uint32_t filesz;
  uint16_t creator1;
  uint16_t creator2;
  uint32_t bmp_offset;
};

struct bmp_dib_v3_header_t {
  uint32_t header_sz;
  uint32_t width;
  uint32_t height;
  uint16_t nplanes;
  uint16_t bitspp;
  uint32_t compress_type;
  uint32_t bmp_bytesz;
  uint32_t hres;
  uint32_t vres;
  uint32_t ncolors;
  uint32_t nimpcolors;
};

/**
 * Struct that holds the data from image as well as its images channel. The image
 * are stored as a pointer to an IplImage struct.
 */
struct Image_data {
  IplImage* src;
  IplImage* rImage;
  IplImage* bImage;
  IplImage* gImage;
};


/**
 * Read the pixels of an image from an array of integers and return the Image_data 
 * struct associated.
 * @param pixels Image as an array of integers.
 * @param width Width of image.
 * @param height Height of image.
 * @param red_filter Boolean value that indicates if the red channel must be calculated.
 * @param blue_filter Boolean value that indicates if the blue channel must be calculated.
 * @param green_filter Boolean value that indicates if the green channel must be calculated.
 * @return Pointer to image_data struct
 */
Image_data* loadPixelsFiltered(int* pixels, int width, int height , 
  bool red_filter=true , bool green_filter=true , bool blue_filter=true);

/**
 * Convert a image stored in a jintArray in a Image_data struct.
 * @param env Variable used by JNI to handle the communication between Java and C++.
 * @param array_data Image as a jintArray.
 * @param width Width of image.
 * @param height Height of image.
 * @return Pointer to image_data struct or NULL if the process fails.
 */
Image_data* getIplImageFromIntArray(JNIEnv* env, jintArray array_data , int width, int height );

/**
 * Convert an IplImage in a jbooleanArray with the format of an bmp image.
 * @param env Variable used by JNI to handle the communication between Java and C++.
 * @param pImage pointer to an image in format IplImage.
 * Code taken from
 */
jbooleanArray getBmpImage( JNIEnv* env, IplImage* pImage );

#ifdef __cplusplus
}
#endif

#endif //IMAGE_H

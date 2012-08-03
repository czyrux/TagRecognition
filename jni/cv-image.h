#ifndef IMAGE_H
#define IMAGE_H

#include <jni.h>
#include <stdint.h>
#include <cv.h>
#include <cxcore.h>
#include "cv-log.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int RED_THRESHOLD;
extern int BLUE_THRESHOLD;
extern int GREEN_THRESHOLD;

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

struct Image_data {
  IplImage* src;
  IplImage* rImage;
  IplImage* bImage;
  IplImage* gImage;
};

//
//IplImage* loadPixels(int* pixels, int width, int height);

//
Image_data* loadPixelsFiltered(int* pixels, int width, int height , 
  bool red_filter=true , bool green_filter=true , bool blue_filter=true);

//
Image_data* getIplImageFromIntArray(JNIEnv* env, jintArray array_data , int width, int height );

//
jbooleanArray getBmpImage( JNIEnv* env, IplImage* pImage );

#ifdef __cplusplus
}
#endif

#endif //IMAGE_H

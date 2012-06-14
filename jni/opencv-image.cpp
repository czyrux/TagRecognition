#include "opencv-image.h"



/**************************************/

IplImage* loadPixelsFilter(int* pixels, int width, int height) {
	int x, y;
	IplImage *img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	unsigned char* base = (unsigned char*) (img->imageData);
	unsigned char* ptr , R , G , B ;
	int diff;
	for (y = 0; y < height; y++) {
		ptr = base + y * img->widthStep;
		for (x = 0; x < width; x++) {
			B = pixels[x + y * width] & 0xFF;
			G = pixels[x + y * width] >> 8 & 0xFF;
			R = pixels[x + y * width] >> 16 & 0xFF;
			//to take red colours, have more Red than Green and Blue together
			diff = R - (G+B);
			if ( diff > 0 ) { 
				//red colours on black
				ptr[3 * x] = ptr[3 * x + 1] = ptr[3 * x + 2] = 0x00;
			}
			else{ 
				//otherwise on white
				ptr[3 * x] = ptr[3 * x + 1] = ptr[3 * x + 2] = 0xFF;
			}
		}
	}

	/* Rotate operations*/
	/*Mat m (img,true);

	Point2f src_center(m.cols/2.0F, m.rows/2.0F);
    Mat rot_mat = getRotationMatrix2D(src_center,-90, 1.0);
    Mat dst;
    warpAffine(m, dst, rot_mat, m.size());
	img = cvCloneImage(&dst.operator IplImage());
	*/
	return img;
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
			// red
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
		LOGE("Error getting int array of pixels.");
		return 0;
	}
	IplImage *image = loadPixelsFilter(pixels, width, height);
	env->ReleaseIntArrayElements(array_data, pixels, 0);
	if (image == 0) {
		LOGE("Error loading pixel array.");
		return 0;
	}
	return image;
}

/**************************************/

jbooleanArray getBmpImage( JNIEnv* env, IplImage* pImage )
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

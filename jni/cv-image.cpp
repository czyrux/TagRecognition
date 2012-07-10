#include "cv-image.h"

/**************************************/

Image_data* loadPixelsFilter(int* pixels, int width, int height,
								bool red_filter , bool green_filter , bool blue_filter) 
{
	IplImage *img = NULL, *img_red = NULL, *img_green = NULL, *img_blue= NULL;
	uchar *base = NULL , *ptr = NULL, R , G , B ;
	int diff;

	//Create images to store data
	if(red_filter) 	img_red 	= cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	if(green_filter)img_green 	= cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1); 
	if(blue_filter) img_blue 	= cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	base = (uchar*) (img->imageData);

	//Get pixels
	for (int y = 0; y < height; y++) 
	{
		ptr = base + y * img->widthStep;
		for (int x = 0; x < width; x++) 
		{
			B = pixels[x + y * width] & 0xFF;
			G = pixels[x + y * width] >> 8 & 0xFF;
			R = pixels[x + y * width] >> 16 & 0xFF;

			//Normal image
			ptr[3 * x] = B;// blue
			ptr[3 * x + 1] = G;// green
			ptr[3 * x + 2] = R;// red
			
			//to take RED colours, have more Red than Green and Blue together
			if ( red_filter ) {
				diff = R - (G+B);
				((uchar *)(img_red->imageData + y*img_red->widthStep))[x] 
				= (diff > RED_BOUNDARY)? 0x00 : 0xFF ; //black : white
			}
			
			//to take GREEN colours
			if ( green_filter ) {
				diff = G - (R+B);
				((uchar *)(img_green->imageData + y*img_green->widthStep))[x] 
				= ( diff > GREEN_BOUNDARY )? 0x00 : 0xFF ; //black : white
			}
			
			//to take BLUE colours
			if ( blue_filter ) {
				diff = B - (G+R);
				((uchar *)(img_blue->imageData + y*img_blue->widthStep))[x]= 
				( diff > BLUE_BOUNDARY )? 0x00 : 0xFF ; //black : white
			}
		}
	}

	//assign pointer to return
	Image_data* image = new Image_data;
	image->src = img;
	image->rImage = img_red;
	image->bImage = img_blue;
	image->gImage = img_green;

	return image;
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

Image_data* getIplImageFromIntArray(JNIEnv* env, jintArray array_data,
		jint width, jint height )
{
	int *pixels = env->GetIntArrayElements(array_data, 0);
	if (pixels == NULL) {
		LOGE("Error getting int array of pixels.");
		return 0;
	}

	Image_data* img = loadPixelsFilter(pixels, width, height,true,true,true);
	env->ReleaseIntArrayElements(array_data, pixels, 0);
	if (img->src == NULL) {
		LOGE("Error loading pixel array.");
		return 0;
	}
	return img;
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

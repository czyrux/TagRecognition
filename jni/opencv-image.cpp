#include "opencv-image.h"

/**************************************/

Image_data* loadPixelsFilter(int* pixels, int width, int height,
								bool red_filter , bool green_filter , bool blue_filter) 
{
	int x, y;
	unsigned char *base = NULL, *base_red = NULL, *base_blue = NULL, *base_green= NULL;
	unsigned char *ptr = NULL, *ptr_r = NULL, *ptr_b = NULL, *ptr_g = NULL, R , G , B ;
	int diff;
	IplImage *img = NULL, *img_red = NULL, *img_green = NULL, *img_blue= NULL;

	//Create images to store data
	img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	base = (unsigned char*) (img->imageData);
	if (red_filter) {
		img_red = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
		base_red = (unsigned char*) (img_red->imageData);
	} 
	if (green_filter) {
		img_green = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
		base_green = (unsigned char*) (img_green->imageData);
	} 
	if (blue_filter) {
		img_blue = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
		base_blue = (unsigned char*) (img_blue->imageData);
	}
	
	//Get pixels
	for (y = 0; y < height; y++) 
	{
		ptr = base + y * img->widthStep;
		if (red_filter) ptr_r = base_red + y * img->widthStep;
		if (green_filter)ptr_g = base_green + y * img->widthStep;
		if (blue_filter)ptr_b = base_blue + y * img->widthStep;
		for (x = 0; x < width; x++) 
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
				if ( diff > 0 ) { //if red colours
					ptr_r[3 * x] = ptr_r[3 * x + 1] = ptr_r[3 * x + 2] = 0x00; //black
				}
				else{ //otherwise
					ptr_r[3 * x] = ptr_r[3 * x + 1] = ptr_r[3 * x + 2] = 0xFF;//white
				}
			}
			
			//to take GREEN colours
			if ( green_filter ) {
				diff = G - (R+B);
				if ( diff > -20 ) { 
					ptr_g[3 * x] = ptr_g[3 * x + 1] = ptr_g[3 * x + 2] = 0x00;//black
				}
				else{ //otherwise on white
					ptr_g[3 * x] = ptr_g[3 * x + 1] = ptr_g[3 * x + 2] = 0xFF;//white
				}
			}
			
			//to take BLUE colours
			if ( blue_filter ) {
				diff = B - (G+R);
				if ( diff > 0 ) {//blue colours on black
					ptr_b[3 * x] = ptr_b[3 * x + 1] = ptr_b[3 * x + 2] = 0x00;//black
				}
				else{//otherwise on white
					ptr_b[3 * x] = ptr_b[3 * x + 1] = ptr_b[3 * x + 2] = 0xFF;//white
				}
			}
		}
	}

	//assign pointer
	Image_data* image = new Image_data;
	image->src = img;
	image->red = img_red;
	image->blue = img_blue;
	image->green = img_green;

	/* ROTATE IMAGE */
	/*Mat m (img,true);
	Point2f src_center(m.cols/2.0F, m.rows/2.0F);
    Mat rot_mat = getRotationMatrix2D(src_center,-90, 1.0);
    Mat dst;
    warpAffine(m, dst, rot_mat, m.size());
	img = cvCloneImage(&dst.operator IplImage());
	*/
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

	Image_data* img = loadPixelsFilter(pixels, width, height,true,false,false);
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

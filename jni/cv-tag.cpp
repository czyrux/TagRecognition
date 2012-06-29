#include "cv-tag.h"

/**************************************/

std::vector<std::string> decodeTags (const std::vector<std::vector<cv::Mat> >& subsquares ) 
{
}

/**************************************/

//HACERLA PARA UN CUADRADO ALREDEDOR DEL PUNTO
bool checkPoint ( const cv::Mat& image , int y , int x ) {
	if ( image.at<uchar>(y, x) == 0x00 ||
		image.at<uchar>(y-1, x) == 0x00 || image.at<uchar>(y+1, x) == 0x00 || 
		image.at<uchar>(y, x-1) == 0x00 || image.at<uchar>(y, x-1) == 0x00
		)
		return true;
	else
		return false;
}

std::string decodeTag ( std::vector<cv::Mat>& subsquares ) {
	std::string tag;
	LOGI("decodeTag");
	if ( subsquares.size() != 3) {
    	return 0;
    }
    //Matrix's for the three spaces of colour
	cv::Mat rImage = subsquares[0] , gImage = subsquares[1] , bImage = subsquares[2] ;
	
	//CONSTANT FOR TAGS
	int COLS = 4 , ROWS = 3;

	//Value of increment for the index
	int incWidth = rImage.cols/COLS+1 , incHeight = rImage.rows/ROWS ;

	if (rImage.rows > rImage.cols) {
		LOGE(" ABORT more rows than cols");
    	return 0;
    }

    //y height-rows , x width-cols
    //Check the tag
    uchar value;
    std::vector<std::vector<int> > v;
    v.resize(ROWS);
	for( int i = 0, y = incHeight/2; y < rImage.rows; y += incHeight , i++ )
	{
		v[i].resize(COLS);
		LOGI("resize");
    	for( int j= 0 , x = incWidth/2; x < rImage.cols; x += incWidth , j++ )
    	{

        	
        	//check for red circle
        	if (/*rImage.at<uchar>(y, x) == 0x00*/ checkPoint(rImage,y,x) ) {
        		v[i][j] = 1;
        	}
        	//if there is not red, check for blue
        	else if (/*bImage.at<uchar>(y, x) == 0x00*/ checkPoint(bImage,y,x)) {
        		v[i][j] = 2;
        	}
        	//if neither blue, check for green. More voluble to detect other like him
        	else if (/*gImage.at<uchar>(y, x) == 0x00*/ checkPoint(gImage,y,x)) {
        		v[i][j] = 3;
        	}
        	//not colour detected
        	else
        		v[i][j] = 0;

        	//cv::Point p(x,y); //(x,y)
        	//cv::circle(rImage, p , 3, cv::Scalar(0,0,255),1,CV_AA);
        	//cv::circle(gImage, p , 3, cv::Scalar(0,0,255),1,CV_AA);
			//cv::circle(bImage, p , 3, cv::Scalar(0,0,255),1,CV_AA);
    	}
	}

	//Print matrix
	for ( int i=0 ; i<v.size() ; i++ ) {
		std::stringstream os;
		for (int j=0 ; j<v[0].size() ; j++ ) {
    		os << " " << v[i][j];
		}
		LOGI(os.str().c_str());
	}
	
	std::string file = "/mnt/sdcard/Pictures/MyCameraApp/pointsR.jpeg";
    cv::imwrite(file,rImage);
    file = "/mnt/sdcard/Pictures/MyCameraApp/pointsG.jpeg";
    cv::imwrite(file,gImage);
    file = "/mnt/sdcard/Pictures/MyCameraApp/pointsB.jpeg";
    cv::imwrite(file,bImage); 
    

	return tag;
} 
/*
int indx=0;
 const double start = 1/((double)_length*2), step = 2/((double)_length*2);
 
 for (int i=0; i < _size; i++)
  _tag[i] = W;
 
 LOGI("decoding");
 
 
 for (double i = start; i < 1.0; i+=step) {
    
  for (double j = start; j < 1.0; j += step, indx++ ) {
     
   for (int l = -2; l < 2 && rImage.rows * i + l < rImage.rows * (i  + start) ; l++){
      
    if(rImage.rows * i + l > rImage.rows * (i == start? 0 :  i - start)){
     
     for (int k = -2; k < 2 && 3 * ((int)rImage.cols*j + k) < 3 * ((int)rImage.cols * ((j < 1.0)? j + start : j ))  ; k++) {
      
      if(3 * ((int)gImage.cols *j + k) < 3 * ((int)gImage.cols * ( j - start)))
       continue;
      
       unsigned char* ptrR = rImage.ptr<unsigned char>((int)(rImage.rows * i) + l);
       unsigned char* ptrG = gImage.ptr<unsigned char>((int)(gImage.rows * i) + l);
       unsigned char* ptrB = bImage.ptr<unsigned char>((int)(bImage.rows * i) + l);
  
      
      if(ptrR[3 * (int)(rImage.cols * j + k) + 2] < 50 ){//Red
       _tag[indx]= R;
       //ptrR[3 * (int)(rImage.cols * j + k) + 2] = 0xFF;
       
      }
      if(ptrG[3 * (int)(gImage.cols * j + k)] < 50 ){ //Green
       _tag[indx] = G;
       //ptrG[3 * (int)(rImage.cols * j + k) ] = 0xFF;
      }
      if(ptrB[3 * (int)(bImage.cols * j + k) + 1] < 50 ){ //Blue
       _tag[indx] = B;
       //ptrB[3 * (int)(rImage.cols * j + k) + 1] = 0xFF;
      }
     
     }
    }
   }
  
  }
 }
 //LOGI("...out decodedImageToVector");
}
*/

/**************************************/

/**************************************/

/**************************************/

/**************************************/

/**************************************/
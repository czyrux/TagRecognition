#include "cv-tag.h"

/**************************************/

std::vector<std::string> decodeTags (const std::vector<std::vector<cv::Mat> >& subsquares ) 
{
    std::vector<std::string> solution;
    solution.clear();
    for ( int i=0 ; i<subsquares.size() ; i++ ) {
        solution.push_back(decodeTag(subsquares[i],i) );
    }

    return solution;
}

/**************************************/

//HACERLA PARA UN CUADRADO ALREDEDOR DEL PUNTO
bool checkPoint ( const cv::Mat& image , int y , int x , uchar colour ) {
    //return (image.at<uchar>(y, x) == colour );  
    for ( int i=-1 ; i<1 ; i++ )
        for ( int j=-1 ; j<1 ; j++ )
            if ( image.at<uchar>(y+i, x+j) == colour )
                return true;
    return false;
}

/**************************************/

//USA LA GRANDE TAMB
std::string decodeTag (const std::vector<cv::Mat>& subsquares , int index) {
	std::string tag;
	LOGI("decoding...");
	/*if ( subsquares.size() != 3) {
        LOGE("not enough parameters");
    	return 0;
    }*/
    //Matrix's for the three spaces of colour
	cv::Mat rImage = subsquares[0] , gImage = subsquares[1] , bImage = subsquares[2] ,
    img = subsquares[3];
	
	//CONSTANT FOR TAGS
	int cols = COLS , rows = ROWS;

    if (rImage.rows > rImage.cols) { //vertical tag
        std::swap(cols,rows);
    }

	//Value of increment for the index
	int incWidth = rImage.cols/cols , incHeight = rImage.rows/rows ;

    //y height-rows , x width-cols
    //Check the tag
    std::vector<std::vector<int> > v;
    v.resize(rows);
	for( int i = 0, y = incHeight/2; y < rImage.rows; y += incHeight , i++ )
	{
		v[i].resize(cols);
    	for( int j = 0 , x = incWidth/2; x < rImage.cols; x += incWidth , j++ )
    	{
        	//check for red circle
        	if ( checkPoint(rImage,y,x,0x00) ) {
        		v[i][j] = RED_VALUE;
        	}
        	//if there is not red, check for blue
        	else if ( checkPoint(bImage,y,x,0x00)) {
        		v[i][j] = BLUE_VALUE;
        	}
        	//if neither blue, check for green. More voluble to detect other like him
        	else if ( checkPoint(gImage,y,x,0x00)) {
        		v[i][j] = GREEN_VALUE;
        	}
        	//not colour detected
        	else
        		v[i][j] = DEFAULT_VALUE;

        	cv::Point p(x,y); //(x,y)
            cv::circle(img, p , 2, cv::Scalar(0,255,255),1,CV_AA);
        	//cv::circle(rImage, p , 3, cv::Scalar(0,0,255),1,CV_AA);
        	//cv::circle(gImage, p , 3, cv::Scalar(0,0,255),1,CV_AA);
			//cv::circle(bImage, p , 3, cv::Scalar(0,0,255),1,CV_AA);
    	}
	}

	//Print matrix
    LOGI("Readed");
	for ( int i=0 ; i<v.size() ; i++ ) {
		std::stringstream os;
		for (int j=0 ; j<v[0].size() ; j++ ) {
    		os << " " << v[i][j];
		}
		LOGI(os.str().c_str());
	}
	
    //Oriented tag
    orientedTag(v);

    //Create string for tag
    std::stringstream os;
    for ( int i=0 ; i<v.size() ; i++ )
        for (int j=0 ; j<v[0].size() ; j++ ) {
            os << v[i][j];
        }
    tag = os.str();

    //LOG
    LOGI(tag.c_str());
    if ( tag == "123211131323" )
        LOGI("CHECKED");

    std::stringstream file;
    file << "/mnt/sdcard/Pictures/MyCameraApp/points_" << index << ".jpeg";
    cv::imwrite(file.str().c_str(),img);
    /*
	std::string file = "/mnt/sdcard/Pictures/MyCameraApp/pointsR.jpeg";
    cv::imwrite(file,rImage);
    file = "/mnt/sdcard/Pictures/MyCameraApp/pointsG.jpeg";
    cv::imwrite(file,gImage);
    file = "/mnt/sdcard/Pictures/MyCameraApp/pointsB.jpeg";
    cv::imwrite(file,bImage); 
    */
    
    LOGI("decode done");
	return tag;
}

/**************************************/

void orientedTag ( std::vector<std::vector<int> > &v ) {
    //check if we need to turn the matrix (rows>cols)
    std::vector<std::vector<int> > aux;
    int w ;
    if ( v.size() > v[0].size() ) {
        aux.resize(ROWS, std::vector<int>(COLS));
        w = v[0].size();
        for ( int i=0 ; i<v.size() ; i++ ) {
            for ( int j=0 , k=w-1; j<w; j++ , k--) {
                aux[k][i] = v[i][j];
            }
        }
        v.clear();
        v = aux;
    }
    
    //check if it's good oriented
    bool oriented = true;
    for (int i=0 ; i<v.size() ; i++ )
        if ( v[i][0] != RED_VALUE ) {
            oriented = false;
            break;
        }

    //if not, oriented
    if (!oriented) {
        for ( int i=0 , k = (v.size()-1) ; i<(v.size()/2)+1 ; i++ , k-- )
            for ( int j=0 , l = v[0].size()-1; j<(v[0].size()) && !(j >= l && i==k) ; j++ , l-- ) {
                std::swap(v[i][j],v[k][l]);
            }
    }
} 

/**************************************/

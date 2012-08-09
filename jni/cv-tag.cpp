#include "cv-tag.h"

/**************************************/

std::vector<std::string> decodeTags (const std::vector<std::vector<cv::Mat> >& subsquares , bool oriented ) 
{
    std::vector<std::string> solution;
    std::string tag ;
    solution.clear();
    for ( int i=0 ; i<subsquares.size() ; i++ ) {
        tag = decodeTag(subsquares[i],i,oriented);
        if (validTag(tag)) {
            solution.push_back(tag);
        }
    }

    return solution;
}

/**************************************/

bool validTag (const  std::string &tag ) {
    /* VALID FORMAT
        [1]  ... [2]
          .   .  .
        [*] ... [*]
    */
    return (tag[0] == '1' && tag[COLS-1] == '2');
}

/**************************************/

bool checkPoint ( const cv::Mat& image , int y , int x , uchar colour ) {
    //return (image.at<uchar>(y, x) == colour );
    //if ( y < image.rows && x < image.cols )  
    for ( int i=-1 ; i<1 ; i++ )
        for ( int j=-1 ; j<1 ; j++ )
            if ( image.at<uchar>(y+i, x+j) == colour )
                return true;
    return false;
}

/**************************************/

std::string decodeTag (const std::vector<cv::Mat>& subsquares , int index , bool oriented ) {
	std::string tag;
    if (DEBUG) {
	   LOGI("DECODING...");
    }

    //Matrix's for the three spaces of colour
	cv::Mat rImage = subsquares[0] , gImage = subsquares[1] , bImage = subsquares[2] , img;
    if (DEBUG) img = subsquares[3];
	//CONSTANT FOR TAGS
	int cols = COLS , rows = ROWS;

    //check the orientation of the tag
    if (rImage.rows > rImage.cols) { //vertical tag
        std::swap(cols,rows);
    }

	//Value of increment for the index
	int incWidth = rImage.cols/cols , incHeight = rImage.rows/rows ;

    //y height-rows , x width-cols
    //Check the tag
    std::vector<std::vector<int> > v;
    v.resize(rows);
	for( int i = 0, y = incHeight/2 ; y < rImage.rows; y += incHeight , i++ )
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

            if (DEBUG) {
        	   cv::Point p(x,y); //(x,y)
                if (x<rImage.cols && y<rImage.rows)
                    cv::circle(img, p , 1, cv::Scalar(0,255,255),1,CV_AA);
            }
    	}
	}

	//Print matrix
    if (DEBUG) {
        LOGI("Tag Readed");
	   for ( int i=0 ; i<v.size() ; i++ ) {
	       std::stringstream os;
	       for (int j=0 ; j<v[0].size() ; j++ ) {
    		  os << " " << v[i][j];
		  }
		  LOGI(os.str().c_str());
	   }
    }

    //Oriented tag
    if ( !oriented ) {
        orientedTag(v);
    }

    //Print matrix
    if (DEBUG) {
        LOGI("Oriented");
        for ( int i=0 ; i<v.size() ; i++ ) {
            std::stringstream os;
            for (int j=0 ; j<v[0].size() ; j++ ) {
                os << " " << v[i][j];
            }
            LOGI(os.str().c_str());
        }
    }

    //Create string for tag
    std::stringstream os;
    for ( int i=0 ; i<v.size() ; i++ )
        for (int j=0 ; j<v[0].size() ; j++ ) {
            os << v[i][j];
        }
    tag = os.str();

    if (DEBUG) {
        //store image with points
        std::stringstream file;
        file << ANDROID_PATH + "points_" << index << ".jpeg";
        cv::imwrite(file.str().c_str(),img);
    
        LOGI(".... DONE");
    }
	return tag;
}

/**************************************/

void orientedTag ( std::vector<std::vector<int> > &v ) {
    
    //check if we need to turn the matrix (rows>cols)
    std::vector<std::vector<int> > aux;
    if ( v.size() > v[0].size() ) {
        int w ;
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

    //if not, oriented
    if (v[0][0] != RED_VALUE ) {
        for ( int i=0 , k = (v.size()-1) ; i<(v.size()/2)+1 && i <= k ; i++ , k-- ) {
            for ( int j=0 , l = v[0].size()-1; j<(v[0].size()) && !(j >= l && i==k) ; j++ , l-- ) {
                std::swap(v[i][j],v[k][l]);
            }
        }
    }
} 

/**************************************/

std::vector<Tag> findTags (const Image_data* data , bool oriented ) 
{
    std::vector<Tag> tags;
    tags.clear();
    
    if ( DEBUG ) {
        cv::Mat img1 (data->src,false) , img2 (data->rImage,false) , img3 (data->bImage,false) , img4 (data->gImage,false); //NOT COPY OF IMAGE
        std::string file1 = ANDROID_PATH + "filter_src.jpeg";
        cv::imwrite(file1,img1);
        file1 = ANDROID_PATH + "filter_red.jpeg";
        cv::imwrite(file1,img2);
        file1 = ANDROID_PATH + "filter_green.jpeg";
        cv::imwrite(file1,img4);
        file1 =  ANDROID_PATH + "filter_blue.jpeg";
        cv::imwrite(file1,img3);
    }
    
    

    //Convert IplImage to Mat
    cv::Mat img (data->rImage,false); //NOT COPY OF IMAGE
    
    //find rectangles on image
    std::vector<Rect> rect = findRect(img);

    //log
    std::stringstream os;
    if (DEBUG) {
        os << "Square found before: " << rect.size() ;
    }
    
    //remove rectangles inside others ones
    filterRect(rect);

    //log
    if (DEBUG) {
        os << " later: " << rect.size() ;
        LOGI(os.str().c_str());
    }

    //cut and deskew rectangles
    std::vector<std::vector<cv::Mat> > subrect = cutRect(data,rect);

    //recognize tag's codes in rectangles
    std::vector<std::string> codes = decodeTags(subrect , oriented );

    //build tags
    Tag aux;
    for ( int i=0 ; i<codes.size() ; i++ ) {
        aux.x = (rect[i].bounding_box.center.x * 100 ) / img.cols;
        aux.y = (rect[i].bounding_box.center.y * 100 ) / img.rows;
        aux.code = codes[i];
        tags.push_back(aux);
    }

    if (DEBUG) {
        //draw the rectangles founded and store them
        cv::Mat img_org (data->src,false); //NOT COPY OF IMAGE
        drawRect(img_org, rect);
        std::string file = ANDROID_PATH + "src_squares.jpeg";
        cv::imwrite(file,img_org);
    }
    
    //return the image with the rectangles
    //return getBmpImage(env,&img_org.operator IplImage());
    
    //return tags founded
    return tags;
}

/**************************************/

void adjustRGBThreshold(const std::string &readed ,const std::string &original) 
{
    //boolean's values who represent if the color had been good detected
    short r = 0 , b = 0 , g = 0;
    short desc = 1;

    //check wich values are incorrect
    for ( int i=0 ; i<readed.size() ; i++ ) {
        std::stringstream os;
        if (DEBUG) os << "compared:" <<  readed[i] << " y " << original[i];
        if ( readed[i] != original[i] ) {   
            switch(original[i]) {
                case '1':
                    r += desc; //it should have been red, but we didn't detect it
                    break;
                case '2':
                    b += desc; //it should have been blue, but we didn't detect it
                    //if we confused one color for another, up the color wrong
                    if ( readed[i] == '1' ) r += -desc;
                    break;
                case '3':
                    g += desc; //it should have been green, but we didn't detect it
                    //if we confused one color for another, up the color wrong
                    if ( readed[i] == '1' ) r -= desc;
                    if ( readed[i] == '2' ) b -= desc;
                    break;
                default:
                    break;
            }
        }
        LOGI(os.str().c_str());
    }

    //adjust incorrect values
    if (r) {
        RED_THRESHOLD -= r * 5;
    }
    if (b) {
        BLUE_THRESHOLD -= b * 5;
    } 
    if (g) {
        GREEN_THRESHOLD -= g * 7;
    }

}

/**************************************/
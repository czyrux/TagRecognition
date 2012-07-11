#include "cv-tag.h"

std::string PATH = "/mnt/sdcard/Pictures/TagRecognizerApp/";

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
	LOGI("DECODING...");
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
            cv::circle(img, p , 1, cv::Scalar(0,255,255),1,CV_AA);
        	//cv::circle(rImage, p , 3, cv::Scalar(0,0,255),1,CV_AA);
        	//cv::circle(gImage, p , 3, cv::Scalar(0,0,255),1,CV_AA);
			//cv::circle(bImage, p , 3, cv::Scalar(0,0,255),1,CV_AA);
    	}
	}

	//Print matrix
    LOGI("Tag Readed");
	for ( int i=0 ; i<v.size() ; i++ ) {
		std::stringstream os;
		for (int j=0 ; j<v[0].size() ; j++ ) {
    		os << " " << v[i][j];
		}
		LOGI(os.str().c_str());
	}
	
    //Oriented tag
    orientedTag(v);

    //Print matrix
    /*LOGI("Oriented");
    for ( int i=0 ; i<v.size() ; i++ ) {
        std::stringstream os;
        for (int j=0 ; j<v[0].size() ; j++ ) {
            os << " " << v[i][j];
        }
        LOGI(os.str().c_str());
    }*/

    //Create string for tag
    std::stringstream os;
    for ( int i=0 ; i<v.size() ; i++ )
        for (int j=0 ; j<v[0].size() ; j++ ) {
            os << v[i][j];
        }
    tag = os.str();

    
    //store image with points
    std::stringstream file;
    file << PATH + "points_" << index << ".jpeg";
    cv::imwrite(file.str().c_str(),img);
    
    LOGI(".... DONE");
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
        for ( int i=0 , k = (v.size()-1) ; i<(v.size()/2)+1 && i <= k ; i++ , k-- ) {
            for ( int j=0 , l = v[0].size()-1; j<(v[0].size()) && !(j >= l && i==k) ; j++ , l-- ) {
                std::swap(v[i][j],v[k][l]);
            }
        }
    }
} 

/**************************************/

std::vector<std::string> findTags (const Image_data* data) 
{
    std::vector<std::string> tags;
    tags.clear();
    
    cv::Mat img1 (data->src,false) , img2 (data->rImage,false) , img3 (data->bImage,false) , img4 (data->gImage,false); //NOT COPY OF IMAGE
    std::string file1 = PATH + "filter_src.jpeg";
    //cv::imwrite(file1,img1);
    file1 = PATH + "filter_red.jpeg";//+ RED_BOUNDARY + ".jpeg";
    cv::imwrite(file1,img2);
    file1 = PATH + "filter_green.jpeg";//+ GREEN_BOUNDARY + ".jpeg";
    cv::imwrite(file1,img4);
    file1 =  PATH + "filter_blue.jpeg";//+ BLUE_BOUNDARY + ".jpeg";
    cv::imwrite(file1,img3);
    

    //Convert IplImage to Mat
    cv::Mat img (data->rImage,false); //NOT COPY OF IMAGE
    cv::Mat img_org (data->src,false); //NOT COPY OF IMAGE
    
    //find squares on image
    std::vector<Square> squares;
    findSquares(img, squares);

    //log
    std::stringstream os;
    os << "Square found before: " << squares.size() ;
    
    //remove squares inside others ones
    filterSquares(squares);

    //log
    os << " later: " << squares.size() ;
    LOGI(os.str().c_str());

    //cut squares
    std::vector<std::vector<cv::Mat> > subsquares;
    cutSquares(data,squares,subsquares);

    //recognize tag's in squares
    tags = decodeTags(subsquares);

    //draw the squares founded and store them
    drawSquares(img_org, squares);
    std::string file = PATH + "src_squares.jpeg";
    cv::imwrite(file,img_org);
    
    //return the image with the squares
    //return getBmpImage(env,&img_org.operator IplImage());
    
    //return tags founded
    return tags;
}

/**************************************/

void adjustRGBBoundaries(std::string readed ,std::string original) 
{
    //boolean's values who represent if the color had been good detected
    short r = 0 , b = 0 , g = 0;
    short desc = 1;
    //CONTAR FALLOS Y EN FUNCION DE ESO RESTAR

    //check wich values are incorrect
    for ( int i=0 ; i<readed.size() ; i++ ) {
        std::stringstream os;
        os << "compared:" <<  readed[i] << " y " << original[i];
        if ( readed[i] != original[i] ) {   
            switch(original[i]) {
                case '1':
                    os << " fallo rojo";
                    r += desc; //it should have been red, but we didn't detect it
                    break;
                case '2':
                    os << " fallo azul";
                    b += desc; //it should have been blue, but we didn't detect it
                    break;
                case '3':
                    os << " fallo verde";
                    g += desc; //it should have been green, but we didn't detect it
                    break;
                default:
                    break;
            }
        }
        LOGI(os.str().c_str());
    }

    //adjust incorrect values
    if (r) {
        RED_BOUNDARY -= r;
        LOGI("Ajuste red");
    }
    if (b) {
        BLUE_BOUNDARY -= b * 5;
        LOGI("Ajuste blue");
    } 
    if (g) {
        GREEN_BOUNDARY -= g * 5;
        LOGI("Ajuste green");
    }

    std::stringstream os1;
    os1 << "dentro:" << " R: " << RED_BOUNDARY << " G: " << GREEN_BOUNDARY << " B: " << BLUE_BOUNDARY;
    LOGI(os1.str().c_str());
}

/**************************************/
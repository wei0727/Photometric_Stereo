#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>
#include <fstream>
#include <string>
using namespace std ;
using namespace cv ;

int main(){
	IplImage *img[6] ;
	string path = "bunny/pic" ;
	for(int i=0; i<6; i++){
		cout << path + (char)(i+49) + ".bmp" << endl ;
		img[i] = cvLoadImage((path + (char)(i+49) + ".bmp").c_str(), CV_LOAD_IMAGE_GRAYSCALE) ;
		cvShowImage((path + (char)(i+49) + ".bmp").c_str(), img[i]) ;
	}
	cvWaitKey(0) ;
	return 0 ;
}
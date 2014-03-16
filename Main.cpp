#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std ;
using namespace cv ;

void normalColorMap(IplImage *img[6], Mat lights, IplImage *nImg, vector<vector<Vec3d>> &nMap){
	Mat sMat = (lights.t()*lights).inv() * lights.t() ;
	for(int row=0; row<img[0]->height; row++){
		for(int col=0; col<img[0]->width; col++){
			double iArr[6] ;
			bool nonZero = false ;
			for(int i=0; i<6; i++){
				iArr[i] = cvGetReal2D(img[i], row, col) ;
				if(iArr[i] > 0) nonZero = true ;
			}
			Mat iMat = Mat(6, 1, CV_64F, iArr) ;
			Mat bMat = sMat * iMat ;
			Vec3d c(bMat.at<double>(2, 0), -bMat.at<double>(1, 0), bMat.at<double>(0, 0)) ;
			if(norm(c)!=0){
				c = c / norm(c) ;
			}
			c = (c+Vec3d(1, 1, 1))*255/2 ;
			if(nonZero){
				//cout << iMat << endl << bMat << endl ;
			}
			cvSet2D(nImg, row, col, c) ;
			nMap[row][col] = c ;
		}
	}
}

int main(){
	string path = "bunny/" ;
	IplImage *img[6] ;
 	for(int i=0; i<6; i++){
		cout << path + "pic" + (char)(i+49) + ".bmp" << endl ;
		img[i] = cvLoadImage((path + "pic" + (char)(i+49) + ".bmp").c_str(), CV_LOAD_IMAGE_GRAYSCALE) ;
		//cvShowImage((path + (char)(i+49) + ".bmp").c_str(), img[i]) ;
	}
	double lightArr[] = {238,235,2360,
					298,65,2480,
					-202,225,2240,
					-252,115,2310,
					18,45,2270,
					-22,295,2230} ;
	Mat lights = Mat(6, 3, CV_64F, lightArr).clone() ;
	cout << lights << endl ;
	IplImage *nImg = cvCreateImage(cvSize(img[0]->width, img[0]->height), img[0]->depth, 3) ;
	cvSetZero(nImg) ;
	vector<vector<Vec3d>> nMap(img[0]->height, vector<Vec3d>(img[0]->width)) ;
	normalColorMap(img, lights, nImg, nMap) ;
	//cvShowImage("nmap", nImg) ;
	cvSaveImage("bunny_normal.jpg", nImg) ;
	cvWaitKey(0) ;
	return 0 ;
}
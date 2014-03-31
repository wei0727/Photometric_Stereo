#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
using namespace std ;
using namespace cv ;

void normalColorMap(IplImage *img[6], Mat lights, IplImage *nImg, vector<vector<Vec3d>> &nMap){
	//Ux = y
	// x = inverse(transpose(U)*U) * transpose(U) * y
	//preompute inverse(transpose(U)*U) * transpose(U)
	Mat sMat = (lights.t()*lights).inv() * lights.t() ;
	for(int row=0; row<img[0]->height; row++){
		for(int col=0; col<img[0]->width; col++){
			double iArr[6] ;
			bool nonZero = false ;
			//get gray level from 6 images
			for(int i=0; i<6; i++){
				iArr[i] = cvGetReal2D(img[i], row, col) ;
			}
			Mat iMat = Mat(6, 1, CV_64F, iArr) ;
			Mat bMat = sMat * iMat ;
			//(z, y, x) --> (b, g, r)
			Vec3d c(bMat.at<double>(2, 0), -bMat.at<double>(1, 0), bMat.at<double>(0, 0)) ;
			if(norm(c)!=0)
				c = c / norm(c) ;
			nMap[row][col] = Vec3d(c[2], c[1], c[0]) ;
			//normalize to 0~255
			c = (c+Vec3d(1, 1, 1))*255/2 ;
			//set color to nImg
			cvSet2D(nImg, row, col, c) ;
		}
	}
}

void surfaceReconstruct(string fileName, vector<vector<Vec3d>> &nMap){
	const int r = nMap.size() ;
	const int c = nMap[0].size() ;
	ofstream depthFile(fileName) ;
	depthFile << 0.05 << endl ;
	depthFile << c << endl ;
	depthFile << r << endl ;
	vector<vector<double>> heightField(r, vector<double>(c, 0.0)) ;
	double integral_y=0, integral_x=0 ;
	double dmax, dmin ;
	dmax = dmin = 0 ;
	//calculate depth
	for(int row=1; row<r; row++){
		//prevent divide by zero
		if(nMap[row][0][2]!=0)
			heightField[row][0] = heightField[row-1][0] + -nMap[row][0][1]/nMap[row][0][2] ;
		else
			heightField[row][0] = heightField[row-1][0] ;
		dmax = std::max(dmax, heightField[row][0]) ;
		dmin = std::min(dmin, heightField[row][0]) ;
	}
	for(int row=0; row<r; row++){
		for(int col=1; col<c; col++){
			if(nMap[row][col][2]!=0)
				heightField[row][col] = heightField[row][col-1] + -nMap[row][col][0]/nMap[row][col][2] ;
			else
				heightField[row][col] = heightField[row][col-1] ;
			dmax = std::max(dmax, heightField[row][col]) ;
			dmin = std::min(dmin, heightField[row][col]) ;
		}
	}
	//output depth to file
	for(int row=0; row<r; row++){
		for(int col=0; col<c; col++){
			//background, |normal|=0 --> depth=0
			if(norm(nMap[row][col])!=0)
				//normalize depth to 0~5
				depthFile << (heightField[row][col]-dmin)*5/(dmax-dmin) << " " ;
				//depthFile << heightField[row][col] << " " ;
			else
				depthFile << 0 << " " ;
		}
		depthFile << endl ;
	}
	depthFile.close() ;
	//cout << "max min: " << dmax << "\t" << dmin << endl ;
}

void surfaceReconstruct_avg(string fileName, vector<vector<Vec3d>> &nMap){
	const int r = nMap.size() ;
	const int c = nMap[0].size() ;
	ofstream depthFile(fileName) ;
	depthFile << 0.05 << endl ;
	depthFile << r << endl ;
	depthFile << c << endl ;
	vector<vector<double>> heightFieldR(r, vector<double>(c, 0.0)) ;
	vector<vector<double>> heightFieldL(r, vector<double>(c, 0.0)) ;
	double integral_y=0, integral_x=0 ;
	double dmax, dmin ;
	dmax = dmin = 0 ;
	//calculate depth
	//left top to right bottom
	for(int row=1; row<r; row++){
		if(nMap[row][0][2]!=0)
			heightFieldL[row][0] = heightFieldL[row-1][0] + -nMap[row][0][1]/nMap[row][0][2] ;
		else
			heightFieldL[row][0] = heightFieldL[row-1][0] ;
	}
	for(int row=0; row<r; row++){
		for(int col=1; col<c; col++){
			if(nMap[row][col][2]!=0)
				heightFieldL[row][col] = heightFieldL[row][col-1] + -nMap[row][col][0]/nMap[row][col][2] ;
			else
				heightFieldL[row][col] = heightFieldL[row][col-1] ;
		}
	}
	//right bottom to left top
	for(int row=r-2; row>=0; row--){
		if(nMap[row][c-1][2]!=0)
			heightFieldR[row][c-1] = heightFieldR[row+1][c-1] + -nMap[row][c-1][1]/nMap[row][c-1][2] ;
		else
			heightFieldR[row][c-1] = heightFieldR[row+1][c-1] ;
	}
	for(int row=r-1; row>=0; row--){
		for(int col=c-2; col>=0; col--){
			if(nMap[row][col][2]!=0)
				heightFieldR[row][col] = heightFieldR[row][col+1] + -nMap[row][col][0]/nMap[row][col][2] ;
			else
				heightFieldR[row][col] = heightFieldR[row][col+1] ;
			heightFieldL[row][col] = (heightFieldL[row][col]+heightFieldR[row][col])/2 ;
			dmax = std::max(dmax, heightFieldL[row][col]) ;
			dmin = std::min(dmin, heightFieldL[row][col]) ;
		}
	}
	//output depth to file
	for(int row=0; row<r; row++){
		for(int col=0; col<c; col++){
			//background, |normal|=0 --> depth=0
			if(norm(nMap[row][col])!=0)
				//normalize depth to 0~5
				depthFile << (heightFieldL[row][col]-dmin)*5/(dmax-dmin) << " " ;
				//depthFile << heightFieldR[row][col] << " " ;
			else
				depthFile << 0 << " " ;
		}
		depthFile << endl ;
	}
	depthFile.close() ;
}

void photometric(string path, double *lightArr){
	IplImage *img[6] ;
	//load 6 image from given path(bunny or teapot)
	for(int i=0; i<6; i++){
		cout << path + "/pic" + (char)(i+49) + ".bmp" << endl ;
		img[i] = cvLoadImage((path + "/pic" + (char)(i+49) + ".bmp").c_str(), CV_LOAD_IMAGE_GRAYSCALE) ;
		//cvShowImage((path + (char)(i+49) + ".bmp").c_str(), img[i]) ;
		//cout << img[i]->nChannels << endl ;
	}
	Mat lights = Mat(6, 3, CV_64F, lightArr).clone() ;
	//Normal color image
	IplImage *nImg = cvCreateImage(cvSize(img[0]->width, img[0]->height), img[0]->depth, 3) ;
	cvSetZero(nImg) ;
	//Original Normals 
	vector<vector<Vec3d>> nMap(img[0]->height, vector<Vec3d>(img[0]->width)) ;

	//compute normal and set to nImg, nMap
	normalColorMap(img, lights, nImg, nMap) ;
	//compute depth value from given nMap and save to corresponding file
	surfaceReconstruct(path+"_depth.txt", nMap) ;

	//surfaceReconstruct_avg(path+"_depth_avg.txt", nMap) ;
	cvSaveImage((path+"_normal.jpg").c_str(), nImg) ;
	cout << path+"_normal.jpg  saved" << endl ;
	cout << path+"_depth.txt  saved" << endl ;
}

int main(){
	//light source of bunny
	double lightArr_bunny[] = {238,235,2360,
					298,65,2480,
					-202,225,2240,
					-252,115,2310,
					18,45,2270,
					-22,295,2230} ;
	//light source of teapot
	double lightArr_teapot[] = {0,0,150,
								22,29,156,
								-46,38,163,
								-25,13,160,
								12,10,168,
								35,5,168} ;
	photometric("bunny", lightArr_bunny) ;
	photometric("teapot", lightArr_teapot) ;
	cvWaitKey(0) ;
	return 0 ;
}
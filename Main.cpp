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
			if(norm(c)!=0)
				c = c / norm(c) ;
			nMap[row][col] = c ;
			c = (c+Vec3d(1, 1, 1))*255/2 ;
			if(nonZero){
				//cout << iMat << endl << bMat << endl ;
			}
			cvSet2D(nImg, row, col, c) ;
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
	IplImage *nImg = cvCreateImage(cvSize(img[0]->width, img[0]->height), img[0]->depth, 3) ;
	cvSetZero(nImg) ;
	vector<vector<Vec3d>> nMap(img[0]->height, vector<Vec3d>(img[0]->width)) ;
	normalColorMap(img, lights, nImg, nMap) ;

	vector<vector<double>> heightField(nImg->height, vector<double>(nImg->width, 0.0)) ;
	vector<vector<double>> pArr(nImg->height, vector<double>(nImg->width, 0.0)) ;
	vector<vector<double>> qArr(nImg->height, vector<double>(nImg->width, 0.0)) ;
	ofstream depthFile("depth_bunny.txt") ;
	ofstream pFile("pFile.txt") ;
	ofstream qFile("qFile.txt") ;
	depthFile << 0.05 << endl ;
	depthFile << 120 << endl ;
	depthFile << 120 << endl ;
	double integral_y=0, integral_x=0 ;
	/*
	for(int row=0; row<nImg->height; row++){
		if(row!=0){
			//if(nMap[row][0][2]!=0)
				integral_y += -nMap[row][0][1]/nMap[row][0][2] ;
		}
		integral_x = 0 ;
		for(int col=0; col<nImg->width; col++){
			if(col!=0)
				//if(nMap[row][col][2]!=0)
					integral_x += -nMap[row][col][0]/nMap[row][col][2] ;
			heightField[row][col] = integral_y + integral_x ;
			depthFile << heightField[row][col]/8.0 << " " ;
		}
		depthFile << endl ;
	}
	*/

	for(int row=1; row<nImg->height; row++){
		if(nMap[row][0][2]!=0)
			heightField[row][0] = heightField[row-1][0] + -nMap[row][0][1]/nMap[row][0][2] ;
		else
			heightField[row][0] = heightField[row-1][0] ;
	}
	for(int row=0; row<nImg->height; row++){
		for(int col=1; col<nImg->width; col++){
			if(nMap[row][col][2]!=0)
				heightField[row][col] = heightField[row][col-1] + -nMap[row][col][0]/nMap[row][col][2] ;
			else
				heightField[row][col] = heightField[row][col-1] ;
				//if(abs(nMap[row][col][0]/nMap[row][col][2])>500)
					//cout << nMap[row][col][0] << "\t" << nMap[row][col][2] << "\t" << (nMap[row][col][0]/nMap[row][col][2]) << endl ;
			
		}
	}
	double max, min ;
	double pmax, pmin, qmax, qmin ;
	max = min = heightField[0][0] ;
	pmax = pmin = qmax = qmin = 0 ;
	int zeroCount = 0 ;
	for(int row=0; row<nImg->height; row++){
		for(int col=0; col<nImg->width; col++){
			if(nMap[row][col][2]!=0){
				pArr[row][col] = -nMap[row][col][0]/nMap[row][col][2] ;
				qArr[row][col] = -nMap[row][col][1]/nMap[row][col][2] ;
			}
			else
				zeroCount++ ;;
			max = std::max(max, heightField[row][col]) ;
			min = std::min(min, heightField[row][col]) ;
			pmax = std::max(pmax, pArr[row][col]) ;
			pmin = std::min(pmin, pArr[row][col]) ;
			qmax = std::max(qmax, qArr[row][col]) ;
			qmin = std::min(qmin, qArr[row][col]) ;
			//depthFile << heightField[row][col]/10.0 << " " ;
		}
		//depthFile << endl ;
	}
	for(int row=0; row<nImg->height; row++){
		for(int col=0; col<nImg->width; col++){
			if(heightField[row][col]!=0)
				depthFile << (heightField[row][col]-min)*5/(max-min) << " " ;
			else
				depthFile << 0 << " " ;
			
			pFile << pArr[row][col] << "\t" ;
			qFile << qArr[row][col] << "\t" ;
		}
		depthFile << endl ;
		pFile << endl ;
		qFile << endl ;
	}
	int count=0 ;
	for(int row=1; row<nImg->height; row++){
		for(int col=1; col<nImg->width; col++){
			double dpdy = pArr[row][col]-pArr[row-1][col] ;
			double dqdx = qArr[row][col]-qArr[row][col-1] ;
			if(abs(dpdy-dqdx) > 1000){
				cout << nMap[row][col][0] << "\t" << nMap[row][col][1] << "\t" << nMap[row][col][2] << endl ;
				count++ ;
			}
		}
	}
	cout << "2nd derivative: " << count << endl ;
	depthFile.close() ;
	pFile.close() ;
	qFile.close() ;
	cout << "depth: " << max << "\t" << min << endl ;
	cout << "p: " << pmax << "\t" << pmin << endl ;
	cout << "q: " << qmax << "\t" << qmin << endl ;
	//cvShowImage("nmap", nImg) ;
	cvSaveImage("bunny_normal.jpg", nImg) ;
	cvWaitKey(0) ;
	return 0 ;
}
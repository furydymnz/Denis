/*********************************************************** 
*  --- OpenSURF ---                                       *
*  This library is distributed under the GNU GPL. Please   *
*  use the contact form at http://www.chrisevansdev.com    *
*  for more information.                                   *
*                                                          *
*  C. Evans, Research Into Robust Visual Features,         *
*  MSc University of Bristol, 2008.                        *
*                                                          *
************************************************************/

#include <highgui.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <cv.h>
#include <cmath>
#include "utils.h"
#include <vector>
#include <limits>
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;
//-------------------------------------------------------

static const int NCOLOURS = 8;
static const CvScalar COLOURS [] = {cvScalar(255,0,0), cvScalar(0,255,0), 
                                    cvScalar(0,0,255), cvScalar(255,255,0),
                                    cvScalar(0,255,255), cvScalar(255,0,255),
                                    cvScalar(255,255,255), cvScalar(0,0,0)};

//-------------------------------------------------------

//! Display error message and terminate program
void error(const char *msg) 
{
  cout << "\nError: " << msg;
  getchar();
  exit(0);
}

//-------------------------------------------------------

//! Show the provided image and wait for keypress
void showImage(const IplImage *img)
{
  cvNamedWindow("Surf", CV_WINDOW_AUTOSIZE); 
  cvShowImage("Surf", img);  
  cvWaitKey(0);
}

//-------------------------------------------------------

//! Show the provided image in titled window and wait for keypress
void showImage(char *title,const IplImage *img)
{
  cvNamedWindow(title, CV_WINDOW_AUTOSIZE); 
  cvShowImage(title, img);  
  cvWaitKey(0);
}

//-------------------------------------------------------

// Convert image to single channel 32F
IplImage *getGray(const IplImage *img)
{
  // Check we have been supplied a non-null img pointer
  if (!img) error("Unable to create grayscale image.  No image supplied");

  IplImage* gray8, * gray32;

  gray32 = cvCreateImage( cvGetSize(img), IPL_DEPTH_32F, 1 );

  if( img->nChannels == 1 )
    gray8 = (IplImage *) cvClone( img );
  else {
    gray8 = cvCreateImage( cvGetSize(img), IPL_DEPTH_8U, 1 );
    cvCvtColor( img, gray8, CV_BGR2GRAY );
  }

  cvConvertScale( gray8, gray32, 1.0 / 255.0, 0 );

  cvReleaseImage( &gray8 );
  return gray32;
}

//-------------------------------------------------------


//-------------------------------------------------------


//! Draw a single feature on the image
void drawPoint(IplImage *img, Ipoint &ipt)
{
  float s, o;
  int r1, c1;

  s = 3;
  o = ipt.orientation;
  r1 = fRound(ipt.y);
  c1 = fRound(ipt.x);

  cvCircle(img, cvPoint(c1,r1), fRound(s), COLOURS[ipt.clusterIndex%NCOLOURS], -1);
  cvCircle(img, cvPoint(c1,r1), fRound(s+1), COLOURS[(ipt.clusterIndex+1)%NCOLOURS], 2);

}

//-------------------------------------------------------

//! Draw a single feature on the image
void drawPoints(IplImage *img, vector<Ipoint> &ipts)
{
  float s, o;
  int r1, c1;

  for(unsigned int i = 0; i < ipts.size(); i++) 
  {
    s = 3;
    o = ipts[i].orientation;
    r1 = fRound(ipts[i].y);
    c1 = fRound(ipts[i].x);

    cvCircle(img, cvPoint(c1,r1), fRound(s), COLOURS[ipts[i].clusterIndex%NCOLOURS], -1);
    cvCircle(img, cvPoint(c1,r1), fRound(s+1), COLOURS[(ipts[i].clusterIndex+1)%NCOLOURS], 2);
  }
}

//! Draw features on the image
void drawPoints(Mat &image, vector<Ipoint> &ipts)
{
	float s, o;
	int r1, c1;

	for (unsigned int i = 0; i < ipts.size(); i++)
	{
		s = 3;
		o = ipts[i].orientation;
		r1 = fRound(ipts[i].y);
		c1 = fRound(ipts[i].x);

		circle(image, Point2i(c1, r1), s, COLOURS[ipts[i].clusterIndex%NCOLOURS], -1);
		circle(image, Point2i(c1, r1), s+1, COLOURS[ipts[i].clusterIndex%NCOLOURS], 2);
	}
}

//-------------------------------------------------------


//-------------------------------------------------------


//-------------------------------------------------------


//-------------------------------------------------------

//-------------------------------------------------------


void findmaxima(int& maxX, int& maxY, int& minX, int& minY, Mat& image, Mat& warpMat){

	const int cornerNum = 4;
	std::vector<Point2f> corner(4);
	corner[0] = Point2f(0,0); corner[1] = Point2f( image.size().width, 0 );
	corner[2] = Point2f( image.size().width, image.size().height ); corner[3] = Point2f( 0, image.size().height );

	cout << "width:" << image.size().width  << " height:" << image.size().height << " " << endl;

	perspectiveTransform(corner, corner, warpMat);

	maxX = minX = corner[0].x;
	maxY = minY = corner[0].y;

	for(int i = 0;i < cornerNum;i++){
		if (corner[i].x > maxX) {
			maxX = corner[i].x;
		}
		if(corner[i].x < minX){
			minX = corner[i].x;
		}
		if (corner[i].y > maxY) {
			maxY = corner[i].y;
		}
		if(corner[i].y < minY){
			minY = corner[i].y;
		}
	}

	cout << "x:" << minX << " " << maxX << endl << " y:" << minY << " " << maxY << endl;
}


cv::Mat border(cv::Mat mask)
{
	cv::Mat gx;
	cv::Mat gy;

	cv::Sobel(mask, gx, CV_32F, 1, 0, 3);
	cv::Sobel(mask, gy, CV_32F, 0, 1, 3);

	cv::Mat border;
	cv::magnitude(gx, gy, border);

	gx.release();
	gy.release();

	return border <100;
}
double ComputeError(const cv::Mat& image1, const cv::Mat& image2, int i, int c)
{
	cv::Vec3b c1 = image1.at<Vec3b>(i, c);
	cv::Vec3b c2 = image2.at<Vec3b>(i, c);
	double b = (c1[0] - c2[0])*(c1[0] - c2[0]);
	double g = (c1[1] - c2[1])*(c1[1] - c2[1]);
	double r = (c1[2] - c2[2])*(c1[2] - c2[2]);
	return sqrt(b + g + r);
}


void uniqueRandom(int **value, int pointAmount, int randomMax)
{
	//generate unique four pts
	(*value) = new int [pointAmount]; 
	for (int m = 0;m < pointAmount;m++){
		bool check; //variable to check or number is already used
		int n; //variable to store the number in
		do{
			n=rand()%randomMax;
			//check or number is already used:
			check=true;
			for (int j = 0;j < m;j++)
				if (n == (*value)[j])
				{
					check=false; 
					break; //no need to check the other elements of value[]
				}
		} while (!check); //loop until new, unique number is found
		(*value)[m]=n; //store the generated number in the array
	}
}

Mat findhomography(IpPairVec& matches, int reverse = 0)
{
	std::vector<Point2f> targetPt;
	std::vector<Point2f> destPt;
	std::vector<Point2f> targetPt_r;
	std::vector<Point2f> destPt_r;

	const double ransacThreshold = 1.0;
	int maxInlier = 0;

	//RANSAC
	maxInlier = 0;

	int pointAmount = 4;
	int *point = new int [pointAmount];
	Mat H;
	for(int k = 0;k < 5000;k++){

		int *value;
		uniqueRandom(&value, pointAmount, matches.size());

		//get and store the points
		for(int j = 0;j < pointAmount;j++){
			int index = value[j];
			if (!reverse){
				destPt.push_back(Point2f(matches[index].first.x, matches[index].first.y));
				targetPt.push_back(Point2f(matches[index].second.x, matches[index].second.y));
			}
			else{
				targetPt.push_back(Point2f(matches[index].first.x, matches[index].first.y));
				destPt.push_back(Point2f(matches[index].second.x, matches[index].second.y));
			}
		}

		//Mat tempH = findHomography(targetPt, destPt, 0);
		Mat tempH = getPerspectiveTransform(targetPt, destPt);

		for(int j = 0;j < matches.size();j++){
			bool check = false;
			for(int n = 0;n < pointAmount;n++){
				if(j == value[n]){
					check = true;
					break;
				}
			}
			if(!check){
				if (!reverse){
					destPt_r.push_back(Point2f(matches[j].first.x, matches[j].first.y));
					targetPt_r.push_back(Point2f(matches[j].second.x, matches[j].second.y));
				}
				else
				{
					targetPt_r.push_back(Point2f(matches[j].first.x, matches[j].first.y));
					destPt_r.push_back(Point2f(matches[j].second.x, matches[j].second.y));
				}
			}
		}

		std::vector<Point2f> tempPt(matches.size() - pointAmount);
		perspectiveTransform(targetPt_r, tempPt, tempH);

		int inlier_num = 0;
		for(int j = 0;j < matches.size() - pointAmount;j++){
			if(DIS(tempPt[j].x, tempPt[j].y, destPt_r[j].x, destPt_r[j].y) < ransacThreshold){
				inlier_num++;
			}
		}
		if(inlier_num > maxInlier){
			maxInlier = inlier_num;
			for(int i = 0;i < pointAmount;i++){
				point[i] = value[i];
			}
			H = tempH;
		}

		destPt.clear();
		targetPt.clear();
		destPt_r.clear();
		targetPt_r.clear();
		tempPt.clear();
		delete []value;
	}

	//cout << "number of inliers:" << maxInlier << endl;
	/*
	for(int i = 0;i < pointAmount;i++){
		//cout << "x1: " <<  matches[i].first.x << " y2: " << matches[i].first.y << endl;
		//cout << "x2: " <<  matches[i].second.x << " y2: " << matches[i].second.y << endl;
		cout << "x1: " <<  matches[point[i]].first.x << " y1: " << matches[point[i]].first.y << endl;
		cout << "x2: " <<  matches[point[i]].second.x << " y2: " << matches[point[i]].second.y << endl;
	}*/

	//cout << "---------------------------------" << endl;
	return H;
}

inline double DIS(double x1, double y1, double x2, double y2)
{
	return sqrt(((x1)-(x2))*((x1)-(x2))+((y1)-(y2))*((y1)-(y2)));
}


void findIntersection(Mat& mask1, Mat& mask2, Mat& intersection)
{
	Mat border1 = border(mask1);
	Mat border2 = border(mask2);
	intersection = ~(border1 | border2);

	border1.release();
	border2.release();
}
double getDPError(int i, int j, Mat &errorMap, direction **dirMap)
{
	if (i < 0 || j < 0 || j >= errorMap.cols || i >= errorMap.rows || dirMap[i][j] == YO)
		return -1;
	return errorMap.at<double>(i, j);
}

void ComputeError(int i, int j, Mat &image1, Mat &image2, direction **dirMap, Mat &errorMap, Mat &textMask)
{
	double eCurrent;
	if (textMask.at<unsigned char>(i, j) == 255)
	{
		eCurrent = 999;
	}
	else
	{
		eCurrent = ComputeError(image1, image2, i, j);
	}
	//TL T TR L R
	double errors[5] = {getDPError(i-1, j-1, errorMap, dirMap),
					getDPError(i-1, j, errorMap, dirMap),
					getDPError(i-1, j+1, errorMap, dirMap),
					getDPError(i, j-1, errorMap, dirMap),
					getDPError(i, j+1, errorMap, dirMap),};
	double minError = 110000;
	int minDir = -1;
	for(int i=0 ; i<5 ; i++)
	{
		if(errors[i]==-1)
			continue;
		if(errors[i]<minError)
		{
			minError = errors[i];
			minDir = i;
		}
	}
	if(minDir==-1)
	{
		dirMap[i][j] = CURRENT;
		errorMap.at<double>(i, j) = eCurrent;
	}
	else
	{
		switch(minDir)
		{
		case 0:
			dirMap[i][j]=TOPLEFT;
			break;
		case 1:
			dirMap[i][j]=TOP;
			break;
		case 2:
			dirMap[i][j]=TOPRIGHT;
			break;
		case 3:
			dirMap[i][j]=LEFT;
			break;
		case 4:
			dirMap[i][j]=RIGHT;
			break;
		default:
			dirMap[i][j]=CURRENT;
		}
		errorMap.at<double>(i, j) = eCurrent + minError;
	}

}


void ComputeHorizontalError(int i, int j, Mat &image1, Mat &image2, direction **dirMap, Mat &errorMap, Mat &textMask)
{
	double eCurrent;
	if (textMask.at<unsigned char>(i, j) == 255)
	{
		eCurrent = 999;
	}
	else
	{
		eCurrent = ComputeError(image1, image2, i, j);
	}
	//TL T L BL B
	double errors[5] = {getDPError(i-1, j-1, errorMap, dirMap),
		getDPError(i-1, j, errorMap, dirMap),
		getDPError(i, j-1, errorMap, dirMap),
		getDPError(i+1, j-1, errorMap, dirMap),
		getDPError(i+1, j, errorMap, dirMap),};
	double minError = DBL_MAX;
	int minDir = -1;
	for(int i=0 ; i<5 ; i++)
	{
		if(errors[i]==-1)
			continue;
		if(errors[i] < minError)
		{
			minError = errors[i];
			minDir = i;
		}
	}
	if(minDir==-1)
	{
		dirMap[i][j] = CURRENT;
		errorMap.at<double>(i, j) = eCurrent;
	}
	else
	{
		switch(minDir)
		{
		case 0:
			dirMap[i][j]=TOPLEFT;
			break;
		case 1:
			dirMap[i][j]=TOP;
			break;
		case 2:
			dirMap[i][j]=LEFT;
			break;
		case 3:
			dirMap[i][j]=BOTTOMLEFT;
			break;
		case 4:
			dirMap[i][j]=BOTTOM;
			break;
		default:
			dirMap[i][j]=CURRENT;
		}
		if(eCurrent <= DBL_MAX)
			errorMap.at<double>(i, j) = eCurrent + minError;
		else
			errorMap.at<double>(i, j) = DBL_MAX;
	}

}


int findIntersectionPts(Point2i& pt1, Point2i& pt2, Mat& intersection, Mat& andMasks)
{
	vector<Point2i> interpts;

	//get points from intersection Mat
	for(int i = 0;i < intersection.rows; i++)
	{
		for(int j = 0;j < intersection.cols;j++)
		{
			if (intersection.at<unsigned char>(i, j) != 0 && andMasks.at<unsigned char>(i, j) != 0)
				interpts.push_back(Point2i(j, i));
		}
	}

	if (interpts.size() == 0)
	{
		printf("No intersection\n");
		return -1;
	}

	//find the pair of points that have the longest distance
	double maxDis = 0;
	
	for(int i = 0;i < interpts.size() - 1;i++)
	{
		for(int j = i + 1;j < interpts.size();j++)
		{
			double tempDis = DIS(interpts[i].x, interpts[i].y, interpts[j].x, interpts[j].y);
			if(tempDis > maxDis)
			{
				maxDis = tempDis;
				pt1 = interpts[i];
				pt2 = interpts[j];
			}
		}
	}

	printf("pt1.x:%d pt1.y:%d\npt2.x:%d pt2.y:%d\n",pt1.x, pt1.y, pt2.x, pt2.y);
	return 0;
}

ErrorBundle horizontalErrorMap(cv::Mat image1, cv::Mat image2, Mat mask1, Mat mask2, Mat textMask1, Mat textMask2, double scale)
{
	ErrorBundle errorBundle;
	cv::Mat andMasks = mask1 & mask2;
	Mat errorMap;
	Mat textMask = textMask1 | textMask2;
	// ------------------------------------------
	double eTopLeft = 0, eBottomLeft = 0, eTop = 0, eLeft = 0, eBottom = 0, eCurrent;

	double maxError = 0;

	Mat intersection;
	findIntersection(mask1, mask2, intersection);
	Point2i pt1, pt2;
	findIntersectionPts(pt1, pt2, intersection, andMasks);

	Point hd_pt1(pt1.x, pt1.y);
	Point hd_pt2(pt2.x, pt2.y);
	Mat hd_andMask;

	Mat tempMask1;
	Mat tempMask2;
	Mat tempImage1;
	Mat tempImage2;
	Mat tempTextMask1;
	Mat tempTextMask2;
	if (scale != 1.0)
	{
		tempImage1 = image1.clone();
		tempImage2 = image2.clone();
		resize(mask1, tempMask1, Size(0, 0), scale, scale, INTER_LINEAR);
		resize(mask2, tempMask2, Size(0, 0), scale, scale, INTER_LINEAR);
		resize(image1, image1, Size(0, 0), scale, scale, INTER_LINEAR);
		resize(image2, image2, Size(0, 0), scale, scale, INTER_LINEAR);
		resize(textMask1, tempTextMask1, Size(0, 0), scale, scale, INTER_LINEAR);
		resize(textMask2, tempTextMask2, Size(0, 0), scale, scale, INTER_LINEAR);

		textMask = tempTextMask1 | tempTextMask2;
		andMasks = tempMask1 & tempMask2;
		findIntersection(tempMask1, tempMask2, intersection);
		findIntersectionPts(pt1, pt2, intersection, andMasks);

		hd_andMask = mask1 & mask2;
	}
	imwrite("YO//textMask.jpg", textMask);
	errorMap = Mat(image1.size(), CV_64FC1);

	errorBundle.setErrorMap(errorMap);
	
	//let pt1 be the leftmost point
	Point2i temp;
	if (pt1.x > pt2.x) {
		temp = pt1;
		pt1 = pt2;
		pt2 = temp;
	}

	if (hd_pt1.x > hd_pt2.x) {
		temp = hd_pt1;
		hd_pt1 = hd_pt2;
		hd_pt2 = temp;
	}


	//DP
	direction **dirMap;
	dirMap = new direction *[image1.rows];
	for (int i = 0; i < image1.rows; i++)
	{
		dirMap[i] = new direction[image1.cols];
		for (int j = 0; j < image1.cols; j++)
		{
			//YO is meaningless
			dirMap[i][j] = YO;
		}
	}

	for (int j = pt1.x; j <= pt2.x; j++)
	{
		if (j == pt1.x)
		{
			for (int i = 0; i < errorMap.rows; i++)
			{
				if (andMasks.at<unsigned char>(i, j) == 0)
					continue;
				if (i == pt1.y)
				{
					errorMap.at<double>(i, j) = ComputeError(image1, image2, i, j);
					dirMap[i][j] = CURRENT;
				}
				else
					ComputeHorizontalError(i, j, image1, image2, dirMap, errorMap, textMask);
			}
			for (int i = errorMap.rows - 1; i >= 0; i--)
			{
				if (andMasks.at<unsigned char>(i, j) == 0)
					continue;
				if (i == pt1.y)
				{
					errorMap.at<double>(i, j) = ComputeError(image1, image2, i, j);
					dirMap[i][j] = CURRENT;
				}
				else
					ComputeHorizontalError(i, j, image1, image2, dirMap, errorMap, textMask);
			}
		}
		else
		{
			for (int i = 0; i < errorMap.rows; i++)
			{
				if (andMasks.at<unsigned char>(i, j) == 0)
					continue;
				ComputeHorizontalError(i, j, image1, image2, dirMap, errorMap, textMask);
			}
			for (int i = errorMap.rows - 1; i >= 0; i--)
			{
				if (andMasks.at<unsigned char>(i, j) == 0)
					continue;
				ComputeHorizontalError(i, j, image1, image2, dirMap, errorMap, textMask);
			}
		}

	}
	for (int j = pt1.x; j <= pt2.x; j++)
	{
		for (int i = 0; i < errorMap.rows; i++)
			if (i + 1<errorMap.rows && i>0
				&& dirMap[i - 1][j] == BOTTOM && dirMap[i][j] == TOP)
			{
				//may have some error
				dirMap[i - 1][j] = TOP;
				dirMap[i][j] = BOTTOM;
			}
	}
	
	double minError = errorMap.at<double>(pt2.y, pt2.x);
	Point2i startpt = pt2;
	vector<Point2i> &seam = errorBundle.getpath();
	errorBundle.setPathError(minError);
	seam.push_back(startpt);
	int x = startpt.x, y = startpt.y;
	while (1) {
		if (dirMap[y][x] == CURRENT || dirMap[y][x] == YO)
			break;
		switch (dirMap[y][x]) {
		case TOPLEFT:
			x--;
			y--;
			break;
		case TOP:
			y--;
			break;
		case LEFT:
			x--;
			break;
		case BOTTOMLEFT:
			x--;
			y++;
			break;
		case BOTTOM:
			y++;
			break;
		}
		seam.push_back(Point2i(x, y));
		//errorSeam.at<Vec3b>(y, x) = Vec3b(0, 0, 255);
		//seamMap.at<unsigned char>(y, x) = 255;
	}
/*
	char a[100];
	static int c = 0;
	sprintf(a, "%d.jpg", c++);
	Mat seamMap(image1.size(), CV_8UC1,Scalar(0));
	for (int i = 0; i < seam.size(); i++)
		seamMap.at<unsigned char>(seam[i]) = 255;
	imwrite(a, seamMap);
	*/
	if (scale != 1.0)
	{
		fixSeam(seam, hd_pt1, hd_pt2, scale, hd_andMask);
		image1 = tempImage1.clone();
		image2 = tempImage2.clone();
	}



	andMasks.release();
	intersection.release();
	errorMap.release();
	//errorSeam.release();
	//seamMap.release();

	return errorBundle;
}

ErrorBundle verticalErrorMap(cv::Mat image1, cv::Mat image2, Mat mask1, Mat mask2, Mat textMask1, Mat textMask2, double scale)
{
	// edited: find regions where no mask is set
	// compute the region where no mask is set at all, to use those color values unblended

	//5-way DP seam finder
	ErrorBundle errorBundle;
	cv::Mat andMasks = mask1 & mask2;
	Mat errorMap;
	Mat errorGraph(image1.size(), CV_8UC1);
	Mat textMask = textMask1 | textMask2;
	// ------------------------------------------
	double eTopLeft = 0, eTopRight = 0, eTop = 0, eLeft = 0, eRight = 0, eCurrent;

	double maxError = 0;
	Mat intersection;
	findIntersection(mask1, mask2, intersection);
	Point2i pt1, pt2;
	findIntersectionPts(pt1, pt2, intersection, andMasks);

	Point hd_pt1(pt1.x, pt1.y);
	Point hd_pt2(pt2.x, pt2.y);
	Mat hd_andMask;

	Mat tempMask1;
	Mat tempMask2;
	Mat tempImage1;
	Mat tempImage2;
	Mat tempTextMask1;
	Mat tempTextMask2;
	if (scale != 1.0)
	{
		tempImage1 = image1.clone();
		tempImage2 = image2.clone();
		resize(mask1, tempMask1, Size(0, 0), scale, scale, INTER_LINEAR);
		resize(mask2, tempMask2, Size(0, 0), scale, scale, INTER_LINEAR);
		resize(image1, image1, Size(0, 0), scale, scale, INTER_LINEAR);
		resize(image2, image2, Size(0, 0), scale, scale, INTER_LINEAR);
		resize(textMask1, tempTextMask1, Size(0, 0), scale, scale, INTER_LINEAR);
		resize(textMask2, tempTextMask2, Size(0, 0), scale, scale, INTER_LINEAR);

		textMask = tempTextMask1 | tempTextMask2;
		andMasks = tempMask1 & tempMask2;
		findIntersection(tempMask1, tempMask2, intersection);
		findIntersectionPts(pt1, pt2, intersection, andMasks);

		hd_andMask = mask1 & mask2;
	}
	imwrite("YO//textMask.jpg", textMask);
	errorMap = Mat(image1.size(), CV_64FC1);

	errorBundle.setErrorMap(errorMap);

	//let pt1 be the leftmost point
	Point2i temp;
	if (pt1.y > pt2.y) {
		temp = pt1;
		pt1 = pt2;
		pt2 = temp;
	}

	if (hd_pt1.y > hd_pt2.y) {
		temp = hd_pt1;
		hd_pt1 = hd_pt2;
		hd_pt2 = temp;
	}

	//DP
	direction **dirMap;
	dirMap = new direction *[image1.rows];
	for (int i = 0; i < image1.rows; i++)
	{
		dirMap[i] = new direction[image1.cols];
		for (int j = 0; j < image1.cols; j++)
			dirMap[i][j] = YO;
	}

	for (int i = pt1.y; i <= pt2.y; i++)
	{
		if (i == pt1.y)
		{
			for (int j = 0; j < errorMap.cols; j++)
			{
				if (andMasks.at<unsigned char>(i, j) == 0)
					continue;
				if (j == pt1.x)
				{
					errorMap.at<double>(i, j) = ComputeError(image1, image2, i, j);
					dirMap[i][j] = CURRENT;
				}
				else
					ComputeError(i, j, image1, image2, dirMap, errorMap, textMask);
			}
			for (int j = errorMap.cols - 1; j >= 0; j--)
			{
				if (andMasks.at<unsigned char>(i, j) == 0)
					continue;
				if (j == pt1.x)
				{
					errorMap.at<double>(i, j) = ComputeError(image1, image2, i, j);
					dirMap[i][j] = CURRENT;
				}
				else
					ComputeError(i, j, image1, image2, dirMap, errorMap, textMask);
			}
		}
		else
		{
			for (int j = 0; j<errorMap.cols; j++)
			{
				if (andMasks.at<unsigned char>(i, j) == 0)
					continue;
				ComputeError(i, j, image1, image2, dirMap, errorMap, textMask);
			}
			for (int j = errorMap.cols - 1; j >= 0; j--)
			{
				if (andMasks.at<unsigned char>(i, j) == 0)
					continue;
				ComputeError(i, j, image1, image2, dirMap, errorMap, textMask);
			}
		}
	}
	for (int i = pt1.y; i <= pt2.y; i++)
	{
		for (int j = 0; j < errorMap.cols; j++)
			if (j + 1<errorMap.cols && j>0
				&& dirMap[i][j - 1] == RIGHT && dirMap[i][j] == LEFT)
			{
				dirMap[i][j - 1] = LEFT;
				dirMap[i][j] = RIGHT;
			}
	}
	
	double minError = errorMap.at<double>(pt2.y, pt2.x);
	Point2i startpt = pt2;
	vector<Point2i> &seam = errorBundle.getpath();
	errorBundle.setPathError(minError);
	printf("~~~%lf\n", minError);
	seam.push_back(startpt);
	int x = startpt.x, y = startpt.y;
	while (1) {
		if (dirMap[y][x] == CURRENT || dirMap[y][x] == YO)
			break;
		switch (dirMap[y][x]) {
		case TOPLEFT:
			x--;
			y--;
			break;
		case TOP:
			y--;
			break;
		case TOPRIGHT:
			x++;
			y--;
			break;
		case LEFT:
			x--;
			break;
		case RIGHT:
			x++;
			break;
		}
		seam.push_back(Point2i(x, y));
		//errorSeam.at<Vec3b>(y, x) = Vec3b(0, 0, 255);
		//seamMap.at<unsigned char>(y, x) = 255;
	}
	/*
	char a[100];
	static int c = 0;
	sprintf(a, "%d.jpg", c++);
	Mat seamMap(image1.size(), CV_8UC1, Scalar(0));
	for (int i = 0; i < seam.size(); i++)
		seamMap.at<unsigned char>(seam[i]) = 255;
	imwrite(a, seamMap);
	*/
	if (scale != 1.0)
	{
		fixSeam(seam, hd_pt1, hd_pt2, scale, hd_andMask);
		image1 = tempImage1.clone();
		image2 = tempImage2.clone();
	}


	andMasks.release();
	intersection.release();
	errorGraph.release();
	errorMap.release();
	//errorSeam.release();
	//seamMap.release();

	return errorBundle;
}

inline bool checkBoundry(int width, int height, int x, int y) {
	return (x >= 0 && x < width && y >= 0 && y < height);
}

void fixSeam(vector<Point2i> &seam, Point pt1, Point pt2, double scale, Mat andMask)
{
	vector<Point2i> fixedSeam;
	Point currentPoint(pt2.x, pt2.y);
	Point nextPoint(seam[0].x / scale, seam[0].y / scale);
	int dx, dy, index = 1;

	fixedSeam.push_back(currentPoint);
	while (index < seam.size())
	{
		if (andMask.at<unsigned char>(nextPoint) != 255) {
			int x, y;
			int i = 1;
			x = nextPoint.x;
			y = nextPoint.y;
			while (true) {
				if (checkBoundry(andMask.cols,andMask.rows, y + i, x) && andMask.at<unsigned char>(y + i, x) == 255) {
					nextPoint.y = y + i;
					break;
				}
				if (checkBoundry(andMask.cols, andMask.rows, y - i, x) && andMask.at<unsigned char>(y - i, x) == 255) {
					nextPoint.y = y - i;
					break;
				}
				if (checkBoundry(andMask.cols, andMask.rows, y, x + i) && andMask.at<unsigned char>(y, x + i) == 255) {
					nextPoint.x = x + i;
					break;
				}
				if (checkBoundry(andMask.cols, andMask.rows, y, x - i) && andMask.at<unsigned char>(y, x - i) == 255) {
					nextPoint.x = x - i;
					break;
				}
				i++;
			}
		}

		dx = (int)ceil(nextPoint.x - currentPoint.x);
		dy = (int)ceil(nextPoint.y - currentPoint.y);

		while (dx != 0 || dy != 0)
		{
			if (dx < 0) {
				currentPoint.x--;
				dx++;
			}
			else if (dx>0) {
				currentPoint.x++;
				dx--;
			}
			if (dy < 0) {
				currentPoint.y--;
				dy++;
			}
			else if (dy>0) {
				currentPoint.y++;
				dy--;
			}
			fixedSeam.push_back(currentPoint);
		}
		if (++index >= seam.size())
			break;
		nextPoint = Point(seam[index].x / scale, seam[index].y / scale);
	}


	dx = (int)ceil(pt1.x - currentPoint.x);
	dy = (int)ceil(pt1.y - currentPoint.y);

	while (dx != 0 || dy != 0)
	{
		if (dx < 0) {
			currentPoint.x--;
			dx++;
		}
		else if (dx>0) {
			currentPoint.x++;
			dx--;
		}
		if (dy < 0) {
			currentPoint.y--;
			dy++;
		}
		else if (dy>0) {
			currentPoint.y++;
			dy--;
		}
		fixedSeam.push_back(currentPoint);
	}
	printf("-------------WHAT-------------\n");
	printf("pt1: %d %d\n", pt1.x, pt1.y);
	printf("pt2: %d %d\n", pt2.x, pt2.y);

	seam.clear();
	for (int i = 0; i < fixedSeam.size(); i++){
		seam.push_back(fixedSeam[i]);
	}
}

void verticalBlending(Mat& blended, Mat& image1, Mat& image2, Mat& mask1, Mat& mask2, vector<Point2i>& seam)
{
	//TODO seam walking on the edge of andmask
	cv::Mat andMasks = mask1 & mask2;
	Mat xormask1 = mask1 ^ andMasks;
	Mat xormask2 = mask2 ^ andMasks;
	xormask1 = xormask1 > 0;
	xormask2 = xormask2 > 0;
	Mat seamMap(image1.size(), CV_8UC1, Scalar(0));

	image1.copyTo(blended, xormask1);
	image2.copyTo(blended, xormask2);

	for (int i = 0; i < seam.size(); i++)
	{
		seamMap.at<unsigned char>(seam[i]) = 255;
	}
	char a[100];
	static int c = 0;
	sprintf(a, "YO/seam%d.jpg", c++);
	imwrite(a, seamMap);

	bool passedSeam;
	bool image1Left;
	
	bool isSet = false;
	for (int j = 0; j < blended.cols; j++)
	{
		for (int i = 0; i < blended.rows; i++)
		{
			if (mask1.at<unsigned char>(i, j) != 0)
			{
				image1Left = true;
				isSet = true;
				break;
			}
			else if (mask2.at<unsigned char>(i, j) != 0)
			{
				image1Left = false;
				isSet = true;
				break;
			}
		}
		if (isSet)
			break;
	}

	if (image1Left)
	{
		printf("dX>=0\n");
		for (int i = 0; i < blended.rows; i++)
		{
			passedSeam = false;
			for (int j = 0; j < blended.cols; j++)
			{
				if (andMasks.at<unsigned char>(i, j) == 255)
				{
					if (seamMap.at<unsigned char>(i, j) == 255)
						passedSeam = true;
					if (!passedSeam)
						blended.at<Vec3b>(i, j) = image1.at<Vec3b>(i, j);
					else
						blended.at<Vec3b>(i, j) = image2.at<Vec3b>(i, j);
				}
			}
		}
	}
	else
	{
		printf("dX<0\n");
		for (int i = 0; i < blended.rows; i++) {
			passedSeam = false;
			for (int j = 0; j < blended.cols; j++) {
				if (andMasks.at<unsigned char>(i, j) == 255)
				{
					if (seamMap.at<unsigned char>(i, j) == 255)
						passedSeam = true;
					if (!passedSeam)
						blended.at<Vec3b>(i, j) = image2.at<Vec3b>(i, j);
					else
						blended.at<Vec3b>(i, j) = image1.at<Vec3b>(i, j);
				}
			}

		}
	}
	andMasks.release();
	xormask1.release();
	xormask2.release();
	seamMap.release();
}

void horizontalBlending(Mat& blended, Mat& image1, Mat& image2, Mat& mask1, Mat& mask2, vector<Point2i>& seam)
{
	//TODO seam walking on the edge of andmask

	cv::Mat andMasks = mask1 & mask2;
	Mat xormask1 = mask1 ^ andMasks;
	Mat xormask2 = mask2 ^ andMasks;
	xormask1 = xormask1 > 0;
	xormask2 = xormask2 > 0;
	andMasks = andMasks > 0;
	Mat seamMap(image1.size(), CV_8UC1, Scalar(0));

	image1.copyTo(blended, xormask1);
	image2.copyTo(blended, xormask2);


	for (int i = 0; i < seam.size(); i++)
	{
		seamMap.at<unsigned char>(seam[i]) = 255;
	}

	char a[100];
	static int c = 0;
	sprintf(a, "YO/seam%d.jpg", c++);
	imwrite(a, seamMap);

	bool image1Above;
	bool isSet = false;
	for (int i = 0; i < blended.rows; i++)
	{
		for (int j = 0; j < blended.cols; j++)
		{
			if (mask1.at<unsigned char>(i, j) != 0)
			{
				image1Above = true;
				isSet = true;
				break;
			}
			else if (mask2.at<unsigned char>(i, j) != 0)
			{
				image1Above = false;
				isSet = true;
				break;
			}
		}
		if (isSet)
			break;
	}
	bool passedSeam;
	if (image1Above)
	{
		printf("dY>=0\n");
		for (int j = 0; j < blended.cols; j++) {
			passedSeam = false;
			for (int i = 0; i < blended.rows; i++) {
				if (andMasks.at<unsigned char>(i, j) == 255)
				{
					if (seamMap.at<unsigned char>(i, j) == 255)
						passedSeam = true;

					if (!passedSeam)
						blended.at<Vec3b>(i, j) = image1.at<Vec3b>(i, j);
						//blended.at<Vec3b>(i, j) = temp.at<Vec3b>(0, 0);
					else
						blended.at<Vec3b>(i, j) = image2.at<Vec3b>(i, j);
						//blended.at<Vec3b>(i, j) = temp.at<Vec3b>(0, 0);
				}
			}
		}
	}
	else
	{
		printf("dY<0\n");
		for (int j = 0; j < blended.cols; j++) {
			passedSeam = false;
			for (int i = 0; i < blended.rows; i++) {
				if (andMasks.at<unsigned char>(i, j) == 255)
				{
					if (seamMap.at<unsigned char>(i, j) == 255)
						passedSeam = true;
					if (!passedSeam)
						blended.at<Vec3b>(i, j) = image2.at<Vec3b>(i, j);
						//blended.at<Vec3b>(i, j) = temp.at<Vec3b>(0, 0);
					else
						blended.at<Vec3b>(i, j) = image1.at<Vec3b>(i, j);
						//blended.at<Vec3b>(i, j) = temp.at<Vec3b>(0, 0);
				}

			}
		}
	}

	andMasks.release();
	xormask1.release();
	xormask2.release();
	seamMap.release();
}


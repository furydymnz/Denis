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

#include "surflib.h"
#include "utils.h"
#include "RouteHandler.h"
#include "MatchTracker.h"
#include "BaseImage.h"
#include <ctime>
#include <iostream>
#include <vector>
#include <cmath>
#include <opencv2/opencv.hpp>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"

using namespace std;
using namespace cv;
//-------------------------------------------------------
// In order to you use OpenSURF, the following illustrates
// some of the simple tasks you can do.  It takes only 1
// function call to extract described SURF features!
// Define PROCEDURE as:
//  - 1 and supply image path to run on static image
//  - 2 to capture from a webcam
//  - 3 to match find an object in an image (work in progress)
//  - 4 to display moving features (work in progress)
//  - 5 to show matches between static images

#define PROCEDURE 8

//-------------------------------------------------------

int mainStaticStitching(int imageCount, char *imageStr[]){
	vector <IplImage * > vImage;

	IplImage *tempImage;

	//Load images
	for(int i=1 ; i<imageCount ; i++)
	{
		tempImage = cvLoadImage(imageStr[i]);
		if (tempImage != NULL)
		{
			vImage.push_back(tempImage);
			printf("%s\n", imageStr[i]);
		}
		else
			printf("%s can not be loaded!\n", imageStr[i]);
	}
	imageCount = vImage.size();
	if (imageCount == 0) return -1;

	MatchTracker matchTracker(imageCount);
	for (int i = 0; i < vImage.size(); i++)
		matchTracker.push_back(new BaseImage(vImage[i]));

	//Find surf descriptions
	IpVec tempIpVec;
	vector <IpVec > vIpVec;
	for(int i=0 ; i<imageCount ; i++)
	{
		surfDetDes(vImage[i], tempIpVec, false, 4, 4, 2, 0.0001f);
		vIpVec.push_back(tempIpVec);
	}
	printf("surfDetDes\n");

	//Find match
	IpPairVec tempMatch, bestMatch;
	int iBestMatch;
	
	for(int i=0 ; i<imageCount-1 ; i++)
	{
		for(int r=i+1 ; r<imageCount ; r++)
		{
			getMatches(vIpVec[i],vIpVec[r], tempMatch);

			matchTracker.assignFPNum(i, r, tempMatch.size());
			matchTracker.assignFPPair(i, r, tempMatch);
			printf("getMatches %d %d\n", i, r);
		}

	}

	printf("\n");
	for (int i = 0; i < imageCount; i++)
	{
		printf("For %5d: ", i);
		for (int r = 0; r < imageCount; r++)
		{
			printf("%5d", matchTracker.getPairNum(i)[r]);
		}
		printf("---%5d\n", vIpVec[i].size());
	}
	printf("\n");


	RouteHandler::findConnectingRoute(matchTracker);

	//sort
	const int pivotIndex = 0;
	vector<int> routeIndex, routeSize;
	for(int i = 0;i < matchTracker.getSize();i++)
	{
		if(i == pivotIndex) continue;
		routeIndex.push_back(i);
		routeSize.push_back(matchTracker.getRoute(i).route[0].size());
	}
	for(int i = 0;i < routeSize.size() - 1;i++)
	{
		for(int j = i + 1;j < routeSize.size();j++)
		{
			if(routeSize[j] < routeSize[i])
			{
				int temp = routeIndex[i];
				routeIndex[i] = routeIndex[j];
				routeIndex[j] = temp;
				temp = routeSize[i];
				routeSize[i] = routeSize[j];
				routeSize[j] = temp;
			}
		}	
	}
	
	for(int i = 0;i < routeIndex.size();i++)
		printf("%d ",routeIndex[i]);
	printf("\n");

	Mat H = Mat::eye(3,3,CV_64F);
	int startpt;
	for(int i = 0;i < routeIndex.size();i++)
	{
		H = Mat::eye(3,3,CV_64F);
		vector<int> &route = matchTracker.getRoute(routeIndex[i]).route[0];
		startpt = route[0];
		for(int j = 0;j < routeSize[i] - 1;j++)
		{
			if ((matchTracker.getHomographyPair(route[j], route[j + 1])).at<double>(0, 0) != -1)
			{
				printf("%d %d have pair\n", route[j], route[j + 1]);
				H = H*matchTracker.getHomographyPair(route[j], route[j + 1]);
				matchTracker.assignHomographyPair(startpt, route[j + 1], H);
			}
			else
			{
				printf("%d %d do not have pair\n", route[j], route[j + 1]);
				Mat tempH = findhomography(matchTracker.getPairFP(route[j], route[j+1]));
				matchTracker.assignHomographyPair(route[j], route[j + 1], tempH);
				H = H*tempH;
				matchTracker.assignHomographyPair(startpt, route[j + 1], H);
			}
		}
		
	}

	for(int i = 0;i < imageCount;i++)
	{
		for(int j = 0;j < imageCount;j++)
		{
			if ((matchTracker.getHomographyPair(i, j)).at<double>(0, 0) != -1)
				printf("1 ");
			else
				printf("0 ");
		}
		printf("\n");
	}




	char c;
	scanf(" %c", &c);
	/*
	//show match lines
	IpPairVec matches;
	Matcher *match;
	IplImage *img1, *img2;

	Mat H, blended;
	Mat preH(3, 3, CV_64F);
	preH.row(0).col(0) = 1;	preH.row(0).col(1) = 0;	preH.row(0).col(2) = 0;
	preH.row(1).col(0) = 0;	preH.row(1).col(1) = 1;	preH.row(1).col(2) = 0;
	preH.row(2).col(0) = 0;	preH.row(2).col(1) = 0;	preH.row(2).col(2) = 1;

	match = &(vMatcher[0]);
	matches = match->getMatches();
	img1 = vImage[match->getI1()];
	img2 = vImage[match->getI2()];
	printf("draw %d %d\n", match->getI1(), match->getI2());
	Mat image2(img2), image1(img1);
	H = findhomography(matches);
	preH = H;
	for (int i = 0; i < vMatcher.size(); i++)
	{
		if(i!=0)
		{
			match = &(vMatcher[i]);
			matches = match->getMatches();
			img1 = vImage[match->getI1()];
			img2 = vImage[match->getI2()];
			printf("draw %d %d\n", match->getI1(), match->getI2());
			Mat temp(img2);
			image2 = temp;
			image1 = blended;
			H = findhomography(matches);
			H = H*preH;
			preH = H;
		}
		
		
		
		int sizeX, sizeY, trans_x, trans_y;

		int maxX, maxY, minX, minY;
		findmaxima(maxX, maxY, minX, minY, image2, H);

		int dX = 0, dY = minY;
		if (minX < 0)
		{
			H.row(0).col(2) = H.at<double>(0,2) - minX;
			sizeX = maxX - minX;
			dX = -minX / H.at<double>(0, 0);
			dY = -minY / H.at<double>(1, 1);
		}
		else if (minX >= 0)
		{
			H.row(0).col(2) = H.at<double>(0,2) - minX;
			sizeX = maxX - minX;
			dX = minX;
		}
		if (minY < 0)
		{
			H.row(1).col(2) = H.at<double>(1, 2) - minY;
			sizeY = maxY - minY;
		}
		else if (minY >= 0)
		{
			H.row(1).col(2) = H.at<double>(1, 2) - minY;
			sizeY = maxY - minY;
		}
		for (int i = 0; i < H.rows; i++)
		{
			for (int r = 0; r < H.cols; r++)
			{
				printf("%.10lf\t", H.at<double>(i, r));
			}
			printf("\n");
		}
		printf("size_x: %d\tsize_y: %d\n", sizeX, sizeY);
		printf("dX:%d dY:%d\n",dX,dY);
		Mat rotated;
		Mat mask2(image2.size(), CV_8UC1, cv::Scalar(255));
		Mat mask1(image1.size(), CV_8UC1, cv::Scalar(255));
		warpPerspective(mask2, mask2, H, Size(sizeX, sizeY), INTER_LINEAR, BORDER_CONSTANT);
		warpPerspective(image2, rotated, H, Size(sizeX, sizeY), INTER_LINEAR, BORDER_CONSTANT);

		
		imwrite("test/1.jpg", image1);
		imwrite("test/2.jpg", image2);
		imwrite("test/transformed.jpg", rotated);


		double alpha = 0.0;
		
		cvNamedWindow("1", CV_WINDOW_AUTOSIZE);
		printf("image1 (width, height) = (%d, %d)\n", image1.cols, image1.rows);
		printf("rotated (width, height) = (%d, %d)\n", rotated.cols, rotated.rows);

		
		if (minX >= 0)
		{
			if(abs(dX) >= abs(dY))
				blendImage(TRANSVERSE,image1, rotated, blended, mask1, mask2, dX, dY);
			else
				blendImage(LONGITUDINAL,image1, rotated, blended, mask1, mask2, dX, dY);
			//drawFeaturePoint(blended,H,matches, dX, dY);
		}
		else                         
		{
			if(abs(dX) >= abs(dY))
				blendImage(TRANSVERSE,rotated, image1, blended, mask2, mask1, dX, dY);
			else
				blendImage(LONGITUDINAL,rotated, image1, blended, mask2, mask1, dX, dY);
			//m_drawFeaturePoint(blended,H,matches, dX, dY);
		}
		
		//preH = H;
		

	}*/
	//imshow("1", blended);
	//imwrite("test/blended.jpg", blended);
	cvWaitKey(0);
	
	return 0;
}
//-------------------------------------------------------

//-------------------------------------------------------

int main(int argc, char *argv[]) 
{
	srand(time(NULL));
	return mainStaticStitching(argc, argv);
}

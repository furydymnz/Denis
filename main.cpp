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
#include "Blender.h"
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
	if (imageCount <= 1) return -1;

	MatchTracker matchTracker(imageCount);
	for (int i = 0; i < vImage.size(); i++)
	{
		matchTracker.pushImage(new BaseImage(vImage[i]));
	}
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

	matchTracker.calculatePairConnection();
	RouteHandler::findConnectingRoute(matchTracker);
	RouteHandler::calculateHomography(matchTracker);

	for (int r = 0; r < 3; r++)
	{
		for (int p = 0; p < 3; p++)
			printf("%20.8lf", (matchTracker.getHomographyPair(0, 0)).at<double>(r, p));
		printf("\n");
	}
	for (int i = 0; i < imageCount; i++)
	{
		for (int j = 0; j < imageCount; j++)
		{
			if ((matchTracker.getHomographyPair(i, j)).at<double>(0, 0) != -1)
				printf("1 ");
			else
				printf("0 ");
		}
		printf("\n");
	}
	
	matchTracker.assignHomographyToImage();
	matchTracker.calculateBoundary();
	matchTracker.printHomography();
	matchTracker.calculateTranslation();

	matchTracker.generateMask();
	//matchTracker.applyHomography();
	
	matchTracker.pixelPadding();
	RouteHandler::findBlendingOrder(matchTracker);

	printf("===========Pair Connection============\n");
	for (int i = 0; i < imageCount; i++)
	{
		for (int j = 0; j < imageCount; j++)
			printf("%2d", matchTracker.getPairConnection(i, j));
		printf("\n");
	}

	printf("============Blending Order============\n");
	int size = matchTracker.getBlendingOrder().size();
	for (int i = 0; i < size; i++)
	{
		vector<int> &order = matchTracker.getBlendingOrder()[i];
		for (int r = 0; r < order.size(); r++)
			printf("%d ->", order[r]);
		printf("\n");
	}
	/*
	Blender blender(&matchTracker);
	blender.generateBlendingOrder();
	blender.printBlendingOrder();
	*/

	matchTracker.calculateErrorPair();


	printf("=============PairError=========\n");
	for (int i = 0; i < imageCount; i++)
	{
		printf("For %5d: ", i);
		for (int r = 0; r < imageCount; r++)
		{
			printf("%20.8lf", matchTracker.getPairError(i, r));
		}
		printf("---\n");
	}
	printf("\n");

	printf("============Blending===========\n");
	Mat blended = matchTracker.blending();
	imwrite("YO/blended.jpg", blended);
	printf("done\n");

	char c;
	scanf(" %c", &c);
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

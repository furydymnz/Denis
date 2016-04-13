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
#include "time.h"
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
	vector <char * > vString;
	IplImage *tempImage;

	clock_t start, stop;
	clock_t tstart, tstop;

	//Load images
	tstart = clock();
	start = clock();
	for(int i=1 ; i<imageCount ; i++)
	{
		tempImage = cvLoadImage(imageStr[i]);
		if (tempImage != NULL)
		{
			vImage.push_back(tempImage);
			printf("%s\n", imageStr[i]);
			vString.push_back(imageStr[i]);
		}
		else
			printf("%s can not be loaded!\n", imageStr[i]);
	}
	imageCount = vImage.size();
	if (imageCount <= 1) return -1;
	stop = clock();
	printf("Time of LoadImages is: %lf seconds\n", double(stop - start) / CLOCKS_PER_SEC);

	start = clock();

	MatchTracker matchTracker(imageCount);

	for (int i = 0; i < vImage.size(); i++)
	{
		matchTracker.pushImage(new BaseImage(vString[i]));
	}
	//Find surf descriptions
	IpVec tempIpVec;
	vector <IpVec > vIpVec;
	for(int i=0 ; i<imageCount ; i++)
	{
		surfDetDes(vImage[i], tempIpVec, false, 4, 4, 2, 0.0001f);
		vIpVec.push_back(tempIpVec);
		cvReleaseImage(&(vImage[i]));
	}
	printf("surfDetDes\n");

	stop = clock();
	printf("Time of Surf is: %lf seconds\n", double(stop - start) / CLOCKS_PER_SEC);

	start = clock();

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

	stop = clock();
	printf("Time of getMatches is: %lf seconds\n", double(stop - start) / CLOCKS_PER_SEC);

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

	start = clock();
	matchTracker.calculatePairConnection();
	stop = clock();
	printf("Time of calculatePairConnection is: %lf seconds\n", double(stop - start) / CLOCKS_PER_SEC);

	start = clock();
	RouteHandler::findConnectingRoute(matchTracker);
	stop = clock();
	printf("Time of findConnectingRoute is: %lf seconds\n", double(stop - start) / CLOCKS_PER_SEC);

	start = clock();
	RouteHandler::calculateHomography(matchTracker);
	stop = clock();
	printf("Time of calculateHomography is: %lf seconds\n", double(stop - start) / CLOCKS_PER_SEC);

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
	
	start = clock();
	matchTracker.assignHomographyToImage();
	stop = clock();
	printf("Time of assignHomographyToImage is: %lf seconds\n", double(stop - start) / CLOCKS_PER_SEC);
	
	start = clock();
	matchTracker.calculateBoundary();
	stop = clock();
	printf("Time of calculateBoundary is: %lf seconds\n", double(stop - start) / CLOCKS_PER_SEC);

	//matchTracker.printHomography();
	start = clock();
	matchTracker.calculateTranslation();
	stop = clock();
	printf("Time of calculateTranslation is: %lf seconds\n", double(stop - start) / CLOCKS_PER_SEC);

	printf("===========generateMask============\n");
	start = clock();
	matchTracker.generateMask();
	stop = clock();
	printf("Time of generateMask is: %lf seconds\n", double(stop - start) / CLOCKS_PER_SEC);
	//matchTracker.applyHomography();
	
	printf("===========pixelPadding============\n");
	start = clock();
	matchTracker.pixelPadding();
	stop = clock();
	printf("Time of pixelPadding is: %lf seconds\n", double(stop - start) / CLOCKS_PER_SEC);

	printf("===========findBlendingOrder============\n");
	start = clock();
	RouteHandler::findBlendingOrder(matchTracker);
	stop = clock();
	printf("Time of findBlendingOrder is: %lf seconds\n", double(stop - start) / CLOCKS_PER_SEC);

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

	start = clock();
	matchTracker.calculateErrorPair();
	stop = clock();
	printf("Time of calculateErrorPair is: %lf seconds\n", double(stop - start) / CLOCKS_PER_SEC);


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
	start = clock();
	Mat blended = matchTracker.blending();
	imwrite("YO/blended.jpg", blended);
	printf("done\n");
	stop = clock();
	printf("Time of blending is: %lf seconds\n", double(stop - start) / CLOCKS_PER_SEC);

	blended.release();
	
	tstop = clock();
	printf("Total Time of blending is: %lf seconds\n", double(tstop - tstart) / CLOCKS_PER_SEC);

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

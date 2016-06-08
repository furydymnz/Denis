
#include "utils.h"
#include "RouteHandler.h"
#include "MatchTracker.h"
#include "BaseImage.h"
#include "Blender.h"
#include "time.h"
#include <stdio.h>
#include <ctime>
#include <iostream>
#include <vector>
#include <cmath>
#include <opencv2/opencv.hpp>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/opencv_modules.hpp"
# include "opencv2/core/core.hpp"
# include "opencv2/features2d/features2d.hpp"
# include "opencv2/nonfree/features2d.hpp"

using namespace std;
using namespace cv;

#define PROCEDURE 8
#define MODE 0//0 for sceneryMoe; 1 for TextMode

//-------------------------------------------------------

int mainStaticStitching(int imageCount, char *imageStr[]){
	vector <Mat > vImage;
	vector <char * > vString;
	Mat tempImage;

	double scale = 1.0;
	const int maxBorder = 1000;
	const double minScale = 0.3;

	TextDetector textDetector;

	clock_t start, stop;
	clock_t tstart, tstop;

	//Load images
	tstart = clock();
	start = clock();
	
	for(int i=1 ; i<imageCount ; i++)
	{
		tempImage = imread(imageStr[i]);
		if (i == 1)
		{
			if ((tempImage.size().width) > (tempImage.size().height))
			{
				if ((tempImage.size().width) > maxBorder)
					scale = (double)maxBorder / (tempImage.size().width);
			}
			else
			{
				if ((tempImage.size().height) > maxBorder)
					scale = (double)maxBorder / (tempImage.size().height);
			}
			if (scale<minScale)
				scale = minScale;
		}
		printf("scale: %lf\n",scale);
		if (scale != 1.0)
			resize(tempImage, tempImage, Size(0, 0), scale, scale, INTER_LINEAR);

		if (!tempImage.empty())
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

#if MODE
	printf("===========detectText==============\n");
	start = clock();
	char a[100];
	for (int i = 0; i < imageCount; i++) {
		textDetector.detect(vImage[i]);
		sprintf(a, "test%d.jpg", i);
		imwrite(a, vImage[i]);
	}
	stop = clock();
	printf("Time of TextDetect is: %lf seconds\n", double(stop - start) / CLOCKS_PER_SEC);

#endif // MODE

	
	start = clock();

	//Find surf descriptions
	vector<Mat> descriptorsList;
	vector<vector<Point2f> > arraysOfKeyPoints;
	int minHessian = 400;

	SurfFeatureDetector detector(minHessian);
	SurfDescriptorExtractor extractor;

	for (int i = 0; i < imageCount; i++) {
		std::vector<KeyPoint> keypoints;
		Mat descriptors;
		detector.detect(vImage[i], keypoints);
		extractor.compute(vImage[i], keypoints, descriptors);
		descriptorsList.push_back(descriptors.clone());
		vector<Point2f> temp;
		for (int j = 0; j < keypoints.size(); j++) {
			temp.push_back(keypoints[j].pt);
		}
		arraysOfKeyPoints.push_back(temp);
	}

	stop = clock();
	printf("Time of Surf is: %lf seconds\n", double(stop - start) / CLOCKS_PER_SEC);

	start = clock();

	MatchTracker matchTracker(imageCount, scale);
	for (int i = 0; i < vImage.size(); i++)
	{
		matchTracker.pushImage(new BaseImage(vString[i]));
		vImage[i].release();
	}
	vImage.clear();
	vString.clear();

	FlannBasedMatcher matcher;
	for (int i = 0; i < imageCount - 1; i++) {
		for (int j = i + 1; j < imageCount; j++) {
			std::vector<vector<DMatch> > matches;
			matcher.knnMatch(descriptorsList[i], descriptorsList[j], matches, 5);
			int fpNum = 0;

			IpPairVec tempMatch;
			vector<Point2f> keyPoint1 = arraysOfKeyPoints[i];
			vector<Point2f> keyPoint2 = arraysOfKeyPoints[j];
			for (int k = 0; k < matches.size(); k++) {
				vector<DMatch> dmatch = matches[k];
				if (dmatch[0].distance / dmatch[1].distance < 0.65) {
					Point2f pt1 = keyPoint1[dmatch[0].queryIdx];
					Point2f pt2 = keyPoint2[dmatch[0].trainIdx];
					tempMatch.push_back(make_pair(Ipoint((pt1.x/scale), pt1.y / scale), Ipoint(pt2.x / scale, pt2.y / scale)));
					fpNum++;
				}
			}
			matchTracker.assignFPNum(i, j, fpNum);
			matchTracker.assignFPPair(i, j, tempMatch);
			printf("getMatches %d %d %d\n", i, j, fpNum);
		}
	}
	
	
	printf("surfDetDes\n");
	

	stop = clock();
	printf("Time of getMatches is: %lf seconds\n", double(stop - start) / CLOCKS_PER_SEC);

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
	
	start = clock();
	matchTracker.assignHomographyToImage();
	stop = clock();
	printf("Time of assignHomographyToImage is: %lf seconds\n", double(stop - start) / CLOCKS_PER_SEC);
	
	start = clock();
	matchTracker.calculateBoundary();
	stop = clock();
	printf("Time of calculateBoundary is: %lf seconds\n", double(stop - start) / CLOCKS_PER_SEC);

	start = clock();
	matchTracker.calculateTranslation();
	stop = clock();
	printf("Time of calculateTranslation is: %lf seconds\n", double(stop - start) / CLOCKS_PER_SEC);

	printf("===========generateMask============\n");
	start = clock();
	matchTracker.generateMask();
	stop = clock();
	printf("Time of generateMask is: %lf seconds\n", double(stop - start) / CLOCKS_PER_SEC);
	
	
	//printf("===========detectText==============\n");
	//start = clock();
	//Assign Zero Mask
	matchTracker.detectText();
	//stop = clock();
	//printf("Time of detectText is: %lf seconds\n", double(stop - start) / CLOCKS_PER_SEC);
	

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

	//imwrite("YO/blended.jpg", blended);
	imwrite("blended.jpg", blended);
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

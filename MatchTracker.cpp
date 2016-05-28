#include "MatchTracker.h"
#include "ErrorBundle.h"
#include "utils.h"
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#define DEBUG

MatchTracker::MatchTracker(int size, double scale)
{
	this->size = size;
	this->scale = scale;
	pairNum.resize(size);
	pairFP.resize(size);
	routes.resize(size);
	images.clear();
	pairHomography.resize(size);
	pairError.resize(size);
	pairConnection.resize(size);
	pairIntersection.resize(size);
	pairSeam.resize(size);

	for (int i = 0; i < size; i++)
	{
		pairNum[i].resize(size);
		pairFP[i].resize(size);
		pairHomography[i].resize(size);
		pairFP[i].clear();
		pairError[i].resize(size);
		pairConnection[i].resize(size);
		pairIntersection[i].resize(size);
		pairSeam[i].resize(size);
		for (int j = 0; j < size; j++)
		{
			pairHomography[i][j] = Mat(3, 3, CV_64F, Scalar(-1, -1, -1));
			pairError[i][j] = -1;
		}
	}
	pivotIndex = size / 2 - 1;
}

IpPairVec& MatchTracker::getPairFP(int i, int r, int & reverse)
{
	if (!pairFP[i][r].empty())
	{
		reverse = 1;
		return pairFP[i][r];
	}
	else if (!pairFP[r][i].empty())
	{
		reverse = 0;
		return pairFP[r][i];
	}
	return IpPairVec();
}

void MatchTracker::calculatePairConnection()
{
	const float fpThreshold = 0.3;
	const int fpBottomLimit = 10;
	for (int i = 0; i < size; i++)
	{
		vector<int> currentLine = getPairNum(i);
		int maxMatch = 0;
		for (int j = 0; j < size; j++)
		{
			if (currentLine[j] > maxMatch)
				maxMatch = currentLine[j];
		}

		for (int r = 0; r < size; r++)
		{
			if (currentLine[r] < maxMatch*fpThreshold ||
				currentLine[r] < fpBottomLimit)
				pairConnection[i][r] = 0;
			else
				pairConnection[i][r] = 1;
		}

	}

	//check and fix exception
	for (int i = 0; i < size; i++)
		for (int j = 0; j < size; j++)
			if (pairConnection[i][j] == 1)
				pairConnection[j][i] = 1;

}

void MatchTracker::assignHomographyToImage()
{
	for (int i = 0; i < size; i++)
	{
		if (images[i]->isEmpty()) continue;
		/*
		printf("assignHomographyToImage: pivotIndex:%d, pairHomographysize:%d \n", pivotIndex, pairHomography.size());
		printf("pairHomography[i].size():%d \n",  pairHomography[i].size());
		printf("(%d, %d)", i, pivotIndex);
		*/
		if (((pairHomography[i][pivotIndex])).at<double>(0, 0) != -1)
		{
			(images[i])->assignHomography((pairHomography[i][pivotIndex]));
		}
		else
			printf("FUCK!!!!!!!!!!!!!!!!!!!!\n");
	}
}

void MatchTracker::calculateBoundary()
{
	images[pivotIndex]->findBoundary();
	maxX = images[pivotIndex]->maxX;
	minX = images[pivotIndex]->minX;
	maxY = images[pivotIndex]->maxY;
	minY = images[pivotIndex]->minY;
	for (int i = 0; i < size; i++)
	{
		if (i == pivotIndex) continue;
		if (images[i]->isEmpty()) continue;
		images[i]->findBoundary();
		if (images[i]->maxX > maxX)
			maxX = images[i]->maxX;
		if (images[i]->minX < minX)
			minX = images[i]->minX;
		if (images[i]->maxY > maxY)
			maxY = images[i]->maxY;
		if (images[i]->minY < minY)
			minY = images[i]->minY;
	}
	imageSize = Size(maxX - minX, maxY - minY);
	printf("minX: %5d maxX: %5d minY: %5d maxY: %5d\n",
		minX, maxX, minY, maxY);
}

void MatchTracker::pixelPadding()
{
	Mat temp;
	imageSize = Size(imageSize.width + 2, imageSize.height + 2);
	for (int i = 0; i < size; i++)
	{
		if (images[i]->isEmpty()) continue;
		temp = Mat(imageSize, CV_8UC3, cv::Scalar(0, 0, 0));
		images[i]->getImage().copyTo(temp(Rect(1, 1, images[i]->getImage().cols, images[i]->getImage().rows)));
		images[i]->assignImage(temp);

		temp = Mat(imageSize, CV_8UC1, cv::Scalar(0));
		images[i]->getMask().copyTo(temp(Rect(1, 1, images[i]->getMask().cols, images[i]->getMask().rows)));
		images[i]->assignMask(temp);

		temp = Mat(imageSize, CV_8UC1, cv::Scalar(0));
		images[i]->getTextMask().copyTo(temp(Rect(1, 1, images[i]->getTextMask().cols, images[i]->getTextMask().rows)));
		images[i]->assignTextMask(temp);

	}
}




void MatchTracker::applyHomography()
{
	warpPerspective(images[pivotIndex]->getImage(), images[pivotIndex]->getImage(), images[pivotIndex]->getHomography(), imageSize, INTER_NEAREST, BORDER_CONSTANT);
	warpPerspective(images[pivotIndex]->getMask(), images[pivotIndex]->getMask(), images[pivotIndex]->getHomography(), imageSize, INTER_NEAREST, BORDER_CONSTANT);
	warpPerspective(images[pivotIndex]->getTextMask(), images[pivotIndex]->getTextMask(), images[pivotIndex]->getHomography(), imageSize, INTER_NEAREST, BORDER_CONSTANT);
}

void MatchTracker::generateMask()
{
	
	for (int i = 0; i < size; i++)
	{
		Mat mask((images[i]->getImage()).size(), CV_8UC1, cv::Scalar(255));

		if (images[i]->isEmpty()) continue;
	//	if ( i==pivotIndex )
		//	warpPerspective(mask, mask, images[i]->getHomography(), imageSize, INTER_NEAREST, BORDER_CONSTANT);
		images[i]->assignMask(mask);
	}
}


//!Move all the images to visible plane
void MatchTracker::calculateTranslation()
{
	Mat H;
	Mat transM = Mat::eye(3, 3, CV_64F);
	transM.at<double>(0, 2) = -minX;
	transM.at<double>(1, 2) = -minY;
	for (int i = 0; i < size; i++)
	{
		H = images[i]->getHomography();
		H = transM*H;
	}
}

void MatchTracker::detectText()
{
	for (int i = 0; i < size; i++) {
		if (images[i]->isEmpty()) 
			continue;
		images[i]->assignTextMask(textDetector.detect(images[i]->getImage()));
	}
}

void MatchTracker::printHomography()
{
	for (int i = 0; i < size; i++)
	{
		for (int r = 0; r < size; r++)
		{
			printf("\n%d %d:\n", i, r);
			for (int m = 0; m < 3; m++)
			{
				for (int n = 0; n < 3; n++)
				{
					printf("%10.5lf", pairHomography[i][r].at<double>(m, n));
				}
				printf("\n");
			}
		}
	}
}

void MatchTracker::calculateErrorPair()
{
	Mat andMask;
	ErrorBundle errorBundle;

	for (int i = 0; i < size - 1; i++)
	{
		Mat mask1;
		warpPerspective(getImage(i)->getMask(), mask1, images[i]->getHomography(), imageSize, INTER_NEAREST, BORDER_CONSTANT);
		Mat image1;
		warpPerspective(getImage(i)->getImage(), image1, images[i]->getHomography(), imageSize, INTER_NEAREST, BORDER_CONSTANT);
		Mat textMask1;
		warpPerspective(getImage(i)->getTextMask(), textMask1, images[i]->getHomography(), imageSize, INTER_NEAREST, BORDER_CONSTANT);
		for (int r = i + 1; r < size; r++)
		{
			if (!getPairConnection(i, r))
				continue;
			Mat mask2;
			warpPerspective(getImage(r)->getMask(), mask2, images[r]->getHomography(), imageSize, INTER_NEAREST, BORDER_CONSTANT);
			Mat image2;
			warpPerspective(getImage(r)->getImage(), image2, images[r]->getHomography(), imageSize, INTER_NEAREST, BORDER_CONSTANT);
			Mat textMask2;
			warpPerspective(getImage(r)->getTextMask(), textMask2, images[r]->getHomography(), imageSize, INTER_NEAREST, BORDER_CONSTANT);
			andMask = mask1 & mask2;

			Mat intersection;
			findIntersection(mask1, mask2, intersection);
			Point2i pt1, pt2;
			if (findIntersectionPts(pt1, pt2, intersection, andMask) == -1)
			{
				assignErrorPair(i, r, -1);
				assignIntersectionPair(i, r, pair<Point2i, Point2i>(Point2i(-1, -1), Point2i(-1, -1)));
				continue;
			}
			assignIntersectionPair(i, r, pair<Point2i, Point2i>(pt1, pt2));

			if (abs(pt1.x - pt2.x) > abs(pt1.y - pt2.y))
			{
				printf("Horizontal\n");
				errorBundle = horizontalErrorMap(image1, image2, mask1, mask2,
					textMask1, textMask2, scale);
			}
			else
			{
				printf("Vertical\n");
				errorBundle = verticalErrorMap(image1, image2, mask1, mask2, 
					textMask1, textMask2, scale);
			}

			double pathError = errorBundle.getPathError() / errorBundle.getpath().size();
			assignSeamPair(i, r, errorBundle.getpath());

			assignErrorPair(i, r, pathError);
			
			intersection.release();
			andMask.release();
			image2.release();
			mask2.release();
			textMask2.release();
		}
		image1.release();
		mask1.release();
		textMask1.release();
	}
}


Mat MatchTracker::blending()
{
	Mat blended;
	int minErrorIndex;
	double minError = DBL_MAX, tempError;
	for (int i = 0; i < blendingOrder.size(); i++)
	{
		tempError = 0;
		for (int j = 0; j < blendingOrder[i].size() - 1; j++)
			tempError += getPairError(blendingOrder[i][j], blendingOrder[i][j + 1]);
		if (tempError < minError)
		{
			minError = tempError;
			minErrorIndex = i;
		}
	}
	
	Mat orMask, andMask;
#ifdef DEBUG
	Mat textMask;
	vector<vector<Point2i> > allTheSeams;
#endif // DEBUG
	for (int i = 0; i < blendingOrder[minErrorIndex].size() - 1; i++)
	{
		pair<Point2i, Point2i> pts;
		int j = blendingOrder[minErrorIndex][i];
		int k = blendingOrder[minErrorIndex][i + 1];
		Mat mask2;
		warpPerspective(getImage(k)->getMask(), mask2, 
			getImage(k)->getHomography(), imageSize, INTER_NEAREST, BORDER_CONSTANT);
		Mat mask1;
		warpPerspective(getImage(j)->getMask(), mask1,
			getImage(j)->getHomography(), imageSize, INTER_NEAREST, BORDER_CONSTANT);
		Mat image2;
		warpPerspective(getImage(k)->getImage(), image2,
			getImage(k)->getHomography(), imageSize, INTER_NEAREST, BORDER_CONSTANT);
		Mat image1;
		warpPerspective(getImage(j)->getImage(), image1,
			getImage(j)->getHomography(), imageSize, INTER_NEAREST, BORDER_CONSTANT);
		Mat textMask1;
		warpPerspective(getImage(j)->getTextMask(), textMask1, getImage(j)->getHomography(), imageSize, INTER_NEAREST, BORDER_CONSTANT);
		Mat textMask2;
		warpPerspective(getImage(k)->getTextMask(), textMask2, getImage(k)->getHomography(), imageSize, INTER_NEAREST, BORDER_CONSTANT);
#ifdef DEBUG
		if (i == 0) {
			textMask = textMask1 | textMask2;
		}
		else {
			textMask = textMask | textMask2;
		}
#endif // DEBUG
		if (i == 0) 
		{
			blended = image1.clone();

			pts = getPairInteresection(blendingOrder[minErrorIndex][i], blendingOrder[minErrorIndex][i + 1]);
			if (abs(pts.first.x - pts.second.x) > abs(pts.first.y - pts.second.y))
			{
				horizontalBlending(blended, blended, image2,
					mask1, mask2, getPairSeam(j, k));
			}
			else
			{
				verticalBlending(blended, blended, image2,
					mask1, mask2, getPairSeam(j, k));
			}
#ifdef DEBUG
			allTheSeams.push_back(getPairSeam(j, k));
#endif // DEBUG

		}
		else
		{
			mask1 = orMask;
			andMask = mask1 & mask2;

			Mat intersection;
			ErrorBundle errorBundle;
			findIntersection(mask1, mask2, intersection);
			Point2i pt1, pt2;
			findIntersectionPts(pt1, pt2, intersection, andMask);

			if (abs(pt1.x - pt2.x) > abs(pt1.y - pt2.y))
			{
				printf("Horizontal\n");
				errorBundle = horizontalErrorMap(blended, image2, mask1, mask2, 
					textMask1, textMask2, scale);
				horizontalBlending(blended, blended, image2,
					mask1, mask2, errorBundle.getpath());
			}
			else
			{
				printf("Vertical\n");
				errorBundle = verticalErrorMap(blended, image2, mask1, mask2, 
					textMask1, textMask2, scale);
				verticalBlending(blended, blended, image2,
					mask1, mask2, errorBundle.getpath());
			}
#ifdef DEBUG
			allTheSeams.push_back(errorBundle.getpath());
#endif // DEBUG
			intersection.release();
		}
		orMask = mask1 | mask2;
		andMask.release();
		mask1.release();
		mask2.release();
		image2.release();
		image1.release();
		textMask1.release();
		textMask2.release();
		getImage(j)->release();
	}
#ifdef DEBUG
	Mat seamMap;
	imwrite("YO//textMask.jpg", textMask);
	cvtColor(textMask, seamMap, CV_GRAY2RGB);
	for (int i = 0; i < allTheSeams.size(); i++)
	{
		for (int r = 0; r < allTheSeams[i].size(); r++)
		{
			seamMap.at<Vec3b>(allTheSeams[i][r]) = Vec3b(0, 255, 255);
		}
	}
	imwrite("YO//seamMap.jpg", seamMap);
#endif // DEBUG
	return  blended;
}

void MatchTracker::calculateIndividualImageBoundary()
{
	printf("Boundary after projecting to the big photo\n");
	for (int i = 0; i < size; i++)
	{
		images[i]->findBoundary();
	}

}


void MatchTracker::fixHomography()
{
}

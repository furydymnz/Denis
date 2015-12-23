#include "MatchTracker.h"
#include "ErrorBundle.h"
#include "utils.h"
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"


MatchTracker::MatchTracker(int size)
{
	this->size = size;
	pairNum.resize(size);
	pairFP.resize(size);
	routes.resize(size);
	images.clear();
	pairHomography.resize(size);
	pairError.resize(size);
	pairConnection.resize(size);
	for (int i = 0; i < size; i++)
	{
		pairNum[i].resize(size);
		pairFP[i].resize(size);
		pairHomography[i].resize(size);
		pairFP[i].clear();
		pairError[i].resize(size);
		pairConnection[i].resize(size);
		for (int j = 0; j < size; j++)
		{
			pairHomography[i][j] = Mat(3, 3, CV_64F, Scalar(-1, -1, -1));
			pairError[i][j] = -1;
		}
	}
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
}

void MatchTracker::assignHomographyToImage()
{
	for (int i = 0; i < size; i++)
	{
		if (images[i]->isEmpty()) continue;
		printf("assignHomographyToImage: pivotIndex:%d, pairHomographysize:%d \n", pivotIndex, pairHomography.size());
		printf("pairHomography[i].size():%d \n",  pairHomography[i].size());
		printf("(%d, %d)", i, pivotIndex);
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
	for (int i = 1; i < size; i++)
	{
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
		//char f[100];
		//sprintf(f, "YO/impad%d.jpg", i);
		//imwrite(f, temp);
		temp = Mat(imageSize, CV_8UC1, cv::Scalar(0));
		images[i]->getMask().copyTo(temp(Rect(1, 1, images[i]->getMask().cols, images[i]->getMask().rows)));
		images[i]->assignMask(temp);
		//images[i]->assignMask(temp);
		//sprintf(f, "YO/impadt%d.jpg", i);
		//imwrite(f, temp);
	}
}
void MatchTracker::applyHomography()
{
	

	for (int i = 0; i < size; i++)
	{	
		if (images[i]->isEmpty()) continue;
		
		
		warpPerspective(images[i]->getImage(), images[i]->getImage(), images[i]->getHomography(), imageSize, INTER_LINEAR, BORDER_CONSTANT);

		char f[100];
		sprintf(f, "YO/%d.jpg", i);
		imwrite(f, images[i]->getImage());
	}
}

void MatchTracker::generateMask()
{
	
	for (int i = 0; i < size; i++)
	{
		Mat mask((images[i]->getImage()).size(), CV_8UC1, cv::Scalar(255));

		if (images[i]->isEmpty()) continue;

		warpPerspective(mask, mask, images[i]->getHomography(), imageSize, INTER_LINEAR, BORDER_CONSTANT);
		
		char f[100];
		sprintf(f, "YO/m%d.jpg", i);
		imwrite(f, mask);
		images[i]->assignMask(mask);
	}
}

void MatchTracker::calculateTranslation()
{
	int dX, dY;
	Mat H;
	for (int i = 0; i < size; i++)
	{
		H = images[i]->getHomography();
		if (minX < 0)
		{
			H.row(0).col(2) = H.at<double>(0, 2) - minX;// / H.at<double>(0, 0);
			//dX = -minX / H.at<double>(0, 0);
		}
		else if (minX >= 0)
		{
			H.row(0).col(2) = H.at<double>(0, 2) - minX;// *H.at<double>(0, 0);
			//dX = minX * H.at<double>(0, 0);
		}
		if (minY < 0)
		{
			H.row(1).col(2) = H.at<double>(1, 2) - minY;// / H.at<double>(1, 1);
			//dY = -minY / H.at<double>(1, 1);
		}
		else if (minY >= 0)
		{
			H.row(1).col(2) = H.at<double>(1, 2) - minY;// *H.at<double>(1, 1);
			//dY = minY* H.at<double>(1, 1);
		}
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
		for (int r = i + 1; r < size; r++)
		{
			if (!getPairConnection(i, r))
				continue;
			Mat& mask1 = getImage(i)->getMask();
			Mat& mask2 = getImage(r)->getMask();

			andMask = mask1 & mask2;

			Mat intersection;
			findIntersection(mask1, mask2, intersection);
			Point2i pt1, pt2;
			if (findIntersectionPts(pt1, pt2, intersection, andMask) == -1)
			{
				assignErrorPair(i, r, -1);
				continue;
			}
			
			if (abs(pt1.x - pt2.x) > abs(pt1.y - pt2.y))
			{
				errorBundle = horizontalErrorMap(images[i]->getImage(), images[r]->getImage(), mask1, mask2, i, r);
			}
			else
			{
				errorBundle = verticalErrorMap(images[i]->getImage(), images[r]->getImage(), mask1, mask2, i, r);
			}

			double pathError = errorBundle.getPathError() / errorBundle.getpath().size();

			assignErrorPair(i, r, pathError);

		}
	}
}

void MatchTracker::calculateErrorSeamTest()
{
	ErrorBundle errorBundle;
	const float fpThreshold = 0.3;
	const int fpBottomLimit = 10;
	for (int i = 0; i < size-1; i++)
	{
		for (int j = i + 1; j < size; j++)
		{
			
		}
	}

}


void MatchTracker::fixHomography()
{
	/*
	Mat h = Mat::eye(3, 3, CV_8UC1);
	h.row(0).col(2) = -minX;
	h.row(1).col(2) = -minY;

	for (int i = 0; i < size; i++)
	{
		images[i]->assignHomography(images[i]->getHomography())
	}
	*/
}

#include "MatchTracker.h"
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"

MatchTracker::MatchTracker(const MatchTracker & m)
{
	this->size = m.size;
	this->images = m.images;
	this->routes = m.routes;
	this->pairNum = m.pairNum;
	this->pairFP = m.pairFP;
	this->pairHomography = m.pairHomography;
	this->maxX = m.maxX;
	this->maxY = m.maxY;
	this->minX = m.minX;
	this->minY = m.minY;
}

MatchTracker& MatchTracker:: operator=(const MatchTracker& m)
{
	this->size = m.size;
	this->images = m.images;
	this->routes = m.routes;
	this->pairNum = m.pairNum;
	this->pairFP = m.pairFP;
	this->pairHomography = m.pairHomography;
	this->maxX = m.maxX;
	this->maxY = m.maxY;
	this->minX = m.minX;
	this->minY = m.minY;
}

MatchTracker::MatchTracker(int size)
{
	this->size = size;
	pairNum.resize(size);
	pairFP.resize(size);
	routes.resize(size);
	images.clear();
	pairHomography.resize(size);
	for (int i = 0; i < size; i++)
	{
		pairNum[i].resize(size);
		pairFP[i].resize(size);
		pairHomography[i].resize(size);
		pairFP[i].clear();

		for (int j = 0; j < size; j++)
			pairHomography[i][j] = Mat(3, 3, CV_64F, Scalar(-1, -1, -1));
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

	printf("minX: %5d maxX: %5d minY: %5d maxY: %5d\n",
		minX, maxX, minY, maxY);
}
void MatchTracker::applyHomographyTest()
{
	Mat rotated;
	Mat h;
	for (int i = 0; i < size; i++)
	{	
		if (images[i]->isEmpty()) continue;
		h = images[i]->getHomography().clone();
		//h.row(2).col(0) = 0;
		//h.row(2).col(1) = 0;
		
		int sizeX = maxX - minX, sizeY = maxY - minY;
		//warpPerspective(images[i]->getImage(), rotated, images[i]->getHomography(), Size(sizeX, sizeY), INTER_LINEAR, BORDER_CONSTANT);
		warpPerspective(images[i]->getImage(), rotated, h, Size(sizeX, sizeY), INTER_LINEAR, BORDER_CONSTANT);
		char f[100];
		sprintf(f, "YO/%d.jpg", i);
		imwrite(f, rotated);
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
			H.row(0).col(2) = H.at<double>(0, 2) - minX / H.at<double>(0, 0);
			dX = -minX / H.at<double>(0, 0);
		}
		else if (minX >= 0)
		{
			H.row(0).col(2) = H.at<double>(0, 2) - minX * H.at<double>(0, 0);
			dX = minX * H.at<double>(0, 0);
		}

		if (minY < 0)
		{
			H.row(1).col(2) = H.at<double>(1, 2) - minY / H.at<double>(1, 1);
			dY = -minY / H.at<double>(1, 1);
		}
		else if (minY >= 0)
		{
			H.row(1).col(2) = H.at<double>(1, 2) - minY* H.at<double>(1, 1);
			dY = minY* H.at<double>(1, 1);
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

void MatchTracker::createMasks()
{
	for (int i = 0; i < size; i++)
	{
		images[i]->assignMask(Mat(images[i]->getSize(), CV_8UC1, cv::Scalar(255)));
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

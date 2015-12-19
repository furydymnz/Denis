#include "MatchTracker.h"
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"

void MatchTracker::assignHomographyToImage()
{
	for (int i = 0; i < size; i++)
	{
		printf("assignHomographyToImage: pivotIndex:%d, pairHomographysize:%d \n", pivotIndex, pairHomography.size());
		printf("pairHomography[i].size():%d \n",  pairHomography[i].size());
		printf("(%d, %d)", i, pivotIndex);
		if (((pairHomography[i][pivotIndex])).at<double>(0, 0) != -1)
		{
			printf("assigining\n");
		
			(images[i])->assignHomography((pairHomography[i][pivotIndex]));
			printf("done\n");
		}
		else
			printf("FUCK!!!!!!!!!!!!!!!!!!!!\n");
		
	}
}

void MatchTracker::calculateBoundary()
{
	images[0]->findBoundary();
	maxX = images[0]->maxX;
	minX = images[0]->minX;
	maxY = images[0]->maxY;
	minY = images[0]->minY;
	for (int i = 1; i < size; i++)
	{
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
		h = images[i]->getHomography().clone();
		h.row(2).col(0) = 0;
		h.row(2).col(1) = 0;
		
		int sizeX = maxX - minX, sizeY = maxY - minY;
		//warpPerspective(images[i]->getImage(), rotated, images[i]->getHomography(), Size(sizeX, sizeY), INTER_LINEAR, BORDER_CONSTANT);
		warpPerspective(images[i]->getImage(), rotated, h, Size(sizeX, sizeY), INTER_LINEAR, BORDER_CONSTANT);
		char f[100];
		sprintf(f, "YO/%d.jpg", i);
		imwrite(f, rotated);
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
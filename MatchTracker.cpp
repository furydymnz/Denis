#include "MatchTracker.h"


void MatchTracker::assignHomographyToImage()
{
	for (int i = 0; i < size; i++)
	{
		printf("assignHomographyToImage: pivotIndex:%d, pairHomographysize:%d \n", pivotIndex, pairHomography.size());
		printf("pairHomography[i].size():%d \n",  pairHomography[i].size());
		printf("(%d, %d)", i, pivotIndex);
		if (!((pairHomography[i][pivotIndex])).isEmpty())
		{
			printf("assigining\n");
			for (int rss = 0; rss < 3; rss++)
			{
				for (int pss = 0; pss < 3; pss++)
					printf("%20.8lf", (getHomographyPair(0, 0)).at(rss, pss));
				printf("\n");
			}
			
			(images[i])->assignHomography((pairHomography[i][pivotIndex]));
			printf("done\n");
		}
		else
			printf("FUCK!!!!!!!!!!!!!!!!!!!!\n");
		
	}
}

void MatchTracker::calculateBoundary(int &minX, int &minY, int &maxX, int &maxY)
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
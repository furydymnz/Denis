#include "RouteHandler.h"
#include "utils.h"

using namespace std;

//Find connecting route
void RouteHandler::findConnectingRoute(MatchTracker &matchTracker)
{
	int imageCount = matchTracker.getSize();
	//int pivotIndex = 0;
	vector <int> route;
	vector <pair<int, int> > stack;
	int next, index, current;
	for (int i = 0; i < imageCount; i++)
	{
		stack.clear();
		route.clear();
		if (i == matchTracker.pivotIndex) continue;
		route.push_back(i);
		index = 1;
		int maxMatch = 0;
		for (int r = 0; r < imageCount; r++)
		{
			if (r == i) continue;
			if (!matchTracker.getPairConnection(i, r))
				continue;
			stack.push_back(pair<int, int>(r, index));
		}

		while (!stack.empty())
		{
			next = stack.back().first;
			route.push_back(next);
			stack.pop_back();

			if (next == matchTracker.pivotIndex)
			{
				matchTracker.assignRoute(i, route);
				if (stack.size() == 0)
					current = 0;
				else
					current = stack.back().second;
				while (route.size() > current)
					route.pop_back();
				index = current;
				continue;
			}
			bool isPushed = false;
			index++;
			for (int r = 0; r < imageCount; r++)
			{
				if (r == next) continue;

				if (!matchTracker.getPairConnection(next, r))
					continue;

				bool duplicate = false;

				for (int p = 0; p < route.size(); p++)
				if (route[p] == r) duplicate = true;
				if (!duplicate)
				{
					stack.push_back(pair<int, int>(r, index));
					isPushed = true;
				}

			}
			if (!isPushed){
				if (stack.size() == 0)
					current = 0;
				else
					current = stack.back().second;
				while (route.size() > current)
					route.pop_back();

				index--;

			}
		}
	}
	countRoutesWeight(matchTracker);
	printf("Before:\n");
	printRoutes(matchTracker);
	removeRedundantRoutes(matchTracker);
	printf("After:\n");
	printRoutes(matchTracker);
}

void RouteHandler::removeRedundantRoutes(MatchTracker &matchTracker)
{
	int imageCount = matchTracker.getSize();
	for (int i = 0; i < imageCount; i++)
	{
		if (i == matchTracker.pivotIndex) continue;
		if (matchTracker.images[i]->isEmpty()) continue;
		int maxAvgWeight = 0, maxAvgIndex = -1;
		Route &currentRoutes = matchTracker.getRoute(i);
		int routeCount = currentRoutes.route.size();
		for (int p = 0; p < routeCount; p++)
		{
			if (maxAvgWeight < currentRoutes.routeWeightAvg[p])
			{
				maxAvgIndex = p;
				maxAvgWeight = currentRoutes.routeWeightAvg[p];
			}
			else if (maxAvgWeight == currentRoutes.routeWeightAvg[p])
			{
				if (maxAvgIndex == -1)
					maxAvgIndex = p;
				else
					maxAvgIndex =
						currentRoutes.route[p].size() > currentRoutes.route[maxAvgIndex].size() ?
					maxAvgIndex : p;
				maxAvgWeight = currentRoutes.routeWeightAvg[maxAvgIndex];
			}
		}
		if (maxAvgIndex == -1) continue;
		vector<int> route = currentRoutes.route[maxAvgIndex];
		currentRoutes.route.clear();
		currentRoutes.route.push_back(route);
		currentRoutes.routeWeightAvg.clear();
		currentRoutes.routeWeightAvg.push_back(maxAvgWeight);
	}
}
void RouteHandler::countRoutesWeight(MatchTracker &matchTracker)
{
	int imageCount = matchTracker.getSize();
	for (int i = 0; i < imageCount; i++)
	{
		if (i == matchTracker.pivotIndex) continue;
		Route &currentRoutes = matchTracker.getRoute(i);
		int routeCount = currentRoutes.route.size();
		for (int r = 0; r < routeCount; r++)
		{
			int routeLength = currentRoutes.route[r].size();
			int weight = 0;
			for (int p = 0; p < routeLength; p++)
			{
				int x = currentRoutes.route[r][p];
				weight += matchTracker.getPairNum(i, x);
			}
			currentRoutes.routeWeight.push_back(weight);
			currentRoutes.routeWeightAvg.push_back(weight / routeLength);
		}
	}

}

void RouteHandler::printRoutes(MatchTracker &matchTracker)
{

	int imageCount = matchTracker.getSize();
	for (int i = 0; i < imageCount; i++)
	{
		if (i == matchTracker.pivotIndex) continue;
		Route &currentRoutes = matchTracker.getRoute(i);
		int routeCount = currentRoutes.route.size();
		if (routeCount == 0)
		{
			printf("%d has no route\n", i);
			matchTracker.images[i]->setEmpty(1);
			continue;
		}
		for (int r = 0; r < routeCount; r++)
		{
			int routeLength = currentRoutes.route[r].size();
			int weight = currentRoutes.routeWeight[r];
			int weightAvg = currentRoutes.routeWeightAvg[r];
			for (int p = 0; p < routeLength; p++)
			{
				int x = currentRoutes.route[r][p];

				printf("%d", x);
				if (p != routeLength - 1)
					printf("->");
			}

			printf("\t\t\t--- %d %d\n", weight, weightAvg);
		}
	}

}


void RouteHandler::calculateHomography(MatchTracker &matchTracker)
{
	
	const int pivotIndex = 0;
	vector<int> routeIndex, routeSize;
	//sort
	for (int i = 0; i < matchTracker.getSize(); i++)
	{
		if (i == pivotIndex || matchTracker.images[i]->isEmpty()) continue;
		routeIndex.push_back(i);
		routeSize.push_back(matchTracker.getRoute(i).route[0].size());
	}
	for (int i = 0; i < routeSize.size() - 1; i++)
	{
		if (matchTracker.images[i]->isEmpty()) continue;
		for (int j = i + 1; j < routeSize.size(); j++)
		{
			if (matchTracker.images[j]->isEmpty()) continue;
			if (routeSize[j] < routeSize[i])
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

	for (int i = 0; i < routeIndex.size(); i++)
		printf("%d ", routeIndex[i]);
	printf("\n");

	Mat H = Mat::eye(3, 3, CV_64F);

	matchTracker.assignHomographyPair(pivotIndex, pivotIndex, H);

	int startpt;
	for (int i = 0; i < routeIndex.size(); i++)
	{
		if (matchTracker.images[i]->isEmpty()) continue;
		printf("For %d:\n", i);
		printf("routeSize: %d\n", routeSize[i]);
		H = Mat::eye(3, 3, CV_64F);
		vector<int> &route = matchTracker.getRoute(routeIndex[i]).route[0];
		startpt = route[0];
		for (int j = 0; j < routeSize[i] - 1; j++)
		{

			printf("find pair (%d, %d)\n", route[j], route[j + 1]);
			if ((matchTracker.getHomographyPair(route[j], route[j + 1])).at<double>(0, 0)!=-1)
			{
				printf("%d %d have pair\n", route[j], route[j + 1]);
				H = H * (matchTracker.getHomographyPair(route[j], route[j + 1]));
				printf("assigned Address: %u\n", &H);
				matchTracker.assignHomographyPair(startpt, route[j + 1], H);
			}
			else
			{
				printf("%d %d do not have pair\n", route[j], route[j + 1]);
				int reverse = 0;
				IpPairVec ip = matchTracker.getPairFP(route[j], route[j + 1], reverse);
				Mat tempH = findhomography(ip, reverse);
				printf("assigned Address: %u\n", &H);
				matchTracker.assignHomographyPair(route[j], route[j + 1], tempH);
				H = H*tempH;
				
				if (route[j] != startpt)
				{
					printf("assigned Address: %u\n", &tempH);
					matchTracker.assignHomographyPair(startpt, route[j + 1], H);
				}
			}
		}
		printf("\n");
	}
}

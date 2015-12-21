#include "Blender.h"

Blender::Blender(MatchTracker *matchTracker)
{
	this->matchTracker = matchTracker;
}


Blender::~Blender()
{
}

void Blender::generateBlendingOrder()
{
	int imageCount = this->matchTracker->getSize();

	const float fpThreshold = 0.3;
	const int fpBottomLimit = 10;
	vector <int> route;
	vector <pair<int, int> > stack;
	vector <vector <int> > tempOrder;
	int next, index, current;
	for (int i = 0; i < imageCount; i++)
	{
		stack.clear();
		route.clear();
		route.push_back(i);
		index = 1;
		int maxMatch = 0;
		vector<int> currentLine = this->matchTracker->getPairNum(i);
		for (int r = 0; r < imageCount; r++)
		{
			if (r == i) continue;
			if (currentLine[r]>maxMatch)
				maxMatch = currentLine[r];
		}
		for (int r = 0; r < imageCount; r++)
		{
			if (r == i) continue;
			if (currentLine[r] < maxMatch*fpThreshold ||
				currentLine[r] < fpBottomLimit)
				continue;
			stack.push_back(pair<int, int>(r, index));
		}

		while (!stack.empty())
		{
			next = stack.back().first;
			route.push_back(next);
			stack.pop_back();

			if (route.size() == imageCount)
			{
				tempOrder.push_back(route);
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
			vector<int> currentLine = this->matchTracker->getPairNum(next);
			for (int r = 0; r < imageCount; r++)
			{
				if (r == i) continue;
				if (currentLine[r]>maxMatch)
					maxMatch = currentLine[r];
			}
			for (int r = 0; r < imageCount; r++)
			{
				if (r == next) continue;

				if (currentLine[r] < maxMatch*fpThreshold ||
					currentLine[r] < fpBottomLimit)
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
			if (!isPushed) {
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
	
	for (int i = 0; i < tempOrder.size(); i++)
	{
		vector <int> rev; 
		rev.resize(tempOrder[i].size());
		reverse_copy(tempOrder[i].begin(), tempOrder[i].end(), rev.begin());
		for (int j = i + 1; j < tempOrder.size(); j++)
		{
				if (rev == tempOrder[j])
				{
					tempOrder.erase(tempOrder.begin()+j);
					break;
				}
		}
		blendingOrder.push_back(tempOrder[i]);
	}
	
}

void Blender::printBlendingOrder()
{
	for (int i = 0; i < blendingOrder.size(); i++)
	{
		for (int j = 0; j < blendingOrder[i].size(); j++)
		{
			printf("%3d", blendingOrder[i][j]);
		}
		printf("\n");
	}
}

void Blender::findIntersection(Mat& mask1, Mat& mask2, Mat& intersection)
{
	Mat border1 = border(mask1);
	Mat border2 = border(mask2);
	intersection = ~(border1 | border2);
	imwrite("test/inter.jpg", intersection);

}

cv::Mat Blender::border(cv::Mat mask)
{
	cv::Mat gx;
	cv::Mat gy;

	cv::Sobel(mask, gx, CV_32F, 1, 0, 3);
	cv::Sobel(mask, gy, CV_32F, 0, 1, 3);

	cv::Mat border;
	cv::magnitude(gx, gy, border);

	return border <100;
}
#include "Blender.h"

Blender::Blender(MatchTracker *matchTracker)
{
	this->matchTracker = matchTracker;
}


Blender::~Blender()
{
}

void Blender::blend()
{
	generateBlendingOrder();
	printBlendingOrder();
	Mat mask1, mask2;

	for (int i = 0; i < blendingOrder.size(); i++)
	{

		for (int j = 0; j < blendingOrder[i].size() - 1; j++)
		{
			mask1 = matchTracker->getImage(blendingOrder[i][j])->getMask();
			mask2 = matchTracker->getImage(blendingOrder[i][j+1])->getMask();
			if (j != 0)
				mask1 = mask1 | andMask;

			andMask = mask1 & mask2;

			Mat intersection;
			findIntersection(mask1, mask2, intersection);
			Point2i pt1, pt2;
			if (findIntersectionPts(pt1, pt2, intersection) == -1)
				break;


			findSeam(calculateSeamDirection(pt1, pt2));
			calculateSeamError();

		}
	}
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

int Blender::findIntersectionPts(Point2i& pt1, Point2i& pt2, Mat& intersection)
{
	vector<Point2i> interpts;

	//get points from intersection Mat
	for (int i = 0; i < intersection.rows; i++)
	{
		for (int j = 0; j < intersection.cols; j++)
		{
			if (intersection.at<unsigned char>(i, j) != 0 && andMask.at<unsigned char>(i, j) != 0)
				interpts.push_back(Point2i(j, i));
		}
	}

	if (interpts.size() == 0) 
	{
		printf("No intersection\n");
		return -1;
	}
		

	//find the pair of points that have the longest distance
	double maxDis = 0;

	for (int i = 0; i < interpts.size() - 1; i++)
	{
		for (int j = i + 1; j < interpts.size(); j++)
		{
			double tempDis = DIS(interpts[i].x, interpts[i].y, interpts[j].x, interpts[j].y);
			if (tempDis > maxDis)
			{
				maxDis = tempDis;
				pt1 = interpts[i];
				pt2 = interpts[j];
			}
		}
	}
	//let pt1 be the leftmost point
	Point2i temp;
	if (pt1.x > pt2.x) {
		temp = pt1;
		pt1 = pt2;
		pt2 = temp;
	}

	printf("pt1.x:%d pt1.y:%d\npt2.x:%d pt2.y:%d\n", pt1.x, pt1.y, pt2.x, pt2.y);
	return 0;
}

Blender::seamDirection Blender::calculateSeamDirection(Point2i & pt1, Point2i & pt2)
{
	if (abs(pt1.x - pt2.x) > abs(pt1.y - pt2.y))
		return HORIZONTAL;
	return VERTICAL;
}

void Blender::findSeam(seamDirection seamdir)
{

}

void Blender::calculateSeamError()
{
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

double Blender::ComputeError(const cv::Mat& image1, const cv::Mat& image2, int i, int c)
{
	cv::Vec3b c1 = image1.at<Vec3b>(i, c);
	cv::Vec3b c2 = image2.at<Vec3b>(i, c);
	double b = (c1[0] - c2[0])*(c1[0] - c2[0]);
	double g = (c1[1] - c2[1])*(c1[1] - c2[1]);
	double r = (c1[2] - c2[2])*(c1[2] - c2[2]);
	return sqrt(b + g + r);
}

double Blender::getDPError(int i, int j, Mat &errorMap, direction **dirMap)
{
	if (i < 0 || j < 0 || j > errorMap.cols || i > errorMap.rows || dirMap[i][j] == YO)
		return -1;
	return errorMap.at<double>(i, j);
}

void Blender::ComputeError(int i, int j, Mat &image1, Mat &image2, direction **dirMap, Mat &errorMap)
{
	double eCurrent = ComputeError(image1, image2, i, j);
	//TL T TR L R
	double errors[5] = { getDPError(i - 1, j - 1, errorMap, dirMap),
		getDPError(i - 1, j, errorMap, dirMap),
		getDPError(i - 1, j + 1, errorMap, dirMap),
		getDPError(i, j - 1, errorMap, dirMap),
		getDPError(i, j + 1, errorMap, dirMap), };
	double minError = 110000;
	int minDir = -1;
	for (int i = 0; i<5; i++)
	{
		if (errors[i] == -1)
			continue;
		if (errors[i]<minError)
		{
			minError = errors[i];
			minDir = i;
		}
	}
	if (minDir == -1)
	{
		dirMap[i][j] = CURRENT;
		errorMap.at<double>(i, j) = eCurrent;
	}
	else
	{
		switch (minDir)
		{
		case 0:
			dirMap[i][j] = TOPLEFT;
			break;
		case 1:
			dirMap[i][j] = TOP;
			break;
		case 2:
			dirMap[i][j] = TOPRIGHT;
			break;
		case 3:
			dirMap[i][j] = LEFT;
			break;
		case 4:
			dirMap[i][j] = RIGHT;
			break;
		default:
			dirMap[i][j] = CURRENT;
		}
		errorMap.at<double>(i, j) = eCurrent + minError;
	}

}
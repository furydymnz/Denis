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

void Blender::ComputeHorizontalError(int i, int j, Mat &image1, Mat &image2, direction **dirMap, Mat &errorMap)
{
	double eCurrent = ComputeError(image1, image2, i, j);
	//TL T L BL B
	double errors[5] = { getDPError(i - 1, j - 1, errorMap, dirMap),
		getDPError(i - 1, j, errorMap, dirMap),
		getDPError(i, j - 1, errorMap, dirMap),
		getDPError(i + 1, j - 1, errorMap, dirMap),
		getDPError(i + 1, j, errorMap, dirMap), };
	double minError = DBL_MAX;
	int minDir = -1;
	for (int i = 0; i<5; i++)
	{
		if (errors[i] == -1)
			continue;
		if (errors[i] < minError)
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
			dirMap[i][j] = LEFT;
			break;
		case 3:
			dirMap[i][j] = BOTTOMLEFT;
			break;
		case 4:
			dirMap[i][j] = BOTTOM;
			break;
		default:
			dirMap[i][j] = CURRENT;
		}
		errorMap.at<double>(i, j) = eCurrent + minError;
	}

}

Mat Blender::verticalBlending(cv::Mat image1, cv::Mat image2, Mat mask1, Mat mask2, vector<Point2i>& seam)
{
	// edited: find regions where no mask is set
	// compute the region where no mask is set at all, to use those color values unblended

	//5-way DP seam finder
	cv::Mat bothMasks = mask1 | mask2;
	cv::Mat andMasks = mask1 & mask2;
	cv::Mat noMask = 255 - bothMasks;
	Mat xormask2 = mask2 ^ andMasks;
	Mat xormask1 = mask1 ^ andMasks;
	xormask1 = xormask1 > 0;
	xormask2 = xormask2 > 0;
	imwrite("test/xormaks1.jpg", xormask1);
	imwrite("test/xormaks2.jpg", xormask2);
	Mat errorMap(image1.size(), CV_64FC1);
	Mat errorGraph(image1.size(), CV_8UC1);
	// ------------------------------------------
	double eTopLeft = 0, eTopRight = 0, eTop = 0, eLeft = 0, eRight = 0, eCurrent;

	double maxError = 0;
	imwrite("test/andmask.jpg", andMasks);
	Mat intersection;
	findIntersection(mask1, mask2, intersection);
	Point2i pt1, pt2;
	findIntersectionPts(pt1, pt2, intersection);

	//DP
	direction **dirMap;
	dirMap = new direction *[image1.rows];
	for (int i = 0; i < image1.rows; i++)
	{
		dirMap[i] = new direction[image1.cols];
		for (int j = 0; j < image1.cols; j++)
		{
			dirMap[i][j] = YO;
		}
	}

	for (int i = pt1.y; i <= pt2.y; i++)
	{
		/*
		if( i == pt1.y )
		{
		for (int j = 0; j < errorMap.cols; j++)
		{
		if (andMasks.at<unsigned char>(i, j)==0)
		continue;
		errorMap.at<double>(i, j) = ComputeError(image1, image2, i, j);
		dirMap[i][j] = CURRENT;
		}
		}
		*/
		if (i == pt1.y)
		{
			for (int j = 0; j < errorMap.cols; j++)
			{
				if (andMasks.at<unsigned char>(i, j) == 0)
					continue;
				if (j == pt1.x)
				{
					errorMap.at<double>(i, j) = ComputeError(image1, image2, i, j);
					dirMap[i][j] = CURRENT;
				}
				else
				{
					ComputeError(i, j, image1, image2, dirMap, errorMap);
				}
			}
			for (int j = errorMap.cols - 1; j >= 0; j--)
			{
				if (andMasks.at<unsigned char>(i, j) == 0)
					continue;
				if (j == pt1.x)
				{
					errorMap.at<double>(i, j) = ComputeError(image1, image2, i, j);
					dirMap[i][j] = CURRENT;
				}
				else
				{
					ComputeError(i, j, image1, image2, dirMap, errorMap);
				}
			}
		}
		else
		{
			for (int j = 0; j<errorMap.cols; j++)
			{
				if (andMasks.at<unsigned char>(i, j) == 0)
					continue;
				ComputeError(i, j, image1, image2, dirMap, errorMap);
			}
			for (int j = errorMap.cols - 1; j >= 0; j--)
			{
				if (andMasks.at<unsigned char>(i, j) == 0)
					continue;
				ComputeError(i, j, image1, image2, dirMap, errorMap);
			}
		}
	}
	for (int i = pt1.y; i <= pt2.y; i++)
	{
		for (int j = 0; j < errorMap.cols; j++)
			if (j + 1<errorMap.cols && j>0
				&& dirMap[i][j - 1] == RIGHT && dirMap[i][j] == LEFT)
			{
				dirMap[i][j - 1] = LEFT;
				dirMap[i][j] = RIGHT;
			}
	}


	//draw errorGraph
	for (int i = pt1.y; i <= pt2.y; i++)
	{
		for (int j = 0; j < errorMap.cols; j++)
		{
			if (maxError < errorMap.at<double>(i, j))
				maxError = errorMap.at<double>(i, j);
		}
	}
	double scale = 255 / maxError;
	for (int i = 0; i < errorGraph.rows; i++)
	{
		for (int r = 0; r < errorGraph.cols; r++)
		{
			if (andMasks.at<unsigned char>(i, r) == 0)
				continue;
			errorGraph.at<unsigned char>(i, r) =
				(int)((255 / maxError)*
					errorMap.at<double>(i, r));

		}
	}
	imwrite("test/errorMap.jpg", errorGraph);

	Mat errorSeam(image1.size(), CV_8UC3);
	Mat seamMap(image1.size(), CV_8UC1, Scalar(0));
	cvtColor(errorGraph, errorSeam, CV_GRAY2BGR);

	double minError = errorMap.at<double>(pt2.y, pt2.x);
	Point2i startpt = pt2;
	/*
	for(int x = 0;x < errorMap.cols;x++)
	{
	if(andMasks.at<unsigned char>(pt2.y, x) != 0 && dirMap[pt2.y][x] != YO)
	{
	if(errorMap.at<double>(pt2.y, x) < minError)
	{
	minError = errorMap.at<double>(pt2.y, x);
	startpt = Point2i(x, pt2.y);
	}
	}
	}*/
//	vector<Point2i> seam;
	seam.push_back(startpt);
	int x = startpt.x, y = startpt.y;
	while (1) {
		if (dirMap[y][x] == CURRENT || dirMap[y][x] == YO)
			break;
		switch (dirMap[y][x]) {
		case TOPLEFT:
			x--;
			y--;
			break;
		case TOP:
			y--;
			break;
		case TOPRIGHT:
			x++;
			y--;
			break;
		case LEFT:
			x--;
			break;
		case RIGHT:
			x++;
			break;
		}
		//char c;
		//printf("x:%d y:%d\ndir:%d\n",x, y,dirMap[y][x]);
		//scanf("%c",&c);
		seam.push_back(Point2i(x, y));
		errorSeam.at<Vec3b>(y, x) = Vec3b(0, 0, 255);
		seamMap.at<unsigned char>(y, x) = 255;
	}

	imwrite("test/errorSeam.jpg", errorSeam);
	imwrite("test/seamMap.jpg", seamMap);

	//blending
	Mat blended(image1.size(), CV_8UC3, cv::Scalar(0, 0, 0));
	for (int i = 0; i < blended.rows; i++)
	{
		bool passedSeam = false;
		for (int j = 0; j < blended.cols; j++)
		{
			if (andMasks.at<unsigned char>(i, j) == 255)
			{
				if (seamMap.at<unsigned char>(i, j) == 255)
					passedSeam = true;
				if (!passedSeam)
					blended.at<Vec3b>(i, j) = image1.at<Vec3b>(i, j);
				else
					blended.at<Vec3b>(i, j) = image2.at<Vec3b>(i, j);
			}
			else if (xormask1.at<unsigned char>(i, j) == 255)
				blended.at<Vec3b>(i, j) = image1.at<Vec3b>(i, j);
			else
				blended.at<Vec3b>(i, j) = image2.at<Vec3b>(i, j);

		}
	}

	return blended;
}

Mat Blender::horizontalBlending(cv::Mat image1, cv::Mat image2, Mat mask1, Mat mask2, int dY, vector<Point2i>& seam)
{
	// edited: find regions where no mask is set
	// compute the region where no mask is set at all, to use those color values unblended

	//5-way DP seam finder
	cv::Mat bothMasks = mask1 | mask2;
	cv::Mat andMasks = mask1 & mask2;
	cv::Mat noMask = 255 - bothMasks;
	Mat xormask2 = mask2 ^ andMasks;
	Mat xormask1 = mask1 ^ andMasks;
	xormask1 = xormask1 > 0;
	xormask2 = xormask2 > 0;
	imwrite("test/xormaks1.jpg", xormask1);
	imwrite("test/xormaks2.jpg", xormask2);
	Mat &errorMap = Mat(image1.size(), CV_64FC1);
	Mat errorGraph(image1.size(), CV_8UC1);
	// ------------------------------------------
	double eTopLeft = 0, eBottomLeft = 0, eTop = 0, eLeft = 0, eBottom = 0, eCurrent;

	double maxError = 0;
	imwrite("test/andmask.jpg", andMasks);
	Mat intersection;
	findIntersection(mask1, mask2, intersection);
	Point2i pt1, pt2;
	findIntersectionPts(pt1, pt2, intersection);

	//DP
	direction **dirMap;
	dirMap = new direction *[image1.rows];
	for (int i = 0; i < image1.rows; i++)
	{
		dirMap[i] = new direction[image1.cols];
		for (int j = 0; j < image1.cols; j++)
		{
			//YO is meaningless
			dirMap[i][j] = YO;
		}
	}

	for (int j = pt1.x; j <= pt2.x; j++)
	{
		if (j == pt1.x)
		{
			for (int i = 0; i < errorMap.rows; i++)
			{
				if (andMasks.at<unsigned char>(i, j) == 0)
					continue;
				if (i == pt1.y)
				{
					errorMap.at<double>(i, j) = ComputeError(image1, image2, i, j);
					dirMap[i][j] = CURRENT;
				}
				else
				{
					ComputeHorizontalError(i, j, image1, image2, dirMap, errorMap);
				}
			}
			for (int i = errorMap.rows - 1; i >= 0; i--)
			{
				if (andMasks.at<unsigned char>(i, j) == 0)
					continue;
				if (i == pt1.y)
				{
					errorMap.at<double>(i, j) = ComputeError(image1, image2, i, j);
					dirMap[i][j] = CURRENT;
				}
				else
				{
					ComputeHorizontalError(i, j, image1, image2, dirMap, errorMap);
				}
			}
		}
		else
		{
			for (int i = 0; i < errorMap.rows; i++)
			{
				if (andMasks.at<unsigned char>(i, j) == 0)
					continue;
				ComputeHorizontalError(i, j, image1, image2, dirMap, errorMap);
			}
			for (int i = errorMap.rows - 1; i >= 0; i--)
			{
				if (andMasks.at<unsigned char>(i, j) == 0)
					continue;
				ComputeHorizontalError(i, j, image1, image2, dirMap, errorMap);
			}
		}

	}
	for (int j = pt1.x; j <= pt2.x; j++)
	{
		for (int i = 0; i < errorMap.rows; i++)
			if (i + 1<errorMap.rows && i>0
				&& dirMap[i - 1][j] == BOTTOM && dirMap[i][j] == TOP)
			{
				//may have some error
				dirMap[i - 1][j] = TOP;
				dirMap[i][j] = BOTTOM;
			}
	}

	//draw errorGraph
	for (int j = pt1.x; j <= pt2.x; j++)
	{
		for (int i = 0; i < errorMap.rows; i++)
		{
			if (maxError < errorMap.at<double>(i, j))
				maxError = errorMap.at<double>(i, j);
		}
	}
	double scale = 255 / maxError;
	for (int i = 0; i < errorGraph.rows; i++)
	{
		for (int r = 0; r < errorGraph.cols; r++)
		{
			if (andMasks.at<unsigned char>(i, r) == 0)
				continue;
			errorGraph.at<unsigned char>(i, r) =
				(int)((255 / maxError)*
					errorMap.at<double>(i, r));

		}
	}
	imwrite("test/errorMap.jpg", errorGraph);

	Mat errorSeam(image1.size(), CV_8UC3);
	Mat seamMap(image1.size(), CV_8UC1, Scalar(0));
	cvtColor(errorGraph, errorSeam, CV_GRAY2BGR);

	double minError = errorMap.at<double>(pt2.y, pt2.x);
	Point2i startpt = pt2;
	/*
	for(int y = 0;y < errorMap.rows;y++)
	{
	if(andMasks.at<unsigned char>(y, pt2.x) != 0 && dirMap[y][pt2.x] != YO)
	{
	if(errorMap.at<double>(y, pt2.x) < minError)
	{
	minError = errorMap.at<double>(y, pt2.x);
	startpt = Point2i(pt2.x, y);
	}
	}
	}*/
//	vector<Point2i> seam;
	seam.push_back(startpt);
	int x = startpt.x, y = startpt.y;
	while (1) {
		if (dirMap[y][x] == CURRENT || dirMap[y][x] == YO)
			break;
		switch (dirMap[y][x]) {
		case TOPLEFT:
			x--;
			y--;
			break;
		case TOP:
			y--;
			break;
		case LEFT:
			x--;
			break;
		case BOTTOMLEFT:
			x--;
			y++;
			break;
		case BOTTOM:
			y++;
			break;
		}
		//char c;
		//printf("x:%d y:%d\ndir:%d\n",x, y,dirMap[y][x]);
		//scanf("%c",&c);
		seam.push_back(Point2i(x, y));
		errorSeam.at<Vec3b>(y, x) = Vec3b(0, 0, 255);
		seamMap.at<unsigned char>(y, x) = 255;
	}

	imwrite("test/errorSeam.jpg", errorSeam);
	imwrite("test/seamMap.jpg", seamMap);


	//blending
	Mat blended(image1.size(), CV_8UC3, cv::Scalar(0, 0, 0));

	bool passedSeam = false;
	if (dY >= 0)
	{
		printf("dY>=0\n");
		for (int j = 0; j < blended.cols; j++) {
			passedSeam = false;
			for (int i = 0; i < blended.rows; i++) {
				if (andMasks.at<unsigned char>(i, j) != 0)
				{
					if (seamMap.at<unsigned char>(i, j) != 0)
						passedSeam = true;
					if (!passedSeam)
						blended.at<Vec3b>(i, j) = image1.at<Vec3b>(i, j);
					else
						blended.at<Vec3b>(i, j) = image2.at<Vec3b>(i, j);
				}
				else if (xormask1.at<unsigned char>(i, j) != 0)
					blended.at<Vec3b>(i, j) = image1.at<Vec3b>(i, j);
				else
					blended.at<Vec3b>(i, j) = image2.at<Vec3b>(i, j);
			}
		}
	}
	else
	{
		printf("dY<0\n");
		for (int j = 0; j < blended.cols; j++) {
			passedSeam = false;
			for (int i = 0; i < blended.rows; i++) {
				if (andMasks.at<unsigned char>(i, j) == 255)
				{
					if (seamMap.at<unsigned char>(i, j) == 255)
						passedSeam = true;
					if (!passedSeam)
						blended.at<Vec3b>(i, j) = image2.at<Vec3b>(i, j);
					else
						blended.at<Vec3b>(i, j) = image1.at<Vec3b>(i, j);
				}
				else if (xormask2.at<unsigned char>(i, j) != 0)
					blended.at<Vec3b>(i, j) = image2.at<Vec3b>(i, j);
				else
					blended.at<Vec3b>(i, j) = image1.at<Vec3b>(i, j);
			}

		}
	}

	return blended;
}
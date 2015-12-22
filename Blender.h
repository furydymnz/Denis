#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "MatchTracker.h"
#include <algorithm>
#include <cmath>

using namespace cv;
using namespace std;

class Blender
{
	enum direction { TOPLEFT, TOP, TOPRIGHT, LEFT, CURRENT, RIGHT, BOTTOMLEFT, BOTTOM, YO };
	enum seamDirection {VERTICAL, HORIZONTAL};

	MatchTracker *matchTracker;
	Mat blended;
	Mat andMask;
	vector<vector<int> >blendingOrder;
	vector<vector<int> >seam;

	
	seamDirection calculateSeamDirection(Point2i& pt1, Point2i& pt2);
	void findSeam(seamDirection seamdir);
	void calculateSeamError();

	inline double DIS(double x1, double y1, double x2, double y2)
	{
		return sqrt(((x1)-(x2))*((x1)-(x2)) + ((y1)-(y2))*((y1)-(y2)));
	}
	cv::Mat border(cv::Mat mask);
	void findIntersection(Mat& mask1, Mat& mask2, Mat& intersection);
	int findIntersectionPts(Point2i& pt1, Point2i& pt2, Mat& intersection);
	double getDPError(int i, int j, Mat &errorMap, direction **dirMap);
	void ComputeError(int i, int j, Mat &image1, Mat &image2, direction **dirMap, Mat &errorMap);
	double ComputeError(const cv::Mat& image1, const cv::Mat& image2, int i, int c);
	

public:
	void generateBlendingOrder();
	void printBlendingOrder();
	Blender(MatchTracker *matchTracker);
	~Blender();
	void blend();
};


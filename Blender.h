#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "MatchTracker.h"
#include <algorithm>

using namespace cv;
using namespace std;

class Blender
{
	MatchTracker *matchTracker;
	Mat blended;
	Mat andMask;
	vector<vector<int> >blendingOrder;
	vector<vector<int> >seam;

	
	void calculateSeamDirection();
	void findSeam();
	void calculateSeamError();

	inline double DIS(double x1, double y1, double x2, double y2)
	{
		return sqrt(((x1)-(x2))*((x1)-(x2)) + ((y1)-(y2))*((y1)-(y2)));
	}
	void findIntersection(Mat& mask1, Mat& mask2, Mat& intersection);
	cv::Mat border(cv::Mat mask);

public:
	void generateBlendingOrder();
	void printBlendingOrder();
	Blender(MatchTracker *matchTracker);
	~Blender();
	void blend();
};


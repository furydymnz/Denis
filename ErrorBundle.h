#pragma once
#include "ipoint.h"
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <vector>
using namespace std;
class ErrorBundle
{
	Mat errorMap;
	vector<Point2i> path;
	double pathError;
public:
	Mat& getErrorMap() { return errorMap; }
	vector<Point2i>& getpath() { return path; }
	void setPathError(double pe) { pathError = pe; }
	double getPathError() { return pathError; }
};
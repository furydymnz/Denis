#pragma once
#include "opencv2/opencv_modules.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
using namespace cv;
using namespace std;

class TextDetector
{
	bool textMode;
public:
	TextDetector();
	~TextDetector();
	Mat detect(Mat img);
	void setTextMode(bool mode);
};


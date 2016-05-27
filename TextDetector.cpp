#include "TextDetector.h"


TextDetector::TextDetector()
{
}


TextDetector::~TextDetector()
{
}

Mat TextDetector::detect(Mat img)
{
	Mat textMask = Mat::zeros(img.size(), CV_8UC1);
	Mat small;
	const int HEIGHT_BOUND = img.size().height / 200;
	const int WIDTH_BOUND = img.size().width / 200;
	cvtColor(img, small, CV_BGR2GRAY);

	// morphological gradient
	Mat grad;
	Mat morphKernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	morphologyEx(small, grad, MORPH_GRADIENT, morphKernel);
	// binarize
	Mat bw;
	threshold(grad, bw, 0.0, 255.0, THRESH_BINARY | THRESH_OTSU);
	// connect horizontally oriented regions
	Mat connected;
	morphKernel = getStructuringElement(MORPH_RECT, Size(9, 1));
	morphologyEx(bw, connected, MORPH_CLOSE, morphKernel);
	// find contours
	Mat mask = Mat::zeros(bw.size(), CV_8UC1);
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(connected, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	// filter contours
	for (int idx = 0; idx >= 0; idx = hierarchy[idx][0])
	{
		Rect rect = boundingRect(contours[idx]);
		Mat maskROI(mask, rect);
		maskROI = Scalar(0, 0, 0);
		// fill the contour
		drawContours(mask, contours, idx, Scalar(255, 255, 255), CV_FILLED);
		// ratio of non-zero pixels in the filled region
		double r = (double)countNonZero(maskROI) / (rect.width*rect.height);

		if (r > .5 /* assume at least 45% of the area is filled if it contains text */
			&&
			(rect.height > HEIGHT_BOUND && rect.width > WIDTH_BOUND) /* constraints on region size */
												/* these two conditions alone are not very robust. better to use something
												like the number of significant peaks in a horizontal projection as a third condition */
			)
		{
			rectangle(textMask, rect, Scalar(255), CV_FILLED);
		}
	}

	return textMask;
}

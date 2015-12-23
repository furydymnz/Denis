#ifndef BASEIMAGE_H
#define BASEIMAGE_H

#include <opencv2/opencv.hpp>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "ipoint.h"
class BaseImage {
	Mat image;
	Mat mask;
	Mat homography;
	int empty;
public:
	int isEmpty() { return empty; }
	void setEmpty(int i = 1) { empty = i; }
	int maxX, maxY, minX, minY;

	void assignImage(Mat im) { image = im.clone();}
	void assignHomography(Mat h){ homography = h.clone(); }
	void assignMask(Mat &m) { mask = m.clone(); }

	Mat getHomography(){ return homography; }
	Mat getMask() { return mask; }
	Mat getImage(){ return image; }
	Size getSize() { return image.size(); }

	void findBoundary();
	
	BaseImage(IplImage *ipImage)
	{
		image = (Mat(ipImage, true)).clone();
		empty = 0;
	}
};

#endif

#ifndef BASEIMAGE_H
#define BASEIMAGE_H

#include "opencv2/core/core.hpp"
#include "ipoint.h"

class BaseImage {
public:
	Mat *image;
	Mat homography;
	Mat mask;
	int maxX, maxY, minX, minY;
	void assignImage(Mat *im) { image = im; }
	void assignImage(Mat im) { image = new Mat(im); }
	void assignHomography(Mat h){ homography = h; }
	void findBoundary();
	
	BaseImage(IplImage *ipImage)
	{
		image = new Mat(ipImage, true);
	}
	~BaseImage()
	{
		delete image;
	}

};

#endif
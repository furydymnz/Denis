#ifndef BASEIMAGE_H
#define BASEIMAGE_H

#include <opencv2/opencv.hpp>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "ipoint.h"
class BaseImage {
public:
	Mat image;
	Mat mask;
	Mat homography;
	int maxX, maxY, minX, minY;

	//int dX, dY;

	//void assignImage(Mat *im) { image = Mat(*im); }
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
		printf("ss");
		image = (Mat(ipImage, true)).clone();
		printf("%d, %d\n", image.size().height, image.size().width);
		imwrite("sdasd.jpg",image);
	}
	~BaseImage()
	{
		//delete image;
	}

};

#endif
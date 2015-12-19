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
	//void assignImage(Mat *im) { image = Mat(*im); }
	void assignImage(Mat im) { image = im.clone();}
	
	void assignHomography(Mat h){ homography = h.clone(); }
	Mat getHomography(){ return homography; }
	Mat getImage(){ return image; }
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
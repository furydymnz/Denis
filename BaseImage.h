#ifndef BASEIMAGE_H
#define BASEIMAGE_H

#include <opencv2/opencv.hpp>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "ipoint.h"
#include "Homography.h"
class BaseImage {
public:
	Mat image;
	Mat *mask;
	Homography homography;
	int maxX, maxY, minX, minY;
	//void assignImage(Mat *im) { image = Mat(*im); }
	void assignImage(Mat im) { printf("bvb"); 
	image = im.clone(); 
	
	}
	void assignHomography(Mat h){ homography.assign(h); }
	void assignHomography(Homography h){ }
	Homography getHomography(){ return homography; }
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
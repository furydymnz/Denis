#include "BaseImage.h"
#include <iostream>
using namespace std;

void BaseImage::findBoundary()
{
	const int cornerNum = 4;
	std::vector<Point2f> corner(4);
	corner[0] = Point2f(0, 0); corner[1] = Point2f(image->size().width, 0);
	corner[2] = Point2f(image->size().width, image->size().height); corner[3] = Point2f(0, image->size().height);

	perspectiveTransform(corner, corner, homography);

	maxX = minX = corner[0].x;
	maxY = minY = corner[0].y;

	for (int i = 0; i < cornerNum; i++){
		if (corner[i].x > maxX) 
			maxX = corner[i].x;
		if (corner[i].x < minX)
			minX = corner[i].x;
		if (corner[i].y > maxY) 
			maxY = corner[i].y;
		if (corner[i].y < minY)
			minY = corner[i].y;
	}
	
	cout << "x:" << minX << " " << maxX << endl << " y:" << minY << " " << maxY << endl;
}
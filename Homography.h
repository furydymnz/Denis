#ifndef HOMOGRAPHY_H
#define HOMOGRAPHY_H

#include "ipoint.h"

class Homography{
	double homo[3][3];
	int empty;
public:
	Homography(){ empty = 1; }
	void assign(Homography &h)
	{
		for (int m = 0; m < 3; m++)
		{
			for (int p = 0; p < 3; p++)
				homo[m][p] = h.homo[m][p];
		}
		empty = 0;
	}
	void assign(Mat &h)
	{
		for (int m = 0; m < 3; m++)
		{
			for (int p = 0; p < 3; p++)
				homo[m][p] = h.at<double>(m, p);
		}
		empty = 0;
	}
	double& at(int i, int r){ return homo[i][r]; }
	Homography eye()
	{
		Homography h = Homography();
		for (int m = 0; m < 3; m++)
		for (int p = 0; p < 3; p++)
			homo[m][p] = m == p ? 1 : 0;

		return h;
	}
	int isEmpty() { return empty; }
	int setEmpty(int e){ empty = e; }
	Mat toMat()
	{
		Mat mat = Mat(3, 3, CV_64F);
		for (int m = 0; m < 3; m++)
		for (int p = 0; p < 3; p++)
			mat.row(m).col(p) = homo[m][p];
		return mat;
	}
};

#endif
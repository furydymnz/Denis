/*********************************************************** 
*  --- OpenSURF ---                                       *
*  This library is distributed under the GNU GPL. Please   *
*  use the contact form at http://www.chrisevansdev.com    *
*  for more information.                                   *
*                                                          *
*  C. Evans, Research Into Robust Visual Features,         *
*  MSc University of Bristol, 2008.                        *
*                                                          *
************************************************************/

#ifndef IPOINT_H
#define IPOINT_H

#include <vector>
#include <math.h>
#include "opencv2/core/core.hpp"

using namespace cv;
//-------------------------------------------------------

class Ipoint; // Pre-declaration
typedef std::vector<Ipoint> IpVec;
typedef std::vector<std::pair<Ipoint, Ipoint> > IpPairVec;

//-------------------------------------------------------

//! Ipoint operations
void getMatches(IpVec &ipts1, IpVec &ipts2, IpPairVec &matches);
int translateCorners(IpPairVec &matches, const CvPoint src_corners[4], CvPoint dst_corners[4]);

//-------------------------------------------------------

class Ipoint {

public:

	//! Destructor
	~Ipoint() {};

	//! Constructor
	Ipoint() : orientation(0) {};

	Ipoint(int x, int y) {
		this->x = x;
		this->y = y;
		orientation = 0;
	};

	//! Gets the distance in descriptor space between Ipoints
	float operator-(const Ipoint &rhs)
	{
		float sum = 0.f;
		for (int i = 0; i < 64; ++i)
			sum += (this->descriptor[i] - rhs.descriptor[i])*(this->descriptor[i] - rhs.descriptor[i]);
		return sqrt(sum);
	};

	//! Coordinates of the detected interest point
	float x, y;

	//! Detected scale
	float scale;

	//! Orientation measured anti-clockwise from +ve x-axis
	float orientation;

	//! Sign of laplacian for fast matching purposes
	int laplacian;

	//! Vector of descriptor components
	float descriptor[64];

	//! Placeholds for point motion (can be used for frame to frame motion analysis)
	float dx, dy;

	//! Used to store cluster index
	int clusterIndex;

	void transform(Mat h)
	{
		x = h.at<double>(0, 0)*x + h.at<double>(0, 1)*x + h.at<double>(0, 2)*x;
		y = h.at<double>(1, 0)*y + h.at<double>(1, 1)*y + h.at<double>(1, 2)*y;
	}
};


//-------------------------------------------------------


#endif

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

#ifndef UTILS_H
#define UTILS_H

#include <cv.h>
#include "ipoint.h"
#include "ErrorBundle.h"
#include "TextDetector.h"
#include "BaseImage.h"
#include <vector>
using namespace cv;

const int TRANSVERSE = 1;
const int LONGITUDINAL = 2;
enum direction{TOPLEFT, TOP, TOPRIGHT, LEFT, CURRENT, RIGHT, BOTTOMLEFT, BOTTOM, YO};

// Convert image to single channel 32F
IplImage* getGray(const IplImage *img);

//! Draw a Point at feature location
void drawPoint(IplImage *img, Ipoint &ipt);

//! Draw a Point at all features
void drawPoints(IplImage *img, std::vector<Ipoint> &ipts);
void drawPoints(Mat &image, vector<Ipoint> &ipts);

//! Round float to nearest integer
inline int fRound(float flt)
{
  return (int) floor(flt+0.5f);
}

//! Find the maximum scale of transformed picture
void findmaxima(int& maxX, int& maxY, int& minX, int& minY, Mat& image, Mat& warpMat);


//!used for RANSAC
void uniqueRandom(int **value, int pointAmount, int randomMax);

//!RANSAC
Mat findhomography(IpPairVec& matches, int reverse);

inline double DIS(double x1, double y1, double x2, double y2);

void findIntersection(Mat& mask1, Mat& mask2, Mat& intersection);
int findIntersectionPts(Point2i& pt1, Point2i& pt2, Mat& intersection, Mat& andMasks);
ErrorBundle horizontalErrorMap(BaseImage *image1, BaseImage *image2, Mat mask1, Mat mask2, Mat textMask1, Mat textMask2, double scale);
ErrorBundle verticalErrorMap(BaseImage *image1, BaseImage *image2, Mat mask1, Mat mask2, Mat textMask1, Mat textMask2, double scale);
void fixSeam(vector<Point2i> &seam, Point pt1, Point pt2, double scale, Mat andMask);
void verticalBlending(Mat& blended, BaseImage *image1, BaseImage *image2, Mat& mask1, Mat& mask2, vector<Point2i>& seam);
void horizontalBlending(Mat& blended, BaseImage *image1, BaseImage *image2, Mat& mask1, Mat& mask2, vector<Point2i>& seam);
#endif

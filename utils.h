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

//! Round float to nearest integer
inline int fRound(float flt)
{
  return (int) floor(flt+0.5f);
}

//! Find the maximum scale of transformed picture
void findmaxima(int& maxX, int& maxY, int& minX, int& minY, Mat& image, Mat& warpMat);

//! FUCK
void blendImage(int type, Mat &image1, Mat &image2, Mat &imageResult, Mat &t_mask1, Mat &t_mask2, int dX, int dY);

//!used for RANSAC
void uniqueRandom(int **value, int pointAmount, int randomMax);

//!RANSAC
Mat findhomography(IpPairVec& matches, int reverse);

inline double DIS(double x1, double y1, double x2, double y2);

void findIntersection(Mat& mask1, Mat& mask2, Mat& intersection);
int findIntersectionPts(Point2i& pt1, Point2i& pt2, Mat& intersection, Mat& andMasks);
Mat verticalBlending(cv::Mat image1, cv::Mat image2, Mat mask1, Mat mask2);
Mat horizontalBlending(cv::Mat image1, cv::Mat image2, Mat mask1, Mat mask2, int dY);
ErrorBundle horizontalErrorMap(cv::Mat image1, cv::Mat image2, Mat mask1, Mat mask2, int imageCodeX = 0, int imageCodeY = 0);
ErrorBundle verticalErrorMap(cv::Mat image1, cv::Mat image2, Mat mask1, Mat mask2, int imageCodeX = 0, int imageCodeY = 0);
#endif

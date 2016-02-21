#ifndef MATCHTRACKER_H
#define MATCHTRACKER_H
#include <vector>
#include "opencv2/core/core.hpp"
#include "BaseImage.h"

using namespace std;

class Route{
public:
	vector< vector<int> > route;
	vector< int > routeWeight;
	vector< int > routeWeightAvg;
};

class MatchTracker {
public:
	int pivotIndex;
	vector < BaseImage *> images;
	vector < Route > routes;
	vector < vector <int> > pairConnection;
	vector < vector <int> > pairNum;
	vector < vector <IpPairVec> > pairFP;
	vector < vector <Mat> > pairHomography;
	vector < vector <double> > pairError;
	vector < vector<int> > blendingOrder;
	vector < vector<pair<Point2i, Point2i> > >pairIntersection;
	vector < vector < vector<Point2i> > >pairSeam;

	int maxX, maxY, minX, minY;
	int size;
	Size imageSize;
	MatchTracker(int size);

	void assignFPNum(int i, int r, int fp) { pairNum[i][r] = fp; pairNum[r][i] = fp; }
	void assignErrorPair(int i, int r, double err) { pairError[i][r] = err; pairError[r][i] = err; }
	void assignFPPair(int i, int r, IpPairVec fp) { pairFP[i][r] = fp; }
	void assignIntersectionPair(int i, int r, pair<Point2i, Point2i>& pts) { pairIntersection[i][r] = pts; pairIntersection[r][i] = pts; }
	void assignSeamPair(int i, int r, vector<Point2i>& seam) { pairSeam[i][r] = seam; pairSeam[r][i] = seam; }

	void pushImage(BaseImage *i) { images.push_back(i);  }

	vector<Point2i> getPairSeam(int i, int r) { return pairSeam[i][r]; }
	pair<Point2i, Point2i> getPairInteresection(int i, int r) { return pairIntersection[i][r]; }
	vector<int>& getPairNum(int i) { return pairNum[i]; }
	int getPairNum(int i, int r){ return pairNum[i][r]; }
	double getPairError(int i, int r) { return pairError[i][r]; }
	int getPairConnection(int i, int r) { return pairConnection[i][r]; };
	BaseImage* getImage(int i){ return (images[i]); }
	Mat getHomographyPair(int i, int r){ return (pairHomography[i][r]); }
	void assignHomographyPair(int i, int r, Mat h){pairHomography[i][r] = h.clone();}
	void calculatePairConnection();

	void assignHomographyToImage();
	void calculateBoundary();
	void fixHomography();
	void applyHomography();

	void generateMask();
	void printHomography();
	void calculateErrorPair();
	void calculateErrorSeamTest();
	void calculateTranslation();
	void pixelPadding();

	Mat blending();

	vector<IpPairVec>& getPairFP(int i) { return pairFP[i]; }
	IpPairVec& getPairFP(int i, int r, int & reverse);
	void assignRoute(int image, vector<int > r){ routes[image].route.push_back(r); }
	Route& getRoute(int i){ return routes[i]; }
	int getSize() { return size; }
	vector < vector<int> >& getBlendingOrder(){ return blendingOrder; }

};

#endif

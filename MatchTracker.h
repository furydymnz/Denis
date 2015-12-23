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
	static const int pivotIndex = 0;
	vector < BaseImage *> images;
	vector < Route > routes;
	vector < vector <int> > pairNum;
	vector < vector <IpPairVec> > pairFP;
	vector < vector <Mat> > pairHomography;
	vector < vector <double> > pairError;
	int maxX, maxY, minX, minY;
	int size;
	MatchTracker(int size);
	MatchTracker(const MatchTracker& m);
	MatchTracker& operator=(const MatchTracker& m);

	void assignFPNum(int i, int r, int fp) { pairNum[i][r] = fp; pairNum[r][i] = fp; }
	void assignErrorPair(int i, int r, double err) { pairError[i][r] = err; pairError[r][i] = err; }
	void assignFPPair(int i, int r, IpPairVec fp) { pairFP[i][r] = fp; }

	void pushImage(BaseImage *i) { images.push_back(i);  }

	vector<int>& getPairNum(int i) { return pairNum[i]; }
	int getPairNum(int i, int r){ return pairNum[i][r]; }
	double getPairError(int i, int r) { return pairError[i][r]; }
	BaseImage* getImage(int i){ return (images[i]); }
	Mat getHomographyPair(int i, int r){ return (pairHomography[i][r]); }
	void assignHomographyPair(int i, int r, Mat h){pairHomography[i][r] = h.clone();}

	void assignHomographyToImage();
	void calculateBoundary();
	void fixHomography();
	void applyHomographyTest();

	void generateMask();
	void printHomography();
	void calculateErrorPair();
	void calculateErrorSeamTest();


	void calculateTranslation();



	vector<IpPairVec>& getPairFP(int i) { return pairFP[i]; }
	IpPairVec& getPairFP(int i, int r, int & reverse);
	void assignRoute(int image, vector<int > r){ routes[image].route.push_back(r); }
	Route& getRoute(int i){ return routes[i]; }
	int getSize() { return size; }
};

#endif

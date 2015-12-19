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
	vector < BaseImage *> images;
	vector < Route > routes;
	vector < vector <int> > pairNum;
	vector < vector <IpPairVec> > pairFP;
	vector < vector <Mat> > pairHomography;

	MatchTracker(int size)
	{
		pairNum.resize(size);
		pairFP.resize(size);
		routes.resize(size);
		images.resize(size);
		pairHomography.resize(size);
		for (int i = 0; i < size; i++)
		{
			pairNum[i].resize(size);
			pairFP[i].resize(size);
			pairHomography[i].resize(size);
			pairFP[i].clear();
			for (int j = 0; j < size; j++)
			{
				pairHomography[i][j] = Mat(3, 3, CV_64F, Scalar(-1, -1, -1));
			}
		}
	}
	void assignFPNum(int i, int r, int fp) { pairNum[i][r] = fp; pairNum[r][i] = fp; }
	
	
	void assignFPPair(int i, int r, IpPairVec fp) { pairFP[i][r] = fp; }
	void push_back(BaseImage *i) { images.push_back(i); }
	vector<int>& getPairNum(int i) { return pairNum[i]; }
	int getPairNum(int i, int r){ return pairNum[i][r]; }
	Mat getHomographyPair(int i, int r){ return pairHomography[i][r]; }
	void assignHomographyPair(int i, int r, Mat &H){ pairHomography[i][r] = H; }
	vector<IpPairVec>& getPairFP(int i) { return pairFP[i]; }
	IpPairVec& getPairFP(int i, int r)
	{
		if (!pairFP[i][r].empty())
			return pairFP[i][r];
		if (!pairFP[r][i].empty())
			return pairFP[r][i];
		return IpPairVec();
	}
	void assignRoute(int image, vector<int > r){ routes[image].route.push_back(r); }
	Route& getRoute(int i){ return routes[i]; }
	int getSize() { return pairNum.size(); }

};

#endif
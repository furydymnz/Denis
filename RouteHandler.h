#ifndef ROUTEHANDLER_H
#define ROUTEHANDLER_H
#include "ipoint.h"
#include <vector>
#include "MatchTracker.h"

static class RouteHandler{
public:
	static void printRoutes(MatchTracker &matchTracker);
	static void removeRedundantRoutes(MatchTracker &matchTracker);
	static void findConnectingRoute(MatchTracker &matchTracker);
	static void countRoutesWeight(MatchTracker &matchTracker);
	static void calculateHomography(MatchTracker &matchTracker);
	static void findBlendingOrder(MatchTracker &matchTracker);
};



#endif
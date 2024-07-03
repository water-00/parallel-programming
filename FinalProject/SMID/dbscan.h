#ifndef DBSCAN_H
#define DBSCAN_H

#include <vector>

using namespace std;

struct Point {
    double x;
    double y;
    int lable;  // -1 unvisited, 0 noise, >0 cluster index
};
double euclidean_distance(Point a, Point b);
int dbscan(vector<Point>& dataset, double eps, int min_pts);
double euclidean_distance_sse(const Point& a, const Point& b);
double euclidean_distance_avx256(const Point& a, const Point& b);
double euclidean_distance_avx512(const Point& a, const Point& b);
#endif /*DBSCAN_H*/

#include "dbscan.h"
#include <math.h>
#include <queue>
#include <iostream>
#include <immintrin.h>
#include <vector>
#include <cstdint>

using namespace std;


// calculate eculidean distance of two 2-D points
double euclidean_distance(Point a, Point b) {
    double x = a.x - b.x;
    double y = a.y - b.y;
    return sqrt(x * x + y * y);
}

// get neighborhood of point p and add it to neighborhood queue
int region_query(vector<Point>& dataset, int p, queue<int>& neighborhood, double eps) {
    //int count = 0;
    for (int i = 0; i < dataset.size(); i++) {
        //cout << "regin_query" << count++ << endl;
        if (i != p) {
            int dist = euclidean_distance(dataset[p], dataset[i]);
            if (dist < eps) {
                neighborhood.push(i);
            }
        }
    }
    return (int)neighborhood.size();
}

// expand cluster formed by p, which works in a way of bfs.
bool expand_cluster(vector<Point>& dataset, int p, int c, double eps, int min_pts) {
    queue<int> neighbor_pts;
    dataset[p].lable = c;

    region_query(dataset, p, neighbor_pts, eps);

    while (!neighbor_pts.empty()) {

        int neighbor = neighbor_pts.front();
        queue<int> neighbor_pts1;
        region_query(dataset, neighbor, neighbor_pts1, eps);

        if (neighbor_pts1.size() >= min_pts - 1) {
            while (!neighbor_pts1.empty()) {

                int pt = neighbor_pts1.front();
                if (dataset[pt].lable == -1) {
                    neighbor_pts.push(pt);
                }
                neighbor_pts1.pop();
            }
        }
        dataset[neighbor].lable = c;
        neighbor_pts.pop();

    }
    return  true;
}

// doing dbscan, given radius and minimum number of neigborhoods.
int dbscan(vector<Point>& dataset, double eps, int min_pts) {
    int c = 0;  // cluster lable
    //int count = 0;
    for (int p = 0; p < dataset.size(); p++) {
        queue<int> neighborhood;
        //cout << count++ << endl;
        region_query(dataset, p, neighborhood, eps);

        if (neighborhood.size() + 1 < min_pts) {
            // mark as noise
            dataset[p].lable = 0;
        }
        else {

            if (dataset[p].lable == -1) {
                c++;
                expand_cluster(dataset, p, c, eps, min_pts);
            }
        }
    }
    return c;

}

// SSE2
double euclidean_distance_sse2(const Point& a, const Point& b) {
    __m128d va = _mm_set_pd(a.y, a.x);
    __m128d vb = _mm_set_pd(b.y, b.x);
    __m128d diff = _mm_sub_pd(va, vb);
    __m128d sq = _mm_mul_pd(diff, diff);
    __m128d sum = _mm_hadd_pd(sq, sq);
    return _mm_cvtsd_f64(sum);
}

// AVX256
double euclidean_distance_avx256(const Point& a, const Point& b) {
    __m256d va = _mm256_set_pd(0, 0, a.y, a.x);
    __m256d vb = _mm256_set_pd(0, 0, b.y, b.x);
    __m256d diff = _mm256_sub_pd(va, vb);
    __m256d sq = _mm256_mul_pd(diff, diff);
    __m256d sum = _mm256_hadd_pd(sq, sq);
    __m128d low = _mm256_extractf128_pd(sum, 0);
    return _mm_cvtsd_f64(low);
}

// AVX512 
double euclidean_distance_avx512(const Point& a, const Point& b) {
    __m512d va = _mm512_set_pd(0, 0, 0, 0, 0, 0, a.y, a.x);
    __m512d vb = _mm512_set_pd(0, 0, 0, 0, 0, 0, b.y, b.x);
    __m512d diff = _mm512_sub_pd(va, vb);
    __m512d sq = _mm512_mul_pd(diff, diff);
    __m256d sum = _mm512_extractf64x4_pd(sq, 0);
    sum = _mm256_hadd_pd(sum, sum);
    __m128d low = _mm256_extractf128_pd(sum, 0);
    return _mm_cvtsd_f64(low);
}
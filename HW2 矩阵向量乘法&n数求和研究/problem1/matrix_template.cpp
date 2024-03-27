#include <iostream>
#include <chrono>
#include <cstdlib>

#ifndef N
#define N 5000
#endif

int mat[N][N];
int vec[N];
int product[N] = { 0 };

// 无缓存优化版本的递归展开
template<int I, int J, int END>
struct UnrollLoopNoCache {
    static void execute(int* product) {
        product[I] += mat[J][I] * vec[J];
        UnrollLoopNoCache<I, J + 1, END>::execute(product);
    }
};

template<int I, int END>
struct UnrollLoopNoCache<I, END, END> {
    static void execute(int* product) {}
};

template<int I = 0>
void computeNoCache() {
    int product[N] = { 0 };
    UnrollLoopNoCache<I, 0, N>::execute(product);
    if constexpr ((I + 1) < N) {
        computeNoCache<I + 1>();
    }
}

// 使用缓存的递归展开
template<int I, int J, int END>
struct UnrollLoopCache {
    static void execute(int* product) {
        product[J] += mat[I][J] * vec[I];
        UnrollLoopCache<I, J + 1, END>::execute(product);
    }
};

template<int I, int END>
struct UnrollLoopCache<I, END, END> {
    static void execute(int* product) {}
};

template<int I = 0>
void computeCache() {
    static int product[N] = { 0 };
    UnrollLoopCache<I, 0, N>::execute(product);
    if constexpr ((I + 1) < N) {
        computeCache<I + 1>();
    }
}

int main() {
    // 初始化mat和vec
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            mat[i][j] = std::rand() % 100;
        }
        vec[i] = std::rand() % 100;
    }

    // 无缓存优化版本计时
    auto startNoCache = std::chrono::high_resolution_clock::now();
    computeNoCache<0>();
    auto endNoCache = std::chrono::high_resolution_clock::now();
    auto elapsedNoCache = std::chrono::duration_cast<std::chrono::microseconds>(endNoCache - startNoCache).count();
    std::cout << "Time taken for no-cache computation: " << elapsedNoCache << " microseconds\n";

    // 使用缓存的版本计时
    auto startCache = std::chrono::high_resolution_clock::now();
    computeCache<0>();
    auto endCache = std::chrono::high_resolution_clock::now();
    auto elapsedCache = std::chrono::duration_cast<std::chrono::microseconds>(endCache - startCache).count();
    std::cout << "Time taken for cache-friendly computation: " << elapsedCache << " microseconds\n";

    return 0;
}
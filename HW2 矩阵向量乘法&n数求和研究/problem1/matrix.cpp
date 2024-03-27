#include <iostream>
#include <chrono>
#include <vector>
#include <cstdlib>

using namespace std;

#ifndef N
#define N 5000
#endif
int mat[N][N];
int vec[N];

void print(int* product) {
    for (int i = 0; i < N; i++) {
        cout << product[i] << " ";
    }
    cout << endl;
}

void no_cache() {
    // multiply, access the matrix by column: one outer loop get one inner-product
    int product[N] = { 0 };
    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            product[i] += mat[j][i] * vec[j];
        }
    }
    auto end = chrono::high_resolution_clock::now();
    auto elapsed_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
    cout << "Time taken for no-cache computation: " << elapsed_time << " microseconds\n";
    // print(product);
}

void cache() {
    // multiply, access the matrix by row to use cache
    int product[N] = { 0 };
    auto start = chrono::high_resolution_clock::now();
    for (int j = 0; j < N; j++) {
        for (int i = 0; i < N; i++) {
            product[i] += mat[j][i] * vec[j];
        }
    }
    auto end = chrono::high_resolution_clock::now();
    auto elapsed_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
    cout << "Time taken for cache-friendly computation: " << elapsed_time << " microseconds\n";
    // print(product);
}

int main(int argc, char* argv[]) {
    // generate random matrix
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            mat[i][j] = rand() % 100;
        }
    }

    // generate random vector
    for (int i = 0; i < N; i++) {
        vec[i] = rand() % 100;
    }

    no_cache();
    cache();

    
    return 0;
}
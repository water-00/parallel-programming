#include <iostream>
#include <chrono>
#include <cstdlib>
#include <cmath>

using namespace std;

#ifndef N
#define N 20 
#endif

void print(int sum) {
    cout << sum << "\n";
}

void normal(int* vec, int n) {
    int sum = 0;
    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++) {
        sum += vec[i];
    }
    auto end = chrono::high_resolution_clock::now();
    auto elapsed_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
    cout << "Time taken for normal computation: " << elapsed_time << " microseconds\n";
    // print(sum);
}

void recursion(int* vec, int n) {
    auto start = chrono::high_resolution_clock::now();
    int stepSize = 1; // 开始时每组只有一个元素
    while (stepSize < n) {
        for (int i = 0; i < n; i += 2 * stepSize) {
            if (i + stepSize < n) { // 确保不会越界
                vec[i] += vec[i + stepSize];
            }
        }
        stepSize *= 2; // 每次迭代后，合并的组大小加倍
    }
    auto end = chrono::high_resolution_clock::now();
    auto elapsed_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
    cout << "Time taken for recursion computation: " << elapsed_time << " microseconds\n";
    // print(vec[0]);
}

void unroll_2(int* vec, int n) {
    int sum = 0;
    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i += 2) {
        sum += vec[i] + vec[i + 1];
    }
    auto end = chrono::high_resolution_clock::now();
    auto elapsed_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
    cout << "Time taken for unroll_2 computation: " << elapsed_time << " microseconds" << endl;
    // print(sum);
}


void unroll_4(int* vec, int n) {
    int sum = 0;
    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i += 4) {
        sum += vec[i] + vec[i + 1] + vec[i + 2] + vec[i + 3];
    }
    auto end = chrono::high_resolution_clock::now();
    auto elapsed_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
    cout << "Time taken for unroll_4 computation: " << elapsed_time << " microseconds\n";
    // print(sum);
}

void unroll_8(int* vec, int n) {
    int sum = 0;
    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i += 8) {
        sum += vec[i] + vec[i + 1] + vec[i + 2] + vec[i + 3] + vec[i + 4] + vec[i + 5] + vec[i + 6] + vec[i + 7];
    }
    auto end = chrono::high_resolution_clock::now();
    auto elapsed_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
    cout << "Time taken for unroll_8 computation: " << elapsed_time << " microseconds\n";
    // print(sum);
}

void unroll_16(int* vec, int n) {
    int sum = 0;
    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i += 16) {
        sum += vec[i] + vec[i + 1] + vec[i + 2] + vec[i + 3] + vec[i + 4] + vec[i + 5] + vec[i + 6] + vec[i + 7] +
            vec[i + 8] + vec[i + 9] + vec[i + 10] + vec[i + 11] + vec[i + 12] + vec[i + 13] + vec[i + 14] + vec[i + 15];
    }
    auto end = chrono::high_resolution_clock::now();
    auto elapsed_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
    cout << "Time taken for unroll_16 computation: " << elapsed_time << " microseconds\n";
    // print(sum); 
}

void unroll_32(int* vec, int n) {
    int sum = 0;
    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i += 32) {
        sum += vec[i] + vec[i + 1] + vec[i + 2] + vec[i + 3] + vec[i + 4] + vec[i + 5] + vec[i + 6] + vec[i + 7] +
            vec[i + 8] + vec[i + 9] + vec[i + 10] + vec[i + 11] + vec[i + 12] + vec[i + 13] + vec[i + 14] + vec[i + 15] +
            vec[i + 16] + vec[i + 17] + vec[i + 18] + vec[i + 19] + vec[i + 20] + vec[i + 21] + vec[i + 22] + vec[i + 23] +
            vec[i + 24] + vec[i + 25] + vec[i + 26] + vec[i + 27] + vec[i + 28] + vec[i + 29] + vec[i + 30] + vec[i + 31];
    }
    auto end = chrono::high_resolution_clock::now();
    auto elapsed_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
    cout << "Time taken for unroll_32 computation: " << elapsed_time << " microseconds\n";
    // print(sum); 
}

int main(int argc, char* argv[]) {
    int size = pow(2, N);
    int* vec = new int[size]; // 动态分配内存

    for (int i = 0; i < size; i++) {
        vec[i] = rand() % 100;
    }

    normal(vec, size);
    unroll_2(vec, size);
    unroll_4(vec, size);
    unroll_8(vec, size);
    unroll_16(vec, size);
    unroll_32(vec, size);

    recursion(vec, size);

    delete[] vec; // 释放动态分配的内存

    return 0;
}
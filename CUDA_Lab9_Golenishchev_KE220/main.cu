#include <cuda_runtime.h>
#include <iostream>
#include <chrono>

// Оригинальная функция для вычисления числа π на CPU
double CalcPi(const int n) {
    double pi = 0;
    const double coef = 1.0 / n;

    for (int i = 0; i < n; ++i) {
        const double xi = (i + 0.5) * coef;
        pi += 4.0 / (1.0 + xi * xi);
    }
    return pi * coef;
}

// CUDA ядро для параллельного вычисления суммы
__global__ void calcPiKernel(const int n, double* partialSums) {
    extern __shared__ double sharedSums[]; // Shared memory для частичных сумм

    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int tid = threadIdx.x;

    const double coef = 1.0 / n;
    sharedSums[tid] = 0.0;

    if (idx < n) {
        const double xi = (idx + 0.5) * coef;
        sharedSums[tid] = 4.0 / (1.0 + xi * xi);
    }

    __syncthreads();

    // Редукция внутри блока
    for (int stride = blockDim.x / 2; stride > 0; stride /= 2) {
        if (tid < stride) {
            sharedSums[tid] += sharedSums[tid + stride];
        }
        __syncthreads();
    }

    // Запись результата блока в глобальную память
    if (tid == 0) {
        partialSums[blockIdx.x] = sharedSums[0];
    }
}

// Функция для вычисления числа π на GPU
double CalcPiGPU(const int n) {
    const int blockSize = 256; // Количество потоков в блоке
    const int numBlocks = (n + blockSize - 1) / blockSize; // Количество блоков

    // Выделение памяти на устройстве
    double* d_partialSums;
    cudaMalloc(&d_partialSums, numBlocks * sizeof(double));

    // Запуск CUDA ядра
    calcPiKernel<<<numBlocks, blockSize, blockSize * sizeof(double)>>>(n, d_partialSums);

    // Копирование частичных сумм на хост
    double* h_partialSums = new double[numBlocks];
    cudaMemcpy(h_partialSums, d_partialSums, numBlocks * sizeof(double), cudaMemcpyDeviceToHost);

    // Редукция на хосте
    double pi = 0.0;
    for (int i = 0; i < numBlocks; ++i) {
        pi += h_partialSums[i];
    }

    // Освобождение памяти
    delete[] h_partialSums;
    cudaFree(d_partialSums);

    return pi * (1.0 / n);
}

int main() {
    const int n = 100000000; // Количество интервалов для вычисления π

    // CPU вычисление
    auto startCPU = std::chrono::high_resolution_clock::now();
    double piCPU = CalcPi(n);
    auto endCPU = std::chrono::high_resolution_clock::now();
    double cpuTime = std::chrono::duration<double>(endCPU - startCPU).count();

    std::cout << "Computed pi (CPU): " << piCPU << ", Time: " << cpuTime << " seconds\n";

    // GPU вычисление
    float gpuTime = 0;
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    cudaEventRecord(start, 0);
    double piGPU = CalcPiGPU(n);
    cudaEventRecord(stop, 0);
    cudaEventSynchronize(stop);

    cudaEventElapsedTime(&gpuTime, start, stop);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);

    std::cout << "Computed pi (GPU): " << piGPU << ", Time: " << gpuTime / 1000.0 << " seconds\n";

    return 0;
}

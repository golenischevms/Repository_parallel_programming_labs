#include <stdio.h>
#include <cuda_runtime.h>

// CUDA kernel для сложения векторов
__global__ void vectorAddGPU(const float *a, const float *b, float *c, size_t N) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < N) {
        c[idx] = a[idx] + b[idx];
    }
}

int main() {
    size_t N = 1 << 20; // Размер вектора (например, 1 миллион элементов)
    size_t bytes = N * sizeof(float);

    // Выделение памяти на хосте
    float *h_a = (float *)malloc(bytes);
    float *h_b = (float *)malloc(bytes);
    float *h_c = (float *)malloc(bytes);

    // Инициализация данных
    for (size_t i = 0; i < N; ++i) {
        h_a[i] = 1.0f;  // Первый вектор
        h_b[i] = 2.0f;  // Второй вектор
    }

    // Выделение памяти на устройстве
    float *d_a, *d_b, *d_c;
    cudaMalloc(&d_a, bytes);
    cudaMalloc(&d_b, bytes);
    cudaMalloc(&d_c, bytes);

    // Копирование данных с хоста на устройство
    cudaMemcpy(d_a, h_a, bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(d_b, h_b, bytes, cudaMemcpyHostToDevice);

    // Настройка сетки и блоков
    int threads = 256;                       // Количество нитей в блоке
    int blocks = (N + threads - 1) / threads; // Количество блоков

    // Измерение времени выполнения на GPU
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    cudaEventRecord(start);
    vectorAddGPU<<<blocks, threads>>>(d_a, d_b, d_c, N);
    cudaEventRecord(stop);

    // Ожидание завершения всех вычислений на GPU
    cudaDeviceSynchronize();

    // Копирование результата обратно на хост
    cudaMemcpy(h_c, d_c, bytes, cudaMemcpyDeviceToHost);

    cudaEventSynchronize(stop);
    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, stop);

    printf("Размер вектора: %zu\n", N);
    printf("Время выполнения на GPU: %f ms\n", milliseconds);

    // Освобождение памяти
    free(h_a);
    free(h_b);
    free(h_c);
    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_c);

    return 0;
}


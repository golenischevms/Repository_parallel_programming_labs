#include <cstdio>
#include <omp.h>

// Golenishchev Artem, KE-220 Task 8
int main() {
    int n;
    // Ввод размера матриц
    printf("Enter the size of the matrices (n x n): ");
    scanf("%d", &n);
    if (n < 1 || n > 10) {
        printf("Invalid size. n must be between 1 and 10.\n");
        return 1;
    }

    double A[10][10], B[10][10], C[10][10] = {{0}};

    // Ввод элементов матрицы A
    printf("Enter the elements of matrix A:\n");
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            scanf("%lf", &A[i][j]);

    // Ввод элементов матрицы B
    printf("Enter the elements of matrix B:\n");
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            scanf("%lf", &B[i][j]);

// Параллельное вычисление произведения матриц
#pragma omp parallel for collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    // Вывод результата
    printf("Resultant matrix C:\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%.2lf ", C[i][j]);
        }
        printf("\n");
    }

    return 0;
}

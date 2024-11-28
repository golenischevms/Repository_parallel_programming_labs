#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h> // Для измерения памяти

// Golenishchev Artem, KE-220 Task 19 (1)
int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size, n, *A = NULL, *B = NULL, *C = NULL;
    double start_time, end_time;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Замер начала выполнения
    double total_start_time = MPI_Wtime();

    if (rank == 0) {
        printf("Enter matrix size: ");
        scanf("%d", &n);
        if (n % size != 0) {
            printf("Matrix size must be divisible by the number of processes.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        A = (int*)malloc(n * n * sizeof(int));
        B = (int*)malloc(n * n * sizeof(int));
        printf("Elements of A:\n");
        for (int i = 0; i < n * n; i++) scanf("%d", &A[i]);
        printf("Elements of B:\n");
        for (int i = 0; i < n * n; i++) scanf("%d", &B[i]);
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0) B = (int*)malloc(n * n * sizeof(int));

    MPI_Bcast(B, n * n, MPI_INT, 0, MPI_COMM_WORLD);

    int rows_proc = n / size;
    int* local_A = (int*)malloc(rows_proc * n * sizeof(int));
    int* local_C = (int*)calloc(rows_proc * n, sizeof(int));

    MPI_Scatter(A, rows_proc * n, MPI_INT, local_A, rows_proc * n, MPI_INT, 0, MPI_COMM_WORLD);

    // Замер времени вычислений
    start_time = MPI_Wtime();

    // Умножение матриц
    for (int i = 0; i < rows_proc; i++)
        for (int j = 0; j < n; j++)
            for (int k = 0; k < n; k++)
                local_C[i * n + j] += local_A[i * n + k] * B[k * n + j];

    // Замер времени завершения вычислений
    end_time = MPI_Wtime();

    if (rank == 0) C = (int*)malloc(n * n * sizeof(int));

    MPI_Gather(local_C, rows_proc * n, MPI_INT, C, rows_proc * n, MPI_INT, 0, MPI_COMM_WORLD);

    // Печать результатов на процессе 0
    if (rank == 0) {
        printf("Result matrix C:\n");
        for (int i = 0; i < n * n; i++) {
            printf("%d ", C[i]);
            if ((i + 1) % n == 0) printf("\n");
        }
        printf("Matrix multiplication execution time: %f seconds\n", end_time - start_time);
        free(A);
        free(B);
        free(C);
    } else {
        free(B);
    }

    // Замер использования памяти
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    printf("Process %d: Max memory usage = %ld KB\n", rank, usage.ru_maxrss);

    free(local_A);
    free(local_C);

    MPI_Finalize();
    return 0;
}

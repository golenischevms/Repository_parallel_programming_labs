#include <sys/resource.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

double get_memory_usage_KB() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss; // Memory usage in KB
}

// Golenishchev Artem, KE-220 Task 19 (2)
int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, size, n = 0;
    int *A = NULL, *B = NULL, *C = NULL;
    double start_time, end_time;
    double mem_usage;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        printf("Enter matrix size n: ");
        scanf("%d", &n);
        if (n % size != 0) {
            printf("Matrix size must be divisible by the number of processes.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        A = (int*)malloc(n * n * sizeof(int));
        B = (int*)malloc(n * n * sizeof(int));
        if (!A || !B) {
            printf("Memory allocation failed.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        printf("Enter matrix A:\n");
        for (int i = 0; i < n * n; i++) scanf("%d", &A[i]);
        printf("Enter matrix B:\n");
        for (int i = 0; i < n * n; i++) scanf("%d", &B[i]);
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (rank != 0) {
        B = (int*)malloc(n * n * sizeof(int));
        if (!B) {
            printf("Memory allocation failed.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    MPI_Bcast(B, n * n, MPI_INT, 0, MPI_COMM_WORLD);

    int rows_proc = n / size;
    int* local_A = (int*)malloc(rows_proc * n * sizeof(int));
    int* local_C = (int*)calloc(rows_proc * n, sizeof(int));
    if (!local_A || !local_C) {
        printf("Memory allocation failed.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (rank == 0) start_time = MPI_Wtime();

    if (rank == 0) {
        for (int i = 1; i < size; i++)
            MPI_Send(A + i * rows_proc * n, rows_proc * n, MPI_INT, i, 0, MPI_COMM_WORLD);
        for (int i = 0; i < rows_proc * n; i++)
            local_A[i] = A[i];
    } else {
        MPI_Recv(local_A, rows_proc * n, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    for (int i = 0; i < rows_proc; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                local_C[i * n + j] += local_A[i * n + k] * B[k * n + j];
            }
        }
    }

    if (rank == 0) {
        C = (int*)malloc(n * n * sizeof(int));
        if (!C) {
            printf("Memory allocation failed.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        for (int i = 0; i < rows_proc * n; i++)
            C[i] = local_C[i];
        for (int i = 1; i < size; i++)
            MPI_Recv(C + i * rows_proc * n, rows_proc * n, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        end_time = MPI_Wtime();

        // Print result matrix C
        printf("Result matrix C:\n");
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                printf("%d ", C[i * n + j]);
            }
            printf("\n");
        }
        printf("Execution time: %.6f seconds\n", end_time - start_time);
    } else {
        MPI_Send(local_C, rows_proc * n, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    mem_usage = get_memory_usage_KB();
    printf("Process %d memory usage: %.2f KB\n", rank, mem_usage);

    free(A);
    free(B);
    free(C);
    free(local_A);
    free(local_C);

    MPI_Finalize();
    return 0;
}

#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h> // Для измерения памяти

// Golenishchev Artem, KE-220 Task 17 (1)
#define MAX_LEN 100
#define CHAR_RANGE 128 // ASCII

int main(int argc, char** argv) {
    int rank, size, n = 0, counts[CHAR_RANGE] = {0}, global_counts[CHAR_RANGE] = {0};
    char buf[MAX_LEN] = {0};
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        printf("Enter the string (max length %d): \n", MAX_LEN);
        fgets(buf, MAX_LEN, stdin);
        if ((n = strlen(buf)) > 0 && buf[n - 1] == '\n') buf[--n] = '\0';
        for (int i = 0; i < n; i++) buf[i] = tolower(buf[i]);
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(buf, n, MPI_CHAR, 0, MPI_COMM_WORLD);

    // Начало вычислений
    start_time = MPI_Wtime();

    for (int i = rank; i < CHAR_RANGE; i += size) {
        for (int j = 0; j < n; j++) {
            if (buf[j] == (char)i) counts[i]++;
        }
    }

    MPI_Reduce(counts, global_counts, CHAR_RANGE, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Конец вычислений
    end_time = MPI_Wtime();

    if (rank == 0) {
        for (int i = 0; i < CHAR_RANGE; i++) {
            if (global_counts[i] > 0 && isprint(i)) {
                printf("%c = %d\n", (char)i, global_counts[i]);
            }
        }
    }

    // Замер использования памяти
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    printf("Process %d: Computation time = %f seconds, Max memory usage = %ld KB\n",
           rank, end_time - start_time, usage.ru_maxrss);

    MPI_Finalize();
    return 0;
}

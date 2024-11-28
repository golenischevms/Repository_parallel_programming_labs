#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>

#define MAX_LEN 100
#define CHAR_RANGE 128

// Golenishchev Artem, KE-220 Task 17 (2)
int main(int argc, char** argv) {
    int rank, size, n = 0, counts[CHAR_RANGE] = {0}, global_counts[CHAR_RANGE] = {0};
    char buf[MAX_LEN] = {0};
    double start_time, end_time;
    struct rusage usage;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        printf("Enter the string (max length %d): \n", MAX_LEN);
        fgets(buf, MAX_LEN, stdin);
        if ((n = strlen(buf)) > 0 && buf[n - 1] == '\n') buf[--n] = '\0';
        for (int i = 0; i < n; i++) buf[i] = tolower(buf[i]);
        for (int dest = 1; dest < size; dest++) {
            MPI_Send(&n, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
            MPI_Send(buf, n, MPI_CHAR, dest, 0, MPI_COMM_WORLD);
        }
    } else {
        MPI_Recv(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(buf, n, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Start timing
    start_time = MPI_Wtime();

    for (int i = rank; i < CHAR_RANGE; i += size)
        for (int j = 0; j < n; j++)
            if (buf[j] == (char)i) counts[i]++;

    if (rank == 0) {
        memcpy(global_counts, counts, sizeof(counts));
        for (int src = 1; src < size; src++) {
            MPI_Recv(counts, CHAR_RANGE, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < CHAR_RANGE; i++) global_counts[i] += counts[i];
        }

        printf("\nCharacter frequencies:\n");
        for (int i = 0; i < CHAR_RANGE; i++)
            if (global_counts[i] > 0 && isprint(i))
                printf("%c = %d\n", (char)i, global_counts[i]);
    } else {
        MPI_Send(counts, CHAR_RANGE, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    // End timing
    end_time = MPI_Wtime();

    // Get memory usage
    getrusage(RUSAGE_SELF, &usage);

    // Print computation time and memory usage
    printf("Process %d: Computation time = %f seconds, Max memory usage = %ld KB\n",
           rank, end_time - start_time, usage.ru_maxrss);

    MPI_Finalize();
    return 0;
}

#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        printf("%d processes.\n", size);
    } else if (rank % 2 == 0) {
        printf("I am %d process: SECOND!\n", rank);
    } else {
        printf("I am %d process: FIRST!\n", rank);
    }

    MPI_Finalize();
    return 0;
}

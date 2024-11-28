#include <mpi.h>
#include <stdio.h>
// Golenishchev Artem, KE-220 Task 14
int main(int argc, char** argv) {
    int rank, size, buf;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Получение номера текущего процесса
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Получение общего числа процессов
    if (rank == 0) {
        // Код для master-процесса
        for (int src = 1; src < size; src++) {
            MPI_Recv(&buf, 1, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("receive message '%d' from %d\n", buf, src);
        }
    } else {
        // Код для slave-процессов
        buf = rank; // Номер процесса
        MPI_Send(&buf, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
}

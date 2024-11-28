#include <mpi.h>
#include <stdio.h>
// Golenishchev Artem, KE-220 Task 16
int main(int argc, char** argv) {
    int rank, size, buf, recv_buf;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Получение номера текущего процесса
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Получение общего числа процессов
    buf = rank; // Сообщение, которое отправляет текущий процесс
    // Передача сообщений от текущего процесса всем другим процессам
    for (int i = 0; i < size; i++) {
        if (i != rank) { // Исключаем отправку самому себе
            MPI_Send(&buf, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }
    // Прием сообщений от всех других процессов
    for (int i = 0; i < size; i++) {
        if (i != rank) { // Исключаем прием от самого себя
            MPI_Recv(&recv_buf, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("[%d]: receive message '%d' from %d\n", rank, recv_buf, i);
        }
    }
    MPI_Finalize();
    return 0;
}

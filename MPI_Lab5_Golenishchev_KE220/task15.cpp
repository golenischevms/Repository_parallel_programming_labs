#include <mpi.h>
#include <stdio.h>
// Golenishchev Artem, KE-220 Task 15
int main(int argc, char** argv) {
    int rank, size, buf, recv_buf;
    MPI_Request requests[2];
    MPI_Status statuses[2];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Получение номера текущего процесса
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Получение общего числа процессов
    // Определяем номера соседних процессов в кольце
    int next = (rank + 1) % size; // Следующий процесс
    int prev = (rank - 1 + size) % size; // Предыдущий процесс (учитываем кольцо)
    buf = rank; // Сообщение, отправляемое текущим процессом
    recv_buf = -1; // Инициализация буфера для получения сообщения
    // Неблокирующая отправка сообщения следующему процессу
    MPI_Isend(&buf, 1, MPI_INT, next, 0, MPI_COMM_WORLD, &requests[0]);
    // Неблокирующий прием сообщения от предыдущего процесса
    MPI_Irecv(&recv_buf, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, &requests[1]);
    // Ожидаем завершения всех операций
    MPI_Waitall(2, requests, statuses);
    printf("[%d]: receive message '%d'\n", rank, recv_buf);
    MPI_Finalize();
    return 0;
}

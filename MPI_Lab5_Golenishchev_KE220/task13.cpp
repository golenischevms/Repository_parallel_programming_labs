#include <mpi.h>
#include <stdio.h>
// Golenishchev Artem, KE-220 Task 13
int main(int argc, char** argv) {
    int rank, size, buf;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Получаем номер текущего процесса
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Получаем общее количество процессов

    if (rank == 0) {
        // Процесс 0 начинает передачу
        buf = 0;
        MPI_Send(&buf, 1, MPI_INT, 1, 0, MPI_COMM_WORLD); // Отправляем процессу 1
        MPI_Recv(&buf, 1, MPI_INT, size - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // 	Принимаем от последнего процесса
    } else {
        // Остальные процессы
        MPI_Recv(&buf, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); /	/ Ждём сообщение от предыдущего процесса
                MPI_Send(&(++buf), 1, MPI_INT, (rank + 1) % size, 0, MPI_COMM_WORLD);
        // Увеличиваем и отправляем следующему
    }
    // Вывод сообщения
    printf("[%d]: receive message '%d'\n", rank, buf);
    MPI_Finalize();
    return 0;
}

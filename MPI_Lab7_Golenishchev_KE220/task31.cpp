#include <mpi.h>
#include <omp.h>
#include <cstdio>
// Golenishchev Artem, KE-220 Task 31
int main(int argc, char** argv) {
    int n; // Количество нитей
    if (argc != 2) {
        printf("Usage: %s <number_of_threads>\n", argv[0]);
        return 1;
    }
    n = std::stoi(argv[1]);
    MPI_Init(&argc, &argv);
    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size); // Общее количество процессов
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); // Номер текущего процесса
    // Рассчитаем общее количество гибридных нитей
    int total_hybrid_threads = n * world_size;
// Параллельный блок OpenMP
#pragma omp parallel num_threads(n)
    {
        int thread_num = omp_get_thread_num(); // Номер нити
        printf("I am %d thread from %d process. Number of hybrid threads = %d\n",
               thread_num, world_rank, total_hybrid_threads);
    }
    MPI_Finalize();
    return 0;
}

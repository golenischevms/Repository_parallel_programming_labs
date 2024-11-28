#include <mpi.h>
#include <omp.h>
#include <iostream>
#include <iomanip>
#include <cmath>

int main(int argc, char** argv) {
    // Инициализация MPI
    MPI_Init(&argc, &argv);

    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size); // Количество MPI процессов
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); // Ранг текущего процесса

    // Ввод данных: количество разбиений (N)
    long long N;
    if (world_rank == 0) {
        if (argc != 2) {
            std::cerr << "Usage: " << argv[0] << " <number_of_intervals>" << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        N = std::stoll(argv[1]);
    }

    // Распространение N на все процессы
    MPI_Bcast(&N, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    // Определение диапазона индексов для текущего процесса
    long long chunk_size = N / world_size;
    long long start = world_rank * chunk_size;
    long long end = (world_rank == world_size - 1) ? N : start + chunk_size;

    double step = 1.0 / static_cast<double>(N); // Шаг разбиения
    double local_sum = 0.0;

    // Параллельное вычисление локальной суммы с OpenMP
#pragma omp parallel for reduction(+:local_sum)
    for (long long i = start; i < end; ++i) {
        double x = (i + 0.5) * step;
        local_sum += 4.0 / (1.0 + x * x);
    }

    // Умножаем на шаг, чтобы завершить интегрирование
    local_sum *= step;

    // Сбор данных от всех процессов
    double global_sum = 0.0;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    // Вывод результата на главном процессе
    if (world_rank == 0) {
        std::cout << std::fixed << std::setprecision(8) << global_sum << std::endl;
    }

    // Завершение MPI
    MPI_Finalize();
    return 0;
}

#include <stdio.h>
#include <mpi.h>
#include <sys/resource.h> // Для измерения памяти

// Golenishchev Artem, KE-220 Task 18 (2)
double compute_pi(int start, int end) {
    double sum = 0.0;
    for (int i = start; i < end; i++) {
        sum += 1.0 / (1.0 + ((i + 0.5) / 1000000000.0) * ((i + 0.5) / 1000000000.0));
    }
    return sum;
}

double get_memory_usage_MB() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss / 1024.0; // Convert KB to MB
}

int main(int argc, char** argv) {
    int rank, size;
    int N;
    double local_sum = 0.0, global_sum = 0.0;
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Замер начала времени
    start_time = MPI_Wtime();

    if (rank == 0) {
        printf("Enter N: ");
        scanf("%d", &N);
    }

    // Рассылка значения N всем процессам
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Разбиение работы между процессами
    int chunk_size = N / size;
    int start = rank * chunk_size;
    int end = (rank == size - 1) ? N : start + chunk_size;

    // Вычисление локальной суммы
    local_sum = compute_pi(start, end);

    // Использование точечных коммуникаций для сбора данных
    if (rank == 0) {
        global_sum = local_sum;
        for (int i = 1; i < size; i++) {
            double temp_sum;
            MPI_Recv(&temp_sum, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            global_sum += temp_sum;
        }
        double pi = global_sum * 4.0 / N;
        printf("Computed pi: %.8f\n", pi);
    } else {
        MPI_Send(&local_sum, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    // Замер завершения времени
    end_time = MPI_Wtime();

    // Печать времени вычислений и памяти для каждого процесса
    double mem_usage = get_memory_usage_MB();
    printf("Process %d execution time: %.6f seconds\n", rank, end_time - start_time);
    printf("Process %d memory usage: %.2f MB\n", rank, mem_usage);

    MPI_Finalize();
    return 0;
}

#include <cstdio>
#include <omp.h>

// Golenishchev Artem, KE-220 Task 6
int main() {
    int k, N;

    // Ввод: количество потоков (k) и верхняя граница суммы (N)
    printf("Enter the number of threads (k): ");
    scanf("%d", &k);
    printf("Enter the number of numbers (N): ");
    scanf("%d", &N);

    int total_sum = 0; // Общая сумма

    // Установка количества потоков
    omp_set_num_threads(k);

#pragma omp parallel
    {
        int thread_num = omp_get_thread_num(); // Номер текущего потока
        int local_sum = 0;                    // Локальная сумма для текущего потока

// Параллельное распределение работы между потоками
#pragma omp for reduction(+ : total_sum)
        for (int i = 1; i <= N; ++i) {
            local_sum += i; // Подсчет локальной суммы
        }

        // Вывод частичной суммы для каждого потока
        printf("[%d]: Sum = %d\n", thread_num, local_sum);
    }

    // Вывод общей суммы
    printf("Sum = %d\n", total_sum);
    return 0;
}

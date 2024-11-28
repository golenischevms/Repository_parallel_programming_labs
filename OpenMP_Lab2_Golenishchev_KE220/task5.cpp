#include <stdio.h>
#include <omp.h>
// Golenishchev Artem, KE-220 Task 5
int main() {
    int n, k;
    // Ввод числа N и количества нитей k
    printf("Enter an integer N: ");
    scanf("%d", &n);
    printf("Enter an integer number of threads k: ");
    scanf("%d", &k);
    int sum = 0; // Общая сумма
    // Установка количества нитей
    omp_set_num_threads(k);
    printf("Partial sums:\n");
#pragma omp parallel reduction(+:sum)
    {
        int thread_id = omp_get_thread_num(); // Номер текущей нити
        int total_threads = omp_get_num_threads(); // Общее количество нитей
        // Диапазон чисел для текущей нити
        int start = (n * thread_id) / total_threads + 1;
        int end = (n * (thread_id + 1)) / total_threads;
        int local_sum = 0; // Частичная сумма для текущей нити
        for (int i = start; i <= end; ++i) {
            local_sum += i;
        }
        // Локальная сумма автоматически добавляется в sum через reduction
        sum += local_sum;
        // Вывод частичной суммы для текущей нити
        printf("[%d]: Sum = %d\n", thread_id, local_sum);
    }
    // Вывод общей суммы
    printf("Result: Sum = %d\n", sum);
    return 0;
}

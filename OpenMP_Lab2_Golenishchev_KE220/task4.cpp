#include <stdio.h>
#include <omp.h>
// Golenishchev Artem, KE-220 Task 4
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
        int thread_id = omp_get_thread_num();         // Номер текущей нити
        int total_threads = omp_get_num_threads();    // Общее количество нитей
        // Диапазон чисел для текущей нити
        int start = (n * thread_id) / total_threads + 1;
        int end = (n * (thread_id + 1)) / total_threads;
        int local_sum = 0; // Частичная сумма для текущей нити
        for (int i = start; i <= end; ++i) {
            local_sum += i;
        }
        // Добавление локальной суммы в глобальную с использованием reduction
        sum += local_sum;
        // Вывод частичной суммы для текущей нити
        printf("[%d]: Sum = %d\n", thread_id, local_sum);
    }
    // Вывод общей суммы
    printf("Result: Sum = %d\n", sum);
    return 0;
}


/*ГОНКА*/

#include <stdio.h>
#include <omp.h>
#include <unistd.h> // Для использования usleep() в Ubuntu
// Golenishchev Artem, KE-220 Task 4
int main() {
    int k;
    // Ввод количества нитей
    printf("Enter number of threads (k): ");
    scanf("%d", &k);
    // Установка количества нитей
    omp_set_num_threads(k);
    int rank;
    // Параллельная область с указанием области видимости
    printf("Rank is shared: \n");
#pragma omp parallel shared(rank)
    {
        rank = omp_get_thread_num(); // Переменная rank общая для всех потоков
        usleep(100000); // Имитация длительных вычислений (100000 микросекунд = 100 мс)
        printf("I am %d thread.\n", rank);
    }
    return 0;
}


#include <stdio.h>
#include <omp.h>

// Golenishchev Artem, KE-220 Task 3
int main() {
    int k;

    printf("Enter the number of threads: ");
    scanf("%d", &k);

    omp_set_num_threads(k);

#pragma omp parallel
    {
        // Получаем идентификатор текущего потока (нити)
        int thread_id = omp_get_thread_num();
        // Получаем общее количество потоков
        int num_threads = omp_get_num_threads();

        // Проверяем, является ли номер нити четным
        if (thread_id % 2 == 0) {
            printf("I am %d thread from %d threads!\n", thread_id, num_threads);
        }
    }

    return 0;
}

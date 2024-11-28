#include <stdio.h>
#include <omp.h>

// Golenishchev Artem, KE-220 Task 9
int main() {
    int k;
    printf("Enter the number of threads: ");
    scanf("%d", &k);
    omp_set_num_threads(k);

#pragma omp parallel
    {
        // Вывод сообщения о параллельной области
        printf("[%d]: parallel region\n", omp_get_thread_num());

#pragma omp barrier // Синхронизация перед секциями

#pragma omp sections
        {
#pragma omp section
            printf("[%d]: came in section 1\n", omp_get_thread_num());

#pragma omp section
            printf("[%d]: came in section 2\n", omp_get_thread_num());

#pragma omp section
            printf("[%d]: came in section 3\n", omp_get_thread_num());
        }
    }
    return 0;
}

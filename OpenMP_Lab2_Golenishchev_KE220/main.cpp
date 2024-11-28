#include <stdio.h>
#include <omp.h>
// Golenishchev Artem, KE-220 Task 4
int main() {
    int k;
    printf("Enter number of threads (k): ");
    scanf("%d", &k);
    omp_set_num_threads(k);
    printf("Rank is shared: \n");
// Параллельная область с общей переменной rank
#pragma omp parallel shared(k)
    {
        int rank = omp_get_thread_num();
        printf("I am %d thread.\n", rank);
    }
    printf("\nRank is private: \n");
// Параллельная область с частной переменной rank
#pragma omp parallel
    {
        int rank = omp_get_thread_num(); // Локальная переменная rank
        printf("I am %d thread.\n", rank);
    }
    return 0;
}

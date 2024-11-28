#include <cstdio>
#include <cstring>
#include <omp.h>

// Golenishchev Artem, KE-220 Task 7
int main() {
    int k, N, chunk_start, chunk_end;
    char schedule_type[20];

    // Ввод параметров
    printf("Threads (k): ");
    scanf("%d", &k);
    printf("Numbers (N): ");
    scanf("%d", &N);
    printf("Schedule (static, dynamic, guided): ");
    scanf("%s", schedule_type);
    printf("Chunk size range (from-to): ");
    scanf("%d %d", &chunk_start, &chunk_end);

    omp_set_num_threads(k); // Установка потоков

    for (int chunk = chunk_start; chunk <= chunk_end; ++chunk) {
        // Установка расписания
        if (strcmp(schedule_type, "static") == 0) {
            printf("\nSchedule(static, %d):\n", chunk);
        } else if (strcmp(schedule_type, "dynamic") == 0) {
            printf("\nSchedule(dynamic, %d):\n", chunk);
        } else if (strcmp(schedule_type, "guided") == 0) {
            printf("\nSchedule(guided, %d):\n", chunk);
        } else {
            printf("Invalid schedule type.\n");
            return 1;
        }

#pragma omp parallel
        {
#pragma omp for schedule(runtime)
            for (int i = 1; i <= N; i++) {
                printf("[Thread %d]: Iteration %d\n", omp_get_thread_num(), i);
            }
        }
    }

    return 0;
}

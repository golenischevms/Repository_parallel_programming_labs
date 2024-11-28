#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <openacc.h>

#define N 1000000000  // количество точек

// Функция для вычисления числа Пи с использованием метода Монте-Карло (последовательная версия)
double monte_carlo_serial() {
    int count = 0;
    for (int i = 0; i < N; i++) {
        float x = (float)rand() / RAND_MAX;  // случайная координата x
        float y = (float)rand() / RAND_MAX;  // случайная координата y
        if (x * x + y * y <= 1.0) {          // точка внутри круга
            count++;
        }
    }
    return 4.0 * count / N;  // возвращаем приближенное значение Pi
}

// Функция для вычисления числа Пи с использованием метода Монте-Карло (с OpenACC)
double monte_carlo_parallel() {
    int count = 0;

    #pragma acc parallel loop reduction(+:count)
    for (int i = 0; i < N; i++) {
        float x = (float)rand() / RAND_MAX;  // случайная координата x
        float y = (float)rand() / RAND_MAX;  // случайная координата y
        if (x * x + y * y <= 1.0) {          // точка внутри круга
            count++;
        }
    }
    return 4.0 * count / N;  // возвращаем приближенное значение Pi
}

int main() {
    // Инициализация генератора случайных чисел
    srand(time(NULL));

    // Измерение времени последовательной реализации
    clock_t start_serial = clock();
    double pi_serial = monte_carlo_serial();
    clock_t end_serial = clock();
    double time_serial = (double)(end_serial - start_serial) / CLOCKS_PER_SEC;

    // Измерение времени параллельной реализации с использованием OpenACC
    clock_t start_parallel = clock();
    double pi_parallel = monte_carlo_parallel();
    clock_t end_parallel = clock();
    double time_parallel = (double)(end_parallel - start_parallel) / CLOCKS_PER_SEC;

    // Вывод результатов
    printf("Результат (последовательная версия): %f\n", pi_serial);
    printf("Время (последовательная версия): %f секунд\n", time_serial);
    printf("Результат (параллельная версия с OpenACC): %f\n", pi_parallel);
    printf("Время (параллельная версия с OpenACC): %f секунд\n", time_parallel);

    return 0;
}


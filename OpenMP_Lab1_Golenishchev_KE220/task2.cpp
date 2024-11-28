#include <stdio.h>
#include <omp.h>
// Golenishchev Artem, KE-220 Task 2
int main() {
// Параллельный регион
#pragma omp parallel
    {
        printf("Hello, World!\n");
    }

    return 0;
}

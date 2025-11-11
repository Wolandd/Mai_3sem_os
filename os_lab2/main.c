#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "find_min_max.h"

static void fill_random(int *data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        data[i] = rand() % 200001 - 100000;  // диапазон [-100000;100000]
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Использование: %s <размер_массива> <макс_потоков>\n", argv[0]);
        return 1;
    }

    size_t len = (size_t)atoll(argv[1]);
    int max_threads = atoi(argv[2]);
    if (len == 0 || max_threads <= 0) {
        fprintf(stderr, "Длина массива и количество потоков должны быть > 0\n");
        return 1;
    }

    int *data = malloc(len * sizeof(int));
    if (!data) {
        perror("malloc");
        return 1;
    }

    srand((unsigned)time(NULL));
    fill_random(data, len);

    clock_t start = clock();
    MinMax res = find_min_max(data, len, max_threads);
    clock_t end = clock();

    printf("Минимум: %d\nМаксимум: %d\n", res.min, res.max);
    printf("Потоков: %d\n", max_threads);
    printf("Время: %.4f с\n", (double)(end - start) / CLOCKS_PER_SEC);

    free(data);
    return 0;
}


#ifndef FIND_MIN_MAX_H
#define FIND_MIN_MAX_H

#include <stddef.h>

typedef struct {
    int min;
    int max;
} MinMax;

MinMax find_min_max(const int *data, size_t len, int max_threads);

#endif


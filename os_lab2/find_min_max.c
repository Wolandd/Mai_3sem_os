#include "find_min_max.h"

#include <limits.h>
#include <pthread.h>
#include <stddef.h>

typedef struct {
    const int *data;
    size_t start;
    size_t end;
    MinMax *out;
} ThreadArgs;

static void *worker(void *arg) {
    ThreadArgs *a = (ThreadArgs *)arg;
    int local_min = a->data[a->start];
    int local_max = a->data[a->start];

    for (size_t i = a->start + 1; i < a->end; ++i) {
        int v = a->data[i];
        if (v < local_min) local_min = v;
        if (v > local_max) local_max = v;
    }

    a->out->min = local_min;
    a->out->max = local_max;
    return NULL;
}

MinMax find_min_max(const int *data, size_t len, int max_threads) {
    MinMax result = { .min = INT_MAX, .max = INT_MIN };
    if (len == 0 || max_threads <= 0) return result;

    size_t threads_count = (size_t)max_threads;
    if (threads_count > len) threads_count = len;

    pthread_t threads[threads_count];
    ThreadArgs args[threads_count];
    MinMax partial[threads_count];

    size_t chunk = (len + threads_count - 1) / threads_count;
    size_t created = 0;

    for (size_t t = 0, start = 0; t < threads_count && start < len; ++t, start += chunk) {
        size_t end = start + chunk;
        if (end > len) end = len;

        args[t].data = data;
        args[t].start = start;
        args[t].end = end;
        args[t].out = &partial[t];

        pthread_create(&threads[t], NULL, worker, &args[t]);
        ++created;
    }

    for (size_t t = 0; t < created; ++t) {
        pthread_join(threads[t], NULL);
        if (partial[t].min < result.min) result.min = partial[t].min;
        if (partial[t].max > result.max) result.max = partial[t].max;
    }

    return result;
}

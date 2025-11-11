#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

/* Константы */
#define SHM_NAME "/shm"
#define SHM_SIZE 1024
#define MAX_FILENAME_LEN 256

/* Сигналы */
#define SIGNAL_READY SIGUSR1
#define SIGNAL_START SIGUSR2
#define SIGNAL_CHILD_EXIT SIGCHLD

/* Прототипы функций для работы с разделяемой памятью */
void* create_shared_memory(const char* name, size_t size);
void* open_shared_memory(const char* name, size_t size);
void close_shared_memory(void* addr, size_t size);
void unlink_shared_memory(const char* name);

#endif /* COMMON_H */




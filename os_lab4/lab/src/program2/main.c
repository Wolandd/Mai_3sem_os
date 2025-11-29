#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "../../include/functions.h"

typedef struct {
    void* handle;
    float (*SinIntegral)(float A, float B, float e);
    char* (*translation)(long x);
    int impl_number;
} LibraryFunctions;

static LibraryFunctions lib = {NULL, NULL, NULL, 1};

int load_library(const char* path) {
    if (lib.handle != NULL) {
        dlclose(lib.handle);
        lib.handle = NULL;
        lib.SinIntegral = NULL;
        lib.translation = NULL;
    }
    
    dlerror();
    
    lib.handle = dlopen(path, RTLD_LAZY);
    if (lib.handle == NULL) {
        const char* error = dlerror();
        if (error != NULL) {
            fprintf(stderr, "Error loading library '%s': %s\n", path, error);
        }
        return 0;
    }
    
    dlerror();
    
    lib.SinIntegral = (float (*)(float, float, float))dlsym(lib.handle, "SinIntegral");
    const char* dlsym_error = dlerror();
    if (dlsym_error != NULL) {
        fprintf(stderr, "Error loading SinIntegral: %s\n", dlsym_error);
        dlclose(lib.handle);
        lib.handle = NULL;
        return 0;
    }
    
    lib.translation = (char* (*)(long))dlsym(lib.handle, "translation");
    dlsym_error = dlerror();
    if (dlsym_error != NULL) {
        fprintf(stderr, "Error loading translation: %s\n", dlsym_error);
        dlclose(lib.handle);
        lib.handle = NULL;
        lib.SinIntegral = NULL;
        return 0;
    }
    
    if (lib.SinIntegral == NULL || lib.translation == NULL) {
        fprintf(stderr, "Error: symbols are NULL\n");
        dlclose(lib.handle);
        lib.handle = NULL;
        return 0;
    }
    
    return 1;
}

void switch_implementation() {
    if (lib.impl_number == 1) {
        if (load_library("../impl2/libimpl2.so")) {
            lib.impl_number = 2;
            printf("Switched to implementation 2\n");
        }
    } else {
        if (load_library("../impl1/libimpl1.so")) {
            lib.impl_number = 1;
            printf("Switched to implementation 1\n");
        }
    }
}

void unload_library() {
    if (lib.handle != NULL) {
        dlclose(lib.handle);
        lib.handle = NULL;
        lib.SinIntegral = NULL;
        lib.translation = NULL;
    }
}

void print_help() {
    printf("Commands: 0, 1 A B e, 2 x, exit\n");
}

int main(int argc, char* argv[]) {
    print_help();
    
    if (!load_library("../impl1/libimpl1.so")) {
        fprintf(stderr, "Failed to load library\n");
        return 1;
    }
    lib.impl_number = 1;
    
    char line[256];
    
    while (1) {
        printf("> ");
        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }
        
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        
        if (strlen(line) == 0) {
            continue;
        }
        
        if (strcmp(line, "exit") == 0 || strcmp(line, "quit") == 0) {
            break;
        }
        
        char command[2];
        if (sscanf(line, "%1s", command) != 1) {
            continue;
        }
        
        if (strcmp(command, "0") == 0) {
            switch_implementation();
        } else if (strcmp(command, "1") == 0) {
            if (lib.SinIntegral == NULL) {
                printf("Error: library not loaded\n");
                continue;
            }
            float A, B, e;
            int parsed = sscanf(line, "%*s %f %f %f", &A, &B, &e);
            if (parsed != 3) {
                printf("Error: invalid arguments\n");
                continue;
            }
            
            if (A >= B) {
                printf("Error: A must be less than B\n");
                continue;
            }
            
            if (e <= 0) {
                printf("Error: step must be positive\n");
                continue;
            }
            
            float result = lib.SinIntegral(A, B, e);
            printf("%f\n", result);
        } else if (strcmp(command, "2") == 0) {
            if (lib.translation == NULL) {
                printf("Error: library not loaded\n");
                continue;
            }
            long x;
            int parsed = sscanf(line, "%*s %ld", &x);
            if (parsed != 1) {
                printf("Error: invalid argument\n");
                continue;
            }
            
            char* result = lib.translation(x);
            if (result != NULL) {
                printf("%s\n", result);
                free(result);
            } else {
                printf("Error: memory allocation failed\n");
            }
        }
    }
    
    unload_library();
    
    return 0;
}


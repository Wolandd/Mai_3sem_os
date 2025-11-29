#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/functions.h"

void print_help() {
    printf("Commands: 0, 1 A B e, 2 x, exit\n");
}

int main(int argc, char* argv[]) {
    print_help();
    
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
            printf("Switching not supported in program1\n");
        } else if (strcmp(command, "1") == 0) {
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
            
            float result = SinIntegral(A, B, e);
            printf("%f\n", result);
        } else if (strcmp(command, "2") == 0) {
            long x;
            int parsed = sscanf(line, "%*s %ld", &x);
            if (parsed != 1) {
                printf("Error: invalid argument\n");
                continue;
            }
            
            char* result = translation(x);
            if (result != NULL) {
                printf("%s\n", result);
                free(result);
            } else {
                printf("Error: memory allocation failed\n");
            }
        }
    }
    
    return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/functions.h"

int test_translation_ternary() {
    printf("Test: translation(5)\n");
    char* result = translation(5);
    
    if (result == NULL) {
        printf("FAILED: NULL pointer\n");
        return 0;
    }
    
    int passed = (strcmp(result, "12") == 0);
    printf("Result: %s, Expected: 12\n", result);
    
    if (passed) {
        printf("PASSED\n");
    } else {
        printf("FAILED\n");
    }
    
    free(result);
    return passed;
}

int main() {
    int passed = test_translation_ternary();
    
    printf("\nResult: %s\n", passed ? "PASSED" : "FAILED");
    
    return passed ? 0 : 1;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/functions.h"

#define PI 3.14159265358979323846f
#define EPSILON 0.01f

int test_sin_integral() {
    printf("Test 1: SinIntegral(0, pi, 0.001)\n");
    float result = SinIntegral(0.0f, PI, 0.001f);
    float expected = 2.0f;
    float diff = fabsf(result - expected);
    
    printf("Result: %f, Expected: %f, Diff: %f\n", result, expected, diff);
    
    if (diff < EPSILON) {
        printf("PASSED\n");
        return 1;
    } else {
        printf("FAILED\n");
        return 0;
    }
}

int test_translation_binary() {
    printf("Test 2: translation(5)\n");
    char* result = translation(5);
    
    if (result == NULL) {
        printf("FAILED: NULL pointer\n");
        return 0;
    }
    
    int passed = (strcmp(result, "101") == 0);
    printf("Result: %s, Expected: 101\n", result);
    
    if (passed) {
        printf("PASSED\n");
    } else {
        printf("FAILED\n");
    }
    
    free(result);
    return passed;
}

int test_translation_zero() {
    printf("Test 3: translation(0)\n");
    char* result = translation(0);
    
    if (result == NULL) {
        printf("FAILED: NULL pointer\n");
        return 0;
    }
    
    int passed = (strcmp(result, "0") == 0);
    printf("Result: %s, Expected: 0\n", result);
    
    if (passed) {
        printf("PASSED\n");
    } else {
        printf("FAILED\n");
    }
    
    free(result);
    return passed;
}

int test_translation_negative() {
    printf("Test 4: translation(-5)\n");
    char* result = translation(-5);
    
    if (result == NULL) {
        printf("FAILED: NULL pointer\n");
        return 0;
    }
    
    int passed = (strcmp(result, "-101") == 0);
    printf("Result: %s, Expected: -101\n", result);
    
    if (passed) {
        printf("PASSED\n");
    } else {
        printf("FAILED\n");
    }
    
    free(result);
    return passed;
}

int test_sin_integral_validation() {
    printf("Test 5: SinIntegral validation\n");
    float result = SinIntegral(5.0f, 3.0f, 0.001f);
    
    int passed = (result == 0.0f);
    printf("Result: %f, Expected: 0.0\n", result);
    
    if (passed) {
        printf("PASSED\n");
    } else {
        printf("FAILED\n");
    }
    
    return passed;
}

int main() {
    int passed = 0;
    int total = 5;
    
    passed += test_sin_integral();
    passed += test_translation_binary();
    passed += test_translation_zero();
    passed += test_translation_negative();
    passed += test_sin_integral_validation();
    
    printf("\nResults: %d/%d tests passed\n", passed, total);
    
    return (passed == total) ? 0 : 1;
}


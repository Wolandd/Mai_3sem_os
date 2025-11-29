#include <stdlib.h>
#include <string.h>
#include "../../include/functions.h"

char* translation(long x) {
    if (x == 0) {
        char* result = (char*)malloc(2);
        if (result == NULL) {
            return NULL;
        }
        result[0] = '0';
        result[1] = '\0';
        return result;
    }
    
    int negative = (x < 0);
    if (negative) {
        x = -x;
    }
    
    int digits = 0;
    long temp = x;
    while (temp > 0) {
        digits++;
        temp /= 3;
    }
    
    char* result = (char*)malloc(digits + (negative ? 2 : 1));
    if (result == NULL) {
        return NULL;
    }
    
    int i = digits + (negative ? 1 : 0);
    result[i] = '\0';
    
    if (negative) {
        result[0] = '-';
    }
    
    temp = x;
    i = digits + (negative ? 1 : 0) - 1;
    while (temp > 0) {
        result[i--] = (temp % 3) + '0';
        temp /= 3;
    }
    
    return result;
}


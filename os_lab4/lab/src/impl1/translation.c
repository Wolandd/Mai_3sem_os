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
    
    int bits = 0;
    long temp = x;
    while (temp > 0) {
        bits++;
        temp >>= 1;
    }
    
    char* result = (char*)malloc(bits + (negative ? 2 : 1));
    if (result == NULL) {
        return NULL;
    }
    
    int i = bits + (negative ? 1 : 0);
    result[i] = '\0';
    
    if (negative) {
        result[0] = '-';
    }
    
    temp = x;
    i = bits + (negative ? 1 : 0) - 1;
    while (temp > 0) {
        result[i--] = (temp % 2) + '0';
        temp /= 2;
    }
    
    return result;
}


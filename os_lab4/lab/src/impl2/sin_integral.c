#include <math.h>
#include "../../include/functions.h"

float SinIntegral(float A, float B, float e) {
    if (A >= B) {
        return 0.0f;
    }
    
    if (e <= 0.0f) {
        return 0.0f;
    }
    
    float integral = 0.0f;
    float x = A;
    
    while (x < B) {
        float step = (x + e < B) ? e : (B - x);
        float y1 = sinf(x);
        float y2 = sinf(x + step);
        integral += (y1 + y2) * step / 2.0f;
        x += step;
    }
    
    return integral;
}


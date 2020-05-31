#pragma once

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_system.h"



#if defined(__cplusplus)
extern "C" {
#endif


#define SETBITS |=
#define CLRBITS &=~



#define clamp_min(var, cval) if ((var) < (cval)) (var) = (cval);
#define clamp_max(var, cval) if ((var) > (cval)) (var) = (cval);

#define min1(var1, var2) (((var1) < (var2))?(var1):(var2))
#define max1(var1, var2) (((var1) > (var2))?(var1):(var2))



uint16_t cycle_through(uint16_t cur_value, uint16_t elem_count, int16_t inc);


#if defined(__cplusplus)
}
#endif


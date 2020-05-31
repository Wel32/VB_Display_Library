#include "common.h"









uint16_t cycle_through(uint16_t cur_value, uint16_t elem_count, int16_t inc)
{
	volatile int32_t temp = cur_value, inc32 = inc;
	temp = temp + inc32;
	
	while (temp >= elem_count) temp -= elem_count;
	while (temp < 0) temp += elem_count;
	
	return temp;
}



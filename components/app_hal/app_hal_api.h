#pragma once
#include "system_include.h"


#if defined(__cplusplus)

#define cpp_swap(var1, var2) {typeof(var1) temp=var1; var1=var2; var2=temp;}

extern "C" {
#endif
	
	
typedef esp_err_t app_err_t;
#define APP_OK ESP_OK
#define APP_ERROR_CHECK(ret) ESP_ERROR_CHECK(ret)

#define SETBITS |=
#define CLRBITS &=~
#define EMBED_CLRBITS(bits) &(~(bits))


#define _CONCAT(arg1, arg2) _INTRN_CONCAT(arg1, arg2)
#define _INTRN_CONCAT(arg1, arg2) arg1##arg2


//uint16_t swap_bytes_16(uint16_t var);
//uint32_t swap_bytes_32(uint32_t var);



#define clamp_min(var, cval) if ((var) < (cval)) (var) = (cval);
#define clamp_max(var, cval) if ((var) > (cval)) (var) = (cval);

#define min1(var1, var2) (((var1) < (var2))?(var1):(var2))
#define max1(var1, var2) (((var1) > (var2))?(var1):(var2))

#define swap1(var1, var2) {uint32_t temp=var1; var1=var2; var2=temp;}
#define update_bits_by_mask(var, new_value, mask) (((var)EMBED_CLRBITS(mask))|((new_value)&(mask)))

#define _0x01_OF(x) ((x)?1:0)

#define check_within(var, min, max) ((var)>=(min)&&(var)<=(max))
#define clamp_within(var, min, max) {clamp_min(var, min);clamp_max(var, max);}
#define check_ptr_within_obj_area(ptr, obj) (check_within((uint8_t*)(ptr), (uint8_t*)&(obj), (uint8_t*)&(obj) + sizeof(obj) - 1))

uint16_t cast_to_module(int32_t num, uint16_t mod);
uint16_t modulo_addition(int32_t num1, int32_t num2, uint16_t mod);
	
#define modulo_addition_in_boundaries(num1, num2, min_value, max_value) (min_value + modulo_addition(ThisScreen.target_t - min_value, step, max_value - min_value + 1))

int32_t get_user_timer_value(TickType_t user_timer_buf);
TickType_t set_user_timer_value(int32_t new_value);
	
int32_t TICKS_OF_MS(int32_t ticks);
	
#define check_user_timer_ms(buf, value) (get_user_timer_value(buf) >= TICKS_OF_MS(value))
#define set_user_timer_ms(buf, new_value) buf = set_user_timer_value(TICKS_OF_MS(new_value))
	
	
typedef union
{
	uint32_t full;
	uint16_t half[2];
	uint8_t b[4];
} i32_databytes_union;


#if defined(__cplusplus)
}
#endif


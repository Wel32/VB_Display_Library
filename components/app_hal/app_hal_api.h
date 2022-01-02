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
	
void safe_change_bits_u8(uint8_t *var, uint8_t bits, uint8_t mask);
void safe_change_bits_u16(uint16_t *var, uint16_t bits, uint16_t mask);
void safe_change_bits_u32(uint32_t *var, uint32_t bits, uint32_t mask);
	
int32_t round_value(float x);
	
	
typedef union
{
	uint32_t full;
	uint16_t half[2];
	uint8_t b[4];
} i32_databytes_union;
	

typedef struct
{
	uint16_t x;
	uint16_t y;
} PiecewiseLinearApproxPointTable;

	uint16_t PiecewiseLlinearApproximation0(uint16_t x, const PiecewiseLinearApproxPointTable* approx_table, uint8_t n_points);
#define PiecewiseLinearApproximation(x, approx_table) PiecewiseLlinearApproximation0(x, approx_table, sizeof(approx_table) / sizeof(PiecewiseLinearApproxPointTable))


uint8_t int_log10(uint16_t var);
int32_t int32_pow(int32_t x, uint8_t y);
uint8_t sign_of_num_mod10(int32_t var, uint8_t n_sign);

uint16_t cycle_through(uint16_t cur_value, uint16_t elem_count, int16_t inc);

uint32_t FindHandle(void** list, void* handle, size_t sizeof_data, uint32_t count);

uint16_t find_index_of_mas(void* elem_ptr, void** in_mas, uint16_t mas_elem_count);


uint8_t random_regularity(uint8_t *index, uint8_t of_numbers_count);


#if defined(__cplusplus)
}
#endif


#include "app_hal_api.h"




inline int32_t TICKS_OF_MS(int32_t ticks)
{
	return ticks / ((int32_t)portTICK_PERIOD_MS);
}

int32_t get_user_timer_value(TickType_t user_timer_buf)
{
	int32_t temp = xTaskGetTickCount();
	temp -= user_timer_buf;
	return temp;
}

TickType_t set_user_timer_value(int32_t new_value)
{
	TickType_t temp = (TickType_t)(-new_value);
	return xTaskGetTickCount() + temp;
}


uint16_t cast_to_module(int32_t num, uint16_t mod)
{
	if (mod == 0) return 0;
	num %= mod;
	if (num < 0) num += mod;
	return num;
}

uint16_t modulo_addition(int32_t num1, int32_t num2, uint16_t mod)
{
	return cast_to_module(num1 + num2, mod);
}



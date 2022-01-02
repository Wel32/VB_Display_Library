#pragma once
#include "app_hal_api.h"

typedef gpio_num_t hal_gpio_num_t;
typedef gpio_config_t hal_gpio_config_t;


#define hal_gpio_config(pGPIOConfig) gpio_config(pGPIOConfig)
#define hal_gpio_pad_select_gpio(gpio_n) gpio_pad_select_gpio(gpio_n)
#define hal_gpio_set_direction(gpio_n, mode) gpio_set_direction(gpio_n, mode)
#define hal_gpio_pulldown_dis(gpio_n) gpio_pulldown_dis(gpio_n)
#define hal_gpio_pullup_en(gpio_n) gpio_pullup_en(gpio_n)
#define hal_gpio_pullup_dis(gpio_n) gpio_pullup_dis(gpio_n)
#define hal_gpio_pulldown_en(gpio_n) gpio_pulldown_en(gpio_n)
#define hal_gpio_get_level(gpio_n) gpio_get_level(gpio_n)
#define hal_gpio_set_level(gpio_n, level) gpio_set_level(gpio_n, level)




#if defined(__cplusplus)
extern "C" {
#endif



#if defined(__cplusplus)
}
#endif
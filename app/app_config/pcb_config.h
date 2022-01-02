#pragma once

#include "pcb_config.h"

#include "app_hal_api.h"
#include "app_hal_gpio/app_hal_gpio.h"
#include "app_hal_spi/app_hal_spi.h"
#include "app_hal_pwm/app_hal_pwm.h"

#include "app/display_init.h"


#if defined(__cplusplus)
extern "C" {
#endif

extern const hal_pwm_timer_config_t* lcd_bl_pwm_timer;
extern const hal_pwm_channel_t* lcd_bl_pwm_channel;
extern const lcd_display_config_t* lcd_gpio_channel;


void gpio_ports_init();



#if defined(__cplusplus)
}
#endif
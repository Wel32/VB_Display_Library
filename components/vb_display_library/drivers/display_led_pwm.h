#pragma once

#include "../app_hal/app_hal_api.h"
#include "app_hal_pwm/app_hal_pwm.h"



#if defined(__cplusplus)
extern "C" {
#endif

	
	
	
#define DISPLAY_LED_PWM_FADE_SPEED 2500
#define DISPLAY_LED_PWM_FADE_CHANGE_MULTIPLIER 3



void display_led_pwm_init(const hal_pwm_channel_config_t* backlight_pwm_channel);
void display_led_set_brightness(uint8_t value);
extern uint8_t immediately_off;
	
	
	
	
	
#if defined(__cplusplus)
}
#endif
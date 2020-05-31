#pragma once

#include "../user_drivers/system_include.h"
#include "../common/common.h"
#include "user_pwm.h"



#if defined(__cplusplus)
extern "C" {
#endif

	
	
	
#define DISPLAY_LED_PWM_FADE_SPEED 2500
#define DISPLAY_LED_PWM_FADE_CHANGE_MULTIPLIER 3



void display_led_pwm_init();
void display_led_set_brightness(uint8_t value);
extern uint8_t immediately_off;
	
	
	
	
	
#if defined(__cplusplus)
}
#endif
#pragma once

#include "app_hal_api.h"
#include "display_options.h"
#include "../drivers/lcd_init.h"
#include "../drivers/display_led_pwm.h"




extern uint8_t ScreenOrient;
extern uint32_t* hide_drawing_layers;
extern uint8_t* display_user_brightness;
extern uint8_t* display_system_brightness;




#if defined(__cplusplus)
extern "C" {
#endif
	
void display_init(const lcd_display_config_t* display_interface_config, const hal_pwm_channel_t* backlight_config);
	
#if ORIENTATIONS_COUNT == 4 || ORIENTATIONS_COUNT == 2
	uint8_t lcdSetOrient(uint8_t orient);
#if ORIENTATIONS_COUNT == 4
	uint8_t lcdChangeOrient(int8_t step_and_dir);
#else
	uint8_t lcdChangeOrient();
#endif
#endif
	
#if defined(__cplusplus)
}
#endif
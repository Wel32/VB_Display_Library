#pragma once

#include "../user_drivers/system_include.h"
#include "display_options.h"





extern uint8_t ScreenOrient;
extern uint8_t* hide_drawing_layers;
extern uint8_t* display_user_brightness;
extern uint8_t* display_system_brightness;




#if defined(__cplusplus)
extern "C" {
#endif
	
void display_init();
	
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
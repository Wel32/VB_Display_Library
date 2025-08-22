#include "display_init.h"
#include "display_core.h"




//Initialize the display
void display_init(const lcd_display_config_t* display_interface_config, const hal_pwm_channel_config_t* backlight_config)
{
	ScreenOrient = INIT_SCREEN_ORIENT;

#if ORIENTATIONS_COUNT == 4 || ORIENTATIONS_COUNT == 2
	init_display_orient();
#endif
	
	lcd_init(display_interface_config, ScreenOrient);
	lcd_led_init(backlight_config);
}
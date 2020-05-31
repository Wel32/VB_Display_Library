#include "display_init.h"
#include "display_core.h"




//Initialize the display
void display_init()
{
#if ORIENTATIONS_COUNT == 4 || ORIENTATIONS_COUNT == 2
	init_display_orient();
#else
	ScreenOrient = INIT_SCREEN_ORIENT;
#endif
	
	lcd_init(ScreenOrient);
	lcd_led_init();
}
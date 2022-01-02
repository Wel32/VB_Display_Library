#include "app_config.h"




uint32_t HideDrawingLayers = 1;
uint8_t LCD_Brightness = 100;


void attach_display_app()
{
	display_system_brightness = &LCD_Brightness;
	hide_drawing_layers = &HideDrawingLayers;
}
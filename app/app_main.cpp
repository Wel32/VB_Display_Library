#include "app_main.h"
#include "app_config/app_config.h"

#include "user_menu/user_menu_screens.h"




void sl_app_hwinit() 
{
	gpio_ports_init();
}

void sl_app_config_init() 
{
	attach_display_app();
	display_init(lcd_gpio_channel, lcd_bl_pwm_channel);
	
	user_menu_init(8000, Screen1::Select);
}

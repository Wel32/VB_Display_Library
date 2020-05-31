#include "display_led.h"



uint8_t* display_user_brightness = NULL;
uint8_t* display_system_brightness = NULL;





void lcd_led_init()
{
	assert(display_system_brightness);
	
	display_led_pwm_init();
	
	/*
	gpio_pad_select_gpio(DISPLAY_LED_GPIO);
	gpio_set_direction(DISPLAY_LED_GPIO, GPIO_MODE_OUTPUT);
	gpio_set_level(DISPLAY_LED_GPIO, 0);
	*/
}
void lcdOff()
{
	display_led_set_brightness(0);
	
	//gpio_set_level(DISPLAY_LED_GPIO, 0);
}

void lcdImmediatelyOff()
{
	immediately_off = 1;
	display_led_set_brightness(0);
}



void lcdOn()
{
	uint8_t brght;
	
	if (display_user_brightness == NULL) brght = *display_system_brightness;
	else if (*display_user_brightness >= 100) brght = *display_system_brightness;
	else if (*display_user_brightness == 0) brght = 0;
	else
	{
		brght = (((uint32_t)(*display_system_brightness))*(*display_user_brightness)) / 100;
	}
	
	display_led_set_brightness(brght);
	
	//gpio_set_level(DISPLAY_LED_GPIO, 1);
}


void UserLcdOff()
{
	if (display_user_brightness == NULL) *display_user_brightness = 0;
	lcdOn();
}
void UserLcdOn()
{
	if (display_user_brightness == NULL) *display_user_brightness = 100;
	lcdOn();
}

void UserLcdSetBrightness(uint8_t value)
{
	clamp_max(value, 100);
	if (display_user_brightness == NULL) *display_user_brightness = value;
	lcdOn();
}

void UserLcdTrimBrightness(uint8_t value)
{
	clamp_max(value, 100);
	
	if (display_user_brightness == NULL) 
	{
		if (value < *display_system_brightness)
		{
			*display_user_brightness = ((uint32_t)value * 100) / (*display_system_brightness);
		}
		else *display_user_brightness = 100;
	}
	
	lcdOn();
}


void UserLcdSetBrightnessQuietly(uint8_t value)
{
	clamp_max(value, 100);
	if (display_user_brightness == NULL) *display_user_brightness = value;
}
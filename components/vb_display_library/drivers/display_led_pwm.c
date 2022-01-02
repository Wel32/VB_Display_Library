#include "display_led_pwm.h"


hal_pwm_channel_t* backlight_pwm_channel = NULL;
SemaphoreHandle_t xChangeDisplayLedPWM = NULL;

uint8_t target_brightness = 0, immediately_off = 0;




void display_led_set_brightness(uint8_t value)
{
	if (value >= 100) target_brightness = 255;
	else if (!value) target_brightness = 0;
	else
	{
		uint32_t internal_value = value;
	
		target_brightness = (internal_value*internal_value * 255) / 10000;
	}
	
	xSemaphoreGive(xChangeDisplayLedPWM);
}


#define current_brightness internal_display_brightness.b[1]

void led_pwm_task(void *pvParameter)
{
	xChangeDisplayLedPWM = xSemaphoreCreateBinary();
	configASSERT(xChangeDisplayLedPWM);

	TickType_t change_speed = portMAX_DELAY;
	int16_t inc = 0;
	uint8_t internal_avg_brightness1 = 0;
	uint8_t internal_avg_brightness2 = 0;
	
	union
	{
		uint32_t full; uint16_t i[2]; uint8_t b[4];
	} internal_display_brightness;
	
	internal_display_brightness.full = 0;

	while (1) 
	{
		if (xSemaphoreTake(xChangeDisplayLedPWM, change_speed) == pdTRUE)
		{
			if (immediately_off)
			{
				current_brightness = target_brightness = 0;

				hal_pwm_set_duty(backlight_pwm_channel, current_brightness);
				
				immediately_off = 0;
				change_speed = portMAX_DELAY;
				continue;
			}
			else if (current_brightness == target_brightness)
			{
				change_speed = portMAX_DELAY;
				continue;
			}
			else
			{
				change_speed = 1;
				
				internal_display_brightness.i[1] = 0;
				
				uint8_t temp11;
					
				if (current_brightness < target_brightness)
				{
					inc = DISPLAY_LED_PWM_FADE_SPEED;
					temp11 = target_brightness - current_brightness;
				}
				else
				{
					inc = -DISPLAY_LED_PWM_FADE_SPEED;
					temp11 = current_brightness - target_brightness;
				}
				
				internal_avg_brightness1 = min1(current_brightness, target_brightness) + temp11 / 3;
				internal_avg_brightness2 = internal_avg_brightness1 + temp11 / 3;
			}
		}
		
		internal_display_brightness.full += inc;
		
		if (inc>0)
		{
			if (current_brightness > internal_avg_brightness1)
			{
				inc *= DISPLAY_LED_PWM_FADE_CHANGE_MULTIPLIER;
				internal_avg_brightness1 = 255;
			}
			
			if (current_brightness > internal_avg_brightness2)
			{
				inc /= DISPLAY_LED_PWM_FADE_CHANGE_MULTIPLIER;
				internal_avg_brightness2 = 255;
			}
			
			clamp_max(current_brightness, target_brightness);
		}
		else
		{
			if (current_brightness < internal_avg_brightness2)
			{
				inc *= DISPLAY_LED_PWM_FADE_CHANGE_MULTIPLIER;
				internal_avg_brightness2 = 0;
			}
			
			if (current_brightness < internal_avg_brightness1)
			{
				inc /= DISPLAY_LED_PWM_FADE_CHANGE_MULTIPLIER;
				internal_avg_brightness1 = 0;
			}
			
			clamp_min(current_brightness, target_brightness);
		}
		
		if (internal_display_brightness.i[1]) //the end of fading
		{
			current_brightness = target_brightness;
			
			change_speed = portMAX_DELAY;
		}

		hal_pwm_set_duty(backlight_pwm_channel, current_brightness);
	}
}



void display_led_pwm_init(const hal_pwm_channel_t* init_pwm_channel)
{
	if (init_pwm_channel!=NULL)
	{
		backlight_pwm_channel = (hal_pwm_channel_t*) init_pwm_channel;

		xTaskCreate(&led_pwm_task, "led_pwm_task", 1000, NULL, 15, NULL);
	}
}
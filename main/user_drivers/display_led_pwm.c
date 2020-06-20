#include "display_led_pwm.h"



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
	
	//ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE, DISPLAY_LED_PWM_CHANNEL, internal_value, DISPLAY_LED_PWM_FADE_SPEED);
	//ledc_fade_start(LEDC_HIGH_SPEED_MODE, DISPLAY_LED_PWM_CHANNEL, LEDC_FADE_NO_WAIT);
	
	//ledc_set_duty(LEDC_HIGH_SPEED_MODE, DISPLAY_LED_PWM_CHANNEL, value);
	//ledc_update_duty(LEDC_HIGH_SPEED_MODE, DISPLAY_LED_PWM_CHANNEL);
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
				
				ledc_set_duty(LEDC_HIGH_SPEED_MODE, DISPLAY_LED_PWM_CHANNEL, current_brightness);
				ledc_update_duty(LEDC_HIGH_SPEED_MODE, DISPLAY_LED_PWM_CHANNEL);
				
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
		
		ledc_set_duty(LEDC_HIGH_SPEED_MODE, DISPLAY_LED_PWM_CHANNEL, current_brightness);
		ledc_update_duty(LEDC_HIGH_SPEED_MODE, DISPLAY_LED_PWM_CHANNEL);
	}
}



void display_led_pwm_init()
{
	ledc_timer_config_t ledc_timer = {
		.duty_resolution = LEDC_TIMER_8_BIT,
		  // resolution of PWM duty
		.freq_hz = 2000,
		                       // frequency of PWM signal
		.speed_mode = LEDC_HIGH_SPEED_MODE, 
		           // timer mode
		.timer_num = DISPLAY_LED_TIMER,
		             // timer index
		//.clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
	};
	// Set configuration of timer0 for high speed channels
	ledc_timer_config(&ledc_timer);

	ledc_channel_config_t ledc_channel = {
    
		.channel = DISPLAY_LED_PWM_CHANNEL,
		.duty = 0,
		.gpio_num = DISPLAY_LED_GPIO,
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.hpoint = 0,
		.intr_type = LEDC_INTR_FADE_END,
		.timer_sel = DISPLAY_LED_TIMER,
    
	};

	ledc_channel_config(&ledc_channel);

	ledc_fade_func_install(0);
	
	xTaskCreate(&led_pwm_task, "led_pwm_task", 1000, NULL, 15, NULL);
}
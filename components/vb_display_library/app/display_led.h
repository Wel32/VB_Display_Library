#pragma once

#include "app_hal_api.h"

#include "../drivers/user_lcd.h"
#include "../drivers/display_led_pwm.h"





void lcd_led_init(const hal_pwm_channel_config_t* backlight_config);
void lcdOff();
void lcdOn();
void lcdImmediatelyOff();

void UserLcdOff();
void UserLcdOn();
void UserLcdSetBrightness(uint8_t value);
void UserLcdTrimBrightness(uint8_t value);
void UserLcdSetBrightnessQuietly(uint8_t value);
void UserLcdTrimBrightnessQuietly(uint8_t value);
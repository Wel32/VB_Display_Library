#pragma once

#include "../user_drivers/system_include.h"
#include "../user_main.h"

#include "../user_drivers/user_lcd.h"
#include "../user_drivers/display_led_pwm.h"







void lcd_led_init();
void lcdOff();
void lcdOn();
void lcdImmediatelyOff();

void UserLcdOff();
void UserLcdOn();
void UserLcdSetBrightness(uint8_t value);
void UserLcdTrimBrightness(uint8_t value);
void UserLcdSetBrightnessQuietly(uint8_t value);
void UserLcdTrimBrightnessQuietly(uint8_t value);
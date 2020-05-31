#pragma once

#include "../user_drivers/system_include.h"



#define DISPLAY_LED_GPIO GPIO_NUM_16



//LCD_SPI_CHANNEL == HSPI_HOST

#define LCD_SPI_CHANNEL HSPI_HOST
#define PIN_NUM_MISO GPIO_NUM_12
#define PIN_NUM_MOSI GPIO_NUM_13
#define PIN_NUM_CLK  GPIO_NUM_14
#define PIN_NUM_CS   GPIO_NUM_15
#define PIN_NUM_DC   GPIO_NUM_27


/*
// LCD_SPI_CHANNEL == VSPI

#define LCD_SPI_CHANNEL VSPI_HOST
#define PIN_NUM_MISO GPIO_NUM_19
#define PIN_NUM_MOSI GPIO_NUM_23
#define PIN_NUM_CLK  GPIO_NUM_18
#define PIN_NUM_CS   GPIO_NUM_5
#define PIN_NUM_DC   GPIO_NUM_22

*/



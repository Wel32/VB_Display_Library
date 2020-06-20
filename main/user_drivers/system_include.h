#pragma once


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_intr_alloc.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/spi_master.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/touch_pad.h"
#include "driver/ledc.h"
#include "sdkconfig.h"

#include "user_gpio_config.h"


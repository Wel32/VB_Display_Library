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
#include "esp_event.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "esp_intr_alloc.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "sdkconfig.h"

#include "user_gpio_config.h"


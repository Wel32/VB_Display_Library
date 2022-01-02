#pragma once
#include "../app_hal/app_hal_api.h"
#include "../app_hal/app_hal_gpio/app_hal_gpio.h"
#include "../app_hal/app_hal_spi/app_hal_spi.h"



extern uint8_t display_is_initialized;



#define LCD_INIT_CMD(cmd,n_arg,post_delay) cmd, n_arg, post_delay
#define LCD_WR_CMD(cmd) lcd_send_cmd(cmd)
#define LCD_WR_DATA(...) {uint8_t d[] = {__VA_ARGS__}; lcd_send_data(d, sizeof(d));}




#define LCD_SPI_ENABLE_PARTIAL_DATA_TRANSFER 1
#define LCD_SPI_ENABLE_DMA 1



//arguments of 0x36 lcd cmd for set the orientation
#define LCD_CMD_SET_ORIENT 0x36

#define LCD_DATA_PR2_ORIENT 0xC0
#define LCD_DATA_AL2_ORIENT 0x60

#define LCD_DATA_PR_ORIENT 0x00
#define LCD_DATA_AL_ORIENT 0xA0

//0x80 //pr mir
//0xE0 //al mir
//0xA0 //al 180
//0x00 //pr 180
//0x60 //al
//0xC0 //pr
//0x50 //pr mir 180
//0x20 //al mir 180


#if defined(__cplusplus)
extern "C" {
#endif

typedef struct
{
    const hal_spi_port_config_t* spi_port_config;
    const hal_gpio_num_t* miso_gpio;
    const hal_gpio_num_t* mosi_gpio;
    const hal_gpio_num_t* clk_gpio;
    const hal_gpio_num_t* cs_gpio;
    const hal_gpio_num_t* dc_gpio;
    const hal_gpio_num_t* rst_gpio;
}
lcd_display_config_t;

void lcd_internal_set_orient(uint8_t orient);
void lcd_init(const lcd_display_config_t* display_interface_config, uint8_t init_orient);
void lcd_select_rect(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void lcd_set_pixel(uint8_t* pix_ptr);
	
#if defined(__cplusplus)
}
#endif
#pragma once

#include "../user_drivers/system_include.h"
#include "lcd_init.h"


#if defined(__cplusplus)
extern "C" {
#endif
	
	
	



#define LCD_SPI_RATE_kHz 40000
#define LCD_SPI_ENABLE_PARTIAL_DATA_TRANSFER 1
#define LCD_SPI_ENABLE_DMA 1


#define LCD_SEL_DATA 1
#define LCD_SEL_CMD 0
	
	
	
#define LCD_PORTRAIT2 3
#define LCD_ALBUM2 2
#define LCD_PORTRAIT1 1
#define LCD_ALBUM1 0


#define LCD_INTERNAL_SPI_ENABLE_DMA (LCD_SPI_ENABLE_DMA && LCD_SPI_ENABLE_PARTIAL_DATA_TRANSFER)
	

extern spi_device_handle_t spi;



typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

typedef enum {
    LCD_TYPE_ILI = 1,
    LCD_TYPE_ST,
    LCD_TYPE_MAX,
} type_lcd_t;




void lcd_spi_pre_transfer_callback(spi_transaction_t *t);
void lcd_spi_post_transfer_callback(spi_transaction_t *t);


void lcd_interface_init();

void lcd_send_cmd(uint8_t cmd);
void lcd_send_data(uint8_t *data, int len);
void lcd_send_part_of_data(uint8_t *data, int len);
void lcd_start_transfer(int mode);
void lcd_end_transfer();
int lcd_isReady();
	
	
uint8_t lcd_set_orient(uint8_t orient);
	
	
	
#if defined(__cplusplus)
}
#endif
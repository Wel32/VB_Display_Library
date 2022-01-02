#include "lcd_init.h"
#include "user_lcd.h"


uint8_t display_is_initialized = 0;


//Initialize the display
void lcd_init(const lcd_display_config_t* display_interface_config, uint8_t init_orient)
{
	lcd_interface_init(display_interface_config);
        
        
	LCD_WR_CMD(0x01);
	LCD_WR_DATA(0x00);
	
	vTaskDelay(50 / portTICK_RATE_MS);

	LCD_WR_CMD(0x28);
	LCD_WR_DATA(0x00);

	LCD_WR_CMD(0xC0);           // Power Control 1
	LCD_WR_DATA(0x13, 0x13);

	LCD_WR_CMD(0xC1);           // Power Control 2
	LCD_WR_DATA(0x47);    //0x44...0x47

	LCD_WR_CMD(0xC2);         // Power Control 3
	LCD_WR_DATA(0x44);  //is highest freq, default - 0x33

	LCD_WR_CMD(0xC5);         // VCOM Control
	LCD_WR_DATA(0x00, 0x48, 0x80, 0x40);

	LCD_WR_CMD(0xB6);           // Display Function Control
	LCD_WR_DATA(0x00, 0x02, 0x3B);
    
	LCD_WR_CMD(0xB7);           // Entry Mode Set
	LCD_WR_DATA(0xC6);
	
	//LCD_WR_CMD(0xCF);
	//LCD_WR_DATA(0x00);

	/*
	LCD_WR_CMD(0xE0);   // PGAMCTRL(Positive Gamma Control)
	LCD_WR_DATA(0x0F,0x1F,0x1C,0x0C,0x0F,0x08,0x48,0x98,0x37,0x0A,0x13,0x04,0x11,0x0D,0x00);

	LCD_WR_CMD(0xE1);   // NGAMCTRL (Negative Gamma Correction)
	LCD_WR_DATA(0x0F,0x32,0x2E,0x0B,0x0D,0x05,0x47,0x75,0x37,0x06,0x10,0x03,0x24,0x20,0x00);
    */
    
	LCD_WR_CMD(0x20);           // Display Inversion OFF, 0x21 = ON
	
	lcd_internal_set_orient(init_orient);          //set orientation
    

	LCD_WR_CMD(0x3A);           // Interface Pixel Format
	LCD_WR_DATA(0x66);

	LCD_WR_CMD(0x11);
	LCD_WR_CMD(0x29);    //Display on

	vTaskDelay(150 / portTICK_RATE_MS);
	
	display_is_initialized = 1;
}



void lcd_internal_set_orient(uint8_t orient)
{
	uint8_t data;
	
	switch (orient)
	{
	case LCD_PORTRAIT1:
		data = LCD_DATA_PR_ORIENT;
		break;
	case LCD_PORTRAIT2:
		data = LCD_DATA_PR2_ORIENT;
		break;
	case LCD_ALBUM2:
		data = LCD_DATA_AL2_ORIENT;
		break;
	default:
		data = LCD_DATA_AL_ORIENT;
	}
	
	lcd_send_cmd(LCD_CMD_SET_ORIENT);
	lcd_send_data(&data, 1);
}


void lcd_select_rect(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
	uint8_t data[4];

	data[0] = x0 >> 8;
	data[1] = x0 & 0xFF;
	data[2] = x1 >> 8;
	data[3] = x1 & 0xFF;

	lcd_send_cmd(0x2A);
	lcd_send_data(data, 4);

	data[0] = y0 >> 8;
	data[1] = y0 & 0xFF;
	data[2] = y1 >> 8;
	data[3] = y1 & 0xFF;

	lcd_send_cmd(0x2B);
	lcd_send_data(data, 4);

	lcd_send_cmd(0x2C);
}

void lcd_set_pixel(uint8_t* pix_ptr)
{

}
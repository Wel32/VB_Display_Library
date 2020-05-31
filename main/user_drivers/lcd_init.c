#include "lcd_init.h"


uint8_t display_is_initialized = 0;


//Initialize the display
void lcd_init(uint8_t init_orient)
{
	lcd_interface_init();
        
        
	LCD_WR_CMD(0x01);
	LCD_WR_DATA(0x00);
	vTaskDelay(50 / portTICK_RATE_MS);
   

	LCD_WR_CMD(0x28);
	LCD_WR_DATA(0x00);
  
   

	LCD_WR_CMD(0xC0);          // Power Control 1
	LCD_WR_DATA(0x13);  //0x0d);
	LCD_WR_DATA(0x13);  //0x0d);

	LCD_WR_CMD(0xC1);          // Power Control 2
	LCD_WR_DATA(0x46);   //0x44...0x47

	//LCD_WR_CMD(0xC2);        // Power Control 3
	//LCD_WR_DATA(0x44); //is highest freq, default - 0x33

	//LCD_WR_CMD(0xC5);        // VCOM Control
	//LCD_WR_DATA(0x00);
	//LCD_WR_DATA(0x48);

	LCD_WR_CMD(0xB6);          // Display Function Control
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x22);             // 0x42 = Rotate display 180 deg.
	LCD_WR_DATA(0x3B);
    
	LCD_WR_CMD(0xB7);          // Entry Mode Set
	//LCD_WR_DATA(0x86);
	LCD_WR_DATA(0xC6);

	LCD_WR_CMD(0xC2);   // Power Control 3 (For Normal Mode)
	        LCD_WR_DATA(0x44);

	LCD_WR_CMD(0xC5);   // VCOM Control
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);

	LCD_WR_CMD(0xE0);   // PGAMCTRL(Positive Gamma Control)
	LCD_WR_DATA(0x0F);
	LCD_WR_DATA(0x1F);
	LCD_WR_DATA(0x1C);
	LCD_WR_DATA(0x0C);
	LCD_WR_DATA(0x0F);
	LCD_WR_DATA(0x08);
	LCD_WR_DATA(0x48);
	LCD_WR_DATA(0x98);
	LCD_WR_DATA(0x37);
	LCD_WR_DATA(0x0A);
	LCD_WR_DATA(0x13);
	LCD_WR_DATA(0x04);
	LCD_WR_DATA(0x11);
	LCD_WR_DATA(0x0D);
	LCD_WR_DATA(0x00);

	LCD_WR_CMD(0xE1);   // NGAMCTRL (Negative Gamma Correction)
	LCD_WR_DATA(0x0F);
	LCD_WR_DATA(0x32);
	LCD_WR_DATA(0x2E);
	LCD_WR_DATA(0x0B);
	LCD_WR_DATA(0x0D);
	LCD_WR_DATA(0x05);
	LCD_WR_DATA(0x47);
	LCD_WR_DATA(0x75);
	LCD_WR_DATA(0x37);
	LCD_WR_DATA(0x06);
	LCD_WR_DATA(0x10);
	LCD_WR_DATA(0x03);
	LCD_WR_DATA(0x24);
	LCD_WR_DATA(0x20);
	LCD_WR_DATA(0x00);
       
    
	LCD_WR_CMD(0x20);          // Display Inversion OFF, 0x21 = ON
	
	lcd_internal_set_orient(init_orient);         //set orientation before the command 0x11
    

	LCD_WR_CMD(0x3A);          // Interface Pixel Format
#if COLOR_MODE==565
	    LCD_WR_DATA(0x55); 
#else
	LCD_WR_DATA(0x66); 
#endif

	LCD_WR_CMD(0x11);
	LCD_WR_CMD(0x29);   //Display on

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
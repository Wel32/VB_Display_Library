#pragma once

#include "user_lcd.h"


extern uint8_t display_is_initialized;



#define LCD_INIT_CMD(cmd,n_arg,post_delay) cmd, n_arg, post_delay
#define LCD_WR_CMD(cmd) lcd_send_cmd(cmd)
#define LCD_WR_DATA(data) {uint8_t d = data; lcd_send_data(&d, 1);}



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

void lcd_internal_set_orient(uint8_t orient);
void lcd_init(uint8_t init_orient);
	
#if defined(__cplusplus)
}
#endif
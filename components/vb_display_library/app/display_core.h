#pragma once


#include "app_hal_api.h"

#include "../drivers/user_lcd.h"
#include "display_options.h"
#include "display_init.h"
#include "display_led.h"

#include "display.h"







extern uint8_t* string_alloc_buffer;
void alloc_additional_str_buffer();


///////////////////////////////////////////////////////////////////////////////////////////////////////////


#define LAYER_OPTIONS_FIRST_LAYER (1<<0)
#define LAYER_OPTIONS_SINGLE_LAYER (1<<1)



void init_display_orient();
void lcdInternalSetRect(rect r);
uint32_t calc_start_pos(uint32_t img_bias_pix, int16_t y0, uint16_t img_width_elem, uint8_t elem_per_pix);
uint32_t calc_start_pos_vertical_mirror(uint32_t img_bias_pix, int16_t y0, uint16_t img_width_elem, int16_t img_height, uint8_t elem_per_pix);
bool check_horizontal_overlap(rect img_pos, rect mask);
bool check_vertical_overlap(rect img_pos, rect mask);
void common_str_init(internal_draw_obj* ids, rect img_pos, rect mask);
rect internal_max_rect(rect r1, rect r2);
rect internal_min_rect(rect r1, rect r2);


extern void(*str_init_func[])(internal_draw_obj*);
extern void(*str_memcpy_func[])(uint8_t* buf, internal_draw_obj* img);
extern void(*str_memclear_func[])(internal_draw_obj* img);









#pragma once

#include "../draw.h"



void fill_str_init(internal_draw_obj* img);
void fill_str_memcpy(uint8_t* buf, internal_draw_obj* img);
void fill_str_memclear(internal_draw_obj* img);



//draw_obj make_fill(int16_t x, int16_t y, uint16_t width, uint16_t height, uint32_t color, uint8_t align);
draw_obj make_fill(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color, uint8_t options);
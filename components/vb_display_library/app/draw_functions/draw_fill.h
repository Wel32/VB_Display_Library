#pragma once

#include "../draw.h"



void fill_str_init(internal_draw_obj* img);
void fill_str_memcpy(uint8_t* buf, internal_draw_obj* img);
void fill_str_memclear(internal_draw_obj* img);



draw_obj make_fill(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color, uint8_t options);
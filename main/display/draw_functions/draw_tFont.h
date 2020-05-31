#pragma once

#include "../draw.h"
#include "gamma.h"
#include "draw_bmp.h"



typedef struct {
	wchar_t code;
	const tImage *image;
} tChar;
typedef struct {
	uint16_t length;
	const tChar *chars;
} tFont;




void tFont_str_init(internal_draw_obj* img);
void tFont_str_memcpy(uint8_t* buf, internal_draw_obj* img);
void tFont_str_memclear(internal_draw_obj* img);



draw_obj make_tFont_symbol(wchar_t c, tFont font, int16_t x, int16_t y, int32_t color, uint8_t options, uint8_t align);



rect get_tFont_string_common_rect(tFont font, int16_t char_space, const wchar_t* str);
void draw_tFont_string(draw_obj_list draw_buffer, uint8_t layer, tFont font, int16_t char_space, const wchar_t* str, int16_t text_x0, int16_t text_y0, int32_t color, uint8_t options, uint8_t align);
void draw_tFont_strings(draw_obj_list draw_buffer, uint8_t layer, tFont font, int16_t char_space, int16_t string_space, const wchar_t** str, uint8_t str_cnt, int16_t text_x0, int16_t text_y0, int32_t color, uint8_t options, uint8_t align);
uint8_t draw_tFont_string_separate_sprites(draw_obj_list draw_buffer, uint8_t layer, uint8_t max_symbol_cnt, tFont font, int16_t char_space, const wchar_t* str, int16_t text_x0, int16_t text_y0, int32_t color, uint8_t options, uint8_t align);
int16_t fit_tFont_string_char_space(tFont font, int16_t *char_space, const wchar_t* str, int16_t max_length);


bool draw_fitted_tFont_string(draw_obj_list draw_buffer, uint8_t layer, tFont font, int16_t min_char_space, int16_t max_char_space, int16_t max_width, const wchar_t* str, int16_t text_x0, int16_t text_y0, int32_t color, uint32_t options, uint8_t align);
uint8_t draw_fitted_tFont_strings(draw_obj_list draw_buffer, uint8_t layer, tFont font, int16_t min_char_space, int16_t max_char_space, int16_t max_width, int16_t string_space, const wchar_t** str, uint8_t str_cnt, int16_t text_x0, int16_t text_y0, int32_t color, uint32_t options, uint8_t align);
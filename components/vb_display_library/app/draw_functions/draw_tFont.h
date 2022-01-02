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



#if defined(__cplusplus)

draw_obj make_tFont_symbol(wchar_t c, tFont font, int16_t x, int16_t y, uint32_t color, uint8_t options, uint8_t align);



typedef struct
{
	uint32_t layer;
	wstring text;
	tFont font;
	std::vector <int16_t> char_space;
	int16_t string_space;
	uint8_t align;
}
tFont_text;


void set_tFont_text(draw_obj_list* draw_buffer, uint32_t* layer_cnt_store, tFont_text &text, int16_t x0, int16_t y0, uint32_t color, uint8_t options);
tFont_text* tFont_obj_access_to_text_param(draw_obj text_obj);
draw_obj make_tFont_text(draw_obj_list* draw_buffer, tFont_text &text, int16_t x0, int16_t y0, uint32_t color, uint8_t options);

void tFont_text_set_char_space(tFont_text& text, int16_t min_char_space, int16_t max_char_space, int16_t place_length);
void tFont_text_line_autobreake(tFont_text& text, int16_t place_width, int16_t min_char_space, int16_t max_char_space, uint16_t max_lines_cnt);


#endif



uint32_t tFont_autogamma_options(uint32_t bg_color, uint32_t f_color);
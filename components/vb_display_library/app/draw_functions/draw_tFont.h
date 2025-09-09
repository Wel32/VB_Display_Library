#pragma once

#include "../display_datatypes.h"
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




#if defined(__cplusplus)



struct tFontText
{
	wstring text;
	tFont font;
	std::vector <int16_t> char_space;
	int16_t string_space;
	uint8_t align;

	tFontText(tFont init_font = {0, NULL}, uint8_t init_align = CENTER_ALIGN | CENTER_ALIGN, int16_t init_char_space = 0, int16_t init_string_space = 30)
	{
		font = init_font;
		align = init_align;
		string_space = init_string_space;
		if (init_char_space) char_space.push_back(init_char_space);
	}
	tFontText(wstring &str, tFont init_font = {0, NULL}, uint8_t init_align = CENTER_ALIGN | CENTER_ALIGN, int16_t init_char_space = 0, int16_t init_string_space = 30)
	{
		text = str;
		font = init_font;
		align = init_align;
		string_space = init_string_space;
		if (init_char_space) char_space.push_back(init_char_space);
	}

	int16_t get_char_space(uint16_t str_n = 0) {return (str_n < char_space.size()) ? char_space[str_n] : ((char_space.size()) ? char_space[char_space.size() - 1] : 0);}
};


class VBDL_tFont : public draw_obj
{
	friend class VBDL_tFontText;
public:
	VBDL_tFont() = default;
	VBDL_tFont(wchar_t c, tFont font, int16_t x, int16_t y, uint32_t color, uint8_t options, uint8_t align);

	static uint32_t autogamma_options(uint32_t bg_color, uint32_t f_color);

protected:
	void* handle;
	void fill_str_init(std::vector <internal_draw_obj> &buf, rect mask, uint16_t layer_options) override;
	void fill_str_memcpy(uint8_t* buf, internal_draw_obj* img) override;
	void fill_str_memclear(internal_draw_obj* img) override;

private:
	uint8_t conv_data_color(uint8_t data_color, uint8_t options);
};

class VBDL_tFontText : public draw_obj
{
public:
	VBDL_tFontText() = default;
	VBDL_tFontText(tFontText &text, int16_t x0, int16_t y0, uint32_t color, uint8_t options);

	tFontText text_data;

	static uint32_t set_or_update_text(VBDisplay &display, uint32_t* layer_num_handle, uint32_t desired_layer, tFontText &text, int16_t init_text_x0, int16_t init_text_y0, uint32_t color, uint8_t options);
	static void set_or_update_text(VBDisplay &display, uint32_t* layer_num_handle, tFontText &text, int16_t init_text_x0, int16_t init_text_y0, uint32_t color, uint8_t options);
	
	static void set_char_space(tFontText& text, int16_t min_char_space, int16_t max_char_space, int16_t place_length);
	static uint16_t text_lines_count(tFontText& text);
	static uint16_t text_autowrap(tFontText& text, int16_t place_width, int16_t min_char_space, int16_t max_char_space, uint16_t max_lines_cnt);
	static rect get_line_common_rect(tFont font, int16_t char_space, const wchar_t* str, uint32_t max_sym_cnt = -1);

protected:
	void fill_str_init(std::vector <internal_draw_obj> &buf, rect mask, uint16_t layer_options) override;
	void fill_str_memcpy(uint8_t* buf, internal_draw_obj* img) override {}
	void fill_str_memclear(internal_draw_obj* img) override {}

private:
	static void draw_tFont_string(VBDisplay &display, uint32_t layer, rect old_rect, tFont font, int16_t char_space, const wchar_t* str, uint32_t sym_cnt, int16_t text_x0, int16_t text_y0, uint32_t color, uint8_t options, uint8_t align);
	rect internal_fill_str_init(std::vector <internal_draw_obj> *buf, rect mask, uint16_t layer_options);
};

#endif
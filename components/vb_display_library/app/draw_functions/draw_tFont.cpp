#include "../display_core.h"
#include "draw_tFont.h"


#ifdef CORR_GAMMA1
#define INTR_CORR_GAMMA1 CORR_GAMMA1
#else
#define INTR_CORR_GAMMA1 0
#endif

#ifdef CORR_GAMMA2
#define INTR_CORR_GAMMA2 CORR_GAMMA2
#else
#define INTR_CORR_GAMMA2 0
#endif

inline uint8_t VBDL_tFont::conv_data_color(uint8_t data_color, uint8_t options)
{
	if (options&(INTR_CORR_GAMMA1 | INTR_CORR_GAMMA2))
	{
		uint8_t inv_option = options&(INVERT_CORR_GAMMA | DRAW_INV_TRANSPARENCY);
		if (inv_option == INVERT_CORR_GAMMA || inv_option == DRAW_INV_TRANSPARENCY) data_color = 255 - data_color;
		
		if (options&INTR_CORR_GAMMA1) data_color = inverted_tFont_gamma1[data_color];
#ifdef CORR_GAMMA2
		else
#if INTR_CORR_GAMMA1 == 0
			if (options&INTR_CORR_GAMMA2)
#endif
			data_color = inverted_tFont_gamma2[data_color];
#endif
		
		if (options&INVERT_CORR_GAMMA) data_color = 255 - data_color;
	}
	else if (options&DRAW_INV_TRANSPARENCY) data_color = 255 - data_color;
	
	return data_color;
}


rect VBDL_tFontText::get_line_common_rect(tFont font, int16_t char_space, const wchar_t* str, uint32_t max_sym_cnt)
{
	rect res;
	int16_t sym_x0 = res.x0 = res.y0 = 1;
	res.x1 = res.y1 = 0;

	for (const wchar_t *c = str; *c != '\0' && max_sym_cnt; c++, max_sym_cnt--)
	{
		VBDL_tFont sym(*c, font, sym_x0, 1, 0, 0, LEFT_ALIGN | BOTTOM_ALIGN);

		res.x1 = sym.pos.x1;
		res.y1 = max1(res.y1, sym.pos.y1);

		sym_x0 = res.x1 + 1 + char_space;
	}

	return res;
}


uint32_t VBDL_tFont::autogamma_options(uint32_t bg_color, uint32_t f_color)
{
	union intrn_bg_color
	{
		uint32_t full;
		uint8_t b[4];
	} intrn_bg_color, intrn_f_color;
	
	intrn_bg_color.full = bg_color; intrn_f_color.full = f_color;
	
	uint32_t temp_bg = 299*(uint32_t)intrn_bg_color.b[2] + 587*(uint32_t)intrn_bg_color.b[1] + 114*(uint32_t)intrn_bg_color.b[0];
	uint32_t temp_f = 299*(uint32_t)intrn_f_color.b[2] + 587*(uint32_t)intrn_f_color.b[1] + 114*(uint32_t)intrn_f_color.b[0];
	
	if (temp_bg < temp_f) return CORR_GAMMA2 | INVERT_CORR_GAMMA;
	else return 0;
}




////////////////////////////////////////////////////////////////////////////////////

typedef struct 
{
	int16_t min;
	int16_t beg;
}
lbrk_text_length_t;


typedef struct
{
	size_t begin_i;
	size_t end_i;
	lbrk_text_length_t length;
	int16_t char_space;
	bool break_end;
}
lbrk_text_word_info;


void VBDL_tFontText::set_char_space(tFontText& text, int16_t min_char_space, int16_t max_char_space, int16_t place_length)
{
	size_t str_beg_i = 0;
	uint16_t str_cnt = 0;
	text.char_space.clear();

	int16_t past_char_space = 0;

	for (size_t i = 0; i <= text.text.size(); i++)
	{
		if (*(text.text.data() + i) == L'\n' || *(text.text.data() + i) == L'\0')
		{
			int16_t char_cnt = i - str_beg_i;
			int16_t char_space = 0;

			rect check_rect = get_line_common_rect(text.font, char_space, text.text.data() + str_beg_i, i - str_beg_i);
			int16_t text_length = check_rect.x1;

			if (char_cnt > 1)
			{
				char_cnt--;

				int16_t diff_length = text_length - place_length;

				int16_t temp = diff_length / char_cnt;
				if (diff_length % char_cnt)
				{
					if (temp > 0) temp++;
					else if (temp < 0) temp--;
				}

				char_space -= temp;
			}

			str_beg_i = i + 1;

			if (char_space < min_char_space) char_space = min_char_space;
			if (char_space > max_char_space) char_space = max_char_space;

			if (past_char_space != char_space)
			{
				while (text.char_space.size() < str_cnt)
				{
					text.char_space.push_back(past_char_space);
				}

				text.char_space.push_back(char_space);

				past_char_space = char_space;
			}

			str_cnt++;
		}
	}
}

uint16_t VBDL_tFontText::text_lines_count(tFontText& text)
{
	uint16_t ret = 1;

	for (size_t i = 0; i < text.text.size(); i++)
	{
		if (*(text.text.data() + i) == L'\0') break;
		if (*(text.text.data() + i) == L'\n') ret++;
	}

	return ret;
}

uint16_t VBDL_tFontText::text_autowrap(tFontText& text, int16_t place_width, int16_t min_char_space, int16_t max_char_space, uint16_t max_lines_cnt)
{
	if (text.char_space.size()) return text_lines_count(text);
	
	clamp_max(min_char_space, max_char_space);

	int16_t beg_char_space = max_char_space;
	std::vector <lbrk_text_word_info> text_words;

	lbrk_text_length_t total_length = { 0,0 };

	VBDL_tFont space_char(L' ', text.font, 0, 0, 0, 0, LEFT_ALIGN | BOTTOM_ALIGN);
	int16_t sc_width = space_char.pos.x1 - space_char.pos.x0 + 1;

	lbrk_text_length_t text_space_length;
	text_space_length.beg = sc_width + beg_char_space;
	text_space_length.min = sc_width + min_char_space;

	size_t beg_i = 0;
	for (size_t i = 0; i <= text.text.size(); i++)
	{
		if (*(text.text.data() + i) == L'\n') return text_lines_count(text);

		if (*(text.text.data() + i) == L' ' || *(text.text.data() + i) == L'\0')
		{
			lbrk_text_length_t w_lengths = { 0,0 };

			if (i > beg_i)
			{
				rect temp_rect = get_line_common_rect(text.font, beg_char_space, text.text.data() + beg_i, i - beg_i);
				w_lengths.beg = temp_rect.x1 - temp_rect.x0 + 1;
				temp_rect = get_line_common_rect(text.font, min_char_space, text.text.data() + beg_i, i - beg_i);
				w_lengths.min = temp_rect.x1 - temp_rect.x0 + 1;

				total_length.beg += w_lengths.beg;
				total_length.min += w_lengths.min;
			}

			if (*(text.text.data() + i) != L'\0')
			{
				total_length.min += text_space_length.min;
				total_length.beg += text_space_length.beg;
			}

			lbrk_text_word_info temp_wi = { beg_i, i, w_lengths, beg_char_space, 0 };

			text_words.push_back(temp_wi);
			beg_i = i + 1;
		}
	}

	if (total_length.min <= place_width || text_words.size() <= 1)
	{
		set_char_space(text, min_char_space, max_char_space, place_width);
		return 1;
	}

	std::vector <lbrk_text_word_info> temp_text_words1 = text_words, temp_text_words2;
	int16_t max_line_width = 0x7FFF;
	uint32_t passed = 0;
	uint16_t res_lines_cnt = 0;
	uint16_t lines_cnt = 0;

	do {
		temp_text_words2 = text_words;
		text_words = temp_text_words1;

		lbrk_text_length_t max_w_lengths = { 0,0 };
		lbrk_text_length_t group_length = { 0,0 };
		lbrk_text_length_t temp_group_length = { 0,0 };

		lines_cnt = 1;
		size_t past_i = 0;

		for (size_t i = 0; i < text_words.size(); i++)
		{
			temp_group_length.beg = group_length.beg + text_words[i].length.beg;
			temp_group_length.min = group_length.min + text_words[i].length.min;

			bool overlength = temp_group_length.beg >= max_line_width || temp_group_length.min > place_width;

			if (overlength || i == (text_words.size() - 1))
			{
				if (i == past_i)
				{
					group_length.beg = temp_group_length.beg;
					group_length.min = temp_group_length.min;
				}
				else i--;

				past_i = i + 1;

				clamp_min(max_w_lengths.beg, group_length.beg);
				clamp_min(max_w_lengths.min, group_length.min);
				
				if (overlength && i != (text_words.size() - 1))
				{
					text_words[i].break_end = 1;
					group_length = { 0, 0 };

					lines_cnt++;
				}
			}
			else
			{
				group_length.beg = temp_group_length.beg + text_space_length.beg;
				group_length.min = temp_group_length.min + text_space_length.min;
			}
		}

		if (!res_lines_cnt) res_lines_cnt = lines_cnt;
		
		if ((group_length.beg < place_width || group_length.beg < max_line_width) && res_lines_cnt == lines_cnt && res_lines_cnt>1 && (lines_cnt < text_words.size()-1))
		{
			if (max_line_width > max_w_lengths.beg) max_line_width = max_w_lengths.beg;
			else if (max_line_width >= text_space_length.beg) max_line_width -= text_space_length.beg;
		}
		else
		{
			//if (passed && res_lines_cnt != lines_cnt) text_words = temp_text_words2;
			if (passed) text_words = temp_text_words2;
			break;
		}

		passed++;
		if (passed > 200) break;

	} while (1);


	clamp_max(res_lines_cnt, max_lines_cnt);
	uint16_t ret = 1;
	
	for (size_t i = 0; i < text_words.size(); i++)
	{
		if (text_words[i].break_end)
		{
			text.text[text_words[i].end_i] = L'\n';

			if (res_lines_cnt <= 1)
			{
				text.text.resize(text_words[i].end_i);
				break;
			}

			res_lines_cnt--;
			ret++;
		}
	}

	VBDL_tFontText::set_char_space(text, min_char_space, max_char_space, place_width);
	return ret;
}




VBDL_tFont::VBDL_tFont(wchar_t c, tFont font, int16_t _x, int16_t _y, uint32_t _color, uint8_t _options, uint8_t _align)
{
	color = _color;
	options = _options;

	if (c == '\0') return;

	static wchar_t past_c = 0x20;
	static uint16_t past_i = 0;

	const tChar *font_chars = font.chars;
	const tImage *t_char;
	
	uint16_t dc = c - past_c;
	uint16_t i = dc + past_i;
	
	if (i >= font.length || font_chars[i].code != c)
	{
		for (i = 0;; i++)
		{
			if (i == font.length) return;
		
			if (font_chars[i].code == c) break;
		}
	}
	
	past_c = c;
	past_i = i;
	
	t_char = font_chars[i].image;
	
	pos = VBDisplay::get_rect(_x, _y, t_char->width, t_char->height, _align);
	handle = (void*)t_char->data;
}

void VBDL_tFont::fill_str_init(std::vector <internal_draw_obj> &buf, rect mask, uint16_t layer_options)
{
	if (this->handle == NULL) return;
	
	VBDL_InternalPushLayer img(this, mask, layer_options);
	img.temp_payload = this->handle;

	if (!img.is_overlap()) return;

	if (img.obj_handle->options&DRAW_INV_COLOR) img.user_color = VBDisplay::invert_color(img.user_color);
	
#if LCD_INV_BRIGHTNESS
img.user_color = (img.user_color & 0xFF000000) | ((0xFFFFFFFF - img->user_color) & 0xFFFFFF);
#endif
	
	uint32_t com_img_bias = img.calc_start_pos(img.img_cur_pos, img.img_cur_y, img.img_width_elem, 1);
  
	img.img_cur_pos = 0;
	uint8_t data_color = 0;
  
	uint8_t* sprite_data = (uint8_t*)(img.temp_payload);
  
	for (uint32_t i = 0; i < com_img_bias; i++)
	{
		if (!img.img_cur_pos)
		{
			img.img_cur_pos = *sprite_data++;
			img.temp_payload = (void*)sprite_data;
			
			if (img.img_cur_pos < 0x80)
			{
				data_color = *sprite_data++;
				img.temp_payload = (void*)sprite_data;
			}
		}
		
		if (img.img_cur_pos >= 0x80)
		{
			img.img_cur_pos++;
			if (img.img_cur_pos == 256) img.img_cur_pos = 0;
			
			data_color = *sprite_data++;
			img.temp_payload = (void*)sprite_data;
		}
		else img.img_cur_pos--;
	}
  
	union {uint32_t full; uint16_t dc[2]; } mixdata;
	
	uint32_t trans = img.user_color >> 24;
	
	data_color = conv_data_color(data_color, img.obj_handle->options);
		
	mixdata.dc[1] = ((trans * 255 + (data_color * (255 - trans))) * 256) / (255 * 255);
	mixdata.dc[0] = 256 - mixdata.dc[1];     //256...0
	img.user_data = mixdata.full;

	img.confirm(buf);
}
void VBDL_tFont::fill_str_memcpy(uint8_t* buf, internal_draw_obj* img)
{
	uint8_t* sprite_data = (uint8_t*)(img->temp_payload);

	union {uint32_t full; uint8_t color[4]; } spr_color;
	spr_color.full = img->user_color;
  
	union {uint32_t full; uint16_t dc[2]; } mixdata;
	mixdata.full = img->user_data;
  
	uint8_t data_color;
  
	for (uint32_t i = 0; i < img->img_width_elem; i++)
	{
		if (!img->img_cur_pos)
		{
			img->img_cur_pos = *sprite_data++;
			img->temp_payload = (void*)sprite_data;
			
			if (img->img_cur_pos < 0x80)
			{
				data_color = *sprite_data++;
				img->temp_payload = (void*)sprite_data;
				
				uint32_t trans = spr_color.color[3];
				
				data_color = conv_data_color(data_color, img->obj_handle->options);
				
				mixdata.dc[1] = ((trans * 255 + (data_color * (255 - trans))) * 256) / (255 * 255);
				mixdata.dc[0] = 256 - mixdata.dc[1];      //256...0

				img->user_data = mixdata.full;
			}
		}
		
		if (img->img_cur_pos >= 0x80)
		{
			img->img_cur_pos++;
			if (img->img_cur_pos == 256) img->img_cur_pos = 0;
			
			{
				data_color = *sprite_data++;
				img->temp_payload = (void*)sprite_data;
				
				uint8_t trans = spr_color.color[3];
				
				data_color = conv_data_color(data_color, img->obj_handle->options);
				
				mixdata.dc[1] = ((trans * 255 + (data_color * (255 - trans))) * 256) / (255 * 255);
				mixdata.dc[0] = 256 - mixdata.dc[1];       //256...0

				img->user_data = mixdata.full;
			}
		}
		else img->img_cur_pos--;

		if (i < img->buf_cnt_pix)
		{
			*buf = ((mixdata.dc[0]*spr_color.color[0]) + (mixdata.dc[1]*(*buf))) >> 8;
			buf++;
			*buf = ((mixdata.dc[0]*spr_color.color[1]) + (mixdata.dc[1]*(*buf))) >> 8;
			buf++;
			*buf = ((mixdata.dc[0]*spr_color.color[2]) + (mixdata.dc[1]*(*buf))) >> 8;
			buf++;
		}
  
	}
}
void VBDL_tFont::fill_str_memclear(internal_draw_obj* img)
{
}


















VBDL_tFontText::VBDL_tFontText(tFontText &_text, int16_t _x0, int16_t _y0, uint32_t _color, uint8_t _options)
{
	text_data = _text;
	x0 = _x0;
	y0 = _y0;
	color = _color;
	options = _options;
	pos = internal_fill_str_init(NULL, VBDisplay::void_rect, 0);
}

void VBDL_tFontText::fill_str_init(std::vector <internal_draw_obj> &buf, rect mask, uint16_t layer_options)
{
	internal_fill_str_init(&buf, mask, layer_options);
}

inline rect VBDL_tFontText::internal_fill_str_init(std::vector <internal_draw_obj> *out_buf, rect mask, uint16_t layer_options)
{
	int16_t str_x0 = x0;
	int16_t text_y0 = y0;

	uint32_t str_sym_cnt = 0;
	rect res_text_pos = VBDisplay::void_rect;

	int16_t str_char_space = 0;
	uint32_t char_space_string_n = 0;
	if (char_space_string_n < text_data.char_space.size()) str_char_space = text_data.char_space[char_space_string_n++];

	for (const wchar_t* c =  text_data.text.data(); *c != '\0'; c++)
	{
		if (*c != '\n' && *c != '\0')
		{
			if (!str_sym_cnt)
			{
				str_x0 = x0;

				if ((text_data.align & 0x0F) != LEFT_ALIGN)
				{
					for (const wchar_t* c2 = c; *c2 != '\0' && *c2 != '\n'; c2++) str_sym_cnt++;

					if (str_sym_cnt)
					{
						rect new_rect = get_line_common_rect(text_data.font, str_char_space, c, str_sym_cnt);
						str_x0 -= VBDisplay::calc_hor_offset(new_rect.x1, text_data.align);
					}
				}
				else str_sym_cnt = 1;
			}

			if (str_sym_cnt)
			{
				std::unique_ptr<VBDL_tFont> new_sym = std::make_unique<VBDL_tFont>(*c, text_data.font, str_x0, text_y0, color, options, (text_data.align & 0xF0) | LEFT_ALIGN);
	
				if (new_sym->handle != NULL)
				{
					str_x0 += new_sym->pos.x1 - new_sym->pos.x0 + 1 + str_char_space;
			
					if (out_buf != NULL) internal_draw_obj::another_obj_fill_str_init(*out_buf, std::move(new_sym), mask, layer_options);
					else res_text_pos = VBDisplay::max_rect(new_sym->pos, res_text_pos);
				}
			}
		}
		else
		{
			if (char_space_string_n < text_data.char_space.size()) str_char_space = text_data.char_space[char_space_string_n++];
			text_y0 -= text_data.string_space;
			str_sym_cnt = 0;
		}
	}

	return res_text_pos;
}



void VBDL_tFontText::set_or_update_text(VBDisplay &display, uint32_t* layer_cnt_store, tFontText &text, int16_t init_text_x0, int16_t init_text_y0, uint32_t _color, uint8_t _options)
{
	set_or_update_text(display, layer_cnt_store, -1, text, init_text_x0, init_text_y0, _color, _options);
}
uint32_t VBDL_tFontText::set_or_update_text(VBDisplay &display, uint32_t* layer_num_handle, uint32_t desired_layer, tFontText &text, int16_t init_text_x0, int16_t init_text_y0, uint32_t _color, uint8_t _options)
{
	if (!text.text.size()) return display.set_or_update_obj(display.make_void_obj(), layer_num_handle, desired_layer);

	uint32_t disp_obj_count = display.obj_count();
	uint32_t layer = (layer_num_handle != NULL) ? (*layer_num_handle) : desired_layer;

	if (layer >= disp_obj_count)
	{
		layer = disp_obj_count;
		display.screen_buf_insert_obj(display.make_void_obj(), layer_num_handle);
	}

	uint8_t init = 0;
	int16_t y = -1;
	int16_t text_x0 = init_text_x0;
	int16_t text_y0 = init_text_y0;
	uint32_t string_n = 0;
	int16_t str_char_space = 0;
	
	rect old_rect = display.screen_buf_get_obj(layer)->pos;
	rect common_new_rect;

	const wchar_t* c2;
	for (const wchar_t* c =  text.text.data(); *c != '\0'; c = c2, string_n++)
	{
		uint32_t str_sym_cnt = 0;
		for (c2 = c; *c2 != '\0' && *c2 != '\n'; c2++) str_sym_cnt++;
		if (*c2 == '\n') c2++;

		if (string_n < text.char_space.size()) str_char_space = text.char_space[string_n];

		rect new_rect = get_line_common_rect(text.font, str_char_space, c, str_sym_cnt);
		new_rect = VBDisplay::get_rect(text_x0, text_y0, new_rect.x1, new_rect.y1, text.align);

		rect dummy_rect = old_rect;
		if (init) clamp_max(dummy_rect.y1, y);
		if (*c2 != '\0') clamp_min(dummy_rect.y0, new_rect.y0);

		draw_tFont_string(display, layer, dummy_rect, text.font, str_char_space, c, str_sym_cnt, new_rect.x0, new_rect.y0, _color, _options, text.align);

		y = new_rect.y0 - 1;
		text_y0 -= text.string_space;

		if (!init)
		{
			init = 1;
			common_new_rect = new_rect;
		}
		else common_new_rect = VBDisplay::max_rect_no_valid_check(common_new_rect, new_rect);
	}

	std::unique_ptr<VBDL_tFontText> new_text_obj = std::make_unique<VBDL_tFontText>();
	new_text_obj->text_data = text;
	new_text_obj->x0 = init_text_x0;
	new_text_obj->y0 = init_text_y0;
	new_text_obj->color = _color;
	new_text_obj->options = _options;
	new_text_obj->pos = common_new_rect;

	display.screen_buf_update_obj(std::move(new_text_obj), layer);

	return layer + 1;
}

void VBDL_tFontText::draw_tFont_string(VBDisplay &display, uint32_t layer, rect old_rect, tFont font, int16_t char_space, const wchar_t* str, uint32_t sym_cnt, int16_t text_x0, int16_t text_y0, uint32_t color, uint8_t options, uint8_t align)
{
	if (!sym_cnt) char_space = 0;

	uint32_t additional_obj_layer = (uint32_t)(-1);
	if (char_space < 0) display.screen_buf_insert_obj(display.make_void_obj(), &additional_obj_layer);

	int16_t x = -1;
	int16_t internal_text_x = text_x0;

	for (const wchar_t *c = str; *c != '\0' && sym_cnt; c++, sym_cnt--)
	{
		VBDL_tFont sym(*c, font, internal_text_x, text_y0, color, options, BOTTOM_ALIGN | LEFT_ALIGN);
		if (sym.handle == NULL) continue;

		rect dummy_rect = old_rect;

		clamp_min(dummy_rect.x0, x);
		clamp_max(dummy_rect.x1, sym.pos.x1);

		x = sym.pos.x1 + 1;
		internal_text_x = x + char_space;

		std::unique_ptr<draw_obj> cur_sym = std::make_unique<VBDL_tFont>(sym);
		display.screen_buf_update_obj(std::move(cur_sym), layer);

		VBDL_InternalPushLayer::update_area_on_screen(display, dummy_rect, sym.pos, layer);
		
		if (char_space < 0)
		{
			std::unique_ptr<draw_obj> prev_sym = std::make_unique<VBDL_tFont>(sym);
			display.screen_buf_update_obj(std::move(prev_sym), additional_obj_layer);
		}
	}

	rect dummy2_rect = old_rect;
	clamp_min(dummy2_rect.x0, x);
	VBDL_InternalPushLayer::update_area_on_screen(display, dummy2_rect, display.void_rect, layer);

	if (char_space < 0) display.screen_buf_delete_obj(additional_obj_layer);
}
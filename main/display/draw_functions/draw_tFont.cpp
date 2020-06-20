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

inline uint8_t conv_data_color(uint8_t data_color, uint8_t options)
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


void tFont_str_init(internal_draw_obj* img)
{
	if (img->options&DRAW_INV_COLOR) img->user_color = invert_color(img->user_color);
	
#if LCD_INV_BRIGHTNESS
	img->user_color = (img->user_color & 0xFF000000) | ((0xFFFFFFFF - img->user_color) & 0xFFFFFF);
#endif
	
	uint32_t com_img_bias = calc_start_pos(img->img_cur_pos, img->img_cur_y, img->img_width_elem, 1);
  
	img->img_cur_pos = 0;
	uint8_t data_color = 0;
  
	uint8_t* sprite_data = (uint8_t*)(img->handle);
  
	for (uint32_t i = 0; i < com_img_bias; i++)
	{
		if (!img->img_cur_pos)
		{
			img->img_cur_pos = *sprite_data++;
			img->handle = (void*)sprite_data;
			
			if (img->img_cur_pos < 0x80)
			{
				data_color = *sprite_data++;
				img->handle = (void*)sprite_data;
			}
		}
		
		if (img->img_cur_pos >= 0x80)
		{
			img->img_cur_pos++;
			if (img->img_cur_pos == 256) img->img_cur_pos = 0;
			
			data_color = *sprite_data++;
			img->handle = (void*)sprite_data;
		}
		else img->img_cur_pos--;
	}
  
	union {uint32_t full; uint16_t dc[2]; } mixdata;
	
	uint32_t trans = img->user_color >> 24;
	
	data_color = conv_data_color(data_color, img->options);
		
	mixdata.dc[1] = ((trans * 255 + (data_color * (255 - trans))) * 256) / (255 * 255);
	mixdata.dc[0] = 256 - mixdata.dc[1];     //256...0
	img->user_data = mixdata.full;
}


void tFont_str_memcpy(uint8_t* buf, internal_draw_obj* img)
{
	uint8_t* sprite_data = (uint8_t*)(img->handle);

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
			img->handle = (void*)sprite_data;
			
			if (img->img_cur_pos < 0x80)
			{
				data_color = *sprite_data++;
				img->handle = (void*)sprite_data;
				
				uint32_t trans = spr_color.color[3];
				
				data_color = conv_data_color(data_color, img->options);
				
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
				img->handle = (void*)sprite_data;
				
				uint8_t trans = spr_color.color[3];
				
				data_color = conv_data_color(data_color, img->options);
				
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


void tFont_str_memclear(internal_draw_obj* img)
{
}


draw_obj make_tFont_symbol(wchar_t c, tFont font, int16_t x, int16_t y, uint32_t color, uint8_t options, uint8_t align)
{
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
			if (i == font.length) return make_void_obj();
		
			if (font_chars[i].code == c) break;
		}
	}
	
	past_c = c;
	past_i = i;
	
	t_char = font_chars[i].image;
	
	draw_obj res;
  
	res.pos = get_rect(x, y, t_char->width, t_char->height, align);
	res.obj_type = TFONT;
	res.handle = (void*)t_char->data;
	res.color = color;
	res.options = options;
	
	return res;
}







rect get_tFont_string_common_rect(tFont font, int16_t char_space, const wchar_t* str)
{
	draw_obj sym;
	rect res;
	int16_t sym_x0 = res.x0 = res.y0 = 1;
	res.x1 = res.y1 = 0;

	for (const wchar_t *c = str; *c != '\0'; c++)
	{
		sym = make_tFont_symbol(*c, font, sym_x0, 0, 0, 0, LEFT_ALIGN | BOTTOM_ALIGN);

		res.x1 = sym.pos.x1;
		res.y1 = max1(res.y1, sym.pos.y1);

		sym_x0 = res.x1 + 1 + char_space;
	}

	return res;
}

void draw_tFont_string(draw_obj_list draw_buffer, uint8_t layer, tFont font, int16_t char_space, const wchar_t* str, int16_t text_x0, int16_t text_y0, uint32_t color, uint8_t options, uint8_t align)
{
	if (layer >= draw_buffer.elem_cnt) return;
	
	draw_obj sym;
	uint8_t internal_align = (align & 0xF0) | LEFT_ALIGN;
	rect new_rect;

	if (str[0] == '\0')
	{
		delete_img(draw_buffer, layer);
		return;
	}

	if (char_space < 0) draw_buffer.obj[draw_buffer.elem_cnt - 1] = make_void_obj();

	if (((align & 0x0F) == RIGHT_ALIGN) || ((align & 0x0F) == CENTER_ALIGN))
	{
		new_rect = get_tFont_string_common_rect(font, char_space, str);
		text_x0 -= calc_hor_offset(new_rect.x1, align);
	}


	rect old_rect = draw_buffer.obj[layer].pos;
	uint8_t init = 0;
	int16_t x = text_x0;
	int16_t internal_text_x = text_x0;

	for (const wchar_t *c = str; *c != '\0'; c++)
	{
		sym = make_tFont_symbol(*c, font, internal_text_x, text_y0, color, options, internal_align);
		if (sym.handle == NULL) continue;

		draw_buffer.obj[layer].pos = old_rect;

		if (!init)
		{
			init = 1;

			new_rect = sym.pos;
		}
		else
		{
			if (sym.pos.x1 < x)
			{
				delete_img(draw_buffer, layer);
				draw_buffer.obj[layer].pos = new_rect;
				delete_img(draw_buffer, layer);
				return;
			}

			clamp_min(draw_buffer.obj[layer].pos.x0, x);

			new_rect = internal_max_rect(new_rect, sym.pos);
		}

		clamp_max(draw_buffer.obj[layer].pos.x1, sym.pos.x1);

		_update_img(draw_buffer, layer, &sym);

		x = sym.pos.x1 + 1;
		internal_text_x = x + char_space;

		if (char_space < 0) draw_buffer.obj[draw_buffer.elem_cnt - 1] = sym;
	}

	if (char_space < 0) draw_buffer.obj[draw_buffer.elem_cnt - 1] = make_void_obj();

	draw_buffer.obj[layer].pos = old_rect;
	clamp_min(draw_buffer.obj[layer].pos.x0, x);

	delete_img(draw_buffer, layer);

	draw_buffer.obj[layer].pos = new_rect;
	draw_buffer.obj[layer].obj_type = TVOID;
}

void draw_tFont_strings(draw_obj_list draw_buffer, uint8_t layer, tFont font, int16_t char_space, int16_t string_space, const wchar_t** str, uint8_t str_cnt, int16_t text_x0, int16_t text_y0, uint32_t color, uint8_t options, uint8_t align)
{
	if (layer >= draw_buffer.elem_cnt) return;
	
	rect old_rect = draw_buffer.obj[layer].pos;
	rect common_new_rect, new_rect;
	uint8_t internal_align = BOTTOM_ALIGN | LEFT_ALIGN;

	uint8_t init = 0;
	int16_t y = -1;

	for (uint8_t i = 0; i < str_cnt; i++)
	{
		draw_buffer.obj[layer].pos = old_rect;

		new_rect = get_tFont_string_common_rect(font, char_space, str[i]);
		new_rect = get_rect(text_x0, text_y0, new_rect.x1, new_rect.y1, align);

		if (init) clamp_max(draw_buffer.obj[layer].pos.y1, y);
		if (i < str_cnt - 1) clamp_min(draw_buffer.obj[layer].pos.y0, new_rect.y0);

		draw_tFont_string(draw_buffer, layer, font, char_space, str[i], new_rect.x0, new_rect.y0, color, options, internal_align);

		y = new_rect.y0 - 1;
		text_y0 -= string_space;

		if (!init)
		{
			init = 1;

			common_new_rect = new_rect;
		}
		else common_new_rect = internal_max_rect(common_new_rect, new_rect);
	}

	draw_buffer.obj[layer].pos = common_new_rect;
	draw_buffer.obj[layer].obj_type = TVOID;
}




uint8_t draw_tFont_string_separate_sprites(draw_obj_list draw_buffer, uint8_t layer, uint8_t max_symbol_cnt, tFont font, int16_t char_space, const wchar_t* str, int16_t text_x0, int16_t text_y0, uint32_t color, uint8_t options, uint8_t align)
{
	if ((layer + max_symbol_cnt) > draw_buffer.elem_cnt) return 0;
	
	draw_obj sym;
	uint8_t internal_align = (align & 0xF0) | LEFT_ALIGN;
	uint8_t res = 0;
	int16_t res_x1;

	if (((align & 0x0F) == RIGHT_ALIGN) || ((align & 0x0F) == CENTER_ALIGN))
	{
		sym.pos = get_tFont_string_common_rect(font, char_space, str);
		text_x0 -= calc_hor_offset(sym.pos.x1, align);
	}
	
	res_x1 = text_x0;

	const wchar_t* c = str;
	for (uint8_t i = 0; i < max_symbol_cnt; i++)
	{
		if (*c != '\0')
		{
			sym = make_tFont_symbol(*c++, font, text_x0, text_y0, color, options, internal_align);
			if (sym.handle == NULL) continue;
	        
			res_x1 = sym.pos.x1;
			text_x0 = res_x1 + 1 + char_space;

			res++;
			
			_update_img(draw_buffer, layer + i, &sym);
		}
		else
		{
			sym = make_void_obj();
			_update_img(draw_buffer, layer + i, &sym);
			
			draw_buffer.obj[layer + i].pos.x0 = res_x1 + 1;
			draw_buffer.obj[layer + i].pos.x1 = res_x1;
		}
	}

	return res;
}




rect internal_tFont_string_char_space(tFont font, int16_t *char_space, const wchar_t* str, int16_t max_length)
{
	int16_t char_cnt = 0;
	for (const wchar_t* c = str; *c != '\0'; c++) char_cnt++;
	
	rect check_rect = get_tFont_string_common_rect(font, *char_space, str);
	int16_t text_length = check_rect.x1;
	
	if (text_length > max_length)
	{
		if (char_cnt > 1)
		{
			char_cnt--;
			
			int16_t diff_length = text_length - max_length;
			
			int16_t temp = diff_length / char_cnt;
			if (diff_length % char_cnt) temp++;
			
			*char_space -= temp;
			text_length -= temp*char_cnt;
		}
	}
	
	check_rect.x0 = 1;
	check_rect.x1 = text_length;
	
	return check_rect;
}

int16_t fit_tFont_string_char_space(tFont font, int16_t *char_space, const wchar_t* str, int16_t max_length)
{
	int16_t char_cnt = 0;
	for (const wchar_t* c = str; *c != '\0'; c++) char_cnt++;
	
	rect check_rect = get_tFont_string_common_rect(font, *char_space, str);
	int16_t text_length = check_rect.x1;
	
	if (text_length > max_length)
	{
		if (char_cnt > 1)
		{
			char_cnt--;
			
			int16_t diff_length = text_length - max_length;
			
			int16_t temp = diff_length / char_cnt;
			if (diff_length % char_cnt) temp++;
			
			*char_space -= temp;
			text_length -= temp*char_cnt;
		}
	}
	
	return text_length;
}



bool draw_fitted_tFont_string(draw_obj_list draw_buffer, uint8_t layer, tFont font, int16_t min_char_space, int16_t max_char_space, int16_t max_width, const wchar_t* str, int16_t text_x0, int16_t text_y0, uint32_t color, uint32_t options, uint8_t align)
{
	int16_t text_length = fit_tFont_string_char_space(font, &max_char_space, str, max_width);
	if (max_char_space < min_char_space) return 0;
	
	uint8_t internal_align = (align & 0xF0) | LEFT_ALIGN;
	draw_tFont_string(draw_buffer, layer, font, max_char_space, str, text_x0 - calc_hor_offset(text_length, align), text_y0, color, options, internal_align);
	
	return 1;
}


uint8_t draw_fitted_tFont_strings(draw_obj_list draw_buffer, uint8_t layer, tFont font, int16_t min_char_space, int16_t max_char_space, int16_t max_width, int16_t string_space, const wchar_t** str, uint8_t str_cnt, int16_t text_x0, int16_t text_y0, uint32_t color, uint32_t options, uint8_t align)
{
	if (layer >= draw_buffer.elem_cnt) return 0;
	
	uint8_t res = 0;
	
	rect old_rect = draw_buffer.obj[layer].pos;
	rect common_new_rect = old_rect, new_rect;
	uint8_t internal_align = BOTTOM_ALIGN | LEFT_ALIGN;

	uint8_t init = 0;
	int16_t y = -1;

	for (uint8_t i = 0; i < str_cnt; i++, res++)
	{
		int16_t char_space = max_char_space;
		new_rect = internal_tFont_string_char_space(font, &char_space, str[i], max_width);
		
		if (char_space < min_char_space)
		{
			draw_buffer.obj[layer].pos = common_new_rect;
			delete_img(draw_buffer, layer);
			
			return 0;
		}

		draw_buffer.obj[layer].pos = old_rect;
		new_rect = get_rect(text_x0, text_y0, new_rect.x1, new_rect.y1, align);

		if (init) clamp_max(draw_buffer.obj[layer].pos.y1, y);
		if (i < str_cnt - 1) clamp_min(draw_buffer.obj[layer].pos.y0, new_rect.y0);

		draw_tFont_string(draw_buffer, layer, font, char_space, str[i], new_rect.x0, new_rect.y0, color, options, internal_align);

		y = new_rect.y0 - 1;
		text_y0 -= string_space;

		if (!init)
		{
			init = 1;

			common_new_rect = new_rect;
		}
		else common_new_rect = internal_max_rect(common_new_rect, new_rect);
	}

	draw_buffer.obj[layer].pos = common_new_rect;
	draw_buffer.obj[layer].obj_type = TVOID;
	
	return res;
}
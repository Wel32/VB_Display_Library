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
	
	draw_obj res = make_void_obj();
	res.pos = get_rect(x, y, t_char->width, t_char->height, align);
	res.obj_type = TFONT;
	res.handle = (void*)t_char->data;
	res.color = color;
	res.options = options;
	
	return res;
}



rect get_tFont_string_common_rect_v2(tFont font, int16_t char_space, const wchar_t* str, uint32_t max_sym_cnt)
{
	draw_obj sym;
	rect res;
	int16_t sym_x0 = res.x0 = res.y0 = 1;
	res.x1 = res.y1 = 0;

	for (const wchar_t *c = str; *c != '\0' && max_sym_cnt; c++, max_sym_cnt--)
	{
		sym = make_tFont_symbol(*c, font, sym_x0, 1, 0, 0, LEFT_ALIGN | BOTTOM_ALIGN);

		res.x1 = sym.pos.x1;
		res.y1 = max1(res.y1, sym.pos.y1);

		sym_x0 = res.x1 + 1 + char_space;
	}

	return res;
}

rect internal_tFont_string_char_space_v2(tFont font, int16_t *char_space, const wchar_t* str, int16_t max_length, size_t max_char_cnt)
{
	int16_t char_cnt = 0;
	for (const wchar_t* c = str; *c != '\0' && max_char_cnt; c++, max_char_cnt--) char_cnt++;
	
	rect check_rect = get_tFont_string_common_rect_v2(font, *char_space, str, char_cnt);
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



uint32_t alloc_tFont_symbols(draw_obj** buf, tFont_text* text, int16_t obj_x0, int16_t obj_y0, uint32_t color, uint8_t options, bool get_common_rect_only)
{
	draw_obj new_sym = make_void_obj();
	bool new_str = 1;
	rect sym_pos = make_void_rect(), res_text_pos;
	int16_t text_x = obj_x0, text_y = obj_y0;

	uint32_t total_sym_cnt = 0;
	uint32_t real_sym_cnt = 0;
	uint32_t str_sym_cnt = 0;

	uint8_t internal_align = (text->align & 0xF0) | LEFT_ALIGN;

	if (get_common_rect_only)
	{
		total_sym_cnt=1;
		res_text_pos = make_void_rect();
	}
	else
	{
		for (const wchar_t *c = text->text.data(); *c != '\0'; c++)
		{
			if (*c == '\n') continue;
			total_sym_cnt++;
		}
	}

	*buf = new draw_obj[total_sym_cnt];
	draw_obj* buf_ptr1 = *buf;
	draw_obj* str_buf_ptr = *buf;

	if (get_common_rect_only) *str_buf_ptr = new_sym;

	uint32_t string_n = 0;
	int16_t str_char_space = 0;

	if (text->char_space.size()) str_char_space = text->char_space[0];

	for (const wchar_t *c = text->text.data(); ; c++, real_sym_cnt++)
	{
		while (*c == '\n' || *c == '\0')
		{
			if (!new_str)
			{
				new_str = 1;
				string_n++;
				if (string_n < text->char_space.size()) str_char_space = text->char_space[string_n];

				int16_t hor_offs = calc_hor_offset(sym_pos.x1 - sym_pos.x0 + 1, text->align);

				if (!get_common_rect_only)
				{
					for (uint32_t i = 0; i < str_sym_cnt; i++, str_buf_ptr++)
					{
						str_buf_ptr->pos.x0 -= hor_offs;
						str_buf_ptr->pos.x1 -= hor_offs;
					}

					str_sym_cnt = 0;
					str_buf_ptr = buf_ptr1;
				}
				else
				{
					sym_pos.x0 -= hor_offs;
					sym_pos.x1 -= hor_offs;

					res_text_pos = max_rect(sym_pos, res_text_pos);
					str_buf_ptr->pos = res_text_pos;
				}

				text_x = obj_x0;
				text_y -= text->string_space;
			}

			if (*c == '\0') return real_sym_cnt;
			c++;
			if (*c == '\0') return real_sym_cnt;
			
			continue;
		}
		if (text_x<0) text_x = 200;
		new_sym = make_tFont_symbol(*c, text->font, text_x, text_y, color, options, internal_align);
	
		if (new_sym.obj_type != TVOID)
		{
			text_x += new_sym.pos.x1 - new_sym.pos.x0 + 1 + str_char_space;

			if (!new_str) sym_pos = internal_max_rect(sym_pos, new_sym.pos);
			else
			{
				new_str = 0;
				sym_pos = new_sym.pos;
			}
			
			if (!get_common_rect_only)
			{
				*buf_ptr1++ = new_sym;
				str_sym_cnt++;
			}
		}	
	}

	return real_sym_cnt;
}


uint32_t tFont_create_dyn_data_create(void** out_buf_v, void* in_buf_v, rect mask)
{
	draw_obj *in_buf = (draw_obj*)in_buf_v, **out_buf = (draw_obj**)out_buf_v;

	tFont_text* text = (tFont_text*)in_buf->handle;
	if (text == NULL) return 0;

	return alloc_tFont_symbols(out_buf, text, in_buf->x0, in_buf->y0, in_buf->color, in_buf->options, 0);
}

void draw_tFont_string_v2(draw_obj_list* draw_buffer, uint32_t layer, tFont font, int16_t char_space, const wchar_t* str, uint32_t sym_cnt, int16_t text_x0, int16_t text_y0, uint32_t color, uint8_t options, uint8_t align)
{
	draw_obj sym;
	uint8_t internal_align = (align & 0xF0) | LEFT_ALIGN;
	rect new_rect;

	if (char_space < 0) draw_buffer->obj.push_back(make_void_obj());

	if (((align & 0x0F) == RIGHT_ALIGN) || ((align & 0x0F) == CENTER_ALIGN))
	{
		new_rect = get_tFont_string_common_rect_v2(font, char_space, str, sym_cnt);
		text_x0 -= calc_hor_offset(new_rect.x1, align);
	}

	rect old_rect = draw_buffer->obj[layer].pos;
	uint8_t init = 0;
	int16_t x = text_x0;
	int16_t internal_text_x = text_x0;

	for (const wchar_t *c = str; *c != '\0' && sym_cnt; c++, sym_cnt--)
	{
		sym = make_tFont_symbol(*c, font, internal_text_x, text_y0, color, options, internal_align);
		if (sym.handle == NULL) continue;

		draw_buffer->obj[layer].pos = old_rect;

		if (!init)
		{
			init = 1;

			new_rect = sym.pos;
		}
		else
		{
			if (sym.pos.x1 < x)
			{
				hide_obj(draw_buffer, layer);
				draw_buffer->obj[layer].pos = new_rect;
				hide_obj(draw_buffer, layer);
				draw_buffer->obj[layer] = make_void_obj();
				return;
			}

			clamp_min(draw_buffer->obj[layer].pos.x0, x);

			new_rect = internal_max_rect(new_rect, sym.pos);
		}

		clamp_max(draw_buffer->obj[layer].pos.x1, sym.pos.x1);

		_update_obj(draw_buffer, layer, &sym);

		x = sym.pos.x1 + 1;
		internal_text_x = x + char_space;

		if (char_space < 0) draw_buffer->obj[draw_buffer->obj.size() - 1] = sym;
	}

	if (char_space < 0) draw_buffer->obj.erase(draw_buffer->obj.begin() + draw_buffer->obj.size() - 1);

	draw_buffer->obj[layer].pos = old_rect;
	clamp_min(draw_buffer->obj[layer].pos.x0, x);

	hide_obj(draw_buffer, layer);
	draw_buffer->obj[layer] = make_void_obj();

	draw_buffer->obj[layer].pos = new_rect;
	//draw_buffer->obj[layer].obj_type = TVOID;
}

void alloc_tfont_destruct(void* handle)
{
	tFont_text* alloc_tFont_text = (tFont_text*)handle;
	delete alloc_tFont_text;
}

void alloc_tfont_text(draw_obj_list* draw_buffer, draw_obj* obj, tFont_text* text)
{
	tFont_text* alloc_tFont_text = new tFont_text;
	obj->handle = (void*)alloc_tFont_text;

	alloc_data_with_destructor_t reg_data = {obj->handle, alloc_tfont_destruct};
	attach_alloc_data(draw_buffer, reg_data);

	*alloc_tFont_text = *text;
}

void set_tFont_text(draw_obj_list* draw_buffer, uint32_t* layer_cnt_store, tFont_text &text, int16_t init_text_x0, int16_t init_text_y0, uint32_t color, uint8_t options)
{
	uint32_t* layer_store_ptr;

	if (layer_cnt_store!=NULL) layer_store_ptr = layer_cnt_store;
	else layer_store_ptr = &(text.layer);

	if (*layer_store_ptr >= draw_buffer->obj.size())
	{
		draw_buffer->obj.push_back(make_void_obj());

		*layer_store_ptr = draw_buffer->obj.size() - 1;
	}

	uint32_t layer = *layer_store_ptr;
	
	if (!text.text.size())
	{
		delete_obj(draw_buffer, layer);
		return;
	}
	
	rect old_rect = draw_buffer->obj[layer].pos;
	rect common_new_rect, new_rect;
	uint8_t internal_align = BOTTOM_ALIGN | LEFT_ALIGN;

	uint8_t init = 0;
	int16_t y = -1;
	int16_t text_x0 = init_text_x0;
	int16_t text_y0 = init_text_y0;
	uint32_t string_n = 0;
	int16_t str_char_space = 0;

	const wchar_t* c2;
	for (const wchar_t* c =  text.text.data(); *c != '\0'; c = c2, string_n++)
	{
		uint32_t str_sym_cnt = 0;
		for (c2 = c; *c2 != '\0' && *c2 != '\n'; c2++) str_sym_cnt++;
		while (*c2 == '\n') c2++;

		if (string_n < text.char_space.size()) str_char_space = text.char_space[string_n];

		draw_buffer->obj[layer].pos = old_rect;

		new_rect = get_tFont_string_common_rect_v2(text.font, str_char_space, c, str_sym_cnt);
		new_rect = get_rect(text_x0, text_y0, new_rect.x1, new_rect.y1, text.align);

		if (init) clamp_max(draw_buffer->obj[layer].pos.y1, y);
		if (*c2 != '\0') clamp_min(draw_buffer->obj[layer].pos.y0, new_rect.y0);

		draw_tFont_string_v2(draw_buffer, layer, text.font, str_char_space, c, str_sym_cnt, new_rect.x0, new_rect.y0, color, options, internal_align);

		y = new_rect.y0 - 1;
		text_y0 -= text.string_space;

		if (!init)
		{
			init = 1;

			common_new_rect = new_rect;
		}
		else common_new_rect = internal_max_rect(common_new_rect, new_rect);
	}
	
	draw_buffer->obj[layer].layer_ptr = layer_store_ptr;
	draw_buffer->obj[layer].x0 = init_text_x0;
	draw_buffer->obj[layer].y0 = init_text_y0;
	draw_buffer->obj[layer].pos = common_new_rect;
	draw_buffer->obj[layer].obj_type = TFONT;
	draw_buffer->obj[layer].handle = NULL;
	draw_buffer->obj[layer].color = color;
	draw_buffer->obj[layer].options = options;
	draw_buffer->obj[layer].size = 0;
	draw_buffer->obj[layer].dynamic_data_constr = tFont_create_dyn_data_create;
	alloc_tfont_text(draw_buffer, &draw_buffer->obj[layer], &text);
}




draw_obj make_tFont_text(draw_obj_list* draw_buffer, tFont_text &text, int16_t x0, int16_t y0, uint32_t color, uint8_t options)
{
	draw_obj res = make_void_obj();
	draw_obj* out_buf_data = NULL;

	alloc_tFont_symbols(&out_buf_data, &text, x0, y0, color, options, 1);

	if (out_buf_data!=NULL)
	{
		res.x0 = x0;
		res.y0 = y0;
		res.pos = out_buf_data->pos;
		res.obj_type = TFONT;
		res.color = color;
		res.options = options;
		res.size = 0;
		res.dynamic_data_constr = tFont_create_dyn_data_create;
		alloc_tfont_text(draw_buffer, &res, &text);

		free(out_buf_data);
	}

	return res;
}

tFont_text* tFont_obj_access_to_text_param(draw_obj text_obj)
{
	if (text_obj.obj_type != TFONT || text_obj.handle == NULL || text_obj.dynamic_data_constr == NULL) return NULL;
	
	return (tFont_text*)text_obj.handle;
}


uint32_t tFont_autogamma_options(uint32_t bg_color, uint32_t f_color)
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


void tFont_text_set_char_space(tFont_text& text, int16_t min_char_space, int16_t max_char_space, int16_t place_length)
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

			rect check_rect = get_tFont_string_common_rect_v2(text.font, char_space, text.text.data() + str_beg_i, i - str_beg_i);
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
			if (char_space > min_char_space) char_space = max_char_space;

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


void tFont_text_line_autobreake(tFont_text& text, int16_t place_width, int16_t min_char_space, int16_t max_char_space, uint16_t max_lines_cnt)
{
	clamp_max(min_char_space, max_char_space);

	int16_t beg_char_space = max_char_space;
	std::vector <lbrk_text_word_info> text_words;

	lbrk_text_length_t total_length = { 0,0 };

	draw_obj space_char = make_tFont_symbol(L' ', text.font, 0, 0, 0, 0, LEFT_ALIGN | BOTTOM_ALIGN);
	int16_t sc_width = space_char.pos.x1 - space_char.pos.x0 + 1;

	lbrk_text_length_t text_space_length;
	text_space_length.beg = sc_width + beg_char_space;
	text_space_length.min = sc_width + min_char_space;

	size_t beg_i = 0;
	for (size_t i = 0; i <= text.text.size(); i++)
	{
		if (*(text.text.data() + i) == L'\n') return;

		if (*(text.text.data() + i) == L' ' || *(text.text.data() + i) == L'\0')
		{
			lbrk_text_length_t w_lengths = { 0,0 };

			if (i > beg_i)
			{
				rect temp_rect = get_tFont_string_common_rect_v2(text.font, beg_char_space, text.text.data() + beg_i, i - beg_i);
				w_lengths.beg = temp_rect.x1 - temp_rect.x0 + 1;
				temp_rect = get_tFont_string_common_rect_v2(text.font, min_char_space, text.text.data() + beg_i, i - beg_i);
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
		tFont_text_set_char_space(text, min_char_space, max_char_space, place_width);
		return;
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
		}
	}

	tFont_text_set_char_space(text, min_char_space, max_char_space, place_width);
}

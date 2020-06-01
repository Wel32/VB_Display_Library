#include "display_core.h"



uint8_t* hide_drawing_layers = NULL;

#if ORIENTATIONS_COUNT == 4 || ORIENTATIONS_COUNT == 2
uint16_t _ScreenWidth=DISPLAY_SHORT_SIDE_SIZE, _ScreenHeight=DISPLAY_LONG_SIDE_SIZE;
#endif

uint8_t ScreenOrient = 0;


uint8_t* string_alloc_buffer = NULL;





void alloc_additional_str_buffer()
{
	if (string_alloc_buffer == NULL) string_alloc_buffer = (uint8_t*) malloc(ScreenWidth*4);
}






#if ORIENTATIONS_COUNT == 4 || ORIENTATIONS_COUNT == 2

void init_display_orient()
{
	if (PortraitOrientation)
	{
		ScreenWidth = DISPLAY_SHORT_SIDE_SIZE;
		ScreenHeight = DISPLAY_LONG_SIDE_SIZE;
	}
	else
	{
		ScreenWidth = DISPLAY_LONG_SIDE_SIZE;
		ScreenHeight = DISPLAY_SHORT_SIDE_SIZE;
	}
}

#if ORIENTATIONS_COUNT == 4
uint8_t lcdChangeOrient(int8_t step_and_dir)
{
	return lcdSetOrient(cycle_through(ScreenOrient, 4, step_and_dir));
}
#else
uint8_t lcdChangeOrient()
{
	return lcdSetOrient((ScreenOrient) ? 0 : 1);
}
#endif


uint8_t lcdSetOrient(uint8_t orient)
{
	if (lcd_set_orient(orient))
	{
		ScreenOrient = orient;
		init_display_orient();
		
		return 1;
	}
	
	return 0;
}

#endif



uint32_t invert_color(uint32_t color)
{
	union {uint32_t full; uint8_t dc[4]; } temp_color;
	temp_color.full = color;
	uint8_t temp = temp_color.dc[0];
	temp_color.dc[0] = temp_color.dc[2];
	temp_color.dc[2] = temp;
	
	return temp_color.full;
}

uint32_t invert_brightness(uint32_t color)
{
	return (color & 0xFF000000) | ((0xFFFFFFFF - color) & 0xFFFFFF);
}

void lcdInternalSetRect(rect r)
{
	uint8_t data[4];

	data[0] = r.x0 >> 8;
	data[1] = r.x0 & 0xFF;
	data[2] = r.x1 >> 8;
	data[3] = r.x1 & 0xFF;

	lcd_send_cmd(0x2A);
	lcd_send_data(data, 4);

	data[0] = r.y0 >> 8;
	data[1] = r.y0 & 0xFF;
	data[2] = r.y1 >> 8;
	data[3] = r.y1 & 0xFF;

	lcd_send_cmd(0x2B);
	lcd_send_data(data, 4);

	lcd_send_cmd(0x2C);
}


void lcdSetRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint8_t align)
{
	rect r = get_rect(x, y, width, height, align);

	lcdInternalSetRect(r);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

rect get_rect(int16_t x0, int16_t y0, uint16_t width, uint16_t height, uint8_t align)
{
	clamp_max(width, 0x7FFF);
	clamp_max(height, 0x7FFF);

	rect res;

	if ((align & 0x0F) == RIGHT_ALIGN) x0 -= width;
	else if ((align & 0x0F) != LEFT_ALIGN) x0 -= width / 2;

	if ((align & 0xF0) == TOP_ALIGN) y0 -= height;
	else if ((align & 0xF0) != BOTTOM_ALIGN) y0 -= height / 2;

	res.x0 = x0;
	res.y0 = y0;

	res.x1 = x0 + width;
	res.y1 = y0 + height;

	if (res.x1 < res.x0) res.x1 = 0x7FFF;
	else res.x1--;

	if (res.y1 < res.y0) res.y1 = 0x7FFF;
	else res.y1--;

	return res;
}

bool crop_rect_within_screen(rect* r)
{
	if (r->x0 < 0)
	{
		if (r->x1 < 0) return false;
		r->x0 = 0;
	}

	if (r->x1 >= ScreenWidth)
	{
		if (r->x0 >= ScreenWidth) return false;
		r->x1 = ScreenWidth - 1;
	}

	if (r->y0 < 0)
	{
		if (r->y1 < 0) return false;
		r->y0 = 0;
	}

	if (r->y1 >= ScreenHeight)
	{
		if (r->y0 >= ScreenHeight) return false;
		r->y1 = ScreenHeight - 1;
	}

	return true;
}

bool check_rect_direction(rect* r)
{
	if (r->x1 < r->x0) return false;
	if (r->y1 < r->y0) return false;

	return true;
}

bool check_equal_rects(rect* r1, rect* r2)
{
	if (r1->x0 != r2->x0) return false;
	if (r1->x1 != r2->x1) return false;
	if (r1->y0 != r2->y0) return false;
	if (r1->y1 != r2->y1) return false;

	return true;
}



uint32_t calc_start_pos(uint32_t img_bias_pix, int16_t y0, uint16_t img_width_elem, uint8_t elem_per_pix)
{
	return ((y0 > 0) ? (((uint16_t)y0)*img_width_elem) : 0) + img_bias_pix*elem_per_pix;
}

uint32_t calc_start_pos_vertical_mirror(uint32_t img_bias_pix, int16_t y0, uint16_t img_width_elem, int16_t img_height, uint8_t elem_per_pix)
{
	if (y0 < 0) y0 = 0;
	
	img_height--;
	if (img_height < y0) return 0;
	
	return (img_height - y0)*img_width_elem + img_bias_pix*elem_per_pix;
}

bool check_horizontal_overlap(rect img_pos, rect mask)
{
	if (img_pos.x0 > mask.x1 && img_pos.x1 > mask.x1) return 0;
	if (img_pos.x0 < mask.x0 && img_pos.x1 < mask.x0) return 0;

	return 1;
}
bool check_vertical_overlap(rect img_pos, rect mask)
{
	if (img_pos.y0 > mask.y1 && img_pos.y1 > mask.y1) return 0;
	if (img_pos.y0 < mask.y0 && img_pos.y1 < mask.y0) return 0;

	return 1;
}


void common_str_init(internal_draw_obj* ids, rect img_pos, rect mask)
{
	int16_t temp;

	ids->buf_cnt_pix = 0;

	if (img_pos.y1 < img_pos.y0) return;
	if (!check_vertical_overlap(img_pos, mask)) return;

	int16_t width = min1(mask.x1, img_pos.x1) - max1(mask.x0, img_pos.x0) + 1;

	if (width <= 0) return; //check horizontal overelap

	temp = img_pos.x1 - img_pos.x0 + 1;
	if (temp <= 0) return; //img_width==0
	ids->img_width_elem = temp;  //=img_width_pix preset

	temp = img_pos.x0 - mask.x0;
	ids->buf_bias_elem = ((uint16_t)((temp > 0) ? temp : 0)) * 3;

	ids->buf_cnt_pix = width;
	ids->img_cur_y = temp = mask.y0 - img_pos.y0;
	ids->img_height = img_pos.y1 - img_pos.y0 + 1;

	temp = mask.x0 - img_pos.x0;
	ids->img_cur_pos = (uint32_t)((temp > 0) ? temp : 0);  //=img_bias_pix preset
}



#ifdef ENABLE_FRAGMEMT_DRAWING
void internal_common_draw(draw_obj_list draw_buffer, rect mask)
#else
void common_draw(draw_obj_list draw_buffer, rect mask)
#endif
{
	if (!check_rect_direction(&mask)) return;
    if(!crop_rect_within_screen(&mask)) return;

	draw_obj* img_list = draw_buffer.obj;
	uint8_t layers = draw_buffer.elem_cnt;

	internal_draw_obj draw_list[MAX_LAYERS_TO_BLENDING], *pdrl;

	int16_t mask_width = mask.x1 - mask.x0 + 1;
	clamp_min(mask_width, 0);
	uint16_t mask_width_elem = ((uint16_t)mask_width) * 3;

	pdrl = draw_list;
	
	bool hidden_drawing = 1;
	uint8_t internal_hide_drawing_layers = (hide_drawing_layers != NULL) ? (*hide_drawing_layers) : 1;

	uint8_t draw_layers = 0;
	for (uint8_t i = 0; i < layers; i++, img_list++)
	{
		if (i >= internal_hide_drawing_layers) hidden_drawing = 0;
		
		if ( ((uint8_t)((img_list->color)>>24)) == 255 || img_list->obj_type >= TVOID) continue;

		common_str_init(pdrl, img_list->pos, mask);

		if (pdrl->buf_cnt_pix == 0) continue;
		
		pdrl->handle = img_list->handle;
		pdrl->obj_type = img_list->obj_type;
		pdrl->options = img_list->options;
		pdrl->user_color = img_list->color;
		
		pdrl->layer_options = 0;
		if (draw_layers == 0) pdrl->layer_options SETBITS LAYER_OPTIONS_FIRST_LAYER;
		if (layers == 1) pdrl->layer_options SETBITS LAYER_OPTIONS_SINGLE_LAYER;

		(*str_init_func[img_list->obj_type])(pdrl);

		if (pdrl->obj_type >= TVOID) continue;

		pdrl++;
		draw_layers++;
		if (draw_layers == MAX_LAYERS_TO_BLENDING) break;
	}

	if (!draw_layers) return;

	if (hidden_drawing) lcdImmediatelyOff();  //lcdOff();
	
#if ORIENTATIONS_COUNT == 4 || ORIENTATIONS_COUNT == 2
		uint8_t pixel_buf[max1(DISPLAY_LONG_SIDE_SIZE, DISPLAY_SHORT_SIDE_SIZE) * 3];
	#if LCD_SPI_ENABLE_DMA
		uint8_t pixel_buf2[max1(DISPLAY_LONG_SIDE_SIZE, DISPLAY_SHORT_SIDE_SIZE) * 3];
	#endif
#else
		uint8_t pixel_buf[(DISPLAY_WIDTH)* 3];
	#if LCD_SPI_ENABLE_DMA
		uint8_t pixel_buf2[(DISPLAY_WIDTH)* 3];
	#endif
#endif
	

	uint8_t* pb_ptr = pixel_buf;

	lcdInternalSetRect(mask);
	lcd_start_transfer(LCD_SEL_DATA);

	for (int16_t y = mask.y0; y <= mask.y1; y++)
	{
		for (uint8_t i = 0; i < draw_layers; i++)
		{
			pdrl = &draw_list[i];

			if ((pdrl->img_cur_y >= 0) && (pdrl->img_cur_y < pdrl->img_height))
			{
				(*str_memcpy_func[draw_list[i].obj_type])(pb_ptr + (pdrl->buf_bias_elem), pdrl);
			}

			pdrl->img_cur_y++;
		}

		lcd_send_part_of_data(pb_ptr, mask_width_elem);

#if LCD_SPI_ENABLE_DMA
		pb_ptr = (pb_ptr == pixel_buf) ? pixel_buf2 : pixel_buf;
#endif
	}

	lcd_end_transfer();

	if (hidden_drawing) lcdOn();
	
	for (uint8_t i = 0; i < draw_layers; i++)
	{
		if (str_memclear_func[draw_list[i].obj_type] != NULL) (*str_memclear_func[draw_list[i].obj_type])(&draw_list[i]);
	}
	
	if (string_alloc_buffer != NULL)
	{
		free(string_alloc_buffer);
		string_alloc_buffer = NULL;
	}
}


#ifdef ENABLE_FRAGMEMT_DRAWING
void common_draw(draw_obj_list draw_buffer, rect mask)
{
	if (!check_rect_direction(&mask)) return;
	
	if (draw_buffer.end_fragments_layer <= draw_buffer.start_fragments_layer)
	{
		internal_common_draw(draw_buffer, mask);
		return;
	}
	
	for (uint8_t layer = draw_buffer.start_fragments_layer; layer < draw_buffer.end_fragments_layer; layer++)
	{
		if (!check_rect_direction(&draw_buffer.obj[layer].pos))
		{
			internal_common_draw(draw_buffer, mask);
			return;
		}
		
		rect part_mask = internal_min_rect(draw_buffer.obj[layer].pos, mask);
		internal_common_draw(draw_buffer, part_mask);
	}
}
#endif


void redraw_all(draw_obj_list draw_buffer)
{
	uint8_t total_elem_cnt = draw_buffer.elem_cnt;

	for (uint8_t i = 0; i < total_elem_cnt; i++)
	{
		draw_buffer.elem_cnt = i + 1;
		common_draw(draw_buffer, draw_buffer.obj[i].pos);
	}
}


void redraw_group(draw_obj_list draw_buffer, uint8_t first_ayer, uint8_t end_layer)
{
	if (first_ayer >= draw_buffer.elem_cnt || end_layer >= draw_buffer.elem_cnt) return;
	
	uint8_t draw_buf_end_layer = draw_buffer.elem_cnt;

	for (uint8_t i = first_ayer; i < end_layer; i++)
	{
		if (end_layer == draw_buf_end_layer) draw_buffer.elem_cnt = i + 1;
		common_draw(draw_buffer, draw_buffer.obj[i].pos);
	}
}




rect make_void_rect()
{
	rect res;

	res.x0 = -1;
	res.x1 = -2;
	res.y0 = -1;
	res.y1 = -2;

	return res;
}


draw_obj make_void_obj()
{
	draw_obj res;

	res.pos = make_void_rect();
	res.obj_type = TVOID;
	res.handle = NULL;
	res.color = DELETE_IMG_COLOR;

	return res;
}



void clear_screen_data(draw_obj_list* dl)
{
	for (uint8_t i = 0; i < dl->elem_cnt; i++) dl->obj[i] = make_void_obj();
	
#ifdef ENABLE_FRAGMEMT_DRAWING
	dl->start_fragments_layer = 0;
	dl->end_fragments_layer = 0;
#endif
}


void draw_img(draw_obj_list draw_buffer, uint8_t img_layer)
{
	if (img_layer >= draw_buffer.elem_cnt) return;
	
	common_draw(draw_buffer, draw_buffer.obj[img_layer].pos);
}


void delete_img(draw_obj_list draw_buffer, uint8_t img_layer)
{
	if (img_layer >= draw_buffer.elem_cnt) return;
	
	draw_buffer.obj[img_layer].color = DELETE_IMG_COLOR;
	common_draw(draw_buffer, draw_buffer.obj[img_layer].pos);

	draw_buffer.obj[img_layer] = make_void_obj();
}

void hide_img(draw_obj_list draw_buffer, uint8_t img_layer)
{
	if (img_layer >= draw_buffer.elem_cnt) return;
	
	draw_buffer.obj[img_layer].color |= DELETE_IMG_COLOR;
	common_draw(draw_buffer, draw_buffer.obj[img_layer].pos);
}


void update_img(draw_obj_list draw_buffer, uint8_t img_layer, draw_obj new_img)
{
	_update_img(draw_buffer, img_layer, &new_img);
}



#ifdef ENABLE_FRAGMEMT_DRAWING
#define intrn_common_draw1(draw_buffer, del_mask) internal_common_draw(draw_buffer, del_mask)
#else
#define intrn_common_draw1(draw_buffer, del_mask) common_draw(draw_buffer, del_mask)
#endif

void _update_img(draw_obj_list draw_buffer, uint8_t img_layer, draw_obj* new_img)
{
	if (img_layer >= draw_buffer.elem_cnt) return;
	
	draw_obj* cur_img2 = &draw_buffer.obj[img_layer];

	if (check_rect_direction(&(cur_img2->pos)))
	{
		//Delete the image in whole or in part if the old and new images overlap//////////////////////
		rect part_mask, del_mask;

		cur_img2->color = DELETE_IMG_COLOR;
		
		
#ifdef ENABLE_FRAGMEMT_DRAWING
		
		uint8_t continue_cycle = 1;
		for (uint8_t layer = draw_buffer.start_fragments_layer; continue_cycle; layer++)
		{
			if (draw_buffer.end_fragments_layer <= draw_buffer.start_fragments_layer) 
			{
				part_mask = cur_img2->pos;
				continue_cycle = 0;
			}
			else if (!check_rect_direction(&draw_buffer.obj[layer].pos))
			{
				part_mask = cur_img2->pos;
				continue_cycle = 0;
			}
			else
			{
				if (layer >= draw_buffer.end_fragments_layer) break;
						
				part_mask = internal_min_rect(draw_buffer.obj[layer].pos, cur_img2->pos);
				
				if (!check_rect_direction(&part_mask)) continue;
			}
#else
		{
			
			part_mask = cur_img2->pos;
#endif
			
			int16_t temp_y0 = max1(part_mask.y0, min1(part_mask.y1, new_img->pos.y1) + 1);
			int16_t temp_y1 = min1(part_mask.y1, max1(part_mask.y0, new_img->pos.y0) - 1);
				
			//delete sect1//
			del_mask.x1 = part_mask.x1;
			del_mask.x0 = part_mask.x0;
			del_mask.y1 = temp_y1;
			del_mask.y0 = part_mask.y0;

			intrn_common_draw1(draw_buffer, del_mask);

			//delete sect3//
			del_mask.y1 = part_mask.y1;
			del_mask.y0 = temp_y0;

			intrn_common_draw1(draw_buffer, del_mask);

			//delete sect2
			del_mask.x1 = min1(part_mask.x1, new_img->pos.x0 - 1);
			//del_mask.x0 = part_mask.x0;
			del_mask.y1 = temp_y0 - 1;
			del_mask.y0 = temp_y1 + 1;

			intrn_common_draw1(draw_buffer, del_mask);

			//delete sect4
			del_mask.x1 = part_mask.x1;
			del_mask.x0 = max1(part_mask.x0, new_img->pos.x1 + 1);

			intrn_common_draw1(draw_buffer, del_mask);
		}
		
	}

	if (check_rect_direction(&(new_img->pos)))
	{
		//Update and drawing new image///////////////////
		*cur_img2 = *new_img;

		common_draw(draw_buffer, new_img->pos);
	}
}


void move_rect_to_new_xy(rect *r, int16_t new_x, int16_t new_y)
{
	int16_t width1 = r->x1 - r->x0;
	int16_t height1 = r->y1 - r->y0;

	r->x0 = new_x;
	r->x1 = new_x + width1;
	r->y0 = new_y;
	r->y1 = new_y + height1;

	if (!check_rect_direction(r)) *r = make_void_rect();
}

void move_rect_dxdy(rect *r, int16_t dx, int16_t dy)
{
	r->x0 += dx;
	r->x1 += dx;
	r->y0 += dy;
	r->y1 += dy;

	if (!check_rect_direction(r)) *r = make_void_rect();
}


void move_img_to_new_xy(draw_obj_list draw_buffer, uint8_t img_layer, int16_t new_x, int16_t new_y, uint8_t align)
{
	draw_obj* cur_img = &draw_buffer.obj[img_layer];
	draw_obj new_img = *cur_img;

    new_img.pos=get_rect(new_x, new_y, cur_img->pos.x1-cur_img->pos.x0+1, cur_img->pos.y1-cur_img->pos.y0+1, align);

	if (check_equal_rects(&new_img.pos, &cur_img->pos)) return;

	_update_img(draw_buffer, img_layer, &new_img);
}

void move_img_dxdy(draw_obj_list draw_buffer, uint8_t img_layer, int16_t dx, int16_t dy)
{
	if (dx == 0 && dy == 0) return;

	draw_obj* cur_img = &draw_buffer.obj[img_layer];
	draw_obj new_img = *cur_img;

	new_img.pos.x0 = cur_img->pos.x0 + dx;
	new_img.pos.x1 = cur_img->pos.x1 + dx;
	new_img.pos.y0 = cur_img->pos.y0 + dy;
	new_img.pos.y1 = cur_img->pos.y1 + dy;

	_update_img(draw_buffer, img_layer, &new_img);
}


/////////////////////////////////////////////////////////////////////

int16_t calc_hor_offset(int16_t width, uint8_t align)
{
	if ((align & 0x0F) == RIGHT_ALIGN) return width;
	else if ((align & 0x0F) != LEFT_ALIGN) return width / 2;

	return 0;
}


int16_t calc_ver_offset(int16_t height, uint8_t align)
{
	if ((align & 0xF0) == TOP_ALIGN) return height;
	else if ((align & 0xF0) != BOTTOM_ALIGN) return height / 2;

	return 0;
}

void align_group(draw_obj* draw_buffer, uint8_t first_layer, uint8_t end_layer, uint8_t align)
{
	if (end_layer <= first_layer) return;

	end_layer--;

	int16_t offset = calc_hor_offset(draw_buffer[end_layer].pos.x1 - draw_buffer[first_layer].pos.x0, align);

	if (offset)
	{
		for (uint8_t i = first_layer; i <= end_layer; i++)
		{
			draw_buffer[i].pos.x0 -= offset;
			draw_buffer[i].pos.x1 -= offset;
		}
	}
}



//////////DRAWING GROUPS/////////////////////////////////////

rect internal_max_rect(rect r1, rect r2)
{
	rect res;

	res.x0 = min1(r1.x0, r2.x0);
	res.y0 = min1(r1.y0, r2.y0);

	res.x1 = max1(r1.x1, r2.x1);
	res.y1 = max1(r1.y1, r2.y1);

	return res;
}

rect internal_min_rect(rect r1, rect r2)
{
	rect res;

	res.x0 = max1(r1.x0, r2.x0);
	res.y0 = max1(r1.y0, r2.y0);

	res.x1 = min1(r1.x1, r2.x1);
	res.y1 = min1(r1.y1, r2.y1);

	return res;
}

rect max_rect(rect r1, rect r2)
{
	if (!check_rect_direction(&r1)) return r2;
	if (!check_rect_direction(&r2)) return r1;

	return internal_max_rect(r1, r2);
}

rect min_rect(rect r1, rect r2)
{
	if (!check_rect_direction(&r1)) return make_void_rect();
	if (!check_rect_direction(&r2)) return make_void_rect();

	return internal_min_rect(r1, r2);
}

rect common_rect(draw_obj* scr_obj, uint8_t cnt)
{
	rect res;
	uint8_t i = 0;

	for (; i < cnt; i++)
	{
		res = scr_obj[i].pos;
		if (check_rect_direction(&res)) break;
	}

	i++;

	for (; i < cnt; i++)
	{
		if (!check_rect_direction(&scr_obj[i].pos)) continue;
		res = internal_max_rect(res, scr_obj[i].pos);
	}

	return res;
}






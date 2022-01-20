#include "display_core.h"



uint32_t* hide_drawing_layers = NULL;

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
	return lcdSetOrient(modulo_addition(ScreenOrient, step_and_dir, 4));
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



inline void lcdInternalSetRect(rect r)
{
	lcd_select_rect(r.x0, r.y0, r.x1, r.y1);
}


void lcdSetRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint8_t align)
{
	rect r = get_rect(x, y, width, height, align);
	lcd_select_rect(r.x0, r.y0, r.x1, r.y1);
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

bool _check_rect_direction(rect r)
{
	if (r.x1 < r.x0) return false;
	if (r.y1 < r.y0) return false;

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
void internal_common_draw(draw_obj_list* draw_buffer, rect mask, uint32_t end_layer)
#else
void common_draw(draw_obj_list* draw_buffer, rect mask, uint32_t end_layer)
#endif
{
	if (!check_rect_direction(&mask)) return;
    if(!crop_rect_within_screen(&mask)) return;

	uint32_t layers = min1(draw_buffer->obj.size(), end_layer);

	std::vector<draw_obj*> dynamic_obj_alloc;
	std::vector<internal_draw_obj> draw_list;

	int16_t mask_width = mask.x1 - mask.x0 + 1;
	clamp_min(mask_width, 0);
	uint16_t mask_width_elem = ((uint16_t)mask_width) * 3;
	
	uint32_t internal_hide_drawing_layers;
	
	if (hide_drawing_layers != NULL) internal_hide_drawing_layers = *hide_drawing_layers;
	else
	{
#ifdef ENABLE_FRAGMEMT_DRAWING
		if (draw_buffer->end_fragments_layer > draw_buffer->start_fragments_layer) internal_hide_drawing_layers = draw_buffer->end_fragments_layer;
		else
#endif
			internal_hide_drawing_layers = 1;
	}
			
	

	uint32_t draw_layers = 0;
	for (uint32_t i = 0; i < layers; i++)
	{
		draw_obj* img_list = draw_buffer->obj.data() + i;
		
		uint32_t obj_fragm_count = 1;
		
		if (img_list->dynamic_data_constr != NULL)
		{
			if (((uint8_t)((img_list->color) >> 24)) == 255 || img_list->obj_type >= TVOID) continue;

			if (check_vertical_overlap(img_list->pos, mask) && check_horizontal_overlap(img_list->pos, mask))
			{
				draw_obj temp = *img_list;
				void *in_buf_ptr = (void*)&temp;
				void *out_buf_data = NULL;
				void **out_buf_ptr = &out_buf_data;

				obj_fragm_count = img_list->dynamic_data_constr(out_buf_ptr, in_buf_ptr, mask);

				if (*out_buf_ptr!=NULL)
				{
					img_list = (draw_obj*) *out_buf_ptr;
					dynamic_obj_alloc.push_back(img_list);
				}
				else continue;
			}
			else continue;
		}
		
		for (uint32_t j = 0; j < obj_fragm_count; j++, img_list++)
		{
			if (((uint8_t)((img_list->color) >> 24)) == 255 || img_list->obj_type >= TVOID) continue;

			internal_draw_obj drl;

			common_str_init(&drl, img_list->pos, mask);

			if (drl.buf_cnt_pix == 0) continue;
		
			drl.handle = img_list->handle;
			drl.obj_type = img_list->obj_type;
			drl.options = img_list->options;
			drl.user_color = img_list->color;
		
			drl.layer_options = 0;
			if (draw_layers == 0) drl.layer_options SETBITS LAYER_OPTIONS_FIRST_LAYER;
			if (layers == 1) drl.layer_options SETBITS LAYER_OPTIONS_SINGLE_LAYER;

			(*str_init_func[img_list->obj_type])(&drl);

			if (drl.obj_type >= TVOID) continue;

			draw_list.push_back(drl);
			
			draw_layers++;
			if (draw_layers == MAX_LAYERS_TO_BLENDING) break;
		}
	}

	if (!draw_layers) return;

	if (layers <= internal_hide_drawing_layers) lcdImmediatelyOff();    //lcdOff();
	
	std::vector <uint8_t> pixel_buf(ScreenWidth * 3);
#if LCD_SPI_ENABLE_DMA
	std::vector <uint8_t> pixel_buf2(ScreenWidth * 3);
#endif
	

	uint8_t* pb_ptr = pixel_buf.data();

	lcdInternalSetRect(mask);
	lcd_start_transfer(LCD_SEL_DATA);

	for (int16_t y = mask.y0; y <= mask.y1; y++)
	{
		for (uint32_t i = 0; i < draw_layers; i++)
		{
			internal_draw_obj *pdrl = draw_list.data() + i;

			if ((pdrl->img_cur_y >= 0) && (pdrl->img_cur_y < pdrl->img_height))
			{
				(*str_memcpy_func[draw_list[i].obj_type])(pb_ptr + (pdrl->buf_bias_elem), pdrl);
			}

			pdrl->img_cur_y++;
		}

		lcd_send_part_of_data(pb_ptr, mask_width_elem);

#if LCD_SPI_ENABLE_DMA
		pb_ptr = (pb_ptr == pixel_buf.data()) ? pixel_buf2.data() : pixel_buf.data();
#endif
	}

	lcd_end_transfer();

	if (layers == internal_hide_drawing_layers) lcdOn();
	
	for (uint32_t i = 0; i < draw_layers; i++)
	{
		internal_draw_obj *pdrl = draw_list.data() + i;
		if (str_memclear_func[pdrl->obj_type] != NULL) (*str_memclear_func[pdrl->obj_type])(pdrl);
	}
	
	for (uint32_t i = 0; i < dynamic_obj_alloc.size(); i++) delete[] dynamic_obj_alloc[i];
	
	if (string_alloc_buffer != NULL)
	{
		free(string_alloc_buffer);
		string_alloc_buffer = NULL;
	}
}



#ifdef ENABLE_FRAGMEMT_DRAWING
void common_draw(draw_obj_list* draw_buffer, rect mask, uint32_t end_layer)
{
	if (!check_rect_direction(&mask)) return;
	
	if (draw_buffer->end_fragments_layer <= draw_buffer->start_fragments_layer)
	{
		internal_common_draw(draw_buffer, mask, end_layer);
		return;
	}
	
	for (uint32_t layer = draw_buffer->start_fragments_layer; layer < draw_buffer->end_fragments_layer; layer++)
	{
		rect part_mask = min_rect(draw_buffer->obj[layer].pos, mask);
		internal_common_draw(draw_buffer, part_mask, end_layer);
	}
}
#endif


void redraw_all(draw_obj_list* draw_buffer)
{
	uint32_t total_elem_cnt = draw_buffer->obj.size();

	for (uint32_t i = 0; i < total_elem_cnt; i++) common_draw(draw_buffer, draw_buffer->obj[i].pos, i+1);
}


void redraw_group_with_lower_layers_only(draw_obj_list* draw_buffer, uint32_t first_ayer, uint32_t end_layer)
{
	if (first_ayer >= draw_buffer->obj.size() || end_layer > draw_buffer->obj.size()) return;

	for (uint32_t i = first_ayer; i < end_layer; i++) common_draw(draw_buffer, draw_buffer->obj[i].pos, i+1);
}


void redraw_group(draw_obj_list* draw_buffer, uint32_t first_ayer, uint32_t end_layer)
{
	if (first_ayer >= draw_buffer->obj.size() || end_layer > draw_buffer->obj.size()) return;
	
	uint32_t draw_buf_end_layer = draw_buffer->obj.size();

	for (uint32_t i = first_ayer; i < end_layer; i++)
	{
		if (end_layer == draw_buffer->obj.size()) draw_buf_end_layer = i + 1;
		common_draw(draw_buffer, draw_buffer->obj[i].pos, draw_buf_end_layer);
	}
}


void redraw_objects_in_one_pass(draw_obj_list* draw_buffer, std::vector <uint32_t> &obj_layers_list, rect* add_rect)
{
	rect common_rect = make_void_rect();
	uint32_t layers_cnt = draw_buffer->obj.size();

	for (uint32_t i = 0; i<obj_layers_list.size(); i++)
	{
		if (obj_layers_list[i] <= layers_cnt) common_rect = max_rect(draw_buffer->obj[obj_layers_list[i]].pos, common_rect);
	}

	if (add_rect != NULL) common_rect = max_rect(*add_rect, common_rect);

	common_draw(draw_buffer, common_rect, -1);
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
	res.x0 = res.pos.x0;
	res.y0 = res.pos.y0;
	res.obj_type = TVOID;
	res.handle = NULL;
	res.color = DELETE_IMG_COLOR;
	res.size = 0;
	res.dynamic_data_constr = NULL;
	res.layer_ptr = NULL;

	return res;
}


bool check_valid_obj(draw_obj* obj)
{
	return obj->obj_type != TVOID;
}

void* push_alloc_data(draw_obj_list* draw_buffer, void* data, size_t size)
{
	alloc_data_with_destructor_t al_data;

	al_data.handle = malloc(size);
	al_data.destr_func = NULL;

	if (data!=NULL) memcpy(al_data.handle, data, size);

	draw_buffer->alloc_data_register.push_back(al_data);

	return al_data.handle;
}

void attach_alloc_data(draw_obj_list* draw_buffer, alloc_data_with_destructor_t data)
{
	if (data.handle!=NULL) draw_buffer->alloc_data_register.push_back(data);
}

void delete_alloc_data(draw_obj_list* draw_buffer, uint32_t layer)
{
	if (draw_buffer->obj[layer].handle !=NULL)
	{
		for (uint32_t i=0; i < draw_buffer->alloc_data_register.size(); i++)
		{
			if (draw_buffer->obj[layer].handle == draw_buffer->alloc_data_register[i].handle)
			{
				if (draw_buffer->alloc_data_register[i].destr_func != NULL) draw_buffer->alloc_data_register[i].destr_func(draw_buffer->alloc_data_register[i].handle);
				else free(draw_buffer->alloc_data_register[i].handle);

				draw_buffer->alloc_data_register.erase(draw_buffer->alloc_data_register.begin() + i);
				draw_buffer->obj[layer].handle = NULL;
			}
		}
	}
}

void clear_screen_data(draw_obj_list* draw_buffer)
{
	for (uint32_t i=0; i < draw_buffer->alloc_data_register.size(); i++)
	{
		if (draw_buffer->alloc_data_register[i].destr_func != NULL) draw_buffer->alloc_data_register[i].destr_func(draw_buffer->alloc_data_register[i].handle);
		else free(draw_buffer->alloc_data_register[i].handle);
	}

	draw_buffer->obj.clear();
	draw_buffer->alloc_data_register.clear();
	
#ifdef ENABLE_FRAGMEMT_DRAWING
	draw_buffer->start_fragments_layer = 0;
	draw_buffer->end_fragments_layer = 0;
#endif
}



void screen_buf_insert_obj(draw_obj_list* draw_buffer, draw_obj obj, uint32_t* layer_num_store, uint32_t layer)
{
	obj.layer_ptr = layer_num_store;

	if (layer >= draw_buffer->obj.size())
	{
		obj.layer_ptr = layer_num_store;
		draw_buffer->obj.push_back(obj);
		if (layer_num_store!=NULL) *layer_num_store = draw_buffer->obj.size()-1;

		return;
	}
	
	if (layer_num_store!=NULL) *layer_num_store = layer;

	draw_buffer->obj.emplace(draw_buffer->obj.begin() + layer, obj);

	for (uint32_t i=layer+1; i < draw_buffer->obj.size(); i++)
	{
		if (draw_buffer->obj[i].layer_ptr != NULL) *draw_buffer->obj[i].layer_ptr = *draw_buffer->obj[i].layer_ptr + 1;
	}
}

draw_obj* screen_buf_get_obj(draw_obj_list* draw_buffer, uint32_t layer)
{
	if (layer < draw_buffer->obj.size()) return draw_buffer->obj.data() + layer;
	return NULL;
}

void screen_buf_delete_obj(draw_obj_list* draw_buffer, uint32_t layer)
{
	if (layer < draw_buffer->obj.size())
	{
		delete_alloc_data(draw_buffer, layer);
		if (draw_buffer->obj[layer].layer_ptr != NULL) *draw_buffer->obj[layer].layer_ptr = 0xFFFFFFFF;

		draw_buffer->obj.erase(draw_buffer->obj.begin() + layer);
	}

	for (uint32_t i=layer; i < draw_buffer->obj.size(); i++)
	{
		if (draw_buffer->obj[i].layer_ptr != NULL) *draw_buffer->obj[i].layer_ptr = *draw_buffer->obj[i].layer_ptr - 1;
	}
}

void screen_buf_update_obj(draw_obj_list* draw_buffer, draw_obj obj, uint32_t layer)
{
	if (layer < draw_buffer->obj.size())
	{
		uint32_t *temp = draw_buffer->obj[layer].layer_ptr;
		delete_alloc_data(draw_buffer, layer);

		draw_buffer->obj[layer] = obj;
		draw_buffer->obj[layer].layer_ptr = temp;
	}
}

void set_or_update_obj(draw_obj_list* draw_buffer, draw_obj obj, uint32_t* layer_num_store, bool update_on_the_screen, bool update_obj_if_exist)
{
	uint32_t end_layer = draw_buffer->obj.size();
	uint32_t req_layer = (layer_num_store != NULL)?(*layer_num_store):end_layer;

	if (req_layer < end_layer && update_obj_if_exist)
	{
		if (update_on_the_screen) _update_obj(draw_buffer, *layer_num_store, &obj);
		else screen_buf_update_obj(draw_buffer, obj, *layer_num_store);
	}
	else
	{
		screen_buf_insert_obj(draw_buffer, obj, layer_num_store, req_layer);
		if (update_on_the_screen) common_draw(draw_buffer, draw_buffer->obj[draw_buffer->obj.size()-1].pos, 0xFFFFFFFF);
	}
}

void pin_layer_buffer_to_obj(draw_obj_list* draw_buffer, uint32_t obj_layer, uint32_t* layer_num_store)
{
	if (obj_layer < draw_buffer->obj.size())
	{
		draw_buffer->obj[obj_layer].layer_ptr = layer_num_store;
	}
}

void change_obj_layer(draw_obj_list* draw_buffer, uint32_t obj_layer, uint32_t new_layer)
{
	if (obj_layer>=draw_buffer->obj.size() || new_layer>=draw_buffer->obj.size() || obj_layer == new_layer) return;

	draw_buffer->obj.emplace(draw_buffer->obj.begin() + new_layer, draw_buffer->obj[obj_layer]);

	uint32_t erase_layer = obj_layer;
	if (obj_layer > new_layer) erase_layer++;
	draw_buffer->obj.erase(draw_buffer->obj.begin() + erase_layer);

	uint32_t start_layer = min1(obj_layer, new_layer), end_layer = max1(obj_layer, new_layer);
	for (uint32_t i=start_layer; i <= end_layer; i++)
	{
		if (draw_buffer->obj[i].layer_ptr != NULL) *draw_buffer->obj[i].layer_ptr = i;
	}
}

void draw_new_obj(draw_obj_list* draw_buffer, uint32_t img_layer)
{
	if (img_layer >= draw_buffer->obj.size()) return;
	
	common_draw(draw_buffer, draw_buffer->obj[img_layer].pos, 0xFFFFFFFF);
}


void delete_obj(draw_obj_list* draw_buffer, uint32_t img_layer)
{
	if (img_layer >= draw_buffer->obj.size()) return;
	
	draw_buffer->obj[img_layer].color = DELETE_IMG_COLOR;
	common_draw(draw_buffer, draw_buffer->obj[img_layer].pos, 0xFFFFFFFF);

	screen_buf_delete_obj(draw_buffer, img_layer);
}

void hide_obj(draw_obj_list* draw_buffer, uint32_t img_layer)
{
	if (img_layer >= draw_buffer->obj.size()) return;
	
	draw_buffer->obj[img_layer].color |= DELETE_IMG_COLOR;
	common_draw(draw_buffer, draw_buffer->obj[img_layer].pos, 0xFFFFFFFF);
}


void update_obj(draw_obj_list* draw_buffer, uint32_t img_layer, draw_obj new_img)
{
	_update_obj(draw_buffer, img_layer, &new_img);
}



#ifdef ENABLE_FRAGMEMT_DRAWING
#define intrn_common_draw1(draw_buffer, del_mask, end_layer) internal_common_draw(draw_buffer, del_mask, end_layer)
#else
#define intrn_common_draw1(draw_buffer, del_mask, end_layer) common_draw(draw_buffer, del_mask, end_layer)
#endif

void _update_obj(draw_obj_list* draw_buffer, uint32_t img_layer, draw_obj* new_img)
{
	if (img_layer >= draw_buffer->obj.size()) return;
	
	draw_obj* cur_img2 = draw_buffer->obj.data() + img_layer;
	if (cur_img2 == NULL) return;

	rect cur_img_rect = cur_img2->pos;
	if (check_rect_direction(&cur_img_rect))
	{
		//Delete the image in whole or in part if the old and new images overlap//////////////////////
		rect part_mask, del_mask;

		cur_img2->color = DELETE_IMG_COLOR;

	
		
#ifdef ENABLE_FRAGMEMT_DRAWING
		
		uint8_t continue_cycle = 1;
		for (uint32_t layer = draw_buffer->start_fragments_layer; continue_cycle; layer++)
		{
			if (draw_buffer->end_fragments_layer <= draw_buffer->start_fragments_layer) 
			{
				part_mask = cur_img2->pos;
				continue_cycle = 0;
			}
			else
			{
				if (layer >= draw_buffer->end_fragments_layer) break;
						
				part_mask = min_rect(draw_buffer->obj[layer].pos, cur_img2->pos);
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

			intrn_common_draw1(draw_buffer, del_mask, 0xFFFFFFFF);

			//delete sect3//
			del_mask.y1 = part_mask.y1;
			del_mask.y0 = temp_y0;

			intrn_common_draw1(draw_buffer, del_mask, 0xFFFFFFFF);

			//delete sect2
			del_mask.x1 = min1(part_mask.x1, new_img->pos.x0 - 1);
			//del_mask.x0 = part_mask.x0;
			del_mask.y1 = temp_y0 - 1;
			del_mask.y0 = temp_y1 + 1;

			intrn_common_draw1(draw_buffer, del_mask, 0xFFFFFFFF);

			//delete sect4
			del_mask.x1 = part_mask.x1;
			del_mask.x0 = max1(part_mask.x0, new_img->pos.x1 + 1);

			intrn_common_draw1(draw_buffer, del_mask, 0xFFFFFFFF);
		}
		
	}

	if (check_rect_direction(&(new_img->pos)))
	{
		//Update and drawing new image///////////////////
		screen_buf_update_obj(draw_buffer, *new_img, img_layer);

		common_draw(draw_buffer, new_img->pos, 0xFFFFFFFF);
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


void move_obj_to_new_xy(draw_obj_list* draw_buffer, uint32_t img_layer, int16_t new_x, int16_t new_y, uint8_t align, bool update_on_the_screen)
{
	draw_obj* cur_img = draw_buffer->obj.data() + img_layer;
	draw_obj new_img = *cur_img;

    new_img.pos=get_rect(new_x, new_y, cur_img->pos.x1-cur_img->pos.x0+1, cur_img->pos.y1-cur_img->pos.y0+1, align);
	new_img.x0 += new_x;
	new_img.y0 += new_y;

	if (!update_on_the_screen) return;

	if (check_equal_rects(&new_img.pos, &cur_img->pos)) return;

	_update_obj(draw_buffer, img_layer, &new_img);
}

void move_obj_dxdy(draw_obj_list* draw_buffer, uint32_t img_layer, int16_t dx, int16_t dy, bool update_on_the_screen)
{
	if (dx == 0 && dy == 0) return;

	draw_obj new_img = draw_buffer->obj[img_layer];

	new_img.x0 += dx;
	new_img.y0 += dy;
	new_img.pos.x0 += dx;
	new_img.pos.x1 += dx;
	new_img.pos.y0 += dy;
	new_img.pos.y1 += dy;

	if (!update_on_the_screen) return;

	_update_obj(draw_buffer, img_layer, &new_img);
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


void align_obj_group(draw_obj_list* draw_buffer, std::vector <uint32_t> &obj_layers_list, int16_t new_x, int16_t new_y, uint8_t align)
{
	rect common_rect = make_void_rect();
	uint32_t layers_cnt = draw_buffer->obj.size();

	for (uint32_t i = 0; i<obj_layers_list.size(); i++)
	{
		if (obj_layers_list[i] <= layers_cnt) common_rect = max_rect(draw_buffer->obj[obj_layers_list[i]].pos, common_rect);
	}

	int16_t dx = -calc_hor_offset(common_rect.x1-common_rect.x0+1, align);
	int16_t dy = -calc_ver_offset(common_rect.y1-common_rect.y0+1, align);

	for (uint32_t i = 0; i<obj_layers_list.size(); i++)
	{
		if (obj_layers_list[i] <= layers_cnt)
		{
			int16_t obj_dx = -draw_buffer->obj[obj_layers_list[i]].x0;
			int16_t obj_dy = -draw_buffer->obj[obj_layers_list[i]].y0;

			draw_buffer->obj[obj_layers_list[i]].x0 = new_x + dx;
			draw_buffer->obj[obj_layers_list[i]].y0 = new_y + dy;

			obj_dx += draw_buffer->obj[obj_layers_list[i]].x0;
			obj_dy += draw_buffer->obj[obj_layers_list[i]].y0;

			draw_buffer->obj[obj_layers_list[i]].pos.x0 += obj_dx;
			draw_buffer->obj[obj_layers_list[i]].pos.x1 += obj_dx;
			draw_buffer->obj[obj_layers_list[i]].pos.y0 += obj_dy;
			draw_buffer->obj[obj_layers_list[i]].pos.y1 += obj_dy;
		}
	}
}

void align_obj_group(std::vector <draw_obj*> &obj_list, int16_t new_x, int16_t new_y, uint8_t align)
{
	rect common_rect = make_void_rect();

	for (uint32_t i = 0; i<obj_list.size(); i++)
	{
		common_rect = max_rect(obj_list[i]->pos, common_rect);
	}

	int16_t dx = new_x - common_rect.x0 - calc_hor_offset(common_rect.x1-common_rect.x0+1, align);
	int16_t dy = new_y - common_rect.y0 - calc_ver_offset(common_rect.y1-common_rect.y0+1, align);

	for (uint32_t i = 0; i<obj_list.size(); i++)
	{
		obj_list[i]->x0 += dx;
		obj_list[i]->y0 += dy;

		obj_list[i]->pos.x0 += dx;
		obj_list[i]->pos.x1 += dx;
		obj_list[i]->pos.y0 += dy;
		obj_list[i]->pos.y1 += dy;
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

rect common_rect(draw_obj* scr_obj, uint32_t cnt)
{
	rect res;
	uint32_t i = 0;

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






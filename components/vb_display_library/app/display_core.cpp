#include "display_core.h"



uint32_t* hide_drawing_layers = NULL;

#if ORIENTATIONS_COUNT == 4 || ORIENTATIONS_COUNT == 2
uint16_t _ScreenWidth=DISPLAY_SHORT_SIDE_SIZE, _ScreenHeight=DISPLAY_LONG_SIDE_SIZE;
#endif

uint8_t ScreenOrient = 0;


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


uint32_t VBDisplay::invert_color(uint32_t color)
{
	union {uint32_t full; uint8_t dc[4]; } temp_color;
	temp_color.full = color;
	uint8_t temp = temp_color.dc[0];
	temp_color.dc[0] = temp_color.dc[2];
	temp_color.dc[2] = temp;
	
	return temp_color.full;
}

uint32_t VBDisplay::invert_brightness(uint32_t color)
{
	return (color & 0xFF000000) | ((0xFFFFFFFF - color) & 0xFFFFFF);
}

inline void VBDisplay::lcdInternalSetRect(rect r)
{
	lcd_select_rect(r.x0, r.y0, r.x1, r.y1);
}

rect VBDisplay::get_rect(int16_t x0, int16_t y0, uint16_t width, uint16_t height, uint8_t align)
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

bool VBDisplay::crop_rect_within_screen(rect* r)
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

bool VBDisplay::check_valid_rect(rect r)
{
	if (r.x1 < r.x0) return false;
	if (r.y1 < r.y0) return false;

	return true;
}
bool VBDisplay::check_valid_rect(rect* r)
{
	if (r->x1 < r->x0) return false;
	if (r->y1 < r->y0) return false;

	return true;
}

bool VBDisplay::check_equal_rects(rect r1, rect r2)
{
	if (r1.x0 != r2.x0) return false;
	if (r1.x1 != r2.x1) return false;
	if (r1.y0 != r2.y0) return false;
	if (r1.y1 != r2.y1) return false;

	return true;
}
bool VBDisplay::check_equal_rects(rect* r1, rect* r2)
{
	if (r1->x0 != r2->x0) return false;
	if (r1->x1 != r2->x1) return false;
	if (r1->y0 != r2->y0) return false;
	if (r1->y1 != r2->y1) return false;

	return true;
}

bool VBDisplay::check_horizontal_overlap(rect img_pos, rect mask)
{
	if (img_pos.x0 > mask.x1 && img_pos.x1 > mask.x1) return 0;
	if (img_pos.x0 < mask.x0 && img_pos.x1 < mask.x0) return 0;

	return 1;
}
bool VBDisplay::check_vertical_overlap(rect img_pos, rect mask)
{
	if (img_pos.y0 > mask.y1 && img_pos.y1 > mask.y1) return 0;
	if (img_pos.y0 < mask.y0 && img_pos.y1 < mask.y0) return 0;

	return 1;
}

void VBDisplay::move_rect_to_new_xy(rect *r, int16_t new_x, int16_t new_y)
{
	int16_t width1 = r->x1 - r->x0;
	int16_t height1 = r->y1 - r->y0;

	r->x0 = new_x;
	r->x1 = new_x + width1;
	r->y0 = new_y;
	r->y1 = new_y + height1;

	if (!check_valid_rect(r)) *r = void_rect;
}

void VBDisplay::move_rect_dxdy(rect *r, int16_t dx, int16_t dy)
{
	r->x0 += dx;
	r->x1 += dx;
	r->y0 += dy;
	r->y1 += dy;

	if (!check_valid_rect(r)) *r = void_rect;
}

int16_t VBDisplay::calc_hor_offset(int16_t width, uint8_t align)
{
	if ((align & 0x0F) == RIGHT_ALIGN) return width;
	else if ((align & 0x0F) != LEFT_ALIGN) return width / 2;

	return 0;
}


int16_t VBDisplay::calc_ver_offset(int16_t height, uint8_t align)
{
	if ((align & 0xF0) == TOP_ALIGN) return height;
	else if ((align & 0xF0) != BOTTOM_ALIGN) return height / 2;

	return 0;
}

rect VBDisplay::max_rect_no_valid_check(rect r1, rect r2)
{
	rect res;

	res.x0 = min1(r1.x0, r2.x0);
	res.y0 = min1(r1.y0, r2.y0);

	res.x1 = max1(r1.x1, r2.x1);
	res.y1 = max1(r1.y1, r2.y1);

	return res;
}

rect VBDisplay::min_rect_no_valid_check(rect r1, rect r2)
{
	rect res;

	res.x0 = max1(r1.x0, r2.x0);
	res.y0 = max1(r1.y0, r2.y0);

	res.x1 = min1(r1.x1, r2.x1);
	res.y1 = min1(r1.y1, r2.y1);

	return res;
}

rect VBDisplay::max_rect(rect r1, rect r2)
{
	if (!check_valid_rect(&r1)) return r2;
	if (!check_valid_rect(&r2)) return r1;

	return max_rect_no_valid_check(r1, r2);
}

rect VBDisplay::min_rect(rect r1, rect r2)
{
	if (!check_valid_rect(&r1)) return VBDisplay::void_rect;
	if (!check_valid_rect(&r2)) return VBDisplay::void_rect;

	return min_rect_no_valid_check(r1, r2);
}

uint16_t VBDisplay::get_obj_width(draw_obj* obj)
{
	if (obj != NULL) return obj->pos.x1 - obj->pos.x0 + 1;
	return 0;
}
uint16_t VBDisplay::get_obj_height(draw_obj* obj)
{
	if (obj != NULL) return obj->pos.y1 - obj->pos.y0 + 1;
	return 0;
}




void VBDL_InternalPushLayer::common_init(draw_obj* obj, std::unique_ptr<draw_obj> _alloc_obj, rect _mask, layer_options_t _layer_options)
{
	out_buf = NULL;
	mask = _mask;
	temp_payload = NULL;
	buf_cnt_pix = 0;
	layer_options = _layer_options;
	user_color = 0xFF000000;
	user_data = 0;

	if (_alloc_obj) {alloc_obj = std::move(_alloc_obj); obj = alloc_obj.get();}
	obj_handle = obj;

	if (obj_handle == NULL) return;
	
	if (((uint8_t)((obj_handle->color) >> 24)) == 255) return;
	user_color = obj_handle->color;

	if (obj_handle->pos.y1 < obj_handle->pos.y0) return;
	if (!VBDisplay::check_vertical_overlap(obj_handle->pos, mask)) return;

	int16_t width = min1(mask.x1, obj_handle->pos.x1) - max1(mask.x0, obj_handle->pos.x0) + 1;

	if (width <= 0) return; //check horizontal overlap

	int16_t temp = obj_handle->pos.x1 - obj_handle->pos.x0 + 1;
	if (temp <= 0) return; //img_width==0
	img_width_elem = temp;  //=img_width_pix preset

	temp = obj_handle->pos.x0 - mask.x0;
	buf_bias_elem = ((uint16_t)((temp > 0) ? temp : 0)) * 3;

	buf_cnt_pix = width;
	img_cur_y = temp = mask.y0 - obj_handle->pos.y0;
	img_height = obj_handle->pos.y1 - obj_handle->pos.y0 + 1;

	temp = mask.x0 - obj_handle->pos.x0;
	img_cur_pos = (uint32_t)((temp > 0) ? temp : 0);  //=img_bias_pix preset
}
VBDL_InternalPushLayer::VBDL_InternalPushLayer(draw_obj* obj, rect mask, layer_options_t layer_options)
{ 
	common_init(obj, nullptr, mask, layer_options);
}
VBDL_InternalPushLayer::VBDL_InternalPushLayer(std::unique_ptr<draw_obj> obj, rect mask, layer_options_t layer_options)
{
	common_init(nullptr, std::move(obj), mask, layer_options);
}
VBDL_InternalPushLayer::~VBDL_InternalPushLayer()
{
	if (out_buf == NULL) return;
	if (buf_cnt_pix == 0) return;
	if (obj_handle == NULL) return;
	if (out_buf->size() >= MAX_LAYERS_TO_BLENDING) return;

	out_buf->push_back(std::move(*this));
}

uint32_t VBDL_InternalPushLayer::calc_start_pos(uint32_t img_bias_pix, int16_t y0, uint16_t img_width_elem, uint8_t elem_per_pix)
{
	return ((y0 > 0) ? (((uint16_t)y0)*img_width_elem) : 0) + img_bias_pix*elem_per_pix;
}

uint32_t VBDL_InternalPushLayer::calc_start_pos_vertical_mirror(uint32_t img_bias_pix, int16_t y0, uint16_t img_width_elem, int16_t img_height, uint8_t elem_per_pix)
{
	if (y0 < 0) y0 = 0;
	
	img_height--;
	if (img_height < y0) return 0;
	
	return (img_height - y0)*img_width_elem + img_bias_pix*elem_per_pix;
}




#ifdef ENABLE_FRAGMEMT_DRAWING
void VBDisplay::internal_common_draw(rect mask, uint32_t end_layer)
#else
void VBDisplay::common_draw(rect mask, uint32_t end_layer)
#endif
{
	if (!check_valid_rect(&mask)) return;
    if(!crop_rect_within_screen(&mask)) return;

	uint32_t layers = min1(draw_buffer.obj.size(), end_layer);

	std::vector<internal_draw_obj> draw_list;

	int16_t mask_width = mask.x1 - mask.x0 + 1;
	clamp_min(mask_width, 0);
	uint16_t mask_width_elem = ((uint16_t)mask_width) * 3;
	
	uint32_t internal_hide_drawing_layers;
	
	if (hide_drawing_layers != NULL) internal_hide_drawing_layers = *hide_drawing_layers;
	else
	{
#ifdef ENABLE_FRAGMEMT_DRAWING
		if (draw_buffer.end_fragments_layer > draw_buffer.start_fragments_layer) internal_hide_drawing_layers = draw_buffer.end_fragments_layer;
		else
#endif
			internal_hide_drawing_layers = 1;
	}

	
	for (uint32_t i = 0; i < layers; i++)
	{
		draw_obj* img_list = (draw_buffer.obj[i]).get();

		uint16_t layer_options = 0;
		if (draw_list.size() == 0) layer_options SETBITS LAYER_OPTIONS_FIRST_LAYER;
		if (layers == 1) layer_options SETBITS LAYER_OPTIONS_SINGLE_LAYER;

		img_list->fill_str_init(draw_list, mask, layer_options);
	}

	if (draw_list.size() == 0) return;

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
		for (uint32_t i = 0; i < draw_list.size(); i++)
		{
			internal_draw_obj *pdrl = draw_list.data() + i;
			if (pdrl->obj_handle == NULL) continue;

			if ((pdrl->img_cur_y >= 0) && (pdrl->img_cur_y < pdrl->img_height))
			{
				pdrl->obj_handle->fill_str_memcpy(pb_ptr + (pdrl->buf_bias_elem), pdrl);
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
	
	for (uint32_t i = 0; i < draw_list.size(); i++)
	{
		internal_draw_obj *pdrl = draw_list.data() + i;
		pdrl->obj_handle->fill_str_memclear(pdrl);
	}
	
	if (string_alloc_buffer != NULL)
	{
		free(string_alloc_buffer);
		string_alloc_buffer = NULL;
	}
}

uint8_t* VBDisplay::string_alloc_buffer = NULL;
uint8_t* VBDisplay::alloc_additional_str_buffer_to_draw()
{
	if (string_alloc_buffer == NULL) string_alloc_buffer = (uint8_t*) malloc(ScreenWidth*4);
	return string_alloc_buffer;
}



#ifdef ENABLE_FRAGMEMT_DRAWING
void VBDisplay::common_draw(rect mask, uint32_t end_layer)
{
	if (!check_valid_rect(&mask)) return;
	
	if (draw_buffer.end_fragments_layer <= draw_buffer.start_fragments_layer)
	{
		internal_common_draw(mask, end_layer);
		return;
	}
	
	for (uint32_t layer = draw_buffer.start_fragments_layer; layer < draw_buffer.end_fragments_layer; layer++)
	{
		if (layer >= draw_buffer.obj.size()) break;
		if (!draw_buffer.obj[layer]) continue;

		draw_obj* temp = draw_buffer.obj[layer].get();
		rect part_mask = min_rect(temp->pos, mask);
		
		internal_common_draw(part_mask, end_layer);
	}
}
#endif


void VBDisplay::redraw_all()
{
	uint32_t total_elem_cnt = draw_buffer.obj.size();

	for (uint32_t i = 0; i < total_elem_cnt; i++) common_draw(draw_buffer.obj[i]->pos, i+1);
}


void VBDisplay::redraw_group_with_lower_layers_only(uint32_t first_ayer, uint32_t end_layer)
{
	if (first_ayer >= draw_buffer.obj.size() || end_layer > draw_buffer.obj.size()) return;

	for (uint32_t i = first_ayer; i < end_layer; i++) common_draw(draw_buffer.obj[i]->pos, i+1);
}


void VBDisplay::redraw_group(uint32_t first_ayer, uint32_t end_layer)
{
	if (first_ayer >= draw_buffer.obj.size() || end_layer > draw_buffer.obj.size()) return;
	
	uint32_t draw_buf_end_layer = draw_buffer.obj.size();

	for (uint32_t i = first_ayer; i < end_layer; i++)
	{
		if (end_layer == draw_buffer.obj.size()) draw_buf_end_layer = i + 1;
		common_draw(draw_buffer.obj[i]->pos, draw_buf_end_layer);
	}
}


void VBDisplay::redraw_objects_in_one_pass(std::vector <uint32_t> &obj_layers_list, rect* add_rect)
{
	rect common_rect = void_rect;
	uint32_t layers_cnt = draw_buffer.obj.size();

	for (uint32_t i = 0; i<obj_layers_list.size(); i++)
	{
		if (obj_layers_list[i] <= layers_cnt) common_rect = max_rect(draw_buffer.obj[obj_layers_list[i]]->pos, common_rect);
	}

	if (add_rect != NULL) common_rect = max_rect(*add_rect, common_rect);

	common_draw(common_rect, -1);
}

bool VBDisplay::check_valid_obj(draw_obj* obj)
{
	if (obj == NULL) return 0;
	if (!check_valid_rect(&(obj->pos))) return 0;
	return 1;
}

std::unique_ptr<draw_obj> VBDisplay::make_void_obj()
{
	std::unique_ptr<draw_obj> obj = std::make_unique<draw_void_obj>();
	return std::move(obj);
}

void VBDisplay::clear_data()
{
	draw_buffer.obj.clear();
	
#ifdef ENABLE_FRAGMEMT_DRAWING
	draw_buffer.start_fragments_layer = 0;
	draw_buffer.end_fragments_layer = 0;
#endif
}



void VBDisplay::screen_buf_insert_obj(std::unique_ptr<draw_obj> obj, uint32_t* layer_num_handle, uint32_t layer)
{
	if (!obj) return;

	obj->layer_ptr = layer_num_handle;

	if (layer >= draw_buffer.obj.size())
	{
		draw_buffer.obj.push_back(std::move(obj));
		if (layer_num_handle!=NULL) *layer_num_handle = draw_buffer.obj.size()-1;

		return;
	}
	
	if (layer_num_handle!=NULL) *layer_num_handle = layer;

	draw_buffer.obj.emplace(draw_buffer.obj.begin() + layer, std::move(obj));

	for (uint32_t i=layer+1; i < draw_buffer.obj.size(); i++)
	{
		if (draw_buffer.obj[i]->layer_ptr != NULL) *draw_buffer.obj[i]->layer_ptr = *draw_buffer.obj[i]->layer_ptr + 1;
	}
}

draw_obj* VBDisplay::screen_buf_get_obj(uint32_t layer)
{
	if (layer < draw_buffer.obj.size()) return (draw_buffer.obj[layer]).get();
	return NULL;
}

draw_obj* VBDisplay::screen_buf_get_obj(uint32_t* layer_num_handle)
{
	if (layer_num_handle == NULL) return NULL;
	if (*layer_num_handle < draw_buffer.obj.size()) return (draw_buffer.obj[*layer_num_handle]).get();
	return NULL;
}

void VBDisplay::screen_buf_delete_obj(uint32_t layer)
{
	if (layer < draw_buffer.obj.size())
	{
		if (draw_buffer.obj[layer]->layer_ptr != NULL) *draw_buffer.obj[layer]->layer_ptr = 0xFFFFFFFF;
		draw_buffer.obj.erase(draw_buffer.obj.begin() + layer);
	}

	for (uint32_t i=layer; i < draw_buffer.obj.size(); i++)
	{
		if (draw_buffer.obj[i]->layer_ptr != NULL) *draw_buffer.obj[i]->layer_ptr = *draw_buffer.obj[i]->layer_ptr - 1;
	}
}

void VBDisplay::screen_buf_update_obj(std::unique_ptr<draw_obj> obj, uint32_t layer)
{
	if (!obj)
	{
		screen_buf_delete_obj(layer);
		return;
	}

	if (layer < draw_buffer.obj.size())
	{
		std::unique_ptr<draw_obj> old_obj_to_delete = std::move(draw_buffer.obj[layer]);
		obj->layer_ptr = old_obj_to_delete->layer_ptr;
		draw_buffer.obj[layer] = std::move(obj);
	}
}

uint32_t VBDisplay::set_or_update_obj(std::unique_ptr<draw_obj> obj, uint32_t* layer_num_handle, uint32_t desired_layer, bool update_on_the_screen)
{
	uint32_t req_layer = (layer_num_handle != NULL)?(*layer_num_handle):desired_layer;

	if (req_layer < draw_buffer.obj.size())
	{
		if (update_on_the_screen) update_obj(std::move(obj), req_layer);
		else screen_buf_update_obj(std::move(obj), req_layer);

		return req_layer + 1;
	}
	else
	{
		if (!obj) return desired_layer;

		if (desired_layer > draw_buffer.obj.size()) desired_layer = draw_buffer.obj.size();

		screen_buf_insert_obj(std::move(obj), layer_num_handle, desired_layer);
		if (update_on_the_screen) common_draw((draw_buffer.obj[desired_layer]).get()->pos, 0xFFFFFFFF);

		return desired_layer + 1;
	}
}

void VBDisplay::pin_layer_handle_to_obj(uint32_t obj_layer, uint32_t* layer_num_handle)
{
	if (obj_layer < draw_buffer.obj.size())
	{
		draw_buffer.obj[obj_layer]->layer_ptr = layer_num_handle;
	}
}

void VBDisplay::change_obj_layer(uint32_t obj_layer, uint32_t new_layer)
{
	if (obj_layer>=draw_buffer.obj.size() || new_layer>=draw_buffer.obj.size() || obj_layer == new_layer) return;

	std::unique_ptr moving_obj = std::move(draw_buffer.obj[obj_layer]);
	draw_buffer.obj.erase(draw_buffer.obj.begin() + obj_layer);
	draw_buffer.obj.emplace(draw_buffer.obj.begin() + new_layer, std::move(moving_obj));

	uint32_t start_layer = min1(obj_layer, new_layer), end_layer = max1(obj_layer, new_layer);
	for (uint32_t i=start_layer; i <= end_layer; i++)
	{
		if (draw_buffer.obj[i]->layer_ptr != NULL) *draw_buffer.obj[i]->layer_ptr = i;
	}
}

void VBDisplay::redraw_obj(uint32_t img_layer)
{
	if (img_layer >= draw_buffer.obj.size()) return;
	
	common_draw(draw_buffer.obj[img_layer]->pos, 0xFFFFFFFF);
}

void VBDisplay::delete_obj(uint32_t img_layer)
{
	if (img_layer >= draw_buffer.obj.size()) return;
	
	draw_buffer.obj[img_layer]->color = DELETE_IMG_COLOR;
	common_draw(draw_buffer.obj[img_layer]->pos, 0xFFFFFFFF);

	screen_buf_delete_obj(img_layer);
}

void VBDisplay::hide_obj(uint32_t img_layer)
{
	if (img_layer >= draw_buffer.obj.size()) return;
	
	draw_buffer.obj[img_layer]->color |= DELETE_IMG_COLOR;
	common_draw(draw_buffer.obj[img_layer]->pos, 0xFFFFFFFF);
}


void VBDisplay::update_obj(std::unique_ptr<draw_obj> new_obj, uint32_t img_layer)
{
	if (img_layer >= draw_buffer.obj.size()) return;

	rect cur_pos;
	draw_obj* cur_obj = (draw_buffer.obj[img_layer]).get();
	
	if (cur_obj != NULL) cur_pos = cur_obj->pos;
	else cur_pos = void_rect;

	draw_obj* new_obj_dataptr = new_obj.get();
	rect new_pos;

	if (new_obj_dataptr != NULL) new_pos = new_obj_dataptr->pos;
	else new_pos = void_rect;

	screen_buf_update_obj(std::move(new_obj), img_layer);
	update_area_on_screen(cur_pos, new_pos);
}

#ifdef ENABLE_FRAGMEMT_DRAWING
#define intrn_common_draw2(del_mask, end_layer) internal_common_draw(del_mask, end_layer)
#else
#define intrn_common_draw2(del_mask, end_layer) common_draw(del_mask, end_layer)
#endif

void VBDisplay::update_area_on_screen(rect cur_pos, rect new_pos)
{
	if (check_valid_rect(&cur_pos))
	{
		//Delete the image in whole or in part if the old and new images overlap//////////////////////
		rect part_mask, del_mask;

		//cur_img2->color = DELETE_IMG_COLOR;
		
#ifdef ENABLE_FRAGMEMT_DRAWING
		
		uint8_t continue_cycle = 1;
		for (uint32_t layer = draw_buffer.start_fragments_layer; continue_cycle; layer++)
		{
			if (draw_buffer.end_fragments_layer <= draw_buffer.start_fragments_layer) 
			{
				part_mask = cur_pos;
				continue_cycle = 0;
			}
			else
			{
				if (layer >= draw_buffer.end_fragments_layer) break;
						
				part_mask = min_rect(draw_buffer.obj[layer]->pos, cur_pos);
				if (!check_valid_rect(&part_mask)) continue;
			}
#else
		{
			
			part_mask = cur_pos;
#endif

			
			int16_t temp_y0 = max1(part_mask.y0, min1(part_mask.y1, new_pos.y1) + 1);
			int16_t temp_y1 = min1(part_mask.y1, max1(part_mask.y0, new_pos.y0) - 1);
				
			//delete sect1//
			del_mask.x1 = part_mask.x1;
			del_mask.x0 = part_mask.x0;
			del_mask.y1 = temp_y1;
			del_mask.y0 = part_mask.y0;

			intrn_common_draw2(del_mask, 0xFFFFFFFF);

			//delete sect3//
			del_mask.y1 = part_mask.y1;
			del_mask.y0 = temp_y0;

			intrn_common_draw2(del_mask, 0xFFFFFFFF);

			//delete sect2
			del_mask.x1 = min1(part_mask.x1, new_pos.x0 - 1);
			//del_mask.x0 = part_mask.x0;
			del_mask.y1 = temp_y0 - 1;
			del_mask.y0 = temp_y1 + 1;

			intrn_common_draw2(del_mask, 0xFFFFFFFF);

			//delete sect4
			del_mask.x1 = part_mask.x1;
			del_mask.x0 = max1(part_mask.x0, new_pos.x1 + 1);

			intrn_common_draw2(del_mask, 0xFFFFFFFF);
		}
	}

	if (check_valid_rect(&new_pos)) common_draw(new_pos, 0xFFFFFFFF);
}


void VBDisplay::move_obj_to_new_xy(draw_obj* obj, int16_t new_x, int16_t new_y, uint8_t align)
{
	if (obj == NULL) return;

	rect cur_pos = obj->pos;

    obj->pos=get_rect(new_x, new_y, cur_pos.x1 - cur_pos.x0+1, cur_pos.y1 - cur_pos.y0+1, align);
	obj->x0 += new_x;
	obj->y0 += new_y;
}
void VBDisplay::move_obj_dxdy(draw_obj* obj, int16_t dx, int16_t dy)
{
	if ((dx == 0 && dy == 0) || obj == NULL) return;

	obj->x0 += dx;
	obj->y0 += dy;
	obj->pos.x0 += dx;
	obj->pos.x1 += dx;
	obj->pos.y0 += dy;
	obj->pos.y1 += dy;
}

void VBDisplay::move_obj_to_new_xy(uint32_t img_layer, int16_t new_x, int16_t new_y, uint8_t align, bool update_on_the_screen)
{
	if (img_layer >= draw_buffer.obj.size()) return;

	draw_obj* new_img = (draw_buffer.obj[img_layer]).get();
	rect cur_pos = new_img->pos;

    new_img->pos=get_rect(new_x, new_y, cur_pos.x1 - cur_pos.x0+1, cur_pos.y1 - cur_pos.y0+1, align);
	new_img->x0 += new_x;
	new_img->y0 += new_y;

	if (!update_on_the_screen) return;
	if (check_equal_rects(&(new_img->pos), &cur_pos)) return;

	update_area_on_screen(cur_pos, new_img->pos);
}

void VBDisplay::move_obj_dxdy(uint32_t img_layer, int16_t dx, int16_t dy, bool update_on_the_screen)
{
	if (dx == 0 && dy == 0) return;

	draw_obj* new_img = (draw_buffer.obj[img_layer]).get();
	rect cur_pos = new_img->pos;

	new_img->x0 += dx;
	new_img->y0 += dy;
	new_img->pos.x0 += dx;
	new_img->pos.x1 += dx;
	new_img->pos.y0 += dy;
	new_img->pos.y1 += dy;

	if (!update_on_the_screen) return;

	update_area_on_screen(cur_pos, new_img->pos);
}


void VBDisplay::align_obj_group(std::vector <uint32_t> &obj_layers_list, int16_t new_x, int16_t new_y, uint8_t align)
{
	rect common_rect = void_rect;
	uint32_t layers_cnt = draw_buffer.obj.size();

	for (uint32_t i = 0; i<obj_layers_list.size(); i++)
	{
		if (obj_layers_list[i] <= layers_cnt) common_rect = max_rect(draw_buffer.obj[obj_layers_list[i]]->pos, common_rect);
	}

	int16_t dx = -calc_hor_offset(common_rect.x1-common_rect.x0+1, align);
	int16_t dy = -calc_ver_offset(common_rect.y1-common_rect.y0+1, align);

	for (uint32_t i = 0; i<obj_layers_list.size(); i++)
	{
		if (obj_layers_list[i] <= layers_cnt)
		{
			int16_t obj_dx = -draw_buffer.obj[obj_layers_list[i]]->x0;
			int16_t obj_dy = -draw_buffer.obj[obj_layers_list[i]]->y0;

			draw_buffer.obj[obj_layers_list[i]]->x0 = new_x + dx;
			draw_buffer.obj[obj_layers_list[i]]->y0 = new_y + dy;

			obj_dx += draw_buffer.obj[obj_layers_list[i]]->x0;
			obj_dy += draw_buffer.obj[obj_layers_list[i]]->y0;

			draw_buffer.obj[obj_layers_list[i]]->pos.x0 += obj_dx;
			draw_buffer.obj[obj_layers_list[i]]->pos.x1 += obj_dx;
			draw_buffer.obj[obj_layers_list[i]]->pos.y0 += obj_dy;
			draw_buffer.obj[obj_layers_list[i]]->pos.y1 += obj_dy;
		}
	}
}

void VBDisplay::align_obj_group(std::vector <draw_obj*> &obj_list, int16_t new_x, int16_t new_y, uint8_t align)
{
	rect common_rect = void_rect;

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

void VBDisplay::align_obj_group(std::vector <std::unique_ptr<draw_obj>> &obj_list, int16_t new_x, int16_t new_y, uint8_t align)
{
	rect common_rect = void_rect;

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





void internal_draw_obj::another_obj_fill_str_init(std::vector <internal_draw_obj> &out_buf, std::unique_ptr<draw_obj> obj, rect mask, uint16_t layer_options)
{
	size_t buf_size = out_buf.size();
	obj->fill_str_init(out_buf, mask, layer_options);

	if ((buf_size + 1) == out_buf.size())
	{
		internal_draw_obj* out_buf_ptr = &(out_buf[buf_size]);

		if (!(out_buf_ptr->alloc_obj))
		{
			out_buf_ptr->alloc_obj = std::move(obj);
			out_buf_ptr->obj_handle = out_buf_ptr->alloc_obj.get();
		}
	}
}
#pragma once


#include "app_hal_api.h"




typedef struct 
{
	int16_t x0;
	int16_t y0;
	int16_t x1;
	int16_t y1;
}rect;


#if defined(__cplusplus)

class VBDisplay;
class draw_obj;

typedef uint16_t layer_options_t;

class internal_draw_obj
{
public:
	std::unique_ptr<draw_obj> alloc_obj;
	draw_obj* obj_handle;

	void* temp_payload;
  
	uint16_t img_width_elem; 
	int16_t img_height;
  
	uint16_t buf_bias_elem;  //elem==pix*3
	uint16_t buf_cnt_pix;
  
	uint32_t img_cur_pos;
	int16_t img_cur_y;
	layer_options_t layer_options;
	uint32_t user_color;
	uint32_t user_data;

	static void another_obj_fill_str_init(std::vector <internal_draw_obj> &out_buf, std::unique_ptr<draw_obj> obj, rect mask, uint16_t layer_options);
};


class draw_obj
{
	friend class VBDisplay;
	friend class internal_draw_obj;

public:
	int16_t x0;
	int16_t y0;
	rect pos;
	uint16_t options;
	uint32_t color;
	uint32_t *layer_ptr;

	static constexpr rect void_rect = {.x0 = -1, .y0 = -1, .x1 = -2, .y1 = -2};

	draw_obj()
	{
		x0 = 0;
		y0 = 0;
		pos = void_rect;
		options = 0;
		color = 0xFF000000;
		layer_ptr = NULL;
	}

	virtual ~draw_obj() = default;

protected:
	virtual void fill_str_init(std::vector <internal_draw_obj> &buf, rect mask, uint16_t layer_options) = 0;
	virtual void fill_str_memcpy(uint8_t* buf, internal_draw_obj* img) = 0;
	virtual void fill_str_memclear(internal_draw_obj* img) = 0;
};


class draw_void_obj : public draw_obj
{
protected:
	void fill_str_init(std::vector <internal_draw_obj> &buf, rect mask, uint16_t layer_options) override {}
	void fill_str_memcpy(uint8_t* buf, internal_draw_obj* img) override {}
	void fill_str_memclear(internal_draw_obj* img) override {}
};


struct draw_obj_list
{
	std::vector <std::unique_ptr<draw_obj>> obj;
	
#ifdef ENABLE_FRAGMEMT_DRAWING
	uint32_t start_fragments_layer;
	uint32_t end_fragments_layer;

	draw_obj_list() {start_fragments_layer = end_fragments_layer = 0;}
#endif
};


#endif






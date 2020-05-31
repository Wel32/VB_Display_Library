#pragma once


#include "../user_drivers/system_include.h"






typedef struct 
{
	int16_t x0;
	int16_t y0;
	int16_t x1;
	int16_t y1;
}rect;


typedef struct
{
	rect pos;
	uint16_t obj_type;
	uint16_t options;
	void* handle;
	int32_t color;
}
draw_obj;


typedef struct
{
	draw_obj* obj;
	uint8_t elem_cnt;
#ifdef ENABLE_FRAGMEMT_DRAWING
	uint8_t start_fragments_layer;
	uint8_t end_fragments_layer;
#endif
}
draw_obj_list;



typedef struct
{
	uint16_t obj_type;
	uint16_t layer_options;
	void* handle;
  
	uint16_t img_width_elem; 
	int16_t img_height;
  
	uint16_t buf_bias_elem;  //elem==pix*3
	uint16_t buf_cnt_pix;
  
	uint32_t img_cur_pos;
	int16_t img_cur_y;
	uint16_t options;
	int32_t user_color;
	int32_t user_data;
}
internal_draw_obj;



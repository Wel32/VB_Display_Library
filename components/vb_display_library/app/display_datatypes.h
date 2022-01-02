#pragma once


#include "app_hal_api.h"






typedef struct 
{
	int16_t x0;
	int16_t y0;
	int16_t x1;
	int16_t y1;
}rect;


typedef struct
{
	void* handle;
	int16_t x0;
	int16_t y0;
	rect pos;
	uint16_t obj_type;
	uint16_t options;
	size_t size;
	uint32_t color;
	uint32_t(*dynamic_data_constr)(void** out_buf, void* in_buf, rect mask);
	uint32_t *layer_ptr;
}
draw_obj;



#if defined(__cplusplus)


typedef void (*destr_alloc_data_func_t)(void*);
typedef struct
{
	void* handle;
	destr_alloc_data_func_t destr_func;
}
alloc_data_with_destructor_t;

typedef struct
{
	std::vector <draw_obj> obj;
	std::vector <alloc_data_with_destructor_t> alloc_data_register;
#ifdef ENABLE_FRAGMEMT_DRAWING
	uint32_t start_fragments_layer;
	uint32_t end_fragments_layer;
#endif
}
draw_obj_list;


#endif


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
	uint32_t user_color;
	uint32_t user_data;
}
internal_draw_obj;



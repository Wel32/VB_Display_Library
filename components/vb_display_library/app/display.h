#pragma once

#include "display_options.h"
#include "display_init.h"
#include "display_led.h"
#include "draw.h"







#if ORIENTATIONS_COUNT == 4 || ORIENTATIONS_COUNT == 2
extern uint16_t _ScreenWidth;
extern uint16_t _ScreenHeight;
#endif



#include "display_macro.h"






	
uint32_t invert_color(uint32_t color);
uint32_t invert_brightness(uint32_t color);

void lcdSetRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint8_t align);
rect get_rect(int16_t x0, int16_t y0, uint16_t width, uint16_t height, uint8_t align);
rect make_void_rect();
bool crop_rect_within_screen(rect* r);
bool check_rect_direction(rect* r);
bool check_equal_rects(rect* r1, rect* r2);

void move_rect_to_new_xy(rect *r, int16_t new_x, int16_t new_y);
void move_rect_dxdy(rect *r, int16_t dx, int16_t dy);

int16_t calc_hor_offset(int16_t width, uint8_t align);
int16_t calc_ver_offset(int16_t height, uint8_t align);

rect min_rect(rect r1, rect r2);
rect max_rect(rect r1, rect r2);
rect common_rect(draw_obj* scr_obj, uint32_t cnt);



#if defined(__cplusplus)

bool check_valid_obj(draw_obj* obj);

void common_draw(draw_obj_list* draw_buffer, rect mask, uint32_t end_layer);

void redraw_all(draw_obj_list* draw_buffer);
void redraw_group(draw_obj_list* draw_buffer, uint32_t first_layer, uint32_t end_layer);
void redraw_objects_in_one_pass(draw_obj_list* draw_buffer, std::vector <uint32_t> &obj_layers_list, rect* add_rect);
void redraw_group_with_lower_layers_only(draw_obj_list* draw_buffer, uint32_t first_ayer, uint32_t end_layer);


draw_obj make_void_obj();
void clear_screen_data(draw_obj_list* dl);
void set_or_update_obj(draw_obj_list* draw_buffer, draw_obj obj, uint32_t* layer_num_store, bool update_on_the_screen = 1, bool update_obj_if_exist = 1);

void draw_new_obj(draw_obj_list* draw_buffer, uint32_t img_layer);
void delete_obj(draw_obj_list* draw_buffer, uint32_t img_layer);
void hide_obj(draw_obj_list* draw_buffer, uint32_t img_layer);
void _update_obj(draw_obj_list* draw_buffer, uint32_t img_layer, draw_obj* new_img);
void update_obj(draw_obj_list* draw_buffer, uint32_t img_layer, draw_obj new_img);
void move_obj_to_new_xy(draw_obj_list* draw_buffer, uint32_t img_layer, int16_t new_x, int16_t new_y, uint8_t align);
void move_obj_dxdy(draw_obj_list* draw_buffer, uint32_t img_layer, int16_t dx, int16_t dy);
void align_obj_group(draw_obj_list* draw_buffer, std::vector <uint32_t> &obj_layers_list, int16_t new_x, int16_t new_y, uint8_t align);
void screen_buf_insert_obj(draw_obj_list* draw_buffer, draw_obj obj, uint32_t* layer_num_store, uint32_t layer);
draw_obj* screen_buf_get_obj(draw_obj_list* draw_buffer, uint32_t layer);
void screen_buf_delete_obj(draw_obj_list* draw_buffer, uint32_t layer);
void screen_buf_update_obj(draw_obj_list* draw_buffer, draw_obj obj, uint32_t layer);

	

void* push_alloc_data(draw_obj_list* draw_buffer, void* data, size_t size);
void attach_alloc_data(draw_obj_list* draw_buffer, alloc_data_with_destructor_t data);
void delete_alloc_data(draw_obj_list* draw_buffer, uint32_t layer);

#endif
	





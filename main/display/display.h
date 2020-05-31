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

void common_draw(draw_obj_list draw_buffer, rect mask);

void redraw_all(draw_obj_list draw_buffer);
void redraw_group(draw_obj_list draw_buffer, uint8_t first_layer, uint8_t end_layer);


void move_rect_to_new_xy(rect *r, int16_t new_x, int16_t new_y);
void move_rect_dxdy(rect *r, int16_t dx, int16_t dy);

draw_obj make_void_obj();
void clear_screen_data(draw_obj_list* dl);
void draw_img(draw_obj_list draw_buffer, uint8_t img_layer);
void delete_img(draw_obj_list draw_buffer, uint8_t img_layer);
void hide_img(draw_obj_list draw_buffer, uint8_t img_layer);
void _update_img(draw_obj_list draw_buffer, uint8_t img_i, draw_obj* new_img);
void update_img(draw_obj_list draw_buffer, uint8_t img_i, draw_obj new_img);
void move_img_to_new_xy(draw_obj_list draw_buffer, uint8_t img_layer, int16_t new_x, int16_t new_y, uint8_t align);
void move_img_dxdy(draw_obj_list draw_buffer, uint8_t img_layer, int16_t dx, int16_t dy);

void align_group(draw_obj* draw_buffer, uint8_t first_layer, uint8_t end_layer, uint8_t align);

int16_t calc_hor_offset(int16_t width, uint8_t align);
int16_t calc_ver_offset(int16_t height, uint8_t align);

rect min_rect(rect r1, rect r2);
rect max_rect(rect r1, rect r2);
rect common_rect(draw_obj* scr_obj, uint8_t cnt);
	




#define INIT_DROBJ_STRING(scrn, scrn_data) {scrn.obj = scrn_data; scrn.elem_cnt = (sizeof(scrn_data)/sizeof(draw_obj));}
#define NEW_SCREEN(scrn, scrn_data, layers_cnt) draw_obj scrn_data[layers_cnt]; draw_obj_list scrn; INIT_DROBJ_STRING(scrn, scrn_data); clear_screen_data(&scrn);


	




